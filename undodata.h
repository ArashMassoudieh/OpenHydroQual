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
