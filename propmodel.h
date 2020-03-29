#ifndef PROPMODEL_H
#define PROPMODEL_H
#include <QAbstractTableModel>
#include <QuanSet.h>

class PropModel: public QAbstractTableModel
{
private:
    Q_OBJECT
    QuanSet *quanset;
public:
    PropModel(QuanSet*, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
private:

signals:
     void editCompleted(const QString &);
};

#endif // PROPMODEL_H
