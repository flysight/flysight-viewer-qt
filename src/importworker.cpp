/***************************************************************************
**                                                                        **
**  FlySight Viewer                                                       **
**  Copyright 2018 Michael Cooper                                         **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>. **
**                                                                        **
****************************************************************************
**  Contact: Michael Cooper                                               **
**  Website: http://flysight.ca/                                          **
****************************************************************************/

#include <QProcess>
#include <QSharedMemory>
#include <QSystemSemaphore>

#include "importworker.h"

ImportWorker::ImportWorker()
{
    // Initialize here
}

void ImportWorker::process()
{
    QSharedMemory shared("FlySight_Viewer_Import_Shared");
    QSystemSemaphore free("FlySight_Viewer_Import_Free", 1, QSystemSemaphore::Open);
    QSystemSemaphore used("FlySight_Viewer_Import_Used", 0, QSystemSemaphore::Open);

    shared.create(1000);
    shared.attach();

    while (1)
    {
        used.acquire();

        shared.lock();
        QByteArray bytes((const char *) shared.constData(), shared.size());
        emit importFile(QString::fromUtf8(bytes));
        shared.unlock();

        free.release();
    }
}
