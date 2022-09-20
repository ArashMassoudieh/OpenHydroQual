#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "Wizard_Argument.h"

class Wizard_Argument
{
public:
    Wizard_Argument();
    Wizard_Argument(const QString &exp);
    Wizard_Argument(const Wizard_Argument& WA);
    Wizard_Argument& operator=(const Wizard_Argument& WA);
    
private:
    

};
