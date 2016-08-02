#ifndef YOUTUBEEXTRACTOR_H
#define YOUTUBEEXTRACTOR_H

#include "../../extractor.h"
#include "QObject"
#include "QProcess"

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


#endif // YOUTUBEEXTRACTOR_H
