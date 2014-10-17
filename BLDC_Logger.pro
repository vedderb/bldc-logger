#-------------------------------------------------
#
# Project created by QtCreator 2014-07-19T13:28:24
#
#-------------------------------------------------

# On Ubuntu:
# sudo apt-get install qt-sdk libqt5multimedia5 libqt5multimedia5-plugins qtmultimedia5-dev

QT       += core
QT       += gui
QT       += printsupport
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BLDC_Logger
CONFIG   += console
CONFIG   -= app_bundle

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui \
        -lopencv_ml -lopencv_video -lopencv_features2d \
        -lopencv_calib3d -lopencv_objdetect -lopencv_contrib \
        -lopencv_legacy -lopencv_flann

TEMPLATE = app


SOURCES += main.cpp \
    packetinterface.cpp \
    serialport.cpp \
    logger.cpp \
    consolereader.cpp \
    utility.cpp \
    MatToQImage.cpp \
    qcustomplot.cpp \
    videocoder.cpp \
    frameplotter.cpp \
    framegrabber.cpp

HEADERS += \
    packetinterface.h \
    serialport.h \
    logger.h \
    consolereader.h \
    utility.h \
    MatToQImage.h \
    qcustomplot.h \
    videocoder.h \
    frameplotter.h \
    framegrabber.h
