#pragma once
#include <qstring.h>
#include "MajorBlock.h" 
#include "wizardparameter.h"
#include "wizardparametergroup.h"
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
    QMap<QString, MajorBlock> &GetMajorBlocks() {return MajorBlocks;}
    QMap<QString, WizardParameter> &GetWizardParameters() {return WizardParameters;}
    QMap<QString, WizardParameterGroup> &GetWizardParameterGroups() {return WizardParameterGroups;}
private:
	QMap<QString, MajorBlock> MajorBlocks;
    QMap<QString, WizardParameter> WizardParameters;
    QMap<QString, WizardParameterGroup> WizardParameterGroups;
    QString iconfilename;
    QString wizardname;
    QString description;
    QStringList addedtemplates;

};

