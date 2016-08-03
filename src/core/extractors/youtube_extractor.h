#ifndef YOUTUBE_EXTRACTOR_H
#define YOUTUBE_EXTRACTOR_H


#include "QObject"
#include "QProcess"
#include "core/extractor.h"


class YoutubeExtractor : public Extractor
{

    Q_OBJECT

public:
    YoutubeExtractor();
    void extract(QString url);
    bool isPlaylist(QString url);
    QString canonicalizeUrl(QString url);

    static bool isSupported(QString url);

private:
    QProcess _process;
    QString _url;
    int _retryCount;

public slots:
    void onProcessFinished(int exitCode);

};


#endif // YOUTUBE_EXTRACTOR_H
