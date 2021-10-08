#include "undodata.h"
#include "mainwindow.h"

UndoData::UndoData(MainWindow *_parent)
{
    parent = _parent;
}

void UndoData::AppendtoLast(const System *system)
{
    Systems.append(*system);
    active = Systems.count() - 1;
    parent->InactivateUndo(false);
}
void UndoData::EliminateFrom(unsigned int i)
{
    if (Systems.count()>i)
        Systems.remove(i,Systems.count()-i);

}
void UndoData::AppendAfterActive(const System *system)
{
    EliminateFrom(active+1);
    AppendtoLast(system);
    active=Systems.count()-1;
}

System *UndoData::Undo()
{
    if (!CanUndo()) return nullptr;
    active--;
    if (active==0) parent->InactivateUndo();


    return &Systems[active];

}
System *UndoData::Redo()
{
    if (active==Systems.size()-1) return nullptr;
    active++;
    return &Systems[active];
}

bool UndoData::CanUndo()
{
    if (active==0) return false; else return true;
}
bool UndoData::CanRedo()
{
    if (active==Systems.size()-1) return false; else return true;

}

