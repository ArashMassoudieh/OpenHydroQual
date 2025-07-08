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
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#include "mainwindow.h"
#include <QApplication>
#include "TimeSeries.h"

//#undef ARMA_USE_OPENMP

int main(int argc, char *argv[])
{
    /*omp_set_nested(0);          // Disable nested parallelism
    omp_set_dynamic(0);         // Optional: disable dynamic thread adjustment
    std::cout << "OMP nested: " << omp_get_nested() << std::endl;
    std::cout << "OMP max threads: " << omp_get_max_threads() << std::endl;
    */
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(QString::fromStdString(RESOURCE_DIRECTORY)+"/icons/Aquifolium.png"));

    MainWindow w;
    w.show();

    return a.exec();
}
