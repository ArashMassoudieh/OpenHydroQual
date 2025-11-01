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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();

    void AppendText(const QString& text);
    void ClearText();
    void SetVersion(const QString& version);
    void SetAboutText(const QString& text);
    void SetLicenseText(const QString& text);
    void SetCreditsText(const QString& text);

private:
    void setupUI();

    QTabWidget* tabWidget;
    QTextBrowser* aboutBrowser;
    QTextBrowser* licenseBrowser;
    QTextBrowser* creditsBrowser;
    QTextBrowser* generalTextBrowser;
    QPushButton* closeButton;
    QLabel* logoLabel;
    QLabel* versionLabel;
};

#endif // ABOUTDIALOG_H