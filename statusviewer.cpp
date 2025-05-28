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


#include "statusviewer.h"
#include <QTimer>
#include <QDebug>

StatusViewer::StatusViewer(QWidget* p): QLabel(p)
{
    setAlignment(Qt::AlignCenter);
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), SLOT(onTimeout()));
}


void StatusViewer::showInfo(const QString &msg, int delay)
{
    setStatus(msg, delay, QColor("black"));
    //qDebug() << msg;
}


void StatusViewer::showWarn(const QString &msg, int delay)
{
    setStatus(msg, delay, QColor("#F68A1E"));
}


void StatusViewer::showError(const QString &msg, int delay)
{
    setStatus(msg, delay, QColor("#D60000"));
}



void StatusViewer::setStatus(const QString &msg, int delay, const QColor &color)
{
    _timer->stop();
    setStyleSheet(QString("color:%1").arg(color.name()));
    setText(msg);
    if (delay > 0) {
        _timer->setInterval(delay * 1000);
        _timer->start();
    }
}


void StatusViewer::onTimeout()
{
    this->setText("");
}
