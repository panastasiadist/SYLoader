/*******************************************************************************
 * Copyright 2015 Panagiotis Anastasiadis
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

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "download.h"
#include "task_processor.h"
#include "downloader_progress.h"
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QElapsedTimer>
#include <QProcess>
#include <QTemporaryFile>

class Downloader : public QObject
{
    Q_OBJECT


public:
    enum Status
    {
        Ready,
        Downloading,
        Converting,
        Complete,
        Canceled,
        ErrorIO,
        ErrorConnection
    };

    Downloader(Download download, QString savePath);
    ~Downloader();

    void start();
    void stop();
    void reset();
    void report();

    Status getStatus();
    Download *getDownload();
    DownloaderProgress getProgress();


private:
    QString _savePath;
    QNetworkAccessManager _gateway;
    QNetworkReply *_videoNetworkReply;
    QNetworkReply *_soundNetworkReply;
    QTemporaryFile *_soundFile;
    QTemporaryFile *_videoFile;

    QElapsedTimer _speedElapsedTimer;
    Download _download;
    Status _status;
    qint64 _bytesDownloaded;
    qint64 _lastBytesDownloaded;
    qint64 _bytesTotal;
    qint64 _videoBytes;
    qint64 _soundBytes;
    qint64 _videoBytesReceived;
    qint64 _soundBytesReceived;
    qint64 _eta;
    qint64 _kbps;
    qint64 _percent;
    int _convertPid;
    int _retryCount;
    bool _cancelationPending;

    QString getOutputPath(QString title, QString extension);
    QString getConversionExtension();

    void download();
    bool redirect(QNetworkReply *reply);
    bool isVideoMode();
    bool isVideoValid();
    void setStatus(Status status);
    void setProgress(qint64 eta, qint64 speed, qint64 percent);

signals:
    void statusChanged();
    void progressChanged();

private slots:
    void onDownloadSslErrors(const QList<QSslError> errors);
    void onTimerTimeout();
    void onDownloadFinished();
    void onDownloadReadyRead();
    void onTaskStatusChanged(TaskProcessor::Status status, int pid, int exitCode);
    void onDownloadProgressChanged(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // DOWNLOADER_H
