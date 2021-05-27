#include <iostream>
#include "Expression.h"
#include "string"
#include "System.h"
#include <iostream>
#include "GA.h"
#include "Script.h"





using namespace std;

int main()
{
	cout<<"Input file name:";
    string filename;
    cin>>filename;
    Script scr(filename);

    CGA<System> TempObj;
    System *S = scr.CreateSystem(); //this creates the system based on the script
    S->errorhandler.Write(S->OutputPath() + "errors.txt");
    S->GetOutputs().writetofile(S->OutputPath() +"output.txt");
	cout<<S->GetObjectiveFunctionValue()<<endl;
	//CGA <System> GA("GA_info.txt",*S);
    //GA.optimize();

    return 0;
}
