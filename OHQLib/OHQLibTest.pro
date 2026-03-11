QT -= gui
QT += core

CONFIG += console c++14
CONFIG -= app_bundle
TEMPLATE = app

TARGET = OHQLibTest

# Link against OHQLib shared library
LIBS += -L$$PWD/../OHQLib/build/Desktop_Qt_6_8_3-Release/ -lOHQLib
INCLUDEPATH += $$PWD/../../build-OHQLib-Desktop-Release/

# Headers
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

SOURCES += main.cpp
