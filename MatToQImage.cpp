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

#include "MatToQImage.h"

// Found here: http://asmaloney.com/2013/11/code/converting-between-cvmat-and-qimage-or-qpixmap/
QImage MatToQImage(const Mat& inMat)
{
    switch (inMat.type()) {
    case CV_8UC4: // 8-bit, 4 channel
    {
        QImage image(inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB32);
        return image;
    }

    // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image(inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }

    // 8-bit, 1 channel
    case CV_8UC1:
    {
        static QVector<QRgb>  sColorTable;

        // only create our color table once
        if (sColorTable.isEmpty()) {
            for ( int i = 0; i < 256; ++i ) {
                sColorTable.push_back( qRgb( i, i, i ) );
            }
        }

        QImage image(inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8);
        image.setColorTable(sColorTable);

        return image;
    }

    default:
        qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
        break;
    }

    return QImage();
}

QPixmap MatToQPixmap(const Mat &inMat)
{
    return QPixmap::fromImage(MatToQImage(inMat));
}

Mat QImageToMat(const QImage &inImage, bool inCloneImageData)
{
    switch (inImage.format())
    {
    // 8-bit, 4 channel
    case QImage::Format_RGB32:
    {
        cv::Mat  mat(inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine());
        return (inCloneImageData ? mat.clone() : mat);
    }

    // 8-bit, 3 channel
    case QImage::Format_RGB888:
    {
        if (!inCloneImageData) {
            qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";
        }
        QImage   swapped = inImage.rgbSwapped();
        return cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();
    }

    // 8-bit, 1 channel
    case QImage::Format_Indexed8:
    {
        cv::Mat  mat(inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine());
        return (inCloneImageData ? mat.clone() : mat);
    }

    default:
        qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
        break;
    }

    return cv::Mat();
}


Mat QPixmapToMat(const QPixmap &inPixmap, bool inCloneImageData)
{
    return QImageToMat(inPixmap.toImage(), inCloneImageData);
}
