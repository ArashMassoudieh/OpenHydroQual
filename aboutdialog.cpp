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

#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QIcon>

AboutDialog::AboutDialog(QWidget* parent) :
    QDialog(parent)
{
    setupUI();
    setWindowTitle("About OpenHydroQual");
    setMinimumSize(600, 500);

    // Set default content
    SetAboutText(
        "<h2>OpenHydroQual</h2>"
        "<p>Environmental Modeling Platform</p>"
        "<p>Copyright \u00A9 2025 EnviroInformatics, LLC</p>"
        "<p><b>OpenHydroQual</b> is a comprehensive platform for environmental modeling "
        "and simulation, providing tools for water quality analysis, hydrological modeling, "
        "and environmental system assessment.</p>"
    );

    SetLicenseText(
        "<h3>GNU Affero General Public License v3.0</h3>"
        "<p>OpenHydroQual is free software: you can redistribute it and/or modify it "
        "under the terms of the GNU Affero General Public License as published by "
        "the Free Software Foundation, either version 3 of the License, or (at your "
        "option) any later version.</p>"
        "<p>This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
        "GNU Affero General Public License for more details.</p>"
        "<br>"
        "<h3>Commercial Use</h3>"
        "<p><b>Free for Commercial Use:</b> You may use this software for commercial purposes "
        "at no cost under the terms of the AGPL v3.0 license.</p>"
        "<p><b>Commercial License Required:</b> If you wish to incorporate this software into "
        "a commercial product or distribute it as part of a proprietary software solution, "
        "you must purchase a commercial license.</p>"
        "<p><b>Contact:</b> For commercial licensing inquiries, please contact "
        "arash.massoudieh@enviroinformatics.co</p>"
    );

    SetCreditsText(
        "<h3>Development Team</h3>"
        "<p><b>Lead Developer:</b> Arash Massoudieh</p>"
        "<p><b>Email:</b> arash.massoudieh@enviroinformatics.co</p>"
        "<br>"
        "<h3>Third-Party Libraries</h3>"
        "<p>OpenHydroQual makes use of the following open-source libraries:</p>"
        "<ul>"
        "<li>Qt Framework</li>"
        "<li>Additional libraries and dependencies</li>"
        "</ul>"
    );
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Header section with logo and version - COMPACT
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    logoLabel = new QLabel(this);
    logoLabel->setFixedSize(64, 64);  // Smaller icon: 48x48 instead of 64x64
    // Set application icon if available
    QIcon appIcon = windowIcon();
    if (!appIcon.isNull()) {
        logoLabel->setPixmap(appIcon.pixmap(48, 48));
    }
    else {
        logoLabel->setText("OHQ");
        logoLabel->setAlignment(Qt::AlignCenter);
        logoLabel->setStyleSheet(
            "QLabel { "
            "background-color: #2196F3; "
            "color: white; "
            "font-size: 16pt; "  // Smaller font
            "font-weight: bold; "
            "border-radius: 6px; "
            "}"
        );
    }

    versionLabel = new QLabel("Version 1.0.0", this);
    versionLabel->setStyleSheet("font-size: 9pt; color: #666;");

    headerLayout->addWidget(logoLabel);

    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);  // Minimal spacing
    QLabel* titleLabel = new QLabel("<h2 style='margin:0; padding:0;'>OpenHydroQual</h2>", this);  // Use h2 instead of h1
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(versionLabel);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);
    // Don't add extra spacing here - remove the addSpacing(10) line

    // Tab widget - THIS SHOULD EXPAND TO FILL SPACE
    tabWidget = new QTabWidget(this);

    // About tab
    aboutBrowser = new QTextBrowser(this);
    aboutBrowser->setOpenExternalLinks(true);
    aboutBrowser->setStyleSheet(
        "QTextBrowser { "
        "border: 1px solid #ddd; "
        "border-radius: 4px; "
        "padding: 10px; "
        "background-color: white; "
        "}"
    );
    tabWidget->addTab(aboutBrowser, "About");

    // License tab
    licenseBrowser = new QTextBrowser(this);
    licenseBrowser->setOpenExternalLinks(true);
    licenseBrowser->setStyleSheet(
        "QTextBrowser { "
        "border: 1px solid #ddd; "
        "border-radius: 4px; "
        "padding: 10px; "
        "background-color: white; "
        "font-family: monospace; "
        "}"
    );
    tabWidget->addTab(licenseBrowser, "License");

    // Credits tab
    creditsBrowser = new QTextBrowser(this);
    creditsBrowser->setOpenExternalLinks(true);
    creditsBrowser->setStyleSheet(
        "QTextBrowser { "
        "border: 1px solid #ddd; "
        "border-radius: 4px; "
        "padding: 10px; "
        "background-color: white; "
        "}"
    );
    tabWidget->addTab(creditsBrowser, "Credits");

    // General text tab (for AppendText functionality)
    generalTextBrowser = new QTextBrowser(this);
    generalTextBrowser->setOpenExternalLinks(true);
    generalTextBrowser->setStyleSheet(
        "QTextBrowser { "
        "border: 1px solid #ddd; "
        "border-radius: 4px; "
        "padding: 10px; "
        "background-color: white; "
        "}"
    );
    tabWidget->addTab(generalTextBrowser, "Information");

    // Add tab widget with stretch factor so it takes all remaining space
    mainLayout->addWidget(tabWidget, 1);  // The "1" is the stretch factor

    // Close button - COMPACT
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    closeButton = new QPushButton("Close", this);
    closeButton->setMinimumWidth(100);
    closeButton->setFixedHeight(32);  // Fixed height for button
    closeButton->setStyleSheet(
        "QPushButton { "
        "padding: 6px 20px; "  // Reduced padding
        "background-color: #2196F3; "
        "color: white; "
        "border: none; "
        "border-radius: 4px; "
        "font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "background-color: #1976D2; "
        "} "
        "QPushButton:pressed { "
        "background-color: #0D47A1; "
        "}"
    );
    connect(closeButton, &QPushButton::clicked, this, &AboutDialog::accept);

    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}
void AboutDialog::AppendText(const QString& text)
{
    generalTextBrowser->append(text);
}

void AboutDialog::ClearText()
{
    generalTextBrowser->clear();
}

void AboutDialog::SetVersion(const QString& version)
{
    versionLabel->setText("Version " + version);
}

void AboutDialog::SetAboutText(const QString& text)
{
    aboutBrowser->setHtml(text);
}

void AboutDialog::SetLicenseText(const QString& text)
{
    licenseBrowser->setHtml(text);
}

void AboutDialog::SetCreditsText(const QString& text)
{
    creditsBrowser->setHtml(text);
}

