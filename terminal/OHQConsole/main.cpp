#include "System.h"
#include "Script.h"

int main(int argc, char *argv[])
{

    if (argc<2)
    {
        cout<<"The name of the input file must be provided\n";
        return 0;
    }
    cout<<"Input file: "<<argv[1]<<endl;
    System *system;
    cout<<"Reading script ..."<<endl;
    if (argv[1]=="")
    {
        cout<<"The name of the input file must be provided\n";
        return 0;
    }
    Script scr(argv[1]);
    scr.SetSystem(system);
    cout<<"Executing script ..."<<endl;
    system = scr.CreateSystem();
    system->SetSilent(false);
    cout<<"Solving ..."<<endl;
    system->Solve();
    cout<<"Writing outputs ..."<<endl;
    system->GetOutputs().writetofile(system->OutputFileName());
    return 0;

}
