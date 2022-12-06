#pragma once
#include "qlineedit.h"
#include "qcombobox.h"
#include "qlistview.h"
#include "XString.h"
#include <QDoubleValidator>
#include <qlayout.h>
class UnitTextBox :
	public QWidget
{	
public:
	UnitTextBox() {};
	UnitTextBox(const XString &X, QWidget * parent = 0) :QWidget(parent)
	{
		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		QHBoxLayout *layout = new QHBoxLayout(this);
#ifndef Qt6
        layout->setMargin(0);
#endif
		layout->setSizeConstraint(QLayout::SetMaximumSize);
		textBox = new QLineEdit(this);
		unitBox = new QComboBox(textBox);
		validator = new QDoubleValidator(this);
		textBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		textBox->setValidator(validator);
		layout->addWidget(textBox);
		layout->addWidget(unitBox);
//		Rect = option.rect;

		setXString(X);
		unitBox->show();
		textBox->show();
		this->show();

	};

	UnitTextBox(const QStyleOptionViewItem &option, QWidget * parent = 0) :QWidget(parent)
	{
		textBox = new QLineEdit(this);
		unitBox = new QComboBox(textBox);
		validator = new QDoubleValidator(this);
		textBox->setValidator(validator);
		Rect = option.rect;
		unitBox->show();
		textBox->show();
		this->show();

		//		QObject::connect(textBox, SIGNAL(textChanged(const QString & text)), this, SIGNAL(editingFinished()));
	};
    ~UnitTextBox(){
        //if (textBox) delete textBox;
        //if (unitBox) delete unitBox;
        //if (validator) delete validator;
    };
	void paintEvent(QPaintEvent * event)
	{
		setGeometry(rect());
		textBox->resize(rect().width(), rect().height());
		QFont QF = font(); QF.setPointSize(QF.pointSize() - 1);
		unitBox->setFont(QF);
		unitBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
		int w = (unitBox->rect().width() < rect().width() / 2) ? unitBox->rect().width() : rect().width() / 2;
		unitBox->setGeometry(rect().width() - unitBox->rect().width(), 0, w, rect().height());
	};
	//XString value() const { return XString(text(), unit(), units()); };
	void setXString(const XString &X)
	{ 
		setText(X.toQString()); 
		setUnitsList(X.unitsList); 
		setUnit(X.unit);
		defaultUnit = X.defaultUnit;

	};
	void setText(const QString &T){ textBox->setText(T); };
	void setUnit(const QString &U){ unitBox->setCurrentText(U); };
	void setUnitsList(const QStringList &L){ for (int i = 0; unitBox->count(); i++) unitBox->clear(); unitBox->addItems(L); };
	QString text() const { return textBox->text(); };
	QString unit() const { return unitBox->currentText(); };
	QStringList units() const { QStringList R; for (int i = 0; i < unitBox->count(); i++) R.append(unitBox->itemText(i)); return R; };
	QStringList list() const { return QStringList() << text() << unit() << units() << defaultUnit; };
	XString toXString() {
        return XString(list());
	}

	void setGeometry(const QRect &R) {Rect = R;};
	QRect rect() const { return Rect; };
	QString defaultUnit;
private:
    QComboBox *unitBox = nullptr;
    QLineEdit *textBox = nullptr;
	QRect Rect;
    QDoubleValidator *validator = nullptr;
public slots:
signals:
/*	
	void	cursorPositionChanged(int old, int new1);
	void	editingFinished();
	void	returnPressed();
	void	selectionChanged();
	void	textChanged(const QString & text);
	void	textEdited(const QString & text);
	*/
	};

