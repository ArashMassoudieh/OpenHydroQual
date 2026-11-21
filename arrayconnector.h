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

#ifndef ARRAYCONNECTOR_H
#define ARRAYCONNECTOR_H

#include <QDialog>
#include <QMap>
#include <QStringList>
#include <string>

class MainWindow;
class System;
class QTabWidget;
class QListWidget;
class QComboBox;
class QLabel;
class QFormLayout;
class QPushButton;
class QLineEdit;

// Add Objects > Connect Two Arrays: links block (i:j) of one array to block (i:j) of another array of the same size
// (arrays are recognized by their block names, Name(i:j), as made by Create 2D Array), with one link type.
// Tab 1 picks the two arrays (check boxes), the link type and the direction; the link type is checked against the
// block types (every quantity its expressions read from the source (.s) and end (.e) block must exist there).
// Tab 2 sets the link's properties that are asked from the user.
class ArrayConnector : public QDialog
{
public:
    explicit ArrayConnector(MainWindow *parent);

    struct ArrayInfo { QString name; int nx = 0; int ny = 0; std::string blocktype; bool mixed = false; };

private:
    MainWindow *mainwindow;
    System *system();
    QMap<QString, ArrayInfo> arrays;
    QTabWidget *tabs = nullptr;
    QListWidget *arrayList = nullptr;
    QComboBox *linkType = nullptr;
    QComboBox *direction = nullptr;
    QLabel *status = nullptr;
    QFormLayout *propertyForm = nullptr;
    QPushButton *okButton = nullptr;
    struct PropertyEditor { QLineEdit *value = nullptr; QComboBox *unit = nullptr; std::string delegate; };
    QMap<QString, PropertyEditor> editors;

    void FindArrays();
    QStringList CheckedArrays() const;
    void UpdateDirections();
    void PopulateProperties();
    bool Validate(QString *message);
    QStringList MissingVariables(const std::string &linktype, const std::string &blocktype, const QString &suffix);
    void Connect();
};

#endif // ARRAYCONNECTOR_H
