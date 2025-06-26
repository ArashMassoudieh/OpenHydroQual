greaterThan(QT_MAJOR_VERSION, 5): QT += core gui svgwidgets network websockets charts
lessThan(QT_MAJOR_VERSION, 6): QT += core gui svg network websockets charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


DEFINES += HTTPs
#DEFINES += LOCALHOST
CONFIG += c++17

QMAKE_CXXFLAGS += -Wno-deprecated-declarations -Wno-unused-parameter

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

macx:{
DEFINES += mac_version
}

linux:
{
DEFINES += ubuntu_version
}
SOURCES += \
    ../UnitTextBox3.cpp \
    ../XString.cpp \
    ../aquifolium/src/Utilities.cpp \
    FilePushButton.cpp \
    WizBlockArray.cpp \
    SetValEntity.cpp \
    WizConnector.cpp \
    WizSingleBlock.cpp \
    Wizard_Argument.cpp \
    Wizard_Entity.cpp \
    Wizard_Script.cpp \
    filechecker.cpp \
    main.cpp \
    mainwindow.cpp \
    rosettafetcher.cpp \
    svgviewer.cpp \
    timeseriesloader.cpp \
    wizard_assigned_value.cpp \
    wizardcriteria.cpp \
    wizarddialog.cpp \
    wizardparameter.cpp \
    wizardparametergroup.cpp \
    wsclient.cpp

HEADERS += \
    ../UnitTextBox3.h \
    ../XString.h \
    ../aquifolium/src/Utilities.cpp \
    FilePushButton.h \
    WizBlockArray.h \
    SetValEntity.h \
    WizConnector.h \
    WizSingleBlock.h \
    Wizard_Argument.h \
    Wizard_Entity.h \
    Wizard_Script.h \
    filechecker.h \
    mainwindow.h \
    rosettafetcher.h \
    svgviewer.h \
    timeseriesloader.h \
    wizard_assigned_value.h \
    wizardcriteria.h \
    wizarddialog.h \
    wizardparameter.h \
    wizardparametergroup.h \
    wsclient.h

INCLUDEPATH += ../ \
../aquifolium/include



FORMS += \
    mainwindow.ui \
    wizarddialog.ui

TRANSLATIONS += \
    ModelWizard_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
