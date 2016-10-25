#-------------------------------------------------
#
# Project created by QtCreator 2016-02-13T14:55:32
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CPPsolver
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    network.cpp \
    CPService.cpp \
    CPAlgorothm.cpp \
    CPSettings.cpp \
    graphmlReader.cpp \
    graphviz.cpp \
    topogenerator.cpp

HEADERS  += mainwindow.h \
    network.h \
    exceptions.h \
    CPService.h \
    CPAlgorothm.h \
    CPSettings.h \
    graphmlReader.h \
    topoelements.h \
    graphviz.h \
    topogenerator.h

FORMS    += mainwindow.ui \
    cppparameters.ui \
    alogorithmparametrs.ui \
    topogeneratordialog.ui

DISTFILES += \
    README.md
