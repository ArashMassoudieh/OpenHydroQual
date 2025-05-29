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


#include "options.h"
#include "ui_Options.h"
#include "mainwindow.h"
#include "node.h"
#include "edge.h"

OptionsDialog::OptionsDialog(MainWindow *_parent) :
    QDialog(_parent),
    ui(new Ui::Dialog)
{
    parent = _parent;
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(on_ok_clicked()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(on_cancel_clicked()));
    ui->doubleSpinBox->setValue(parent->GetDiagramView()->fontfactor);
    ui->LineThickness->setValue(parent->GetDiagramView()->linkthickness);
    ui->radioButton->setChecked(parent->GetDiagramView()->showlinkicons);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_ok_clicked()
{
    if (parent)
    {
        parent->GetDiagramView()->fontfactor = ui->doubleSpinBox->value();
        parent->GetDiagramView()->linkthickness = ui->LineThickness->value();
        parent->GetDiagramView()->showlinkicons = ui->radioButton->isChecked();
        parent->GetDiagramView()->update();
        parent->GetDiagramView()->scene()->update(parent->GetDiagramView()->sceneRect());
        for (int i=0; i<parent->GetDiagramView()->Nodes().size(); i++)
            parent->GetDiagramView()->Nodes()[i]->update();
        for (int i=0; i<parent->GetDiagramView()->Edges().size(); i++)
            parent->GetDiagramView()->Edges()[i]->update();


    }
    this->close();
}

void OptionsDialog::on_cancel_clicked()
{
    this->close();
}

