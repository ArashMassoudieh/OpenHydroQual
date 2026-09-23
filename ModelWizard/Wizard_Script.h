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
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <string>

// Locate resources/Wizard_Scripts.
//
// This used to be a hard-coded "<appdir>/../../resources/Wizard_Scripts/",
// which only resolves when the executable sits exactly two levels below the
// repository root -- the bin-Wizard/release layout. Qt Creator's default
// shadow build puts it three levels down (ModelWizard/build/<kit>/), so the
// path landed in ModelWizard/resources, that folder does not exist, and the
// model catalog came up silently empty.
//
// Search order:
//   1. $OHQ_WIZARD_SCRIPTS, if set (deployment and testing override)
//   2. <appdir>/resources/Wizard_Scripts   (deployed / AppImage layout)
//   3. walk up from <appdir> looking for resources/Wizard_Scripts
//   4. the old <appdir>/../../resources/Wizard_Scripts, so a layout that
//      worked before still works even if nothing above matched
inline std::string OHQWizardScriptsFolder()
{
    static std::string cached;
    if (!cached.empty())
        return cached;

    const QString appDir = QCoreApplication::applicationDirPath();

    auto accept = [&](const QString &dir) -> bool {
        if (dir.isEmpty() || !QFileInfo(dir).isDir())
            return false;
        cached = (QDir::cleanPath(dir) + "/").toStdString();
        return true;
    };

    const QByteArray fromEnv = qgetenv("OHQ_WIZARD_SCRIPTS");
    if (!fromEnv.isEmpty() && accept(QString::fromLocal8Bit(fromEnv)))
        return cached;

    if (accept(appDir + "/resources/Wizard_Scripts"))
        return cached;

    QDir up(appDir);
    for (int level = 0; level < 6; ++level)
    {
        if (accept(up.absolutePath() + "/resources/Wizard_Scripts"))
            return cached;
        if (!up.cdUp())
            break;
    }

    cached = (appDir + "/../../resources/Wizard_Scripts/").toStdString();
    return cached;
}

#define wizardsfolder OHQWizardScriptsFolder()

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
    bool AssignParameterValues(const QJsonObject &jsonObject);
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

