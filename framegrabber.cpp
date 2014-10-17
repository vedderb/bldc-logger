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

#include "framegrabber.h"
#include <QDebug>

FrameGrabber::FrameGrabber(int w, int h, int fps, int dev, QObject *parent) :
    QThread(parent)
{
    mAbort = false;

    mVideoCap.open(dev);

    mVideoCap.set(CV_CAP_PROP_FRAME_WIDTH, w);
    mVideoCap.set(CV_CAP_PROP_FRAME_HEIGHT, h);
    mVideoCap.set(CV_CAP_PROP_FPS, fps);

    mVideoCap >> mFrameMat;
    mLatestFrame = MatToQImage(mFrameMat);
}

QImage FrameGrabber::getLatestFrame()
{
    QMutexLocker lock(&mMutex);
    return mLatestFrame;
}

bool FrameGrabber::isOpened()
{
    return mVideoCap.isOpened();
}

void FrameGrabber::stopAndWait()
{
    mAbort = true;
    wait();

    if(mVideoCap.isOpened())
    {
        mVideoCap.release();
    }
}

void FrameGrabber::run()
{
    while (!mAbort) {
        // Seems to wait based on the FPS of the camera.
        mVideoCap >> mFrameMat;

        {
            QMutexLocker lock(&mMutex);
            mLatestFrame = MatToQImage(mFrameMat);
        }
    }
}
