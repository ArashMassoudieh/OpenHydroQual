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
#include <fstream>

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
    // Solver settings parsed from the .ohq only reach the solver through this
    // call; without it they keep their default-constructed values and Solve()
    // aborts in Armadillo while sizing its work vectors.
    system->SetSystemSettings();
    cout<<"Solving ..."<<endl;
    system->Solve();
    cout<<"Writing outputs ..."<<endl;
    // TimeSeriesSet's write method is write(filename, delimiter), not
    // writetofile() -- API drift from when this file was last touched.
    system->GetOutputs().write(system->OutputFileName());
    // fit_measures.txt, in the GUI's format: one row per observation (MSE, R2, NSE),
    // blank when the observation has no observed data.
    if (system->ObservationsCount() > 0)
    {
        std::ofstream fitfile(system->GetWorkingFolder() + "fit_measures.txt");
        fitfile << "Observation\tMSE\tR2\tNSE\n";
        for (unsigned int i = 0; i < system->ObservationsCount(); i++)
        {
            Observation *ob = system->observation(i);
            TimeSeries<timeseriesprecision> *od = ob->Variable("observed_data")->GetTimeSeries();
            fitfile << ob->GetName();
            if (od != nullptr && od->size() > 0)
            {
                ob->CalcMisfit();
                if (ob->fit_measures.size() == 3)
                    for (unsigned int j = 0; j < 3; j++) fitfile << "\t" << ob->fit_measures[j];
                else
                    fitfile << "\t\t\t";
            }
            else
                fitfile << "\t\t\t";
            fitfile << "\n";
        }
        cout << "Fit measures written to fit_measures.txt" << endl;
    }
    return 0;

}
