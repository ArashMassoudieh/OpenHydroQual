#include "System.h"
#include "Script.h"
#include <QCoreApplication>
#include "qfileinfo.h"

 

int main(int argc, char *argv[])
{

    QCoreApplication a(argc,argv);
    if (argc<2)
    {
        cout<<"The name of the input file must be provided\n";
        return 0;
    }
    cout<<"Input file: "<<argv[1]<<endl;
    System *system=new System();
    cout<<"Reading script ..."<<endl;
    if (argv[1]=="")
    {
        cout<<"The name of the input file must be provided\n";
        return 0;
    }
    
    string defaulttemppath = qApp->applicationDirPath().toStdString() + "/../../resources/";
    cout << "Default Template path = " + defaulttemppath +"\n";
    system->SetDefaultTemplatePath(defaulttemppath);
    system->SetWorkingFolder(QFileInfo(QString::fromStdString(argv[1])).canonicalPath().toStdString() + "/");
    string settingfilename = qApp->applicationDirPath().toStdString() + "/../../resources/settings.json";
    Script scr(argv[1],system);
    cout<<"Executing script ..."<<endl;
    system->CreateFromScript(scr,settingfilename);
    system->SetSilent(false);
    cout<<"Solving ..."<<endl;
    system->Solve();
    cout<<"Writing outputs in '"<< system->GetWorkingFolder() + system->OutputFileName() +"'";
    system->GetOutputs().writetofile(system->GetWorkingFolder() + system->OutputFileName());
    return 0;

}
