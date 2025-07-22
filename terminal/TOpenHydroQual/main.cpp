/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


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
    system->GetOutputs().write(system->GetWorkingFolder() + system->OutputFileName());
    return 0;

}
