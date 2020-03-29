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


QWidget *Delegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    //this->parent->endEditingDelegate();

    if (index.column() == 0) return QStyledItemDelegate::createEditor(parent, option, index);

    QString delegateType = index.data(CustomRoleCodes::Role::TypeRole).toString();
    if (delegateType.toLower().contains("date"))
    {
        QCalendarWidget *editor = new QCalendarWidget(nullptr);
        //(option.rect.left());
        return editor;
    }
    if (delegateType.contains("PushButton"))
    {
        QPushButton *editor = new QPushButton(parent);
        editor->setText(index.data().toString());
        return editor;
    }
    if (delegateType.contains("UnitBox"))
    {
        QTextEdit *editor = new QTextEdit(parent);
        QString text = index.data(Qt::DisplayRole).toString();
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

    //if (delegateType.contains("expressionEditor"))
    //{
    //    QStringList words = index.data(allowableWordsRole).toStringList();
        //adding possible terms in expressions
    //    words << this->parent->EntityNames("Constituent");
    //    words << this->parent->EntityNames("Reaction parameter");
    //    words << this->parent->PhysicalCharacteristicsList;
    //    words << this->parent->functionList;

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
        qint64 selectedDate = index.model()->data(index, Qt::EditRole).toDouble();
        qint64 julianDate = xldate2julian(selectedDate);// currentDate += 2415020;// QDate(1900, 1, 1).toJulianDay();
        QDate date = QDate::fromJulianDay(julianDate);

        QDateEdit *textBox = static_cast<QDateEdit*>(editor);
        textBox->setDate(date);
        textBox->show();
        return;
    }

    if (delegateType.contains("String"))
    {
        QTextEdit *textBox = static_cast<QTextEdit*>(editor);
        textBox->setText(index.model()->data(index, Qt::EditRole).toString());
        textBox->show();
        return;
    }
    if (delegateType.contains("UnitBox"))
    {
        QTextEdit *textBox = static_cast<QTextEdit*>(editor);
        textBox->setText(index.model()->data(index, Qt::EditRole).toString());
        textBox->show();
        return;
    }
    if (delegateType.contains("MultiComboBox"))
    {
        QListWidget *list = static_cast<QListWidget*>(editor);
        QStringList selectedList = index.model()->data(index, Qt::EditRole).toString().split(':');
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
        comboBox->setCurrentText(index.model()->data(index, Qt::EditRole).toString());
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
        //index.model()->data(index, saveIndex);
        QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(dirBrowserClicked()));
        return;
    }
    if (delegateType.contains("Browser"))
    {
        QPushButton *pushButton = static_cast<QPushButton*>(editor);
        pushButton->setText(index.data().toString());
        //index.model()->data(index, saveIndex);
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
    if (delegateType.toLower().contains("date"))
    {
        return;
    }

    if (delegateType.contains("UnitBox"))
    {
        QTextEdit *textBox = static_cast<QTextEdit*>(editor);
        model->setData(index, textBox->toPlainText());
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
            model->setData(index, newValue, Qt::EditRole);
        return;
    }
    if (delegateType.contains("ComboBox"))
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        if (model->data(index, Qt::EditRole).toString() != comboBox->currentText())
            model->setData(index, comboBox->currentText(), Qt::EditRole);
        return;
    }
    if (delegateType.contains("CheckBox"))
    {
        QCheckBox *checkBox = static_cast<QCheckBox*>(editor);
        if (model->data(index, Qt::CheckStateRole).toBool() != checkBox->checkState())
            model->setData(index, (checkBox->checkState()) ? 1 : 0, Qt::EditRole);
        return;
    }
    if (delegateType.contains("DirBrowser"))
    {
        return;
    }
    if (delegateType.contains("Browser"))
    {
        return;
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        //qDebug() << comboBox->currentText();
        model->setData(index, comboBox->currentText(), Qt::EditRole);
        return;
    }
    if (delegateType.contains("ListBox"))
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        if (model->data(index, Qt::EditRole).toString() != comboBox->currentData().toString())
            model->setData(index, comboBox->currentData().toString(), Qt::EditRole);
        return;
    }
    if (delegateType.contains("PushButton"))
    {
        return;
    }
    if (delegateType.contains("Memo"))
    {
        QTextEdit *memo = static_cast<QTextEdit*>(editor);
        if (index.model()->data(index, Qt::EditRole).toString() != memo->toPlainText())
            model->setData(index, memo->toPlainText(), Qt::EditRole);
        return;
    }
    if (delegateType.contains("expressionEditor"))
    {
        return;
    }
    if (delegateType.toLower().contains("text"))
    {
        QLineEdit *textBox = static_cast<QLineEdit*>(editor);

        if (model->data(index, Qt::EditRole).toString() != textBox->text())
            model->setData(index, textBox->text(), Qt::EditRole);
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
        return;
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
