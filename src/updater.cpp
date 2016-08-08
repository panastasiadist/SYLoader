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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>
#include "updater.h"
#include "global.h"



Updater::Updater()
{
    _checkingUpdates = false;
    _updatingYoutubeDl = false;
}



bool
Updater::parseVersionString(QString versionStr,
                            int &major,
                            int &minor,
                            int &patch)
{
    major = 0;
    minor = 0;
    patch = 0;

    QStringList versionDigits = versionStr.split(".", QString::SkipEmptyParts);

    if (versionDigits.length() == 3)
    {
        bool majorOk = false;
        bool minorOk = false;
        bool patchOk = false;

        int tMajor = versionDigits.at(0).toInt(&majorOk, 10);
        int tMinor = versionDigits.at(1).toInt(&minorOk, 10);
        int tPatch = versionDigits.at(2).toInt(&patchOk, 10);

        if (majorOk && minorOk && patchOk)
        {
            major = tMajor;
            minor = tMinor;
            patch = tPatch;
            return true;
        }
    }

    return false;
}


bool
Updater::isNewer(int latestMajor,
                 int latestMinor,
                 int latestPatch,
                 int currentMajor,
                 int currentMinor,
                 int currentPatch)
{
    bool hasUpdate = false;

    if (latestMajor > currentMajor)
    {
        hasUpdate = true;
    }
    else if (latestMajor == currentMajor)
    {
        if (latestMinor > currentMinor)
        {
            hasUpdate = true;
        }
        else if (latestMinor == currentMinor)
        {
            if (latestPatch > currentPatch) {
                hasUpdate = true;
            }
        }
    }

    return hasUpdate;
}



bool
Updater::fillProgramUpdateInfo(QJsonObject object)
{
    QString latestVersionString = object.value("version").toString();

    int latestMajor, latestMinor, latestPatch;

    bool latestVersionParsed = parseVersionString(latestVersionString,
                                             latestMajor,
                                             latestMinor,
                                             latestPatch);

    if (!latestVersionParsed) {
        return false;
    }


    bool update = isNewer(latestMajor,
                          latestMinor,
                          latestPatch,
                          SOFTWARE_VERSION_MAJOR,
                          SOFTWARE_VERSION_MINOR,
                          SOFTWARE_VERSION_PATCH);

    _updateData.ProgramMajor = latestMajor;
    _updateData.ProgramMinor = latestMinor;
    _updateData.ProgramPatch = latestPatch;
    _updateData.ProgramUpdate = update;

    return true;
}



bool
Updater::fillYoutubeDlUpdateInfo(QJsonObject object)
{
    QString latestVersionString = object.value("version").toString();

#ifdef _WIN32
    QString packageUrl = object.value("package_win").toString();
#else
    QString packageUrl = object.value("package_lin").toString();
#endif

    // If youtube-dl version file can't be found, force update of youtube-dl


    QString currentVersionString = "0.0.0";

    int latestMajor, latestMinor, latestPatch;
    int currentMajor, currentMinor, currentPatch;

    QFile file(YOUTUBEDL_VERSION_FILE);
    if(file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        currentVersionString = in.readLine();
        file.close();
    }

    bool latestVersionParsed = parseVersionString(latestVersionString,
                                                  latestMajor,
                                                  latestMinor,
                                                  latestPatch);

    bool currentVersionParsed = parseVersionString(currentVersionString,
                                                   currentMajor,
                                                   currentMinor,
                                                   currentPatch);

    if (!latestVersionParsed || !currentVersionParsed) {
        return false;
    }

    bool update = isNewer(latestMajor,
                          latestMinor,
                          latestPatch,
                          currentMajor,
                          currentMinor,
                          currentPatch);

    _updateData.YdlMajor = latestMajor;
    _updateData.YdlMinor = latestMinor;
    _updateData.YdlPatch = latestPatch;
    _updateData.YdlUpdate = update;
    _updateData.YdlPackageUrl = packageUrl;

    return true;
}



void Updater::check()
{
    _checkingUpdates = true;

    QNetworkRequest request(QUrl(UPDATE_CHECK_URL));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    _networkReply = Gateway->get(request);

    connect(_networkReply,
            SIGNAL(finished()),
            this,
            SLOT(onNetworkReplyFinished()));
}



UpdateInfo
Updater::getUpdateData()
{
    return _updateData;
}


void
Updater::updateYdl()
{
    _updatingYoutubeDl = true;

    QNetworkRequest request(QUrl(_updateData.YdlPackageUrl));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    _networkReply = Gateway->get(request);

    connect(_networkReply,
            SIGNAL(finished()),
            this,
            SLOT(onNetworkReplyFinished()));
}



void
Updater::onNetworkReplyFinished()
{
    QByteArray replyBytes = _networkReply->readAll();
    _networkReply->deleteLater();

    if (_checkingUpdates)
    {
        _checkingUpdates = false;

        QString replyString = QString(replyBytes);


        // The answer is a JSON object containing information about the main
        // components of the software. Grab the node regarding each component
        // and pass it to the relevant functions for further analysis.
        // Finally _updateData struct will contain version information about the
        // components which can used by the rest of the software to initiate the
        // update procedure.

        QJsonDocument doc = QJsonDocument::fromJson(QString(replyString).toUtf8());
        QJsonObject odoc = doc.object();
        QJsonObject syloader = odoc.value("syloader").toObject();
        QJsonObject youtubedl = odoc.value("youtubedl").toObject();

        bool res1 = fillProgramUpdateInfo(syloader);
        bool res2 = fillYoutubeDlUpdateInfo(youtubedl);

        if (res1 && res2)
        {
            emit updateCheckFinished(true);
        }
        else
        {
            emit updateCheckFinished(false);
        }
    }
    else if (_updatingYoutubeDl)
    {
        _updatingYoutubeDl = false;

        QString version = QString("%1.%2.%3")
                .arg(_updateData.YdlMajor)
                .arg(_updateData.YdlMinor)
                .arg(_updateData.YdlPatch);

        QFile file(YOUTUBEDL_EXECUTABLE);
        if (file.open(QIODevice::ReadWrite))
        {
            file.write(replyBytes);
            file.close();
        }
        else
        {
            emit updateInstallationFinished(false);
        }

        QFile file2(YOUTUBEDL_VERSION_FILE);
        if (file2.open(QIODevice::ReadWrite))
        {
            QTextStream stream(&file2);
            stream << version;
            file2.close();
        }
        else
        {
            emit updateInstallationFinished(false);
        }

        emit updateInstallationFinished(true);
    }
}
