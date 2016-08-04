#include "queue_processor.h"



QueueProcessor::QueueProcessor()
{
    _lastDownloaderId = 0;
    _numConcurrentDownloads = 0;
    _autoProcessing = false;
}



QueueProcessor::~QueueProcessor()
{
    clear();
}



void
QueueProcessor::setSavepath(QString path)
{
    _savePath = path;
}



void
QueueProcessor::setConcurrentDownloads(int num)
{
    _numConcurrentDownloads = num;
}



void
QueueProcessor::setAutoProcessing(bool enabled)
{
    _autoProcessing = enabled;
}



int
QueueProcessor::enqueue(Download download)
{
    // Create a new downloader for the passed in download, store it and return
    // its unique id.
    Downloader *downloader = new Downloader(download, _savePath);

    connect(downloader,
            SIGNAL(statusChanged()),
            this,
            SLOT(onDownloaderStatusChanged()));

    connect(downloader,
            SIGNAL(progressChanged()),
            this,
            SLOT(onDownloaderProgressChanged()));


    _lastDownloaderId++;
    _queue.append(_lastDownloaderId);
    _idToDownloader.insert(_lastDownloaderId, downloader);

    return _lastDownloaderId;
}



bool
QueueProcessor::remove(int id)
{
    Downloader *downloader = _idToDownloader.value(id);

    if (downloader != NULL)
    {
        downloader->deleteLater();

        _queue.removeOne(id);
        _idToDownloader.remove(id);

        return true;
    }
    else
    {
        return false;
    }
}


bool
QueueProcessor::clear()
{
    foreach (Downloader *downloader, _idToDownloader.values())
    {
        downloader->deleteLater();
    }

    _queue.empty();
    _idToDownloader.empty();

    return true;
}



bool
QueueProcessor::start(int id)
{
    Downloader *downloader = _idToDownloader.value(id);

    if (downloader != NULL)
    {
        Downloader::Status status = downloader->getStatus();

        if (status == Downloader::Canceled ||
            status == Downloader::ErrorIO ||
            status == Downloader::ErrorConnection)
        {
            downloader->reset();
        }

        if (status != Downloader::Complete &&
            status != Downloader::Converting &&
            status != Downloader::Downloading)
        {
            downloader->start();
            return true;
        }
    }

    return false;
}



bool
QueueProcessor::stop(int id)
{
    Downloader *downloader = _idToDownloader.value(id);

    if (downloader != NULL)
    {
        Downloader::Status status = downloader->getStatus();

        if (status == Downloader::Downloading ||
            status == Downloader::Converting)
        {
            downloader->stop();
            return true;
        }
    }

    return false;
}



bool
QueueProcessor::stopAll()
{
    foreach (Downloader *downloader, _idToDownloader.values())
    {
        Downloader::Status status = downloader->getStatus();

        if (status == Downloader::Downloading ||
            status == Downloader::Converting)
        {
            downloader->stop();
        }
    }

    return true;
}



bool
QueueProcessor::running()
{
    DownloaderStats stats = getStats();
    return stats.downloading + stats.converting > 0;
}



DownloaderStats
QueueProcessor::getStats()
{
    DownloaderStats stats;
    stats.canceled = 0;
    stats.completed = 0;
    stats.converting = 0;
    stats.downloading = 0;
    stats.errored = 0;
    stats.ready = 0;

    foreach (Downloader *downloader, _idToDownloader.values())
    {
        Downloader::Status status = downloader->getStatus();
        switch (status)
        {
            case Downloader::Ready:
                stats.ready++;
                break;
            case Downloader::Downloading:
                stats.downloading++;
                break;
            case Downloader::Converting:
                stats.converting++;
                break;
            case Downloader::Complete:
                stats.completed++;
                break;
            case Downloader::Canceled:
                stats.canceled++;
                break;
            case Downloader::ErrorConnection:
            case Downloader::ErrorIO:
                stats.errored++;
        }
    }

    return stats;
}



Downloader*
QueueProcessor::getDownloader(int id)
{
    return _idToDownloader.value(id);
}



QList<Downloader*>
QueueProcessor::getDownloaders()
{
    return _idToDownloader.values();
}



int
QueueProcessor::process()
{
    DownloaderStats stats = getStats();

    int downloading = stats.downloading;
    int ready = stats.ready;
    int started = 0;

    // No processors currently run, now starting.
    if (ready == _idToDownloader.keys().length()) {
        emit downloadsStarted();
    }

    // We will start a new download as soon as a processor has finished
    // downloading taking into account the maximum number of concurrent
    // downloads the user has set.
    if (ready > 0)
    {
        int availableSlots = _numConcurrentDownloads - downloading;
        int newDownloadSlots = 0;

        // It may be less than zero if the user has decreased concurrent
        // downloads while downloading has started.
        if (availableSlots <= 0) {
            return 0;
        }

        if (availableSlots >= ready) {
            newDownloadSlots = ready;
        } else {
            newDownloadSlots = availableSlots;
        }

        started = newDownloadSlots;

        foreach (Downloader *processor, _idToDownloader.values())
        {
            if (processor->getStatus() == Downloader::Ready)
            {
                processor->start();
                newDownloadSlots--;
            }

            if (newDownloadSlots == 0) {
                break;
            }
        }
    }

    return started;
}



void
QueueProcessor::onDownloaderStatusChanged()
{
    Downloader *downloader = qobject_cast<Downloader*>(QObject::sender());
    Downloader::Status status = downloader->getStatus();

    int id = _idToDownloader.key(downloader);

    emit downloadStatusChanged(id, status);

    if (status == Downloader::ErrorConnection ||
        status == Downloader::ErrorIO ||
        status == Downloader::Complete ||
        status == Downloader::Converting ||
        status == Downloader::Canceled)
    {
        //DownloaderStats stats = getStats();
        //int run = stats.canceled + stats.completed + stats.errored;

        if (!running())
        {
            emit downloadsFinished();
        }
        else
        {
            // Autoprocessing enables automatic start of the next download in
            // the queue when the current one is finished.
            if (_autoProcessing) {
                process();
            }
        }
    }
}



void
QueueProcessor::onDownloaderProgressChanged()
{
    Downloader *downloader = qobject_cast<Downloader*>(QObject::sender());
    DownloaderProgress progress = downloader->getProgress();

    int id = _idToDownloader.key(downloader);

    emit downloadProgressChanged(id, progress);
}

