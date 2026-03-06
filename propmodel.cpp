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
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#include "propmodel.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include "enums.h"
#include "qmessagebox.h"
#include "mainwindow.h"
#include "utilityfuncs.h"
#include "diagramview.h"
#include "node.h"
#include "System.h"
#include "XString.h"

PropModel::PropModel(QuanSet* _quanset, QObject *parent, MainWindow *_mainwindow)
    : QAbstractTableModel(parent)
{
    quanset = _quanset;
    mainwindow = _mainwindow;
}

int PropModel::rowCount(const QModelIndex &i) const
{
    if (quanset != nullptr)
        return int(quanset->AskableSize());
    else
        return 0; 
}

int PropModel::columnCount(const QModelIndex &i) const
{
    return 2;
}

QVariant PropModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
   // generate a log message when this method gets called
   //qDebug() << QString("row %1, col%2, role %3")
   //        .arg(row).arg(col).arg(role);

    if (role == CustomRoleCodes::Role::saveIndex)
    {
        save(index);
        return "index Saved";
    }



   switch (role) {
   case Qt::ToolTipRole:
       if (col == 0)
       {   if (quanset->GetVarAskable(index.row())->HelpText()!="")
               return XString::reform(QString::fromStdString(quanset->GetVarAskable(index.row())->HelpText()));
           else
               return XString::reform(QString::fromStdString(quanset->GetVarAskable(index.row())->Description()));

       }
       if (col == 1)
       {
           if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).toLower().contains("date") == false)
               return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty());
           else
               return float2date(quanset->GetVarAskable(index.row())->GetVal());
       }
       break;
   case Qt::DisplayRole:
       if (col == 0) return XString::reform(QString::fromStdString(quanset->GetVarAskable(index.row())->Description()));
       if (col == 1)
       {
           if (quanset->GetVarAskable(index.row())->GetParameterAssignedTo()=="")
           {
                if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).toLower().contains("date") == false)
                {
                    if (quanset->GetVarAskable(index.row())->Delegate()=="expressionEditor")
                    {
                        return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty(false));
                    }
                    else if (quanset->GetVarAskable(index.row())->Delegate()=="UnitBox" && quanset->GetVarAskable(index.row())->Units()!="")
                    {
                        double coefficient = XString::coefficient(QString::fromStdString(quanset->GetVarAskable(index.row())->Unit()));
                        double value = atof(quanset->GetVarAskable(index.row())->GetProperty(true).c_str())/coefficient;
                        return QString::number(value) + "["+XString::reform(QString::fromStdString(quanset->GetVarAskable(index.row())->Unit()))+"]";
                    }
                    else if (quanset->GetVarAskable(index.row())->Delegate().find("Browser") != std::string::npos
                             && quanset->GetVarAskable(index.row())->Units()!="")
                    {
                        // Get the property (filename [unit])
                        QString displayText = QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty(true));

                        // Reform unit in brackets if present
                        int bracketStart = displayText.indexOf('[');
                        if (bracketStart != -1)
                        {
                            int bracketEnd = displayText.indexOf(']', bracketStart);
                            if (bracketEnd != -1)
                            {
                                QString filename = displayText.left(bracketStart).trimmed();
                                QString unit = displayText.mid(bracketStart + 1, bracketEnd - bracketStart - 1);

                                // Reform the unit (m^3 → m³)
                                QString reformedUnit = XString::reform(unit);

                                return filename + " [" + reformedUnit + "]";
                            }
                        }
                        return displayText;
                    }
                    else
                    {
                        return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty(true));
                    }
                }
                else
                    return float2date(quanset->GetVarAskable(index.row())->GetVal());
           }
           else
           {
               return  QString::fromStdString(quanset->GetVarAskable(index.row())->GetParameterAssignedTo());
           }
       }
       break;
   case Qt::EditRole:
       if (col == 0) return QString::fromStdString(quanset->GetVarAskable(index.row())->Description());
       if (col == 1)
       {
           if (quanset->GetVarAskable(index.row())->GetParameterAssignedTo()=="")
           {
                if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).toLower().contains("date") == false)
                {
                    if (quanset->GetVarAskable(index.row())->Delegate()=="expressionEditor")
                    {
                        return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty(false));
                    }
                    else
                    {
                        return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty(true));
                    }

                }
                else
                    return float2date(quanset->GetVarAskable(index.row())->GetVal());
           }
           else
           {
               return  QString::fromStdString(quanset->GetVarAskable(index.row())->GetParameterAssignedTo());
           }
       }
       break;
   case Qt::FontRole:
       if (col == 1) {
           QFont boldFont;
           boldFont.setBold(true);
           return boldFont;
       }
       break;
   case Qt::BackgroundRole:
       if (col == 0)
           return QBrush(Qt::lightGray);
       break;
   case Qt::CheckStateRole:
       if (quanset->GetVarAskable(index.row())->Delegate()=="CheckBox" && col==1)
       {   if (quanset->GetVarAskable(index.row())->GetVal()==1)
               return Qt::CheckState::Checked;
           else
               return Qt::CheckState::Unchecked;
       }
       break;
   case CustomRoleCodes::UnitsListRole:
       return  XString::reform(QString::fromStdString(quanset->GetVarAskable(index.row())->Units())).split(";");
       break;
   case CustomRoleCodes::defaultUnitRole:
       return  XString::reform(QString::fromStdString(quanset->GetVarAskable(index.row())->DefaultUnit()));
       break;
   case CustomRoleCodes::UnitRole:
       return  XString::reform(QString::fromStdString(quanset->GetVarAskable(index.row())->Unit()));
       break;
   case CustomRoleCodes::TypeRole:
       return QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate());
       break;

   case CustomRoleCodes::fullFileNameRole:
       return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty());
       break;

   case CustomRoleCodes::ObjectName:
       return QString::fromStdString(quanset->Parent()->GetName());
       break;

   case CustomRoleCodes::VariableName:
       return QString::fromStdString(quanset->GetVarAskable(index.row())->GetName());
       break;

   case CustomRoleCodes::EstimateCode:
       return quanset->GetVarAskable(index.row())->Estimable();
       break;

   case CustomRoleCodes::VariableNameRole:
       return QString::fromStdString(quanset->GetVarAskable(index.row())->GetName());
       break;
   case CustomRoleCodes::Role::DefaultValuesListRole:
       if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).contains("Sources"))
       {
           QStringList out = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
           out.append("");
           return out;
       }
       if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).contains("BlockLinks"))
       {
           QStringList items = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
           items.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
           return items;
       }
       if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).contains("items"))
       {
           QStringList items = QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).split(":")[1].split(",");
           return items;
       }
       break;
   case CustomRoleCodes::Role::referedObjectRole:
       {
            if (quanset->Count("object")>0)
                return QString::fromStdString(quanset->GetVar("object").GetProperty());
            else
               return "";

       }
   case CustomRoleCodes::Role::allowableWordsRole:
       {
           QStringList words;
           for (unsigned int i=0; i<quanset->AllConstituents().size(); i++)
                words << QString::fromStdString(quanset->AllConstituents()[i]);

           for (unsigned int i=0; i<quanset->AllReactionParameters().size(); i++)
                words << QString::fromStdString(quanset->AllReactionParameters()[i]);

           return words;
       }
   }

   return QVariant();
}

QVariant PropModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("Property");
            case 1:
                return QString("Value");
            }
        }
    }
    return QVariant();
}

bool PropModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.row() >= rows()) return false;

    QString VariableName;

    // Get variable name from the passed index
    VariableName = index.data(CustomRoleCodes::VariableNameRole).toString();

    if (VariableName.isEmpty())
    {
        qDebug() << "Error: VariableName is empty in setData";
        return false;
    }

    // Handle loadIndex specially - it triggers file loading
    if (role == CustomRoleCodes::Role::loadIndex) {
        save(index);  // Save the index for use in browserClicked
        // Now actually load the file by calling SetProperty
        bool r = quanset->GetVar(VariableName.toStdString()).SetProperty(value.toString().toStdString(), true);

        if (!r && value.toString() != "")
        {
            if (quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::prec_timeseries ||
                quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::timeseries)
            {
                QMessageBox::question(mainwindow, "File does not have the right format!",
                                      "File does not have the right format!", QMessageBox::Ok);
            }
        }

        emit editCompleted(value.toString());
        return true;
    }

    bool r;
    if (role==CustomRoleCodes::UnitRole)
    {
        // Call SetUnit to trigger unit conversion
        r = quanset->GetVar(VariableName.toStdString()).SetUnit(XString::reformBack(value.toString()).toStdString());
        emit editCompleted(value.toString());
        return r;
    }
    else if (role!=CustomRoleCodes::setParamRole)
    {
        if (quanset->GetVar(VariableName.toStdString()).GetParameterAssignedTo()!="")
        {
            mainwindow->GetSystem()->RemoveAsParameter(quanset->Parent()->GetName(),VariableName.toStdString(),quanset->GetVar(VariableName.toStdString()).GetParameterAssignedTo());
            quanset->GetVar(VariableName.toStdString()).SetParameterAssignedTo("");
            r = true;
        }

        if (quanset->GetVar(VariableName.toStdString()).Delegate()=="expressionEditor")
            r = quanset->GetVar(VariableName.toStdString()).SetProperty(value.toString().toStdString(),false);
        else if (quanset->GetVar(VariableName.toStdString()).Delegate()=="UnitBox")
        {
            double coefficient = XString::coefficient(QString::fromStdString(quanset->GetVarAskable(index.row())->Unit()));
            double _value = value.toString().split('[')[0].toDouble()*coefficient;
            r = quanset->GetVar(VariableName.toStdString()).SetProperty(aquiutils::numbertostring(_value),true);
        }
        else
        {
            if (quanset->GetVar(VariableName.toStdString()).GetParent()==nullptr)
            {
                quanset->GetVar(VariableName.toStdString()).SetParent(quanset->Parent());
            }
            r = quanset->GetVar(VariableName.toStdString()).SetProperty(value.toString().toStdString(),true);
        }

        if (VariableName == "x")
        {
            mainwindow->GetDiagramView()->node(QString::fromStdString(quanset->Parent()->GetName()))->setX(value.toInt());
        }
        if (VariableName == "y")
        {
            mainwindow->GetDiagramView()->node(QString::fromStdString(quanset->Parent()->GetName()))->setY(value.toInt());
        }
        if (VariableName == "_width")
        {
            mainwindow->GetDiagramView()->node(QString::fromStdString(quanset->Parent()->GetName()))->setWidth(value.toInt());
        }
        if (VariableName == "_height")
        {
            mainwindow->GetDiagramView()->node(QString::fromStdString(quanset->Parent()->GetName()))->setHeight(value.toInt());
        }
    }
    else
    {
        quanset->GetVar(VariableName.toStdString()).SetParameterAssignedTo(value.toString().toStdString());
        mainwindow->GetSystem()->SetAsParameter(quanset->Parent()->GetName(),VariableName.toStdString(),value.toString().toStdString());
        r=true;
    }

    if ((!r && value!="") && (quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::prec_timeseries || quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::timeseries))
    {
        QMessageBox::question(mainwindow, "File does not have the right format!", "File does not have the right format!", QMessageBox::Ok);
    }
    else if (!r & value!="")
    {
        QMessageBox::question(mainwindow, "Validation Failed", QString::fromStdString(quanset->GetVar(VariableName.toStdString()).WarningMessage()), QMessageBox::Ok);
    }

    QString result = value.toString();

    emit editCompleted(result);
    return true;
}

Qt::ItemFlags PropModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

System* PropModel::GetSystem()
{
    if (quanset!=nullptr)
        if (quanset->Parent()!=nullptr)
            if (quanset->Parent()->Parent()!=nullptr)
                return quanset->Parent()->Parent();
    return nullptr;
}

