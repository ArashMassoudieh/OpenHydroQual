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
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QTimer>
#ifdef Q_OS_WASM
#include <emscripten/val.h>
#include <QUrl>
#include <QUrlQuery>
#endif

QString getBrowserArg(const QString &key)
{
#ifdef Q_OS_WASM
    using namespace emscripten;
    std::string href = val::global("window")["location"]["href"].as<std::string>();
    QUrl url(QString::fromStdString(href));
    QUrlQuery query(url.query());
    return query.queryItemValue(key);
#else
    return QString();
#endif
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ModelWizard_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    qApp->setStyleSheet(R"(
    QWidget {
        color: black;
        background-color: white;
    }

    QLineEdit, QTextEdit, QPlainTextEdit {
        color: black;
        background-color: white;
        border: 1px solid #ccc;
    }

    QPushButton {
        background-color: #f0f0f0;
        color: black;
    }

    QLabel {
        color: black;
    }
    )");

    MainWindow *w = new MainWindow();

#ifdef Q_OS_WASM
    QString modelArg = getBrowserArg("model");
    qDebug() << "Model Selected: " + modelArg;
    if (!modelArg.isEmpty())
        w->SetModelTemplate(modelArg);
    else
        w->SetModelTemplate("StormwaterPond.json");
#else
    if (argc > 1)
        w->SetModelTemplate(argv[1]);
    else
        w->SetModelTemplate("StormwaterPond.json");
#endif

    w->show();
    return a.exec();
}
