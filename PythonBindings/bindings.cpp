#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "System.h"
#include "Script.h"
#include "Block.h"
#include "Link.h"
#include "Parameter.h"
#include "Objective_Function.h"
#include "Source.h"
#include <QCoreApplication>
#include <QJsonDocument>

namespace py = pybind11;

static void ensure_qapp() {
    static int argc = 1;
    static char name[] = "openhydroqual_py";
    static char* argv[] = {name};
    if (!QCoreApplication::instance())
        new QCoreApplication(argc, argv);
}

PYBIND11_MODULE(openhydroqual_py, m) {
    m.doc() = "OpenHydroQual Python bindings";
    ensure_qapp();

    py::class_<System>(m, "System")
        .def(py::init<>())

        // --- Paths ---
        .def("set_default_template_path", &System::SetDefaultTemplatePath)
        .def("set_working_folder",        &System::SetWorkingFolder)
        .def("set_silent",                &System::SetSilent)
        .def("working_folder",            &System::GetWorkingFolder)
        .def("output_filename",           &System::OutputFileName)

        // --- Template loading ---
        .def("load_template", [](System& s, const std::string& filename) {
            if (s.GetQuanTemplate(filename)) return true;
            std::string fullpath = s.DefaultTemplatePath()
                                   + aquiutils::GetOnlyFileName(filename);
            if (s.GetQuanTemplate(fullpath)) return true;
            std::cerr << "Template not found: " << filename << "\n";
            return false;
        }, py::arg("filename"))

        .def("add_template", [](System& s, const std::string& filename) {
            if (s.AppendQuanTemplate(filename)) return true;
            // Use GetWorkingFolder's sibling — the default template path
            // DefaultTemplatePath() has a side effect, call it carefully
            std::string templatepath = s.DefaultTemplatePath();
            std::string fullpath = templatepath + aquiutils::GetOnlyFileName(filename);
            if (s.AppendQuanTemplate(fullpath)) return true;
            // Also try just the filename directly under the template path
            std::string justname = aquiutils::GetOnlyFileName(filename);
            std::cerr << "Template not found: " << fullpath << "\n";
            return false;
        }, py::arg("filename"))

        // --- System settings (equivalent to setvalue; object=system) ---
        .def("set_system_value", [](System& s,
                                    const std::string& quantity,
                                    const std::string& value) {
            return s.SetSystemSettingsObjectProperties(quantity, value, false);
        }, py::arg("quantity"), py::arg("value"))

        // --- Load from script file ---
        .def("create_from_script", [](System& s,
                                      const std::string& scriptfile,
                                      const std::string& settingsfile) {
            Script scr(scriptfile, &s);
            return s.CreateFromScript(scr, settingsfile);
        }, py::arg("scriptfile"), py::arg("settingsfile"))

        // --- Block creation ---
        .def("create_block", [](System& s,
                                const std::string& name,
                                const std::string& type,
                                const std::map<std::string, std::string>& properties) {
            if (s.GetModel(type) == nullptr) {
                std::cerr << "Type '" << type << "' does not exist in MetaModel\n";
                return false;
            }
            Block b;
            b.SetName(name);
            b.SetType(type);
            s.AddBlock(b);
            Block* stored = s.block(name);
            stored->SetProperty("name", name, true, false);
            if (!stored) {
                std::cerr << "Block '" << name << "' not found after AddBlock\n";
                return false;
            }
            for (auto& kv : properties)
                stored->SetProperty(kv.first, kv.second, true, false);
            return true;
        }, py::arg("name"), py::arg("type"),
             py::arg("properties") = std::map<std::string, std::string>())

        .def("create_link", [](System& s,
                               const std::string& name,
                               const std::string& type,
                               const std::string& from,
                               const std::string& to,
                               const std::map<std::string, std::string>& properties) {
            if (s.GetModel(type) == nullptr) {
                std::cerr << "Type '" << type << "' does not exist in MetaModel\n";
                return false;
            }
            Link l;
            l.SetName(name);
            l.SetType(type);
            if (!s.AddLink(l, from, to)) {
                std::cerr << "AddLink failed for '" << name << "' from '" << from << "' to '" << to << "'\n";
                return false;
            }
            Link* stored = s.link(name);
            stored->SetProperty("name", name, true, false);
            if (!stored) {
                std::cerr << "Link '" << name << "' not found after AddLink\n";
                return false;
            }
            for (auto& kv : properties)
                stored->SetProperty(kv.first, kv.second, true, false);
            return true;
        }, py::arg("name"), py::arg("type"),
             py::arg("from_block"), py::arg("to_block"),
             py::arg("properties") = std::map<std::string, std::string>())

        .def("get_link_json", [](System& s, const std::string& name) -> std::string {
            Link* l = s.link(name);
            if (!l) {
                std::cerr << "Link '" << name << "' not found\n";
                return "{}";
            }
            QJsonObject obj = l->toJson(true, false);
            QJsonDocument doc(obj);
            return doc.toJson(QJsonDocument::Indented).toStdString();
        })

        .def("get_block_json", [](System& s, const std::string& name) -> std::string {
            Block* bl = s.block(name);
            if (!bl) {
                std::cerr << "Block '" << name << "' not found\n";
                return "{}";
            }
            QJsonObject obj = bl->toJson(true, false);
            QJsonDocument doc(obj);
            return doc.toJson(QJsonDocument::Indented).toStdString();
        })

        .def("set_link_property", [](System& s,
                                     const std::string& name,
                                     const std::string& key,
                                     const std::string& value) {
            if (!s.link(name)) return false;
            return s.link(name)->SetProperty(key, value, true, false);
        }, py::arg("name"), py::arg("key"), py::arg("value"))

        // --- Entity name listing ---
        .def("block_names",  &System::GetAllBlockNames)
        .def("link_names",   &System::GetAllLinkNames)
        .def("block_count",  &System::BlockCount)
        .def("link_count",   &System::LinksCount)

        // --- Available types from MetaModel ---
        .def("block_types",  &System::GetAllBlockTypes)
        .def("link_types",   &System::GetAllLinkTypes)

        // --- Parameters ---
        .def("append_parameter", [](System& s, const std::string& name,
                                    double low, double high,
                                    const std::string& prior = "normal") {
            s.AppendParameter(name, low, high);
            Parameter* p = s.parameter(name);
            if (!p) return false;
            p->SetProperty("low",  std::to_string(low),  false);
            p->SetProperty("high", std::to_string(high), false);
            p->SetProperty("prior_distribution", prior, false);
            return true;
        }, py::arg("name"), py::arg("low"), py::arg("high"),
             py::arg("prior_distribution") = "normal")
        .def("set_as_parameter", [](System& s, const std::string& obj,
                                    const std::string& quantity,
                                    const std::string& param) {
            s.SetAsParameter(obj, quantity, param);
            if (s.object(obj) && s.object(obj)->HasQuantity(quantity))
                s.object(obj)->Variable(quantity)->SetParameterAssignedTo(param);
            return true;
        })
        .def("set_parameter_value",
             py::overload_cast<const std::string&, const double&>
             (&System::SetParameterValue),
             py::arg("name"), py::arg("value"))
        .def("apply_parameters", &System::ApplyParameters)

        // --- Objective functions ---
        .def("get_objective_function_value", &System::GetObjectiveFunctionValue)

        // --- Finalize after manual build ---
        .def("finalize", [](System& s) {
            s.SetAllParents();
            s.SetQuanPointers();
            s.CalcAllInitialValues();
            s.InitiatePrecalculatedFunctions();
        })

        // --- Solve ---
        .def("solve", &System::Solve,
             py::arg("apply_parameters")  = true,
             py::arg("uniformize_output") = true)

        // --- Save model to .ohq script file ---
        .def("save_to_script", [](System& s,
                                  const std::string& filename,
                                  const std::string& templatefilename = "",
                                  const std::vector<std::string>& addedtemplates = {}) {
            return s.SavetoScriptFile(filename, templatefilename, addedtemplates);
        },
             py::arg("filename"),
             py::arg("templatefilename") = "",
             py::arg("addedtemplates")   = std::vector<std::string>())

        // --- Save model to .json ---
        .def("save_to_json", [](System& s,
                                const std::string& filename,
                                const std::vector<std::string>& addedtemplates = {},
                                bool allvariables   = false,
                                bool calculatevalue = false) {
            return s.SavetoJson(filename, addedtemplates, allvariables, calculatevalue);
        },
             py::arg("filename"),
             py::arg("addedtemplates")  = std::vector<std::string>(),
             py::arg("allvariables")    = false,
             py::arg("calculatevalue")  = false)

        // --- Outputs ---
        .def("write_outputs", &System::WriteOutPuts)

        // --- Block properties (counterpart of set_link_property) ---
        .def("set_block_property", [](System& s,
                                      const std::string& name,
                                      const std::string& key,
                                      const std::string& value) {
            if (!s.block(name)) return false;
            return s.block(name)->SetProperty(key, value, true, false);
        }, py::arg("name"), py::arg("key"), py::arg("value"))

        // --- Interval-by-interval simulation (closed-loop control) ---
        .def("begin_stepping", &System::BeginStepping,
             py::arg("apply_parameters") = false,
             "Initialize the solver exactly as solve() does, without running.")
        .def("advance_to", &System::AdvanceTo, py::arg("t"),
             "Continue the simulation to time t (clamped to the end time) without "
             "re-initializing. Returns False if the solve failed.")
        .def("end_stepping", &System::EndStepping, py::arg("uniformize_output") = true,
             "Finalize the outputs of a stepped run.")
        .def("is_stepping", &System::IsStepping)
        .def("current_time", [](System& s) { return s.GetTime(); })
        .def("start_time", [](System& s) { return s.tstart(); })
        .def("end_time", [](System& s) { return s.tend(); })
        .def("solution_failed", &System::GetSolutionFailed)

        // --- State variables and quantities of blocks, links and sources ---
        .def("get_value", [](System& s, const std::string& object, const std::string& quantity) {
            Object* o = s.object(object);
            if (!o) throw py::key_error("object not found: " + object);
            if (!o->Variable(quantity)) throw py::key_error(object + " has no quantity " + quantity);
            return o->GetVal(quantity, Expression::timing::present);
        }, py::arg("object"), py::arg("quantity"),
           "Current value of a quantity (e.g. a state such as Storage). A constituent "
           "concentration is addressed as 'Constituent:concentration'.")
        .def("get_values", [](System& s, const std::vector<std::pair<std::string, std::string>>& items) {
            std::vector<double> out;
            out.reserve(items.size());
            for (const auto& it : items) {
                Object* o = s.object(it.first);
                if (!o || !o->Variable(it.second))
                    throw py::key_error("not found: " + it.first + ":" + it.second);
                out.push_back(o->GetVal(it.second, Expression::timing::present));
            }
            return out;
        }, py::arg("items"), "Current values of a list of (object, quantity) pairs.")
        .def("set_value", [](System& s, const std::string& object, const std::string& quantity,
                             double value) {
            Object* o = s.object(object);
            if (!o || !o->Variable(quantity))
                throw py::key_error("not found: " + object + ":" + quantity);
            return o->SetVal(quantity, value, Expression::timing::both);
        }, py::arg("object"), py::arg("quantity"), py::arg("value"),
           "Set the current value of a quantity (present and past), e.g. a state.")

        // --- Forcing time series ---
        .def("set_timeseries", [](System& s, const std::string& object, const std::string& property,
                                  const std::vector<double>& t, const std::vector<double>& v) {
            Object* o = s.object(object);
            if (!o || !o->Variable(property))
                throw py::key_error("not found: " + object + ":" + property);
            if (t.size() != v.size() || t.empty())
                throw py::value_error("t and v must be non-empty and of equal length");
            TimeSeries<timeseriesprecision> ts(t, v);
            ts.assign_D();
            return o->Variable(property)->SetTimeSeries(ts);
        }, py::arg("object"), py::arg("property"), py::arg("t"), py::arg("v"),
           "Replace the time series of a time-series property (e.g. a controlled forcing).")

        // --- Objective functions ---
        .def("objective_names", [](System& s) {
            std::vector<std::string> names;
            for (unsigned int i = 0; i < s.ObjectiveFunctionsCount(); i++)
                names.push_back(s.ObjectiveFunctions()[int(i)]->GetName());
            return names;
        })
        .def("objective_method", [](System& s, const std::string& name) {
            Objective_Function* f = s.ObjectiveFunctions()[name];
            if (!f) throw py::key_error("objective function not found: " + name);
            return f->Variable("method") ? f->Variable("method")->GetProperty() : std::string("integrate");
        }, py::arg("name"))
        .def("objective_weight", [](System& s, const std::string& name) {
            Objective_Function* f = s.ObjectiveFunctions()[name];
            if (!f) throw py::key_error("objective function not found: " + name);
            return f->Weight();
        }, py::arg("name"))
        .def("objective_integral", [](System& s, const std::string& name) {
            Objective_Function* f = s.ObjectiveFunctions()[name];
            if (!f) throw py::key_error("objective function not found: " + name);
            return f->RunningIntegral();
        }, py::arg("name"),
           "Integral of the objective function's expression from the start of the "
           "run to the current time (value at the end of each step times its length).")
        .def("objective_current_value", [](System& s, const std::string& name) {
            Objective_Function* f = s.ObjectiveFunctions()[name];
            if (!f) throw py::key_error("objective function not found: " + name);
            return f->Value();
        }, py::arg("name"), "Value of the expression at the last accepted step.")
        .def("objective_value", [](System& s, const std::string& name) {
            Objective_Function* f = s.ObjectiveFunctions()[name];
            if (!f) throw py::key_error("objective function not found: " + name);
            return f->GetObjective();
        }, py::arg("name"),
           "The objective function as OHQ evaluates it (its method over the stored "
           "series), independent of the parameter-estimation mode.");


}
