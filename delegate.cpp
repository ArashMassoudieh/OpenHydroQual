#include "delegate.h"
#include "enums.h"
#include "QCalendarWidget"
#include "QPushButton"
#include "QListWidget"
#include "QComboBox"
#include "QCheckBox"
#include "QTextEdit"
#include "utilityfuncs.h"
#include "QDateEdit"
#include "QLineEdit"
#include "mainwindow.h"
#include "QMessageBox"
#include "expEditor.h"
#include "UnitTextBox.h"
#include "XString.h"

Delegate::Delegate(QObject *parent, MainWindow *_mainwindow) : QStyledItemDelegate(parent)
{
    mainwindow = _mainwindow;
}

QWidget *Delegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    //this->parent->endEditingDelegate();

    if (index.column() == 0) return QStyledItemDelegate::createEditor(parent, option, index);

    QString delegateType = index.data(CustomRoleCodes::Role::TypeRole).toString();
    if (delegateType.toLower().contains("date"))
    {
        QDateEdit *editor = new QDateEdit(parent);
        editor->setDisplayFormat("MM.dd.yyyy");
        QVariant var = index.data(Qt::DisplayRole);
        QDateTime text = index.data(Qt::DisplayRole).toDateTime();
        editor->setDate(text.date());
        return editor;
    }
    if (delegateType.contains("PushButton"))
    {
        QPushButton *editor = new QPushButton(parent);
        editor->setText(index.data().toString());
        return editor;
    }
    if (delegateType.contains("String"))
    {
        QLineEdit *editor = new QLineEdit(parent);
        QString text = index.data(Qt::DisplayRole).toString();
        editor->setText(text);
        return editor;
    }

    if (delegateType.contains("ValueBox"))
    {
        QLineEdit *editor = new QLineEdit(parent);
        QString text = index.data(Qt::DisplayRole).toString();
        editor->setText(text);
        return editor;
    }
    if (delegateType.contains("UnitBox"))
    {
        UnitTextBox *editor = new UnitTextBox(option,parent);
        QString text = index.data(Qt::DisplayRole).toString().split('[')[0];
        QStringList QL = index.data(CustomRoleCodes::Role::UnitsListRole).toStringList();
        editor->setUnitsList(index.data(CustomRoleCodes::Role::UnitsListRole).toStringList());
        editor->setText(text);
        return editor;
    }
    if (delegateType.contains("MultiComboBox"))
    {
        QListWidget *editor = new QListWidget(parent);
        QStringList allItems = index.data(CustomRoleCodes::Role::DefaultValuesListRole).toStringList();
        foreach (QString item , allItems)
        {
            QListWidgetItem* wi = new QListWidgetItem(item);
            //wi.setText(item);
            editor->addItem(wi);
        }
        editor->setSelectionMode(QAbstractItemView::MultiSelection);
        return editor;
    }


    if (delegateType.contains("ComboBox"))
    {
        QComboBox *editor = new QComboBox(parent);
        editor->setFrame(false);
        QStringList DefaultValues = index.data(CustomRoleCodes::Role::DefaultValuesListRole).toStringList();

        editor->addItems(DefaultValues);
        if (index.data(CustomRoleCodes::Role::InputMethodRole).toString() == "Input") editor->setEditable(true);
        if (index.data(CustomRoleCodes::Role::InputMethodRole).toString() == "Select") editor->setEditable(false);
        return editor;
    }
    if (delegateType.contains("CheckBox"))
    {
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }
    if (delegateType.contains("DirBrowser"))
    {
        QPushButton *editor = new QPushButton(parent);
        editor->setText(index.data().toString());
        return editor;
    }
    if (delegateType.contains("Browser"))
    {
        QPushButton *editor = new QPushButton(parent);
        editor->setText(index.data().toString());
        return editor;
    }
    if (delegateType.contains("Browser_Save"))
    {
        QPushButton *editor = new QPushButton(parent);
        editor->setText(index.data().toString());
        return editor;
    }
    if (delegateType.contains("ListBox"))
    {
        QComboBox *editor = new QComboBox(parent);
        //connect(editor, SIGNAL(clicked()), this, SLOT(browserClicked()));
        QStringList DefaultValues = index.data(CustomRoleCodes::Role::DefaultValuesListRole).toStringList();

        editor->addItems(DefaultValues);
        editor->setAutoCompletion(true);
        return editor;
    }
    if (delegateType.contains("Memo"))
    {
        QTextEdit* editor = new QTextEdit(parent);
        editor->setWordWrapMode(QTextOption::NoWrap);
        return editor;
    }

    if (delegateType.contains("expressionEditor"))
    {
        QStringList words = index.data(CustomRoleCodes::Role::allowableWordsRole).toStringList();
        QString referedObjectName = index.data(CustomRoleCodes::Role::referedObjectRole).toString();
        Object *obj = nullptr;
        if (referedObjectName!="")
            obj = mainwindow->GetSystem()->object(referedObjectName.toStdString());

        //QTextEdit* editor = new QTextEdit(parent);
        expEditor* editor = new expEditor(obj, words, nullptr, parent);
        //qDebug()<<index.data(Qt::DisplayRole).toString();
        editor->setText(index.data(Qt::DisplayRole).toString());
        return editor;
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void Delegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    //parent->startEditingDelegate(index.data(VariableNameRole).toString());

    if (index.column() == 0) QStyledItemDelegate::setEditorData(editor, index);
    QString delegateType = index.data(CustomRoleCodes::Role::TypeRole).toString();
    if (delegateType.toLower().contains("date"))
    {
        QString thisdate = index.model()->data(index, Qt::DisplayRole).toString();
        QDate date = QDate::fromString(thisdate, "MMM dd yyyy");
        //qint64 selectedDate = index.model()->data(index, Qt::DisplayRole).toDouble();
        //qint64 julianDate = xldate2julian(selectedDate);// currentDate += 2415020;// QDate(1900, 1, 1).toJulianDay();
        //QDate date = QDate::fromJulianDay(julianDate);

        QDateEdit *textBox = static_cast<QDateEdit*>(editor);
        textBox->setDate(date);
        textBox->show();
        return;
    }

    if (delegateType.contains("String"))
    {
        QLineEdit *textBox = static_cast<QLineEdit*>(editor);
        textBox->setText(index.model()->data(index, Qt::DisplayRole).toString());
        textBox->show();
        return;
    }
    if (delegateType.contains("ValueBox"))
    {
        QLineEdit *textBox = static_cast<QLineEdit*>(editor);
        textBox->setText(index.model()->data(index, Qt::DisplayRole).toString());
        textBox->show();
        return;
    }
    if (delegateType.contains("UnitBox"))
    {
        UnitTextBox *textBox = static_cast<UnitTextBox*>(editor);
        textBox->setText(index.model()->data(index, Qt::DisplayRole).toString().split('[')[0]);
        textBox->setUnitsList(index.model()->data(index, CustomRoleCodes::Role::UnitsListRole).toStringList());
        textBox->setUnit(index.model()->data(index, CustomRoleCodes::Role::UnitRole).toString());
        textBox->show();
        return;
    }
    if (delegateType.contains("MultiComboBox"))
    {
        QListWidget *list = static_cast<QListWidget*>(editor);
        QStringList selectedList = index.model()->data(index, Qt::DisplayRole).toString().split(':');
        for (int i = 0; i < list->count(); i++)
        {
            if (selectedList.contains(list->item(i)->text()))
                list->item(i)->setSelected(true);
            else
                list->item(i)->setSelected(false);
        }
        return;
    }

    if (delegateType.contains("ComboBox"))
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentText(index.model()->data(index, Qt::DisplayRole).toString());
        return;
    }
    if (delegateType.contains("CheckBox"))
    {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        checkBox->setCheckState((index.model()->data(index, Qt::CheckStateRole).toBool()) ? Qt::Checked : Qt::Unchecked);
        return;
    }
    if (delegateType.contains("DirBrowser"))
    {
        QPushButton *pushButton = static_cast<QPushButton*>(editor);
        pushButton->setText(index.data().toString());
        index.model()->data(index, CustomRoleCodes::Role::saveIndex);
        QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(dirBrowserClicked()));
        return;
    }
    if (delegateType.contains("Browser"))
    {
        QPushButton *pushButton = static_cast<QPushButton*>(editor);
        pushButton->setText(index.data().toString());
        index.model()->data(index, CustomRoleCodes::Role::saveIndex);
        if (delegateType.contains("Browser_Save"))
            QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(browserSaveClicked()));
        else
            QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(browserClicked()));
        return;
    }
    if (delegateType.contains("ListBox"))
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentText(index.model()->data(index, Qt::EditRole).toString());
        return;
    }
    if (delegateType.contains("PushButton"))
    {
        QPushButton *pushButton = static_cast<QPushButton*>(editor);
        pushButton->setText(index.data().toString());
        return;
    }
    if (delegateType.contains("Memo"))
    {
        QTextEdit *memo = static_cast<QTextEdit*>(editor);
        memo->setText(index.model()->data(index, Qt::EditRole).toString());
        return;
    }
    if (delegateType.contains("expressionEditor"))
    {
        expEditor* expressionEditor = static_cast<expEditor*>(editor);
        expressionEditor->setText(index.model()->data(index, Qt::EditRole).toString());
        return;
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void Delegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    //parent->endEditingDelegate();
    if (index.column() == 0) QStyledItemDelegate::setModelData(editor, model, index);
    QString Property = model->data(index.sibling(index.row(), 0)).toString();

    QString delegateType = index.data(CustomRoleCodes::Role::TypeRole).toString();
    mainwindow->SetActiveUndo();
    if (delegateType.toLower().contains("date"))
    {
        QDateEdit *textBox = static_cast<QDateEdit*>(editor);

        if (model->setData(index, QDate2Xldate(textBox->dateTime())))
            mainwindow->AddStatetoUndoData();
        return;
    }

    if (delegateType.contains("ValueBox"))
    {
        QLineEdit *textBox = static_cast<QLineEdit*>(editor);
        if (model->setData(index, textBox->text()))
            mainwindow->AddStatetoUndoData();
    //	delete editor;
        return;
    }
    if (delegateType.contains("UnitBox"))
    {
        UnitTextBox *textBox = static_cast<UnitTextBox*>(editor);
        if (model->data(index, CustomRoleCodes::Role::UnitRole).toString()!=textBox->unit())
            if (model->setData(index, textBox->unit(),CustomRoleCodes::UnitRole))
                mainwindow->AddStatetoUndoData();
        if (model->data(index, Qt::DisplayRole).toString() != textBox->text())
            if(model->setData(index, textBox->text()))
                mainwindow->AddStatetoUndoData();
        //qDebug()<<model->data(index, CustomRoleCodes::Role::UnitRole).toString()<<":"<<textBox->unit();

    //	delete editor;
        return;
    }
    if (delegateType.contains("MultiComboBox"))
    {
        QListWidget *list = static_cast<QListWidget*>(editor);
        QStringList selectedList;
        //list->selectAll();
        for (int i = 0; i < list->count(); i++)
            if (list->item(i)->isSelected() && list->item(i)->text().size())
                selectedList.append(list->item(i)->text());
        QString newValue = selectedList.join(':');
        if (model->data(index, Qt::EditRole).toString() != newValue)
            if (model->setData(index, newValue, Qt::EditRole))
                mainwindow->AddStatetoUndoData();
        return;
    }
    if (delegateType.contains("ComboBox"))
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        if (model->data(index, Qt::EditRole).toString() != comboBox->currentText())
            if (model->setData(index, comboBox->currentText(), Qt::EditRole))
                mainwindow->AddStatetoUndoData();
        return;
    }
    if (delegateType.contains("CheckBox"))
    {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        if (model->data(index, Qt::CheckStateRole).toBool() != checkBox->checkState())
            if (model->setData(index, (checkBox->checkState()) ? 1 : 0, Qt::EditRole))
                mainwindow->AddStatetoUndoData();
        return;
    }
    if (delegateType.contains("DirBrowser"))
    {
        return;
    }
    if (delegateType.contains("Browser"))
    {
        return;

    }
    if (delegateType.contains("Browser_Save"))
    {
        return;

    }
    if (delegateType.contains("ListBox"))
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        if (model->data(index, Qt::EditRole).toString() != comboBox->currentData().toString())
            if (model->setData(index, comboBox->currentData().toString(), Qt::EditRole))
                mainwindow->AddStatetoUndoData();
        return;
    }
    if (delegateType.contains("PushButton"))
    {
        QPushButton *comboBox = static_cast<QPushButton*>(editor);
        if (model->setData(index, comboBox->text(), Qt::EditRole))
            mainwindow->AddStatetoUndoData();
        return;
    }
    if (delegateType.contains("Memo"))
    {
        QTextEdit *memo = static_cast<QTextEdit*>(editor);
        if (index.model()->data(index, Qt::EditRole).toString() != memo->toPlainText())
            if (model->setData(index, memo->toPlainText(), Qt::EditRole))
                mainwindow->AddStatetoUndoData();
        return;
    }
    if (delegateType.contains("expressionEditor"))
    {
        expEditor* expression = static_cast<expEditor*>(editor);
        if (index.model()->data(index, Qt::EditRole).toString() != expression->text())
            if (model->setData(index, expression->text(), Qt::EditRole))
                mainwindow->AddStatetoUndoData();
        return;
    }
    if (delegateType.toLower().contains("String"))
    {
        QLineEdit *textBox = static_cast<QLineEdit*>(editor);

        if (model->data(index, Qt::EditRole).toString() != textBox->text())
            if (model->setData(index, textBox->text(), Qt::EditRole))
                mainwindow->AddStatetoUndoData();
//		parent->setFocus(); // tableProp->setFocus();
//		QWidget * ed = QStyledItemDelegate::createEditor(parent, editor->rect(), index.sibling(index.row(), 0);
//		ed->setFocus();
        return;
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}

void Delegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    //parent->endEditingDelegate();
    QString delegateType = index.data(CustomRoleCodes::Role::TypeRole).toString();

    if (delegateType.toLower().contains("date"))
    {
        QRect tallerRect = option.rect;
        tallerRect.setHeight(option.rect.height());
        tallerRect.setTop(option.rect.top());
//		tallerRect.setTop(option.rect.top() + option.rect.height());
        editor->setGeometry(tallerRect);
        return;
    }
    if (delegateType.toLower().contains("multicombobox"))
    {
        QRect tallerRect = option.rect;
        tallerRect.setHeight(option.rect.height() * 5);
        tallerRect.setTop(option.rect.top());
//		tallerRect.setTop(option.rect.top() + option.rect.height());
        editor->setGeometry(tallerRect);
        return;
    }
    if (delegateType.toLower().contains("memo"))
    {
        QRect bigerRect = option.rect;
        bigerRect.setHeight(option.rect.height() * 5);
        bigerRect.setTop(option.rect.top() + option.rect.height());
        bigerRect.setLeft(option.rect.left() - 100);

        editor->setGeometry(bigerRect);
        return;
    }

    editor->setGeometry(option.rect);
    return;
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QString delegateType = index.data(CustomRoleCodes::TypeRole).toString();
    if (delegateType.contains("BarTextBox") && index.column() == 1)
    {
        int progress = index.data().toFloat() * 100;

        QStyleOptionProgressBar progressBarOption;
        QRect rect = option.rect;
        rect.setHeight(rect.height() /2);
        progressBarOption.rect = rect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.textVisible = true;
    }
    else
        QStyledItemDelegate::paint(painter, option, index);
}


void Delegate::browserClicked()
{
    QString fileName = QFileDialog::getOpenFileName(mainwindow,
            tr("Open"), "",
            tr("txt files (*.txt);; csv files (*.csv);; All files (*.*)"));
    if (fileName!="")
    {
        if (QFile(fileName).exists())
            selected_fileName = fileName;
        else {
            QMessageBox::information(mainwindow, "File not found!", "File " + fileName + " was not found!", QMessageBox::Ok, QMessageBox::StandardButton::Ok);
        }
    }
    else
        selected_fileName = "";

    if (mainwindow->propModel())
        mainwindow->propModel()->setData(selectedindex, fileName, CustomRoleCodes::Role::loadIndex);
}

void Delegate::browserSaveClicked()
{
    QString fileName = QFileDialog::getSaveFileName(mainwindow,
            tr("Save"), "",
            tr("txt files (*.txt);; csv files (*.csv);; All files (*.*)"));
    mainwindow->propModel()->setData(selectedindex, fileName, CustomRoleCodes::Role::loadIndex);
}
void Delegate::dirBrowserClicked()
{

}
