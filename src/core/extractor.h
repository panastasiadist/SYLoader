#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include "download.h"
#include <QObject>
#include <QString>
#include <QList>

class Extractor : public QObject
{

    Q_OBJECT


public:
    virtual void extract(QString url) {}
    virtual bool isPlaylist(QString url) {}
    virtual QString canonicalizeUrl(QString url) {}

signals:
    void finished(int, QList<Download>);

};

#endif // EXTRACTOR_H
