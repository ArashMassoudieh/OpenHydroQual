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


#ifndef WIZARD_SELECT_DIALOG_H
#define WIZARD_SELECT_DIALOG_H

#include <QWidget>
#include <qdialog.h>
#include "ui_wizard_select_dialog.h"
#include "qstringlist.h"
#include "qmap.h"

struct plugin_information
{
    QString Filename;
    QString Description;
    QString IconFileName;
};

class MainWindow;

class Wizard_select_dialog : public QDialog
{
	Q_OBJECT

public:
    Wizard_select_dialog(MainWindow *parent);
	~Wizard_select_dialog();

private:
	Ui::Wizard_select_dialog ui;
    QString selected_template;
    QVector<plugin_information> DefaultPlugins;
    bool get_templates(const QString &TemplateList);
    MainWindow *parent = nullptr;


private slots:
	void on_ok_clicked();
	void on_cancel_clicked();


};


#endif // WIZARD_SELECT_DIALOG_H
