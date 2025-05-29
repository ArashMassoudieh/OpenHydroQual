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


#ifndef PROPMODEL_H
#define PROPMODEL_H
#include <QAbstractTableModel>
#include <QuanSet.h>
class MainWindow;
class System;
class PropModel: public QAbstractTableModel
{
private:
    Q_OBJECT
    QuanSet *quanset = nullptr;
    MainWindow *mainwindow;
public:
    PropModel(QuanSet*, QObject *parent = nullptr, MainWindow *_mainwindow = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    System* GetSystem();
private:
    int rows(const QModelIndex & index = QModelIndex()) const{

        return quanset->QuanNames().size();
    }
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
signals:
     void editCompleted(const QString &);
};

#endif // PROPMODEL_H
