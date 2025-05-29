/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


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
