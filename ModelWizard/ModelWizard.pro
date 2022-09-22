QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

macx:{
DEFINES += mac_version
}
SOURCES += \
    ../UnitTextBox3.cpp \
    ../XString.cpp \
    ../aquifolium/src/Utilities.cpp \
    MajorBlock.cpp \
    Wizard_Argument.cpp \
    Wizard_Script.cpp \
    main.cpp \
    mainwindow.cpp \
    wizard_assigned_value.cpp \
    wizardcriteria.cpp \
    wizarddialog.cpp \
    wizardparameter.cpp \
    wizardparametergroup.cpp

HEADERS += \
    ../UnitTextBox3.h \
    ../XString.h \
    ../aquifolium/src/Utilities.cpp \
    MajorBlock.h \
    Wizard_Argument.h \
    Wizard_Script.h \
    mainwindow.h \
    wizard_assigned_value.h \
    wizardcriteria.h \
    wizarddialog.h \
    wizardparameter.h \
    wizardparametergroup.h

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
