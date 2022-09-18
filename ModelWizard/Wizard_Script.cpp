#include "Wizard_Script.h"
#include "QJsonDocument"
#include "QFile"
#include "QIcon"
#include "QJsonArray"



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
            if (it.key() == "major_block")
            {
                MajorBlock mBlock(it.value().toObject());
                QString mbname = mBlock.Name();
                MajorBlocks[mbname] = mBlock;
            }
            if (it.key() == "parameter")
            {
                WizardParameter param(it.value().toObject());
                QString paramname = param.Name();
                WizardParameters[paramname] = param;
            }
            if (it.key() == "parameter_group")
            {
                WizardParameterGroup paramgroup(it.value().toObject());
                QString paramname = paramgroup.Name();
                WizardParameterGroups[paramname] = paramgroup;
            }
        }

}
WizardScript::WizardScript(const WizardScript &WS)
{
    iconfilename = WS.iconfilename;
    wizardname = WS.wizardname;
    description = WS.description;
    MajorBlocks = WS.MajorBlocks;
    WizardParameters = WS.WizardParameters;
}
WizardScript& WizardScript::operator=(const WizardScript& WS)
{
    iconfilename = WS.iconfilename;
    wizardname = WS.wizardname;
    description = WS.description;
    MajorBlocks = WS.MajorBlocks;
    WizardParameters = WS.WizardParameters;
    return *this;
}
QIcon WizardScript::Icon()
{
    QIcon icon(QString(wizardsfolder) + "Wizard_Icons/" + iconfilename);

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
