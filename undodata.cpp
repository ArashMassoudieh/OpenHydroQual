#include "undodata.h"

UndoData::UndoData()
{

}

void UndoData::AppendtoLast(const System *system)
{
    Systems.append(*system);
    active = Systems.count() - 1;
}
void UndoData::EliminateFrom(unsigned int i)
{
    if (Systems.count()>i)
        Systems.remove(i,Systems.count());
}
void UndoData::AppendAfterActive(const System *system)
{
    EliminateFrom(active+1);
    AppendtoLast(system);
    active=Systems.count()-1;
}
