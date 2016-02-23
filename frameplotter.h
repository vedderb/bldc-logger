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

#ifndef FRAMEPLOTTER_H
#define FRAMEPLOTTER_H

#include <QThread>
#include <QList>
#include <qmutex.h>
#include "packetinterface.h"
#include "qcustomplot.h"
#include "opencv/cv.h"
#include "MatToQImage.h"

class FramePlotter : public QThread
{
    Q_OBJECT
public:
    typedef struct {
        QImage img;
        MC_VALUES values;
    } FrameData;

    explicit FramePlotter(QObject *parent = 0);
    void setNextValues(FrameData val);
    void stopAndWait();

signals:
    void frameReady(QImage frame);

public slots:


protected:
    void run();

private:
    volatile bool mAbort;
    volatile bool mFrameReady;
    QList<FrameData> mNextValues;
    QMutex mMutex;
    QCustomPlot mCurrentPlot;
    QVector<double> tempPcbVec;
    QVector<double> currInVec;
    QVector<double> currMotorVec;
    QVector<double> dutyVec;
    QVector<double> rpmVec;
    QVector<double> voltInVec;

    void appendDoubleAndTrunc(QVector<double> *vec, double num, int maxSize);

};

#endif // FRAMEPLOTTER_H
