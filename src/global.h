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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "messenger.h"
#include "scheduler.h"
#include "networkgateway.h"
#include <QSettings>

#define SOFTWARE_VERSION_MAJOR 1
#define SOFTWARE_VERSION_MINOR 0
#define SOFTWARE_VERSION_PATCH 0

#define UPDATE_CHECK_URL "http://panastasiadist.github.io/SYLoader/lversion.html"
#define HOMEPAGE_URL "http://panastasiadist.github.io/SYLoader"
#define DOWNLOADS_URL "http://panastasiadist.github.io/SYLoader"
#define CONTACT_URL "https://twitter.com/panastasiadist"
#define FACEBOOK_URL "https://www.facebook.com/pages/SYLoader/1684668085090102"
#define TWITTER_URL "https://twitter.com/hashtag/SYLoader"


#if defined(_WIN32)
    #define FFMPEG_PATH "ffmpeg.exe"
#else
    #define FFMPEG_PATH "ffmpeg"
#endif


extern QSettings *Settings;
extern Messenger *MessageBus;
extern Scheduler *Tasks;
extern NetworkGateway *Gateway;


#endif // GLOBAL_H

