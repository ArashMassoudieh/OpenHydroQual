#include "gridgenerator.h"
#include "ui_gridgenerator.h"
#include "mainwindow.h"
#include "UnitTextBox3.h"
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
    connect(ui->listWidgetBlocks,SIGNAL(itemSelectionChanged()),this,SLOT(on_Selected_block_changed()));
    connect(ui->listWidgetLinks,SIGNAL(itemSelectionChanged()),this,SLOT(on_Selected_link_changed()));
    connect(ui->pushButtonNext,SIGNAL(clicked()),this,SLOT(on_NextClicked()));
    connect(ui->pushButtonPrevious,SIGNAL(clicked()),this,SLOT(on_PreviousClicked()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)), this, SLOT(on_TabChanged()));
    ui->pushButtonNext->setEnabled(false);
    ui->tabWidget->setCurrentIndex(0);
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

void GridGenerator::on_Selected_block_changed()
{
    ClearPropertiesWindow(objectType::block);
    if (ui->listWidgetBlocks->selectedItems().count()>0)
        Block_type_selected = ui->listWidgetBlocks->selectedItems()[0]->data(1000).toString();
    else
    {
        ui->tab_BlockProperties->setEnabled(false);
        return;

    }

    QuanSet blockQS = *system()->GetModel(Block_type_selected.toStdString());

    PopulatePropertiesTab(blockQS, ui->gridLayout_Block, objectType::block);

    ui->tab_BlockProperties->setEnabled(true);
    ui->pushButtonNext->setEnabled(true);

}

void GridGenerator::on_Selected_link_changed()
{
    ClearPropertiesWindow(objectType::link);

    if (ui->listWidgetLinks->selectedItems().count()>0)
        Link_type_selected = ui->listWidgetLinks->selectedItems()[0]->data(1000).toString();
    else
    {   ui->tab_LinkProperties->setEnabled(false);
        return;
    }

    QuanSet linkQS = *system()->GetModel(Link_type_selected.toStdString());

    PopulatePropertiesTab(linkQS, ui->gridLayout_Link, objectType::link);

    ui->tab_LinkProperties->setEnabled(true);
    ui->pushButtonNext->setEnabled(true);

}


void GridGenerator::PopulatePropertiesTab(QuanSet& qs, QGridLayout *layout, objectType ObjectType)
{

    for (unsigned int i=0; i<qs.AskableSize(); i++)
    {
        QLabel *label = new QLabel;
        label->setText(QString::fromStdString(qs.GetVarAskable(i)->Description()));
        if (ObjectType == objectType::block)
            BlockPropertiesLabels.append(label);
        else if (ObjectType == objectType::link)
            LinkPropertiesLabels.append(label);
        layout->addWidget(label,i+1,0);

        if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("UnitBox"))
        {
            UnitTextBox3 *editor = new UnitTextBox3(QStyleOptionViewItem() ,this);
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));
            editor->setUnit(QString::fromStdString(qs.GetVarAskable(i)->Unit()));
            editor->setUnitsList(XString::reform(QString::fromStdString(qs.GetVarAskable(i)->Units()).split(";")));
            editor->setMaximumSize(max_size_x,max_size_y);

            layout->addWidget(editor,i+1,1);
            UnitTextBox3 *incrementTxtBox = new UnitTextBox3(QStyleOptionViewItem(),false ,this);
            incrementTxtBox->setUnit(QString::fromStdString(qs.GetVarAskable(i)->Unit()));
            incrementTxtBox->setUnitsList(XString::reform(QString::fromStdString(qs.GetVarAskable(i)->Units()).split(";")));
            if (ObjectType == objectType::block)
            {   BlockPropertiesIncrements.append(incrementTxtBox);
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesIncrements.append(incrementTxtBox);
                LinkPropertiesValues.append(editor);
            }
            incrementTxtBox->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(incrementTxtBox,i+1,2);
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("ValueBox"))
        {   QLineEdit *editor = new QLineEdit();
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            QLineEdit *incrementTxtBox = new QLineEdit();
            if (ObjectType == objectType::block)
            {   BlockPropertiesIncrements.append(incrementTxtBox);
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesIncrements.append(incrementTxtBox);
                LinkPropertiesValues.append(editor);
            }
            layout->addWidget(incrementTxtBox,i+1,2);
            incrementTxtBox->setMaximumSize(max_size_x,max_size_y);
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("String"))
        {   QLineEdit *editor = new QLineEdit();
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("MultiComboBox"))
        {
            QListWidget *editor = new QListWidget(this);
            QStringList allItems;
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(qs.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            foreach (QString item , allItems)
            {
                QListWidgetItem* wi = new QListWidgetItem(item);
                //wi.setText(item);
                editor->addItem(wi);
            }
            editor->setSelectionMode(QAbstractItemView::MultiSelection);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("ComboBox"))
        {
            QComboBox *editor = new QComboBox(this);
            editor->setFrame(false);
            QStringList allItems;
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(qs.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            editor->addItems(allItems);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("CheckBox"))
        {
            QCheckBox *editor = new QCheckBox(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("DirBrowser"))
        {
            QPushButton *editor = new QPushButton(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            QObject::connect(editor, SIGNAL(clicked()), this, SLOT(dirBrowserClicked()));
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Browser"))
        {
            QPushButton *editor = new QPushButton(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            QObject::connect(editor, SIGNAL(clicked()), this, SLOT(browserClicked()));
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }

        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("ListBox"))
        {
            QComboBox *editor = new QComboBox(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            //connect(editor, SIGNAL(clicked()), this, SLOT(browserClicked()));
            QStringList allItems;
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Sources"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllSourceNames());
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("BlockLinks"))
            {
                allItems = toQStringList(mainwindow->GetSystem()->GetAllBlockNames());
                allItems.append(toQStringList(mainwindow->GetSystem()->GetAllLinkNames()));
            }
            if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("items"))
            {
                allItems = QString::fromStdString(qs.GetVarAskable(i)->Delegate()).split(":")[1].split(",");
            }
            editor->addItems(allItems);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("Memo"))
        {
            QTextEdit* editor = new QTextEdit(this);
            editor->setMaximumSize(max_size_x,max_size_y);
            editor->setWordWrapMode(QTextOption::NoWrap);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }
        else if (QString::fromStdString(qs.GetVarAskable(i)->Delegate()).contains("expressionEditor"))
        {
            QStringList words;
            for (unsigned int i=0; i<qs.AllConstituents().size(); i++)
                 words << QString::fromStdString(qs.AllConstituents()[i]);

            for (unsigned int i=0; i<qs.AllReactionParameters().size(); i++)
                 words << QString::fromStdString(qs.AllReactionParameters()[i]);

            QString objname;
            if (qs.Count("object")>0)
                objname = QString::fromStdString(qs.GetVar("object").GetProperty());

            Object *obj = nullptr;
            if (objname!="")
                obj = system()->object(objname.toStdString());

            expEditor* editor = new expEditor(obj, words, nullptr, this);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }
        else
        {
            QLineEdit *editor = new QLineEdit();
            editor->setText(QString::fromStdString(qs.GetVarAskable(i)->Default()));
            BlockPropertiesValues.append(editor);
            editor->setMaximumSize(max_size_x,max_size_y);
            layout->addWidget(editor,i+1,1);
            if (ObjectType == objectType::block)
            {
                BlockPropertiesValues.append(editor);
            }
            else if (ObjectType == objectType::block)
            {
                LinkPropertiesValues.append(editor);
            }
        }

    }

    if (ObjectType == objectType::block)
    {
        verticalSpacer_blocks = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        ui->gridLayout_Block->addItem(verticalSpacer_blocks, qs.AskableSize(), 0, 1, 3);

    }
    else if (ObjectType == objectType::link)
    {
        verticalSpacer_links = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        ui->gridLayout_Link->addItem(verticalSpacer_links, qs.AskableSize(), 0, 1, 3);
    }

}


void GridGenerator::ClearPropertiesWindow(objectType ObjectType)
{
    if (ObjectType == objectType::block)
    {   for (int i=0; i<BlockPropertiesLabels.count(); i++)
        {   ui->gridLayout_Block->removeWidget(BlockPropertiesLabels[i]);
            delete BlockPropertiesLabels[i];
        }
        for (int i=0; i<BlockPropertiesValues.count(); i++)
        {   ui->gridLayout_Block->removeWidget(BlockPropertiesValues[i]);
            delete BlockPropertiesValues[i];
        }
        for (int i=0; i<BlockPropertiesIncrements.count(); i++)
        {   ui->gridLayout_Block->removeWidget(BlockPropertiesIncrements[i]);
            delete BlockPropertiesIncrements[i];
        }

        if (verticalSpacer_blocks!=nullptr)
        {   ui->gridLayout_Link->removeItem(verticalSpacer_blocks);
            delete verticalSpacer_blocks;
        }
        ui->tab_BlockProperties->update();
        BlockPropertiesIncrements.clear();
        BlockPropertiesLabels.clear();
        BlockPropertiesValues.clear();

    }
    if (ObjectType == objectType::link)
    {   for (int i=0; i<LinkPropertiesLabels.count(); i++)
        {   ui->gridLayout_Link->removeWidget(LinkPropertiesLabels[i]);
            delete LinkPropertiesLabels[i];
        }
        for (int i=0; i<LinkPropertiesValues.count(); i++)
        {   ui->gridLayout_Link->removeWidget(LinkPropertiesValues[i]);
            delete LinkPropertiesValues[i];
        }
        for (int i=0; i<LinkPropertiesIncrements.count(); i++)
        {   ui->gridLayout_Link->removeWidget(LinkPropertiesIncrements[i]);
            delete LinkPropertiesIncrements[i];
        }

        if (verticalSpacer_links !=nullptr)
        {   ui->gridLayout_Link->removeItem(verticalSpacer_links);
            delete verticalSpacer_links;
        }

        ui->tab_LinkProperties->update();
        LinkPropertiesIncrements.clear();
        LinkPropertiesLabels.clear();
        LinkPropertiesValues.clear();

    }

}

void GridGenerator::browserClicked()
{
    QString fileName = QFileDialog::getOpenFileName(mainwindow,
            tr("Open"), "",
            tr("txt files (*.txt);; csv files (*.csv);; All files (*.*)"));
    if (fileName!="")
    {
        if (QFile(fileName).exists())
        {   QObject::sender()->setProperty("text",fileName);
            UpdateToolTipes();
        }
        else {
            QMessageBox::information(mainwindow, "File not found!", "File " + fileName + " was not found!", QMessageBox::Ok, QMessageBox::StandardButton::Ok);
        }
    }

}

void GridGenerator::UpdateToolTipes()
{

}

void GridGenerator::on_NextClicked()
{
    if (ui->tabWidget->currentIndex()<ui->tabWidget->count()-1)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()+1);

    on_TabChanged();
}
void GridGenerator::on_PreviousClicked()
{
    if (ui->tabWidget->currentIndex()>0)
        ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()-1);

    on_TabChanged();
}

void GridGenerator::on_TabChanged()
{
    if (ui->tabWidget->currentIndex()==0)
        ui->pushButtonPrevious->setEnabled(false);
    else
        ui->pushButtonPrevious->setEnabled(true);
    if (ui->tabWidget->currentIndex()==ui->tabWidget->count()-1)
        ui->pushButtonNext->setEnabled(false);
    else
        ui->pushButtonNext->setEnabled(true);

}
