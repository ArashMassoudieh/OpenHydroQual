#include "System.h"
#include "Script.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc < 2) {
        cout << "Usage: OHQLibTest <input_file>" << endl;
        return 1;
    }

    cout << "Input file: " << argv[1] << endl;

    System *system = new System();

    string defaulttemppath = qApp->applicationDirPath().toStdString() + "/../../../resources/";

    cout << "Default Template path = " << defaulttemppath << "\n";

    system->SetDefaultTemplatePath(defaulttemppath);
    system->SetWorkingFolder(
        QFileInfo(QString::fromStdString(argv[1]))
        .canonicalPath().toStdString() + "/");


    string settingfilename  = qApp->applicationDirPath().toStdString() + "/../../../resources/settings.json";

    Script scr(argv[1], system);
    cout << "Executing script ..." << endl;
    system->CreateFromScript(scr, settingfilename);
    system->SetSilent(false);

    cout << "Solving ..." << endl;
    system->Solve();

    cout << "Writing outputs in '"
         << system->GetWorkingFolder() + system->OutputFileName() << "'" << endl;
    system->GetOutputs().write(
        system->GetWorkingFolder() + system->OutputFileName());

    delete system;
    return 0;
}
