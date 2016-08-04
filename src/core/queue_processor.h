#ifndef QUEUE_PROCESSOR_H
#define QUEUE_PROCESSOR_H

#include <QObject>
#include "download.h"
#include "downloader.h"
#include "downloader_stats.h"
#include "downloader_progress.h"

class QueueProcessor : public QObject
{
    Q_OBJECT

public:
    QueueProcessor();
    ~QueueProcessor();
    DownloaderStats getStats();
    Downloader *getDownloader(int id);
    QList<Downloader*> getDownloaders();

    int process();
    int enqueue(Download download);
    bool remove(int id);
    bool clear();
    bool start(int id);
    bool stop(int id);
    bool stopAll();
    bool running();
    void setSavepath(QString path);
    void setAutoProcessing(bool enabled);
    void setConcurrentDownloads(int num);


private:
    QQueue<int> _queue;
    QMap<int, Downloader*> _idToDownloader;
    QString _savePath;

    int _lastDownloaderId;
    int _numConcurrentDownloads;
    bool _autoProcessing;


signals:
    void downloadStatusChanged(int id, Downloader::Status status);
    void downloadProgressChanged(int id, DownloaderProgress progress);
    void downloadsFinished();
    void downloadsStarted();


private slots:
    void onDownloaderStatusChanged();
    void onDownloaderProgressChanged();
};

#endif // QUEUE_PROCESSOR_H
