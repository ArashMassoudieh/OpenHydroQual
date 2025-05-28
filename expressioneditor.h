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
