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

#include "processor.h"
#include "global.h"
#include "utility.h"
#include <math.h>
#include <QDir>
#include <QTemporaryFile>
#include <QtWidgets/QMessageBox>
#include <QCoreApplication>
#include <QDebug>





Processor::Processor(const Download d, const QString savePath)
{
    // Initialize some variables
    _savePath = savePath;
    _download = d;

    // Set up slots
    connect (&_videoNetworkManager,
             SIGNAL(finished(QNetworkReply*)),
             this,
             SLOT(onDownloadNetworkManagerFinished(QNetworkReply*)));

    connect (&_soundNetworkManager,
             SIGNAL(finished(QNetworkReply*)),
             this,
             SLOT(onDownloadNetworkManagerFinished(QNetworkReply*)));

    connect (&_convertProcess,
             SIGNAL(finished(int, QProcess::ExitStatus)),
             this,
             SLOT(onConvertCompleted(int, QProcess::ExitStatus)));


    qDebug() << QString("Artist: %1, Coartist: %2, Title: %3")
                .arg(d.artist, d.coartist, d.title);

    reset();
}





void
Processor::start()
{
    reset();
    download();
}





void
Processor::stop()
{
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
    }
    else if (_status == Converting)
    {
        /* Call the converting process to terminate.
         * Because we have set cancelation pending, the termination process
         * will not raise an IO Error.
         * statusChanged() will be emitted by the convertProcess SLOT.
         */
        _convertProcess.terminate();
    }
    else if (_status == Ready)
    {
        // If the processor hasn't run yet, just mark it as canceled
        _status = Canceled;
        setDisplay(Canceled, 0, 0, 0);
        emit statusChanged();
    }

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
Processor::onDownloadNetworkManagerFinished(QNetworkReply *networkReply)
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
            _status = Canceled;
            setDisplay(Canceled, 0, 0, 0);
            emit statusChanged();
            goto Cleanup;
        }
        else if (videoError || soundError)
        {
            // Some kind of error occurred on both downloaders. Examine it.
            goto ErrorProcedure;
        }
        else
        {
            // The video and music streams of the video have been downloaded.
            QTemporaryFile soundFile;
            QTemporaryFile videoFile;


            if (soundFile.open())
            {
                QByteArray data = _soundNetworkReply->readAll();
                soundFile.write(data);
                soundFile.close();
                soundFile.setAutoRemove(false);
            }


            if (isVideoMode()) {

                if (videoFile.open())
                {
                    QByteArray data = _videoNetworkReply->readAll();
                    videoFile.write(data);
                    videoFile.close();
                    videoFile.setAutoRemove(false);
                }
            }

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


            if (isVideoMode())
            {
                QString extension = _download.videoExtension;
                QString savePath = getSaveFilepath(filename, extension);
                QString vfilename = videoFile.fileName();
                QString sfilename = soundFile.fileName();
                QString args = "%1 -i \"%2\" -i \"%3\" -acodec copy -vcodec copy %4 \"%5\"";


                _convertProcess.start(
                    QString(args)
                        .arg(FFMPEG_PATH, vfilename, sfilename, iargs, savePath));
            }
            else
            {
                QString extension = _download.convertExtension;
                QString savePath = getSaveFilepath(filename, extension);
                QString sfilename = soundFile.fileName();
                QString command =
                        QString("%1 -y -i \"%2\" %3 \"%4\"")
                            .arg(FFMPEG_PATH, sfilename, iargs, savePath);


                _convertProcess.start(command);
            }

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
    _status = ErrorConnection;
    setDisplay(ErrorConnection, 0, 0, 0);

    if (videoError)
        _soundNetworkReply->abort();

    if (soundError && isVideoMode())
        _videoNetworkReply->abort();

    emit statusChanged();
    goto Cleanup;


Cleanup:
    disconnect(_soundNetworkReply);
    _soundNetworkReply->deleteLater();

    if (isVideoMode()) {
        disconnect(_videoNetworkReply);
        _videoNetworkReply->deleteLater();
    }

}





void
Processor::onConvertCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0)
    {
        _status = Complete;
        setDisplay(Complete, 0, 0, 100);
    }
    else
    {
        if (_cancelationPending)
        {
            _status = Canceled;
            setDisplay(Canceled, 0, 0, 100);
        }
        else
        {
            _status = ErrorIO;
            setDisplay(ErrorIO, 0, 0, 100);
        }
    }

    emit statusChanged();
}





void
Processor::onDownloadProgressChanged(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());


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


    if (_speedElapsedTimer.elapsed() > 900)
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





void Processor::download()
{
    _bytesDownloaded = 0;
    _lastBytesDownloaded = 0;
    _bytesTotal = 0;
    _speedElapsedTimer.start();


    QNetworkRequest srequest;
    srequest.setUrl(QUrl(_download.soundUrl));
    srequest.setRawHeader("Accept-Charset", "utf-8");
    srequest.setRawHeader("charset", "utf-8");
    _soundNetworkReply = _soundNetworkManager.get(srequest);

    connect (_soundNetworkReply,
             SIGNAL(downloadProgress(qint64,qint64)),
             this,
             SLOT(onDownloadProgressChanged(qint64, qint64)));


    /* Video specific setup is needed only when downloading the video and not
     * only the sound stream.
     */
    if (isVideoMode())
    {
        QNetworkRequest vrequest;
        vrequest.setUrl(QUrl(_download.videoUrl));
        vrequest.setRawHeader("Accept-Charset", "utf-8");
        vrequest.setRawHeader("charset", "utf-8");
        _videoNetworkReply = _videoNetworkManager.get(vrequest);

        connect (_videoNetworkReply,
                 SIGNAL(downloadProgress(qint64,qint64)),
                 this,
                 SLOT(onDownloadProgressChanged(qint64, qint64)));
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
            s = tr("Ready");
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
