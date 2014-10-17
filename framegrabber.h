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

#ifndef FRAMEGRABBER_H
#define FRAMEGRABBER_H

#include <QThread>
#include <QImage>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QMutex>
#include "MatToQImage.h"

class FrameGrabber : public QThread
{
    Q_OBJECT
public:
    explicit FrameGrabber(int w, int h, int fps, int dev = 0, QObject *parent = 0);
    QImage getLatestFrame();
    bool isOpened();
    void stopAndWait();

signals:

public slots:

protected:
    void run();

private:
    cv::VideoCapture mVideoCap;
    QImage mLatestFrame;
    cv::Mat mFrameMat;
    QMutex mMutex;
    volatile bool mAbort;

};

#endif // FRAMEGRABBER_H
