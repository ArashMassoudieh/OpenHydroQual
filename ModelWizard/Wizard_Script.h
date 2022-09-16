#pragma once
#include <qstring.h>
#include "MajorBlock.h" 
#include "wizardparameter.h"
#include <QMap>

#define wizardsfolder "../../resources/Wizard_Scripts/"

class WizardScript
{
public: 
	WizardScript(); 
	WizardScript(const QString& filename);
	WizardScript(const WizardScript &WS);
	WizardScript& operator=(const WizardScript& WS);
    QIcon Icon();
	QString Name();
	QString Description(); 
private:
	QMap<QString, MajorBlock> MajorBlocks;
    QMap<QString, WizardParameter> WizardParameters;
    QString iconfilename;
    QString wizardname;
    QString description;
    QStringList addedtemplates;

};

