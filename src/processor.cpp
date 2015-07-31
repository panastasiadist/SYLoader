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
 * along with SYLoader.  If not, see http://www.gnu.org/licenses.
 ******************************************************************************/

#include "processor.h"
#include "global.h"
#include "utility.h"
#include <math.h>
#include <QDir>
#include <QTemporaryFile>
#include <QtWidgets/QMessageBox>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>


#define MAX_RETRIES 3
#define RETRY_INTERVAL 10000


Processor::Processor(const Download d, const QString savePath)
{
    // Initialize some variables
    _savePath = savePath;
    _download = d;
    _retryCount = 0;
    _soundNetworkReply = NULL;
    _videoNetworkReply = NULL;
    _soundFile = NULL;
    _videoFile = NULL;



    connect(Tasks,
            SIGNAL(statusChanged(Scheduler::Status,int,int)),
            this,
            SLOT(onStatusChanged(Scheduler::Status,int,int)));




    reset();


    if (isVideoValid())
    {
        qDebug() << QString("Processing %1. Artist: %2. Coartist: %3. Title: %4")
                    .arg(d.videoTitle, d.artist, d.coartist, d.title);
    }
    else
    {
        qDebug() << QString("Processor stopped on an A/S Error");
        d.statusItem->setText(tr("A/S Error"));
    }
}





Processor::~Processor()
{
    Tasks->disconnect(this);
}





void
Processor::start()
{
    // The video's information is not valid. We can't start.
    if (!isVideoValid()) {
        return;
    }

    reset();
    download();
}





void
Processor::stop()
{
    /* The video's information is not valid.
     * This processor never started.
     */
    if (!isVideoValid()) {
        return;
    }


    // Check if a cancelation has been already issued.
    if (_cancelationPending)
        return;

    _cancelationPending = true;


    if (_status == Downloading)
    {
        /* Call the downloading managers to abort downloading.
         * statusChanged() will be emitted by the managers.
         */
        _soundNetworkReply->abort();

        /* The video download manager has been activated to download the video
         * stream of a video only if video mode is enabled.
         */
        if (isVideoMode())
            _videoNetworkReply->abort();

        _status = Canceled;
    }
    else if (_status == Converting)
    {
        /* Call the converting process to terminate.
         * Because we have set cancelation pending, the termination process
         * will not raise an IO Error.
         * statusChanged() will be emitted by the convertProcess SLOT.
         */
        //_convertProcess.terminate();
        Tasks->abort(_convertPid);

        _status = Canceled;
    }
    else if (_status == Ready)
    {
        // If the processor hasn't run yet, just mark it as canceled
        _status = Canceled;
        //setDisplay(Canceled, 0, 0, 0);
        //emit statusChanged();
    }

    setDisplay(Canceled, 0, 0, 0);
    emit statusChanged();

}





void
Processor::reset()
{
    _status = Ready;
    _videoBytes = 0;
    _soundBytes = 0;
    _bytesTotal = 0;
    _bytesDownloaded = 0;
    _videoBytesReceived = 0;
    _soundBytesReceived = 0;
    _lastBytesDownloaded = 0;
    _cancelationPending = false;

    setDisplay(Ready, 0, 0, 0);
}





Download*
Processor::getDownload()
{
    return &_download;
}





Processor::Status
Processor::getStatus()
{
    return _status;
}





void
Processor::onDownloadFinished()
{
    bool finished = false;
    QNetworkReply::NetworkError videoError;
    QNetworkReply::NetworkError soundError = _soundNetworkReply->error();


    if (isVideoMode())
        videoError = _videoNetworkReply->error();
    else
        videoError = QNetworkReply::NoError;


    if (isVideoMode())
    {
        bool videoFinished = _videoNetworkReply->isFinished();
        bool soundFinished = _soundNetworkReply->isFinished();

        finished = videoFinished && soundFinished;
    } else {
        finished = _soundNetworkReply->isFinished();
    }


    if (finished)
    {
        if (soundError == QNetworkReply::OperationCanceledError && !isVideoMode())
            videoError = QNetworkReply::OperationCanceledError;

        if (videoError == QNetworkReply::OperationCanceledError &&
            soundError == QNetworkReply::OperationCanceledError)
        {
            // User has canceled downloading
            //_status = Canceled;
            //setDisplay(Canceled, 0, 0, 0);
            //emit statusChanged();
            goto Cleanup;
        }
        else if (videoError || soundError)
        {
            // Some kind of error occurred on both downloaders. Examine it.
            goto ErrorProcedure;
        }
        else
        {
            if (_soundBytesReceived == 0)
            {
                qDebug() << QString("Invalid sound data from %1")
                            .arg(_soundNetworkReply->url().toString());
                goto ErrorProcedure;
            }

            if (isVideoMode())
            {
                if (_videoBytesReceived == 0)
                {
                    qDebug() << QString("Invalid video data from %1")
                                .arg(_videoNetworkReply->url().toString());
                    goto ErrorProcedure;
                }
            }

            // The video and music streams of the video have been downloaded.
            /*QTemporaryFile soundFile;
            QTemporaryFile videoFile;


            _soundNetworkReply->open(QIODevice::ReadOnly);
            QByteArray soundData = _soundNetworkReply->readAll();
            QByteArray videoData;


            if (soundData.length() == 0)
            {
                qDebug() << QString("Invalid sound data from %1")
                            .arg(_soundNetworkReply->url().toString());
                goto ErrorProcedure;
            }


            if (isVideoMode())
            {
                _videoNetworkReply->open(QIODevice::ReadOnly);
                videoData = _videoNetworkReply->readAll();
                if (videoData.length() == 0)
                {
                    qDebug() << QString("Invalid video data from %1")
                                .arg(_videoNetworkReply->url().toString());
                    goto ErrorProcedure;
                }
            }





            if (soundFile.open())
            {
                soundFile.write(soundData);
                soundFile.close();
                soundFile.setAutoRemove(false);
            }


            if (isVideoMode())
            {
                if (videoFile.open())
                {
                    videoFile.write(videoData);
                    videoFile.close();
                    videoFile.setAutoRemove(false);
                }
            }*/

            QString iargs = " -id3v2_version 3 -write_id3v1 1 ";
            QString filename = "";

            bool hasArtist = !_download.artist.isEmpty();
            bool hasTitle = !_download.title.isEmpty();
            bool hasCoartist = !_download.coartist.isEmpty();

            if (hasArtist && hasTitle)
            {
                filename += _download.artist;

                if (hasCoartist)
                    filename += " ft. " + _download.coartist;

                /* The filename so far contains the artist's name
                 * Write it using ID3 tags.
                 */
                iargs += " -metadata artist=\""+filename+"\" ";

                filename += " - " + _download.title;
                iargs += " -metadata title=\""+_download.title+"\" ";
            } else {
                filename = _download.videoTitle;
            }

            QString command;
            if (isVideoMode())
            {
                QString extension = _download.videoExtension;
                QString savePath = getSaveFilepath(filename, extension);
                QString vfilename = _videoFile->fileName();
                QString sfilename = _soundFile->fileName();
                _videoFile->close();
                _soundFile->close();
                QString args = "%1 -i \"%2\" -i \"%3\" -acodec copy -vcodec copy %4 \"%5\"";
                command = QString(args)
                            .arg(FFMPEG_PATH, vfilename, sfilename, iargs, savePath);
            }
            else
            {
                QString extension = _download.convertExtension;
                QString savePath = getSaveFilepath(filename, extension);
                QString sfilename = _soundFile->fileName();
                _soundFile->close();
                command = QString("%1 -y -i \"%2\" %3 \"%4\"")
                            .arg(FFMPEG_PATH, sfilename, iargs, savePath);
            }

            _convertPid = Tasks->enqueue(command);

            _status = Converting;
            setDisplay(Converting, 0, 0, 100);
            emit statusChanged();
            goto Cleanup;
        }
    }
    else
    {
        /* One of the two downloaders (video/sound) has finished.
         * This may be normal. A common scenario is that sound has been
         * downloaded because of smaller size but video is still pending.
         * But maybe a downloader has finished because of error.
         * Check and take action accordingly.
         */
        if (videoError || soundError) {
            goto ErrorProcedure;
        }
    }

    return;


ErrorProcedure:


    // If at least one downloader has errored, stop the other.
    if (videoError)
        _soundNetworkReply->abort();

    if (soundError && isVideoMode())
        _videoNetworkReply->abort();


    if (_retryCount < MAX_RETRIES)
    {
        _retryCount++;

        qDebug() << QString("Processor had an error connection. Retrying for %1 time in %2 ms")
                    .arg(QString::number(_retryCount), QString::number(RETRY_INTERVAL));

        QTimer *timer = new QTimer();
        timer->setSingleShot(true);
        timer->start(RETRY_INTERVAL);
        connect(timer,
                SIGNAL(timeout()),
                this,
                SLOT(onTimerTimeout()));

        return;
    }
    else
    {
        qDebug() << QString("Processor had an error connection. Max retries of %1 reached")
                    .arg(MAX_RETRIES);

        _status = ErrorConnection;
        setDisplay(ErrorConnection, 0, 0, 0);
        emit statusChanged();
        goto Cleanup;
    }





Cleanup:
    disconnect(_soundNetworkReply);
    if (_soundNetworkReply->isOpen()) {
        _soundNetworkReply->close();
    }
    _soundNetworkReply->deleteLater();
    _soundNetworkReply = NULL;

    if (_soundFile->isOpen())
        _soundFile->close();
    delete _soundFile;
    _soundFile = NULL;

    if (isVideoMode()) {
        disconnect(_videoNetworkReply);
        if (_videoNetworkReply->isOpen()) {
            _videoNetworkReply->close();
        }
        _videoNetworkReply->deleteLater();
        _videoNetworkReply = NULL;

        if (_videoFile->isOpen())
            _videoFile->close();
        delete _videoFile;
        _videoFile = NULL;
    }

}






void
Processor::onStatusChanged(Scheduler::Status status, int pid, int exitCode)
{
    if (pid != _convertPid)
        return;


    if (status == Scheduler::Started)
    {
        _status = Converting;
        setDisplay(Converting, 0, 0, 100);

        qDebug() << "Converter started";
    }
    else if (status == Scheduler::Finished)
    {
        if (_cancelationPending) {
            return;
        }

        if (exitCode == 0)
        {
            _status = Complete;
        }
        else
        {
            qDebug() << QString("Converter errored: %1").arg(exitCode);
            _status = ErrorIO;
        }

        /*
        if (_cancelationPending)
        {
            _status = Canceled;
            setDisplay(Canceled, 0, 0, 100);
        }
        else
        {
            if (exitCode == 0)
            {
                _status = Complete;
                setDisplay(Complete, 0, 0, 100);
            }
            else
            {
                _status = ErrorIO;
                setDisplay(ErrorIO, 0, 0, 100);

                qDebug() << QString("Converter errored: %1").arg(exitCode);
            }
        }*/
    }

    setDisplay(_status, 0, 0, 100);
    emit statusChanged();
}





void
Processor::onDownloadProgressChanged(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());

    // May be 0, on a connection error or glitch;
    if (bytesTotal == 0)
        return;

    if (reply == _videoNetworkReply)
    {
        if (_videoBytes == 0)
        {
            _videoBytes = bytesTotal;
            _bytesTotal += bytesTotal;
        }

        _videoBytesReceived = bytesReceived;
    }


    if (reply == _soundNetworkReply)
    {
        if (_soundBytes == 0)
        {
            _soundBytes = bytesTotal;
            _bytesTotal += bytesTotal;
        }

        _soundBytesReceived = bytesReceived;
    }


    if (_speedElapsedTimer.elapsed() > 1000)
    {
        _bytesDownloaded = _videoBytesReceived + _soundBytesReceived;

        qint64 progress = _bytesDownloaded * 100 / _bytesTotal;
        qint64 bytesDiff = _bytesDownloaded - _lastBytesDownloaded;
        qint64 speed = bytesDiff / _speedElapsedTimer.elapsed();
        if (speed == 0) return;

        _lastBytesDownloaded = _bytesDownloaded;
        _speedElapsedTimer.restart();

        qint64 seconds = ((_bytesTotal - _bytesDownloaded) / 1024) / speed;

        setDisplay(Downloading, seconds, speed, progress);
    }

}





void
Processor::onDownloadReadyRead()
{
    QByteArray soundData = _soundNetworkReply->readAll();
    _soundFile->write(soundData);

    if (isVideoMode())
    {
        QByteArray videoData = _videoNetworkReply->readAll();
        _videoFile->write(videoData);
    }
}




void
Processor::onDownloadSslErrors(const QList<QSslError> errors)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    reply->ignoreSslErrors();

    qDebug() << QString("SSL error on %1")
                .arg(reply->url().toString());
}




void
Processor::onTimerTimeout()
{
    QTimer *timer = qobject_cast<QTimer*>(QObject::sender());
    timer->disconnect();
    timer->deleteLater();


    // Maybe a cancellation has been issued before the timeout.
    if (_cancelationPending) {
        return;
    }

    qDebug() << "Retrying download...";

    reset();
    download();
}





void
Processor::download()
{
    _speedElapsedTimer.start();

    _soundFile = new QTemporaryFile();
    _soundFile->setAutoRemove(false);
    _soundFile->open();



    QNetworkRequest srequest;
    srequest.setUrl(QUrl(_download.soundUrl));
    srequest.setRawHeader("Accept", "*/*");
    srequest.setRawHeader("Accept-Encoding", "gzip, deflate, sdch");
    srequest.setRawHeader("Accept-Language", "en-US,en;q=0.8");
    srequest.setRawHeader("Accept-Charset", "utf-8");
    srequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.125 Safari/537.36");
    _soundNetworkReply = _networkManager.get(srequest);

    connect (_soundNetworkReply,
             SIGNAL(downloadProgress(qint64,qint64)),
             this,
             SLOT(onDownloadProgressChanged(qint64, qint64)));

    connect (_soundNetworkReply,
             SIGNAL(readyRead()),
             this,
             SLOT(onDownloadReadyRead()));

    connect (_soundNetworkReply,
             SIGNAL(finished()),
             this,
             SLOT(onDownloadFinished()));

    connect (_soundNetworkReply,
             SIGNAL(sslErrors(QList<QSslError>)),
             this,
             SLOT(onDownloadSslErrors(QList<QSslError>)));


    /* Video specific setup is needed only when downloading the video and not
     * only the sound stream.
     */
    if (isVideoMode())
    {
        _videoFile = new QTemporaryFile();
        _videoFile->setAutoRemove(false);
        _videoFile->open();

        QNetworkRequest vrequest;
        vrequest.setUrl(QUrl(_download.videoUrl));
        vrequest.setRawHeader("Accept-Charset", "utf-8");
        vrequest.setRawHeader("charset", "utf-8");
        _videoNetworkReply = _networkManager.get(vrequest);

        connect (_videoNetworkReply,
                 SIGNAL(downloadProgress(qint64,qint64)),
                 this,
                 SLOT(onDownloadProgressChanged(qint64, qint64)));

        connect (_videoNetworkReply,
                 SIGNAL(readyRead()),
                 this,
                 SLOT(onDownloadReadyRead()));

        connect (_videoNetworkReply,
                 SIGNAL(finished()),
                 this,
                 SLOT(onDownloadFinished()));

        connect (_videoNetworkReply,
                 SIGNAL(sslErrors(QList<QSslError>)),
                 this,
                 SLOT(onDownloadSslErrors(QList<QSslError>)));
    }


    _status = Downloading;
    setDisplay(Downloading, 0, 0, 0);
    emit statusChanged();
}





QString
Processor::getSaveFilepath(const QString &title, const QString &extension)
{
    QString separator = QDir::separator();
    QString cleanFilename = Utility::cleanFilename(title);
    QString returnFilename = _savePath;
    returnFilename += separator;
    returnFilename += QString("%1.%2").arg(cleanFilename, extension);

    int index = 0;
    while (QFile::exists(returnFilename))
    {
        index++;
        returnFilename = _savePath;
        returnFilename += separator;
        returnFilename +=
            QString("%1-%2.%3")
                .arg(cleanFilename, QString::number(index), extension);
    }

    return returnFilename;
}





void
Processor::setDisplay(Status status, qint64 eta, qint64 speed, qint64 progress)
{
    QString s = "";
    switch (status)
    {
        case Ready:
            s = tr("Queued");
            break;
        case Downloading:
            s = tr("Downloading");
            break;
        case Converting:
            s = tr("Converting");
            break;
        case Complete:
            s = tr("Complete");
            break;
        case ErrorIO:
            s = tr("I/O Error");
            break;
        case ErrorConnection:
            s = tr("Connection Error");
            break;
        case Canceled:
            s = tr("Canceled");
            break;
    }

    _download.statusItem->setText(s);


    if (eta != -1)
    {
        // eta is the estimated time of availability in seconds
        int minutes = eta / 60;

        QString remainingString;
        if (eta <= 60) {
            remainingString = QString("%1s").arg(QString::number(eta));
        }
        else
        {
            QString m = QString::number(minutes);
            QString s = QString::number(eta - (minutes * 60));
            remainingString = QString("%1m %2s").arg(m, s);
        }

        _download.etaItem->setText(remainingString);
    } else {
        _download.etaItem->setText(tr("N/A"));
    }

    if (speed != -1)
    {
        QString s = QString::number(speed);
        _download.speedItem->setText(QString("%1 KB/s").arg(s));
    } else {
        _download.speedItem->setText(tr("N/A"));
    }



    _download.progressItem->setText(QString::number(progress));
}





bool
Processor::isVideoMode()
{
    return _download.convertExtension == "";
}





bool
Processor::isVideoValid()
{
    bool hasVideoUrl = !_download.videoUrl.isEmpty();
    bool hasSoundUrl = !_download.soundUrl.isEmpty();
    bool hasTitle = !_download.videoTitle.isEmpty();
    bool hasVideoExtension = !_download.videoExtension.isEmpty();
    bool hasSoundExtension = !_download.soundExtension.isEmpty();

    return hasVideoUrl &&
           hasSoundUrl &&
           hasTitle &&
           hasVideoExtension &&
           hasSoundExtension;
}
