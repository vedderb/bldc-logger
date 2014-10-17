/*
    Copyright 2012-2014 Benjamin Vedder	benjamin@vedder.se

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <consolereader.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "MatToQImage.h"
#include "serialport.h"
#include "packetinterface.h"
#include "qcustomplot.h"
#include "videocoder.h"
#include "frameplotter.h"
#include "framegrabber.h"

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);
    ~Logger();

signals:

public slots:


private slots:
    void consoleLineReceived(QString line);
    void serialDataAvailable();
    void timerSlot();
    void packetDataToSend(QByteArray &data);
    void mcValuesReceived(PacketInterface::MC_VALUES values);
    void printReceived(QString str);
    void samplesReceived(QByteArray data);
    void rotorPosReceived(double pos);
    void experimentSamplesReceived(QVector<double> samples);
    void audioNotify();

private:
    SerialPort *mPort;
    QTimer *mTimer;
    PacketInterface *mPacketInterface;
    QFile *mValueFile;
    QFile *mPrintFile;
    QTextStream *mValueStream;
    QTextStream *mPrintStream;
    ConsoleReader *mConsoleReader;
    FrameGrabber *mFrameGrabber;
    VideoCoder *mCoder;
    FramePlotter *mPlotter;
    int mVidW;
    int mVidH;
    double mVidFps;
    QFile mAudioFile;
    QAudioInput *mAudio;
    int mFAudioSamp;
    PacketInterface::MC_VALUES mLastMcVal;

    void write_little_endian(unsigned int word, int num_bytes, QFile &wav_file);
    void rawToWav(const char *out_file, const char *in_file, int s_rate);

};

#endif // LOGGER_H
