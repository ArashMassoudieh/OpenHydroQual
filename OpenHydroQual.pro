#-------------------------------------------------
#
# Project created by QtCreator 2020-03-22T17:57:35
#
#-------------------------------------------------

#CONFIG -= app_bundle

CONFIG += c++17

QT += core gui opengl printsupport widgets

# For Qt5
lessThan(QT_MAJOR_VERSION, 6): {
    QT += svg charts
}

# For Qt6
equals(QT_MAJOR_VERSION, 6): {
    QT += svgwidgets charts
    DEFINES += Qt6
}


DEFINES += QCharts
INCLUDEPATH += ./aquifolium/include
INCLUDEPATH += ./aquifolium/src
INCLUDEPATH += ./aquifolium/include/GA
INCLUDEPATH += ./aquifolium/include/MCMC
INCLUDEPATH += jsoncpp/include/
INCLUDEPATH += include/
INCLUDEPATH += ../qcustomplot6/

macx: DEFINES +=mac_version
linux: DEFINES +=ubuntu_version
win32: DEFINES +=windows_version
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += GSL

TARGET = OpenHydroQual
TEMPLATE = app
win32:QMAKE_CXXFLAGS += /MP

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS Q_GUI_SUPPORT Q_JSON_SUPPORT Aquifolium
#DEFINES += Debug_GA
#DEFINES += VALGRIND
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0



macx: {
    QMAKE_LFLAGS += -lomp
}


macx {
    CONFIG += c++17 app_bundle

    LLVM_DIR = /opt/homebrew/opt/llvm

    QMAKE_CC  = $$LLVM_DIR/bin/clang
    QMAKE_CXX = $$LLVM_DIR/bin/clang++

    QMAKE_CXXFLAGS += -fopenmp -I$$LLVM_DIR/include
    QMAKE_LFLAGS   += -L$$LLVM_DIR/lib -lomp

    INCLUDEPATH += $$LLVM_DIR/include
    LIBS += -L$$LLVM_DIR/lib -lomp

    DEFINES += mac_version ARMA_USE_LAPACK ARMA_USE_BLAS

    # Armadillo and GSL (adjust versions if needed)
    INCLUDEPATH += $$PWD/../Armadillo
    DEPENDPATH  += $$PWD/../Armadillo
    LIBS += -L$$PWD/../Armadillo -larmadillo.11.2.3 -llapack.3.10.1 -lblas.3.10.1

    INCLUDEPATH += /opt/homebrew/Cellar/gsl/2.8/include
    LIBS += -L/opt/homebrew/Cellar/gsl/2.8/lib -lgsl

    QMAKE_POST_LINK += cp -R $$PWD/resources $$OUT_PWD/OpenHydroQual.app/Contents/
}


CONFIG(debug, debug|release) {
    message(Building in debug mode)
    !macx: QMAKE_CXXFLAGS *= -fopenmp -O3 -march=native
    !macx: QMAKE_LFLAGS +=  -fopenmp
    !macx: LIBS += -lgomp -lpthread
    LIBS += -lpthread
    DEFINES += _NO_OPENMP DEBUG

} else {
    message(Building in release mode)
    !macx: QMAKE_CXXFLAGS *= -fopenmp -O3 -march=native
    !macx: QMAKE_LFLAGS +=  -fopenmp
    # QMAKE_CFLAGS+=-pg
    # QMAKE_CXXFLAGS+=-pg
    # QMAKE_LFLAGS+=-pg
    # macx: DEFINES += NO_OPENMP
    ! macx: LIBS += -lgomp -lpthread -lopenblas
    macx: LIBS += -lpthread
    DEFINES += DEBUG
}



SOURCES += \
    ProgressWindow.cpp \
    VisualizationDialog.cpp \
    VisualizationGraphicsView.cpp \
    chartview.cpp \
    metamodelhelpdialog.cpp \
    qplotter.cpp \
    ./aquifolium/src/RxnParameter.cpp \
    ./aquifolium/src/constituent.cpp \
    ./aquifolium/src/observation.cpp \
    ./aquifolium/src/precalculatedfunction.cpp \
    ./aquifolium/src/solutionlogger.cpp \
    jsoncpp/src/lib_json/json_reader.cpp \
    jsoncpp/src/lib_json/json_value.cpp \
    jsoncpp/src/lib_json/json_writer.cpp \
    CustomPlotZoom.cpp \
    ItemNavigator.cpp \
    ItemPropertiesWidget.cpp \
    UnitTextBox3.cpp \
    XString.cpp \
    aboutdialog.cpp \
    aquifolium/src/Utilities.cpp \
    aquifolium/src/restorepoint.cpp \
    diagramview.cpp \
    edge.cpp \
    gridgenerator.cpp \
    main.cpp \
    mainwindow.cpp \
    ./aquifolium/src/Block.cpp \
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
    qplotwindow.cpp \
    ray.cpp \
    undodata.cpp \
    utilityfuncs.cpp \
    ./aquifolium/src/Command.cpp \
    ./aquifolium/src/Script.cpp \
    ./aquifolium/src/GA/Binary.cpp \
    ./aquifolium/src/GA/Individual.cpp \
    ./aquifolium/src/GA/DistributionNUnif.cpp \
    ./aquifolium/src/GA/Distribution.cpp \
    ../qcustomplot6/qcustomplot.cpp \
    expEditor.cpp \
    statusviewer.cpp \
    expressioneditor.cpp \
    logwindow.cpp \
    ./aquifolium/src/Link.cpp \
    ./aquifolium/src/reaction.cpp \
    wizard_select_dialog.cpp

HEADERS += \
    ProgressWindow.h \
    VisualizationDialog.h \
    VisualizationGraphicsView.h \
    aquifolium/include/TimeSeries.h \
    aquifolium/include/TimeSeriesSet.h \
    aquifolium/src/TimeSeries.hpp \
    aquifolium/src/TimeSeriesSet.hpp \
    chartview.h \
    metamodelhelpdialog.h \
    qplotter.h \
    qplotwindow.h \
    ./aquifolium/include/Objective_Function.h \
    ./aquifolium/include/Objective_Function_Set.h \
    ./aquifolium/include/Precipitation.h \
    ./aquifolium/include/RxnParameter.h \
    ./aquifolium/include/constituent.h \
    ./aquifolium/include/observation.h \
    ./aquifolium/include/precalculatedfunction.h \
    ./aquifolium/include/solutionlogger.h \
    CustomPlotZoom.h \
    ItemNavigator.h \
    ItemPropertiesWidget.h \
    UnitTextBox.h \
    UnitTextBox3.h \
    XString.h \
    aboutdialog.h \
    aquifolium/include/GA/GA.h \
    aquifolium/include/MCMC/MCMC.h \
    aquifolium/include/MCMC/MCMC.hpp \
    aquifolium/include/Utilities.h \
    aquifolium/include/restorepoint.h \
    aquifolium/include/safevector.h \
    aquifolium/include/safevector.hpp \
    chartview.h \
    diagramview.h \
    edge.h \
    gridgenerator.h \
        mainwindow.h \
    ./aquifolium/include/Block.h \
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
    ./aquifolium/include/System.h \
    ./aquifolium/include/Vector.h \
    ./aquifolium/include/Vector_arma.h \
    jsoncpp/include/json/allocator.h \
    jsoncpp/include/json/assertions.h \
    jsoncpp/include/json/autolink.h \
    jsoncpp/include/json/config.h \
    jsoncpp/include/json/features.h \
    jsoncpp/include/json/forwards.h \
    jsoncpp/include/json/json.h \
    jsoncpp/include/json/reader.h \
    jsoncpp/include/json/value.h \
    jsoncpp/include/json/version.h \
    jsoncpp/include/json/writer.h \
    jsoncpp/src/lib_json/json_tool.h \
    jsoncpp/src/lib_json/version.h.in \
    enums.h \
    node.h \
    options.h \
    propmodel.h \
    delegate.h \
    qplotter.h \
    qplotwindow.h \
    ray.h \
    undodata.h \
    utilityfuncs.h \
    ./aquifolium/include/Parameter.h \
    ./aquifolium/include/Parameter_Set.h \
    ./aquifolium/include/Command.h \
    ./aquifolium/include/Script.h \
    ./aquifolium/include/GA/Binary.h \
    ./aquifolium/include/GA/Distribution.h \
    ./aquifolium/include/GA/DistributionNUnif.h \
    ./aquifolium/include/GA/Individual.h \
    ../qcustomplot6/qcustomplot.h \
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
    qplotwindow.ui \
    Options.ui \
    aboutdialog.ui \
    gridgenerator.ui \
    itempropertieswidget.ui \
    mainwindow.ui \
    logwindow.ui \
    wizard_select_dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



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
     LIBS += -larmadillo -llapack -lblas -lgsl -lopenblas

}

