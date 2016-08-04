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


#include "downloader.h"
#include "global.h"
#include "utility.h"
#include "downloader_progress.h"
#include <math.h>
#include <QDir>
#include <QTemporaryFile>
#include <QtWidgets/QMessageBox>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>



#define MAX_RETRIES 3
#define RETRY_INTERVAL 10000



Downloader::Downloader(Download d, QString savePath)
{
    // Initialize some variables
    _savePath = savePath;
    _download = d;
    _retryCount = 0;
    _soundNetworkReply = NULL;
    _videoNetworkReply = NULL;
    _soundFile = NULL;
    _videoFile = NULL;
    _convertPid = 0;

    connect(Tasks,
            SIGNAL(statusChanged(TaskProcessor::Status, int, int)),
            this,
            SLOT(onTaskStatusChanged(TaskProcessor::Status, int, int)));

    reset();

    if (isVideoValid())
    {
        qDebug() << QString("Processing %1. Artist: %2. Coartist: %3. Title: %4")
                    .arg(d.videoTitle, d.artist, d.coartist, d.title);
    }
    else
    {
        qDebug() << QString("Processor stopped on an A/S Error");
    }
}



Downloader::~Downloader()
{
    Tasks->disconnect(this);
}



void
Downloader::start()
{
    // The video's information is not valid. We can't start.
    if (!isVideoValid()) {
        return;
    }

    if (_status == Ready) {
        download();
    }
}



void
Downloader::stop()
{
    // The video's information is not valid. This processor never started.
    if (!isVideoValid()) {
        return;
    }


    // Check if a cancelation has been already issued.
    if (_cancelationPending) {
        return;
    }

    // The rest of the code should know that cancelation has been issued.
    _cancelationPending = true;


    if (_status == Downloading)
    {
        // Call the downloading managers to abort downloading.
        if (_soundNetworkReply != NULL) {
            _soundNetworkReply->abort();
        }

        // The video download manager has been activated to download the video
        if (_videoNetworkReply != NULL) {
            _videoNetworkReply->abort();
        }

    }
    else if (_status == Converting)
    {
        // Call the converting process to terminate.
        // Because we have set cancelation pending, the termination process will
        // not raise an IO Error.
        Tasks->abort(_convertPid);
    }
    else if (_status == Ready)
    {
        // If the processor hasn't run yet, just mark it as canceled
    }


    setStatus(Canceled);
    setProgress(0, 0, 0);
}



void
Downloader::reset()
{
    QString filename = "";

    _videoBytes = 0;
    _soundBytes = 0;
    _bytesTotal = 0;
    _bytesDownloaded = 0;
    _videoBytesReceived = 0;
    _soundBytesReceived = 0;
    _lastBytesDownloaded = 0;
    _eta = 0;
    _kbps = 0;
    _percent = 0;
    _cancelationPending = false;
    _convertPid = 0;

    bool hasArtist = !_download.artist.isEmpty();
    bool hasTitle = !_download.title.isEmpty();
    bool hasCoartist = !_download.coartist.isEmpty();

    if (hasArtist && hasTitle)
    {
        filename += _download.artist;

        if (hasCoartist)
            filename += " ft. " + _download.coartist;

        filename += " - " + _download.title;
    }
    else
    {
        filename = _download.videoTitle;
    }

    _download.filename = filename;

    QString extension = _download.convertExtension;
    QString savePath = getOutputPath(filename, extension);

    if (QFile::exists(savePath))
    {
        setStatus(Complete);
        setProgress(0, 0, 100);
    }
    else
    {
        setStatus(Ready);
        setProgress(0, 0, 0);
    }
}


void
Downloader::report()
{
    emit statusChanged();
    emit progressChanged();
}



Download *Downloader::getDownload()
{
    return &_download;
}



Downloader::Status
Downloader::getStatus()
{
    return _status;
}



DownloaderProgress
Downloader::getProgress()
{
    DownloaderProgress progress;

    progress.kbps = _kbps;
    progress.percent = _percent;
    progress.seconds = _eta;

    return progress;
}



void
Downloader::onDownloadFinished()
{
    QNetworkReply::NetworkError videoError;
    QNetworkReply::NetworkError soundError;
    bool finished = false;


    if (_videoNetworkReply != NULL) {
        videoError = _videoNetworkReply->error();
    } else {
        videoError = QNetworkReply::NoError;
    }


    if (_soundNetworkReply != NULL) {
        soundError = _soundNetworkReply->error();
    } else {
        soundError = QNetworkReply::NoError;
    }


    if (_videoNetworkReply != NULL && _soundNetworkReply != NULL)
    {
        bool videoFinished = _videoNetworkReply->isFinished();
        bool soundFinished = _soundNetworkReply->isFinished();

        finished = videoFinished && soundFinished;
    }
    else if (_videoNetworkReply != NULL)
    {
        finished = _videoNetworkReply->isFinished();
    }
    else
    {
        finished = _soundNetworkReply->isFinished();
    }


    if (finished)
    {
        if (soundError == QNetworkReply::OperationCanceledError &&
            _videoNetworkReply == NULL)
        {
            videoError = QNetworkReply::OperationCanceledError;
        }

        if (videoError == QNetworkReply::OperationCanceledError ||
            soundError == QNetworkReply::OperationCanceledError)
        {
            // User has canceled downloading
            goto Cleanup;
        }
        else if (videoError || soundError)
        {
            // Some kind of error occurred on both downloaders. Examine it.
            goto ErrorProcedure;
        }
        else
        {
            // It's possible that the downloaders finish without an error but
            // with no data downloaded. I've noticed that the video is still
            // available for download. Don't know why this happens.
            // Just check for this behavior and take action.
            if (_soundNetworkReply != NULL && _soundBytesReceived == 0)
            {
                // Maybe a redirect from YouTube. Try it.
                // If it fails then, something else happened. Try error control.
                if (redirect(_soundNetworkReply)) {
                    return;
                }

                qDebug() << QString("Invalid sound data from %1")
                            .arg(_soundNetworkReply->url().toString());

                goto ErrorProcedure;
            }

            if (_videoNetworkReply != NULL && _videoBytesReceived == 0)
            {
                if (redirect(_videoNetworkReply)) {
                    return;
                }

                qDebug() << QString("Invalid video data from %1")
                            .arg(_videoNetworkReply->url().toString());

                goto ErrorProcedure;
            }


            QString iargs = " -id3v2_version 3 -write_id3v1 1 ";

            bool hasArtist = !_download.artist.isEmpty();
            bool hasTitle = !_download.title.isEmpty();
            bool hasCoartist = !_download.coartist.isEmpty();

            if (hasArtist && hasTitle)
            {
                QString artist = _download.artist;

                if (hasCoartist) {
                    artist += " ft. " + _download.coartist;
                }

                // The filename so far contains the artist's name
                // Write it using ID3 tags.
                iargs += " -metadata artist=\""+artist+"\" ";
                iargs += " -metadata title=\""+_download.title+"\" ";
            }

            QString command;

            QString filename = _download.filename;
            QString extension = _download.convertExtension;
            QString savePath = getOutputPath(filename, extension);

            // Video service which separates sound and video stream.
            // The video was requested as both streams have been downloaded.
            // We will merge them into the final video file.
            if (_videoNetworkReply != NULL && _soundNetworkReply != NULL)
            {
                QString vfilename = _videoFile->fileName(),
                        sfilename = _soundFile->fileName(),
                        args = "%1 -i \"%2\" -i \"%3\" -acodec copy -vcodec copy %4 \"%5\"";

                command = QString(args)
                          .arg(FFMPEG_PATH,
                               vfilename,
                               sfilename,
                               iargs,
                               savePath);

                _videoFile->close();
                _soundFile->close();
            }
            else if (_videoNetworkReply != NULL && _soundNetworkReply == NULL)
            {
                // This is a video service which supports one stream
                // containing both the video and the sound stream.

                QString vfilename = _videoFile->fileName();

                if (_download.convertExtension.isEmpty() == false)
                {
                    QString args = "%1 -i \"%2\" -acodec copy %3 \"%4\"";
                    command = QString(args)
                              .arg(FFMPEG_PATH,
                                   vfilename,
                                   iargs,
                                   savePath);
                }
            }
            else if (_videoNetworkReply == NULL && _soundNetworkReply != NULL)
            {
                // Video service which separates sound and video streams.
                // Only the sound stream has been downloaded, so the final file
                // will be sound only.


            }

            if (command.length() == 0)
            {
                QString sfilename = _soundFile->fileName();

                command = QString("%1 -y -i \"%2\" %3 \"%4\"")
                            .arg(FFMPEG_PATH,
                                 sfilename,
                                 iargs,
                                 savePath);

                _soundFile->close();
            }


            _convertPid = Tasks->enqueue(command);

            setStatus(Converting);
            setProgress(0, 0, 100);

            goto Cleanup;
        }
    }
    else
    {
        // One of the two downloaders (video/sound) has finished.
        // This may be normal. A common scenario is that sound has been
        // downloaded because of smaller size but video is still pending.
        // But maybe a downloader has finished because of an error.
        // Check and take action accordingly.
        if (videoError || soundError) {
            goto ErrorProcedure;
        }
    }

    return;


ErrorProcedure:

    // If at least one downloader has errored, stop the other.
    if (videoError && _soundNetworkReply != NULL) {
        _soundNetworkReply->abort();
    }

    if (soundError && _videoNetworkReply != NULL) {
        _videoNetworkReply->abort();
    }


    if (_retryCount < MAX_RETRIES)
    {
        _retryCount++;

        qDebug() << QString("Processor had an error connection. Retrying for %1 time in %2 ms")
                    .arg(QString::number(_retryCount))
                    .arg(QString::number(RETRY_INTERVAL));

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

        setStatus(ErrorConnection);
        setProgress(0, 0, 0);

        goto Cleanup;
    }


Cleanup:
    if (_soundNetworkReply != NULL)
    {
        _soundNetworkReply->deleteLater();
        _soundNetworkReply = NULL;

        if (_soundFile->isOpen()) {
            _soundFile->close();
        }

        delete _soundFile;

        _soundFile = NULL;
    }

    if (_videoNetworkReply != NULL)
    {
        _videoNetworkReply->deleteLater();
        _videoNetworkReply = NULL;

        if (_videoFile->isOpen()) {
            _videoFile->close();
        }

        delete _videoFile;

        _videoFile = NULL;
    }

}



void
Downloader::onTaskStatusChanged(TaskProcessor::Status status, int pid, int exitCode)
{
    if (pid != _convertPid) {
        return;
    }

    if (status == TaskProcessor::Started)
    {
        setStatus(Converting);

        qDebug() << "Converter started";
    }
    else if (status == TaskProcessor::Finished)
    {
        if (_cancelationPending) {
            return;
        }

        if (exitCode == 0)
        {
            setStatus(Complete);
        }
        else
        {
            setStatus(ErrorIO);

            qDebug() << QString("Converter errored: %1").arg(exitCode);
        }
    }

    setProgress(0, 0, 100);
}



void
Downloader::onDownloadProgressChanged(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());

    // May be 0, on a connection error or glitch;
    if (bytesTotal == 0) {
        return;
    }


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

        if (speed == 0) {
            return;
        }

        _lastBytesDownloaded = _bytesDownloaded;
        _speedElapsedTimer.restart();

        qint64 seconds = ((_bytesTotal - _bytesDownloaded) / 1024) / speed;

        setProgress(seconds, speed, progress);
    }

}



void
Downloader::onDownloadReadyRead()
{
    if (_soundNetworkReply != NULL)
    {
        QByteArray soundData = _soundNetworkReply->readAll();
        _soundFile->write(soundData);
    }

    if (_videoNetworkReply != NULL)
    {
        QByteArray videoData = _videoNetworkReply->readAll();
        _videoFile->write(videoData);
    }
}



void
Downloader::onDownloadSslErrors(const QList<QSslError> errors)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());

    reply->ignoreSslErrors();

    qDebug() << QString("SSL error on %1").arg(reply->url().toString());
}



void
Downloader::onTimerTimeout()
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
Downloader::download()
{
    _speedElapsedTimer.start();


    if (_download.soundUrl.length() > 0)
    {
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
        _soundNetworkReply = Gateway->get(srequest);

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
    }


    if ((_soundNetworkReply == NULL) ||
        (_soundNetworkReply != NULL && isVideoMode()))
    {
        _videoFile = new QTemporaryFile();
        _videoFile->setAutoRemove(false);
        _videoFile->open();

        QNetworkRequest vrequest;
        vrequest.setUrl(QUrl(_download.videoUrl));
        vrequest.setRawHeader("Accept", "*/*");
        vrequest.setRawHeader("Accept-Encoding", "gzip, deflate, sdch");
        vrequest.setRawHeader("Accept-Language", "en-US,en;q=0.8");
        vrequest.setRawHeader("Accept-Charset", "utf-8");
        vrequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.125 Safari/537.36");
        _videoNetworkReply = Gateway->get(vrequest);

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


    setStatus(Downloading);
    setProgress(0, 0, 0);
}



bool
Downloader::redirect(QNetworkReply *reply)
{
    if (reply == _soundNetworkReply)
    {
        _soundNetworkReply->deleteLater();
    }
    else if (reply == _videoNetworkReply)
    {
        _videoNetworkReply->deleteLater();
    }


    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();


    if(redirect.isValid() && reply->url() != redirect)
    {
        if(redirect.isRelative()) {
            redirect = reply->url().resolved(redirect);
        }

        QNetworkRequest request;
        request.setUrl(redirect);
        request.setRawHeader("Accept", "*/*");
        request.setRawHeader("Accept-Encoding", "gzip, deflate, sdch");
        request.setRawHeader("Accept-Language", "en-US,en;q=0.8");
        request.setRawHeader("Accept-Charset", "utf-8");
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.125 Safari/537.36");

        if (reply == _soundNetworkReply)
        {
            disconnect(_soundNetworkReply);
            _soundNetworkReply->deleteLater();
            _soundNetworkReply = reply = Gateway->get(request);
        }
        else if (reply == _videoNetworkReply)
        {
            disconnect(_videoNetworkReply);
            _videoNetworkReply->deleteLater();
            _videoNetworkReply = reply = Gateway->get(request);
        }

        connect (reply,
                 SIGNAL(downloadProgress(qint64,qint64)),
                 this,
                 SLOT(onDownloadProgressChanged(qint64, qint64)));

        connect (reply,
                 SIGNAL(readyRead()),
                 this,
                 SLOT(onDownloadReadyRead()));

        connect (reply,
                 SIGNAL(finished()),
                 this,
                 SLOT(onDownloadFinished()));

        connect (reply,
                 SIGNAL(sslErrors(QList<QSslError>)),
                 this,
                 SLOT(onDownloadSslErrors(QList<QSslError>)));


        qDebug() << QString("Redirecting to %1").arg(redirect.toString());

        return true;
    }
    else
    {
        return false;
    }
}



QString
Downloader::getOutputPath(QString title, QString extension)
{
    QString separator = QDir::separator(),
            cleanFilename = Utility::cleanFilename(title),
            returnFilename = _savePath;

    returnFilename += separator;
    returnFilename += QString("%1.%2").arg(cleanFilename, extension);

    return returnFilename;
}



void
Downloader::setStatus(Downloader::Status status)
{
    _status = status;

    emit statusChanged();
}



void
Downloader::setProgress(qint64 eta, qint64 speed, qint64 percent)
{
    _kbps = speed;
    _percent = percent;
    _eta = eta;

    emit progressChanged();
}



bool
Downloader::isVideoMode()
{
    return _download.convertExtension == "mp4";
}



bool
Downloader::isVideoValid()
{
    bool titleOK = true,
         videoUrlOK = true,
         soundUrlOK = true,
         videoExtensionOK = true,
         soundExtensionOK = true;


    // Always available
    if (_download.videoTitle.isEmpty()) {
        titleOK = false;
    }

    // Always available.
    if(_download.videoUrl.isEmpty()) {
        videoUrlOK = false;
    }

    // Always available because video url is always available.
    if (_download.videoExtension.isEmpty()) {
        videoExtensionOK = false;
    }

    // Maybe available if the video website separates the video stream from the
    // sound stream. Then an additional sound url will be available.
    if (!_download.soundUrl.isEmpty())
    {
        if (_download.soundExtension.isEmpty()) {
            soundExtensionOK = false;
        }
    }


    return titleOK &&
           videoUrlOK &&
           soundUrlOK &&
           videoExtensionOK &&
           soundExtensionOK;
}
