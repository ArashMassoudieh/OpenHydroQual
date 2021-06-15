#-------------------------------------------------
#
# Project created by QtCreator 2020-03-22T17:57:35
#
#-------------------------------------------------

QT       += core gui opengl printsupport svg
INCLUDEPATH += ./aquifolium/include
INCLUDEPATH += ./aquifolium/include/GA
INCLUDEPATH += ../jsoncpp/include/
INCLUDEPATH += include/
INCLUDEPATH += ../qcustomplot/

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenHydroQual
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS Q_version Aquifolium
#DEFINES += VALGRIND
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

CONFIG(debug, debug|release) {
    message(Building in debug mode)
    DEFINES += NO_OPENMP DEBUG

} else {
    message(Building in release mode)
    QMAKE_CXXFLAGS+= -fopenmp
    QMAKE_LFLAGS +=  -fopenmp
    LIBS += -lgomp -lpthread
}



SOURCES += \
    ./aquifolium/src/RxnParameter.cpp \
    ./aquifolium/src/constituent.cpp \
    ./aquifolium/src/observation.cpp \
    ./aquifolium/src/precalculatedfunction.cpp \
    ./aquifolium/src/solutionlogger.cpp \
    ../jsoncpp/src/lib_json/json_reader.cpp \
    ../jsoncpp/src/lib_json/json_value.cpp \
    ../jsoncpp/src/lib_json/json_writer.cpp \
    CustomPlotZoom.cpp \
    aboutdialog.cpp \
    diagramview.cpp \
    edge.cpp \
        main.cpp \
        mainwindow.cpp \
    ./aquifolium/src/Block.cpp \
    ./aquifolium/src/BTC.cpp \
    ./aquifolium/src/BTCSet.cpp \
    ./aquifolium/src/Expression.cpp \
    ./aquifolium/src/Matrix.cpp \
    ./aquifolium/src/Matrix_arma.cpp \
    ./aquifolium/src/MetaModel.cpp \
    ./aquifolium/src/NormalDist.cpp \
    ./aquifolium/src/Object.cpp \
    ./aquifolium/src/Quan.cpp \
    ./aquifolium/src/QuanSet.cpp \
    ./aquifolium/src/QuickSort.cpp \
    ./aquifolium/src/System.cpp \
    ./aquifolium/src/Vector.cpp \
    ./aquifolium/src/Vector_arma.cpp \
    ./aquifolium/src/Source.cpp \
    ./aquifolium/src/Rule.cpp \
    ./aquifolium/src/Objective_Function_Set.cpp \
    ./aquifolium/src/Objective_Function.cpp \
    ./aquifolium/src/Precipitation.cpp \
    ./aquifolium/src/Condition.cpp \
    ./aquifolium/src/Parameter_Set.cpp \
    ./aquifolium/src/Parameter.cpp \
    ./aquifolium/src/ErrorHandler.cpp \
    node.cpp \
    options.cpp \
    propmodel.cpp \
    delegate.cpp \
    ray.cpp \
    utilityfuncs.cpp \
    ./aquifolium/src/Command.cpp \
    ./aquifolium/src/Script.cpp \
    ./aquifolium/src/GA/Binary.cpp \
    ./aquifolium/src/GA/Individual.cpp \
    ./aquifolium/src/GA/DistributionNUnif.cpp \
    ./aquifolium/src/GA/Distribution.cpp \
    runtimewindow.cpp \
    ../qcustomplot/qcustomplot.cpp \
    plotter.cpp \
    expEditor.cpp \
    statusviewer.cpp \
    expressioneditor.cpp \
    logwindow.cpp \
    ./aquifolium/src/Link.cpp \
    ./aquifolium/src/reaction.cpp \
    wizard_select_dialog.cpp

HEADERS += \
    ./aquifolium/include/Objective_Function.h \
    ./aquifolium/include/Objective_Function_Set.h \
    ./aquifolium/include/Precipitation.h \
    ./aquifolium/include/RxnParameter.h \
    ./aquifolium/include/constituent.h \
    ./aquifolium/include/observation.h \
    ./aquifolium/include/precalculatedfunction.h \
    ./aquifolium/include/solutionlogger.h \
    CustomPlotZoom.h \
    aboutdialog.h \
    diagramview.h \
    edge.h \
        mainwindow.h \
    ./aquifolium/include/Block.h \
    ./aquifolium/include/BTC.h \
    ./aquifolium/include/BTCSet.h \
    ./aquifolium/include/Expression.h \
    ./aquifolium/include/Link.h \
    ./aquifolium/include/Matrix.h \
    ./aquifolium/include/Matrix_arma.h \
    ./aquifolium/include/MetaModel.h \
    ./aquifolium/include/NormalDist.h \
    ./aquifolium/include/Object.h \
    ./aquifolium/include/Quan.h \
    ./aquifolium/include/QuanSet.h \
    ./aquifolium/include/QuickSort.h \
    ./aquifolium/include/StringOP.h \
    ./aquifolium/include/System.h \
    ./aquifolium/include/Vector.h \
    ./aquifolium/include/Vector_arma.h \
    ../jsoncpp/include/json/allocator.h \
    ../jsoncpp/include/json/assertions.h \
    ../jsoncpp/include/json/autolink.h \
    ../jsoncpp/include/json/config.h \
    ../jsoncpp/include/json/features.h \
    ../jsoncpp/include/json/forwards.h \
    ../jsoncpp/include/json/json.h \
    ../jsoncpp/include/json/reader.h \
    ../jsoncpp/include/json/value.h \
    ../jsoncpp/include/json/version.h \
    ../jsoncpp/include/json/writer.h \
    ../jsoncpp/src/lib_json/json_tool.h \
    ../jsoncpp/src/lib_json/version.h.in \
    enums.h \
    node.h \
    options.h \
    propmodel.h \
    delegate.h \
    ray.h \
    utilityfuncs.h \
    ./aquifolium/include/Parameter.h \
    ./aquifolium/include/Parameter_Set.h \
    ./aquifolium/include/Command.h \
    ./aquifolium/include/Script.h \
    ./aquifolium/include/GA/Binary.h \
    ./aquifolium/include/GA/Distribution.h \
    ./aquifolium/include/GA/DistributionNUnif.h \
    ./aquifolium/include/GA/Individual.h \
    runtimewindow.h \
    ../qcustomplot/qcustomplot.h \
    plotter.h \
    ./aquifolium/include/Objective_Function.h \
    ./aquifolium/include/Objective_Function_Set.h \
    expEditor.h \
    statusviewer.h \
    expressioneditor.h \
    ./aquifolium/include/GA/GA.hpp \
    logwindow.h \
    ./aquifolium/include/reaction.h \
    wizard_select_dialog.h

FORMS += \
    Options.ui \
    aboutdialog.ui \
        mainwindow.ui \
    runtimewindow.ui \
    plotter.ui \
    logwindow.ui \
    wizard_select_dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# LAPACK — Linear Algebra PACKage lib and include locations

win32 {

    LAPACK_INCLUDE = $$PWD/include
    #64 bits build
    contains(QMAKE_TARGET.arch, x86_64) {
        #debug
        CONFIG(debug, debug|release) {
            LAPACK_LIB_DIR = $$PWD/libs/lapack-blas_lib_win64/debug
            LIBS +=  -L$${LAPACK_LIB_DIR} -llapack_win64_MTd \
                    -lblas_win64_MTd
        }
        #release
        CONFIG(release, debug|release) {
            LAPACK_LIB_DIR = $$PWD/libs/lapack-blas_lib_win64/release
            LIBS +=  -L$${LAPACK_LIB_DIR} -llapack_win64_MT \
                    -lblas_win64_MT
        }
    }

    INCLUDEPATH += $${LAPACK_INCLUDE}

    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS

}

linux {
    #sudo apt-get install libblas-dev liblapack-dev
     DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS
     LIBS += -larmadillo -llapack -lblas
}