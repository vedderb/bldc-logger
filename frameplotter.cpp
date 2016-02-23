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

#include "frameplotter.h"
#include <QDebug>

FramePlotter::FramePlotter(QObject *parent) :
    QThread(parent)
{
    mAbort = false;
    mFrameReady = false;

    QFont plotFont;
    plotFont.setPointSize(8);

    int graphIndex = 0;
    mCurrentPlot.addGraph();
    mCurrentPlot.graph(graphIndex)->setPen(QPen(Qt::blue));
    mCurrentPlot.graph(graphIndex)->setName("Battery current");
    graphIndex++;

    mCurrentPlot.addGraph();
    mCurrentPlot.graph(graphIndex)->setPen(QPen(Qt::red));
    mCurrentPlot.graph(graphIndex)->setName("Motor current");
    graphIndex++;

    mCurrentPlot.addGraph(mCurrentPlot.xAxis, mCurrentPlot.yAxis2);
    mCurrentPlot.graph(graphIndex)->setPen(QPen(Qt::green));
    mCurrentPlot.graph(graphIndex)->setName("Duty cycle");
    graphIndex++;

    mCurrentPlot.legend->setVisible(true);
    mCurrentPlot.legend->setFont(plotFont);
    mCurrentPlot.axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    mCurrentPlot.legend->setBrush(QBrush(QColor(255,255,255,230)));
    mCurrentPlot.xAxis->setLabel("Seconds (s)");
    mCurrentPlot.yAxis->setLabel("Current (A)");
    mCurrentPlot.yAxis2->setLabel("Duty Cycle");

    mCurrentPlot.yAxis->setRange(-20, 130);
    mCurrentPlot.yAxis2->setRange(-0.2, 1.3);
    mCurrentPlot.yAxis2->setVisible(true);

    mCurrentPlot.xAxis->setLabelFont(plotFont);
    mCurrentPlot.yAxis->setLabelFont(plotFont);
    mCurrentPlot.yAxis2->setLabelFont(plotFont);

    mCurrentPlot.xAxis->setTickLabelFont(plotFont);
    mCurrentPlot.yAxis->setTickLabelFont(plotFont);
    mCurrentPlot.yAxis2->setTickLabelFont(plotFont);
}

void FramePlotter::setNextValues(FrameData val)
{
    {
        QMutexLocker lock(&mMutex);
        if (mNextValues.size() < 200) {
            mNextValues.append(val);
        } else {
            mNextValues[mNextValues.size() - 1] = val;
            qWarning() << "Dropping frame to plot";
        }
        mFrameReady = true;
    }
}

void FramePlotter::stopAndWait()
{
    mAbort = true;
    wait();
}

void FramePlotter::run()
{
    while (!mAbort || mFrameReady) {
        while (!mFrameReady) {
            if (mAbort) {
                return;
            }

            QThread::msleep(1);
        }

        FrameData data;

        {
            QMutexLocker lock(&mMutex);
            data = mNextValues[0];
        }

        //qDebug() << "Frames to plot:" << mNextValues.size();

        MC_VALUES values = data.values;
        int vidw = data.img.width();
        int vidh = data.img.height();

        QImage *img = &data.img;
        QPainter painter(img);
        QFont font;
        QPen pen;
        QString str;

        const double fs = 16;
        const double box1_w = 450;
        const double box2_w = 450;
        const double box_h = 180;
        const double plot_w = 500;
        const double plot_h = 250;

        const double wheel_diam = 0.096;
        const double motor_poles = 4.0;
//        const double gear_ratio = 40.0 / 15.0;
        const double gear_ratio = 4.75;


        // Upper left info box
        str.sprintf("Power           : %.1f W\n"
                    "Duty cycle      : %.2f %%\n"
                    "Speed           : %.1f Km/h\n"
                    "Battery current : %.2f A\n"
                    "Motor current   : %.2f A\n"
                    "MOSFET temp     : %.2f Deg C\n"
                    "Fault code      : %s",
                    values.v_in * values.current_in,
                    values.duty_now * 100.0,
                    wheel_diam * M_PI * (values.rpm / (motor_poles / 2.0) / gear_ratio / 60.0) * 3.6,
                    values.current_in,
                    values.current_motor,
                    values.temp_mos1,
                    data.values.fault_str.remove(0, 11).toLocal8Bit().data());

        painter.setOpacity(0.5);
        painter.fillRect(0, 0, box1_w, box_h, Qt::black);
        painter.setOpacity(1.0);

        pen.setColor(Qt::white);
        painter.setPen(pen);
        font.setPointSize(fs);
        font.setFamily("Monospace");
        painter.setFont(font);
        painter.drawText(QRectF(5, 5, box1_w, box_h), Qt::AlignLeft, str);

        // Upper right info box
        str.sprintf("Drawn cap       : %.1f mAh\n"
                    "Charged cap     : %.1f mAh\n"
                    "Drawn energy    : %.2f Wh\n"
                    "Charged energy  : %.2f Wh\n"
                    "Distance        : %.1f m\n"
                    "RPM             : %.1f RPM\n"
                    "Battery voltage : %.1f V",
                    values.amp_hours * 1000.0,
                    values.amp_hours_charged * 1000.0,
                    values.watt_hours,
                    values.watt_hours_charged,
                    values.tachometer_abs * wheel_diam * M_PI / gear_ratio / motor_poles / 3.0,
                    values.rpm,
                    values.v_in);

        painter.setOpacity(0.5);
        painter.fillRect(vidw - box2_w, 0, box2_w, box_h, Qt::black);
        painter.setOpacity(1.0);

        pen.setColor(Qt::white);
        painter.setPen(pen);
        font.setPointSize(fs);
        font.setFamily("Monospace");
        painter.setFont(font);
        painter.drawText(QRectF(vidw -box2_w + 5, 5, box2_w, box_h), Qt::AlignLeft, str);

        const int maxS = 500;

        appendDoubleAndTrunc(&tempPcbVec, values.temp_pcb, maxS);
        appendDoubleAndTrunc(&currInVec, values.current_in, maxS);
        appendDoubleAndTrunc(&currMotorVec, values.current_motor, maxS);
        appendDoubleAndTrunc(&dutyVec, values.duty_now, maxS);
        appendDoubleAndTrunc(&rpmVec, values.rpm, maxS);

        int dataSize = tempPcbVec.size();
        float fsamp = 10.0;

        QVector<double> xAxis(dataSize);
        for (int i = 0;i < tempPcbVec.size();i++) {
            xAxis[i] = (double)i / fsamp;
        }

        int graphIndex = 0;

        // Current and duty-plot
        graphIndex = 0;
        mCurrentPlot.graph(graphIndex++)->setData(xAxis, currInVec);
        mCurrentPlot.graph(graphIndex++)->setData(xAxis, currMotorVec);
        mCurrentPlot.graph(graphIndex++)->setData(xAxis, dutyVec);

        mCurrentPlot.rescaleAxes();

        if (dataSize < maxS) {
            mCurrentPlot.xAxis->setRange(0, dataSize / fsamp);
        }

        painter.setOpacity(0.7);
        painter.drawPixmap(0, vidh - plot_h, mCurrentPlot.toPixmap(plot_w, plot_h));

        emit frameReady(*img);

        {
            QMutexLocker lock(&mMutex);
            mNextValues.removeFirst();
            if (mNextValues.isEmpty()) {
                mFrameReady = false;
            }
        }
    }
}

void FramePlotter::appendDoubleAndTrunc(QVector<double> *vec, double num, int maxSize)
{
    vec->append(num);

    if(vec->size() > maxSize) {
        vec->remove(0, vec->size() - maxSize);
    }
}
