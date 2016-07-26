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
