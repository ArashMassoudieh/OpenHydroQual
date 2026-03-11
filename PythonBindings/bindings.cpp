#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "System.h"
#include "Script.h"
#include "Block.h"
#include "Link.h"
#include "Parameter.h"
#include <QCoreApplication>

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

    // --- System ---
    py::class_<System>(m, "System")
        .def(py::init<>())

        // Setup
        .def("set_default_template_path", &System::SetDefaultTemplatePath)
        .def("set_working_folder",        &System::SetWorkingFolder)
        .def("set_silent",                &System::SetSilent)

        // Load from script
        .def("create_from_script", [](System& s,
                                      const std::string& scriptfile,
                                      const std::string& settingsfile) {
            Script scr(scriptfile, &s);
            return s.CreateFromScript(scr, settingsfile);
        })

        // Entity addition
        .def("add_block",       [](System& s, Block& b)  { return s.AddBlock(b); })
        .def("add_link",        [](System& s, Link& l,
                                   const std::string& src,
                                   const std::string& dst) {
            return s.AddLink(l, src, dst);
        })

        // Entity access
        .def("block_names",     &System::GetAllBlockNames)
        .def("link_names",      &System::GetAllLinkNames)
        .def("block_count",     &System::BlockCount)
        .def("link_count",      &System::LinksCount)

        // Parameters
        .def("set_parameter_value", 
             py::overload_cast<const std::string&, const double&>
             (&System::SetParameterValue))
        .def("get_parameter", 
             py::overload_cast<const std::string&>
             (&System::GetParameter),
             py::return_value_policy::reference)
        .def("append_parameter",
             py::overload_cast<const std::string&, const double&,
                               const double&, const std::string&>
             (&System::AppendParameter))
        .def("apply_parameters", &System::ApplyParameters)

        // Objective functions
        .def("get_objective_function_value", &System::GetObjectiveFunctionValue)
        .def("append_objective_function",
             py::overload_cast<const std::string&, const std::string&,
                               const Expression&, double>
             (&System::AppendObjectiveFunction))

        // Solve
        .def("solve",           &System::Solve)

        // Outputs
        .def("write_outputs",   &System::WriteOutPuts)
        .def("output_filename", &System::OutputFileName)
        .def("working_folder",  &System::GetWorkingFolder)

        // RL-specific
        .def("update_objective_functions", &System::UpdateObjectiveFunctions)
        .def("get_all_block_types",        &System::GetAllBlockTypes)
        .def("get_all_link_types",         &System::GetAllLinkTypes);
}
