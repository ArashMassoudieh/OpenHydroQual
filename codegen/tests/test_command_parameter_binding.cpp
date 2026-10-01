/* Regression test for setasparameter command consistency. */
#include "Script.h"
#include "System.h"

#include <QCoreApplication>
#include <QStringList>
#include <cstdio>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    int failures = 0;

    System invalidSystem;
    invalidSystem.SetDefaultTemplatePath(std::string(OHQ_TEST_RESOURCES) + "/");
    Script invalidScript;
    const QStringList invalidCommands = {
        "create block; type=Catchment, name=Cell",
        "setasparameter; object=Cell, quantity=Runoff_coeff, parametername=missing"
    };
    invalidScript.CreateSystemFromQStringList(invalidCommands, &invalidSystem);
    Object* invalidCell = invalidSystem.object("Cell");
    const bool invalidHasQuantity = invalidCell && invalidCell->HasQuantity("Runoff_coeff");
    const std::string invalidAssignment = invalidHasQuantity
        ? invalidCell->Variable("Runoff_coeff")->GetParameterAssignedTo() : "<unavailable>";
    const bool invalidClean = invalidCell
        && invalidHasQuantity
        && invalidAssignment.empty()
        && !invalidScript.Errors().empty();
    std::printf("[%s] missing parameter leaves quantity unassigned "
                "(cell=%d quantity=%d assigned='%s' errors=%zu)\n",
                invalidClean ? "PASS" : "FAIL", invalidCell != nullptr,
                invalidHasQuantity, invalidAssignment.c_str(),
                invalidScript.Errors().size());
    if (!invalidClean) ++failures;

    System validSystem;
    validSystem.SetDefaultTemplatePath(std::string(OHQ_TEST_RESOURCES) + "/");
    Script validScript;
    const QStringList validCommands = {
        "create block; type=Catchment, name=Cell",
        "create parameter; type=Parameter, high=2, low=1, name=runoff_scale",
        "setasparameter; object=Cell, quantity=Runoff_coeff, parametername=runoff_scale"
    };
    validScript.CreateSystemFromQStringList(validCommands, &validSystem);
    Object* validCell = validSystem.object("Cell");
    Parameter* parameter = validSystem.GetParameter("runoff_scale");
    const bool validHasQuantity = validCell && validCell->HasQuantity("Runoff_coeff");
    const std::string validAssignment = validHasQuantity
        ? validCell->Variable("Runoff_coeff")->GetParameterAssignedTo() : "<unavailable>";
    const bool validBound = validCell && validHasQuantity && parameter
        && validAssignment == "runoff_scale"
        && parameter->GetLocations().size() == 1
        && parameter->GetLocations()[0] == "Cell"
        && parameter->GetQuans().size() == 1
        && parameter->GetQuans()[0] == "Runoff_coeff";
    std::printf("[%s] valid parameter records both sides of binding "
                "(cell=%d quantity=%d parameter=%d assigned='%s' locations=%zu quans=%zu errors=%zu)\n",
                validBound ? "PASS" : "FAIL", validCell != nullptr,
                validHasQuantity, parameter != nullptr, validAssignment.c_str(),
                parameter ? parameter->GetLocations().size() : 0,
                parameter ? parameter->GetQuans().size() : 0,
                validScript.Errors().size());
    if (!validBound) ++failures;

    return failures ? 1 : 0;
}
