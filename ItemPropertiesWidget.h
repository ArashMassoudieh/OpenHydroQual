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


#ifndef ITEMPROPERTIESWIDGET_H_9DE3B491A3066902
#define ITEMPROPERTIESWIDGET_H_9DE3B491A3066902

#include "ItemNavigator.h"

#include <QFrame>
#include "propmodel.h"
#include <memory>
#include <QTableView>

class QSortFilterProxyModel;
class QToolButton;

namespace Ui {
class ItemPropertiesWidget;
}


class MetaLayerItem;
class ItemNavigator;

//////////////////////////////////////////////////////////////////////////
// ItemPropertiesWidget

class ItemPropertiesWidget: public QFrame, public ItemNavigatorHolder {
    Q_OBJECT
public:
    explicit ItemPropertiesWidget(QWidget* parent = nullptr);
    ~ItemPropertiesWidget();
    QTableView* tableView();
    // Property model
    void setTableModel(PropModel* propmodel = nullptr);

    QSize minimumSizeHint() const override;
    void SetTitleText(const QString &title);
    void setIcon(const QString &IconFileName);
signals:
    void showTimeSeries(QStringView item_name, QStringView prop_name,
        QStringView ts_path, ItemNavigatorPtr item_navigator);
    
protected slots:
    //void on_btnNavigate_clicked();
    //void on_edtFilter_textChanged(const QString& text);

    // ItemNavigatorHolder
    void onNavigatorSet() override;

private:
    void setupControls();
    void onButtonWidgetClicked(int prop_index);

    Ui::ItemPropertiesWidget* ui;
    QToolButton* btnNavigate_ = nullptr;

    QSortFilterProxyModel* prop_proxy_model_ = nullptr;
    PropModel* prop_model_       = nullptr;
    QWidget* _parent = nullptr;
};



//////////////////////////////////////////////////////////////////////////
#endif // ITEMPROPERTIESWIDGET_H_9DE3B491A3066902
