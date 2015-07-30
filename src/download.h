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

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QStandardItem>

struct Download
{
    QString normalUrl;
    QString videoUrl;
    QString soundUrl;
    QString videoTitle;
    QString videoExtension;
    QString soundExtension;
    QString convertExtension;
    QString artist;
    QString title;
    QString coartist;
    QStandardItem *titleItem;
    QStandardItem *statusItem;
    QStandardItem *progressItem;
    QStandardItem *speedItem;
    QStandardItem *etaItem;

};


#endif // DOWNLOAD_H

