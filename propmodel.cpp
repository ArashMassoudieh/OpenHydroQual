#include "propmodel.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include "enums.h"
#include "qmessagebox.h"
#include "mainwindow.h"
#include "utilityfuncs.h"

PropModel::PropModel(QuanSet* _quanset, QObject *parent, MainWindow *_mainwindow)
    : QAbstractTableModel(parent)
{
    quanset = _quanset;
    mainwindow = _mainwindow;
}

int PropModel::rowCount(const QModelIndex &i) const
{
   return int (quanset->AskableSize());
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
   case Qt::DisplayRole:
       if (col == 0) return QString::fromStdString(quanset->GetVarAskable(index.row())->Description());
       if (col == 1)
       {
           if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).toLower().contains("date") == false)
               return QString::fromStdString(quanset->GetVarAskable(index.row())->GetProperty());
           else
               return float2date(quanset->GetVarAskable(index.row())->GetVal());

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

   case CustomRoleCodes::VariableNameRole:
       return QString::fromStdString(quanset->GetVarAskable(index.row())->GetName());
       break;
   case CustomRoleCodes::Role::DefaultValuesListRole:
       if (QString::fromStdString(quanset->GetVarAskable(index.row())->Delegate()).contains("Sources"))
       {

           return toQSringList(mainwindow->GetSystem()->GetAllSourceNames());
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

    bool r = quanset->GetVar(VariableName.toStdString()).SetProperty(value.toString().toStdString());

    if (!r && (quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::prec_timeseries || quanset->GetVar(VariableName.toStdString()).GetType() == Quan::_type::timeseries))
    {
        QMessageBox::question(mainwindow, "File does not have the right format!", "File does not have the right format!", QMessageBox::Ok);

    }

    QString result = value.toString();

    emit editCompleted(result);
    return true;
}

Qt::ItemFlags PropModel::flags(const QModelIndex & /*index*/) const
{
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled ;
}

