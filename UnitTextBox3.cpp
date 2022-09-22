#include "UnitTextBox3.h"

UnitTextBox3::UnitTextBox3(const XString &X, bool openFileMenu, QWidget * parent)
    :QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    textBox = new QLineEdit(this);
    layout->addWidget(textBox);

    unitBox = new QComboBox(textBox);
    layout->addWidget(unitBox);
    layout->setSizeConstraint(QLayout::SetMaximumSize);
    setLayout(layout);
    validator = new QDoubleValidator(this);
    textBox->setValidator(validator);
    setGeometry(QRect(0, 0, 300, 20));
    setXString(X);
    setFocusProxy(textBox);
    updateContextMenu(openFileMenu);
    connect(textBox, SIGNAL(textEdited(const QString&)),this, SLOT(on_text_edited(const QString&)));
}

void UnitTextBox3::on_text_edited(const QString& text)
{
    emit textEdited(text);
}
