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


#ifndef UNDODATA_H
#define UNDODATA_H

#include "System.h"
#include <QVector>

class MainWindow;

class UndoData
{
public:
    UndoData(MainWindow *_parent=nullptr);
    int active=-1;
    QVector<System> Systems;
    void AppendtoLast(const System *system);
    void EliminateFrom(unsigned int i);
    void AppendAfterActive(const System *system);
    System *Undo();
    System *Redo();
    bool CanUndo();
    bool CanRedo();
    void SetActiveSystem(const System *system);
    MainWindow *parent;
    unsigned int capacity=20;

};

#endif // UNDODATA_H
