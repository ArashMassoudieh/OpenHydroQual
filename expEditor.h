#ifndef expEditor_H
#define expEditor_H

#include <QLineEdit>

struct expEditorPri;
class StatusViewer;
class Object;

class expEditor : public QLineEdit
{
	Q_OBJECT
public:
    explicit expEditor(Object *obj, QStringList keywords, StatusViewer* statusbar, QWidget* p = nullptr);
    ~expEditor();
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
    expEditorPri* d;
};
int lastIndexOfNonVariable(const QString& str);

#endif // expEditor_H
