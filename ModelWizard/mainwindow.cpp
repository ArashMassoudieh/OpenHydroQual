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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Wizard_Script.h"
#include "QDir"
#include "wizarddialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listWidget->setFlow(QListView::LeftToRight);

    //Dynamically adjust contents
    ui->listWidget->setResizeMode(QListView::Adjust);

    //This is an arbitrary value, but it forces the layout into a grid
    ui->listWidget->setGridSize(QSize(150, 150));
    ui->listWidget->setIconSize(QSize(128, 128));
    //As an alternative to using setGridSize(), set a fixed spacing in the layout:
    ui->listWidget->setSpacing(5);

    //And the most important part:
    ui->listWidget->setViewMode(QListView::IconMode);
    QObject::connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
    PopulateListOfWizards();
    QIcon mainicon(QString::fromStdString(wizardsfolder) + "/../Icons/wizard blue.png");
    
    setWindowIcon(mainicon);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::PopulateListOfWizards()
{

    QDir directory(QString::fromStdString(wizardsfolder));
    qDebug() << directory.absolutePath(); 
    qDebug() << directory.exists();
    QStringList wizardfiles = directory.entryList(QStringList() << "*.json" << "*.JSON",QDir::Files);

    foreach(QString filename, wizardfiles) {
        WizardScript wiz(QString::fromStdString(wizardsfolder)+filename);

        QListWidgetItem *item = new QListWidgetItem(wiz.Icon(),wiz.Name());
        item->setData(1000,QString::fromStdString(wizardsfolder)+filename);
        item->setToolTip(wiz.Description());

        ui->listWidget->addItem(item);
    }
}

void MainWindow::itemDoubleClicked(QListWidgetItem* wizitem)
{
    WizardDialog *wizDialog = new WizardDialog();
    
    WizardScript wiz(wizitem->data(1000).toString());
    wizDialog->setWindowTitle(wiz.Description());
    wizDialog->CreateItems(&wiz);
    wizDialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

    wizDialog->showMaximized();
    wizDialog->resizeEvent();
}

