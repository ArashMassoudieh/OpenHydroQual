#include "gridgenerator.h"
#include "ui_gridgenerator.h"
#include "mainwindow.h"
#include "UnitTextBox.h"
#include "utilityfuncs.h"
#include "expEditor.h"

GridGenerator::GridGenerator(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::GridGenerator)
{
    ui->setupUi(this);
    mainwindow = parent;
    PopulateBlocks();
    PopulateLinks(); 
    connect(ui->listWidgetBlocks,SIGNAL(itemSelectionChanged()),this,SLOT(on_Selected_item_changed()));
    connect(ui->listWidgetLinks,SIGNAL(itemSelectionChanged()),this,SLOT(on_Selected_item_changed()));
    ui->pushButtonNext->setEnabled(false);
}

GridGenerator::~GridGenerator()
{
    delete ui;
}

void GridGenerator::PopulateBlocks()
{

    ui->listWidgetBlocks->setIconSize(QSize(50, 50));
    for (unsigned int i = 0; i < system()->GetAllBlockTypes().size(); i++)
    {

        //qDebug() << QString::fromStdString(system.GetAllBlockTypes()[i]);

        QIcon icon;
        if (QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()).contains("/"))
        {
            if (QFile::exists(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {
            if (QFile::exists(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllBlockTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);

        }

        QListWidgetItem *list_item = new QListWidgetItem(icon, QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        list_item->setData(1000,QString::fromStdString(system()->GetAllBlockTypes()[i]));
        //qDebug()<<QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        //string _text = system()->GetModel(system()->GetAllBlockTypes()[i])->Description();
        //QString text = QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        list_item->setToolTip(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        list_item->setText(QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description()));
        ui->listWidgetBlocks->addItem(list_item);
    }

}
void GridGenerator::PopulateLinks()
{
    ui->listWidgetLinks->setIconSize(QSize(50, 50));
    for (unsigned int i = 0; i < system()->GetAllLinkTypes().size(); i++)
    {

        QIcon icon;
        if (QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()).contains("/"))
        {
            if (QFile::exists(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);
        }
        else
        {
            if (QFile::exists(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName())))
                icon.addFile(QString::fromStdString(qApp->applicationDirPath().toStdString() + "/../../resources/Icons/" + system()->GetModel(system()->GetAllLinkTypes()[i])->IconFileName()), QSize(), QIcon::Normal, QIcon::Off);

        }

        QListWidgetItem* list_item = new QListWidgetItem(icon, QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        list_item->setData(1000,QString::fromStdString(system()->GetAllLinkTypes()[i]));
        //qDebug()<<QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        //string _text = system()->GetModel(system()->GetAllBlockTypes()[i])->Description();
        //QString text = QString::fromStdString(system()->GetModel(system()->GetAllBlockTypes()[i])->Description());
        list_item->setToolTip(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        list_item->setText(QString::fromStdString(system()->GetModel(system()->GetAllLinkTypes()[i])->Description()));
        ui->listWidgetLinks->addItem(list_item);
    }
}

System * GridGenerator::system()
{
    if (mainwindow!=nullptr)
        return mainwindow->GetSystem();
    else
        return nullptr;
}

void GridGenerator::on_Selected_item_changed()
{
    ClearPropertiesWindow();
    if (ui->listWidgetBlocks->selectedItems().count()>0)
        Block_type_selected = ui->listWidgetBlocks->selectedItems()[0]->data(1000).toString();
    else
        return;

    if (ui->listWidgetLinks->selectedItems().count()>0)
        Link_type_selected = ui->listWidgetBlocks->selectedItems()[0]->data(1000).toString();
    else
        return;

    QuanSet blockQS = *system()->GetModel(Block_type_selected.toStdString());
    QuanSet linkQS = *system()->GetModel(Link_type_selected.toStdString());

    for (unsigned int i=0; i<blockQS.AskableSize(); i++)
    {
        QLabel *label = new QLabel;
        label->setText(QString::fromStdString(blockQS.GetVarAskable(i)->GetName()));
        BlockPropertiesLabels.append(label);
        ui->gridLayout->addWidget(label,i+1,0);

        if (blockQS.GetVarAskable(i)->Delegate()=="UnitBox")
        {   UnitTextBox *unitBox = new UnitTextBox(QStyleOptionViewItem() ,this);
            unitBox->setText(QString::fromStdString(blockQS.GetVarAskable(i)->Default()));
            unitBox->setUnit(QString::fromStdString(blockQS.GetVarAskable(i)->Unit()));
            unitBox->setUnitsList(XString::reform(QString::fromStdString(blockQS.GetVarAskable(i)->Units()).split(";")));
            BlockPropertiesValues.append(unitBox);
            ui->gridLayout->addWidget(unitBox,i+1,1);
        }
        else if (blockQS.GetVarAskable(i)->Delegate()=="ValueBox")
        {   QLineEdit *valueBox = new QLineEdit();
            valueBox->setText(QString::fromStdString(blockQS.GetVarAskable(i)->Default()));
            BlockPropertiesValues.append(valueBox);
            ui->gridLayout->addWidget(valueBox,i+1,1);
        }
        else if (blockQS.GetVarAskable(i)->Delegate() == "MultiComboBox")
        {
            QListWidget *editor = new QListWidget(this);
            QStringList allItems;
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            foreach (QString item , allItems)
            {
                QListWidgetItem* wi = new QListWidgetItem(item);
                //wi.setText(item);
                editor->addItem(wi);
            }
            editor->setSelectionMode(QAbstractItemView::MultiSelection);
            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);
        }
        else if (blockQS.GetVarAskable(i)->Delegate() == "ComboBox")
        {
            QComboBox *editor = new QComboBox(this);
            editor->setFrame(false);
            QStringList allItems;
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            editor->addItems(allItems);

            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);
        }
        else if (blockQS.GetVarAskable(i)->Delegate() == "CheckBox")
        {
            QCheckBox *editor = new QCheckBox(this);
            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);
        }
        else if (blockQS.GetVarAskable(i)->Delegate() == "DirBrowser")
        {
            QPushButton *editor = new QPushButton(this);
            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);
        }
        else if (blockQS.GetVarAskable(i)->Delegate() == "Browser")
        {
            QPushButton *editor = new QPushButton(this);
            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);

        }
        else if (blockQS.GetVarAskable(i)->Delegate() == "ListBox")
        {
            QComboBox *editor = new QComboBox(this);
            //connect(editor, SIGNAL(clicked()), this, SLOT(browserClicked()));
            QStringList allItems;
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(blockQS.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            editor->addItems(allItems);
            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);
        }
        else if (blockQS.GetVarAskable(i)->Delegate() == "Memo")
        {
            QTextEdit* editor = new QTextEdit(this);
            editor->setWordWrapMode(QTextOption::NoWrap);
            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);
        }

        else if (blockQS.GetVarAskable(i)->Delegate() == "expressionEditor")
        {
            QStringList words;
            for (unsigned int i=0; i<blockQS.AllConstituents().size(); i++)
                 words << QString::fromStdString(blockQS.AllConstituents()[i]);

            for (unsigned int i=0; i<blockQS.AllReactionParameters().size(); i++)
                 words << QString::fromStdString(blockQS.AllReactionParameters()[i]);

            QString objname;
            if (blockQS.Count("object")>0)
                objname = QString::fromStdString(blockQS.GetVar("object").GetProperty());

            Object *obj = nullptr;
            if (objname!="")
                obj = system()->object(objname.toStdString());

            expEditor* editor = new expEditor(obj, words, nullptr, this);
            ui->gridLayout->addWidget(editor,i+1,1);
            BlockPropertiesValues.append(editor);
        }
        else
        {
            QLineEdit *valueBox = new QLineEdit();
            valueBox->setText(QString::fromStdString(blockQS.GetVarAskable(i)->Default()));
            BlockPropertiesValues.append(valueBox);
            ui->gridLayout->addWidget(valueBox,i+1,1);
            BlockPropertiesValues.append(valueBox);
        }

    }
    ui->tab_2->setEnabled(true);
    ui->pushButtonNext->setEnabled(true);
}

void GridGenerator::ClearPropertiesWindow()
{
    for (int i=0; i<BlockPropertiesLabels.count(); i++)
        ui->gridLayout->removeWidget(BlockPropertiesLabels[i]);
    for (int i=0; i<BlockPropertiesValues.count(); i++)
        ui->gridLayout->removeWidget(BlockPropertiesValues[i]);



}
