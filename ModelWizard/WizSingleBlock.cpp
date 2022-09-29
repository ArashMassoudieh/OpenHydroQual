#include "WizSingleBlock.h"

SingleBlock::SingleBlock():Wizard_Entity()
{

}
SingleBlock::SingleBlock(const QJsonObject& json_obj)
{
    for (QJsonObject::const_iterator it = json_obj.begin(); it!=json_obj.end(); it++)
    {
        if (it.key()=="name")
        {
            name = json_obj["name"].toString();
        }
        else if (it.key()=="type")
        {
            type = json_obj["type"].toString();
        }
        else
        {
            QString full_expression_string = it.value().toString();
            QString expression = full_expression_string; 
            QString unit; 
            if (full_expression_string.contains(";"))
            {
                unit = full_expression_string.split(";")[1];
                expression = full_expression_string.split(";")[0];
            }
            QString key = it.key();

            Wizard_Argument arg(expression.toStdString(), unit.toStdString());
            Arguments[key] = arg;
        }
    }
}
SingleBlock::SingleBlock(const SingleBlock& MB) :Wizard_Entity(MB)
{    

}
SingleBlock& SingleBlock::operator=(const SingleBlock& MB)
{
    Wizard_Entity::operator=(MB);
    return *this;
}


QStringList SingleBlock::GenerateScript(QMap<QString, WizardParameter> *params)
{
    QStringList output;    
    QString line;
    line += "create block;";
    line += "type = " + Type();
    line += ", name =" +Name();
    for (QMap<QString, Wizard_Argument>::iterator it = Arguments.begin(); it != Arguments.end(); it++)
    {
        line += "," + it.key() + "=" + QString::number(it.value().calc(params))+"["+it.value().Unit()+"]";
    }
    output << line;
    qDebug() << output; 
    return output; 
    
}
