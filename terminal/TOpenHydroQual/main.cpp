#include "System.h"
#include "Script.h"

int main(int argc, char *argv[])
{
    cout<<"Input file: "<<argv[1]<<endl;
    System system;
    cout<<"Reading script ..."<<endl;
    Script scr(argv[1],&system);
    cout<<"Solving ..."<<endl;
    system.Solve();

}
