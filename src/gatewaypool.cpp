#include "gatewaypool.h"

/* Per QT docs, QNetworkAccessManager processes up to 6 connections at the same
 * time on the desktop platforms. The rest are enqueued;
 */
#define MAX_CONNECTIONS 6





GatewayPool::GatewayPool()
{

}





GatewayPool::~GatewayPool()
{
    foreach(GatewayPoolItem item, _managers) {
        item.manager->deleteLater();
    }

    _managers.clear();
}





QNetworkReply*
GatewayPool::get(QNetworkRequest request)
{
    QNetworkReply *reply = NULL;

    foreach(GatewayPoolItem item, _managers)
    {
        QNetworkAccessManager *manager = item.manager;
        int connections = item.connections;

        if (connections < MAX_CONNECTIONS)
        {
            /* Sometimes the downloaders finish with no data received.
             * The video is actually available and accessible.
             * I don't know why this happens but creating a new QNAM seems to
             * minimize the appearance of the problem.
             */
            manager->clearAccessCache();

            item.connections++;
            reply = manager->get(request);
            break;
        }
    }

    if (reply == NULL)
    {
        QNetworkAccessManager *manager = new QNetworkAccessManager();

        connect(manager,
                SIGNAL(finished(QNetworkReply*)),
                this,
                SLOT(onFinished(QNetworkReply*)));

        reply = manager->get(request);

        GatewayPoolItem item;
        item.manager = manager;
        item.connections = 1;

        _managers.append(item);
    }

    return reply;
}





void
GatewayPool::onFinished(QNetworkReply *reply)
{
    QNetworkAccessManager *manager =
            qobject_cast<QNetworkAccessManager*>(QObject::sender());

    int itemToDelete = -1;
    int count = _managers.count();

    for (int idx = 0; idx < count; idx++)
    {
        GatewayPoolItem item = _managers.at(idx);
        if (item.manager == manager)
        {
            item.connections--;

            if (item.connections == 0) {
                itemToDelete = idx;
            }
        }
    }

    // Don't delete the last one QNAM. Most probably it will be needed again.
    if (itemToDelete != -1 && count > 1)
    {
        _managers.at(itemToDelete).manager->deleteLater();
        _managers.removeAt(itemToDelete);
    }

}

