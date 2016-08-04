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



#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QStringList>
#include "updater.h"
#include "global.h"



Updater::Updater() {}



bool
Updater::checkForUpdates(bool &hasUpdate)
{
    // Default.

    hasUpdate = false;


    // Make a request to fetch the latest version string

    QNetworkReply *updateReply = Gateway->get(
                QNetworkRequest(QUrl(UPDATE_CHECK_URL)));


    // Asynchronously wait for the request to finish before further executing.

    QEventLoop loop;
    QObject::connect(updateReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();


    // The request has finished. Read the content of the URL to buffer.

    QByteArray replyBytes = updateReply->readAll();
    QString replyString = QString(replyBytes);
    updateReply->deleteLater();


    // The version string returned is of major.minor.patch format where
    // major, minor, patch are integers. Split the string by dot in order to
    // get all parts of the version string.

    QStringList versionDigits = replyString.split(".", QString::SkipEmptyParts);


    // Examine and compare current and latest version only if the latest version
    // fetched has exactly three parts (major, minor, patch). Otherwise the
    // remote version string if malformed (just contact me :P)

    bool hasError = true;

    if (versionDigits.length() == 3)
    {
        // Continue only if the three parts of the version string are valid
        // integers and convert them to integers for further comparisons.

        bool majorOk = false;
        bool minorOk = false;
        bool patchOk = false;

        int major = versionDigits.at(0).toInt(&majorOk, 10);
        int minor = versionDigits.at(1).toInt(&minorOk, 10);
        int patch = versionDigits.at(2).toInt(&patchOk, 10);


        // Now decide if we have the latest version.

        if (majorOk && minorOk && patchOk)
        {
            hasError = false;

            if (major > SOFTWARE_VERSION_MAJOR)
            {
                hasUpdate = true;
            }
            else if (major == SOFTWARE_VERSION_MAJOR)
            {
                if (minor > SOFTWARE_VERSION_MINOR)
                {
                    hasUpdate = true;
                }
                else if (minor == SOFTWARE_VERSION_MINOR)
                {
                    if (patch > SOFTWARE_VERSION_PATCH) {
                        hasUpdate = true;
                    }
                }
            }
        }
    }


    // Cleanup and return.
    delete updateReply;
    return hasError;
}

