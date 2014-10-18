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

#include "videocoder.h"
#include <QtDebug>
#include "MatToQImage.h"

VideoCoder::VideoCoder(int w, int h, double fps, const char *file, QObject *parent) :
    QThread(parent)
{
    mAbort = false;
    mFrameReady = false;

    cv::Size S = cv::Size(w, h);
//    mVideoWriter.open(file, CV_FOURCC('H','2','6','4'), fps, S);
    mVideoWriter.open(file, CV_FOURCC('X','V','I','D'), fps, S);
//    mVideoWriter.open(file, CV_FOURCC('M','J','P','G'), fps, S);
//    mVideoWriter.open(file, CV_FOURCC('I','Y','U','V'), fps, S);
}

void VideoCoder::setNextFrame(QImage img)
{
    QMutexLocker lock(&mMutex);
    if (mNextFrame.size() < 200) {
        mNextFrame.append(img);
    } else {
        mNextFrame[mNextFrame.size() - 1] = img;
        qWarning() << "Dropping frame to compress";
    }
    mFrameReady = true;
}

void VideoCoder::stopAndWait()
{
    mAbort = true;
    wait();

    if(mVideoWriter.isOpened())
    {
        mVideoWriter.release();
    }
}

void VideoCoder::run()
{
    while (!mAbort || mFrameReady) {
        while (!mFrameReady) {
            if (mAbort) {
                return;
            }

            QThread::msleep(1);
        }

        QImage img;

        {
            QMutexLocker lock(&mMutex);
            img = mNextFrame[0];
        }

        //qDebug() << "Frames to compress:" << mNextFrame.size();

        mVideoWriter << QImageToMat(img);

        {
            QMutexLocker lock(&mMutex);
            mNextFrame.removeFirst();
            if (mNextFrame.isEmpty()) {
                mFrameReady = false;
            }
        }
    }
}
