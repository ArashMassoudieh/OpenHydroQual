/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


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
    bool GetFromJsonDoc(const QJsonDocument& json_doc);
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
    QMap<QString, QMap<QString, QString>> &GetParameterPopulateMaps() {return ParameterPopulateMaps;}
    QMap<QString, QString> &GetParameterPopulateMaps(const QString &parametermap) {return ParameterPopulateMaps[parametermap];}
    QMap<QString, Connector>& GetConnectors() { return Connectors; }
    QStringList Script();
    Wizard_Entity* FindEntity(QString name);
    QStringList CheckParameters();
    bool AssignParameterValues();
    bool AssignParameterValues(const QJsonObject &jsonObject);
    QString Url();
    void SetWorkingFolder(const QString &wrkingfolder) {workingfolder = wrkingfolder;}
    QString WorkingFolder() {return workingfolder;}
private:
    QMap<QString, BlockArray> BlockArrays;
    QMap<QString, SingleBlock> SingleBlocks;
    QMap<QString, WizardParameter> WizardParameters;
    QMap<QString, WizardParameterGroup> WizardParameterGroups;
    QMap<QString, Wizard_Entity> Entities;
    QMap<QString, SetVal_Entity> SetValEntities;
    QMap<QString, Connector> Connectors;
    QMap<QString, QMap<QString, QString>> ParameterPopulateMaps;
    QString iconfilename;
    QString diagramfilename; 
    QString wizardname;
    QString description;
    QStringList addedtemplates;
    QString url;
    QString workingfolder;

};

