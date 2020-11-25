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
    int row = index.row();
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
       if (col == 0) return QString::fromStdString(quanset->GetVarAskable(index.row())->Description());
       if (col == 1)
       {
           if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).toLower().contains("date") == false)
               return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty());
           else
               return float2date(quanset->GetVarAskable(index.row())->GetVal());
       }
       break;
   case Qt::DisplayRole:
       if (col == 0) return QString::fromStdString(quanset->GetVarAskable(index.row())->Description());
       if (col == 1)
       {
           if (quanset->GetVarAskable(index.row())->GetParameterAssignedTo()=="")
           {
                if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).toLower().contains("date") == false)
                {
                    if (quanset->GetVarAskable(index.row())->Delegate()=="expressionEditor")
                    {
                        qDebug()<<"In propmodel: " << QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty(false));
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
                        qDebug()<<"In propmodel: " << QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty(false));
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
       //if (row == 1 && col == 0) //add a checkbox to cell(1,0)
       //    return Qt::Checked;
       break;
   case CustomRoleCodes::TypeRole:
       return QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate());
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
           return toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
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
    int row = index.row();
    int col = index.column();

    QString VariableName;
    if (role == CustomRoleCodes::Role::loadIndex) {
        QModelIndex idx = load();
        role = Qt::EditRole;
        //VariableName = idx.sibling(idx.row(), 0).data().toString();
        VariableName = idx.data(CustomRoleCodes::VariableNameRole).toString();
    }
    else
        VariableName = index.data(CustomRoleCodes::VariableNameRole).toString();

    if (role!=CustomRoleCodes::setParamRole)
    {
        if (quanset->GetVar(VariableName.toStdString()).GetParameterAssignedTo()!="")
        {
            mainwindow->GetSystem()->RemoveAsParameter(quanset->Parent()->GetName(),VariableName.toStdString(),quanset->GetVar(VariableName.toStdString()).GetParameterAssignedTo());
            quanset->GetVar(VariableName.toStdString()).SetParameterAssignedTo("");
        }
        bool r;
        if (quanset->GetVar(VariableName.toStdString()).Delegate()=="expressionEditor")
            r = quanset->GetVar(VariableName.toStdString()).SetProperty(value.toString().toStdString(),false);
        else
            r = quanset->GetVar(VariableName.toStdString()).SetProperty(value.toString().toStdString(),true);
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

        if (!r && (quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::prec_timeseries || quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::timeseries))
        {
            QMessageBox::question(mainwindow, "File does not have the right format!", "File does not have the right format!", QMessageBox::Ok);
        }
        else if (!r)
        {
            QMessageBox::question(mainwindow, "Validation Failed", QString::fromStdString(quanset->GetVar(VariableName.toStdString()).WarningMessage()), QMessageBox::Ok);
        }
    }
    else
    {
        quanset->GetVar(VariableName.toStdString()).SetParameterAssignedTo(value.toString().toStdString());
        mainwindow->GetSystem()->SetAsParameter(quanset->Parent()->GetName(),VariableName.toStdString(),value.toString().toStdString());
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

