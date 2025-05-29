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


#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QDialog>

namespace Ui {
class logwindow;
}

class logwindow : public QDialog
{
    Q_OBJECT

public:
    explicit logwindow(QWidget *parent = nullptr);
    ~logwindow();
    void AppendText(const QString &s);
    void AppendError(const QString &s);
    void AppendBlue(const QString &s);

private:
    Ui::logwindow *ui;
};

#endif // LOGWINDOW_H
