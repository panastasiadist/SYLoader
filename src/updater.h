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
#ifndef UPDATER_H
#define UPDATER_H



#include <QObject>
#include <QNetworkReply>
#include "update_info.h"



class Updater : public QObject
{
    Q_OBJECT

public:
    Updater();
    UpdateInfo getUpdateData();
    void check();
    void updateYdl();

private:
    UpdateInfo _updateData;
    QNetworkReply *_networkReply;
    bool _checkingUpdates;
    bool _updatingYoutubeDl;

    bool parseVersionString(QString versionStr,
                            int &major,
                            int &minor,
                            int &patch);
    bool isNewer(int latestMajor,
                 int latestMinor,
                 int latestPatch,
                 int currentMajor,
                 int currentMinor,
                 int currentPatch);
    bool fillProgramUpdateInfo(QJsonObject object);
    bool fillYoutubeDlUpdateInfo(QJsonObject object);


signals:
    void updateCheckFinished(bool success);
    void updateInstallationFinished(bool success);


private slots:
    void onNetworkReplyFinished();
};



#endif // UPDATER_H
