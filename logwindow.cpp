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
#include <QLayout>
#include <QScrollBar>

logwindow::logwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::logwindow)
{
    ui->setupUi(this);

    // Kept deliberately plain: the log sits next to the diagram, so it should read as a
    // status strip rather than compete with it.
    layout()->setContentsMargins(0, 0, 0, 0);
    ui->textBrowser->setFrameShape(QFrame::NoFrame);
    ui->textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->textBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QFont logfont = ui->textBrowser->font();
    logfont.setPointSize(8);
    ui->textBrowser->setFont(logfont);
    ui->textBrowser->document()->setDocumentMargin(4);
}

logwindow::~logwindow()
{
    delete ui;
}

void logwindow::ScrollToBottom()
{
    // The newest line is the one worth seeing, and the pane is short, so every append
    // pins the view to the end rather than leaving it wherever the user last scrolled.
    QScrollBar *bar = ui->textBrowser->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void logwindow::AppendText(const QString &s)
{
    ui->textBrowser->append(s);
    ScrollToBottom();
}

void logwindow::AppendError(const QString &s)
{
    ui->textBrowser->setFontWeight( QFont::DemiBold );
    ui->textBrowser->setTextColor( QColor( "red" ) );
    ui->textBrowser->append(s);
    ui->textBrowser->setFontWeight( QFont::Normal );
    ui->textBrowser->setTextColor( QColor( "black" ) );
    ScrollToBottom();
}

void logwindow::AppendBlue(const QString &s)
{
    ui->textBrowser->setTextColor( QColor( "blue" ) );
    ui->textBrowser->append(s);
    ui->textBrowser->setFontWeight( QFont::Normal );
    ui->textBrowser->setTextColor( QColor( "black" ) );
    ScrollToBottom();
}

void logwindow::Clear()
{
    ui->textBrowser->clear();
}
