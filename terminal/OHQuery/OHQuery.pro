QT -= gui
QT += core websockets

CONFIG += console
CONFIG -= app_bundle

CONFIG += debug
CONFIG -= release

QMAKE_CXXFLAGS_DEBUG += -O0 -g

CONFIG += c++17

INCLUDEPATH += ../../aquifolium/include
INCLUDEPATH += ../../aquifolium/src
INCLUDEPATH += ../../aquifolium/include/GA
INCLUDEPATH += ../../aquifolium/include/MCMC
INCLUDEPATH += ../../../jsoncpp/include/

if==macx:CONFIG += staticlib
macx: DEFINES +=mac_version
linux: DEFINES +=ubuntu_version
win32: DEFINES +=windows_version

DEFINES += Terminal_version

INCLUDEPATH += include

TARGET = OHQServer
TEMPLATE = app
win32:QMAKE_CXXFLAGS += /MP

macx: {
    QMAKE_CXXFLAGS += -Xpreprocessor -fopenmp -lomp -Iusr/local/lib/
}

macx: {
    QMAKE_LFLAGS += -lomp
}

macx: {
    LIBS += -L /usr/local/lib /usr/local/lib/libomp.dylib
}

macx: {
    INCLUDEPATH += /usr/local/include/
}


CONFIG(debug, debug|release) {
    message(Building in debug mode)
    !macx: QMAKE_CXXFLAGS *= "-Xpreprocessor -fopenmp"
    !macx: QMAKE_LFLAGS +=  -fopenmp
    !macx: LIBS += -lgomp -lpthread
    LIBS += -lpthread
    DEFINES += NO_OPENMP DEBUG

} else {
    message(Building in release mode)
    !macx: QMAKE_CXXFLAGS *= "-Xpreprocessor -fopenmp"
    !macx: QMAKE_LFLAGS +=  -fopenmp
    # QMAKE_CFLAGS+=-pg
    # QMAKE_CXXFLAGS+=-pg
    # QMAKE_LFLAGS+=-pg
    # macx: DEFINES += NO_OPENMP
    ! macx: LIBS += -lgomp -lpthread
    macx: LIBS += -lpthread
}


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../../aquifolium/src/Block.cpp \
        ../../aquifolium/src/Command.cpp \
        ../../aquifolium/src/Condition.cpp \
        ../../aquifolium/src/ErrorHandler.cpp \
        ../../aquifolium/src/Expression.cpp \
        ../../aquifolium/src/Link.cpp \
        ../../aquifolium/src/Matrix.cpp \
        ../../aquifolium/src/Matrix_arma.cpp \
        ../../aquifolium/src/MetaModel.cpp \
        ../../aquifolium/src/NormalDist.cpp \
        ../../aquifolium/src/Object.cpp \
        ../../aquifolium/src/Objective_Function.cpp \
        ../../aquifolium/src/Objective_Function_Set.cpp \
        ../../aquifolium/src/Parameter.cpp \
        ../../aquifolium/src/Parameter_Set.cpp \
        ../../aquifolium/src/Precipitation.cpp \
        ../../aquifolium/src/Quan.cpp \
        ../../aquifolium/src/QuanSet.cpp \
        ../../aquifolium/src/QuickSort.cpp \
        ../../aquifolium/src/Rule.cpp \
        ../../aquifolium/src/RxnParameter.cpp \
        ../../aquifolium/src/Script.cpp \
        ../../aquifolium/src/Source.cpp \
        ../../aquifolium/src/System.cpp \
        ../../aquifolium/src/Utilities.cpp \
        ../../aquifolium/src/Vector.cpp \
        ../../aquifolium/src/Vector_arma.cpp \
        ../../aquifolium/src/constituent.cpp \
        ../../aquifolium/src/observation.cpp \
        ../../aquifolium/src/precalculatedfunction.cpp \
        ../../aquifolium/src/reaction.cpp \
        ../../aquifolium/src/restorepoint.cpp \
        ../../aquifolium/src/solutionlogger.cpp \
        ../../aquifolium/src/GA/Binary.cpp \
        ../../aquifolium/src/GA/Individual.cpp \
        ../../aquifolium/src/GA/DistributionNUnif.cpp \
        ../../aquifolium/src/GA/Distribution.cpp \
        ../../../jsoncpp/src/lib_json/json_reader.cpp \
        ../../../jsoncpp/src/lib_json/json_value.cpp \
        ../../../jsoncpp/src/lib_json/json_writer.cpp \
        main.cpp \
        serverops.cpp \
        weatherretriever.cpp \
        wsserverop.cpp

HEADERS += \
    ../../aquifolium/include/Objective_Function.h \
    ../../aquifolium/include/Objective_Function_Set.h \
    ../../aquifolium/include/Precipitation.h \
    ../../aquifolium/include/RxnParameter.h \
    ../../aquifolium/include/constituent.h \
    ../../aquifolium/include/observation.h \
    ../../aquifolium/include/precalculatedfunction.h \
    ../../aquifolium/include/solutionlogger.h \
    ../../aquifolium/include/GA/GA.h \
    ../../aquifolium/include/MCMC/MCMC.h \
    ../../aquifolium/include/MCMC/MCMC.hpp \
    ../../aquifolium/include/Utilities.h \
    ../../aquifolium/include/restorepoint.h \
    ../../aquifolium/include/safevector.h \
    ../../aquifolium/include/safevector.hpp \
    ../../aquifolium/include/Block.h \
    ../../aquifolium/include/BTC.h \
    ../../aquifolium/include/BTCSet.h \
    ../../aquifolium/include/Expression.h \
    ../../aquifolium/include/Link.h \
    ../../aquifolium/include/Matrix.h \
    ../../aquifolium/include/Matrix_arma.h \
    ../../aquifolium/include/MetaModel.h \
    ../../aquifolium/include/NormalDist.h \
    ../../aquifolium/include/Object.h \
    ../../aquifolium/include/Quan.h \
    ../../aquifolium/include/QuanSet.h \
    ../../aquifolium/include/QuickSort.h \
    ../../aquifolium/include/System.h \
    ../../aquifolium/include/Vector.h \
    ../../aquifolium/include/Vector_arma.h \
    ../../../jsoncpp/include/json/allocator.h \
    ../../../jsoncpp/include/json/assertions.h \
    ../../../jsoncpp/include/json/autolink.h \
    ../../../jsoncpp/include/json/config.h \
    ../../../jsoncpp/include/json/features.h \
    ../../../jsoncpp/include/json/forwards.h \
    ../../../jsoncpp/include/json/json.h \
    ../../../jsoncpp/include/json/reader.h \
    ../../../jsoncpp/include/json/value.h \
    ../../../jsoncpp/include/json/version.h \
    ../../../jsoncpp/include/json/writer.h \
    ../../../jsoncpp/src/lib_json/json_tool.h \
    ../../../jsoncpp/src/lib_json/version.h.in \
    ../../aquifolium/include/Parameter.h \
    ../../aquifolium/include/Parameter_Set.h \
    ../../aquifolium/include/Command.h \
    ../../aquifolium/include/Script.h \
    ../../aquifolium/include/GA/Binary.h \
    ../../aquifolium/include/GA/Distribution.h \
    ../../aquifolium/include/GA/DistributionNUnif.h \
    ../../aquifolium/include/GA/Individual.h \
    ../../aquifolium/include/Objective_Function.h \
    ../../aquifolium/include/Objective_Function_Set.h \
    ../../aquifolium/include/GA/GA.hpp \
    ../../aquifolium/src/BTC.hpp \
    ../../aquifolium/src/BTCSet.hpp \
    ../../aquifolium/include/reaction.h \
    cors_middleware.h \
    serverops.h \
    weatherretriever.h \
    wsserverop.h


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
     DEFINES += ARMA_USE_LAPACK ARMA_USE_BLAS GSL
     LIBS += -larmadillo -llapack -lblas -lgsl
}

macx {

    LIBS += /opt/homebrew/Cellar/armadillo/11.4.2/lib/libarmadillo.dylib
    INCLUDEPATH += $$PWD/../../../../opt/homebrew/Cellar/armadillo/11.4.2/include
    DEPENDPATH += $$PWD/../../../../opt/homebrew/Cellar/armadillo/11.4.2/include

}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../../aquifolium/src/BTC.hpp \
    ../../aquifolium/src/BTCSet.hpp
