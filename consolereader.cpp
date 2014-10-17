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

#include "consolereader.h"
#include <QTextStream>
#include <unistd.h> // Provides STDIN_FILENO

ConsoleReader::ConsoleReader(QObject *parent) :
    QObject(parent)
{
    notifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(text()));
}

void ConsoleReader::text()
{
    QTextStream qin(stdin);
    QString line = qin.readLine();
    emit textReceived(line);
}
