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
