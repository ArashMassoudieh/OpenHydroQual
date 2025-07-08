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


#include "wizarddialog.h"
#include "ui_wizarddialog.h"
#include <QLabel>
#include <QLineEdit>
#include "UnitTextBox3.h"
#include <QSpinBox>
#include <QStandardPaths>
#include <QDateEdit>
#include "FilePushButton.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSvgRenderer>
#include "rosettafetcher.h"


WizardDialog::WizardDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WizardDialog)
{

    ui->setupUi(this);
    ui->label_version->setText("Version: " +  version);
    this->setStyleSheet("background-color: white;");

    svgviewer = new SVGViewer(this);
    svgviewer->setObjectName(QString::fromUtf8("SVGViewer"));
    svgviewer->setMinimumSize(QSize(300, 0));

    ui->horizontalLayout_2->addWidget(svgviewer);

    ui->horizontalLayout_2->setStretch(0, 3);
    ui->horizontalLayout_2->setStretch(1, 1);

    ui->Previous->setEnabled(false);
    connect(ui->Next, SIGNAL(clicked()),this, SLOT(on_next_clicked()));
    connect(ui->Previous, SIGNAL(clicked()),this, SLOT(on_previous_clicked()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)), this, SLOT(on_TabChanged()));
    connect(svgviewer,SIGNAL(resizeevent()),this, SLOT(fit_diagram()));
    connect(ui->OpenHTML, SIGNAL(clicked()),this,SLOT(open_html()));

}

WizardDialog::~WizardDialog()
{
    delete ui;

}

void WizardDialog::CreateItems(WizardScript *wizscript)
{
    SelectedWizardScript = *wizscript;
    if (SelectedWizardScript.Url().isEmpty())
        ui->OpenHTML->setEnabled(false);
    else
    {   connect(svgviewer,&SVGViewer::doubleClicked,this, &WizardDialog::open_html);
        svgviewer->setToolTip("Double-click to get more detailed explanations.");

    }
    for (QMap<QString,WizardParameterGroup>::Iterator it=wizscript->GetWizardParameterGroups().begin(); it!=wizscript->GetWizardParameterGroups().end(); it++)
    {
        tab this_tab;
        this_tab.tab = new QWidget();
        this_tab.tab->setObjectName(it->Name());

        this_tab.horizontalLayout = new QHBoxLayout(this_tab.tab);
        this_tab.horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        this_tab.scrollArea = new QScrollArea(this_tab.tab);
        this_tab.scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        this_tab.scrollArea->setWidgetResizable(true);
        this_tab.scrollAreaWidgetContents = new QWidget();
        this_tab.scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        this_tab.formLayout = new QFormLayout(this_tab.scrollAreaWidgetContents);
        this_tab.formLayout->setObjectName(it->Description());


        this_tab.scrollArea->setWidget(this_tab.scrollAreaWidgetContents);

        this_tab.horizontalLayout->addWidget(this_tab.scrollArea);

        ui->tabWidget->addTab(this_tab.tab,it->Description());

        ui->tabWidget->setTabText(ui->tabWidget->indexOf(this_tab.tab), it->Description());
        ui->tabWidget->widget(ui->tabWidget->indexOf(this_tab.tab))->setObjectName(it.value().Name());
        tabs[it->Name()] = this_tab;
        PopulateTab(&it.value());
        tabs[it->Name()].parametergroup = &it.value();

    }
    on_TabChanged();
    QGraphicsScene* scene = new QGraphicsScene();
    svgviewer->setScene(scene);
    if (wizscript->DiagramFileName().split(".").size()>1)
    {
       fetchSvgAsync(QUrl("https://www.greeninfraiq.com/svgs/" + wizscript->DiagramFileName()));
    }

    resizeEvent();
    setWindowState(Qt::WindowMaximized);
    repaint();

}



void WizardDialog::PopulateTab(QWidget *scrollAreaWidgetContents, QFormLayout *formLayout, WizardParameterGroup *paramgroup)
{
    for (int i=0; i<paramgroup->ParametersCount(); i++)
    {
        QLabel *label = new QLabel(scrollAreaWidgetContents);
        label->setObjectName(paramgroup->Parameter(i)+"_label");
        label->setText(SelectedWizardScript.GetWizardParameters()[paramgroup->Parameter(i)].Question());

        QLineEdit *lineEdit = new QLineEdit(scrollAreaWidgetContents);
        lineEdit->setObjectName(paramgroup->Parameter(i)+"_lineedit");
        formLayout->addRow(label,lineEdit);



    }
}

void WizardDialog::PopulateTab(WizardParameterGroup *paramgroup)
{
    tab this_tab = tabs[paramgroup->Name()];
    for (int i=0; i<paramgroup->ParametersCount(); i++)
    {
        WizardParameter *parameter = &SelectedWizardScript.GetWizardParameters()[paramgroup->Parameter(i)];
        if (parameter->Delegate() != "")
        {
            QLabel* label = new QLabel(this_tab.scrollAreaWidgetContents);
            label->setObjectName(paramgroup->Parameter(i) + "_label");
            label->setText(parameter->Question());
            if (parameter->Delegate() == "ValueBox")
            {
                QLineEdit* Editor = new QLineEdit(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                Editor->setText(parameter->Default());
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "UnitBox")
            {
                UnitTextBox3* Editor = new UnitTextBox3(QStyleOptionViewItem(), this_tab.scrollAreaWidgetContents);
                Editor->setUnitsList(parameter->Units());
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                Editor->setMinimumHeight(30);
                Editor->setMinimumWidth(100);
                Editor->setText(parameter->Default());
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "SpinBox")
            {
                QSpinBox* Editor = new QSpinBox(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                Editor->setRange(parameter->Range()[0], parameter->Range()[1]);
                Editor->setMinimumWidth(100);
                Editor->setValue(parameter->Default().toInt());
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "ComboBox")
            {
                QComboBox* Editor = new QComboBox(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                Editor->addItems(parameter->ComboItems());
                Editor->setCurrentText(parameter->Default());
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "DateBox")
            {
                QDateEdit* Editor = new QDateEdit(this_tab.scrollAreaWidgetContents);
                Editor->setDisplayFormat("MM.dd.yyyy");
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate() == "FileBrowser")
            {
                FilePushButton* Editor = new FilePushButton(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
            else if (parameter->Delegate().contains("CombofromAPI"))
            {
                QComboBox* Editor = new QComboBox(this_tab.scrollAreaWidgetContents);
                Editor->setObjectName(paramgroup->Parameter(i) + "_edit");
                RosettaFetcher *rosettaFetcher = new RosettaFetcher();
                rosettaFetchers[parameter->Name()] = rosettaFetcher;
                Editor->setProperty("SoilMap",parameter->Delegate().split("|").last());
                rosettaFetcher->fetchJson(QUrl(parameter->Delegate().split("|")[1]));
                Editor->setProperty("Parameter",parameter->Name());
                connect(rosettaFetcher, &RosettaFetcher::dataReady, this, [this, Editor]() {
                    onDataReceived(Editor);
                });
                this_tab.formLayout->addRow(label, Editor);
                parameter->SetEntryItem(Editor);
            }
        }
    }
}

void WizardDialog::onDataReceived(QComboBox* editor) {
    QString parameter = SelectedWizardScript.GetWizardParameters()[editor->property("Parameter").toString()].Name();
    editor->clear();
    editor->addItems(rosettaFetchers[parameter]->getTextureClasses());
    editor->setCurrentText(SelectedWizardScript.GetWizardParameters()[editor->property("Parameter").toString()].Default());
    connect(editor, &QComboBox::currentTextChanged, this,
            [this, editor](const QString &text) {
                onComboChanged(editor, text);
            });
    onComboChanged(editor,editor->currentText());
}

void WizardDialog::onComboChanged(QComboBox* editor, const QString& text)
{
    QString parameter = SelectedWizardScript.GetWizardParameters()[editor->property("Parameter").toString()].Name();
    QMap<QString, double> parametervalues = rosettaFetchers[parameter]->getData()[text];
    QMap<QString, QString> parameter_map = SelectedWizardScript.GetParameterPopulateMaps(editor->property("SoilMap").toString()) ;
    for (QMap<QString,double>::Iterator it = parametervalues.begin(); it != parametervalues.end(); ++it) {
        qDebug() << "Key:" << it.key() << "Value:" << it.value();

        if (parameter_map.contains(it.key()))
        {
            WizardParameter *entry_param = &SelectedWizardScript.GetWizardParameters()[parameter_map[it.key()]];
            if (entry_param->Delegate() == "ValueBox")
            {
                static_cast<QLineEdit*>(entry_param->EntryItem())->setText(QString::number(it.value()));
            }
            else if (entry_param->Delegate() == "UnitBox")
            {
                static_cast<UnitTextBox3*>(entry_param->EntryItem())->setText(QString::number(it.value()));
            }

        }

    }


}

void WizardDialog::on_next_clicked()
{
    if (ui->tabWidget->currentIndex()<ui->tabWidget->count()-1)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()+1);
    else
    {
        GenerateModel();
    }
    on_TabChanged();
}
void WizardDialog::on_previous_clicked()
{
    if (ui->tabWidget->currentIndex()>0)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()-1);

    on_TabChanged();
}

void WizardDialog::on_TabChanged()
{
    if (currenttabindex == ui->tabWidget->currentIndex())
        return;

    SelectedWizardScript.AssignParameterValues();
    QStringList Errors = SelectedWizardScript.GetWizardParameterGroups()[ui->tabWidget->widget(currenttabindex)->objectName()].CheckCriteria(&SelectedWizardScript.GetWizardParameters());

    if (!Errors.isEmpty())
    {
        auto *msgBox = new QMessageBox(QMessageBox::Information,
                                       "Invalid parameter value!",
                                       Errors[0],
                                       QMessageBox::Ok,
                                       this);

        connect(msgBox, &QMessageBox::finished, this, [=](int){
            ui->tabWidget->setCurrentIndex(currenttabindex);  // reset only after user acknowledges
            msgBox->deleteLater();
        });

        msgBox->open();  // asynchronous and safe in WebAssembly
        return;
    }

    ui->Previous->setEnabled(ui->tabWidget->currentIndex() != 0);
    ui->Next->setText(ui->tabWidget->currentIndex() == ui->tabWidget->count() - 1 ? "Create Model" : "Next");
    currenttabindex = ui->tabWidget->currentIndex();
}


bool  WizardDialog::Verify()
{
    SelectedWizardScript.AssignParameterValues();
    QStringList Errors;
    for (QMap<QString,WizardParameterGroup>::Iterator it=SelectedWizardScript.GetWizardParameterGroups().begin(); it!=SelectedWizardScript.GetWizardParameterGroups().end(); it++)
        Errors << SelectedWizardScript.GetWizardParameterGroups()[it.key()].CheckCriteria(&SelectedWizardScript.GetWizardParameters());


    if (Errors.count()>0)
    {   QMessageBox::information(this, "Invalid parameter value!", Errors[0], QMessageBox::Ok, QMessageBox::StandardButton::Ok);
        ui->tabWidget->setCurrentIndex(currenttabindex);
        return false;
    }
    return true;

}

void WizardDialog::GenerateModel()
{
    if (!Verify())
    {
        emit model_generate_requested(QJsonDocument());
        return;
    }

    QJsonObject all_parameters;
    SelectedWizardScript.AssignParameterValues();
    for (QMap<QString,WizardParameter>::iterator it=SelectedWizardScript.GetWizardParameters().begin(); it!=SelectedWizardScript.GetWizardParameters().end(); it++)
    {
        all_parameters[it.key()] = it->Value();
    }
    QJsonObject out;
    out["Parameters"] = all_parameters;
    emit model_generate_requested(QJsonDocument(out));

}


void WizardDialog::open_html()
{
    QDesktopServices::openUrl(QUrl(SelectedWizardScript.Url()));
}


void WizardDialog::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (svgitem)
    {   qDebug() << "SVG boundingRect:" << svgitem->boundingRect();
        qDebug() << "Scene rect:" << svgviewer->scene()->sceneRect();
        qDebug() << "Viewer size:" << svgviewer->size();
    }

    if (!svgviewer || !svgviewer->scene())
        return;

    if (diagram_pix)
    {
        QPixmap scaled_img = diagram_pix->scaled(svgviewer->size(), Qt::KeepAspectRatio);
        svgviewer->scene()->clear();
        svgviewer->scene()->addPixmap(scaled_img);
        svgviewer->fitInView(svgviewer->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
    else if (svgitem)
    {
        svgviewer->scene()->setSceneRect(svgitem->boundingRect());
        svgviewer->fitInView(svgitem->boundingRect(), Qt::KeepAspectRatio);
    }
}

QGraphicsSvgItem* WizardDialog::fetchSvgAsGraphicsItem(const QUrl& svgUrl)
{
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply* reply = manager.get(QNetworkRequest(svgUrl));

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return nullptr;
    }

    QByteArray svgData = reply->readAll();
    reply->deleteLater();

    // Load into QSvgRenderer
    QSvgRenderer* renderer = new QSvgRenderer(svgData);
    if (!renderer->isValid()) {
        qWarning() << "Failed to load SVG";
        delete renderer;
        return nullptr;
    }

    // Create QGraphicsSvgItem
    QGraphicsSvgItem* svgItem = new QGraphicsSvgItem();
    svgItem->setSharedRenderer(renderer);
    return svgItem;
}

void WizardDialog::fetchSvgAsync(const QUrl& svgUrl)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(svgUrl);
    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Network error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray svgData = reply->readAll();
        reply->deleteLater();

        QSvgRenderer* renderer = new QSvgRenderer(svgData, this);
        if (!renderer->isValid()) {
            qWarning() << "Invalid SVG data";
            delete renderer;
            return;
        }

        svgitem = new QGraphicsSvgItem();
        svgitem->setSharedRenderer(renderer);
        svgviewer->scene()->clear();
        svgviewer->scene()->addItem(svgitem);
        svgviewer->scene()->setSceneRect(svgitem->boundingRect());
        svgviewer->fitInView(svgitem->boundingRect(), Qt::KeepAspectRatio);
    });
}

void WizardDialog::SetDisabled(bool state)
{
    if (state)
    {
        ui->Next->setEnabled(false);
        ui->Previous->setEnabled(false);
        ui->tabWidget->setEnabled(false);

        QString disabledStyle = "background-color: lightgray;";

        ui->Next->setStyleSheet(disabledStyle);
        ui->Previous->setStyleSheet(disabledStyle);
    }
    else
    {
        ui->Next->setEnabled(true);
        ui->Previous->setEnabled(true);
        ui->tabWidget->setEnabled(true);

        ui->Next->setStyleSheet("");      // Reset to default
        ui->Previous->setStyleSheet("");  // Reset to default
    }
}
