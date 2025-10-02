/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 *
 * This file is part of OpenHydroQual.
 */

#include "metamodelhelpdialog.h"
#include <QDebug>

MetaModelHelpDialog::MetaModelHelpDialog(MetaModel* metamodel, const QString& resourceDirectory, QWidget *parent)
    : QDialog(parent)
    , m_metamodel(metamodel)
    , m_resourceDirectory(resourceDirectory)
{
    setupUI();
    populateList();
}

MetaModelHelpDialog::~MetaModelHelpDialog()
{
    // Qt handles cleanup of child widgets
}

void MetaModelHelpDialog::setupUI()
{
    setWindowTitle(tr("MetaModel Help - Object Descriptions"));
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    resize(900, 600);
    setSizeGripEnabled(true);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title label
    QLabel* titleLabel = new QLabel(tr("<h2>Object Type Reference</h2>"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    mainLayout->addWidget(titleLabel);

    // Filter/search box
    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* filterLabel = new QLabel(tr("Filter:"), this);
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(tr("Type to filter objects..."));
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(m_filterEdit);
    mainLayout->addLayout(filterLayout);

    // Splitter for list and description
    m_splitter = new QSplitter(Qt::Horizontal, this);

    // Left panel - List of QuanSets
    m_listWidget = new QListWidget(this);
    m_listWidget->setIconSize(QSize(32, 32));
    m_listWidget->setMinimumWidth(250);
    m_listWidget->setSortingEnabled(true);
    m_splitter->addWidget(m_listWidget);

    // Right panel - Description browser
    m_descriptionBrowser = new QTextBrowser(this);
    m_descriptionBrowser->setOpenExternalLinks(false);
    m_descriptionBrowser->setMinimumWidth(400);
    m_splitter->addWidget(m_descriptionBrowser);

    // Set initial splitter sizes (30% list, 70% description)
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 7);

    mainLayout->addWidget(m_splitter);

    // Bottom button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_closeButton = new QPushButton(tr("Close"), this);
    m_closeButton->setDefault(true);
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_listWidget, &QListWidget::currentItemChanged,
            this, &MetaModelHelpDialog::onItemSelectionChanged);
    connect(m_closeButton, &QPushButton::clicked,
            this, &MetaModelHelpDialog::onCloseClicked);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &MetaModelHelpDialog::onFilterTextChanged);

    // Initial message
    m_descriptionBrowser->setHtml(
        "<div style='text-align: center; margin-top: 50px;'>"
        "<h3>Welcome to MetaModel Help</h3>"
        "<p>Select an object type from the list on the left to view its description.</p>"
        "</div>"
        );
}

void MetaModelHelpDialog::populateList()
{
    if (!m_metamodel) {
        qWarning() << "MetaModel pointer is null!";
        return;
    }

    m_listWidget->clear();
    m_descriptions.clear();

    // Iterate through all QuanSets in the MetaModel
    for (auto it = m_metamodel->begin(); it != m_metamodel->end(); ++it)
    {
        const QString typeName = QString::fromStdString(it->first);

        // Skip internal entries like "solutionorder"
        if (typeName == "solutionorder") {
            continue;
        }

        QuanSet& quanset = it->second;

        // Get description for display
        QString displayName = QString::fromStdString(quanset.Description());
        if (displayName.isEmpty()) {
            displayName = typeName;  // Fallback to type name if no description
        }

        // Create list item
        QListWidgetItem* item = new QListWidgetItem(m_listWidget);
        item->setText(displayName);  // Use description for display
        item->setIcon(getIconForType(typeName));
        item->setData(Qt::UserRole, typeName); // Still store type name for internal use

        // Generate and cache description
        QString description = generateDescription(quanset, typeName);
        m_descriptions[typeName] = description;

        m_listWidget->addItem(item);
    }

    // Select first item if available
    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
    }
}

QString MetaModelHelpDialog::generateDescription(const QuanSet& quanset, const QString& typeName) const
{
    QString html;
    html += "<html><head><style>";
    html += "body { font-family: Arial, sans-serif; margin: 10px; }";
    html += "h1 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 5px; }";
    html += "h2 { color: #34495e; margin-top: 20px; }";
    html += "table { border-collapse: collapse; width: 100%; margin-top: 10px; }";
    html += "th { background-color: #3498db; color: white; padding: 8px; text-align: left; }";
    html += "td { border: 1px solid #ddd; padding: 8px; }";
    html += "tr:nth-child(even) { background-color: #f2f2f2; }";
    html += ".info-box { background-color: #e8f4f8; border-left: 4px solid #3498db; padding: 10px; margin: 10px 0; }";
    html += ".code { background-color: #f5f5f5; padding: 5px; font-family: monospace; border-radius: 3px; }";
    html += "</style></head><body>";

    // Title
    html += QString("<h1>%1</h1>").arg(typeName);

    // QuanSet Description
    QString quansetDesc = QString::fromStdString(quanset.Description());
    if (!quansetDesc.isEmpty()) {
        html += "<div class='info-box'>";
        html += QString("<strong>Description:</strong> %1").arg(quansetDesc);
        html += "</div>";
    }

    // Summary info
    if (quanset.size() > 0) {
        html += "<div class='info-box'>";
        html += QString("<strong>Object Type:</strong> %1<br>").arg(typeName);
        html += QString("<strong>Number of Properties:</strong> %1").arg(quanset.size());
        html += "</div>";
    }

    // Properties table
    if (quanset.size() > 0) {
        html += "<h2>Properties</h2>";
        html += "<table>";
        html += "<tr><th>Property Name</th><th>Type</th><th>Description</th><th>Details</th></tr>";

        for (auto prop_it = quanset.begin(); prop_it != quanset.end(); ++prop_it)
        {
            QString propName = QString::fromStdString(prop_it->first);
            const Quan& quan = prop_it->second;

            // Get type
            QString propType = QString::fromStdString(tostring(quan.GetType()));

            // Get description
            QString propDesc = QString::fromStdString(quan.Description());
            if (propDesc.isEmpty()) {
                propDesc = "<i>No description available</i>";
            }

            QString details = "";
            if (quan.GetType() == Quan::_type::expression) {
                QString expr = QString::fromStdString(quan.GetExpression().ToString());
                if (!expr.isEmpty()) {
                    details = QString("<span class='code'>%1</span>").arg(expr);
                }
                else {
                    QString helpText = QString::fromStdString(quan.HelpText());
                    if (!helpText.isEmpty()) {
                        details = helpText;
                    }
                }
            }
            else if (quan.GetType() == Quan::_type::timeseries || quan.GetType() == Quan::_type::prec_timeseries) {
                details = "<i>Time series</i>";
            }
            else if (quan.GetType() == Quan::_type::source) {
                QString srcName = QString::fromStdString(quan.SourceName());
                if (!srcName.isEmpty()) {
                    details = QString("Source: %1").arg(srcName);
                }
            }

            // If details is still empty, try to use HelpText
            if (details.isEmpty()) {
                QString helpText = QString::fromStdString(quan.HelpText());
                if (!helpText.isEmpty()) {
                    details = helpText;
                }
            }

            html += QString("<tr><td><strong>%1</strong></td><td>%2</td><td>%3</td><td>%4</td></tr>")
                        .arg(propName)
                        .arg(propType)
                        .arg(propDesc)
                        .arg(details);
        }

        html += "</table>";
    } else {
        html += "<p><i>No properties defined for this object type.</i></p>";
    }

    html += "</body></html>";

    return html;
}
QIcon MetaModelHelpDialog::getIconForType(const QString& typeName)
{
    // Try to get icon from the QuanSet's IconFileName
    if (m_metamodel && m_metamodel->count(typeName.toStdString()) > 0) {
        std::string iconFileName = m_metamodel->at(typeName.toStdString()).IconFileName();

        if (!iconFileName.empty()) {
            QString iconPath = QString::fromStdString(iconFileName);

            // If it's not an absolute path, prepend resource directory
            if (!iconPath.startsWith("/") && !m_resourceDirectory.isEmpty()) {
                iconPath = m_resourceDirectory + "/Icons/" + iconPath;
            }

            QIcon icon(iconPath);
            if (!icon.isNull()) {
                return icon;
            }
        }
    }

    // Return default icon
    return QApplication::style()->standardIcon(QStyle::SP_FileIcon);
}

void MetaModelHelpDialog::onItemSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    Q_UNUSED(previous);

    if (!current) {
        return;
    }

    QString typeName = current->data(Qt::UserRole).toString();

    if (m_descriptions.contains(typeName)) {
        m_descriptionBrowser->setHtml(m_descriptions[typeName]);
    }
}

void MetaModelHelpDialog::onCloseClicked()
{
    accept();
}

void MetaModelHelpDialog::onFilterTextChanged(const QString& text)
{
    QString filterText = text.trimmed().toLower();

    // Show/hide items based on filter
    for (int i = 0; i < m_listWidget->count(); ++i) {
        QListWidgetItem* item = m_listWidget->item(i);
        QString itemText = item->text().toLower();

        if (filterText.isEmpty() || itemText.contains(filterText)) {
            item->setHidden(false);
        } else {
            item->setHidden(true);
        }
    }
}

void MetaModelHelpDialog::setResourceDirectory(const QString& path)
{
    m_resourceDirectory = path;
}
