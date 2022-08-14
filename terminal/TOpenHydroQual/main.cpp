#include "System.h"
#include "Script.h"

int main(int argc, char *argv[])
{
    cout<<"Input file: "<<argv[1]<<endl;
    System *system;
    cout<<"Reading script ..."<<endl;
    Script scr(argv[1]);
    scr.SetSystem(system);
    cout<<"Executing script ..."<<endl;
    system = scr.CreateSystem();
    system->SetSilent(false);
    cout<<"Solving ..."<<endl;
    system->Solve();
    cout<<"Writing outputs ..."<<endl;
    system->GetOutputs().writetofile(system->OutputFileName());


}
