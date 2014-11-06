bldc-logger
===========

A file and video-overlay logger for my BLDC motor controller that uses opencv for capturing and compressing video and Qt to overlay the frames with data. Four threads are used:

1. Capture images from the webcam (framegrabber.cpp).
2. Overlay the images with real-time information (frameplotter.cpp).
3. Encode a x264 video from the images (videocoder.cpp).
4. Record audio, fetch data from the ESC and keep everything in sync (logger.cpp).

Note that even though this is a console application, an X server has to be running for the video overlay to work. This is because qcustomplot and some qpainter operations require that.

Usage on ubuntu:
================
1. Install dependencies:  
sudo apt-get install qt-sdk libqt5multimedia5 libqt5multimedia5-plugins qtmultimedia5-dev

2. Install a recent version of opencv:  
https://help.ubuntu.com/community/OpenCV

3. Compile  
qmake  
make

4. Test it with a motor controller and a webcam connected  
./BLDC_Logger

If it deosn't work, open logger.cpp and make sure that the correct /dev/ttyACMX port is chosen. Also, make sure that the correct camera and resolution is selected.

