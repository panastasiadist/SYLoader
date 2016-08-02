#include "helper.h"
#include <QTextStream>
#include <QRegularExpression>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QMessageBox>
#include <QFile>
#include <QList>
#include <QDebug>


Helper::Helper() {}



Download
Helper::decorateDownload(Download download)
{
    QString videoTitle = download.videoTitle;
    QString title = "";
    QString artist = "";
    QString coartist = "";

    QStringList titleparts = videoTitle.split("-");

    QRegularExpression featRegex("(feat\\.*|ft\\.*)");
    featRegex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);


    /* Most commonly a video's title has two parts
     * Try analyzing of the following patterns:
     * Artist feat. Coartist - Song
     * Artist ft. Coartist - Song
     * Artist - Song feat. Coartist
     * Artist - Song ft. Coartist
     */
    int tcount = titleparts.count();
    if (tcount > 1)
    {
        QString left = QString(titleparts.at(0)).trimmed();
        QString right = QString(titleparts.at(1)).trimmed();

        QStringList lparts = left.split(featRegex);
        if (lparts.count() == 2)
        {
            artist = QString(lparts.at(0)).trimmed();
            coartist = QString(lparts.at(1)).trimmed();
            title = right;
        }
        else
        {
            QStringList rparts = right.split(featRegex);
            if (rparts.count() == 2)
            {
                title = QString(rparts.at(0)).trimmed();
                coartist = QString(rparts.at(1)).trimmed();
                artist = left;
            }
            else
            {
                artist = left;
                title = right;
            }
        }

        /* If more than 2, the title has more than one -
         * The first and second parts are already examined.
         * Append any remaining parts to the title as a fallback
         */
        for (int i = 2; i < tcount; i++)
            title += " " + titleparts.at(i);
    }

    download.title = title;
    download.artist = artist;
    download.coartist = coartist;

    return download;
}
