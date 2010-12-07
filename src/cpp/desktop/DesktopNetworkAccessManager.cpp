/*
 * DesktopNetworkAccessManager.cpp
 *
 * Copyright (C) 2009-11 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */

#include "DesktopNetworkAccessManager.hpp"

NetworkAccessManager::NetworkAccessManager(QString secret, QObject *parent) :
    QNetworkAccessManager(parent), secret_(secret)
{
}

QNetworkReply* NetworkAccessManager::createRequest(
      Operation op,
      const QNetworkRequest& req,
      QIODevice* outgoingData)
{
   QNetworkRequest req2 = req;
   req2.setRawHeader(QString("X-Shared-Secret").toAscii(),
                     secret_.toAscii());
   return this->QNetworkAccessManager::createRequest(op, req2, outgoingData);
}
