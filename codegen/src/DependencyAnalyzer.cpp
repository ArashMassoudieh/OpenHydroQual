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

// Recursively collect (name, location) of every parameter leaf in an Expression.
static void collectRefs(const Expression& e, std::vector<std::pair<std::string, Loc>>& out)
{
    if (e.param_constant_expression == "parameter") {
        out.emplace_back(e.parameter, locFromText(e.text));
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
                    info.dependencies.push_back(resolveRef(system, obj, isLink, r.first, r.second));
                break;
            }
            case Quan::_type::rule:
                // Conservative: rules are treated as per-iteration (state) for now.
                info.dependsOnState = true;
                break;
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
