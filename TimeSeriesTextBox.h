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

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QHBoxLayout>
#include <QStyleOptionViewItem>

class TimeSeriesTextBox : public QWidget
{
    Q_OBJECT

public:
    TimeSeriesTextBox(const QStyleOptionViewItem &option, QWidget *parent = nullptr);
    ~TimeSeriesTextBox();

    // Setters
    void setText(const QString &filename);
    void setUnit(const QString &unit);
    void setUnitsList(const QStringList &units);
    void setDefaultUnit(const QString &unit);
    void setWorkingFolder(const QString &folder);

    // Getters
    QString text() const;
    QString unit() const;
    QStringList units() const;
    QString defaultUnit() const;

    void paintEvent(QPaintEvent *event) override;
    void setGeometry(const QRect &R);
    QRect rect() const;

private:
    QLineEdit *filePathEdit = nullptr;
    QPushButton *browseButton = nullptr;
    QComboBox *unitBox = nullptr;
    QRect Rect;
    QString m_defaultUnit;
    QString m_workingFolder;

private slots:
    void onBrowseClicked();

signals:
    void fileSelected(const QString &filename);
    void unitChanged(const QString &unit);
};
