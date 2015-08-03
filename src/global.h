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
 * along with SYLoader.  If not, see http://www.gnu.org/licenses.
 ******************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#include "messenger.h"
#include "scheduler.h"
#include "gatewaypool.h"
#include <QSettings>

#define SOFTWARE_VERSION_MAJOR 1
#define SOFTWARE_VERSION_MINOR 0
#define SOFTWARE_VERSION_PATCH 0

#define UPDATE_CHECK_URL "http://panastas91.github.io/SYLoader/lversion.html"
#define HOMEPAGE_URL "http://panastas91.github.io/SYLoader"
#define DOWNLOADS_URL "http://panastas91.github.io/SYLoader"
#define CONTACT_URL "https://twitter.com/panastasiadist"
#define FACEBOOK_URL "https://www.facebook.com/groups/512464022107865"
#define TWITTER_URL "https://twitter.com/hashtag/syloader"


#if defined(_WIN32)
    #define FFMPEG_PATH "ffmpeg.exe"
#else
    #define FFMPEG_PATH "ffmpeg"
#endif


extern QSettings *Settings;
extern Messenger *MessageBus;
extern Scheduler *Tasks;
extern GatewayPool *Gateway;


#endif // GLOBAL_H

