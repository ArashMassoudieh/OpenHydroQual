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
        w.SetModelTemplate(argv[1]);
    else
        w.SetModelTemplate("StormwaterPond.json");
#endif

    w->show();
    return a.exec();
}
