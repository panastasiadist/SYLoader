/*******************************************************************************
 * Copyright 2016 Panagiotis Anastasiadis
 * This file is part of SYLoader.
 *
 * SYLoader is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * SYLoader is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SYLoader. If not, see http://www.gnu.org/licenses.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you
 * do not wish to do so, delete this exception statement from your
 * version. If you delete this exception statement from all source
 * files in the program, then also delete it here.
 ******************************************************************************/
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
    bool hasDownload(QString downloadSignature);
    void setSavepath(QString path);
    void setAutoProcessing(bool enabled);
    void setConcurrentDownloads(int num);


private:
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
