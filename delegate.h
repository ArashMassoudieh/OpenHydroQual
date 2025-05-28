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


#ifndef DELEGATE_H
#define DELEGATE_H
#include <qstyleditemdelegate.h>
#include <qfiledialog.h>

class MainWindow;

class Delegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    MainWindow *mainwindow;
    Delegate(QObject *parent, MainWindow *mainwindow);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;


public slots:
    void browserClicked();
    void dirBrowserClicked();
    void browserSaveClicked();
    //void browserCheck(QString);// const;

private:
    QString browserFileName;
    QModelIndex *tempIndex;
    QFileDialog *fileDialog;
    QStyleOptionViewItem option;
    QString selected_fileName;
    QModelIndex selectedindex;
};
#endif // DELEGATE_H
