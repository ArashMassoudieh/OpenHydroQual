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

 


WizardDialog::WizardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizardDialog)
{
    ui->setupUi(this);
    ui->Previous->setEnabled(false);
    connect(ui->Next, SIGNAL(clicked()),this, SLOT(on_next_clicked()));
    connect(ui->Previous, SIGNAL(clicked()),this, SLOT(on_previous_clicked()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)), this, SLOT(on_TabChanged()));
    connect(ui->graphicsView,SIGNAL(resizeevent()),this, SLOT(fit_diagram()));

}

WizardDialog::~WizardDialog()
{
    delete ui;

}

void WizardDialog::CreateItems(WizardScript *wizscript)
{
    SelectedWizardScript = *wizscript;
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
        this_tab.scrollAreaWidgetContents->setGeometry(QRect(0, 0, 528, 308));
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
    ui->graphicsView->setScene(scene);
    if (wizscript->DiagramFileName().split(".")[1]=="png")
        diagram_pix = new QPixmap(QString::fromStdString(wizardsfolder)+"Diagrams/"+wizscript->DiagramFileName());
    else if (wizscript->DiagramFileName().split(".")[1]=="svg")
    {   svgitem = new QGraphicsSvgItem(QString::fromStdString(wizardsfolder)+"Diagrams/"+wizscript->DiagramFileName());
        qDebug()<<scene->sceneRect();
        ui->graphicsView->scene()->clear();
        ui->graphicsView->scene()->addItem(svgitem);
    }

    resizeEvent();
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

void  WizardDialog::on_TabChanged()
{
    if (currenttabindex==ui->tabWidget->currentIndex())
        return;
    SelectedWizardScript.AssignParameterValues();
    QStringList Errors = SelectedWizardScript.GetWizardParameterGroups()[ui->tabWidget->widget(currenttabindex)->objectName()].CheckCriteria(&SelectedWizardScript.GetWizardParameters());
    if (Errors.count()>0)
    {   QMessageBox::information(this, "Invalid parameter value!", Errors[0], QMessageBox::Ok, QMessageBox::StandardButton::Ok);
        ui->tabWidget->setCurrentIndex(currenttabindex);
        return;
    }

    if (ui->tabWidget->currentIndex()==0)
        ui->Previous->setEnabled(false);
    else
        ui->Previous->setEnabled(true);
    if (ui->tabWidget->currentIndex()==ui->tabWidget->count()-1)
        ui->Next->setText("Create Model");
    else
        ui->Next->setText("Next");
    currenttabindex = ui->tabWidget->currentIndex();
}

void WizardDialog::GenerateModel()
{
    SelectedWizardScript.AssignParameterValues();
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        tr("OpenHydroQual files (*.ohq)"),nullptr,QFileDialog::DontUseNativeDialog);
    QStringList script = SelectedWizardScript.Script();

    if (fileName != "")
    {
        if (!fileName.contains("."))
            fileName = fileName + ".ohq";
        else if (fileName.split('.')[fileName.split('.').size() - 1] != "ohq")
        {
            fileName = fileName + ".ohq";
        }

        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        QTextStream qout(&file);
        for (int i = 0; i < script.count(); i++)
            qout << script[i] + "\n";

        file.close();
    }

}

void WizardDialog::resizeEvent(QResizeEvent *)
{

    QSize gvs  = ui->graphicsView->size();
    if (diagram_pix!=nullptr)
    {   QPixmap scaled_img = diagram_pix->scaled(gvs, Qt::IgnoreAspectRatio);
        ui->graphicsView->scene()->clear();
        ui->graphicsView->scene()->addPixmap(scaled_img);
    }
    else if (svgitem!=nullptr)
    {
        QRectF newRect = ui->graphicsView->scene()->itemsBoundingRect();
        float width = float(newRect.width());
        float height = float(newRect.height());
        float scale = float(1.05);
        newRect.setLeft(newRect.left() - float(scale - 1) / 2 * float(width));
        newRect.setTop(newRect.top() - (scale - 1) / 2 * height);
        newRect.setWidth(qreal(width * scale));
        newRect.setHeight(qreal(height * scale));
        if (width>ui->graphicsView->scene()->sceneRect().width() || height>ui->graphicsView->scene()->sceneRect().height())
            ui->graphicsView->scene()->setSceneRect(newRect);
        ui->graphicsView->fitInView(newRect,Qt::KeepAspectRatio);
        ui->graphicsView->repaint();
    }
}
