#ifndef PROPMODEL_H
#define PROPMODEL_H

#include "qabstractitemmodel.h"
#include "qstring.h"
#include "qfiledialog.h"
#include "qdir.h"
#include "System.h"

template <class T>
class PropModel :
    public QAbstractTableModel
{
public:

    PropModel(){}
    PropModel(QuanSet* parent){ this->parent = parent; }

    QList<T*> itemsList() {
        return items;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    int rows(const QModelIndex & index = QModelIndex()) const{
    //	//qDebug() << "rows" <<  parent->getmList(parent->Filter()).size();
        return parent->size();
    }
    int rowCount(const QModelIndex & index = QModelIndex()) const{
        return parent->size();
    }
    int columnCount(const QModelIndex & parent = QModelIndex()) const {
        return 2; }
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const{
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
    Qt::ItemFlags flags(const QModelIndex & index) const{
        if (index.row() >= rows()) return nullptr;
        int col = index.column();
        //	if (index.row() >= list.size()) return 0;
        if (col == 0) return Qt::ItemIsEnabled;

        if (col == 1)
        {
            if (data(index, TypeRole) == "ComboBox" && data(index, DefaultValuesListRole).toStringList().count() == 0)
                return nullptr;
            return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
        }
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    ~PropModel(){}
    QuanSet *parent;
    QList<T*> items;
    GraphWidget *mainGraphWidget() const {
        return parent->parent;
    }
    void update(){
//		if (repaintAll) emit datachanged(index(0, 0), index(rowCount(), columnCount()));
        emit changed();
    }
signals:
    void changed();
public slots:
    void browserCheck(QString _fileName)
    {
        QString file;
        if (_fileName.contains("..."))
            file =
            QFileDialog::getOpenFileName(
                        nullptr, //qApp->activeWindow(),
            tr("Select the File"),
            QDir::currentPath(),
            tr("Time Series Input Files (*.txt *.csv)"));
        else file = _fileName;
        if (file.isEmpty()) return;

        QString fileName = file;

        if (!mainGraphWidget()->inflowFileNames.contains(fileName)) mainGraphWidget()->inflowFileNames.append(fileName);
        setData(QModelIndex(), fileName, loadIndex);
        //parent->deselectAll();
        return;
    }

private:
    void save(const QModelIndex index) const{
        loadSave(index, "s");
    }
    QModelIndex load() const{
        return loadSave(QModelIndex(), "l");
    }

    QModelIndex loadSave(QModelIndex index, QString ls) const
    {
        static QModelIndex r;
        if (ls == "s") {
            r = index;
            return QModelIndex();
        }
        if (ls == "l")return r;
        return QModelIndex();
    }
};

#endif // PROPMODEL_H
