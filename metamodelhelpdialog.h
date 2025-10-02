/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 *
 * This file is part of OpenHydroQual.
 */

#ifndef METAMODHELPDIALOG_H
#define METAMODHELPDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QApplication>
#include <QStyle>
#include "MetaModel.h"

/**
 * @class MetaModelHelpDialog
 * @brief A dialog for displaying help and descriptions of QuanSet objects from a MetaModel
 *
 * This dialog provides an interactive interface for browsing QuanSet objects in a MetaModel.
 * It displays a list of all QuanSets with their icons on the left, and shows detailed
 * descriptions when items are clicked on the right.
 *
 * Features:
 * - List view with icons for each QuanSet type
 * - Detailed description panel
 * - Search/filter capability
 * - Resizable split view
 *
 * Example usage:
 * @code
 *   MetaModel model;
 *   model.GetFromJsonFile("config.json");
 *
 *   MetaModelHelpDialog dialog(&model, this);
 *   dialog.exec();
 * @endcode
 */
class MetaModelHelpDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param metamodel Pointer to the MetaModel to display
     * @param resourceDirectory Path to the resource directory for loading icons
     * @param parent Parent widget (optional)
     *
     * Creates the dialog and populates it with QuanSet information from the provided MetaModel.
     */
    explicit MetaModelHelpDialog(MetaModel* metamodel, const QString& resourceDirectory, QWidget *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~MetaModelHelpDialog();

    /**
     * @brief Sets the resource directory path for loading icons
     * @param path The path to the resource directory
     */
    void setResourceDirectory(const QString& path);

private slots:
    /**
     * @brief Handles item selection in the list
     * @param current The currently selected item
     * @param previous The previously selected item
     *
     * Updates the description panel when a new QuanSet is selected.
     */
    void onItemSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous);

    /**
     * @brief Handles the close button click
     */
    void onCloseClicked();

    /**
     * @brief Filters the list based on search text
     * @param text The search/filter text
     */
    void onFilterTextChanged(const QString& text);

private:
    /**
     * @brief Sets up the user interface components
     */
    void setupUI();

    /**
     * @brief Populates the list with QuanSet items from the MetaModel
     */
    void populateList();

    /**
     * @brief Generates HTML description for a QuanSet
     * @param quanset The QuanSet to describe
     * @param typeName The type name of the QuanSet
     * @return HTML formatted description string
     */
    QString generateDescription(const QuanSet& quanset, const QString& typeName) const;

    /**
     * @brief Gets the icon for a specific QuanSet type
     * @param typeName The type name of the QuanSet
     * @return QIcon for the type, or a default icon if not found
     */
    QIcon getIconForType(const QString& typeName);

    // UI Components
    QListWidget* m_listWidget;          ///< List of QuanSet types
    QTextBrowser* m_descriptionBrowser; ///< Description display area
    QLineEdit* m_filterEdit;            ///< Search/filter input
    QPushButton* m_closeButton;         ///< Close button
    QSplitter* m_splitter;              ///< Splitter for resizable panels

    // Data
    MetaModel* m_metamodel;             ///< Pointer to the MetaModel
    QMap<QString, QString> m_descriptions; ///< Cached descriptions for each type
    QString m_resourceDirectory;        ///< Path to resource directory for icons
};

#endif // METAMODHELPDIALOG_H
