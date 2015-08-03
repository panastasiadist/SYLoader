#ifndef GATEWAYPOOL_H
#define GATEWAYPOOL_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct GatewayPoolItem
{
    QNetworkAccessManager *manager;
    int connections;
};


class GatewayPool : public QObject
{
    Q_OBJECT

private:
    QList<GatewayPoolItem> _managers;


public:
    GatewayPool();
    ~GatewayPool();
    QNetworkReply *get(QNetworkRequest request);


signals:

public slots:
    void onFinished(QNetworkReply *reply);
};

#endif // GATEWAYPOOL_H
