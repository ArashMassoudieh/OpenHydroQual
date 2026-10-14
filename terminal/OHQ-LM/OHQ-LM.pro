# OHQ-LM - standalone headless Levenberg-Marquardt runner for OpenHydroQual
#
# This project lives in the OpenHydroQual repository under terminal/, beside
# TOpenHydroQual, and builds against the aquifolium sources in the same checkout.
#
#   mkdir -p build && cd build
#   qmake6 ../OHQ-LM.pro OHQ=/path/to/OpenHydroQual
#   make -j$(nproc)
#
# Run:
#   ./OHQ-LM <model.ohq> [working_folder] [--verify-residuals]
#
# Dependencies (Ubuntu):
#   sudo apt-get install libarmadillo-dev liblapack-dev libblas-dev libgsl-dev

isEmpty(OHQ): OHQ = $$PWD/../..
isEmpty(JSONCPP): JSONCPP = $$OHQ/jsoncpp
!exists($$OHQ/aquifolium/src/System.cpp): \
    error("OpenHydroQual sources not found under OHQ=$$OHQ. Pass OHQ=<path> to qmake.")

message(Building OHQ-LM against OHQ = $$OHQ)

QT -= gui
QT += core
CONFIG += console c++17
CONFIG -= app_bundle

INCLUDEPATH += $$OHQ/aquifolium/include
INCLUDEPATH += $$OHQ/aquifolium/src
INCLUDEPATH += $$OHQ/aquifolium/include/GA
INCLUDEPATH += $$OHQ/aquifolium/include/MCMC
INCLUDEPATH += $$OHQ/aquifolium/include/LM
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

TARGET = OHQ-LM
TEMPLATE = app
win32: QMAKE_CXXFLAGS += /MP

CONFIG(debug, debug|release) {
    message(  debug mode)
    DEFINES += NO_OPENMP DEBUG
    !win32: LIBS += -lpthread
} else {
    message(  release mode)
    unix:!macx: QMAKE_CXXFLAGS *= "-Xpreprocessor -fopenmp"
    unix:!macx: QMAKE_LFLAGS  += -fopenmp
    unix:!macx: LIBS += -lgomp
    !win32: LIBS += -lpthread
    win32: QMAKE_CXXFLAGS += /openmp
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
# Windows (MSVC 2022 + Qt 6 msvc2019_64), same third-party layout as the GUI's
# QAquifolium.vcxproj. Override any of these on the qmake command line, e.g.
#   qmake ..\OHQ-LM.pro ARMA_DIR=C:/Projects/armadillo-15.2.3
win32 {
    isEmpty(ARMA_DIR):     ARMA_DIR     = C:/Projects/armadillo-15.2.3
    isEmpty(OPENBLAS_DIR): OPENBLAS_DIR = C:/Projects/OpenBLAS-0.3.31-x64/win64
    isEmpty(GSL_DIR):      GSL_DIR      = C:/Projects/vcpkg/packages/gsl_x64-windows
    # _WINDOWS keeps MCMC.hpp away from openblas_config.h / openblas_set_num_threads;
    # ARMA_DONT_USE_WRAPPER links BLAS/LAPACK straight from OpenBLAS.
    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS ARMA_DONT_USE_WRAPPER GSL _WINDOWS WIN64 NOMINMAX _USE_MATH_DEFINES
    INCLUDEPATH += $$ARMA_DIR/include $$OPENBLAS_DIR/include $$GSL_DIR/include
    LIBS += -L$$OPENBLAS_DIR/lib -llibopenblas -L$$GSL_DIR/lib -lgsl -lgslcblas
    QMAKE_CXXFLAGS += /bigobj /utf-8
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
    $$OHQ/aquifolium/include/LM/LM.h \
    $$OHQ/aquifolium/include/LM/LM.hpp \
    $$OHQ/aquifolium/include/MCMC/MCMC.h \
    $$OHQ/aquifolium/include/MCMC/MCMC.hpp
