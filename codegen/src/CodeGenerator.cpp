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
#include <cstdio>
#include <cstdlib>
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
#include "Parameter.h"
#include "observation.h"

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
// A source's value/constant quantities are members (not folded literals) so a
// parameter bound to one (e.g. Evap_Coefficient -> solar_scale_fact) can change.
static std::string srcSym(const std::string& sourceName, const std::string& q)
{
    return "src_" + sanitize(sourceName) + "__" + sanitize(q) + "_";
}
// C string literal for a model name.
static std::string cstr(const std::string& s)
{
    std::string o = "\"";
    for (char c : s) { if (c == '\\' || c == '"') o += '\\'; o += c; }
    return o + "\"";
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

    // ---- G1: parameters -> runtime inputs -----------------------------------
    // Every quantity bound by `setasparameter` (Parameter::GetLocations/GetQuans)
    // is assigned from params_[i] inside buildConstants() instead of a literal,
    // so setParameter(i, v) + applyParameters() recomputes every derived constant
    // exactly like System::ApplyParameters followed by a solve. Index i is the
    // model's parameter order -- what CMCMC/CGA pass to SetParameterValue(i, .).
    struct ParamInfo { std::string name; double value; };
    std::vector<ParamInfo> params;
    std::map<std::string, int> paramBound;   // "<object>::<quan>" -> parameter index
    std::vector<std::string> paramNotes;     // bindings the kernel cannot own (observation sigma, ...)
    for (unsigned i = 0; i < system.ParametersCount(); ++i) {
        Parameter* p = system.GetParameter(i);
        params.push_back({p->GetName(), p->GetValue()});
        const std::vector<std::string> locs = p->GetLocations(), qs = p->GetQuans();
        bool anyLive = false, hostOwnedOnly = true;   // see the ISSUE 17 guard below
        for (size_t k = 0; k < locs.size() && k < qs.size(); ++k) {
            Object* o = system.object(locs[k]);
            if (!o) { paramNotes.push_back(p->GetName() + " -> '" + locs[k] + "': object not found"); continue; }
            const object_type ot = o->ObjectType();
            // A bound parameter MUST become a live params_[i] in the generated
            // code, whatever kind of object carries it. The old code allow-listed
            // block/link/source and let everything else fall through to a comment
            // saying "the host applies it" -- so a ReactionParameter driving every
            // sorption rate was baked as a literal, and a GA driving the kernel
            // optimised a model whose rate constants never moved. Resolve
            // generically instead, and fail loudly on anything left unbound.
            // Host-owned first: an observation's sigma and an objective-function
            // weight are likelihood parameters, not part of the forward model.
            // They DO exist as quantities on their objects, so resolving them
            // generically would "bind" something the kernel never emits.
            if (ot == object_type::observation || ot == object_type::objective_function) {
                paramNotes.push_back(p->GetName() + " -> " + locs[k] + "." + qs[k]
                                     + ": likelihood parameter, applied by the host (not part of the forward model)");
                continue;   // host-owned: leaves hostOwnedOnly intact
            }
            hostOwnedOnly = false;   // a forward-model target: it must end up live
            bool bound = false;
            if (Quan* q = o->Variable(qs[k])) {
                if (q->GetType() == Quan::_type::value || q->GetType() == Quan::_type::constant) {
                    paramBound[o->GetName() + "::" + qs[k]] = static_cast<int>(i);
                    bound = true;
                }
            }
            if (ot == object_type::constituent) {
                // A constituent property (dispersivity, diffusion_coefficient, ...)
                // is ALSO reachable as "<constituent>:<quan>" from every block and
                // link (Object::GetVal, Object.cpp:127/138), so bind those copies
                // too or applyParameters() leaves them at the baked value.
                const std::string cq = o->GetName() + ":" + qs[k];
                for (unsigned b = 0; b < system.BlockCount(); ++b)
                    if (system.block(b)->Variable(cq)) { paramBound[system.block(b)->GetName() + "::" + cq] = static_cast<int>(i); bound = true; }
                for (unsigned l = 0; l < system.LinksCount(); ++l)
                    if (system.link(l)->Variable(cq)) { paramBound[system.link(l)->GetName() + "::" + cq] = static_cast<int>(i); bound = true; }
                if (o->Variable(qs[k])) bound = true;   // reached via the constituent object itself
            }
            if (bound) anyLive = true;
            if (!bound) {
                // Two legitimate cases where the kernel does NOT bind, and must not:
                //  1. a likelihood parameter (an observation's sigma) -- host-owned;
                //  2. a target the INTERPRETER cannot meaningfully set either.
                //     ApplyParameters does SetVal(quan, v), which writes a Quan's
                //     `past` value; an expression/balance quantity is recomputed
                //     from its expression on every `present` read (Quan::GetVal),
                //     so the write has no lasting effect there. Ignoring such a
                //     binding therefore PRESERVES parity rather than breaking it.
                //     (Real case: `porosity_all -> Col1-1.moisture_content`, where
                //     moisture_content is the expression Storage/(depth*area).)
                const bool host_owned = (ot == object_type::observation
                                      || ot == object_type::objective_function);
                Quan* tq = o->Variable(qs[k]);
                const bool inert = tq && (tq->GetType() == Quan::_type::expression
                                       || tq->GetType() == Quan::_type::balance
                                       || tq->GetType() == Quan::_type::rule);
                if (host_owned)
                    paramNotes.push_back(p->GetName() + " -> " + locs[k] + "." + qs[k]
                                         + ": likelihood parameter, applied by the host (not part of the forward model)");
                else if (inert)
                    paramNotes.push_back(p->GetName() + " -> " + locs[k] + "." + qs[k]
                                         + ": target is a derived (expression/balance/rule) quantity; setting it is"
                                           " inert in the interpreter too, so it is deliberately not bound here");
                else
                    throw std::runtime_error(
                        "CodeGenerator: estimated parameter '" + p->GetName() + "' is bound to "
                        + locs[k] + "." + qs[k] + ", which the generator cannot make live in the "
                        "emitted code. Refusing to emit a kernel that would silently ignore it.");
            }
        }
        // ISSUE 17's guarantee: a parameter a calibration cannot drive at all is
        // still fatal. Inert/host-owned bindings are fine ONLY alongside a live one.
        if (!anyLive && !hostOwnedOnly)
            throw std::runtime_error(
                "CodeGenerator: estimated parameter '" + p->GetName() + "' has no binding the "
                "kernel can drive (all of its targets are derived quantities). Refusing to emit "
                "a kernel a calibration cannot drive.");
    }
    const unsigned nP = static_cast<unsigned>(params.size());
    auto paramRef = [&](const std::string& obj, const std::string& q) -> std::string {   // "" if unbound
        auto it = paramBound.find(obj + "::" + q);
        return it == paramBound.end() ? std::string() : "params_[" + std::to_string(it->second) + "]";
    };
    // A ReactionParameter's effective value is CalcVal("value"), which is
    // base_value corrected for temperature (Arrhenius). When base_value is the
    // estimated parameter we must emit params_[i] so applyParameters() moves it;
    // we only do that when the Arrhenius correction is the identity, otherwise
    // the correction would be silently dropped and the literal is kept.
    auto reactionParamRef = [&](const std::string& name) -> std::string {
        Object* rp = (Object*)system.reactionparameter(name);
        const double v = rp->CalcVal("value", Expression::timing::present);
        const std::string pb = paramRef(name, "base_value");
        if (!pb.empty()) {
            const double b = rp->GetVal("base_value", Expression::timing::present);
            if (b != 0 && std::fabs(v - b) <= 1e-12 * std::fabs(b)) return pb;
            paramNotes.push_back(name + ".base_value is estimated but value != base_value "
                                 "(temperature correction); left as a literal");
        }
        return fmt(v);
    };

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
                    } else coeff = srcSym(s->GetName(), "coefficient");
                }
                if (Quan* rq = s->Variable("rate")) {
                    if (rq->GetType() == Quan::_type::expression)
                        rate = srcFn(s->GetName(), "rate") + "(" + timeVar + ")";
                    else rate = srcSym(s->GetName(), "rate");
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
    std::ostringstream seriesInit;      // baked series tables -> loadSeries(), run once (buildConstants may re-run)
    std::ostringstream flowLocalsBody;  // flow per-iteration quantities cached as members for transport (G8b)
    std::set<double> breakpointSet;   // (unused: dt is clamped via interpol_D, see clampHandles)
    // Series that clamp dt (runtime addClampSeries -> interpol_D = time until the
    // series next changes value). The interpreter registers only PRECIPITATION
    // series (GetTimeSeries(true)); its hourly ET inputs are resolved anyway only
    // because its Newton rarely converges below NR_niteration_lower, so dt_base
    // never grows. A fast solver must clamp to EVERY forcing series or it steps
    // over the diurnal ET cycle at 0.5 d (issues.md ISSUE 8).
    std::vector<std::string> clampHandles;
    // G4: every baked series by (object, quantity) so the host can replace it at
    // runtime by name (DTRunner::injectPrecipitation / DTWeather::injectWeather).
    struct SeriesEntry { std::string obj, q, handle; };
    std::vector<SeriesEntry> seriesTable;

    // Bake a loaded series as a static data table + fill loop. (One push() per
    // point in straight-line code sent GCC's optimizer superlinear: >10 min for
    // 88k hourly points; a brace-initialized table compiles in seconds.)
    auto bakeSeries = [&](const std::string& obj, const std::string& q, const std::string& handle,
                          TimeSeries<timeseriesprecision>* ts, size_t maxPoints, bool clampDt) {
        seriesTable.push_back({obj, q, handle});          // addressable even when empty now
        if (clampDt) clampHandles.push_back(handle);      // (an injected series must clamp too)
        if (!ts || ts->size() == 0) return;
        if (ts->size() > maxPoints) {
            seriesInit << "        // " << handle << ": " << ts->size()
                       << " points not baked (large); use set_" << handle << "().\n";
            return;
        }
        std::ostringstream tbl; size_t n = 0; double tprev = -1e300;
        for (size_t k = 0; k < ts->size(); ++k) {
            double tv = ts->getTime(k), cv = ts->getValue(k);
            if (!std::isfinite(tv) || !std::isfinite(cv)) continue;   // skip NaN header points
            if (tv <= tprev) continue;                                // strictly increasing
            tprev = tv;
            tbl << (n ? "," : "") << (n % 4 == 0 ? "\n            " : " ") << fmt(tv) << ", " << fmt(cv);
            ++n;
        }
        if (n == 0) return;
        seriesInit << "        { static const double D[] = {" << tbl.str() << " };\n"
                   << "          for (size_t k = 0; k < " << n << "; ++k) " << handle
                   << ".push(D[2*k], D[2*k+1]); }\n";
    };

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
            const std::string pr = paramRef(o->GetName(), qname);   // G1: bound -> params_[i]
            out << lhs << " = " << (pr.empty() ? fmt(std::atof(q->GetProperty(true).c_str())) : pr) << ";\n";
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
                bakeSeries(o->GetName(), qn, h, q->GetTimeSeries(), 200000, /*clampDt=*/true);
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
                bakeSeries(s->GetName(), "timeseries", h, tq ? tq->GetTimeSeries() : nullptr, 200000, /*clampDt=*/true);

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
                        return srcSym(src->GetName(), name);   // value / constant / boolean: a member (may be a parameter)
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
                        // clamps dt where the ET input changes (the interpreter only registers
                        // precipitation series here -- see clampHandles comment)
                        bakeSeries(s->GetName(), sqn, hh, sq.GetTimeSeries(), 200000, /*clampDt=*/true);
                    } else if (sq.GetType() == Quan::_type::expression && sqn != "coefficient") {
                        ExpressionEmitter em(makeCtxSrc(s));
                        srcFns << "    double " << srcFn(s->GetName(), sqn) << "(double t) const { (void)t; return "
                               << em.translate(*sq.GetExpression()) << "; }\n";
                    } else if (sq.GetType() == Quan::_type::value || sq.GetType() == Quan::_type::constant ||
                               sq.GetType() == Quan::_type::boolean) {
                        const std::string ms = srcSym(s->GetName(), sqn), pr = paramRef(s->GetName(), sqn);
                        decls    << "    double " << ms << " = 0.0;\n";
                        initBody << "        " << ms << " = "
                                 << (pr.empty() ? fmt(std::atof(sq.GetProperty(true).c_str())) : pr) << ";\n";
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
        const bool needFlowLocals = system.ConstituentsCount() > 0 || system.ObservationsCount() > 0;
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
        std::function<EmitContext(Object*, bool, int, const std::string&)> makeCtxT;   // recursive: source coefficients
        makeCtxT = [&](Object* cur, bool isLink, int curLinkIdx, const std::string& curConst) -> EmitContext {
            EmitContext ctx; ctx.timeVar = "t_new";
            auto target = [&, cur, isLink](Loc loc) -> Object* {
                if (!isLink || loc == Loc::self) return cur;
                if (loc == Loc::source)      return (Object*)system.block(cur->s_Block_No());
                if (loc == Loc::destination) return (Object*)system.block(cur->e_Block_No());
                return cur;
            };
            ctx.resolveValue = [&, target, isLink, curLinkIdx, curConst](const std::string& name0, Loc loc) -> std::string {
                Object* t = target(loc);
                std::string name = name0;
                Quan* q = t->Variable(name);
                // Object::GetVal (Object.cpp:127) resolves a bare name that the
                // object does not carry through the copy made for the constituent
                // being evaluated -- "<constituent>:<name>". That is how a link
                // expression such as
                //   (diffusion_coefficient*area + dispersivity*flow)/length*(...)
                // reaches the constituent's dispersivity. Without this the term
                // silently became 0 and the dispersive coupling vanished from the
                // Jacobian entirely.
                if (!q && !curConst.empty()) {
                    if (Quan* qc = t->Variable(curConst + ":" + name)) { q = qc; name = curConst + ":" + name; }
                }
                // "<constituent>:<property>" that the object does not carry resolves
                // to the CONSTITUENT object's property (Object.cpp:138). This is how
                // a link's diffusive_masstransfer reaches Cu_aq's dispersivity: the
                // links hold no copy of it at all. Missing this baked the whole
                // dispersive term as 0 and dropped its Jacobian coupling.
                if (!q) {
                    const std::size_t colon = name.find(':');
                    if (colon != std::string::npos) {
                        const std::string cname = name.substr(0, colon);
                        const std::string prop  = name.substr(colon + 1);
                        if (Object* cobj = (Object*)system.constituent(cname)) {
                            if (Quan* cq = cobj->Variable(prop)) {
                                if (const std::string pb = paramRef(cname, prop); !pb.empty())
                                    return pb;                       // calibrated: track params_[i]
                                if (cq->GetType() == Quan::_type::value ||
                                    cq->GetType() == Quan::_type::constant)
                                    return fmt(cobj->GetVal(prop, Expression::timing::present));
                            }
                        }
                    }
                }
                // Only after that does a missing quantity evaluate to 0, as it does
                // in the interpreter (e.g. 'inflow' on a fixed_head).
                if (!q) return "0.0";
                if (name == stateVar)                         // flow-phase storage (fixed)
                    return "flowStorage_[" + stateEnum(t->GetName(), stateVar) + "]";
                if (q->GetType() == Quan::_type::balance)     // constituent mass (transport state)
                    return "mass[" + massEnum(t->GetName(), constPrefix(name)) + "]";
                if (isLink && loc == Loc::self && name == flowVar)  // flow-phase flow (fixed)
                    return "flowFlow_[" + std::to_string(curLinkIdx) + "]";
                if (q->GetType() == Quan::_type::source) {
                    // A Source (Precipitation / Evapotranspiration) referenced from a
                    // constituent expression, e.g. AgeTracker's
                    //   inflow_loading = c_in*(inflow+Precipitation) + Evapotranspiration*concentration
                    // Same expansion as the flow phase (coefficient * timeseries * rate);
                    // the coefficient is translated in this block's TRANSPORT context,
                    // where its area/depth are the cached flow-phase members.
                    Source* s = q->GetSource();
                    if (!s) return std::string("0.0");
                    const bool tIsLink = (t->ObjectType() == object_type::link);
                    std::string coeff = "1.0", rate = "1.0";
                    if (Quan* cq = s->Variable("coefficient")) {
                        if (cq->GetType() == Quan::_type::expression)
                            coeff = "(" + ExpressionEmitter(makeCtxT(t, tIsLink, curLinkIdx, curConst)).translate(*cq->GetExpression()) + ")";
                        else coeff = srcSym(s->GetName(), "coefficient");
                    }
                    if (Quan* rq = s->Variable("rate")) {
                        if (rq->GetType() == Quan::_type::expression) rate = srcFn(s->GetName(), "rate") + "(t_new)";
                        else rate = srcSym(s->GetName(), "rate");
                    }
                    const std::string tsFactor = s->Variable("timeseries")
                        ? srcHandle(s->GetName()) + ".interpol(t_new)" : std::string("1.0");
                    return "(" + coeff + " * " + tsFactor + " * " + rate + ")";
                }
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
                    const std::string pr = paramRef(o->GetName(), qn);
                    decls << "    double " << sym(o->GetName(), qn) << " = 0.0;\n";
                    initBody << "        " << sym(o->GetName(), qn) << " = "
                             << (pr.empty() ? fmt(std::atof(q.GetProperty(true).c_str())) : pr) << ";\n";
                } else if (isSeriesType(q.GetType())) {
                    const std::string hh = tsHandle(o->GetName(), qn);
                    seriesDecls  << "    ohq::TimeSeries " << hh << ";\n";
                    seriesSetters << "    void set_" << hh << "(const ohq::TimeSeries& ts) { " << hh << " = ts; }\n";
                    bakeSeries(o->GetName(), qn, hh, q.GetTimeSeries(), 200000, /*clampDt=*/true);
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
                ExpressionEmitter em(makeCtxT(nd.o, nd.isLink, nd.linkIdx, constPrefix(nd.qn)));
                const Expression* ex = nd.o->Variable(nd.qn)->GetExpression();
                const std::string code = em.translate(*ex);
                if (std::getenv("OHQCG_DEBUG"))   // OHQCG_DEBUG=1 ohq_generate ... : trace transport emission
                    std::fprintf(stderr, "[transport] %s  terms=%zu  text=%s\n   -> %s\n",
                                 key.c_str(), ex->terms.size(), ex->ToString().c_str(), code.c_str());
                transBody << "        const double " << sym(nd.o->GetName(), nd.qn) << " = " << code << ";\n";
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
                // Object::GetVal (Object.cpp:99) looks at the object's OWN quantity
                // first and only then falls back to a constituent or a reaction
                // parameter. Checking the reaction parameter first -- as this did --
                // silently diverges whenever a name exists in both places, e.g. a
                // `porosity` ReactionParameter alongside each block's own porosity:
                // the interpreter reads the block's value, codegen read the global.
                // Invisible while the two are bound to the same parameter, wrong the
                // moment they differ (per-column porosity).
                if (name == stateVar)          // flow-phase storage, not the block quan
                    return "flowStorage_[" + stateEnum(blk->GetName(), stateVar) + "]";
                if (!blk->Variable(name)) {
                    if (system.constituent(name) && blk->Variable(name + ":concentration"))
                        return "ohq::pos(" + sym(blk->GetName(), name + ":concentration") + ")";
                    if (system.reactionparameter(name))
                        return reactionParamRef(name);
                }
                Quan* q = blk->Variable(name);
                if (q) {
                    if (q->GetType() == Quan::_type::source)   // same expansion as the transport ctx
                        return makeCtxT(blk, false, -1, std::string()).resolveValue(name, Loc::self);
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
                    EmitContext ctx = makeCtxT(blk, false, -1, std::string());
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

    // ======================================================================
    //  G2: OBSERVATIONS -- each Observation's `expression` evaluated on its
    //  `object` at the ACCEPTED state after a step (Observation::GetValue with
    //  timing::present), recorded after every step like UpdateObservations().
    // ======================================================================
    const unsigned nObs = system.ObservationsCount();
    std::ostringstream obsBody, obsNameArr, paramNameArr, paramInitArr;
    {
        std::function<EmitContext(Object*, bool, int)> makeCtxObs;
        makeCtxObs = [&](Object* cur, bool isLink, int linkIdx) -> EmitContext {
            EmitContext ctx; ctx.timeVar = "t_new";
            auto target = [&, cur, isLink](Loc loc) -> Object* {
                if (!isLink || loc == Loc::self) return cur;
                if (loc == Loc::source)      return (Object*)system.block(cur->s_Block_No());
                if (loc == Loc::destination) return (Object*)system.block(cur->e_Block_No());
                return cur;
            };
            ctx.resolveValue = [&, target, isLink, linkIdx](const std::string& name, Loc loc) -> std::string {
                Object* t = target(loc);
                Quan* q = t->Variable(name);
                if (!q) {
                    // Object::GetVal's fallbacks (Object.cpp:109) for a name that is
                    // not a quantity of this object. Same chain the reaction resolver
                    // uses; without it a bare constituent name in an observation
                    // expression silently evaluated to 0.
                    //   1. a constituent  -> Pos(<constituent>:concentration)
                    //   2. a reaction parameter -> its ":value", else the object's value
                    if (system.constituent(name)) {
                        if (Quan* qc = t->Variable(name + ":concentration")) {
                            if (qc->GetType() == Quan::_type::expression) {
                                const bool tIsLink2 = (t->ObjectType() == object_type::link);
                                int li2 = -1;
                                if (tIsLink2) for (unsigned l = 0; l < nL; ++l)
                                    if ((Object*)system.link(l) == t) li2 = (int)l;
                                return "ohq::pos(" + ExpressionEmitter(makeCtxObs(t, tIsLink2, li2))
                                                        .translate(*qc->GetExpression()) + ")";
                            }
                            return "ohq::pos(" + sym(t->GetName(), name + ":concentration") + ")";
                        }
                    }
                    if (system.reactionparameter(name)) {
                        if (t->Variable(name + ":value")) return sym(t->GetName(), name + ":value");
                        return reactionParamRef(name);
                    }
                    return "0.0";
                }
                if (name == stateVar) return "solver_.storage(" + stateEnum(t->GetName(), stateVar) + ")";
                if (q->GetType() == Quan::_type::balance)
                    return nC ? "transport_.mass(" + massEnum(t->GetName(), constPrefix(name)) + ")" : std::string("0.0");
                if (isLink && loc == Loc::self && name == flowVar)      // limited flow (calc(link, ., limit=true))
                    return "solver_.linkFlow(" + std::to_string(linkIdx) + ")";
                if (q->GetType() == Quan::_type::source) {
                    Source* s = q->GetSource();
                    if (!s) return std::string("0.0");
                    const bool tIsLink = (t->ObjectType() == object_type::link);
                    std::string coeff = "1.0", rate = "1.0";
                    if (Quan* cq = s->Variable("coefficient")) {
                        if (cq->GetType() == Quan::_type::expression)
                            coeff = "(" + ExpressionEmitter(makeCtxObs(t, tIsLink, linkIdx)).translate(*cq->GetExpression()) + ")";
                        else coeff = srcSym(s->GetName(), "coefficient");
                    }
                    if (Quan* rq = s->Variable("rate")) {
                        if (rq->GetType() == Quan::_type::expression) rate = srcFn(s->GetName(), "rate") + "(t_new)";
                        else rate = srcSym(s->GetName(), "rate");
                    }
                    const std::string tsFactor = s->Variable("timeseries")
                        ? srcHandle(s->GetName()) + ".interpol(t_new)" : std::string("1.0");
                    return "(" + coeff + " * " + tsFactor + " * " + rate + ")";
                }
                if (isSeriesType(q->GetType())) return tsHandle(t->GetName(), name) + ".interpol(t_new)";
                if (!constPrefix(name).empty() && q->GetType() == Quan::_type::expression) {
                    // constituent-scoped expression (AgeTracker_1:concentration): a transport-phase
                    // local, not a member -> inline it here
                    const bool tIsLink = (t->ObjectType() == object_type::link);
                    int li = -1; if (tIsLink) for (unsigned l = 0; l < nL; ++l) if ((Object*)system.link(l) == t) li = (int)l;
                    return "(" + ExpressionEmitter(makeCtxObs(t, tIsLink, li)).translate(*q->GetExpression()) + ")";
                }
                return sym(t->GetName(), name);   // constant / per-step / cached per-iteration member
            };
            ctx.resolveSeries = [&, target](const std::string& name, Loc loc) -> std::string {
                return tsHandle(target(loc)->GetName(), name);
            };
            return ctx;
        };
        for (unsigned i = 0; i < nObs; ++i) {
            Observation* ob = system.observation(i);
            obsNameArr << (i ? ", " : "") << cstr(ob->GetName());
            const std::string loc = ob->GetLocation();
            const std::string exprText = ob->Variable("expression") ? ob->Variable("expression")->GetProperty() : "";
            Object* o = system.object(loc);
            const bool ok = o && !exprText.empty() &&
                            (o->ObjectType() == object_type::block || o->ObjectType() == object_type::link);
            if (!ok) { obsBody << "        out[" << i << "] = 0.0;   // '" << ob->GetName() << "': unresolved object/expression\n"; continue; }
            const bool isLink = (o->ObjectType() == object_type::link);
            int li = -1; if (isLink) for (unsigned l = 0; l < nL; ++l) if ((Object*)system.link(l) == o) li = (int)l;
            Expression ex(exprText);
            obsBody << "        out[" << i << "] = " << ExpressionEmitter(makeCtxObs(o, isLink, li)).translate(ex)
                    << ";   // " << ob->GetName() << " @ " << loc << "\n";
        }
        for (unsigned i = 0; i < nP; ++i) {
            paramNameArr << (i ? ", " : "") << cstr(params[i].name);
            paramInitArr << (i ? ", " : "") << fmt(params[i].value);
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
    const bool L = T || nObs > 0;           // cached flow-phase locals needed
    const bool O = nObs > 0;
    // step(): flow phase, then (optionally) cache locals, transport, observations.
    std::string tStep =
        std::string("    bool stepImpl() {\n"
                    "        const double t_prev = solver_.time(); (void)t_prev;\n"
                    "        ++stepCounter_;   // 1-based, as System::Solve's `counter`\n"
                    "        if (solver_.settings().oscillation_control\n"
                    "            && (stepCounter_ - 1) % (long)solver_.settings().restore_interval == 0)\n"
                    "            saveRestorePoint();\n"
                    "        if (!solver_.step()) return false;\n")
        + (L ? "        for (int b=0;b<N_STATES;++b) flowStorage_[b]=solver_.storage(b);\n"
               "        for (int l=0;l<N_LINKS;++l) flowFlow_[l]=solver_.linkFlow(l);\n"
               "        computeFlowLocals(flowStorage_, solver_.time());\n" : "")
        + (T ? "        // integrate transport over the ACCEPTED step. The forcing time is the\n"
               "        // step's START, as in the interpreter (see forcing_at_step_start).\n"
               "        if (!transport_.step(t_prev, solver_.time() - t_prev)) return false;\n" : "")
        + (O ? "        recordObservations(t_prev);   // System.cpp:1492, stamped before t advances\n" : "")
        + "        oscillationControl();   // inert unless the setting is on\n"
        + "        return true;\n    }\n";
    // ---- oscillation control (System.cpp:1531-1610, CountOscillatingStates
    // 1012, ResetBasedOnRestorePoint 5130). Emitted unconditionally; inert
    // unless solver_.settings().oscillation_control is on.
    std::string oscApi =
        std::string("    // ---- oscillation control ---------------------------------------------\n")
        + "    // A converged step can still be wrong: coarse dt against a fast reaction\n"
        + "    // makes the scheme alternate. Detect a sign-alternating triple in the\n"
        + "    // accepted states, then either rewind to the last restore point and\n"
        + "    // restart at dt/5 (discarding the samples already spoiled) or simply\n"
        + "    // continue at dt/5. Mirrors System::Solve's block step for step.\n"
        + "    struct RestorePoint {\n"
        + "        double t = 0, dt = 0;\n"
        + "        std::vector<double> storage, mass, limitFactor;\n"
        + "        std::vector<int> limited;\n"
        + "        unsigned used = 0;\n"
        + "        bool valid = false;\n"
        + "    };\n"
        + "    int  oscillationEvents() const      { return osc_events_; }\n"
        + "    int  oscillationReductions() const  { return osc_reductions_; }\n"
        + "    int  oscillationRelaxations() const { return osc_relaxations_; }\n"
        + "    bool oscillationGaveUp() const      { return osc_gave_up_; }\n"
        + "    // The solution order the interpreter uses: the flow states, then the\n"
        + "    // transport unknowns (System::GatherSolvedState).\n"
        + "    void gatherSolvedState(std::vector<double>& out) const {\n"
        + "        out.clear();\n"
        + "        for (int b = 0; b < N_STATES; ++b) out.push_back(solver_.storage(b));\n"
        + (T ? "        for (int i = 0; i < N_MASS; ++i) out.push_back(transport_.mass(i));\n" : "")
        + "    }\n"
        + "    void saveRestorePoint() {\n"
        + "        rp_.storage.assign(N_STATES, 0.0); rp_.limited.assign(N_STATES, 0);\n"
        + "        rp_.limitFactor.assign(N_STATES, 1.0);\n"
        + (T ? "        rp_.mass.assign(N_MASS, 0.0);\n" : "")
        + "        exportState(rp_.storage.data(), " + std::string(T ? "rp_.mass.data()" : "nullptr")
          + ", rp_.limited.data(), rp_.limitFactor.data());\n"
        + "        rp_.t = solver_.time(); rp_.dt = solver_.dtBase(); rp_.used = 0; rp_.valid = true;\n"
        + "    }\n"
        + "    // ResetBasedOnRestorePoint: restart at a fifth of the step and hold it\n"
        + "    // there with a ceiling, or the oscillation returns within ten steps.\n"
        + "    bool resetToRestorePoint() {\n"
        + "        const auto& S = solver_.settings();\n"
        + "        if (!rp_.valid || rp_.used >= S.restore_point_max_uses) return false;\n"
        + "        importState(rp_.t, rp_.storage.data(), " + std::string(T ? "rp_.mass.data()" : "nullptr")
          + ", rp_.limited.data(), rp_.limitFactor.data());\n"
        + "        rp_.used++;\n"
        + "        const double dtn = rp_.dt / 5.0;\n"
        + "        solver_.setDtBase(dtn); solver_.setDtCeiling(dtn);\n"
        + "        clean_steps_ = 0;\n"
        + "        knockoutOutputs(rp_.t);\n"
        + "        rp_.dt = dtn;\n"
        + "        return true;\n"
        + "    }\n"
        + "    // TimeSeriesSet::knockout -- drop the samples the rewind invalidated.\n"
        + "    void knockoutOutputs(double tcut) {\n"
        + "        for (auto& s : obs_) {\n"
        + "            ohq::TimeSeries kept;\n"
        + "            for (std::size_t k = 0; k < s.size(); ++k)\n"
        + "                if (s.t[k] <= tcut) kept.push(s.t[k], s.c[k]);\n"
        + "            s = kept;\n"
        + "        }\n"
        + "    }\n"
        + "    // CountOscillatingStates: increments that alternate in sign and are not\n"
        + "    // all negligible against the local magnitude (TimeSeries::wiggle_sl).\n"
        + "    int countOscillatingStates(double tol) {\n"
        + "        std::vector<double> x; gatherSolvedState(x);\n"
        + "        osc_history_.push_back(x);\n"
        + "        if (osc_history_.size() > 4) osc_history_.erase(osc_history_.begin());\n"
        + "        if (osc_history_.size() < 4) return 0;\n"
        + "        const std::vector<double>& c4 = osc_history_[0];\n"
        + "        const std::vector<double>& c3 = osc_history_[1];\n"
        + "        const std::vector<double>& c2 = osc_history_[2];\n"
        + "        const std::vector<double>& c1 = osc_history_[3];\n"
        + "        const std::size_t n = c1.size();\n"
        + "        if (c2.size() != n || c3.size() != n || c4.size() != n) return 0;\n"
        + "        int count = 0;\n"
        + "        for (std::size_t i = 0; i < n; ++i) {\n"
        + "            const double scale = (std::fabs(c1[i]) + std::fabs(c2[i]) + std::fabs(c3[i])\n"
        + "                                  + std::fabs(c4[i])) / 4.0 + tol / 100.0;\n"
        + "            if (scale <= 0) continue;\n"
        + "            const double d1 = (c1[i] - c2[i]) / scale;\n"
        + "            const double d2 = (c2[i] - c3[i]) / scale;\n"
        + "            const double d3 = (c3[i] - c4[i]) / scale;\n"
        + "            const bool all_small   = std::fabs(d1) < tol && std::fabs(d2) < tol && std::fabs(d3) < tol;\n"
        + "            const bool alternating = (d1 * d2 < 0) && (d2 * d3 < 0);\n"
        + "            if (!all_small && alternating) count++;\n"
        + "        }\n"
        + "        return count;\n"
        + "    }\n"
        + "    // Run after an accepted step, in System::Solve's order: relax an existing\n"
        + "    // ceiling first, then test for oscillation.\n"
        + "    void oscillationControl() {\n"
        + "        const auto& S = solver_.settings();\n"
        + "        if (solver_.dtCeiling() > 0 && S.oscillation_relax_after > 0) {\n"
        + "            if (++clean_steps_ >= S.oscillation_relax_after) {\n"
        + "                clean_steps_ = 0;\n"
        + "                solver_.setDtCeiling(solver_.dtCeiling() * 2.0);\n"
        + "                osc_relaxations_++;\n"
        + "                if (osc_reductions_ > 0) osc_reductions_--;\n"
        + "                if (solver_.dtCeiling() >= dt0_ * S.dt_max_factor) solver_.setDtCeiling(0.0);\n"
        + "            }\n"
        + "        }\n"
        + "        if (!S.oscillation_control) return;\n"
        + "        if (stepCounter_ <= 4 || stepCounter_ - osc_last_counter_ <= 4) return;   // cooldown\n"
        + "        const int nosc = countOscillatingStates(S.oscillation_tolerance);\n"
        + "        if (nosc <= 0) return;\n"
        + "        clean_steps_ = 0;\n"
        + "        const bool budget_left = osc_reductions_ < S.oscillation_max_reductions;\n"
        + "        bool acted = false;\n"
        + "        if (budget_left && S.oscillation_rewind) acted = resetToRestorePoint();\n"
        + "        else if (budget_left) {\n"
        + "            const double dtn = solver_.dtBase() / 5.0;\n"
        + "            solver_.setDtBase(dtn); solver_.setDtCeiling(dtn);\n"
        + "            clean_steps_ = 0;\n"
        + "            acted = true;\n"
        + "        }\n"
        + "        if (acted) {\n"
        + "            osc_reductions_++; osc_events_++;\n"
        + "            osc_last_counter_ = stepCounter_;\n"
        + "            osc_history_.clear();          // the state jumped backwards\n"
        + "            solver_.requestJacobianUpdate();\n"
        + (T ? "            transport_.requestJacobianUpdate();\n" : "")
        + "        } else if (!osc_gave_up_) {\n"
        + "            osc_gave_up_ = true;           // budget exhausted; report once\n"
        + "        }\n"
        + "    }\n";

    std::string localsFn = L ? (
        std::string("    // flow-phase per-iteration quantities, recomputed once per accepted step\n")
        + "    // from the committed storages (transport expressions and observations read them)\n"
        + "    void computeFlowLocals(const double* eff, double t_new) {\n"
        + "        (void)eff; (void)t_new;\n" + flowLocalsBody.str() + "    }\n") : "";
    // G1/G2 API: parameters as runtime inputs, observations recorded per step.
    std::string paramApi =
        std::string("    // ---- parameters (setasparameter bindings; index = model parameter order) ----\n")
        + "    static const char* parameterName(int i) { static const char* a[] = {" + (nP ? paramNameArr.str() : std::string("\"\"")) + "}; return a[i]; }\n"
        + "    double parameter(int i) const { return params_[i]; }\n"
        + "    void setParameter(int i, double v) { params_[i] = v; }        // then applyParameters()\n"
        + "    void setParameters(const double* v) { for (int i = 0; i < N_PARAMETERS; ++i) params_[i] = v[i]; }\n"
        + "    void applyParameters() { buildConstants(); }                   // == System::ApplyParameters + derived constants\n"
        + [&]{ std::string s; for (const std::string& n : paramNotes) s += "    // NOTE parameter binding not owned by the kernel: " + n + "\n"; return s; }()
        + "    // ---- observations (expression on object, at the accepted state, every step) ----\n"
        + "    static const char* observationName(int i) { static const char* a[] = {" + (O ? obsNameArr.str() : std::string("\"\"")) + "}; return a[i]; }\n"
        + "    // t_eval is the step's START time: the interpreter calls UpdateObservations\n"
          "    // (System.cpp:1492) before advancing SolverTempVars.t, so an observation\n"
          "    // expression sees the new state at the old time.\n"
        + "    void computeObservations(double* out, double t_eval) const {\n        const double t_new = t_eval; (void)t_new; (void)out;\n" + obsBody.str() + "    }\n"
        + "    const ohq::TimeSeries& observationSeries(int i) const { return obs_[i]; }\n"
        + "    void clearObservations() { for (auto& s : obs_) { s.t.clear(); s.c.clear(); } }\n"
        + "    // System::FinalizeOutputs: resample the recorded observations onto the\n"
          "    // uniform initial_time_step grid. The step stayed adaptive; this only\n"
          "    // changes the sample times the caller sees, so a kernel objective compares\n"
          "    // on the same grid Objective_Function uses (make_uniform(dt0)).\n"
        + "    void uniformizeObservations() { for (auto& s : obs_) s = s.makeUniform(dt0_); }\n"
        + "    void recordObservations(double t_eval) {\n        double v[N_OBSERVATIONS > 0 ? N_OBSERVATIONS : 1]; computeObservations(v, t_eval);\n"
          "        for (int i = 0; i < N_OBSERVATIONS; ++i) obs_[i].push(t_eval, v[i]);\n    }\n";
    // G4/G5 API: named series injection; state values in/out.
    std::ostringstream ioApi;
    {
        std::ostringstream so, sq, sp;
        for (size_t k = 0; k < seriesTable.size(); ++k) {
            so << (k ? ", " : "") << cstr(seriesTable[k].obj);
            sq << (k ? ", " : "") << cstr(seriesTable[k].q);
            sp << (k ? ", " : "") << "&" << seriesTable[k].handle;
        }
        const size_t nS = seriesTable.size();
        ioApi << "    // ---- runtime forcing (G4): replace a baked series by (object, quantity) name ----\n"
              << "    enum { N_SERIES = " << nS << " };\n"
              << "    static const char* seriesObject(int k)   { static const char* a[] = {" << (nS ? so.str() : std::string("\"\"")) << "}; return a[k]; }\n"
              << "    static const char* seriesQuantity(int k) { static const char* a[] = {" << (nS ? sq.str() : std::string("\"\"")) << "}; return a[k]; }\n"
              << "    ohq::TimeSeries* series(const char* object, const char* quantity) {\n"
              << "        ohq::TimeSeries* p[] = {" << (nS ? sp.str() : std::string("nullptr")) << "};\n"
              << "        for (int k = 0; k < N_SERIES; ++k) if (std::strcmp(seriesObject(k), object) == 0 && std::strcmp(seriesQuantity(k), quantity) == 0) return p[k];\n"
              << "        return nullptr;\n    }\n"
              << "    // (t, value) samples, in the quantity's SI unit; the dt clamp's D is recomputed lazily\n"
              << "    bool setSeries(const char* object, const char* quantity, const double* t, const double* v, int n) {\n"
              << "        ohq::TimeSeries* s = series(object, quantity); if (!s) return false;\n"
              << "        s->t.assign(t, t + n); s->c.assign(v, v + n); s->d.clear(); return true;\n    }\n"
              << "    // precipitation bins (CPrecipitation: start, end, depth) -> midpoint samples of depth/(end-start),\n"
              << "    // exactly how the interpreter converts a precipitation file / injection\n"
              << "    bool setPrecipitation(const char* object, const char* quantity, const double* start, const double* end, const double* depth, int n) {\n"
              << "        ohq::TimeSeries* s = series(object, quantity); if (!s) return false;\n"
              << "        s->t.clear(); s->c.clear(); s->d.clear();\n"
              << "        for (int k = 0; k < n; ++k) { const double w = end[k] - start[k]; if (w > 0) s->push(0.5 * (start[k] + end[k]), depth[k] / w); }\n"
              << "        return true;\n    }\n"
              << "    // ---- state values in/out (G5): what a hot restart needs -- never model structure ----\n"
              << "    // mass (N_MASS) / limited / limitFactor (N_STATES) may be null.\n"
              << "    void exportState(double* storage, double* mass, int* limited, double* limitFactor) const {\n"
              << "        for (int b = 0; b < N_STATES; ++b) { storage[b] = solver_.storage(b);\n"
              << "            if (limited) limited[b] = solver_.isLimited(b) ? 1 : 0; if (limitFactor) limitFactor[b] = solver_.limitFactor(b); }\n"
              << (T ? "        if (mass) for (int i = 0; i < N_MASS; ++i) mass[i] = transport_.mass(i);\n" : "        (void)mass;\n")
              << "    }\n"
              << "    // t: restart time; mass null = keep, limited null = none, limitFactor null = 1. dt restarts from dt0 as System::Solve does.\n"
              << "    void importState(double t, const double* storage, const double* mass, const int* limited, const double* limitFactor) {\n"
              << "        solver_.setState(t, storage, limited, limitFactor);\n"
              << (T ? "        if (mass) transport_.setMass(mass);\n" : "        (void)mass;\n")
              << (L ? "        for (int b = 0; b < N_STATES; ++b) flowStorage_[b] = solver_.storage(b);\n"
                      "        computeFlowLocals(flowStorage_, solver_.time());\n" : "")
              << "    }\n";
    }
    std::string tHooks = T ? (
        std::string("    // ---- transport hooks ----\n")
        + "    int nMass() const { return N_MASS; }\n"
        + "    int lastTransportIterations() const { return transport_.lastIterations(); }\n"
        + "    int nConst() const { return " + std::to_string(nC) + "; }\n"
        + "    int nLinksT() const { return N_LINKS; }\n"
        + "    int linkSrcT(int l) const { return linkSrc(l); }\n"
        + "    int linkDstT(int l) const { return linkDst(l); }\n"
        + "    double constMass(int i) const { return transport_.mass(i); }\n"
        + "    void initialMass(double* m0) {\n" + iniMassBody.str() + "    }\n"
        + "    void computeTransportFluxes(const double* mass, double t_new, double* massTransfer, double* inflowOwn) const {\n"
        + "        (void)mass; (void)t_new; (void)massTransfer; (void)inflowOwn;\n"
        + transBody.str() + "    }\n") : "";
    std::string tMembers =
        (L ? std::string("    double flowStorage_[N_STATES] = {0};\n")
             + "    double flowFlow_[N_LINKS>0?N_LINKS:1] = {0};\n" : std::string())
        + (T ? "    ohq::TransportSolver<" + cls + "> transport_;\n" : std::string())
        + "    double params_[N_PARAMETERS > 0 ? N_PARAMETERS : 1] = {" + (nP ? paramInitArr.str() : std::string("0.0")) + "};\n"
        + "    ohq::TimeSeries obs_[N_OBSERVATIONS > 0 ? N_OBSERVATIONS : 1];\n"
        + "    double dt0_ = 0;\n"
        + "    // oscillation-control state (see oscillationControl())\n"
        + "    RestorePoint rp_;\n"
        + "    std::vector<std::vector<double>> osc_history_;\n"
        + "    long stepCounter_ = 0, osc_last_counter_ = -1;\n"
        + "    int  clean_steps_ = 0, osc_reductions_ = 0, osc_events_ = 0, osc_relaxations_ = 0;\n"
        + "    bool osc_gave_up_ = false;\n"
        + "    // G6 solver status\n"
        + "    bool solutionFailed_ = false;\n"
        + "    double duration_ = 0.0;\n";

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
          << "; S.dt_min_factor = " << fmt(ss.minimum_timestep / dt0v) << ";\n"
          << "          S.dt_floor_factor = " << fmt(1.0 / (ss.timestepminfactor > 0 ? ss.timestepminfactor : 1e5))
          << "; S.dt_abs_min = " << fmt(ss.minimum_timestep) << ";\n"
          << "          // Newton iterate path -- see SolverSettings::interpreter_newton\n"
          << "          S.interpreter_newton = true; S.optimize_lambda = " << (ss.optimize_lambda ? "true" : "false")
          << "; S.nr_coeff_reduction = " << fmt(ss.NR_coeff_reduction_factor)
          << ";\n          S.update_jacobian_every_iteration = " << (ss.update_jacobian_every_iteration ? "true" : "false")
          << "; S.jac_refresh_every = 50;\n"
          << "          S.oscillation_control = " << (ss.oscillation_control ? "true" : "false")
          << "; S.oscillation_tolerance = " << fmt(ss.oscillation_tolerance)
          << "; S.oscillation_rewind = " << (ss.oscillation_rewind ? "true" : "false") << ";\n"
          << "          S.oscillation_relax_after = " << ss.oscillation_relax_after
          << "; S.oscillation_max_reductions = " << ss.oscillation_max_reductions
          << "; S.restore_interval = " << system.RestoreInterval()
          << "; S.restore_point_max_uses = " << system.RestorePointMaxUses() << "; }\n";
        return b.str();
    };
    std::string settingsInit = settingsFor("solver_")
        + (T ? settingsFor("transport_") : std::string())
        + "        solver_.landtozero = " + fmt(ss.landtozero_factor) + ";\n";

    std::ostringstream h;
    h << "// Auto-generated by OpenHydroQual model compiler. Do not edit.\n"
      << "#ifndef OHQ_GEN_" << sanitize(cls) << "_H\n#define OHQ_GEN_" << sanitize(cls) << "_H\n\n"
      << "#include <cstring>\n"
      << "#include <chrono>\n"
      << "#include \"ohq_intrinsics.h\"\n#include \"ohq_timeseries.h\"\n#include \"ohq_massbalance.h\"\n"
      << tInclude << "\n"
      << "class " << cls << " {\npublic:\n"
      << "    enum State {\n" << stateEnumBody.str() << "        N_STATES = " << nB << "\n    };\n"
      << "    enum { N_LINKS = " << nL << ", N_PARAMETERS = " << nP << ", N_OBSERVATIONS = " << nObs << " };\n"
      << tEnum << "\n"
      << "    " << cls << "() : solver_(*this)" << tCtor << " {}\n\n"
      << seriesSetters.str() << srcFns.str() << "\n"
      << "    void initialize(double tstart = " << fmt(system.tstart()) << ", double dt0 = "
      << fmt(system.dt0()) << ") {\n        loadSeries();\n        buildConstants();\n" << settingsInit
      << "        dt0_ = dt0;   // output grid: System::FinalizeOutputs uniformizes with dt0\n"
      << "        solver_.initialize(tstart, dt0);\n"
      << "        bindSelf();\n"
      << (T ? "        transport_.initialize();\n" : "")
      << (L ? "        for (int b=0;b<N_STATES;++b) flowStorage_[b]=solver_.storage(b);\n"
              "        computeFlowLocals(flowStorage_, solver_.time());\n" : "")
      << (O ? "        clearObservations();   // System::Solve records no observation before the loop\n" : "")
      << "        resetStatus();   // a re-initialize starts a fresh solve (new GA/MCMC sample)\n"
      << "    }\n"
      << tStep
      // ---- G6: solver status (what GetObjectiveFunctionValue and the MCMC
      // detail log read). step() is the public entry: it latches the failure
      // flag so a host that drives stepTo() sees it too.
      << "    bool step() {\n"
         "        if (solver_.model() != this) bindSelf();   // a copied kernel re-binds on first use\n"
         "        const auto _t0 = std::chrono::steady_clock::now();\n"
         "        const bool ok = stepImpl();\n"
         "        duration_ += std::chrono::duration<double>(std::chrono::steady_clock::now() - _t0).count();\n"
         "        if (!ok) solutionFailed_ = true;\n"
         "        return ok;\n    }\n"
      << "    bool runTo(double t_end) {   // duration_ accumulates inside step()\n"
         "        solver_.setStop(t_end);\n"
         "        while (time() < t_end - 1e-30) { if (!step()) return false; }\n"
         "        return true;\n    }\n"
      << "    // true if ANY step failed since initialize()/resetStatus()\n"
      << "    bool solutionFailed() const   { return solutionFailed_; }\n"
      << "    // wall seconds accumulated inside runTo() (System::GetSimulationDuration)\n"
      << "    double simulationDuration() const { return duration_; }\n"
      << "    long stepCount() const        { return stepCounter_; }\n"
      << "    void resetStatus() { solutionFailed_ = false; duration_ = 0.0; }\n"
      // ---- G7: copy safety -------------------------------------------------
      // The solvers hold a POINTER to their model and the dt clamp holds pointers
      // to this object's series members. A copy (one kernel per MCMC chain) must
      // re-point all of them at ITSELF, or it would solve using the ORIGINAL's
      // parameters and forcing. step() self-heals, so a plain copy is safe.
      << "    void bindSelf() {\n"
         "        solver_.rebind(*this);\n"
      << (T ? "        transport_.rebind(*this);\n" : "")
      << "        solver_.clearClampSeries();\n"
      << [&]{ std::ostringstream b; for (const std::string& hh : clampHandles) b << "        solver_.addClampSeries(&" << hh << ");   // dt clamp (interpol_D)\n"; return b.str(); }()
      << "    }\n"
      << paramApi << ioApi.str()
      << "    // the interpreter writes outputs on a uniform grid of initial_time_step\n"
      << "    double outputInterval() const { return dt0_; }\n"
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
      << localsFn
      << tHooks << "\n"
      << oscApi << "\n"
      << "private:\n"
      << "    void loadSeries() {\n" << seriesInit.str() << "    }\n"
      << "    void buildConstants() {\n" << initBody.str() << "    }\n"
      << decls.str()
      << seriesDecls.str()
      << "    ohq::MassBalanceSolver<" << cls << "> solver_;\n"
      << tMembers
      << "};\n\n#endif\n";

    // ---- generation-time guard -------------------------------------------
    // Every estimated parameter must actually appear as params_[i] in the emitted
    // code. If it does not, setParameter()/applyParameters() cannot move it and a
    // calibration driving this kernel would silently optimise a frozen model --
    // which is exactly what happened before the binding was made generic. The
    // only exceptions are the host-owned likelihood parameters recorded above.
    {
        const std::string src = h.str();
        std::vector<int> dead;
        for (unsigned i = 0; i < nP; ++i) {
            const std::string tok = "params_[" + std::to_string(i) + "]";
            if (src.find(tok) == std::string::npos) dead.push_back(int(i));
        }
        std::string msg;
        for (int i : dead) {
            // host-owned parameters legitimately never appear; identify them by
            // the note recorded when the binding was skipped.
            bool excused = false;
            for (const std::string& n : paramNotes)
                if (n.rfind(params[i].name + " ->", 0) == 0 &&
                    n.find("applied by the host") != std::string::npos) excused = true;
            if (!excused)
                msg += "  parameter " + std::to_string(i) + " '" + params[i].name
                     + "' never appears in the generated code\n";
        }
        if (!msg.empty())
            throw std::runtime_error("CodeGenerator: the emitted kernel would ignore these "
                                     "estimated parameters:\n" + msg +
                                     "Refusing to emit a kernel a calibration cannot drive.");
    }

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
                 "#include <cstdio>\n#include <cstdlib>\n#include <chrono>\n#include <string>\n#include <vector>\n"
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
                 "    // The interpreter records every accepted step and then resamples onto a\n"
                 "    // uniform initial_time_step grid before writing (its \"Uniformizing outputs\"\n"
                 "    // pass). Do the same: the step stays adaptive, only the OUTPUT is uniform,\n"
                 "    // so a generated run is directly comparable with an interpreter run.\n"
                 "    std::vector<double> rec_t; std::vector<std::vector<double>> rec_y;\n"
                 "    auto record = [&]() {\n"
                 "        rec_t.push_back(m.time());\n"
                 "        std::vector<double> row;\n"
                 "        for (int i = 0; i < m.nBlocks(); ++i) row.push_back(m.state(i));\n"
              << (T ? "        for (int i = 0; i < m.nMass(); ++i) row.push_back(m.constMass(i));\n" : "")
              << "        rec_y.push_back(std::move(row));\n    };\n\n"
                 "    const auto t0 = std::chrono::steady_clock::now();\n"
                 "    record();\n    bool ok = true;\n"
                 "    // System::Solve runs `while (t < tend)` and does NOT clamp the last\n"
                 "    // step to tend, so the run overshoots by up to one dt. Clamping here\n"
                 "    // instead would make the final step -- and only it -- differ.\n"
                 "    while (m.time() < tEnd) {\n"
                 "        if (!m.step()) { ok = false; break; }\n"
                 "        record();\n    }\n"
                 "    const double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();\n"
                 "    // TimeSeries<double>::make_uniform (aquifolium/src/TimeSeries.hpp:1533):\n"
                 "    // grid anchored at t[0] and advanced by repeated addition of dt0, bracket\n"
                 "    // advanced while t[i+1] < t_grid but never past last-1, every point emitted.\n"
                 "    // The times depend only on rec_t, so all columns share one grid.\n"
                 "    {\n"
                 "        const double dtOut = m.outputInterval();\n"
                 "        const size_t ncol = rec_y.empty() ? 0 : rec_y[0].size();\n"
                 "        if (rec_t.size() > 1 && dtOut > 0) {\n"
                 "            const size_t last = rec_t.size() - 1;\n"
                 "            size_t i = 0;\n"
                 "            for (double cur = rec_t[0]; cur <= rec_t[last]; cur += dtOut) {\n"
                 "                while (i + 1 < last && rec_t[i + 1] < cur) ++i;\n"
                 "                const double dt = rec_t[i + 1] - rec_t[i];\n"
                 "                const double r = (dt == 0.0) ? 0.5 : (cur - rec_t[i]) / dt;\n"
                 "                std::fprintf(f, \"%.10g\", cur);\n"
                 "                for (size_t c = 0; c < ncol; ++c)\n"
                 "                    std::fprintf(f, \",%.10g\", rec_y[i][c] + r * (rec_y[i + 1][c] - rec_y[i][c]));\n"
                 "                std::fprintf(f, \"\\n\");\n"
                 "            }\n"
                 "        } else {\n"
                 "            for (size_t k = 0; k < rec_t.size(); ++k) {\n"
                 "                std::fprintf(f, \"%.10g\", rec_t[k]);\n"
                 "                for (size_t c = 0; c < ncol; ++c) std::fprintf(f, \",%.10g\", rec_y[k][c]);\n"
                 "                std::fprintf(f, \"\\n\");\n"
                 "            }\n"
                 "        }\n"
                 "    }\n"
                 "    std::fclose(f);\n"
                 "    if (" << cls << "::N_OBSERVATIONS > 0) {   // observations.csv next to the output\n"
                 "        std::string op = outPath; const size_t dot = op.rfind('.'); op = op.substr(0, dot == std::string::npos ? op.size() : dot) + \"_observations.csv\";\n"
                 "        if (std::FILE* g = std::fopen(op.c_str(), \"w\")) {\n"
                 "            m.uniformizeObservations();   // System::FinalizeOutputs: ObservedOutputs.make_uniform(dt0)\n"
                 "            std::fprintf(g, \"time\"); for (int i = 0; i < " << cls << "::N_OBSERVATIONS; ++i) std::fprintf(g, \",%s\", m.observationName(i)); std::fprintf(g, \"\\n\");\n"
                 "            const ohq::TimeSeries& s0 = m.observationSeries(0);\n"
                 "            for (size_t k = 0; k < s0.size(); ++k) { std::fprintf(g, \"%.10g\", s0.t[k]);\n"
                 "                for (int i = 0; i < " << cls << "::N_OBSERVATIONS; ++i) std::fprintf(g, \",%.10g\", m.observationSeries(i).c[k]); std::fprintf(g, \"\\n\"); }\n"
                 "            std::fclose(g);\n        }\n    }\n"
                 "    std::printf(\"" << cls << ": t=%.6g  %s  steps=%ld  solve=%.3f s  wall=%.3f s  -> %s\\n\",\n"
                 "                m.time(), ok ? \"OK\" : \"SOLVE FAILED\", m.stepCount(), m.simulationDuration(), sec, outPath);\n"
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
                  "/* parameters (setasparameter bindings, model order) and observations (per accepted step) */\n"
               << CLS << "_API int    " << cls << "_n_parameters(void);\n"
               << CLS << "_API const char* " << cls << "_parameter_name(int i);\n"
               << CLS << "_API double " << cls << "_parameter(const " << cls << "_handle* h, int i);\n"
               << CLS << "_API void   " << cls << "_set_parameter(" << cls << "_handle* h, int i, double v);\n"
               << CLS << "_API void   " << cls << "_apply_parameters(" << cls << "_handle* h);\n"
               << CLS << "_API int    " << cls << "_n_observations(void);\n"
               << CLS << "_API const char* " << cls << "_observation_name(int i);\n"
               << CLS << "_API int    " << cls << "_observation_count(const " << cls << "_handle* h, int i);\n"
               << CLS << "_API int    " << cls << "_observation_at(const " << cls << "_handle* h, int i, int k, double* t, double* v);\n"
               << CLS << "_API void   " << cls << "_clear_observations(" << cls << "_handle* h);\n"
                  "/* solver status: a failed solve must not be scored as a good fit */\n"
               << CLS << "_API int    " << cls << "_solution_failed(const " << cls << "_handle* h);\n"
               << CLS << "_API double " << cls << "_simulation_duration(const " << cls << "_handle* h);\n"
               << CLS << "_API long   " << cls << "_step_count(const " << cls << "_handle* h);\n"
               << CLS << "_API int    " << cls << "_last_iterations(const " << cls << "_handle* h);\n"
               << CLS << "_API void   " << cls << "_reset_status(" << cls << "_handle* h);\n"
                  "/* runtime forcing: replace a baked series by (object, quantity) name */\n"
               << CLS << "_API int    " << cls << "_n_series(void);\n"
               << CLS << "_API const char* " << cls << "_series_object(int k);\n"
               << CLS << "_API const char* " << cls << "_series_quantity(int k);\n"
               << CLS << "_API int    " << cls << "_set_series(" << cls << "_handle* h, const char* object, const char* quantity, const double* t, const double* v, int n);\n"
               << CLS << "_API int    " << cls << "_set_precipitation(" << cls << "_handle* h, const char* object, const char* quantity, const double* start, const double* end, const double* depth, int n);\n"
                  "/* state values in/out (hot restart); mass/limited/limitFactor may be NULL */\n"
               << CLS << "_API void   " << cls << "_export_state(const " << cls << "_handle* h, double* storage, double* mass, int* limited, double* limitFactor);\n"
               << CLS << "_API void   " << cls << "_import_state(" << cls << "_handle* h, double t, const double* storage, const double* mass, const int* limited, const double* limitFactor);\n"
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
                  "int    " << cls << "_n_parameters(void) { return " << cls << "::N_PARAMETERS; }\n"
                  "const char* " << cls << "_parameter_name(int i) { return " << cls << "::parameterName(i); }\n"
                  "double " << cls << "_parameter(const " << cls << "_handle* h, int i) { return h->m.parameter(i); }\n"
                  "void   " << cls << "_set_parameter(" << cls << "_handle* h, int i, double v) { h->m.setParameter(i, v); }\n"
                  "void   " << cls << "_apply_parameters(" << cls << "_handle* h) { h->m.applyParameters(); }\n"
                  "int    " << cls << "_n_observations(void) { return " << cls << "::N_OBSERVATIONS; }\n"
                  "const char* " << cls << "_observation_name(int i) { return " << cls << "::observationName(i); }\n"
                  "int    " << cls << "_observation_count(const " << cls << "_handle* h, int i) { return (int)h->m.observationSeries(i).size(); }\n"
                  "int    " << cls << "_observation_at(const " << cls << "_handle* h, int i, int k, double* t, double* v) {\n"
                  "    const ohq::TimeSeries& s = h->m.observationSeries(i); if (k < 0 || k >= (int)s.size()) return 0; *t = s.t[k]; *v = s.c[k]; return 1; }\n"
                  "void   " << cls << "_clear_observations(" << cls << "_handle* h) { h->m.clearObservations(); }\n"
                  "int    " << cls << "_solution_failed(const " << cls << "_handle* h) { return h->m.solutionFailed() ? 1 : 0; }\n"
                  "double " << cls << "_simulation_duration(const " << cls << "_handle* h) { return h->m.simulationDuration(); }\n"
                  "long   " << cls << "_step_count(const " << cls << "_handle* h) { return h->m.stepCount(); }\n"
                  "int    " << cls << "_last_iterations(const " << cls << "_handle* h) { return h->m.lastIterations(); }\n"
                  "void   " << cls << "_reset_status(" << cls << "_handle* h) { h->m.resetStatus(); }\n"
                  "int    " << cls << "_n_series(void) { return " << cls << "::N_SERIES; }\n"
                  "const char* " << cls << "_series_object(int k) { return " << cls << "::seriesObject(k); }\n"
                  "const char* " << cls << "_series_quantity(int k) { return " << cls << "::seriesQuantity(k); }\n"
                  "int    " << cls << "_set_series(" << cls << "_handle* h, const char* o, const char* q, const double* t, const double* v, int n) { return h->m.setSeries(o, q, t, v, n) ? 1 : 0; }\n"
                  "int    " << cls << "_set_precipitation(" << cls << "_handle* h, const char* o, const char* q, const double* s, const double* e, const double* d, int n) { return h->m.setPrecipitation(o, q, s, e, d, n) ? 1 : 0; }\n"
                  "void   " << cls << "_export_state(const " << cls << "_handle* h, double* st, double* ma, int* li, double* lf) { h->m.exportState(st, ma, li, lf); }\n"
                  "void   " << cls << "_import_state(" << cls << "_handle* h, double t, const double* st, const double* ma, const int* li, const double* lf) { h->m.importState(t, st, ma, li, lf); }\n"
               << (T ? "int    " + cls + "_n_mass(void) { return " + cls + "::N_MASS; }\n"
                       "double " + cls + "_mass(const " + cls + "_handle* h, int i) { return h->m.constMass(i); }\n"
                     : std::string())
               << "\n"
                  "/* ---- model-independent alias ABI -------------------------------------\n"
                  "   The names above carry the class name, so a host that dlopen()s the\n"
                  "   library would have to know it. These fixed names let a generic host\n"
                  "   (OHQ-GA / OHQ-MCMC --kernel) drive ANY generated model. Keep in sync\n"
                  "   with tools/ohq_kernel.h. ohq_kernel_abi_version() guards changes. */\n"
                  "int    ohq_kernel_abi_version(void) { return 1; }\n"
                  "const char* ohq_kernel_class_name(void) { return \"" << cls << "\"; }\n"
                  "void*  ohq_kernel_create(void) { return (void*)" << cls << "_create(); }\n"
                  "void   ohq_kernel_destroy(void* h) { " << cls << "_destroy((" << cls << "_handle*)h); }\n"
                  "void   ohq_kernel_initialize(void* h) { " << cls << "_initialize((" << cls << "_handle*)h); }\n"
                  "int    ohq_kernel_run_to(void* h, double t) { return " << cls << "_run_to((" << cls << "_handle*)h, t); }\n"
                  "/* One accepted step, dt clamped so it never crosses t_end. A host that\n"
                  "   must honour a wall-clock budget (System::Solve aborts at\n"
                  "   maximum_time_allowed) steps with this instead of run_to, which has no\n"
                  "   cancellation: a parameter set that collapses dt to the floor would\n"
                  "   otherwise run for millions of steps with no way out. */\n"
                  "int    ohq_kernel_step_to(void* h, double t) { return " << cls << "_step_to((" << cls << "_handle*)h, t); }\n"
                  "double ohq_kernel_time(const void* h) { return " << cls << "_time((const " << cls << "_handle*)h); }\n"
                  "double ohq_kernel_simulation_start(void) { return " << cls << "_simulation_start(); }\n"
                  "double ohq_kernel_simulation_end(void) { return " << cls << "_simulation_end(); }\n"
                  "int    ohq_kernel_n_parameters(void) { return " << cls << "_n_parameters(); }\n"
                  "const char* ohq_kernel_parameter_name(int i) { return " << cls << "_parameter_name(i); }\n"
                  "void   ohq_kernel_set_parameter(void* h, int i, double v) { " << cls << "_set_parameter((" << cls << "_handle*)h, i, v); }\n"
                  "void   ohq_kernel_apply_parameters(void* h) { " << cls << "_apply_parameters((" << cls << "_handle*)h); }\n"
                  "int    ohq_kernel_n_observations(void) { return " << cls << "_n_observations(); }\n"
                  "const char* ohq_kernel_observation_name(int i) { return " << cls << "_observation_name(i); }\n"
                  "int    ohq_kernel_observation_count(const void* h, int i) { return " << cls << "_observation_count((const " << cls << "_handle*)h, i); }\n"
                  "int    ohq_kernel_observation_at(const void* h, int i, int k, double* t, double* v) { return " << cls << "_observation_at((const " << cls << "_handle*)h, i, k, t, v); }\n"
                  "void   ohq_kernel_clear_observations(void* h) { " << cls << "_clear_observations((" << cls << "_handle*)h); }\n"
                  "int    ohq_kernel_solution_failed(const void* h) { return " << cls << "_solution_failed((const " << cls << "_handle*)h); }\n"
                  "double ohq_kernel_simulation_duration(const void* h) { return " << cls << "_simulation_duration((const " << cls << "_handle*)h); }\n"
                  "long   ohq_kernel_step_count(const void* h) { return " << cls << "_step_count((const " << cls << "_handle*)h); }\n"
                  "int    ohq_kernel_last_iterations(const void* h) { return " << cls << "_last_iterations((const " << cls << "_handle*)h); }\n"
                  "void   ohq_kernel_reset_status(void* h) { " << cls << "_reset_status((" << cls << "_handle*)h); }\n"
                  "/* System::FinalizeOutputs uniformizes ObservedOutputs with dt0 before the\n"
                  "   objective is computed (Objective_Function.cpp:133); a host scoring the\n"
                  "   kernel must do the same or it compares on a different grid. */\n"
                  "void   ohq_kernel_uniformize_observations(void* h) { ((" << cls << "_handle*)h)->m.uniformizeObservations(); }\n"
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
