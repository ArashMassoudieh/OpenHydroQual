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


#include "UnitTextBox3.h"

UnitTextBox3::UnitTextBox3(const XString &X, bool openFileMenu, QWidget * parent)
    :QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    textBox = new QLineEdit(this);
    layout->addWidget(textBox);

    unitBox = new QComboBox(textBox);
    layout->addWidget(unitBox);
    layout->setSizeConstraint(QLayout::SetMaximumSize);
    setLayout(layout);
    validator = new QDoubleValidator(this);
    textBox->setValidator(validator);
    setGeometry(QRect(0, 0, 300, 20));
    setXString(X);
    setFocusProxy(textBox);
    updateContextMenu(openFileMenu);
    connect(textBox, SIGNAL(textEdited(const QString&)),this, SLOT(on_text_edited(const QString&)));
}

void UnitTextBox3::on_text_edited(const QString& text)
{
    emit textEdited(text);
}
