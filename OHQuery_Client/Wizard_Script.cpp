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


#include "Wizard_Script.h"
#include "QJsonDocument"
#include "QFile"
#include "QIcon"
#include "QJsonArray"
#include "qapplication.h"
#include <QLineEdit>
#include "UnitTextBox3.h"
#include <QSpinBox>
#include "FilePushButton.h"
#include "QDateEdit"


WizardScript::WizardScript()
{

}
WizardScript::WizardScript(const QString& filename)
{
    QFile file_obj(filename);
        if (!file_obj.open(QIODevice::ReadOnly)) {
            qDebug()<<"Opening the file failed";
            return;
        }

        QTextStream file_text(&file_obj);
        QString json_string;
        json_string = file_text.readAll();
        file_obj.close();
        
        QJsonParseError parseError;
        QJsonDocument json_doc = QJsonDocument::fromJson(json_string.toUtf8(), &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "Error parsing JSON:";
            qDebug() << "Error message:" << parseError.errorString();
            qDebug() << "Error offset:" << parseError.offset;
            int line = 1, column = 1;
            for (int i = 0; i < parseError.offset; ++i) {
                if (json_string[i] == '\n') {
                    ++line;
                    column = 1;
                }
                else {
                    ++column;
                }
            }
            qDebug() << "Line:" << line << ", Column:" << column;

        }

        if (json_doc.isNull()) {
            qDebug() << "Failed to create JSON doc.";
            return;
        }
        if (!json_doc.isObject()) {
            qDebug() << "JSON is not an object.";
            return;
        }
        GetFromJsonDoc(json_doc);

}


bool WizardScript::GetFromJsonDoc(const QJsonDocument& json_doc)
{

    QJsonObject json_obj = json_doc.object();

    if (json_obj.isEmpty()) {
        qDebug() << "JSON object is empty.";
        return false;
    }

    for (QJsonObject::iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="icon")
        {
            iconfilename = json_obj["icon"].toString();
        }
        if (it.key() == "diagram")
        {
            diagramfilename = json_obj["diagram"].toString();
        }
        if (it.key()=="name")
        {
            wizardname = json_obj["name"].toString();
        }
        if (it.key() == "description")
        {
            description = json_obj["description"].toString();
        }
        if (it.key() == "url")
        {
            url = json_obj["url"].toString();
        }
        if (it.key() == "addtemplate")
        {
            QJsonArray items = it.value().toArray();
            for (int i=0; i<items.count(); i++)
            {
                addedtemplates<<items[i].toString();
            }
        }
        if (it.key() == "blockarrays")
        {
            QJsonArray items = it.value().toArray();
            for (int i=0; i<items.count(); i++)
            {   BlockArray mBlock(items[i].toObject());
                QString mbname = mBlock.Name();
                BlockArrays[mbname] = mBlock;
                BlockArrays[mbname].SetWizardScript(this);
            }
        }
        if (it.key() == "singleblocks")
        {
            QJsonArray items = it.value().toArray();
            for (int i=0; i<items.count(); i++)
            {   SingleBlock mBlock(items[i].toObject());
                QString mbname = mBlock.Name();
                SingleBlocks[mbname] = mBlock;
                SingleBlocks[mbname].SetWizardScript(this);
            }
        }
        if (it.key() == "connectors")
        {
            QJsonArray items = it.value().toArray();
            for (int i = 0; i < items.count(); i++)
            {
                Connector connector(items[i].toObject());
                QString mbname = connector.Name();
                Connectors[mbname] = connector;
                Connectors[mbname].SetWizardScript(this);
            }
        }
        if (it.key() == "entities")
        {
            QJsonArray items = it.value().toArray();
            for (int i = 0; i < items.count(); i++)
            {
                Wizard_Entity entity(items[i].toObject());
                QString enname = entity.Name();
                Entities[enname] = entity;
                Entities[enname].SetWizardScript(this);
            }
        }
        if (it.key() == "setvals")
        {
            QJsonArray items = it.value().toArray();
            for (int i = 0; i < items.count(); i++)
            {
                SetVal_Entity entity(items[i].toObject());
                QString enname = entity.Name();
                SetValEntities[enname] = entity;
                SetValEntities[enname].SetWizardScript(this);
            }
        }
        if (it.key() == "parameters")
        {
            QJsonArray items = it.value().toArray();
            for (int i=0; i<items.count(); i++)
            {   WizardParameter param(items[i].toObject());
                QString paramname = param.Name();
                WizardParameters[paramname] = param;
            }
            WizardParameter i;
            i.SetName("i");
            WizardParameters["i"] = i;
            WizardParameter j;
            i.SetName("j");
            WizardParameters["j"] = i;

        }
        if (it.key() == "parameter_groups")
        {
            QJsonArray items = it.value().toArray();
            for (int i=0; i<items.count(); i++)
            {   WizardParameterGroup paramgroup(items[i].toObject());
                QString paramgroupname = paramgroup.Name();
                WizardParameterGroups[paramgroupname] = paramgroup;
            }
        }
        if (it.key() == "parameter_populate_maps")
        {
            QJsonObject jsonobject = it->toObject();
            qDebug()<<jsonobject;
            for (QJsonObject::iterator parametermap = jsonobject.begin(); parametermap!=jsonobject.end(); parametermap++)
            {
                QMap<QString, QString> parameter_map_set;
                QJsonObject parametermapjsonobject = parametermap->toObject();
                for (QJsonObject::iterator parameteritem = parametermapjsonobject.begin(); parameteritem!=parametermapjsonobject.end(); parameteritem++)
                {
                    qDebug()<<parameteritem.key()<<":"<<parameteritem.value();
                    parameter_map_set[parameteritem.key()] = parameteritem.value().toString();
                }
                ParameterPopulateMaps[parametermap.key()] = parameter_map_set;
            }

        }
    }
    return true;
}



WizardScript::WizardScript(const WizardScript &WS)
{
    iconfilename = WS.iconfilename;
    wizardname = WS.wizardname;
    description = WS.description;
    BlockArrays = WS.BlockArrays;
    SingleBlocks = WS.SingleBlocks;
    WizardParameters = WS.WizardParameters;
    WizardParameterGroups = WS.WizardParameterGroups;
    SetValEntities = WS.SetValEntities;
    addedtemplates = WS.addedtemplates;
    Entities = WS.Entities;
    Connectors = WS.Connectors; 
    diagramfilename = WS.diagramfilename;
    ParameterPopulateMaps = WS.ParameterPopulateMaps;

    url = WS.url;
    SetAllParents(); 


}

void WizardScript::SetAllParents()
{
    for (QMap<QString, BlockArray>::iterator it = BlockArrays.begin(); it != BlockArrays.end(); it++)
    {
        it.value().SetWizardScript(this);
    }
    for (QMap<QString, SingleBlock>::iterator it = SingleBlocks.begin(); it != SingleBlocks.end(); it++)
    {
        it.value().SetWizardScript(this);
    }
    for (QMap<QString, SetVal_Entity>::iterator it = SetValEntities.begin(); it != SetValEntities.end(); it++)
    {
        it.value().SetWizardScript(this);
    }
    for (QMap<QString, Wizard_Entity>::iterator it = Entities.begin(); it != Entities.end(); it++)
    {
        it.value().SetWizardScript(this);
    }
    for (QMap<QString, Wizard_Entity>::iterator it = Entities.begin(); it != Entities.end(); it++)
    {
        it.value().SetWizardScript(this);
    }
    for (QMap<QString, Connector>::iterator it = Connectors.begin(); it != Connectors.end(); it++)
    {
        it.value().SetWizardScript(this);
    }
}

WizardScript& WizardScript::operator=(const WizardScript& WS)
{
    iconfilename = WS.iconfilename;
    wizardname = WS.wizardname;
    description = WS.description;
    BlockArrays = WS.BlockArrays;
    SingleBlocks = WS.SingleBlocks;
    WizardParameters = WS.WizardParameters;
    Entities = WS.Entities;
    addedtemplates = WS.addedtemplates;
    SetValEntities = WS.SetValEntities;
    Connectors = WS.Connectors;
    diagramfilename = WS.diagramfilename;
    WizardParameterGroups = WS.WizardParameterGroups;
    ParameterPopulateMaps = WS.ParameterPopulateMaps;

    url = WS.url;
    SetAllParents();
    return *this;
}

QString WizardScript::Name()
{
    return wizardname;
}
QString WizardScript::Description()
{
    return description;
}
QString WizardScript::Url()
{
    return url;
}

QStringList WizardScript::Script()
{
    QStringList script;
    for (int i = 0; i < addedtemplates.size(); i++)
        script.append("addtemplate; filename = " + addedtemplates[i]); 
    
    for (QMap<QString, SetVal_Entity>::iterator it = SetValEntities.begin(); it != SetValEntities.end(); it++)
    {
        QStringList out = it.value().GenerateScript(&GetWizardParameters());
        script.append(out);
    }

    for (QMap<QString, Wizard_Entity>::iterator it = Entities.begin(); it != Entities.end(); it++)
    {
        if (it.value().Entity()=="constituent")
        {   QStringList out = it.value().GenerateScript(&GetWizardParameters());
            script.append(out);
        }
    }

    for (QMap<QString, Wizard_Entity>::iterator it = Entities.begin(); it != Entities.end(); it++)
    {
        if (it.value().Entity()=="reaction_parameter")
        {   QStringList out = it.value().GenerateScript(&GetWizardParameters());
            script.append(out);
        }
    }

    for (QMap<QString, Wizard_Entity>::iterator it = Entities.begin(); it != Entities.end(); it++)
    {
        if (it.value().Entity()!="reaction_parameter" && it.value().Entity()!="constituent")
        {   QStringList out = it.value().GenerateScript(&GetWizardParameters());
            script.append(out);
        }
    }
    

    for (QMap<QString, BlockArray>::iterator it = GetBlockArrays().begin(); it != GetBlockArrays().end(); it++)
    {
        QStringList out = it.value().GenerateScript(&GetWizardParameters());
        script.append(out);
    }
    for (QMap<QString, SingleBlock>::iterator it = GetSingleBlocks().begin(); it != GetSingleBlocks().end(); it++)
    {
        QStringList out = it.value().GenerateScript(&GetWizardParameters());
        script.append(out);
    }
    for (QMap<QString, Connector>::iterator it = GetConnectors().begin(); it != GetConnectors().end(); it++)
    {
        QStringList out = it.value().GenerateScript(&GetWizardParameters());
        script.append(out);
    }
    return script; 
}

Wizard_Entity* WizardScript::FindEntity(QString name)
{
    if (SingleBlocks.count(name) == 1)
    {
        return &SingleBlocks[name];
    }
    else if (BlockArrays.count(name) == 1)
    {
        return &BlockArrays[name];
    }
    else
        return nullptr;
}

bool WizardScript::AssignParameterValues()
{
    for (QMap<QString,WizardParameter>::iterator it=GetWizardParameters().begin(); it!=GetWizardParameters().end(); it++)
    {
        if (it.value().EntryItem()!=nullptr)
        {   if (it.value().Delegate()=="ValueBox")
                it.value().SetValue(static_cast< QLineEdit*>(it.value().EntryItem())->text());
            else if (it.value().Delegate()=="UnitBox")
                it.value().SetValue(static_cast< UnitTextBox3*>(it.value().EntryItem())->text());
            else if (it.value().Delegate()=="SpinBox")
                it.value().SetValue(static_cast<QSpinBox*>(it.value().EntryItem())->text());
            else if (it.value().Delegate()=="ComboBox")
                it.value().SetValue(static_cast<QComboBox*>(it.value().EntryItem())->currentText());
            else if (it.value().Delegate() == "DateBox")
                it.value().SetValue(QString::number(QString2Xldate(static_cast<QDateEdit*>(it.value().EntryItem())->text())));
            else if (it.value().Delegate() == "FileBrowser")
                it.value().SetValue(static_cast<FilePushButton*>(it.value().EntryItem())->text());
        }
    }
    return true;
}

bool WizardScript::AssignParameterValues(const QJsonObject &jsonObject)
{
    for (QMap<QString,WizardParameter>::iterator it=GetWizardParameters().begin(); it!=GetWizardParameters().end(); it++)
    {
        if (it.value().EntryItem()!=nullptr)
        {   if (it.value().Delegate()=="ValueBox")
                it.value().SetValue(jsonObject[it.key()].toString());
            else if (it.value().Delegate()=="UnitBox")
                it.value().SetValue(jsonObject[it.key()].toString());
            else if (it.value().Delegate()=="SpinBox")
                it.value().SetValue(jsonObject[it.key()].toString());
            else if (it.value().Delegate()=="ComboBox")
                it.value().SetValue(jsonObject[it.key()].toString());
            else if (it.value().Delegate() == "DateBox")
                it.value().SetValue(QString::number(QString2Xldate(jsonObject[it.key()].toString())));
            else if (it.value().Delegate() == "FileBrowser")
                it.value().SetValue(QString::number(QString2Xldate(jsonObject[it.key()].toString())));
        }
    }
    return true;
}

QStringList WizardScript::CheckParameters()
{
    QStringList Errors;
    for (QMap<QString, WizardParameterGroup>::iterator it = WizardParameterGroups.begin(); it!=WizardParameterGroups.end(); it++)
    {
        Errors.append(it.value().CheckCriteria(&WizardParameters));
    }
    return Errors;
}
