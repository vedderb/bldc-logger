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

#include "logger.h"
#include <QCoreApplication>
#include <QDebug>

Logger::Logger(QObject *parent) :
    QObject(parent)
{
    mPort = new SerialPort(this);
    mPacketInterface = new PacketInterface(this);

    mValueFile = new QFile("Data/BLDC_Values");
    mPrintFile = new QFile("Data/BLDC_Print");

    mValueFile->open(QIODevice::WriteOnly | QIODevice::Text);
    mPrintFile->open(QIODevice::WriteOnly | QIODevice::Text);

    mValueStream = new QTextStream(mValueFile);
    mPrintStream = new QTextStream(mPrintFile);

    mPort->openPort("/dev/ttyACM0");

    // Video
    mVidW = 1280;
    mVidH = 720;
    mVidFps = 25.0;
    mFAudioSamp = 44100;

    mFrameGrabber = new FrameGrabber(mVidW, mVidH, mVidFps, 0, this);
    mFrameGrabber->start(QThread::InheritPriority);
    mPlotter = new FramePlotter(this);
    mPlotter->start(QThread::InheritPriority);

    mCoder = new VideoCoder(mVidW, mVidH, mVidFps, "Data/v_video.avi", this);
    mCoder->start(QThread::InheritPriority);

    // Audio recording
    mTimer = 0;
    mAudio = 0;

    if (QAudioDeviceInfo::availableDevices(QAudio::AudioInput).size() > 0) {
        mAudioFile.setFileName("Data/v_audio.raw");
        mAudioFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

        QAudioFormat format;
        // Set up the desired format, for example:
        format.setSampleRate(mFAudioSamp);
        format.setChannelCount(1);
        format.setSampleSize(8);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(format)) {
            qWarning() << "Default format not supported, trying to use the nearest.";
            format = info.nearestFormat(format);
        }

        mAudio = new QAudioInput(format, this);
        mAudio->setNotifyInterval(1000 / mVidFps);
        mAudio->start(&mAudioFile);
    } else {
        mTimer = new QTimer(this);
        mTimer->setInterval(1000 / mVidFps);
        mTimer->start();
    }

    mConsoleReader = new ConsoleReader(this);

    connect(mConsoleReader, SIGNAL(textReceived(QString)),
            this, SLOT(consoleLineReceived(QString)));

    connect(mPort, SIGNAL(serial_data_available()),
            this, SLOT(serialDataAvailable()));

    if (mTimer != 0) {
        connect(mTimer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    }

    if (mAudio != 0) {
        connect(mAudio, SIGNAL(notify()),
                this, SLOT(audioNotify()));

        // Lower the volume to avoid clipping. This seems to be passed to
        // pulseaudio.
        mAudio->setVolume(0.1);
    }

    connect(mPacketInterface, SIGNAL(dataToSend(QByteArray&)),
            this, SLOT(packetDataToSend(QByteArray&)));
    connect(mPacketInterface, SIGNAL(valuesReceived(PacketInterface::MC_VALUES)),
            this, SLOT(mcValuesReceived(PacketInterface::MC_VALUES)));
    connect(mPacketInterface, SIGNAL(printReceived(QString)),
            this, SLOT(printReceived(QString)));
    connect(mPacketInterface, SIGNAL(samplesReceived(QByteArray)),
            this, SLOT(samplesReceived(QByteArray)));
    connect(mPacketInterface, SIGNAL(rotorPosReceived(double)),
            this, SLOT(rotorPosReceived(double)));
    connect(mPacketInterface, SIGNAL(experimentSamplesReceived(QVector<double>)),
            this, SLOT(experimentSamplesReceived(QVector<double>)));

    connect(mPlotter, SIGNAL(frameReady(QImage)),
            mCoder, SLOT(setNextFrame(QImage)));
}

Logger::~Logger()
{
    mValueFile->close();
    mPrintFile->close();
    mFrameGrabber->stopAndWait();
    mPlotter->stopAndWait();
    mCoder->stopAndWait();

    if (mAudio != 0) {
        mAudioFile.close();
        delete mAudio;
        rawToWav("Data/v_audio.wav", "Data/v_audio.raw", mFAudioSamp);
    }

    delete mValueStream;
    delete mPrintStream;
    delete mValueFile;
    delete mPrintFile;
}

void Logger::consoleLineReceived(QString line)
{
    if (line == "q" || line == "Q") {
        qDebug() << "Saving files...";

        if (mAudio != 0) {
            mAudio->stop();
        }

        QCoreApplication::exit(0);
    } else if (line == "1") {
        mPacketInterface->setCurrent(20);
    } else if (line == "2") {
        mPacketInterface->setCurrent(-20);
    } else if (line == "3") {
        mPacketInterface->setDutyCycle(0.1);
    } else if (line == "4") {
        mPacketInterface->setDutyCycle(-0.1);
    } else if (line == "5") {
        mPacketInterface->setRpm(2000);
    } else if (line == "6") {
        mPacketInterface->setRpm(-2000);
    } else {
        qDebug() << "Unknown command:" << line;
        qDebug() << "Stopping motor";
        mPacketInterface->setCurrent(0);
    }
}

void Logger::serialDataAvailable()
{
    while (mPort->bytesAvailable() > 0) {
        QByteArray data = mPort->readAll();
        mPacketInterface->processData(data);
    }
}

void Logger::timerSlot()
{
    mPacketInterface->getValues();
}

void Logger::packetDataToSend(QByteArray &data)
{
    if (mPort->isOpen()) {
        mPort->writeData(data.data(), data.size());
    }
}

void Logger::mcValuesReceived(PacketInterface::MC_VALUES values)
{
    QString str;
    str.sprintf("%.1f,%.2f,%.1f,%.2f,%.2f,%d\n",
                    values.v_in,
                    values.duty_now,
                    values.rpm,
                    values.current_in,
                    values.current_motor,
                    values.fault_code);

    *mValueStream << str;

    mLastMcVal = values;
}

void Logger::printReceived(QString str)
{
    *mPrintStream << str;
}

void Logger::samplesReceived(QByteArray data)
{
    (void)data;
}

void Logger::rotorPosReceived(double pos)
{
    (void)pos;
}

void Logger::experimentSamplesReceived(QVector<double> samples)
{
    (void)samples;
}

void Logger::audioNotify()
{
    mPacketInterface->sendTerminalCmd("fault");
    mPacketInterface->getValues();

    if (mFrameGrabber->isOpened()) {
        FramePlotter::FrameData data;

        data.img = mFrameGrabber->getLatestFrame();
        data.values = mLastMcVal;

        mPlotter->setNextValues(data);
    } else {
        qDebug() << "mFrameGrabber not open";
    }
}

void Logger::write_little_endian(unsigned int word, int num_bytes, QFile &wav_file)
{
    char buf;
    while(num_bytes > 0) {
        buf = word & 0xff;
        wav_file.write(&buf, 1);
        num_bytes--;
        word >>= 8;
    }
}

// Found at: http://karplus4arduino.wordpress.com/2011/10/08/making-wav-files-from-c-programs/
void Logger::rawToWav(const char *out_file, const char *in_file, int s_rate)
{
    QFile wav_file;
    wav_file.setFileName(out_file);

    QFile raw_file;
    raw_file.setFileName(in_file);

    unsigned int sample_rate;
    unsigned int num_channels;
    unsigned int bytes_per_sample;
    unsigned int byte_rate;

    num_channels = 1;   /* monoaural */
    bytes_per_sample = 1;

    if (s_rate <= 0) {
        sample_rate = 44100;
    } else {
        sample_rate = (unsigned int)s_rate;
    }

    byte_rate = sample_rate * num_channels * bytes_per_sample;

    wav_file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    raw_file.open(QIODevice::ReadOnly);

    int num_samples = raw_file.size() / 2;

    /* write RIFF header */
    wav_file.write("RIFF", 4);
    write_little_endian(36 + bytes_per_sample * num_samples * num_channels, 4, wav_file);
    wav_file.write("WAVE", 4);

    /* write fmt  subchunk */
    wav_file.write("fmt ", 4);
    write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
    write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
    write_little_endian(num_channels, 2, wav_file);
    write_little_endian(sample_rate, 4, wav_file);
    write_little_endian(byte_rate, 4, wav_file);
    write_little_endian(num_channels * bytes_per_sample, 2, wav_file);  /* block align */
    write_little_endian(8 * bytes_per_sample, 2, wav_file);  /* bits/sample */

    /* write data subchunk */
    wav_file.write("data", 4);

    for(;;) {
        QByteArray d = raw_file.read(500);
        if (d.isEmpty()) {
            break;
        }
        wav_file.write(d);
    }

    wav_file.close();
    raw_file.close();
}
