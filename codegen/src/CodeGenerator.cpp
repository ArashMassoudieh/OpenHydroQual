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

#include "System.h"
#include "Block.h"
#include "Link.h"
#include "Object.h"
#include "Quan.h"
#include "QuanSet.h"
#include "Expression.h"

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

    // Reject unsupported quantity types up front (loud failure).
    for (unsigned i = 0; i < nB + nL; ++i) {
        Object* o = (i < nB) ? (Object*)system.block(i) : (Object*)system.link(i - nB);
        QuanSet* qs = o->GetVars();
        for (auto it = qs->begin(); it != qs->end(); ++it) {
            if (it->second.GetType() == Quan::_type::rule)
                throw std::runtime_error("CodeGenerator v1: rule quantity '" +
                    it->second.GetName() + "' in '" + o->GetName() + "' not yet supported");
        }
    }

    // A resolver factory bound to the current object / time symbol.
    auto makeCtx = [&](Object* cur, bool isLink, const std::string& timeVar) {
        EmitContext ctx;
        ctx.timeVar = timeVar;
        auto target = [&, cur, isLink](Loc loc) -> Object* {
            if (!isLink || loc == Loc::self) return cur;
            if (loc == Loc::source)      return (Object*)system.block(cur->s_Block_No());
            if (loc == Loc::destination) return (Object*)system.block(cur->e_Block_No());
            return cur;
        };
        ctx.resolveValue = [&, target](const std::string& name, Loc loc) -> std::string {
            Object* t = target(loc);
            Quan* q = t->Variable(name);
            if (!q) return sanitize(name) + "_ /*UNRESOLVED*/";
            if (q->GetType() == Quan::_type::balance)
                return "eff[" + stateEnum(t->GetName(), name) + "]";
            if (isSeriesType(q->GetType()))
                return tsHandle(t->GetName(), name) + ".interpol(" + ctx.timeVar + ")";
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
            const QuantityInfo* qi = info(o, qn);
            if (!qi) continue;
            if (q->GetType() == Quan::_type::balance) continue;  // state, not emitted here
            if (isSeriesType(q->GetType())) {
                // declare a series input + setter once, and bake the model's
                // loaded data points inline (so the generated lib is standalone).
                const std::string h = tsHandle(o->GetName(), qn);
                seriesDecls  << "    ohq::TimeSeries " << h << ";\n";
                seriesSetters << "    void set_" << h << "(const ohq::TimeSeries& ts) { "
                              << h << " = ts; }\n";
                TimeSeries<timeseriesprecision>* ts = q->GetTimeSeries();
                if (ts && ts->size() > 0 && ts->size() <= 50000) {
                    for (size_t k = 0; k < ts->size(); ++k)
                        initBody << "        " << h << ".push(" << fmt(ts->getTime(k))
                                 << ", " << fmt(ts->getValue(k)) << ");\n";
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
            const QuantityInfo* qi = info(o, q.GetName());
            if (qi && qi->tier == Tier::PerIteration)
                perIter[o->GetName() + "::" + q.GetName()] = {o, isLink, q.GetName()};
        }
    }
    // Kahn topological sort using the analyzer's dependency edges (restricted to
    // the per-iteration set; deps on constants/per-step members are always ready).
    {
        std::map<std::string, int> indeg;
        std::map<std::string, std::vector<std::string>> radj;
        for (auto& kv : perIter) indeg[kv.first] = 0;
        for (auto& kv : perIter) {
            auto ti = tiers.find(kv.first);
            if (ti == tiers.end()) continue;
            for (const std::string& dep : ti->second.dependencies) {
                if (dep == kv.first) continue;
                if (perIter.count(dep)) { indeg[kv.first]++; radj[dep].push_back(kv.first); }
            }
        }
        std::vector<std::string> ready;
        for (auto& kv : indeg) if (kv.second == 0) ready.push_back(kv.first);
        std::vector<std::string> ordered;
        while (!ready.empty()) {
            std::string n = ready.back(); ready.pop_back();
            ordered.push_back(n);
            for (const std::string& m : radj[n]) if (--indeg[m] == 0) ready.push_back(m);
        }
        // any leftover (cycle) appended as-is so output is still complete
        for (auto& kv : perIter) if (std::find(ordered.begin(), ordered.end(), kv.first) == ordered.end())
            ordered.push_back(kv.first);
        for (const std::string& key : ordered) {
            const Node& nd = perIter[key];
            emitQuantity(nd.o, nd.isLink, nd.qn, "t_new", resBody, /*asLocal=*/true);
        }
    }

    // flowRaw[l] : raw link flow (before limiting factors)
    for (unsigned i = 0; i < nL; ++i) {
        Link* l = system.link(i);
        Quan* fq = l->Variable(flowVar);
        std::string flowRef = "0.0";
        if (fq) {
            const QuantityInfo* qi = info(l, flowVar);
            if (fq->GetType() == Quan::_type::expression && qi && qi->tier == Tier::PerIteration)
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
    for (unsigned i = 0; i < nB; ++i) {
        Block* b = system.block(i);
        Quan* bal = b->Variable(stateVar);
        const std::string idx = stateEnum(b->GetName(), stateVar);
        if (bal && bal->calcinivalue()) {
            // initial value from an expression (references folded constants,
            // which buildConstants() has already set before initialValues()).
            ExpressionEmitter em(makeCtx(b, false, "0.0"));
            iniBody << "        s0[" << idx << "] = "
                    << em.translate(bal->InitialValueExpression()) << ";\n";
        } else {
            double iv = bal ? std::atof(bal->GetProperty(true).c_str()) : 0.0;
            iniBody << "        s0[" << idx << "] = " << fmt(iv) << ";\n";
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
    std::ostringstream h;
    h << "// Auto-generated by OpenHydroQual model compiler. Do not edit.\n"
      << "#ifndef OHQ_GEN_" << sanitize(cls) << "_H\n#define OHQ_GEN_" << sanitize(cls) << "_H\n\n"
      << "#include \"ohq_intrinsics.h\"\n#include \"ohq_timeseries.h\"\n#include \"ohq_massbalance.h\"\n\n"
      << "class " << cls << " {\npublic:\n"
      << "    enum State {\n" << stateEnumBody.str() << "        N_STATES = " << nB << "\n    };\n"
      << "    enum { N_LINKS = " << nL << " };\n\n"
      << "    " << cls << "() : solver_(*this) {}\n\n"
      << seriesSetters.str() << "\n"
      << "    void initialize(double tstart = " << fmt(system.tstart()) << ", double dt0 = "
      << fmt(system.dt0()) << ") { buildConstants(); solver_.initialize(tstart, dt0); }\n"
      << "    bool step()              { return solver_.step(); }\n"
      << "    bool runTo(double t_end) { return solver_.runTo(t_end); }\n"
      << "    double time() const      { return solver_.time(); }\n"
      << "    int lastIterations() const { return solver_.lastIterations(); }\n"
      << "    double state(int i) const { return solver_.storage(i); }\n"
      << "    bool limited(int i) const { return solver_.isLimited(i); }\n\n"
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
      << "private:\n"
      << "    void buildConstants() {\n" << initBody.str() << "    }\n"
      << decls.str()
      << seriesDecls.str()
      << "    ohq::MassBalanceSolver<" << cls << "> solver_;\n"
      << "};\n\n#endif\n";

    // write files
    const std::string base = opt.outputDir + "/" + cls;
    std::ofstream fh(base + ".h");
    if (!fh) throw std::runtime_error("CodeGenerator: cannot write " + base + ".h");
    fh << h.str();
    fh.close();
    return true;
}

} // namespace ohqcg
