#ifndef ExpressionEditor_H
#define ExpressionEditor_H

#include <QLineEdit>
#include <string>
#include <vector>
struct ExpressionEditorPri;
class StatusViewer;
class Object;

class ExpressionEditor : public QLineEdit
{
	Q_OBJECT
public:
    explicit ExpressionEditor(Object* obj, StatusViewer* statusbar, QWidget* p = nullptr);
    ~ExpressionEditor();
    std::vector<std::string> funcs();

protected:
    void focusInEvent(QFocusEvent * e);
    void focusOutEvent(QFocusEvent * e);
    void keyPressEvent(QKeyEvent*);

private slots:
    void onCompletorActivated(const QString& item);

private:
    void setupCompleter();
    void replaceWordUnderCursorWith(const QString& c);
    QString cursorWord(const QString &sentence) const;

private:
    ExpressionEditorPri* d;
};

#endif // ExpressionEditor_H
