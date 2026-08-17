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


#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>

namespace Ui {
class logwindow;
}

/**
 * @brief The log pane
 *
 * A plain widget rather than a window: the main window puts it inside a dock widget
 * so it can sit out of the way at the bottom, or be floated over the diagram.
 */
class logwindow : public QWidget
{
    Q_OBJECT

public:
    explicit logwindow(QWidget *parent = nullptr);
    ~logwindow();
    void AppendText(const QString &s);
    void AppendError(const QString &s);
    void AppendBlue(const QString &s);
    void Clear();
    /**
     * @brief A deliberately short preferred height
     *
     * A QTextBrowser asks for a tall default, which would make the log dock claim a
     * large slice of the window on startup.
     */
    QSize sizeHint() const override { return QSize(400, 120); }

private:
    Ui::logwindow *ui;
    /** @brief Pins the view to the newest line; called after every append */
    void ScrollToBottom();
};

#endif // LOGWINDOW_H
