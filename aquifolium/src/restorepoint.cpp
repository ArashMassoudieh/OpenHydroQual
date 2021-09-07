#include "restorepoint.h"

RestorePoint::RestorePoint()
{

}

RestorePoint::RestorePoint(System *sys)
{
    CopiedSystem = *sys;
}

System* RestorePoint::GetSystem()
{
    return &CopiedSystem;
}
