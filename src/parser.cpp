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

#include "parser.h"
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QList>
#include <QRegularExpression>




Parser::Parser()
{
    connect(&_process,
            SIGNAL(finished(int, QProcess::ExitStatus)),
            this,
            SLOT(onProcessFinished(int, QProcess::ExitStatus)));
}




void
Parser::parse(QString url)
{
    QString ydlExecutable = "youtube-dl";
    QString format = "%1 --no-warnings --prefer-insecure -J %2";

    _process.start(QString(format).arg(ydlExecutable, url));

    return;
}




void
Parser::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QByteArray data = _process.readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(QString(data).toUtf8());
    QJsonObject odoc = doc.object();
    QList<Download> downloads;
    QJsonArray entries;

    QJsonValue t = odoc.value("entries");
    if (t.isUndefined())
        entries.append(odoc);
    else
        entries = t.toArray();


    /* We will now try extracting useful information about artist, title
     * and coartist (if available) from the video's title.
     * This information is needed in other parts of the software, if the
     * user wants to keep the music stream of the video.
     */
    QRegularExpression featRegex("(feat\\.*|ft\\.*)");
    featRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    foreach (QJsonValue e, entries)
    {
        QJsonObject eo = e.toObject();

        QString videoTitle = eo.value("title").toString();
        QString title = "";
        QString artist = "";
        QString coartist = "";

        QStringList titleparts = videoTitle.split("-");


        /* Most commonly a video's title has two parts
         * Try analyzing of the following patterns:
         * Artist feat. Coartist - Song
         * Artist ft. Coartist - Song
         * Artist - Song feat. Coartist
         * Artist - Song ft. Coartist
         */
        int tcount = titleparts.count();
        if (tcount > 1)
        {
            QString left = QString(titleparts.at(0)).trimmed();
            QString right = QString(titleparts.at(1)).trimmed();

            QStringList lparts = left.split(featRegex);
            if (lparts.count() == 2)
            {
                artist = QString(lparts.at(0)).trimmed();
                coartist = QString(lparts.at(1)).trimmed();
                title = right;
            }
            else
            {
                QStringList rparts = right.split(featRegex);
                if (rparts.count() == 2)
                {
                    title = QString(rparts.at(0)).trimmed();
                    coartist = QString(rparts.at(1)).trimmed();
                    artist = left;
                }
                else
                {
                    artist = left;
                    title = right;
                }
            }

            /* If more than 2, the title has more than one -
             * The first and second parts are already examined.
             * Append any remaining parts to the title as a fallback
             */
            for (int i = 2; i < tcount; i++)
                title += " " + titleparts.at(i);
        }

        Download download;
        download.normalUrl = eo.value("webpage_url").toString();
        download.videoTitle = videoTitle;
        download.artist = artist;
        download.title = title;
        download.coartist = coartist;



        QJsonArray requestedFormats = eo.value("requested_formats").toArray();
        foreach (QJsonValue r, requestedFormats)
        {
            QJsonObject format = r.toObject();
            QString url = format.value("url").toString();
            QString extension = format.value("ext").toString();

            if (format.value("fps").isNull()) {
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
                if (format.value("fps").isNull())
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

        downloads.append(download);

    }

    emit finished(downloads);

    return;
}
