// Regression test: renaming an object must carry every by-name reference along.
//
// Links hold their blocks directly, so they always followed a rename. But
// observations and objective functions name the object they read ("object"),
// parameters name the objects they drive, and quantities name the parameter
// assigned to them. Renaming a block or a parameter used to leave those on the
// old name: observations read nothing and ApplyParameters reported
// "Location '...' does not exist!". System::RenameObjectReferences fixes them.
//
// The model is Examples/LM_Calibration/03_Pond_Weir/calibration.ohq. The test
// renames the pond, the weir link and one parameter, checks the references,
// then saves, reloads the saved script and checks the references again.
//
// Build against the object files of a console build (all except main.o), e.g.:
//   g++ -std=gnu++17 -fopenmp -Dubuntu_version -DTerminal_version -DQ_JSON_SUPPORT -DARMA_USE_LAPACK \
//       -DARMA_USE_BLAS -DGSL -Iaquifolium/include -Iaquifolium/src -Iaquifolium/include/GA \
//       -Iaquifolium/include/MCMC -Ijsoncpp/include -I. $(pkg-config --cflags Qt6Core) \
//       tests/object_rename_references.cpp $(ls BUILD/*.o | grep -v main.o) \
//       -larmadillo -llapack -lblas -lgsl -lsuperlu $(pkg-config --libs Qt6Core) -o rename_test
// Run from the repository root: ./rename_test
#include "System.h"
#include "Script.h"
#include <cstdio>
#include <string>
#include <unistd.h>

static int failures = 0;

static void check(bool ok, const std::string &what)
{
    std::printf("%s  %s\n", ok ? "ok  " : "FAIL", what.c_str());
    if (!ok)
        failures++;
}

static void rename(System *sys, const std::string &oldname, const std::string &newname)
{
    Object *obj = sys->object(oldname);
    check(obj != nullptr && obj->SetName(newname, true), "rename '" + oldname + "' -> '" + newname + "'");
    sys->RenameObjectReferences(oldname, newname);
}

static bool hasLocation(Parameter *p, const std::string &loc)
{
    if (p == nullptr)
        return false;
    for (const std::string &l : p->GetLocations())
        if (l == loc)
            return true;
    return false;
}

static void checkReferences(System *sys, const std::string &stage)
{
    check(sys->block("Pond A") != nullptr && sys->block("Pond (1)") == nullptr, stage + ": pond renamed");
    Link *weir = sys->link("Weir");
    check(weir != nullptr, stage + ": weir link renamed");
    if (weir != nullptr)
        check(weir->GetConnectedBlock(Expression::loc::source)->GetName() == "Pond A",
              stage + ": weir still starts at the pond");

    for (unsigned int i = 0; i < sys->ObservationsCount(); i++)
    {
        Observation *ob = sys->observation(i);
        const std::string loc = ob->GetLocation();
        if (ob->GetName() == "weir_outflow")
            check(loc == "Weir", stage + ": observation weir_outflow reads 'Weir' (got '" + loc + "')");
        else
            check(loc == "Pond A", stage + ": observation " + ob->GetName() + " reads 'Pond A' (got '" + loc + "')");
    }

    Parameter *alpha = sys->GetParameter("alpha_w");
    check(alpha != nullptr && sys->GetParameter("weir_alpha") == nullptr, stage + ": parameter renamed");
    check(hasLocation(alpha, "Weir"), stage + ": alpha_w drives 'Weir'");
    check(hasLocation(sys->GetParameter("crest_elev"), "Weir"), stage + ": crest_elev drives 'Weir'");
    if (weir != nullptr)
    {
        check(weir->Variable("alpha")->GetParameterAssignedTo() == "alpha_w",
              stage + ": weir alpha assigned to alpha_w (got '" + weir->Variable("alpha")->GetParameterAssignedTo() + "')");
        check(weir->Variable("crest_elevation")->GetParameterAssignedTo() == "crest_elev",
              stage + ": weir crest_elevation still assigned to crest_elev");
    }
}

int main()
{
    const std::string folder = "Examples/LM_Calibration/03_Pond_Weir/";
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) == nullptr || chdir(folder.c_str()) != 0)
    {
        std::printf("Run from the repository root.\n");
        return 2;
    }

    Script scr("calibration.ohq");
    System *sys = scr.CreateSystem();
    check(sys != nullptr && sys->block("Pond (1)") != nullptr, "model loaded");
    if (sys == nullptr)
        return 1;

    rename(sys, "Pond (1)", "Pond A");
    rename(sys, "Pond (1) - fixed_head (2)", "Weir");
    rename(sys, "weir_alpha", "alpha_w");
    checkReferences(sys, "after rename");

    sys->GetErrorHandler()->clear();
    sys->ApplyParameters();
    check(sys->GetErrorHandler()->Count() == 0, "ApplyParameters reports no missing locations");

    const std::string saved = std::string("ohq_rename_test_") + std::to_string(getpid()) + ".ohq";
    check(sys->SavetoScriptFile(saved, "../../../resources/main_components.json"), "saved");
    Script scr2(saved);
    System *reloaded = scr2.CreateSystem();
    check(reloaded != nullptr, "saved model reloads");
    if (reloaded != nullptr)
        checkReferences(reloaded, "after reload");
    std::remove(saved.c_str());

    if (chdir(cwd) != 0)
        return 2;
    std::printf(failures == 0 ? "\nAll checks passed.\n" : "\n%d check(s) failed.\n", failures);
    return failures == 0 ? 0 : 1;
}
