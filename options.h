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


#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>

class MainWindow;

namespace Ui {
class Dialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(MainWindow *parent = nullptr);
    ~OptionsDialog();

private:
    Ui::Dialog *ui;
    MainWindow *parent = nullptr;

private slots:
    void on_ok_clicked();
    void on_cancel_clicked();
};

#endif // OPTIONS_H
