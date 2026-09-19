# OHQ-GA - standalone headless runner for OpenHydroQual
#
# This project lives in the OpenHydroQual repository under terminal/, beside
# TOpenHydroQual, and builds against the aquifolium sources in the same checkout.
#
#   mkdir -p build && cd build
#   qmake6 ../OHQ-GA.pro OHQ=/path/to/OpenHydroQual
#   make -j$(nproc)
#
# Run:
#   ./OHQ-GA <model.ohq> [working_folder]
#
# Dependencies (Ubuntu):
#   sudo apt-get install libarmadillo-dev liblapack-dev libblas-dev libgsl-dev

isEmpty(OHQ): OHQ = $$PWD/../..
isEmpty(JSONCPP): JSONCPP = $$OHQ/jsoncpp
!exists($$OHQ/aquifolium/src/System.cpp): \
    error("OpenHydroQual sources not found under OHQ=$$OHQ. Pass OHQ=<path> to qmake.")

message(Building OHQ-GA against OHQ = $$OHQ)

QT -= gui
QT += core
CONFIG += console c++17
CONFIG -= app_bundle

INCLUDEPATH += $$OHQ/aquifolium/include
INCLUDEPATH += $$OHQ/aquifolium/src
INCLUDEPATH += $$OHQ/aquifolium/include/GA
INCLUDEPATH += $$OHQ/aquifolium/include/MCMC
INCLUDEPATH += $$JSONCPP/include
INCLUDEPATH += $$OHQ
INCLUDEPATH += $$PWD/../OHQ-Common

macx:  DEFINES += mac_version
linux: DEFINES += ubuntu_version
win32: DEFINES += windows_version

# Terminal_version routes progress to stdout. Q_GUI_SUPPORT is deliberately NOT
# defined: every ProgressWindow call compiles out and the rtw pointers stay null.
DEFINES += Terminal_version Q_JSON_SUPPORT

# Templates are resolved from here unless the model file gives absolute paths or
# OHQ_RESOURCES is set in the environment at run time.
DEFINES += OHQ_DEFAULT_RESOURCES=\\\"$$OHQ/resources/\\\"

TARGET = OHQ-GA
TEMPLATE = app
win32: QMAKE_CXXFLAGS += /MP

CONFIG(debug, debug|release) {
    message(  debug mode)
    DEFINES += NO_OPENMP DEBUG
    LIBS += -lpthread
} else {
    message(  release mode)
    !macx: QMAKE_CXXFLAGS *= "-Xpreprocessor -fopenmp"
    !macx: QMAKE_LFLAGS  += -fopenmp
    !macx: LIBS += -lgomp
    LIBS += -lpthread
}

linux {
    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS GSL
    LIBS += -larmadillo -llapack -lblas -lgsl -lopenblas -lsuperlu -ldl   # openblas_set_num_threads() in MCMC step()
}
macx {
    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS GSL
    INCLUDEPATH += /opt/homebrew/include /usr/local/include
    LIBS += -L/opt/homebrew/lib -L/usr/local/lib -larmadillo -llapack -lblas -lgsl
}
win32 {
    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS GSL
}

SOURCES += \
        $$OHQ/aquifolium/src/Block.cpp \
        $$OHQ/aquifolium/src/Command.cpp \
        $$OHQ/aquifolium/src/Composite.cpp \
        $$OHQ/aquifolium/src/Condition.cpp \
        $$OHQ/aquifolium/src/constituent.cpp \
        $$OHQ/aquifolium/src/ErrorHandler.cpp \
        $$OHQ/aquifolium/src/Expression.cpp \
        $$OHQ/aquifolium/src/Link.cpp \
        $$OHQ/aquifolium/src/Matrix.cpp \
        $$OHQ/aquifolium/src/Matrix_arma.cpp \
        $$OHQ/aquifolium/src/MetaModel.cpp \
        $$OHQ/aquifolium/src/NormalDist.cpp \
        $$OHQ/aquifolium/src/Object.cpp \
        $$OHQ/aquifolium/src/Objective_Function.cpp \
        $$OHQ/aquifolium/src/Objective_Function_Set.cpp \
        $$OHQ/aquifolium/src/observation.cpp \
        $$OHQ/aquifolium/src/Parameter.cpp \
        $$OHQ/aquifolium/src/Parameter_Set.cpp \
        $$OHQ/aquifolium/src/precalculatedfunction.cpp \
        $$OHQ/aquifolium/src/Precipitation.cpp \
        $$OHQ/aquifolium/src/Quan.cpp \
        $$OHQ/aquifolium/src/QuanSet.cpp \
        $$OHQ/aquifolium/src/QuickSort.cpp \
        $$OHQ/aquifolium/src/reaction.cpp \
        $$OHQ/aquifolium/src/restorepoint.cpp \
        $$OHQ/aquifolium/src/Rule.cpp \
        $$OHQ/aquifolium/src/RxnParameter.cpp \
        $$OHQ/aquifolium/src/Script.cpp \
        $$OHQ/aquifolium/src/solutionlogger.cpp \
        $$OHQ/aquifolium/src/Source.cpp \
        $$OHQ/aquifolium/src/System.cpp \
        $$OHQ/aquifolium/src/Utilities.cpp \
        $$OHQ/aquifolium/src/Vector.cpp \
        $$OHQ/aquifolium/src/Vector_arma.cpp \
        $$OHQ/aquifolium/src/GA/Binary.cpp \
        $$OHQ/aquifolium/src/GA/Distribution.cpp \
        $$OHQ/aquifolium/src/GA/DistributionNUnif.cpp \
        $$OHQ/aquifolium/src/GA/Individual.cpp \
        $$JSONCPP/src/lib_json/json_reader.cpp \
        $$JSONCPP/src/lib_json/json_value.cpp \
        $$JSONCPP/src/lib_json/json_writer.cpp \
        main.cpp

HEADERS += \
    $$PWD/../OHQ-Common/ohq_common.h \
    $$PWD/../OHQ-Common/ohq_kernel.h \
    $$OHQ/aquifolium/include/System.h \
    $$OHQ/aquifolium/include/Script.h \
    $$OHQ/aquifolium/include/GA/GA.h \
    $$OHQ/aquifolium/include/GA/GA.hpp \
    $$OHQ/aquifolium/include/MCMC/MCMC.h \
    $$OHQ/aquifolium/include/MCMC/MCMC.hpp
