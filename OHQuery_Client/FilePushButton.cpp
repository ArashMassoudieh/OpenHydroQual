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
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#include "FilePushButton.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

FilePushButton::FilePushButton(QWidget *parent)
	: QPushButton(parent)
{
    setText("Browse...");
    setMinimumWidth(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setVisible(true);
    QObject::connect(this, SIGNAL(clicked()), this, SLOT(browserClicked()));
}

FilePushButton::~FilePushButton()
{
}

void FilePushButton::browserClicked()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        tr("Open"), "",
        tr("txt files (*.txt);; csv files (*.csv);; All files (*.*)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName != "")
    {
        if (QFile(fileName).exists())
            setText(fileName);
        else {
            QMessageBox::information(nullptr, "File not found!", "File " + fileName + " was not found!", QMessageBox::Ok, QMessageBox::StandardButton::Ok);
        }
    }
    else
        setText("");

}
