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

#include "TimeSeriesTextBox.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

TimeSeriesTextBox::TimeSeriesTextBox(const QStyleOptionViewItem &option, QWidget *parent)
    : QWidget(parent), Rect(option.rect)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout *layout = new QHBoxLayout(this);
#ifndef Qt6
    layout->setMargin(0);
#else
    layout->setContentsMargins(0, 0, 0, 0);
#endif
    layout->setSpacing(2);
    layout->setSizeConstraint(QLayout::SetMaximumSize);

    // File path display
    filePathEdit = new QLineEdit(this);
    filePathEdit->setReadOnly(true);
    filePathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Browse button
    browseButton = new QPushButton("...", this);
    browseButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    browseButton->setMaximumWidth(30);

    // Unit dropdown
    unitBox = new QComboBox(this);
    unitBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    unitBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    layout->addWidget(filePathEdit, 3);  // Give more space to file path
    layout->addWidget(browseButton, 0);
    layout->addWidget(unitBox, 1);

    connect(browseButton, &QPushButton::clicked, this, &TimeSeriesTextBox::onBrowseClicked);
    connect(unitBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &TimeSeriesTextBox::unitChanged);

    show();
}

TimeSeriesTextBox::~TimeSeriesTextBox()
{
    // Qt handles cleanup of child widgets
}

void TimeSeriesTextBox::setText(const QString &filename)
{
    filePathEdit->setText(filename);
}

void TimeSeriesTextBox::setUnit(const QString &unit)
{
    unitBox->setCurrentText(unit);
}

void TimeSeriesTextBox::setUnitsList(const QStringList &units)
{
    unitBox->clear();
    unitBox->addItems(units);
}

void TimeSeriesTextBox::setDefaultUnit(const QString &unit)
{
    m_defaultUnit = unit;
}

void TimeSeriesTextBox::setWorkingFolder(const QString &folder)
{
    m_workingFolder = folder;
}

QString TimeSeriesTextBox::text() const
{
    return filePathEdit->text();
}

QString TimeSeriesTextBox::unit() const
{
    return unitBox->currentText();
}

QStringList TimeSeriesTextBox::units() const
{
    QStringList result;
    for (int i = 0; i < unitBox->count(); i++)
        result.append(unitBox->itemText(i));
    return result;
}

QString TimeSeriesTextBox::defaultUnit() const
{
    return m_defaultUnit;
}

void TimeSeriesTextBox::paintEvent(QPaintEvent *event)
{
    setGeometry(rect());

    // Adjust font size for unit box
    QFont qf = font();
    qf.setPointSize(qf.pointSize() - 1);
    unitBox->setFont(qf);

    QWidget::paintEvent(event);
}

void TimeSeriesTextBox::setGeometry(const QRect &R)
{
    Rect = R;
    QWidget::setGeometry(R);
}

QRect TimeSeriesTextBox::rect() const
{
    return Rect;
}

void TimeSeriesTextBox::onBrowseClicked()
{
    QString startPath = m_workingFolder.isEmpty() ? QDir::currentPath() : m_workingFolder;

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Time Series File"),
        startPath,
        tr("Text files (*.txt);;CSV files (*.csv);;All files (*.*)"),
        nullptr,
        QFileDialog::DontUseNativeDialog
        );

    if (!fileName.isEmpty())
    {
        if (QFile::exists(fileName))
        {
            filePathEdit->setText(fileName);
            emit fileSelected(fileName);
        }
        else
        {
            QMessageBox::information(
                this,
                "File not found!",
                "File " + fileName + " was not found!",
                QMessageBox::Ok
                );
        }
    }
}
