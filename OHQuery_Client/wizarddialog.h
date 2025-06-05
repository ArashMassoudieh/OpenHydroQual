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


#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#include <QDialog>
#include "Wizard_Script.h"
#include <QScrollArea>
#include <QTabWidget>
#include <QFormLayout>
#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include "svgviewer.h"

struct tab {
    QScrollArea *scrollArea;
    QFormLayout *formlayout;
    QWidget *tab;
    QHBoxLayout *horizontalLayout;
    QWidget *scrollAreaWidgetContents;
    QFormLayout *formLayout;
    WizardParameterGroup *parametergroup;
};
class RosettaFetcher;
class QComboBox;
namespace Ui {
class WizardDialog;
}

class WizardDialog : public QWidget
{
    Q_OBJECT

public:
    explicit WizardDialog(QWidget *parent = nullptr);
    void CreateItems(WizardScript *wizscript);
    ~WizardDialog();
    void PopulateTab(QWidget *scrollAreaWidgetContents, QFormLayout *formLayout, WizardParameterGroup *paramgroup);
    void PopulateTab(WizardParameterGroup *paramgroup);
    void GenerateModel();
    void ParametersToJson();
    bool Verify();
    void resizeEvent(QResizeEvent *r=nullptr);
    QGraphicsSvgItem* fetchSvgAsGraphicsItem(const QUrl& svgUrl);
    void fetchSvgAsync(const QUrl& svgUrl);

private:
    Ui::WizardDialog *ui;
    QMap<QString,tab> tabs;
    WizardScript SelectedWizardScript;
    QPixmap* diagram_pix = nullptr;
    QGraphicsSvgItem *svgitem = nullptr;
    int currenttabindex=0;
    SVGViewer *svgviewer = nullptr;
    RosettaFetcher *rosettaFetcher;
public slots:
    void on_next_clicked();
    void on_previous_clicked();
    void on_TabChanged();
    void open_html();
    void onDataReceived(QComboBox* editor);

signals:
    void model_generate_requested(const QJsonDocument &jsondoc);

};

#endif // WIZARDDIALOG_H
