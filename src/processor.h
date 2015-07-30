/*******************************************************************************
 * Copyright 2015 Panagiotis Anastasiadis
 * This file is part of eTube Downloader.
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
 * along with SYLoader.  If not, see http://www.gnu.org/licenses.
 ******************************************************************************/

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "download.h"
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QElapsedTimer>
#include <QProcess>

class Processor : public QObject
{
    Q_OBJECT


public:
    enum Status {
        Ready,
        Downloading,
        Converting,
        Complete,
        Canceled,
        ErrorIO,
        ErrorConnection,
    };



    Processor(const Download download, const QString savePath);

    void start();
    void stop();
    void reset();

    Download *getDownload();
    Status getStatus();


private:
    QString _savePath;
    QNetworkAccessManager _videoNetworkManager;
    QNetworkAccessManager _soundNetworkManager;
    QNetworkReply *_videoNetworkReply;
    QNetworkReply *_soundNetworkReply;

    QElapsedTimer _speedElapsedTimer;
    QProcess _convertProcess;
    Download _download;
    Status _status;
    qint64 _bytesDownloaded;
    qint64 _lastBytesDownloaded;
    qint64 _bytesTotal;

    qint64 _videoBytes;
    qint64 _soundBytes;
    qint64 _videoBytesReceived;
    qint64 _soundBytesReceived;


    bool _cancelationPending;

    QString getSaveFilepath(const QString &title, const QString &extension);
    QString getConversionExtension();
    void setDisplay(Status status, qint64 eta, qint64 speed, qint64 progress);

    void download();

    bool isVideoMode();

signals:
    void statusChanged();

private slots:
    void onDownloadNetworkManagerFinished(QNetworkReply *networkReply);
    void onConvertCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void onDownloadProgressChanged(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // PROCESSOR_H
