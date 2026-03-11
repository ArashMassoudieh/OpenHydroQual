QT -= gui
QT += core

TEMPLATE = lib
CONFIG += plugin no_plugin_name_prefix c++14
TARGET = openhydroqual_py

VERSION = 2.0.4

# Link against OHQLib
LIBS += -L$$PWD/../OHQLib/build/Desktop_Qt_6_8_2-Release/ -lOHQLib
QMAKE_LFLAGS += -Wl,-rpath,$$PWD/../OHQLib/build/Desktop_Qt_6_8_2-Release/

# OHQLib headers
INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../aquifolium/include
INCLUDEPATH += $$PWD/../aquifolium/src
INCLUDEPATH += $$PWD/../aquifolium/include/GA
INCLUDEPATH += $$PWD/../aquifolium/include/MCMC
INCLUDEPATH += $$PWD/../../jsoncpp/include/

# pybind11 and Python — hardcoded
INCLUDEPATH += /home/arash/Projects/ohq_rl_env/lib/python3.12/site-packages/pybind11/include
INCLUDEPATH += /usr/include/python3.12
LIBS += -L/usr/lib/python3.12/config-3.12-x86_64-linux-gnu
LIBS += -L/usr/lib/x86_64-linux-gnu
LIBS += -lpython3.12 -ldl -lm

# Platform defines
macx:  DEFINES += mac_version
linux: DEFINES += ubuntu_version
win32: DEFINES += windows_version
DEFINES += Terminal_version Q_JSON_SUPPORT

# Platform libs
linux {
    DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS GSL
    LIBS += -larmadillo -llapack -lblas -lgsl -lgomp -lpthread
}

macx {
    LIBS += /opt/homebrew/Cellar/armadillo/11.4.2/lib/libarmadillo.dylib
    INCLUDEPATH += $$PWD/../../../../../opt/homebrew/Cellar/armadillo/11.4.2/include
}

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

# Output as Python-importable module
linux:  TARGET_EXT = .so
macx:   TARGET_EXT = .so
win32:  TARGET_EXT = .pyd

SOURCES += bindings.cpp
