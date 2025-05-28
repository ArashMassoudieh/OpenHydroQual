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


#ifndef STATUSVIEWER_H
#define STATUSVIEWER_H
#include <QLabel>


class QTimer;

class StatusViewer : public QLabel
{
    Q_OBJECT
public:
    StatusViewer(QWidget* p = 0);

    void showInfo(const QString& msg, int delay = -1);
    void showWarn(const QString& msg, int delay = -1);
    void showError(const QString& msg, int delay = -1);

private:
    void setStatus(const QString& msg, int delay, const QColor& color);

private slots:
    void onTimeout();

private:
    QTimer* _timer;
};

#endif // STATUSVIEWER_H
