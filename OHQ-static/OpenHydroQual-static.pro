# ================================
# OpenHydroQual - Static Library
# ================================

QT += core
QT -= gui widgets

TEMPLATE = lib
CONFIG  += staticlib c++17

TARGET = OpenHydroQual

# --------------------------------
# Global DEFINES
# --------------------------------
DEFINES += Terminal_version
DEFINES += Q_JSON_SUPPORT
DEFINES += GSL

# Armadillo: use system BLAS/LAPACK (no wrapper)
DEFINES += ARMA_DONT_USE_WRAPPER ARMA_USE_LAPACK ARMA_USE_BLAS

# --------------------------------
# C++ Standard
# --------------------------------
QMAKE_CXXFLAGS += -std=c++17

# --------------------------------
# MACHINE-SPECIFIC PATHS
# --------------------------------

CONFIG += PowerEdge
#CONFIG += Hooman
#CONFIG += Arash
#CONFIG += SligoCreek

CONFIG(PowerEdge) {
    OHQPATH        = /mnt/3rd900/Projects/OpenHydroQual/aquifolium
    OHQ_LIB_OUTPUT = /mnt/3rd900/Projects/DryWellScriptGenerator/OHQ-static
}

CONFIG(Hooman) {
    OHQPATH        = /home/hoomanmoradpour/Projects/OpenHydroQual/aquifolium
    OHQ_LIB_OUTPUT = /home/hoomanmoradpour/Projects/DryWellScriptGenerator/OHQ-static
}

CONFIG(Arash) {
    OHQPATH        = /home/arash/Projects/OpenHydroQual/aquifolium
    OHQ_LIB_OUTPUT = /home/arash/Projects/DryWellScriptGenerator/OHQ-static
}

CONFIG(SligoCreek) {
    OHQPATH        = /media/arash/E/Projects/OpenHydroQual/aquifolium
    OHQ_LIB_OUTPUT = /media/arash/E/Projects/DryWellScriptGenerator/OHQ-static
}

# --------------------------------
# Output directory for libOpenHydroQual.a
# --------------------------------
DESTDIR = $${OHQ_LIB_OUTPUT}

# --------------------------------
# Include Paths
# --------------------------------
INCLUDEPATH += $${OHQPATH}
INCLUDEPATH += $${OHQPATH}/include
INCLUDEPATH += $${OHQPATH}/include/GA
INCLUDEPATH += $${OHQPATH}/include/MCMC
INCLUDEPATH += $${OHQPATH}/src
INCLUDEPATH += $${OHQPATH}/../jsoncpp/include

# --------------------------------
# Armadillo / GSL / OpenMP / BLAS
# (Note: LIBS here are mostly for test/link checks;
#        the final app (DryWell) must also link them.)
# --------------------------------
unix:!macx {
    INCLUDEPATH += /usr/include

    # OpenMP
    QMAKE_CXXFLAGS += -fopenmp -O3 -march=native
    QMAKE_LFLAGS   += -fopenmp
    LIBS           += -lgomp -lpthread

    # Armadillo + OpenBLAS + GSL
    LIBS += -larmadillo -lopenblas -lgsl -lgfortran
}

macx {
    INCLUDEPATH += /opt/homebrew/include
    LIBS += -L/opt/homebrew/lib -larmadillo -lopenblas -lgsl
}

# --------------------------------
# Source Files (OpenHydroQual Engine)
# --------------------------------
SOURCES += \
    $${OHQPATH}/src/Block.cpp \
    $${OHQPATH}/src/Command.cpp \
    $${OHQPATH}/src/Condition.cpp \
    $${OHQPATH}/src/ErrorHandler.cpp \
    $${OHQPATH}/src/Expression.cpp \
    $${OHQPATH}/src/Link.cpp \
    $${OHQPATH}/src/Matrix.cpp \
    $${OHQPATH}/src/Matrix_arma.cpp \
    $${OHQPATH}/src/MetaModel.cpp \
    $${OHQPATH}/src/NormalDist.cpp \
    $${OHQPATH}/src/Object.cpp \
    $${OHQPATH}/src/Objective_Function.cpp \
    $${OHQPATH}/src/Objective_Function_Set.cpp \
    $${OHQPATH}/src/Parameter.cpp \
    $${OHQPATH}/src/Parameter_Set.cpp \
    $${OHQPATH}/src/Precipitation.cpp \
    $${OHQPATH}/src/Quan.cpp \
    $${OHQPATH}/src/QuanSet.cpp \
    $${OHQPATH}/src/QuickSort.cpp \
    $${OHQPATH}/src/Rule.cpp \
    $${OHQPATH}/src/RxnParameter.cpp \
    $${OHQPATH}/src/Script.cpp \
    $${OHQPATH}/src/Source.cpp \
    $${OHQPATH}/src/System.cpp \
    $${OHQPATH}/src/Utilities.cpp \
    $${OHQPATH}/src/Vector.cpp \
    $${OHQPATH}/src/Vector_arma.cpp \
    $${OHQPATH}/src/constituent.cpp \
    $${OHQPATH}/src/observation.cpp \
    $${OHQPATH}/src/precalculatedfunction.cpp \
    $${OHQPATH}/src/reaction.cpp \
    $${OHQPATH}/src/restorepoint.cpp \
    $${OHQPATH}/src/solutionlogger.cpp \
    $${OHQPATH}/src/GA/Binary.cpp \
    $${OHQPATH}/src/GA/Individual.cpp \
    $${OHQPATH}/src/GA/DistributionNUnif.cpp \
    $${OHQPATH}/src/GA/Distribution.cpp \
    $${OHQPATH}/../jsoncpp/src/lib_json/json_reader.cpp \
    $${OHQPATH}/../jsoncpp/src/lib_json/json_value.cpp \
    $${OHQPATH}/../jsoncpp/src/lib_json/json_writer.cpp

# --------------------------------
# Header Files
# --------------------------------
HEADERS += $$files($${OHQPATH}/include/*.h)
HEADERS += $$files($${OHQPATH}/include/**/*.h)
HEADERS += $$files($${OHQPATH}/src/*.hpp)
HEADERS += $$files($${OHQPATH}/src/**/*.hpp)
HEADERS += $$files($${OHQPATH}/../jsoncpp/include/json/*.h)
