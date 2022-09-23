#pragma once
#include <qstring.h>
#include "MajorBlock.h" 
#include "wizardparameter.h"
#include "wizardparametergroup.h"
#include "Wizard_Entity.h"
#include <QMap>

#ifdef windows_version
#define wizardsfolder qApp->applicationDirPath().toStdString()+"/../../resources/Wizard_Scripts/"
#endif

#ifdef ubuntu_version
#define wizardsfolder qApp->applicationDirPath().toStdString()+"/../../resources/Wizard_Scripts/"
#endif

#ifdef mac_version
#define wizardsfolder "/Users/arash/Projects/OpenHydroQual/resources/Wizard_Scripts/"
#endif

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
    QStringList Script();
private:
	QMap<QString, MajorBlock> MajorBlocks;
    QMap<QString, WizardParameter> WizardParameters;
    QMap<QString, WizardParameterGroup> WizardParameterGroups;
    QMap<QString, Wizard_Entity> Entities;
    QString iconfilename;
    QString wizardname;
    QString description;
    QStringList addedtemplates;

};

