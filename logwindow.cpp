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


#include "logwindow.h"
#include "ui_logwindow.h"

logwindow::logwindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logwindow)
{
    ui->setupUi(this);
}

logwindow::~logwindow()
{
    delete ui;
}

void logwindow::AppendText(const QString &s)
{
    ui->textBrowser->append(s);

}

void logwindow::AppendError(const QString &s)
{
    ui->textBrowser->setFontWeight( QFont::DemiBold );
    ui->textBrowser->setTextColor( QColor( "red" ) );
    ui->textBrowser->append(s);
    ui->textBrowser->setFontWeight( QFont::Normal );
    ui->textBrowser->setTextColor( QColor( "black" ) );

}

void logwindow::AppendBlue(const QString &s)
{
    ui->textBrowser->setTextColor( QColor( "blue" ) );
    ui->textBrowser->append(s);
    ui->textBrowser->setFontWeight( QFont::Normal );
    ui->textBrowser->setTextColor( QColor( "black" ) );

}
