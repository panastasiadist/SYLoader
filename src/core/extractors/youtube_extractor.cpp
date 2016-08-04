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

#include "youtube_extractor.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMessageBox>
#include <QFile>
#include <QList>
#include <QDebug>



#define MAX_RETRIES 10



YoutubeExtractor::YoutubeExtractor()
{
    connect(&_process,
            SIGNAL(finished(int, QProcess::ExitStatus)),
            this,
            SLOT(onProcessFinished(int)));

    _retryCount = 0;
}



void
YoutubeExtractor::extract(QString url)
{
    _url = url;

    QString ydlExecutable = "youtube-dl";
    QString format = "%1 --no-warnings --no-cache-dir --prefer-insecure --no-check-certificate -i -J \"%2\"";

    _process.start(QString(format).arg(ydlExecutable, url));

    return;
}



QString
YoutubeExtractor::canonicalizeUrl(QString url)
{
    if (url.startsWith("https://"))
        url = url.replace("https://", "http://");

    QStringList urlParts = url.split("?");
    QString urlBasicPart = urlParts.at(0);
    QString urlQueryPart = urlParts.at(1);
    QStringList urlQueryItems = urlQueryPart.split("&");
    QString videoQueryItem = "";
    QString listQueryItem = "";

    foreach (QString i, urlQueryItems)
    {
        QStringList t = i.split("=");
        QString verb = t.at(0);
        if (verb == "v")
            videoQueryItem = i;
        else if (verb == "list")
            listQueryItem = i;
    }

    return
        urlBasicPart +
        "?" +
        videoQueryItem +
        (listQueryItem.isEmpty() ? "" : "&" + listQueryItem);
}



bool
YoutubeExtractor::isSupported(QString url)
{
    return
        url.contains("youtube.com/watch") ||
        url.contains("youtube.com/playlist?list=");
}


bool
YoutubeExtractor::isPlaylist(QString url)
{
    return url.contains("playlist?list=");
}



void
YoutubeExtractor::onProcessFinished(int exitCode)
{
    QByteArray data = _process.readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(QString(data).toUtf8());
    QJsonObject odoc = doc.object();
    QList<Download> downloads;
    QJsonArray entries;

    qDebug() << QString("Parser finished with result: %1")
                .arg(QString::number(exitCode));


    /* youtube-dl may return a result different to 0 if it finds a non critical
     * problem. A non critical problem is when a video is not available,
     * deleted, blocked. Then youtube-dl ignores it but its output is valid for
     * the rest of our code. So any error checking will be performed from our
     * code against the returned data.
     */


    QJsonValue t = odoc.value("entries");
    if (t.isUndefined())
        entries.append(odoc);
    else
        entries = t.toArray();


    foreach (QJsonValue e, entries)
    {
        QJsonObject eo = e.toObject();

        QString videoTitle = eo.value("title").toString();

        Download download;

        download.normalUrl = eo.value("webpage_url").toString();
        download.videoTitle = videoTitle;

        QJsonArray requestedFormats = eo.value("requested_formats").toArray();
        foreach (QJsonValue r, requestedFormats)
        {
            QJsonObject format = r.toObject();
            QString url = format.value("url").toString();
            QString extension = format.value("ext").toString();

            if (format.value("acodec").toString() != "none") {
                // Music track
                download.soundExtension = extension;
                download.soundUrl = url;
            } else {
                // Video track
                download.videoExtension = extension;
                download.videoUrl = url;
            }
        }



        /* We need to download m4a sound streams and mp4 video streams.
         * If the best available quality given in requested_formats by
         * youtube-dl is not m4a and mp4, then fallback to the next best
         * quality of the aforementioned formats.
         */
        if (download.soundExtension != "m4a" ||
            download.videoExtension != "mp4")
        {
            int bestAbrSoFar = 0;
            int bestWidthSoFar = 0;

            QJsonArray formats = eo.value("formats").toArray();
            foreach (QJsonValue r, formats)
            {
                QJsonObject format = r.toObject();
                QString ext = format.value("ext").toString();

                // Music track
                if (format.value("acodec").toString() != "none")
                {
                    if (download.soundExtension != "m4a")
                    {
                        int abr = format.value("abr").toInt();
                        if (ext == "m4a" && abr > bestAbrSoFar)
                        {
                            download.soundUrl =
                                    format.value("url").toString();
                            download.soundExtension =
                                    format.value("ext").toString();
                            bestAbrSoFar = abr;
                        }
                    }
                }
                else
                {
                    // Video track
                    if (download.videoExtension != "mp4")
                    {
                        int width = format.value("width").toInt();
                        if (ext == "mp4" && width > bestWidthSoFar)
                        {
                            download.videoUrl =
                                    format.value("url").toString();
                            download.videoExtension =
                                    format.value("ext").toString();
                            bestWidthSoFar = width;
                        }
                    }
                }

            }
        }


        /* The minimum information required by the rest of the software.
         * If for some reason, youtube-dl has exitted normally but has
         * given empty data for a download, then retry parsing.
         * Most often this happens because of a deleted or not available video.
         */
        bool eVideoUrl = download.videoUrl.isEmpty();
        bool eSoundUrl = download.soundUrl.isEmpty();
        bool eTitle = download.videoTitle.isEmpty();
        bool eVideoExtension = download.videoExtension.isEmpty();
        bool eSoundExtension = download.soundExtension.isEmpty();

        if (eVideoUrl || eSoundUrl || eTitle || eVideoExtension || eSoundExtension)
        {
            qDebug() << QString("Parsing requirements error for %1. Ignoring...")
                        .arg(download.normalUrl);

            continue;
        }


        downloads.append(download);

    }

    emit finished(0, downloads);

    return;
}
