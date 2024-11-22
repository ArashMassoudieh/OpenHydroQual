#pragma once
#include <qstring.h>
#include "WizBlockArray.h"
#include "WizSingleBlock.h"
#include "wizardparameter.h"
#include "wizardparametergroup.h"
#include "Wizard_Entity.h"
#include "SetValEntity.h"
#include "WizConnector.h"
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
    void SetAllParents();
	WizardScript& operator=(const WizardScript& WS);
    QIcon Icon();
	QString Name();
	QString Description(); 
    QString DiagramFileName() {return diagramfilename;}
    QMap<QString, BlockArray> &GetBlockArrays() {return BlockArrays;}
    QMap<QString, SingleBlock> &GetSingleBlocks() {return SingleBlocks;}
    QMap<QString, WizardParameter> &GetWizardParameters() {return WizardParameters;}
    QMap<QString, WizardParameterGroup> &GetWizardParameterGroups() {return WizardParameterGroups;}
    QMap<QString, Connector>& GetConnectors() { return Connectors; }
    QStringList Script();
    Wizard_Entity* FindEntity(QString name);
    QStringList CheckParameters();
    bool AssignParameterValues();
    QString Url();
private:
    QMap<QString, BlockArray> BlockArrays;
    QMap<QString, SingleBlock> SingleBlocks;
    QMap<QString, WizardParameter> WizardParameters;
    QMap<QString, WizardParameterGroup> WizardParameterGroups;
    QMap<QString, Wizard_Entity> Entities;
    QMap<QString, SetVal_Entity> SetValEntities;
    QMap<QString, Connector> Connectors;
    QString iconfilename;
    QString diagramfilename; 
    QString wizardname;
    QString description;
    QStringList addedtemplates;
    QString url;

};

