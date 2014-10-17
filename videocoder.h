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

#ifndef VIDEOCODER_H
#define VIDEOCODER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QFile>
#include <QAudioInput>
#include <QList>
#include <QImage>
#include <opencv/cv.h>
#include <opencv/highgui.h>

class VideoCoder : public QThread
{
    Q_OBJECT
public:
    explicit VideoCoder(int w, int h, double fps, const char *file, QObject *parent = 0);
    void stopAndWait();

signals:

public slots:
    void setNextFrame(QImage img);

protected:
    void run();

private:
    volatile bool mAbort;
    volatile bool mFrameReady;
    QList<QImage> mNextFrame;
    cv::VideoWriter mVideoWriter;
    QMutex mMutex;

};

#endif // VIDEOCODER_H
