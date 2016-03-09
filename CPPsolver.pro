#-------------------------------------------------
#
# Project created by QtCreator 2016-02-13T14:55:32
#
#-------------------------------------------------

QT       += core gui

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
    graphviz.cpp

HEADERS  += mainwindow.h \
    network.h \
    exceptions.h \
    CPService.h \
    CPAlgorothm.h \
    CPSettings.h \
    graphmlReader.h \
    topoelements.h \
    graphviz.h

FORMS    += mainwindow.ui \
    cppparameters.ui \
    alogorithmparametrs.ui

DISTFILES += \
    README.md
