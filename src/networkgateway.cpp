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

#include "networkgateway.h"


// The maximum number of requests a manager will concurrently handle before
// a new manager needs to be created for another set of connections.
// Per QT docs, QNetworkAccessManager processes up to 6 connections at the same
// time on the desktop platforms. The rest are enqueued.
#define MAX_CONNECTIONS 6



NetworkGateway::NetworkGateway() {}



NetworkGateway::~NetworkGateway()
{
    // Cleanup
    foreach(NetworkGatewayManager item, _managers) {
        item.manager->deleteLater();
    }

    _managers.clear();
}



QNetworkReply*
NetworkGateway::get(QNetworkRequest request)
{
    QNetworkReply*          reply;
    QNetworkAccessManager*  manager;
    int idx, count;

    reply = NULL;
    count = _managers.count();

    // First search in current managers to find one which hasn't reached
    // MAX_CONNECTIONS of open connections. If one is found, use it for the
    // new request and increase its connection count by one.
    for (idx = 0; idx < count; idx++)
    {
        NetworkGatewayManager item = _managers[idx];
        manager = item.manager;
        int connections = item.connections;

        if (connections < MAX_CONNECTIONS)
        {
            _managers[idx].connections++;
            reply = manager->get(request);
            break;
        }
    }

    // If reply is NULL then either there is no manager created yet to handle
    // the request or all current managers already have MAX_CONNECTIONS
    // connections open. So we need to create and store a new manager for this
    // new request.
    if (reply == NULL)
    {
        // Create and store a new manager which will handle the current request
        // and another MAX_CONNECTIONS - 1 (current request) connections.
        manager = new QNetworkAccessManager();

        NetworkGatewayManager item;
        item.manager = manager;
        item.connections = 1;

        _managers.append(item);

        // Signaled each time a request handled by the manager has finished.
        // Useful for bookkeeping purposes and cleanup.
        connect(manager,
                SIGNAL(finished(QNetworkReply*)),
                this,
                SLOT(onFinished(QNetworkReply*)));

        // Finally make the request and assign the handle to it.
        reply = manager->get(request);
    }

    return reply;
}



void
NetworkGateway::onFinished(QNetworkReply *reply)
{
    QNetworkAccessManager* manager;
    int itemToDelete, count;

    itemToDelete = -1;
    count = _managers.count();
    manager = qobject_cast<QNetworkAccessManager*>(QObject::sender());

    // A request made through a QNAM has finished.
    // Find the manager attached to the specific QNAME and decrease its current
    // connections possibly marking it for removal (if no active connections).
    for (int idx = 0; idx < count; idx++)
    {
        NetworkGatewayManager item = _managers.at(idx);

        if (item.manager == manager)
        {
            item.connections--;

            if (item.connections == 0) {
                itemToDelete = idx;
                break;
            }
        }
    }

    // If the manager has no active connections, it has been marked for removal.
    // Clean it as it no longer needed.
    if (itemToDelete != -1)
    {
        _managers.at(itemToDelete).manager->deleteLater();
        _managers.removeAt(itemToDelete);
    }

}

