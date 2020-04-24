
#include "expressioneditor.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QDebug>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QMessageBox>
#include "statusviewer.h"
#include "Object.h"
#include "utilityfuncs.h"



int lastIndexOfNonVariable(const QString& str)
{
    for (int i = str.length() - 1; i >= 0; i--) {
        QChar c = str.at(i);
        if (!c.isLetter() && c != QChar('_') && !c.isNumber()) return i;
    }
    return -1;
}


struct ExpressionEditorPri{
    Object* object = nullptr;
    QCompleter* Comp;
    QStringList Words;
    ExpressionEditorPri() : Comp(nullptr), Editing(false){}
    StatusViewer* Statusbar;
    bool Editing;
    QString OldExpression;
};

ExpressionEditor::ExpressionEditor(Object* obj, StatusViewer* statusbar, QWidget* p) : QLineEdit(p), d(new ExpressionEditorPri)
{
    d->object = obj;
    d->Statusbar = statusbar;
    setupCompleter();
    //connect(this, SIGNAL(returnPressed()), SLOT(onReturnPressed()));
    //connect(this, SIGNAL(editingFinished()), SLOT(onEditingFinished()));
}

ExpressionEditor::~ExpressionEditor() {delete d;}


void ExpressionEditor::setupCompleter()
{
	if (d->Comp) {
        d->Comp->deleteLater();
        d->Comp = nullptr;
    }

    if (d->object)
    d->Words << toQStringList(d->object->quantitative_variable_list()) << toQStringList(*d->object->functions()) << toQStringList(*d->object->operators());
    d->Comp = new QCompleter(d->Words, this);
    d->Comp->setWidget(this);
    d->Comp->setCompletionMode(QCompleter::PopupCompletion);
    d->Comp->setCaseSensitivity(Qt::CaseSensitive);
    connect(d->Comp, SIGNAL(activated(QString)), this, SLOT(onCompletorActivated(QString)));
}



void ExpressionEditor::focusInEvent(QFocusEvent *e)
{
    //qDebug() << "FocusIn event";
    if (d->Comp) {
        d->Comp->setWidget(this);
    }
    if (!d->Editing) {
        d->OldExpression = text();
    }
    d->Editing = true;
    QLineEdit::focusInEvent(e);
}


void ExpressionEditor::focusOutEvent(QFocusEvent *e)
{
    //qDebug() << "FocusOut event";
    d->Editing = false;    
    QLineEdit::focusOutEvent(e);
}


void ExpressionEditor::replaceWordUnderCursorWith(const QString &c)
{
    QString t = text();
    int i = cursorPosition() - 1, r, l;
	while (i < t.length() && (t[i].isLetter() || t[i] == QChar('_') || t[i].isNumber() || t[i] == '.' || t[i] == '~')) i++;
    r = i - 1;
    i = cursorPosition() - 1;
	while (i >= 0 && (t[i].isLetter() || t[i] == QChar('_') || t[i].isNumber() || t[i] == '.' || t[i] == '~')) i--;
	l = i + 1;
    t = t.replace(l, r - l + 1, c);
    setText(t);
}


QString ExpressionEditor::cursorWord(const QString &sentence) const
{
    int ind = lastIndexOfNonVariable(sentence.left(cursorPosition()));
	QString word = sentence.mid(ind + 1, cursorPosition() - ind - 1);
	return word;
}

void ExpressionEditor::keyPressEvent(QKeyEvent *e)
{         
	QLineEdit::keyPressEvent(e);
    QCompleter* c = d->Comp;
	if (!c) {
		return;
	}
    else {
        if (c->popup()->isVisible() && e->key() == Qt::Key_Escape) {
            setText(d->OldExpression); return;
        }
    }

    QChar ck (e->key());

    QStringList operators = toQStringList(*d->object->operators());

    QList<int> escapeKeys; escapeKeys << Qt::Key_Backspace << Qt::Key_Delete << Qt::Key_Enter << Qt::Key_Return
                                      <<Qt::Key_Left << Qt::Key_Right << Qt::Key_Up << Qt::Key_Down
                                     <<Qt::Key_Control << Qt::Key_Alt << Qt::Key_Shift << Qt::Key_CapsLock
									 <<Qt::Key_Escape << Qt::Key_Underscore << 46;

    if (!escapeKeys.contains(e->key()) && !ck.isLetterOrNumber() && !operators.contains(QString(ck))) {
		//qDebug() << e->key() << int(Qt::Key_Enter);
        backspace();return;
    }

    c->setCompletionPrefix(cursorWord(this->text()));
    if (c->completionPrefix().length() < 0) {
        c->popup()->hide(); return;
    }
    if (c->completionCount() < 1) {
        if (QChar(e->key()).isLetter()) {
            backspace();
        }
    }

    c->complete();
}

void ExpressionEditor::onCompletorActivated(const QString &arg)
{
    int ind = lastIndexOfNonVariable(text().left(cursorPosition()));
    setText(text().replace(ind + 1, cursorPosition() - ind - 1, arg));
}
