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
                break;
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

