#include "undodata.h"
#include "mainwindow.h"

UndoData::UndoData(MainWindow *_parent)
{
    Systems.clear();
    parent = _parent;
}

void UndoData::AppendtoLast(const System *system)
{
    EliminateFrom(active+1);
    Systems.append(*system);
    active = Systems.count() - 1;
    parent->InactivateUndo(false);
    parent->InactivateRedo();
    if (Systems.size()>capacity)
    {
        Systems.remove(0);
        active--;
    }
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
    if (active<Systems.size()-1) parent->InactivateRedo(false);

    return &Systems[active];

}
System *UndoData::Redo()
{
    if (!CanRedo()) return nullptr;
    active++;
    if (active==Systems.size()-1) parent->InactivateRedo();
    if (active>0) parent->InactivateUndo(false);
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

void UndoData::SetActiveSystem(const System *system)
{
    Systems[active] = *system;
}

