# ohq_generate - codegen CLI (model.ohq -> standalone C++ / shared kernel), qmake build
#
# The CMake build in this folder links a prebuilt libOHQLib.so (Linux). This
# project compiles the aquifolium sources directly instead, like the terminal
# runners, so it also builds on Windows (MSVC + Qt 6 msvc2019_64):
#
#   mkdir build-gen && cd build-gen        (outside terminal/: its .qmake.stash is gcc's)
#   qmake <OHQ>/codegen/ohq_generate.pro CONFIG+=release
#   nmake release                          (make on Linux)
#
# Usage: ohq_generate <model.ohq> <resources_dir> <out_dir> <ClassName> [stateVar] --project shared

isEmpty(OHQ): OHQ = $$PWD/..
isEmpty(JSONCPP): JSONCPP = $$OHQ/jsoncpp
!exists($$OHQ/aquifolium/src/System.cpp): \
    error("OpenHydroQual sources not found under OHQ=$$OHQ. Pass OHQ=<path> to qmake.")

QT -= gui
QT += core
CONFIG += console c++17
CONFIG -= app_bundle
TARGET = ohq_generate
TEMPLATE = app

INCLUDEPATH += $$OHQ/codegen/include $$OHQ/codegen/runtime
INCLUDEPATH += $$OHQ/aquifolium/include $$OHQ/aquifolium/src
INCLUDEPATH += $$OHQ/aquifolium/include/GA $$OHQ/aquifolium/include/MCMC
INCLUDEPATH += $$JSONCPP/include $$OHQ

DEFINES += Terminal_version Q_JSON_SUPPORT GSL ARMA_USE_LAPACK ARMA_USE_BLAS NO_OPENMP
macx:  DEFINES += mac_version
linux: DEFINES += ubuntu_version
win32: DEFINES += windows_version

linux: LIBS += -larmadillo -llapack -lblas -lgsl -lgslcblas -lopenblas
macx {
    INCLUDEPATH += /opt/homebrew/include /usr/local/include
    LIBS += -L/opt/homebrew/lib -L/usr/local/lib -larmadillo -llapack -lblas -lgsl
}
win32 {
    isEmpty(ARMA_DIR):     ARMA_DIR     = C:/Projects/armadillo-15.2.3
    isEmpty(OPENBLAS_DIR): OPENBLAS_DIR = C:/Projects/OpenBLAS-0.3.31-x64/win64
    isEmpty(GSL_DIR):      GSL_DIR      = C:/Projects/vcpkg/packages/gsl_x64-windows
    DEFINES += ARMA_DONT_USE_WRAPPER _WINDOWS WIN64 NOMINMAX _USE_MATH_DEFINES
    INCLUDEPATH += $$ARMA_DIR/include $$OPENBLAS_DIR/include $$GSL_DIR/include
    LIBS += -L$$OPENBLAS_DIR/lib -llibopenblas -L$$GSL_DIR/lib -lgsl -lgslcblas
    QMAKE_CXXFLAGS += /MP /bigobj /utf-8
}

SOURCES += \
        $$OHQ/codegen/src/ohq_generate.cpp \
        $$OHQ/codegen/src/CodeGenerator.cpp \
        $$OHQ/codegen/src/DependencyAnalyzer.cpp \
        $$OHQ/codegen/src/EmbeddedRuntime.cpp \
        $$OHQ/codegen/src/ExpressionEmitter.cpp \
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
        $$JSONCPP/src/lib_json/json_writer.cpp
