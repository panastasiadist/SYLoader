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
#include "utility.h"
#include <QRegExp>
#include <QRegularExpression>
#include <QTextStream>
#include <QThread>



#ifdef Q_OS_WIN32
#include <windows.h>



QString
Utility::cleanFilename(QString desiredFilename)
{
    return desiredFilename.replace(QRegExp("[<>:\"/\\|?*]"), QString("-"));
}



QString
Utility::getFFmpegFilename()
{
    return "ffmpeg.exe";
}

#else

#include <stdio.h>
#include <sys/utsname.h>
#include <sys/personality.h>



QString
Utility::cleanFilename(QString desiredFilename)
{
    return desiredFilename.replace(QRegExp("[/]"), QString("-"));
}



QString
Utility::getFFmpegFilename()
{
    return "ffmpeg";
}

#endif



int
Utility::getMaxThreads()
{
    return QThread::idealThreadCount();
}



Download
Utility::decorateDownload(Download download)
{
    QString videoTitle = download.videoTitle;
    QString title = "";
    QString artist = "";
    QString coartist = "";
    QStringList titleparts = videoTitle.split("-");

    QRegularExpression featRegex("(feat\\.*|ft\\.*)");
    featRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);


    // Most commonly a video's title has two parts
    // Try analyzing of the following patterns:
    // Artist feat. Coartist - Song
    // Artist ft. Coartist - Song
    // Artist - Song feat. Coartist
    // Artist - Song ft. Coartist

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

        // If more than 2, the title has more than one dashes.
        // The first and second parts are already examined.
        // Append any remaining parts to the title as a fallback.

        for (int i = 2; i < tcount; i++) {
            title += " " + titleparts.at(i);
        }
    }

    download.title = title;
    download.artist = artist;
    download.coartist = coartist;

    return download;
}
