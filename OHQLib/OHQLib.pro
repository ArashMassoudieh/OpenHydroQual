QT -= gui
QT += core

# Build BOTH shared and static
CONFIG += shared_and_static
TEMPLATE = lib

# Shared library name
TARGET = OHQLib
VERSION = 2.0.4

CONFIG += c++14

INCLUDEPATH += ../
INCLUDEPATH += $$PWD/../aquifolium/include
INCLUDEPATH += $$PWD/../aquifolium/src
INCLUDEPATH += $$PWD/../aquifolium/include/GA
INCLUDEPATH += $$PWD/../aquifolium/include/MCMC
INCLUDEPATH += $$PWD/../../jsoncpp/include/
INCLUDEPATH += $$PWD/../

macx: DEFINES += mac_version
linux: DEFINES += ubuntu_version
win32: DEFINES += windows_version

DEFINES += Terminal_version
DEFINES += Q_JSON_SUPPORT

win32: QMAKE_CXXFLAGS += /MP

macx: {
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp -lomp -Iusr/local/lib/
    QMAKE_LFLAGS += -lomp
    LIBS += -L /usr/local/lib /usr/local/lib/libomp.dylib
    INCLUDEPATH += /usr/local/include/
}

CONFIG(debug, debug|release) {
    !macx: QMAKE_CXXFLAGS *= "-Xpreprocessor -fopenmp"
    !macx: QMAKE_LFLAGS += -fopenmp
    !macx: LIBS += -lgomp -lpthread
    LIBS += -lpthread
    DEFINES += NO_OPENMP DEBUG
} else {
    !macx: QMAKE_CXXFLAGS *= "-Xpreprocessor -fopenmp"
    !macx: QMAKE_LFLAGS += -fopenmp
    !macx: LIBS += -lgomp -lpthread
    macx: LIBS += -lpthread
}

# All sources from the existing console .pro EXCEPT main.cpp
SOURCES += \
    ../aquifolium/src/Block.cpp \
    ../aquifolium/src/Command.cpp \
    ../aquifolium/src/Condition.cpp \
    ../aquifolium/src/ErrorHandler.cpp \
    ../aquifolium/src/Expression.cpp \
    ../aquifolium/src/Link.cpp \
    ../aquifolium/src/Matrix.cpp \
    ../aquifolium/src/Matrix_arma.cpp \
    ../aquifolium/src/MetaModel.cpp \
    ../aquifolium/src/NormalDist.cpp \
    ../aquifolium/src/Object.cpp \
    ../aquifolium/src/Objective_Function.cpp \
    ../aquifolium/src/Objective_Function_Set.cpp \
    ../aquifolium/src/Parameter.cpp \
    ../aquifolium/src/Parameter_Set.cpp \
    ../aquifolium/src/Precipitation.cpp \
    ../aquifolium/src/Quan.cpp \
    ../aquifolium/src/QuanSet.cpp \
    ../aquifolium/src/QuickSort.cpp \
    ../aquifolium/src/Rule.cpp \
    ../aquifolium/src/RxnParameter.cpp \
    ../aquifolium/src/Script.cpp \
    ../aquifolium/src/Source.cpp \
    ../aquifolium/src/System.cpp \
    ../aquifolium/src/Utilities.cpp \
    ../aquifolium/src/Vector.cpp \
    ../aquifolium/src/Vector_arma.cpp \
    ../aquifolium/src/constituent.cpp \
    ../aquifolium/src/observation.cpp \
    ../aquifolium/src/precalculatedfunction.cpp \
    ../aquifolium/src/reaction.cpp \
    ../aquifolium/src/restorepoint.cpp \
    ../aquifolium/src/solutionlogger.cpp \
    ../aquifolium/src/GA/Binary.cpp \
    ../aquifolium/src/GA/Individual.cpp \
    ../aquifolium/src/GA/DistributionNUnif.cpp \
    ../aquifolium/src/GA/Distribution.cpp \
    ../jsoncpp/src/lib_json/json_reader.cpp \
    ../jsoncpp/src/lib_json/json_value.cpp \
    ../jsoncpp/src/lib_json/json_writer.cpp

HEADERS += \
    ../XString.h \
    ../aquifolium/include/Objective_Function.h \
    ../aquifolium/include/Objective_Function_Set.h \
    ../aquifolium/include/Precipitation.h \
    ../aquifolium/include/RxnParameter.h \
    ../aquifolium/include/constituent.h \
    ../aquifolium/include/observation.h \
    ../aquifolium/include/precalculatedfunction.h \
    ../aquifolium/include/solutionlogger.h \
    ../aquifolium/include/GA/GA.h \
    ../aquifolium/include/MCMC/MCMC.h \
    ../aquifolium/include/MCMC/MCMC.hpp \
    ../aquifolium/include/Utilities.h \
    ../aquifolium/include/restorepoint.h \
    ../aquifolium/include/Block.h \
    ../aquifolium/include/BTC.h \
    ../aquifolium/include/BTCSet.h \
    ../aquifolium/include/Expression.h \
    ../aquifolium/include/Link.h \
    ../aquifolium/include/Matrix.h \
    ../aquifolium/include/Matrix_arma.h \
    ../aquifolium/include/MetaModel.h \
    ../aquifolium/include/NormalDist.h \
    ../aquifolium/include/Object.h \
    ../aquifolium/include/Parameter.h \
    ../aquifolium/include/Parameter_Set.h \
    ../aquifolium/include/Quan.h \
    ../aquifolium/include/QuanSet.h \
    ../aquifolium/include/System.h \
    ../aquifolium/include/Vector.h \
    ../aquifolium/include/Vector_arma.h \
    ../aquifolium/include/Command.h \
    ../aquifolium/include/Script.h \
    ../aquifolium/include/reaction.h \
    ../aquifolium/src/BTC.hpp \
    ../aquifolium/src/BTCSet.hpp

# LAPACK
win32 {
    LAPACK_INCLUDE = $$PWD/../include
    contains(QMAKE_TARGET.arch, x86_64) {
        CONFIG(debug, debug|release) {
            LAPACK_LIB_DIR = $$PWD/../libs/lapack-blas_lib_win64/debug
            LIBS += -L$${LAPACK_LIB_DIR} -llapack_win64_MTd -lblas_win64_MTd
        }
        CONFIG(release, debug|release) {
            LAPACK_LIB_DIR = $$PWD/../libs/lapack-blas_lib_win64/release
            LIBS += -L$${LAPACK_LIB_DIR} -llapack_win64_MT -lblas_win64_MT
        }
    }
    INCLUDEPATH += $${LAPACK_INCLUDE}
    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS
}

linux {
    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS GSL
    LIBS += -larmadillo -llapack -lblas -lgsl
}

macx {
    LIBS += /opt/homebrew/Cellar/armadillo/11.4.2/lib/libarmadillo.dylib
    INCLUDEPATH += $$PWD/../../../../../opt/homebrew/Cellar/armadillo/11.4.2/include
}
