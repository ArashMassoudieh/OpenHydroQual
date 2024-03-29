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
        QByteArray json_bytes = json_string.toLocal8Bit();

        // step 3
        auto json_doc = QJsonDocument::fromJson(json_bytes);

        if (json_doc.isNull()) {
            qDebug() << "Failed to create JSON doc.";
            return;
        }
        if (!json_doc.isObject()) {
            qDebug() << "JSON is not an object.";
            return;
        }

        QJsonObject json_obj = json_doc.object();

        if (json_obj.isEmpty()) {
            qDebug() << "JSON object is empty.";
            return;
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
        }

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
    SetAllParents();
    return *this;
}
QIcon WizardScript::Icon()
{
    QIcon icon(QString::fromStdString(wizardsfolder) + "Wizard_Icons/" + iconfilename);

    return  icon;
}
QString WizardScript::Name()
{
    return wizardname;
}
QString WizardScript::Description()
{
    return description;
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
        QStringList out = it.value().GenerateScript(&GetWizardParameters());
        script.append(out);
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

QStringList WizardScript::CheckParameters()
{
    QStringList Errors;
    for (QMap<QString, WizardParameterGroup>::iterator it = WizardParameterGroups.begin(); it!=WizardParameterGroups.end(); it++)
    {
        Errors.append(it.value().CheckCriteria(&WizardParameters));
    }
    return Errors;
}
