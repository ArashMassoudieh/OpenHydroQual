#ifndef UNDODATA_H
#define UNDODATA_H

#include "System.h"
#include <QVector>

class UndoData
{
public:
    UndoData();
    int active=0;
    QVector<System> Systems;
    void AppendtoLast(const System *system);
    void EliminateFrom(unsigned int i);
    void AppendAfterActive(const System *system);

};

#endif // UNDODATA_H
