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
#include <QList>
#include <QDebug>
#include "url_processor.h"
#include "core/extractors/youtube_extractor.h"
#include "core/extractors/vimeo_extractor.h"
#include "utility.h"




UrlProcessor::UrlProcessor() {}



Extractor*
UrlProcessor::getExtractor(QString url)
{
    if (YoutubeExtractor::isSupported(url))
    {
        return new YoutubeExtractor();
    }
    else if (VimeoExtractor::isSupported(url))
    {
        return new VimeoExtractor();
    }
    else
    {
        return NULL;
    }

}



void
UrlProcessor::parse(QString url)
{
    _url = url;

    Extractor* extractor = this->getExtractor(url);

    // Normally, the caller should have already checked that the url is
    // supported by calling isSupported(). But if not then the returned
    // extractor will be null as there is no extractor supporting the url.
    // We perform a check in order to avoid abnormal termination of the program.

    if (extractor != NULL)
    {
        _extractors.append(extractor);

        connect(extractor,
                SIGNAL(finished(int, QList<Download>)),
                this,
                SLOT(onExtractorFinished(int, QList<Download>)));


        extractor->extract(url);
    }

    return;
}



bool
UrlProcessor::parsing()
{
    return _extractors.length() > 0;
}



bool
UrlProcessor::isPlaylist(QString url)
{
    Extractor* extractor = this->getExtractor(url);

    if (extractor != NULL)
    {
        bool res = extractor->isPlaylist(url);

        extractor->deleteLater();

        return res;
    }
    else
    {
        return false;
    }
}



bool
UrlProcessor::isSupported(QString url)
{
    return
            YoutubeExtractor::isSupported(url) ||
            VimeoExtractor::isSupported(url);

}



void
UrlProcessor::onExtractorFinished(int result, QList<Download> downloads)
{
    Extractor* extractor = qobject_cast<Extractor*>(QObject::sender());

    extractor->deleteLater();
    _extractors.removeOne(extractor);

    if (result == 0)
    {
        QList<Download> temp;

        foreach(Download d, downloads) {
            temp.append(Utility::decorateDownload(d));
        }

        extractor->deleteLater();

        emit parsed(temp);
    }

    return;
}
