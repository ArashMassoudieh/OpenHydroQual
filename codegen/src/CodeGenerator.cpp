/*
 * OpenHydroQual - Codegen: model -> standalone C++ solver library (impl, v1)
 * Copyright (C) 2025 EnviroInformatics, LLC
 *
 * Supported quantity types in this first version:
 *   constant, value, boolean  -> folded literals
 *   expression                -> straight-line C++ (via ExpressionEmitter)
 *   timeseries, prec_timeseries, source -> user-supplied ohq::TimeSeries input
 *   balance                   -> flat state array + storage residual
 * Deferred (throws a clear error): rule (needs Condition introspection),
 * constituents/transport, rigid/outflow-limit switching.
 *
 * The emitted class mirrors demo/two_pond_model.h and links only the header-only
 * ohq runtime.
 */
#include "CodeGenerator.h"
#include "DependencyAnalyzer.h"
#include "ExpressionEmitter.h"
#include "EmbeddedRuntime.h"

#include <cctype>
#include <filesystem>

#include "System.h"
#include "Block.h"
#include "Link.h"
#include "Object.h"
#include "Quan.h"
#include "QuanSet.h"
#include "Expression.h"
#include "Source.h"
#include "reaction.h"
#include "RxnParameter.h"

#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <charconv>
#include <array>
#include <cctype>
#include <algorithm>
#include <functional>
#include <cmath>

namespace ohqcg {

// ---- small helpers ---------------------------------------------------------
static std::string sanitize(const std::string& s)
{
    std::string o;
    for (char c : s) o += (std::isalnum((unsigned char)c) ? c : '_');
    if (o.empty() || std::isdigit((unsigned char)o[0])) o = "_" + o;
    return o;
}
static std::string sym(const std::string& obj, const std::string& q)
{
    return sanitize(obj) + "__" + sanitize(q) + "_";
}
static std::string tsHandle(const std::string& obj, const std::string& q)
{
    return "ts_" + sanitize(obj) + "__" + sanitize(q) + "_";
}
static std::string stateEnum(const std::string& block, const std::string& var)
{
    return sanitize(block) + "__" + sanitize(var);
}
static std::string massEnum(const std::string& block, const std::string& constName)
{
    return sanitize(block) + "__" + sanitize(constName) + "__mass";
}
// Constituent prefix of a quantity name ("Tracer:concentration" -> "Tracer"), or "".
static std::string constPrefix(const std::string& name)
{
    auto c = name.find(':');
    return c == std::string::npos ? std::string() : name.substr(0, c);
}
static std::string fmt(double v)
{
    std::array<char, 64> buf{};
    auto r = std::to_chars(buf.data(), buf.data() + buf.size(), v);
    std::string s(buf.data(), r.ptr);
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos &&
        s.find('E') == std::string::npos) s += ".0";
    return s;
}
static bool isSeriesType(Quan::_type t)
{
    return t == Quan::_type::timeseries || t == Quan::_type::prec_timeseries ||
           t == Quan::_type::source;
}
// A directly-baked time series (not a Source object, which is handled separately).
static bool isPlainSeries(Quan::_type t)
{
    return t == Quan::_type::timeseries || t == Quan::_type::prec_timeseries;
}
static std::string srcHandle(const std::string& sourceName)
{
    return "ts_src_" + sanitize(sourceName) + "_";
}
// A source's OTHER time-series inputs (Penman ET: Temperature, wind_speed, R_h,
// solar_radiation) and its internal expression quantities (B, e_as, Ea, Delta,
// Er, rate), which are evaluated in the source's own context.
static std::string srcTs(const std::string& sourceName, const std::string& q)
{
    return "ts_src_" + sanitize(sourceName) + "__" + sanitize(q) + "_";
}
static std::string srcFn(const std::string& sourceName, const std::string& q)
{
    return "src_" + sanitize(sourceName) + "__" + sanitize(q) + "_fn";
}

// ---- generator -------------------------------------------------------------

bool CodeGenerator::generate(System& system, const GenOptions& opt)
{
    const AnalysisResult tiers = DependencyAnalyzer().analyze(system);

    const unsigned nB = system.BlockCount();
    const unsigned nL = system.LinksCount();
    if (nB == 0) throw std::runtime_error("CodeGenerator: model has no blocks");

    // ---- state variable + flux wiring (assumed uniform across blocks) ------
    const std::string stateVar = opt.stateVariable;
    Quan* bal0 = system.block(0)->Variable(stateVar);
    if (!bal0 || bal0->GetType() != Quan::_type::balance)
        throw std::runtime_error("CodeGenerator: block 0 has no balance quantity '" + stateVar + "'");
    const std::string flowVar = bal0->GetCorrespondingFlowVar();

    // A resolver factory bound to the current object / time symbol.
    // (std::function so resolveValue can call makeCtx recursively for Source
    // coefficient/rate sub-expressions.)
    std::function<EmitContext(Object*, bool, const std::string&)> makeCtx;
    makeCtx = [&](Object* cur, bool isLink, const std::string& timeVar) -> EmitContext {
        EmitContext ctx;
        ctx.timeVar = timeVar;
        auto target = [&, cur, isLink](Loc loc) -> Object* {
            if (!isLink || loc == Loc::self) return cur;
            if (loc == Loc::source)      return (Object*)system.block(cur->s_Block_No());
            if (loc == Loc::destination) return (Object*)system.block(cur->e_Block_No());
            return cur;
        };
        ctx.resolveValue = [&, target, timeVar](const std::string& name, Loc loc) -> std::string {
            Object* t = target(loc);
            Quan* q = t->Variable(name);
            if (!q) return sanitize(name) + "_ /*UNRESOLVED*/";
            if (q->GetType() == Quan::_type::balance)
                return "eff[" + stateEnum(t->GetName(), name) + "]";
            if (q->GetType() == Quan::_type::source) {
                // Source value = coefficient * timeseries * rate, per Source::GetValue.
                // `coefficient` is evaluated in THIS object's context (it reads the
                // block's area/depth). `rate` and everything it references live on
                // the Source itself (Penman ET: rate <- Delta, Er, Ea <- e_as, B <-
                // Temperature / wind_speed / R_h series), so it is evaluated in the
                // source's own context through the src_<name>__<q>_fn(t) member
                // functions emitted by the source pre-pass. A source with no
                // `timeseries` quantity contributes 1 for that factor (interpreter).
                Source* s = q->GetSource();
                if (!s) return "0.0";
                const bool tIsLink = (t->ObjectType() == object_type::link);
                std::string coeff = "1.0", rate = "1.0";
                if (Quan* cq = s->Variable("coefficient")) {
                    if (cq->GetType() == Quan::_type::expression) {
                        ExpressionEmitter em(makeCtx(t, tIsLink, timeVar));
                        coeff = "(" + em.translate(*cq->GetExpression()) + ")";
                    } else coeff = fmt(std::atof(cq->GetProperty(true).c_str()));
                }
                if (Quan* rq = s->Variable("rate")) {
                    if (rq->GetType() == Quan::_type::expression)
                        rate = srcFn(s->GetName(), "rate") + "(" + timeVar + ")";
                    else rate = fmt(std::atof(rq->GetProperty(true).c_str()));
                }
                const std::string tsFactor = s->Variable("timeseries")
                    ? srcHandle(s->GetName()) + ".interpol(" + timeVar + ")" : std::string("1.0");
                return "(" + coeff + " * " + tsFactor + " * " + rate + ")";
            }
            if (isPlainSeries(q->GetType()))
                return tsHandle(t->GetName(), name) + ".interpol(" + timeVar + ")";
            return sym(t->GetName(), name);
        };
        ctx.resolveSeries = [&, target](const std::string& name, Loc loc) -> std::string {
            Object* t = target(loc);
            return tsHandle(t->GetName(), name);
        };
        return ctx;
    };

    auto info = [&](Object* o, const std::string& q) -> const QuantityInfo* {
        auto it = tiers.find(o->GetName() + "::" + q);
        return it == tiers.end() ? nullptr : &it->second;
    };

    // ---- collect declarations ---------------------------------------------
    std::ostringstream decls, seriesDecls, seriesSetters, initBody, stepBody, resBody;
    std::ostringstream srcFns;          // source-context member functions (G8a)
    std::ostringstream flowLocalsBody;  // flow per-iteration quantities cached as members for transport (G8b)
    std::set<double> breakpointSet;   // forcing sample times (dt clamped to these)

    // state enum
    std::ostringstream stateEnumBody;
    for (unsigned i = 0; i < nB; ++i) {
        Block* b = system.block(i);
        stateEnumBody << "        " << stateEnum(b->GetName(), stateVar) << " = " << i << ",\n";
    }

    auto emitQuantity = [&](Object* o, bool isLink, const std::string& qname,
                            const std::string& timeVar, std::ostringstream& out,
                            bool asLocal) {
        Quan* q = o->Variable(qname);
        if (!q) return;
        const std::string s = sym(o->GetName(), qname);
        const std::string lhs = (asLocal ? "        const double " + s : "        " + s);
        if (q->GetType() == Quan::_type::expression) {
            ExpressionEmitter em(makeCtx(o, isLink, timeVar));
            out << lhs << " = " << em.translate(*q->GetExpression()) << ";\n";
        } else if (q->GetType() == Quan::_type::rule) {
            // first-matching-condition wins, else 0 (matches Rule::calc)
            ExpressionEmitter em(makeCtx(o, isLink, timeVar));
            Rule* r = q->GetRule();
            out << (asLocal ? "        double " : "        ") << s << " = 0.0;\n";
            int emitted = 0;
            for (int ri = 0; r && ri < r->Count(); ++ri) {
                _condplusresult* cr = (*r)[ri];
                const Condition& c = cr->condition;
                // A well-formed condition is a chained inequality (>= 2 operands).
                // Entries with fewer are rule metadata (e.g. a "unit" key), not a
                // branch -- skip them (they are unreachable in Rule::calc anyway).
                if (c.Count() < 2) continue;
                std::string cond;
                for (unsigned e = 0; e + 1 < c.Count(); ++e) {
                    if (!cond.empty()) cond += " && ";
                    const std::string op = c.IsLessThan(e) ? " < " : " > ";
                    cond += "(" + em.translate(c.Expr(e)) + op + em.translate(c.Expr(e + 1)) + ")";
                }
                out << "        " << (emitted ? "else if (" : "if (") << cond << ") " << s
                    << " = " << em.translate(cr->result) << ";\n";
                ++emitted;
            }
        } else if (q->GetType() == Quan::_type::value ||
                   q->GetType() == Quan::_type::constant ||
                   q->GetType() == Quan::_type::boolean) {
            out << lhs << " = " << fmt(std::atof(q->GetProperty(true).c_str())) << ";\n";
        }
    };

    // per-object walk, honoring QuantitOrder and tiers
    auto walk = [&](Object* o, bool isLink) {
        QuanSet* qs = o->GetVars();
        std::vector<std::string> order = o->QuantitOrder();
        // ensure every quantity is covered even if not in QuantitOrder
        for (auto it = qs->begin(); it != qs->end(); ++it) {
            const std::string& qn = it->second.GetName();
            bool found = false;
            for (auto& x : order) if (x == qn) { found = true; break; }
            if (!found) order.push_back(qn);
        }
        for (const std::string& qn : order) {
            Quan* q = o->Variable(qn);
            if (!q) continue;
            if (!constPrefix(qn).empty()) continue;   // constituent-scoped -> transport phase
            const QuantityInfo* qi = info(o, qn);
            if (!qi) continue;
            if (q->GetType() == Quan::_type::balance) continue;  // state, not emitted here
            if (q->GetType() == Quan::_type::source) continue;   // handled via source expansion
            if (isPlainSeries(q->GetType())) {
                // declare a series input + setter once, and bake the model's
                // loaded data points inline (so the generated lib is standalone).
                const std::string h = tsHandle(o->GetName(), qn);
                seriesDecls  << "    ohq::TimeSeries " << h << ";\n";
                seriesSetters << "    void set_" << h << "(const ohq::TimeSeries& ts) { "
                              << h << " = ts; }\n";
                TimeSeries<timeseriesprecision>* ts = q->GetTimeSeries();
                if (ts && ts->size() > 0 && ts->size() <= 50000) {
                    double tprev = -1e300;
                    for (size_t k = 0; k < ts->size(); ++k) {
                        double tv = ts->getTime(k), cv = ts->getValue(k);
                        if (!std::isfinite(tv) || !std::isfinite(cv)) continue;
                        if (tv <= tprev) continue;
                        tprev = tv;
                        initBody << "        " << h << ".push(" << fmt(tv) << ", " << fmt(cv) << ");\n";
                        breakpointSet.insert(tv);
                    }
                } else if (ts && ts->size() > 50000) {
                    initBody << "        // " << h << ": " << ts->size()
                             << " points not baked (large); use set_" << h << "().\n";
                }
                continue;
            }
            if (qi->tier == Tier::Constant) {
                decls << "    double " << sym(o->GetName(), qn) << " = 0.0;\n";
                emitQuantity(o, isLink, qn, "0.0", initBody, /*asLocal=*/false);
            } else if (qi->tier == Tier::PerStep) {
                decls << "    double " << sym(o->GetName(), qn) << " = 0.0;\n";
                emitQuantity(o, isLink, qn, "t_new", stepBody, /*asLocal=*/false);
            }
            // PerIteration handled in residual pass below
        }
    };

    // Bake each unique Source object's timeseries once (shared across all objects
    // that reference it, e.g. one rain gauge feeding 1024 catchment cells).
    {
        std::set<std::string> baked;
        for (unsigned i = 0; i < nB + nL; ++i) {
            Object* o = (i < nB) ? (Object*)system.block(i) : (Object*)system.link(i - nB);
            QuanSet* qs = o->GetVars();
            for (auto it = qs->begin(); it != qs->end(); ++it) {
                if (it->second.GetType() != Quan::_type::source) continue;
                Source* s = it->second.GetSource();
                if (!s) continue;
                const std::string nm = sanitize(s->GetName());
                if (baked.count(nm)) continue;
                baked.insert(nm);
                const std::string h = srcHandle(s->GetName());
                seriesDecls  << "    ohq::TimeSeries " << h << ";\n";
                seriesSetters << "    void set_" << h << "(const ohq::TimeSeries& ts) { " << h << " = ts; }\n";
                Quan* tq = s->Variable("timeseries");
                TimeSeries<timeseriesprecision>* ts = tq ? tq->GetTimeSeries() : nullptr;
                if (ts && ts->size() > 0 && ts->size() <= 200000) {
                    double tprev = -1e300;
                    for (size_t k = 0; k < ts->size(); ++k) {
                        double tv = ts->getTime(k), cv = ts->getValue(k);
                        if (!std::isfinite(tv) || !std::isfinite(cv)) continue;  // skip NaN header points
                        if (tv <= tprev) continue;                               // strictly increasing
                        tprev = tv;
                        initBody << "        " << h << ".push(" << fmt(tv) << ", " << fmt(cv) << ");\n";
                        breakpointSet.insert(tv);
                    }
                }

                // The source's own graph: every other time series becomes a
                // member (+ setter, baked data) and every expression quantity a
                // member function of t resolved in the SOURCE's context. Member
                // functions call each other, so no ordering pass is needed.
                auto makeCtxSrc = [&](Source* src) {
                    EmitContext ctx; ctx.timeVar = "t";
                    ctx.resolveValue = [&, src](const std::string& name, Loc) -> std::string {
                        if (name == "timeseries") return srcHandle(src->GetName()) + ".interpol(t)";
                        Quan* sq = src->Variable(name);
                        if (!sq) return "0.0";
                        if (isSeriesType(sq->GetType())) return srcTs(src->GetName(), name) + ".interpol(t)";
                        if (sq->GetType() == Quan::_type::expression) return srcFn(src->GetName(), name) + "(t)";
                        return fmt(std::atof(sq->GetProperty(true).c_str()));   // value / constant / boolean
                    };
                    ctx.resolveSeries = [&, src](const std::string& name, Loc) -> std::string {
                        return name == "timeseries" ? srcHandle(src->GetName()) : srcTs(src->GetName(), name);
                    };
                    return ctx;
                };
                QuanSet* sqs = s->GetVars();
                for (auto sit = sqs->begin(); sit != sqs->end(); ++sit) {
                    Quan& sq = sit->second;
                    const std::string& sqn = sq.GetName();
                    if (sqn == "timeseries") continue;
                    if (isSeriesType(sq.GetType())) {
                        const std::string hh = srcTs(s->GetName(), sqn);
                        seriesDecls  << "    ohq::TimeSeries " << hh << ";\n";
                        seriesSetters << "    void set_" << hh << "(const ohq::TimeSeries& ts) { " << hh << " = ts; }\n";
                        TimeSeries<timeseriesprecision>* sts = sq.GetTimeSeries();
                        if (sts && sts->size() > 0 && sts->size() <= 200000) {
                            double tprev = -1e300;
                            for (size_t k = 0; k < sts->size(); ++k) {
                                double tv = sts->getTime(k), cv = sts->getValue(k);
                                if (!std::isfinite(tv) || !std::isfinite(cv)) continue;
                                if (tv <= tprev) continue;
                                tprev = tv;
                                initBody << "        " << hh << ".push(" << fmt(tv) << ", " << fmt(cv) << ");\n";
                                breakpointSet.insert(tv);   // interpreter clamps dt to ALL series
                            }
                        }
                    } else if (sq.GetType() == Quan::_type::expression && sqn != "coefficient") {
                        ExpressionEmitter em(makeCtxSrc(s));
                        srcFns << "    double " << srcFn(s->GetName(), sqn) << "(double t) const { (void)t; return "
                               << em.translate(*sq.GetExpression()) << "; }\n";
                    }
                }
            }
        }
    }

    for (unsigned i = 0; i < nB; ++i) walk(system.block(i), false);
    for (unsigned i = 0; i < nL; ++i) walk(system.link(i), true);

    // ---- residual: per-iteration locals (topologically ordered) -----------
    // Map node key -> the object/quantity so we can sort by dependency and emit
    // each local only after the locals it uses (fixes use-before-declaration).
    struct Node { Object* o; bool isLink; std::string qn; };
    std::map<std::string, Node> perIter;
    for (unsigned i = 0; i < nB + nL; ++i) {
        Object* o = (i < nB) ? (Object*)system.block(i) : (Object*)system.link(i - nB);
        bool isLink = (i >= nB);
        QuanSet* qs = o->GetVars();
        for (auto it = qs->begin(); it != qs->end(); ++it) {
            Quan& q = it->second;
            if (q.GetType() == Quan::_type::balance || isSeriesType(q.GetType())) continue;
            if (!constPrefix(q.GetName()).empty()) continue;   // constituent-scoped -> transport phase
            const QuantityInfo* qi = info(o, q.GetName());
            if (qi && qi->tier == Tier::PerIteration)
                perIter[o->GetName() + "::" + q.GetName()] = {o, isLink, q.GetName()};
        }
    }
    // Emit block per-iteration locals before link ones, each group topologically
    // sorted. Links reference source/dest block quantities (head, area) — often
    // via _ups/_bkw, whose bare arguments look self-located to the analyzer, so
    // that cross-edge isn't in the graph; blocks never depend on link quantities,
    // so blocks-before-links guarantees definitions precede uses.
    {
        // With constituents, transport expressions may read flow-phase geometry
        // (link `area`/`depth`, block `head`) that only exists as locals inside
        // computeFluxes. Emit the same ordered computations a second time into
        // computeFlowLocals(), assigning to members (same symbols; the locals in
        // computeFluxes shadow them), called once per accepted flow step.
        const bool needFlowLocals = system.ConstituentsCount() > 0;
        auto emitGroupF = [&](bool wantLink) {
            std::map<std::string, int> indeg;
            std::map<std::string, std::vector<std::string>> radj;
            for (auto& kv : perIter) if (kv.second.isLink == wantLink) indeg[kv.first] = 0;
            for (auto& kv : perIter) {
                if (kv.second.isLink != wantLink) continue;
                auto ti = tiers.find(kv.first);
                if (ti == tiers.end()) continue;
                for (const std::string& dep : ti->second.dependencies) {
                    if (dep == kv.first) continue;
                    if (perIter.count(dep) && perIter[dep].isLink == wantLink)
                        { indeg[kv.first]++; radj[dep].push_back(kv.first); }
                }
            }
            std::vector<std::string> ready, ordered;
            for (auto& kv : indeg) if (kv.second == 0) ready.push_back(kv.first);
            while (!ready.empty()) {
                std::string n = ready.back(); ready.pop_back(); ordered.push_back(n);
                for (const std::string& m : radj[n]) if (--indeg[m] == 0) ready.push_back(m);
            }
            for (auto& kv : indeg) if (std::find(ordered.begin(), ordered.end(), kv.first) == ordered.end())
                ordered.push_back(kv.first);
            for (const std::string& key : ordered) {
                const Node& nd = perIter[key];
                emitQuantity(nd.o, nd.isLink, nd.qn, "t_new", resBody, /*asLocal=*/true);
                if (needFlowLocals) {
                    decls << "    double " << sym(nd.o->GetName(), nd.qn) << " = 0.0;\n";
                    emitQuantity(nd.o, nd.isLink, nd.qn, "t_new", flowLocalsBody, /*asLocal=*/false);
                }
            }
        };
        emitGroupF(false);   // blocks first
        emitGroupF(true);    // then links
    }

    // flowRaw[l] : raw link flow (before limiting factors)
    for (unsigned i = 0; i < nL; ++i) {
        Link* l = system.link(i);
        Quan* fq = l->Variable(flowVar);
        std::string flowRef = "0.0";
        if (fq) {
            const QuantityInfo* qi = info(l, flowVar);
            if (qi && qi->tier == Tier::PerIteration)
                flowRef = sym(l->GetName(), flowVar);       // per-iteration local above
            else {
                EmitContext ctx = makeCtx(l, true, "t_new");
                flowRef = ctx.resolveValue(flowVar, Loc::self);
            }
        }
        resBody << "        flowRaw[" << i << "] = " << flowRef << ";\n";
    }
    // inflowOwn[b] : sum of each block's own inflow quantities
    for (unsigned i = 0; i < nB; ++i) {
        Block* b = system.block(i);
        const std::string idx = stateEnum(b->GetName(), stateVar);
        Quan* bal = b->Variable(stateVar);
        resBody << "        inflowOwn[" << idx << "] = 0.0";
        if (bal) {
            EmitContext ctx = makeCtx(b, false, "t_new");
            for (const std::string& inflow : bal->GetCorrespondingInflowVar())
                if (!inflow.empty() && b->Variable(inflow))
                    resBody << " + " << ctx.resolveValue(inflow, Loc::self);
        }
        resBody << ";\n";
    }

    // ---- initial values ----------------------------------------------------
    std::ostringstream iniBody;
    // Evaluate initial values numerically via the interpreter and emit literals.
    // Initial-value expressions may reference quantities that are not compile-time
    // constants (e.g. a channel's initial Storage computed from an initial depth),
    // so symbolic emission is unsafe here; the interpreter already resolves them.
    for (unsigned i = 0; i < nB; ++i) system.block(i)->CalculateInitialValues();
    for (unsigned i = 0; i < nB; ++i) {
        Block* b = system.block(i);
        Quan* bal = b->Variable(stateVar);
        const std::string idx = stateEnum(b->GetName(), stateVar);
        double iv = bal ? std::atof(bal->GetProperty(true).c_str()) : 0.0;
        iniBody << "        s0[" << idx << "] = " << fmt(iv) << ";\n";
    }

    // ======================================================================
    //  TRANSPORT (constituents) — second phase, advection (reactions later)
    // ======================================================================
    const unsigned nC = system.ConstituentsCount();
    std::ostringstream massEnumBody, transBody, iniMassBody;
    std::map<std::string, int> constIdx;
    for (unsigned j = 0; j < nC; ++j) constIdx[system.constituent(j)->GetName()] = static_cast<int>(j);

    if (nC > 0) {
        // transport state enum: index = b*nC + j (matches GetResiduals_TR)
        for (unsigned b = 0; b < nB; ++b)
            for (unsigned j = 0; j < nC; ++j)
                massEnumBody << "        "
                    << massEnum(system.block(b)->GetName(), system.constituent(j)->GetName())
                    << " = " << (b * nC + j) << ",\n";

        // transport resolver: Storage->fixed flow storage, flow->fixed flow,
        // <const>:mass->transport state, else member/local.
        auto makeCtxT = [&](Object* cur, bool isLink, int curLinkIdx) {
            EmitContext ctx; ctx.timeVar = "t_new";
            auto target = [&, cur, isLink](Loc loc) -> Object* {
                if (!isLink || loc == Loc::self) return cur;
                if (loc == Loc::source)      return (Object*)system.block(cur->s_Block_No());
                if (loc == Loc::destination) return (Object*)system.block(cur->e_Block_No());
                return cur;
            };
            ctx.resolveValue = [&, target, isLink, curLinkIdx](const std::string& name, Loc loc) -> std::string {
                Object* t = target(loc);
                Quan* q = t->Variable(name);
                // A missing quantity evaluates to 0 in the interpreter (constituent
                // expressions are copied to objects that may lack some referenced
                // quantity, e.g. 'inflow' on a fixed_head, diffusion on a link).
                if (!q) return "0.0";
                if (name == stateVar)                         // flow-phase storage (fixed)
                    return "flowStorage_[" + stateEnum(t->GetName(), stateVar) + "]";
                if (q->GetType() == Quan::_type::balance)     // constituent mass (transport state)
                    return "mass[" + massEnum(t->GetName(), constPrefix(name)) + "]";
                if (isLink && loc == Loc::self && name == flowVar)  // flow-phase flow (fixed)
                    return "flowFlow_[" + std::to_string(curLinkIdx) + "]";
                if (isSeriesType(q->GetType()))
                    return tsHandle(t->GetName(), name) + ".interpol(t_new)";
                return sym(t->GetName(), name);
            };
            ctx.resolveSeries = [&, target](const std::string& name, Loc loc) -> std::string {
                return tsHandle(target(loc)->GetName(), name);
            };
            return ctx;
        };

        // constituent input quantities (values/series) become members, mirroring
        // the flow phase's handling of non-constituent inputs.
        for (unsigned i = 0; i < nB + nL; ++i) {
            Object* o = (i < nB) ? (Object*)system.block(i) : (Object*)system.link(i - nB);
            QuanSet* qs = o->GetVars();
            for (auto it = qs->begin(); it != qs->end(); ++it) {
                Quan& q = it->second;
                const std::string& qn = q.GetName();
                if (constPrefix(qn).empty()) continue;              // constituent-scoped only
                if (q.GetType() == Quan::_type::value || q.GetType() == Quan::_type::constant ||
                    q.GetType() == Quan::_type::boolean) {
                    decls << "    double " << sym(o->GetName(), qn) << " = 0.0;\n";
                    initBody << "        " << sym(o->GetName(), qn) << " = "
                             << fmt(std::atof(q.GetProperty(true).c_str())) << ";\n";
                } else if (isSeriesType(q.GetType())) {
                    const std::string hh = tsHandle(o->GetName(), qn);
                    seriesDecls  << "    ohq::TimeSeries " << hh << ";\n";
                    seriesSetters << "    void set_" << hh << "(const ohq::TimeSeries& ts) { " << hh << " = ts; }\n";
                    TimeSeries<timeseriesprecision>* ts = q.GetTimeSeries();
                    if (ts && ts->size() > 0 && ts->size() <= 50000)
                        for (size_t k = 0; k < ts->size(); ++k)
                            initBody << "        " << hh << ".push(" << fmt(ts->getTime(k))
                                     << ", " << fmt(ts->getValue(k)) << ");\n";
                }
            }
        }

        // collect constituent-scoped expression quantities as transport locals
        struct TNode { Object* o; bool isLink; int linkIdx; std::string qn; };
        std::map<std::string, TNode> tnodes;
        for (unsigned i = 0; i < nB + nL; ++i) {
            Object* o = (i < nB) ? (Object*)system.block(i) : (Object*)system.link(i - nB);
            bool isLink = (i >= nB); int li = isLink ? int(i - nB) : -1;
            QuanSet* qs = o->GetVars();
            for (auto it = qs->begin(); it != qs->end(); ++it) {
                Quan& q = it->second;
                if (q.GetType() != Quan::_type::expression) continue;
                if (constPrefix(q.GetName()).empty()) continue;   // constituent-scoped only
                tnodes[o->GetName() + "::" + q.GetName()] = {o, isLink, li, q.GetName()};
            }
        }
        // Emit block-scoped constituent locals first, then link-scoped ones:
        // links reference source/dest block concentrations via _ups (which look
        // self-located to the analyzer, so that cross-edge isn't in the graph),
        // but blocks never depend on link transport quantities. Within each group
        // a topological sort using the analyzer's (intra-object) edges suffices.
        auto emitGroup = [&](bool wantLink) {
            std::map<std::string, int> indeg; std::map<std::string, std::vector<std::string>> radj;
            for (auto& kv : tnodes) if (kv.second.isLink == wantLink) indeg[kv.first] = 0;
            for (auto& kv : tnodes) {
                if (kv.second.isLink != wantLink) continue;
                auto ti = tiers.find(kv.first);
                if (ti == tiers.end()) continue;
                for (const std::string& dep : ti->second.dependencies)
                    if (dep != kv.first && tnodes.count(dep) && tnodes[dep].isLink == wantLink)
                        { indeg[kv.first]++; radj[dep].push_back(kv.first); }
            }
            std::vector<std::string> ready, ordered;
            for (auto& kv : indeg) if (kv.second == 0) ready.push_back(kv.first);
            while (!ready.empty()) { std::string n = ready.back(); ready.pop_back(); ordered.push_back(n);
                for (auto& m : radj[n]) if (--indeg[m] == 0) ready.push_back(m); }
            for (auto& kv : indeg) if (std::find(ordered.begin(), ordered.end(), kv.first) == ordered.end()) ordered.push_back(kv.first);
            for (const std::string& key : ordered) {
                const TNode& nd = tnodes[key];
                ExpressionEmitter em(makeCtxT(nd.o, nd.isLink, nd.linkIdx));
                transBody << "        const double " << sym(nd.o->GetName(), nd.qn) << " = "
                          << em.translate(*nd.o->Variable(nd.qn)->GetExpression()) << ";\n";
            }
        };
        emitGroup(false);   // blocks (concentration, inflow_loading, ...)
        emitGroup(true);    // links (advective/diffusive/masstransfer)
        // massTransfer[l*nC+j] and inflowOwn[b*nC+j]
        for (unsigned l = 0; l < nL; ++l) {
            Link* lk = system.link(l);
            for (unsigned j = 0; j < nC; ++j) {
                const std::string cj = system.constituent(j)->GetName();
                Quan* mt = lk->Variable(cj + ":masstransfer");
                std::string ref = mt ? sym(lk->GetName(), cj + ":masstransfer") : "0.0";
                transBody << "        massTransfer[" << (l * nC + j) << "] = " << ref << ";\n";
            }
        }
        // Reaction resolver: bare constituent name -> Pos(<block>:concentration),
        // reaction parameter -> its (constant) value, Storage -> fixed flow storage
        // (mirrors Object::GetVal's fallbacks used when evaluating rate/stoich).
        const unsigned nRx = system.ReactionsCount();
        auto makeCtxRxn = [&](Object* blk) {
            EmitContext ctx; ctx.timeVar = "t_new";
            ctx.resolveValue = [&, blk](const std::string& name, Loc) -> std::string {
                if (system.constituent(name) && blk->Variable(name + ":concentration"))
                    return "ohq::pos(" + sym(blk->GetName(), name + ":concentration") + ")";
                if (system.reactionparameter(name))
                    return fmt(system.reactionparameter(name)->CalcVal("value", Expression::timing::present));
                if (name == stateVar)
                    return "flowStorage_[" + stateEnum(blk->GetName(), stateVar) + "]";
                Quan* q = blk->Variable(name);
                if (q) {
                    if (isSeriesType(q->GetType())) return tsHandle(blk->GetName(), name) + ".interpol(t_new)";
                    return sym(blk->GetName(), name);
                }
                return "0.0";
            };
            ctx.resolveSeries = [&, blk](const std::string& name, Loc) -> std::string {
                return tsHandle(blk->GetName(), name);
            };
            return ctx;
        };

        for (unsigned b = 0; b < nB; ++b) {
            Block* blk = system.block(b);
            // per-reaction rate locals for this block (rate * normalizing done below)
            std::vector<std::string> rrate(nRx);
            EmitContext rc = makeCtxRxn(blk);
            for (unsigned r = 0; r < nRx; ++r) {
                Reaction* rx = system.reaction(r);
                Expression* re = rx->RateExpression();
                std::string sr = sanitize(blk->GetName()) + "__rxn_" + sanitize(rx->GetName()) + "_";
                transBody << "        const double " << sr << " = "
                          << (re ? ExpressionEmitter(rc).translate(*re) : std::string("0.0")) << ";\n";
                rrate[r] = sr;
            }
            for (unsigned j = 0; j < nC; ++j) {
                const std::string cj = system.constituent(j)->GetName();
                transBody << "        inflowOwn[" << (b * nC + j) << "] = 0.0";
                Quan* balt = blk->Variable(cj + ":mass");
                if (balt) {
                    EmitContext ctx = makeCtxT(blk, false, -1);
                    for (const std::string& inf : balt->GetCorrespondingInflowVar()) {
                        if (inf.empty()) continue;
                        std::string nm = blk->Variable(inf) ? inf : (blk->Variable(cj + ":" + inf) ? cj + ":" + inf : "");
                        if (!nm.empty()) transBody << " + " << ctx.resolveValue(nm, Loc::self);
                    }
                }
                // reaction contributions: + rate_r * stoich_{r,j} * Storage (subtracted as V[j])
                for (unsigned r = 0; r < nRx; ++r) {
                    Expression* st = system.reaction(r)->Stoichiometric_Constant(cj);
                    if (!st) continue;
                    std::string ss = ExpressionEmitter(rc).translate(*st);
                    if (ss == "0.0" || ss == "0") continue;   // skip zero stoichiometry
                    transBody << " + " << rrate[r] << " * (" << ss << ") * flowStorage_["
                              << stateEnum(blk->GetName(), stateVar) << "]";
                }
                transBody << ";\n";
            }
        }
        // initial masses (numeric; CalculateInitialValues already run above)
        for (unsigned b = 0; b < nB; ++b)
            for (unsigned j = 0; j < nC; ++j) {
                const std::string cj = system.constituent(j)->GetName();
                Quan* balt = system.block(b)->Variable(cj + ":mass");
                double iv = balt ? std::atof(balt->GetProperty(true).c_str()) : 0.0;
                iniMassBody << "        m0[" << (b * nC + j) << "] = " << fmt(iv) << ";\n";
            }
    }

    // ---- topology arrays (link endpoints + rigid flags) -------------------
    std::ostringstream srcArr, dstArr, rigidArr;
    for (unsigned i = 0; i < nL; ++i) {
        Link* l = system.link(i);
        srcArr << (i ? ", " : "") << stateEnum(system.block(l->s_Block_No())->GetName(), stateVar);
        dstArr << (i ? ", " : "") << stateEnum(system.block(l->e_Block_No())->GetName(), stateVar);
    }
    for (unsigned i = 0; i < nB; ++i) {
        Quan* bal = system.block(i)->Variable(stateVar);
        rigidArr << (i ? ", " : "") << ((bal && bal->isrigid()) ? "true" : "false");
    }

    // ======================================================================
    //  write header
    // ======================================================================
    const std::string cls = opt.className;
    const bool T = (nC > 0);

    // transport fragments (empty when there are no constituents)
    std::string tInclude = T ? "#include \"ohq_transport.h\"\n" : "";
    std::string tEnum = T ? ("    enum MassState {\n" + massEnumBody.str() +
                             "        N_MASS = " + std::to_string(nB * nC) + "\n    };\n") : "";
    std::string tCtor = T ? ", transport_(*this)" : "";
    std::string tInit = T ? " transport_.initialize();" : "";
    std::string tStep = T
        ? ("    bool step() {\n"
           "        if (!solver_.step()) return false;\n"
           "        for (int b=0;b<N_STATES;++b) flowStorage_[b]=solver_.storage(b);\n"
           "        for (int l=0;l<N_LINKS;++l) flowFlow_[l]=solver_.linkFlow(l);\n"
           "        computeFlowLocals(flowStorage_, solver_.time());\n"
           "        return transport_.step(solver_.time(), solver_.dt());\n"
           "    }\n")
        : "    bool step()              { return solver_.step(); }\n";
    std::string tHooks = T ? (
        std::string("    // ---- transport hooks ----\n")
        + "    // flow-phase per-iteration quantities, recomputed once per accepted step\n"
        + "    // from the committed storages so transport expressions can read them\n"
        + "    void computeFlowLocals(const double* eff, double t_new) {\n"
        + "        (void)eff; (void)t_new;\n" + flowLocalsBody.str() + "    }\n"
        + "    int nMass() const { return N_MASS; }\n"
        + "    int nConst() const { return " + std::to_string(nC) + "; }\n"
        + "    int nLinksT() const { return N_LINKS; }\n"
        + "    int linkSrcT(int l) const { return linkSrc(l); }\n"
        + "    int linkDstT(int l) const { return linkDst(l); }\n"
        + "    double constMass(int i) const { return transport_.mass(i); }\n"
        + "    void initialMass(double* m0) {\n" + iniMassBody.str() + "    }\n"
        + "    void computeTransportFluxes(const double* mass, double t_new, double* massTransfer, double* inflowOwn) const {\n"
        + "        (void)mass; (void)t_new; (void)massTransfer; (void)inflowOwn;\n"
        + transBody.str() + "    }\n") : "";
    std::string tMembers = T ? (
        std::string("    double flowStorage_[N_STATES] = {0};\n")
        + "    double flowFlow_[N_LINKS>0?N_LINKS:1] = {0};\n"
        + "    ohq::TransportSolver<" + cls + "> transport_;\n") : "";

    // Emit the interpreter's solver settings so the generated solver adapts dt,
    // tolerances and iteration limits identically (not codegen's own defaults).
    const solversettings& ss = system.GetSolverSettings();
    const double dt0v = system.dt0() != 0 ? system.dt0() : 1e-3;
    auto settingsFor = [&](const std::string& obj) {
        std::ostringstream b;
        b << "        { auto& S = " << obj << ".settings();\n"
          << "          S.tolerance = " << fmt(ss.NRtolerance)
          << "; S.iter_lower = " << ss.NR_niteration_lower
          << "; S.iter_upper = " << ss.NR_niteration_upper
          << "; S.max_iterations = " << ss.NR_niteration_max << ";\n"
          << "          S.dt_reduce = " << fmt(ss.NR_timestep_reduction_factor)
          << "; S.dt_grow = " << fmt(1.0 / ss.NR_timestep_reduction_factor)
          << "; S.dt_reduce_fail = " << fmt(ss.NR_timestep_reduction_factor_fail) << ";\n"
          << "          S.dt_max_factor = " << fmt(ss.timestepmaxfactor)
          << "; S.dt_min_factor = " << fmt(ss.minimum_timestep / dt0v) << "; }\n";
        return b.str();
    };
    std::string settingsInit = settingsFor("solver_")
        + (T ? settingsFor("transport_") : std::string())
        + "        solver_.landtozero = " + fmt(ss.landtozero_factor) + ";\n";

    std::ostringstream h;
    h << "// Auto-generated by OpenHydroQual model compiler. Do not edit.\n"
      << "#ifndef OHQ_GEN_" << sanitize(cls) << "_H\n#define OHQ_GEN_" << sanitize(cls) << "_H\n\n"
      << "#include \"ohq_intrinsics.h\"\n#include \"ohq_timeseries.h\"\n#include \"ohq_massbalance.h\"\n"
      << tInclude << "\n"
      << "class " << cls << " {\npublic:\n"
      << "    enum State {\n" << stateEnumBody.str() << "        N_STATES = " << nB << "\n    };\n"
      << "    enum { N_LINKS = " << nL << " };\n"
      << tEnum << "\n"
      << "    " << cls << "() : solver_(*this)" << tCtor << " {}\n\n"
      << seriesSetters.str() << srcFns.str() << "\n"
      << "    void initialize(double tstart = " << fmt(system.tstart()) << ", double dt0 = "
      << fmt(system.dt0()) << ") {\n        buildConstants();\n" << settingsInit
      << "        solver_.initialize(tstart, dt0);\n"
      << [&]{ std::ostringstream b; if(!breakpointSet.empty()){ b << "        { static const double BP[] = {"; bool first=true; for(double t:breakpointSet){ b<<(first?"":", ")<<fmt(t); first=false;} b << "}; solver_.setBreakpoints(BP, " << breakpointSet.size() << "); }\n"; } return b.str(); }()
      << (T ? "        transport_.initialize();\n" : "")
      << "    }\n"
      << tStep
      << (T ? "    bool runTo(double t_end) { solver_.setStop(t_end); while (time() < t_end - 1e-30) { if (!step()) return false; } return true; }\n"
            : "    bool runTo(double t_end) { return solver_.runTo(t_end); }\n")
      << "    double time() const      { return solver_.time(); }\n"
      << "    int lastIterations() const { return solver_.lastIterations(); }\n"
      << "    double state(int i) const { return solver_.storage(i); }\n"
      << "    bool limited(int i) const { return solver_.isLimited(i); }\n"
      << "    // one accepted step whose dt is clamped so it never crosses t_end\n"
      << "    bool stepTo(double t_end) { solver_.setStop(t_end); return step(); }\n"
      << "    // original model object names / time window (for outputs and embedding)\n"
      << "    static const char* stateName(int i) { static const char* a[] = {"
      << [&]{ auto lit=[](const std::string& s){ std::string o="\""; for(char c: s){ if(c=='\\'||c=='"') o+='\\'; o+=c; } return o+"\""; };
              if ((int)nB == 0) return std::string("\"\"");
              std::ostringstream b; for (int i=0;i<(int)nB;++i) b << (i?", ":"") << lit(system.block((unsigned)i)->GetName()); return b.str(); }()
      << "}; return a[i]; }\n"
      << (T ? "    static const char* constituentName(int j) { static const char* a[] = {"
              + [&]{ auto lit=[](const std::string& s){ std::string o="\""; for(char c: s){ if(c=='\\'||c=='"') o+='\\'; o+=c; } return o+"\""; };
                     std::ostringstream b; for (unsigned j=0;j<nC;++j) b << (j?", ":"") << lit(system.constituent(j)->GetName()); return b.str(); }()
              + "}; return a[j]; }\n"
            : std::string())
      << "    static double simulationStart() { return " << fmt(system.tstart()) << "; }\n"
      << "    static double simulationEnd()   { return " << fmt(system.tend()) << "; }\n\n"
      << "    // ---- mass-balance hooks (inlined; no virtual dispatch) ----\n"
      << "    int nBlocks() const { return N_STATES; }\n"
      << "    int nLinks()  const { return N_LINKS; }\n"
      << "    int linkSrc(int l) const { " << (nL ? "static const int a[] = {" + srcArr.str() + "}; return a[l];" : "return 0;") << " }\n"
      << "    int linkDst(int l) const { " << (nL ? "static const int a[] = {" + dstArr.str() + "}; return a[l];" : "return 0;") << " }\n"
      << "    bool rigid(int b) const { static const bool r[] = {" << rigidArr.str() << "}; return r[b]; }\n"
      << "    void initialStorage(double* s0) {\n" << iniBody.str() << "    }\n"
      << "    void precomputeStep(double t_new) {\n" << stepBody.str() << "    }\n"
      << "    void computeFluxes(const double* eff, double t_new, double* flowRaw, double* inflowOwn) const {\n"
      << "        (void)eff; (void)t_new; (void)flowRaw; (void)inflowOwn;\n"
      << resBody.str() << "    }\n\n"
      << tHooks << "\n"
      << "private:\n"
      << "    void buildConstants() {\n" << initBody.str() << "    }\n"
      << decls.str()
      << seriesDecls.str()
      << "    ohq::MassBalanceSolver<" << cls << "> solver_;\n"
      << tMembers
      << "};\n\n#endif\n";

    // write files
    std::filesystem::create_directories(opt.outputDir);
    const std::string base = opt.outputDir + "/" + cls;
    auto writeText = [](const std::string& path, const std::string& text) {
        std::ofstream f(path);
        if (!f) throw std::runtime_error("CodeGenerator: cannot write " + path);
        f << text;
    };
    writeText(base + ".h", h.str());
    if (!opt.emitProject) return true;

    // ======================================================================
    //  standalone project: runtime/ + CMakeLists.txt + (main.cpp | C API)
    //  Self-contained: the runtime headers are the embedded copies, so the
    //  folder builds on a machine with no OpenHydroQual checkout at all.
    // ======================================================================
    {
        std::filesystem::create_directories(opt.outputDir + "/runtime");
        int nrt = 0;
        const RuntimeFile* rt = runtimeFiles(nrt);
        for (int i = 0; i < nrt; ++i)
            writeText(opt.outputDir + "/runtime/" + rt[i].name, rt[i].text);

        std::string CLS = sanitize(cls);            // macro / option prefix
        for (char& c : CLS) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        const std::string banner =
            "// Auto-generated by OpenHydroQual model compiler (Export to C++). Do not edit.\n";

        if (!opt.asLibrary) {
            // ---- executable driver --------------------------------------
            std::ostringstream m;
            m << banner
              << "// Standalone forward model: runs " << cls << " from the model's start time to\n"
                 "// its end time (or a given t_end) and writes every accepted step to a CSV.\n"
                 "//\n//   usage: " << cls << "_solver [output.csv] [t_end]\n"
                 "#include <cstdio>\n#include <cstdlib>\n#include <chrono>\n"
                 "#include \"" << cls << ".h\"\n\n"
                 "int main(int argc, char** argv)\n{\n"
                 "    const char*  outPath = argc > 1 ? argv[1] : \"output.csv\";\n"
                 "    const double tEnd    = argc > 2 ? std::atof(argv[2]) : " << cls << "::simulationEnd();\n\n"
                 "    " << cls << " m;\n    m.initialize();\n\n"
                 "    std::FILE* f = std::fopen(outPath, \"w\");\n"
                 "    if (!f) { std::fprintf(stderr, \"cannot open %s\\n\", outPath); return 1; }\n"
                 "    std::fprintf(f, \"time\");\n"
                 "    for (int i = 0; i < m.nBlocks(); ++i) std::fprintf(f, \",%s:" << stateVar << "\", m.stateName(i));\n"
              << (T ? "    for (int b = 0; b < m.nBlocks(); ++b)\n"
                      "        for (int j = 0; j < m.nConst(); ++j) std::fprintf(f, \",%s:%s:mass\", m.stateName(b), m.constituentName(j));\n" : "")
              << "    std::fprintf(f, \"\\n\");\n"
                 "    auto record = [&]() {\n"
                 "        std::fprintf(f, \"%.10g\", m.time());\n"
                 "        for (int i = 0; i < m.nBlocks(); ++i) std::fprintf(f, \",%.10g\", m.state(i));\n"
              << (T ? "        for (int i = 0; i < m.nMass(); ++i) std::fprintf(f, \",%.10g\", m.constMass(i));\n" : "")
              << "        std::fprintf(f, \"\\n\");\n    };\n\n"
                 "    const auto t0 = std::chrono::steady_clock::now();\n"
                 "    record();\n    bool ok = true;\n"
                 "    while (m.time() < tEnd - 1e-30) {\n"
                 "        if (!m.stepTo(tEnd)) { ok = false; break; }\n"
                 "        record();\n    }\n"
                 "    const double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();\n"
                 "    std::fclose(f);\n"
                 "    std::printf(\"" << cls << ": t=%.6g  %s  wall=%.3f s  -> %s\\n\", m.time(), ok ? \"OK\" : \"SOLVE FAILED\", sec, outPath);\n"
                 "    return ok ? 0 : 2;\n}\n";
            writeText(opt.outputDir + "/main.cpp", m.str());
        } else {
            // ---- C ABI wrapper (works from C, Python ctypes, Fortran, ...) --
            std::ostringstream ah, ac, ex;
            ah << banner
               << "// C API for the " << cls << " solver. Link the library and include this header.\n"
                  "#ifndef " << CLS << "_API_H\n#define " << CLS << "_API_H\n\n"
                  "#if defined(_WIN32) && defined(" << CLS << "_SHARED)\n"
                  "#  ifdef " << CLS << "_EXPORTS\n#    define " << CLS << "_API __declspec(dllexport)\n"
                  "#  else\n#    define " << CLS << "_API __declspec(dllimport)\n#  endif\n"
                  "#else\n#  define " << CLS << "_API\n#endif\n\n"
                  "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
                  "typedef struct " << cls << "_handle " << cls << "_handle;\n\n"
                  "/* lifecycle */\n"
               << CLS << "_API " << cls << "_handle* " << cls << "_create(void);\n"
               << CLS << "_API void   " << cls << "_destroy(" << cls << "_handle* h);\n"
                  "/* initialize at the model's own start time / initial dt, or at a given one */\n"
               << CLS << "_API void   " << cls << "_initialize(" << cls << "_handle* h);\n"
               << CLS << "_API void   " << cls << "_initialize_at(" << cls << "_handle* h, double tstart, double dt0);\n"
                  "/* time stepping: 1 on success, 0 if the solver failed */\n"
               << CLS << "_API int    " << cls << "_step(" << cls << "_handle* h);\n"
               << CLS << "_API int    " << cls << "_step_to(" << cls << "_handle* h, double t_end);  /* one step, dt clamped to t_end */\n"
               << CLS << "_API int    " << cls << "_run_to(" << cls << "_handle* h, double t_end);\n"
                  "/* state access */\n"
               << CLS << "_API double " << cls << "_time(const " << cls << "_handle* h);\n"
               << CLS << "_API int    " << cls << "_n_states(void);\n"
               << CLS << "_API double " << cls << "_state(const " << cls << "_handle* h, int i);\n"
               << CLS << "_API const char* " << cls << "_state_name(int i);\n"
               << CLS << "_API double " << cls << "_simulation_start(void);\n"
               << CLS << "_API double " << cls << "_simulation_end(void);\n"
               << (T ? CLS + "_API int    " + cls + "_n_mass(void);\n"
                     + CLS + "_API double " + cls + "_mass(const " + cls + "_handle* h, int i);  /* constituent mass, index block*nConst+j */\n"
                     : std::string())
               << "\n#ifdef __cplusplus\n}\n#endif\n#endif\n";
            ac << banner
               << "#include \"" << cls << "_api.h\"\n#include \"" << cls << ".h\"\n\n"
                  "struct " << cls << "_handle { " << cls << " m; };\n\n"
                  "extern \"C\" {\n"
               << cls << "_handle* " << cls << "_create(void) { return new " << cls << "_handle(); }\n"
                  "void   " << cls << "_destroy(" << cls << "_handle* h) { delete h; }\n"
                  "void   " << cls << "_initialize(" << cls << "_handle* h) { h->m.initialize(); }\n"
                  "void   " << cls << "_initialize_at(" << cls << "_handle* h, double tstart, double dt0) { h->m.initialize(tstart, dt0); }\n"
                  "int    " << cls << "_step(" << cls << "_handle* h) { return h->m.step() ? 1 : 0; }\n"
                  "int    " << cls << "_step_to(" << cls << "_handle* h, double t) { return h->m.stepTo(t) ? 1 : 0; }\n"
                  "int    " << cls << "_run_to(" << cls << "_handle* h, double t) { return h->m.runTo(t) ? 1 : 0; }\n"
                  "double " << cls << "_time(const " << cls << "_handle* h) { return h->m.time(); }\n"
                  "int    " << cls << "_n_states(void) { return " << cls << "::N_STATES; }\n"
                  "double " << cls << "_state(const " << cls << "_handle* h, int i) { return h->m.state(i); }\n"
                  "const char* " << cls << "_state_name(int i) { return " << cls << "::stateName(i); }\n"
                  "double " << cls << "_simulation_start(void) { return " << cls << "::simulationStart(); }\n"
                  "double " << cls << "_simulation_end(void) { return " << cls << "::simulationEnd(); }\n"
               << (T ? "int    " + cls + "_n_mass(void) { return " + cls + "::N_MASS; }\n"
                       "double " + cls + "_mass(const " + cls + "_handle* h, int i) { return h->m.constMass(i); }\n"
                     : std::string())
               << "}\n";
            ex << "// Example client of the " << cls << " C API (also a link check for the library).\n"
                  "#include <cstdio>\n#include \"" << cls << "_api.h\"\n\n"
                  "int main()\n{\n"
                  "    " << cls << "_handle* h = " << cls << "_create();\n"
                  "    " << cls << "_initialize(h);\n"
                  "    const int ok = " << cls << "_run_to(h, " << cls << "_simulation_end());\n"
                  "    std::printf(\"t=%g %s\\n\", " << cls << "_time(h), ok ? \"OK\" : \"FAILED\");\n"
                  "    for (int i = 0; i < " << cls << "_n_states(); ++i)\n"
                  "        std::printf(\"  %s = %.10g\\n\", " << cls << "_state_name(i), " << cls << "_state(h, i));\n"
                  "    " << cls << "_destroy(h);\n    return ok ? 0 : 2;\n}\n";
            writeText(opt.outputDir + "/" + cls + "_api.h", ah.str());
            writeText(opt.outputDir + "/" + cls + "_api.cpp", ac.str());
            writeText(opt.outputDir + "/example.cpp", ex.str());
        }

        // ---- CMakeLists.txt: Linux/macOS make, Windows Visual Studio ----------
        std::ostringstream cm;
        cm << "# Auto-generated by OpenHydroQual model compiler (Export to C++).\n#\n"
              "# Linux / macOS:\n#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release\n#   cmake --build build\n"
              "# Windows (Visual Studio 2022):\n#   cmake -S . -B build -G \"Visual Studio 17 2022\" -A x64\n"
              "#   cmake --build build --config Release\n"
              "cmake_minimum_required(VERSION 3.16)\nproject(" << cls << " LANGUAGES CXX)\n\n"
              "set(CMAKE_CXX_STANDARD 17)\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
              "if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)\n"
              "  set(CMAKE_BUILD_TYPE Release CACHE STRING \"\" FORCE)\nendif()\n\n"
              "# The generated model header includes the header-only runtime by bare name.\n"
              "set(OHQ_INC \"${CMAKE_CURRENT_SOURCE_DIR}\" \"${CMAKE_CURRENT_SOURCE_DIR}/runtime\")\n\n";
        if (!opt.asLibrary) {
            cm << "add_executable(" << cls << "_solver main.cpp)\n"
                  "target_include_directories(" << cls << "_solver PRIVATE ${OHQ_INC})\n";
        } else {
            cm << "option(" << CLS << "_BUILD_SHARED \"Build " << cls << " as a shared library (.so/.dll)\" "
               << (opt.sharedLibrary ? "ON" : "OFF") << ")\n"
                  "if(" << CLS << "_BUILD_SHARED)\n"
                  "  add_library(" << cls << " SHARED " << cls << "_api.cpp)\n"
                  "  target_compile_definitions(" << cls << " PRIVATE " << CLS << "_EXPORTS PUBLIC " << CLS << "_SHARED)\n"
                  "else()\n  add_library(" << cls << " STATIC " << cls << "_api.cpp)\nendif()\n"
                  "target_include_directories(" << cls << " PUBLIC ${OHQ_INC})\n"
                  "set_target_properties(" << cls << " PROPERTIES POSITION_INDEPENDENT_CODE ON)\n\n"
                  "# tiny client: verifies the library links and runs\n"
                  "add_executable(" << cls << "_example example.cpp)\n"
                  "target_link_libraries(" << cls << "_example PRIVATE " << cls << ")\n";
        }
        writeText(opt.outputDir + "/CMakeLists.txt", cm.str());

        // ---- README ------------------------------------------------------------
        std::ostringstream rd;
        rd << "# " << cls << " -- standalone C++ solver generated by OpenHydroQual\n\n"
              "Every model equation is hard-coded (no expression interpreter). The folder is\n"
              "self-contained: no Qt, no Armadillo, no OpenHydroQual install needed.\n\n"
              "## Files\n"
              "- `" << cls << ".h` -- the generated model class (" << nB << " state variables, " << nL
           << " links" << (T ? ", " + std::to_string(nC) + " constituent(s)" : std::string()) << ")\n"
              "- `runtime/` -- header-only solver runtime (mass balance, sparse/dense Newton, time series)\n"
           << (opt.asLibrary
                ? "- `" + cls + "_api.h/.cpp` -- C ABI wrapper (callable from C, Python ctypes, Fortran, ...)\n- `example.cpp` -- minimal client\n"
                : std::string("- `main.cpp` -- runs the forward model, writes a CSV of every accepted step\n"))
           << "- `CMakeLists.txt`\n\n"
              "## Build\n```\n# Linux / macOS\ncmake -S . -B build -DCMAKE_BUILD_TYPE=Release\ncmake --build build\n\n"
              "# Windows (Visual Studio 2022)\ncmake -S . -B build -G \"Visual Studio 17 2022\" -A x64\n"
              "cmake --build build --config Release\n```\n\n"
           << (opt.asLibrary
                ? "## Use\nLink `" + cls + "` and include `" + cls + "_api.h`:\n```c\n"
                  + cls + "_handle* h = " + cls + "_create();\n" + cls + "_initialize(h);\n"
                  + cls + "_run_to(h, " + cls + "_simulation_end());\n"
                  "double v = " + cls + "_state(h, 0);   /* see " + cls + "_state_name(0) */\n"
                  + cls + "_destroy(h);\n```\nOr use the C++ class directly: `#include \"" + cls
                  + ".h\"` -> `initialize()`, `step()`, `stepTo(t)`, `runTo(t)`, `time()`, `state(i)`.\n"
                : "## Run\n```\n./build/" + cls + "_solver [output.csv] [t_end]\n```\n"
                  "Default `t_end` is the model's simulation end (" + fmt(system.tend())
                  + "). The CSV has one column per state variable (`block:" + stateVar + "`)"
                  + (T ? std::string(" and per `block:constituent:mass`") : std::string()) + ".\n")
           << "\n## Model time window\n`tstart = " << fmt(system.tstart()) << "`, `tend = "
           << fmt(system.tend()) << "`, `dt0 = " << fmt(system.dt0()) << "`\n";
        writeText(opt.outputDir + "/README.md", rd.str());
    }
    return true;
}

} // namespace ohqcg
