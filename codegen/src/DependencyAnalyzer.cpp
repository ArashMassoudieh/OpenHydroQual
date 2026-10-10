/*
 * OpenHydroQual - Codegen: dependency analysis & quantity tiering (impl)
 * Copyright (C) 2025 EnviroInformatics, LLC
 */
#include "DependencyAnalyzer.h"
#include "ExpressionEmitter.h"   // for ohqcg::Loc

#include "System.h"
#include "Block.h"
#include "Link.h"
#include "Object.h"
#include "Quan.h"
#include "QuanSet.h"
#include "Expression.h"

#include <vector>
#include <string>
#include <cstdlib>

namespace ohqcg {

// ---- helpers ---------------------------------------------------------------

// Parse the location suffix of a parameter token ("x", "x.s", "x.e", "x.v").
static Loc locFromText(const std::string& t)
{
    auto dot = t.find_last_of('.');
    if (dot == std::string::npos || dot + 2 != t.size()) return Loc::self;
    char c = t[dot + 1];
    if (c == 's' || c == 'S') return Loc::source;
    if (c == 'e' || c == 'E') return Loc::destination;
    if (c == 'v' || c == 'V') return Loc::average_of_links;
    return Loc::self;
}

// The parsed location of a parameter leaf (text may not carry the suffix).
static Loc locOf(const Expression& e)
{
    switch (e.GetLocation()) {
    case Expression::loc::source:           return Loc::source;
    case Expression::loc::destination:      return Loc::destination;
    case Expression::loc::average_of_links: return Loc::average_of_links;
    default:                                return locFromText(e.text);
    }
}

// Collect leaves, remapping self-located refs to `selfAs` (for _ups/_bkw args).
static void collectRefsSelfAs(const Expression& e, std::vector<std::pair<std::string, Loc>>& out, Loc selfAs)
{
    if (e.param_constant_expression == "parameter") {
        Loc loc = locOf(e);
        if (loc == Loc::self) loc = selfAs;
        out.emplace_back(e.parameter, loc);
        return;
    }
    for (const Expression& t : e.terms)
        collectRefsSelfAs(t, out, selfAs);
}

// Recursively collect (name, location) of every parameter leaf in an Expression.
static void collectRefs(const Expression& e, std::vector<std::pair<std::string, Loc>>& out)
{
    if (e.param_constant_expression == "parameter") {
        out.emplace_back(e.parameter, locOf(e));
        return;
    }
    // The 2-argument link forms of _ups/_bkw evaluate their arguments at the
    // source and destination blocks, so self-located refs inside them actually
    // depend on both endpoints. Record both so tiering/ordering see the coupling.
    if ((e.function == "ups" || e.function == "bkw") && e.terms.size() == 2) {
        for (const Expression& t : e.terms) {
            collectRefsSelfAs(t, out, Loc::source);
            collectRefsSelfAs(t, out, Loc::destination);
        }
        return;
    }
    for (const Expression& t : e.terms)
        collectRefs(t, out);
}

static std::string key(const std::string& obj, const std::string& q)
{
    return obj + "::" + q;
}

// Resolve a reference from `owner` (object index, or -1 for a block) to the node
// key of the referenced quantity, following link endpoints for source/dest.
static std::string resolveRef(System& sys, Object* owner, bool ownerIsLink,
                              const std::string& name, Loc loc)
{
    if (!ownerIsLink || loc == Loc::self)
        return key(owner->GetName(), name);
    if (loc == Loc::source) {
        const Block* b = sys.block(owner->s_Block_No());
        if (b) return key(const_cast<Block*>(b)->GetName(), name);
    } else if (loc == Loc::destination) {
        const Block* b = sys.block(owner->e_Block_No());
        if (b) return key(const_cast<Block*>(b)->GetName(), name);
    }
    // average_of_links or unresolved: fall back to self-scope name
    return key(owner->GetName(), name);
}

// Append the node keys a reference depends on. A block's "x.v" (average over
// the links touching it, Block::GetAvgOverLinks) depends on x of each of those
// links, so e.g. a segment velocity averaged from link flows is per-iteration,
// not a constant evaluated once at t0.
static void addDeps(System& sys, Object* owner, bool ownerIsLink, const std::string& name, Loc loc,
                    std::vector<std::string>& deps)
{
    if (!ownerIsLink && loc == Loc::average_of_links) {
        for (unsigned l = 0; l < sys.LinksCount(); ++l) {
            Link* L = sys.link(l);
            const bool touches = (Object*)sys.block(L->s_Block_No()) == owner ||
                                 (Object*)sys.block(L->e_Block_No()) == owner;
            if (touches && L->HasQuantity(name)) deps.push_back(key(L->GetName(), name));
        }
        return;
    }
    deps.push_back(resolveRef(sys, owner, ownerIsLink, name, loc));
}

// ---- analysis --------------------------------------------------------------

AnalysisResult DependencyAnalyzer::analyze(System& system) const
{
    AnalysisResult result;

    // Pass 1: create a node per (object, quantity) with base labels + refs.
    auto addObject = [&](Object* obj, int index, bool isLink) {
        QuanSet* qs = obj->GetVars();
        for (auto it = qs->begin(); it != qs->end(); ++it) {
            Quan& q = it->second;
            QuantityInfo info;
            info.objectIndex   = index;
            info.objectName    = obj->GetName();
            info.quantityName  = q.GetName();

            switch (q.GetType()) {
            case Quan::_type::balance:
                info.dependsOnState = true;                 // root: a state variable
                break;
            case Quan::_type::timeseries:
            case Quan::_type::prec_timeseries:
            case Quan::_type::source:
                info.dependsOnTime = true;                  // forcing
                break;
            case Quan::_type::constant:
            case Quan::_type::value:
            case Quan::_type::boolean:
                info.isFolded = true;
                info.foldedValue = std::atof(q.GetProperty(true).c_str());
                break;
            case Quan::_type::expression: {
                std::vector<std::pair<std::string, Loc>> refs;
                collectRefs(*q.GetExpression(), refs);
                for (auto& r : refs)
                    addDeps(system, obj, isLink, r.first, r.second, info.dependencies);
                break;
            }
            case Quan::_type::rule: {
                // Collect references from every condition operand and every
                // result expression so the rule tiers/orders like an expression.
                Rule* r = q.GetRule();
                if (r) {
                    std::vector<std::pair<std::string, Loc>> refs;
                    for (int ri = 0; ri < r->Count(); ++ri) {
                        _condplusresult* cr = (*r)[ri];
                        const Condition& c = cr->condition;
                        if (c.Count() < 2) continue;   // skip metadata (e.g. "unit")
                        for (unsigned e = 0; e < c.Count(); ++e)
                            collectRefs(c.Expr(e), refs);
                        collectRefs(cr->result, refs);
                    }
                    for (auto& rf : refs)
                        addDeps(system, obj, isLink, rf.first, rf.second, info.dependencies);
                }
                break;
            }
            default:
                break;
            }
            result[key(info.objectName, info.quantityName)] = info;
        }
    };

    for (unsigned i = 0; i < system.BlockCount(); ++i)
        addObject(system.block(i), static_cast<int>(i), /*isLink=*/false);
    for (unsigned i = 0; i < system.LinksCount(); ++i)
        addObject(system.link(i), static_cast<int>(i), /*isLink=*/true);

    // Pass 2: propagate state/time labels to a fixed point over the ref graph.
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& kv : result) {
            QuantityInfo& n = kv.second;
            for (const std::string& dep : n.dependencies) {
                auto it = result.find(dep);
                if (it == result.end()) continue;
                if (it->second.dependsOnState && !n.dependsOnState) { n.dependsOnState = true; changed = true; }
                if (it->second.dependsOnTime  && !n.dependsOnTime)  { n.dependsOnTime  = true; changed = true; }
            }
        }
    }

    // Pass 3: assign tiers from final labels.
    for (auto& kv : result) {
        QuantityInfo& n = kv.second;
        if (n.dependsOnState)      n.tier = Tier::PerIteration;
        else if (n.dependsOnTime)  n.tier = Tier::PerStep;
        else                       n.tier = Tier::Constant;
    }

    return result;
}

} // namespace ohqcg
