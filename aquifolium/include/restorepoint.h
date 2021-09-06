#ifndef RESTOREPOINT_H
#define RESTOREPOINT_H

#include "System.h"

class RestorePoint
{
public:
    RestorePoint();
    RestorePoint(System *sys);
    System *GetSystem();
    double t;
    double dt;
    unsigned int used_counter=0;
private:
    System CopiedSystem;

};

#endif // RESTOREPOINT_H
