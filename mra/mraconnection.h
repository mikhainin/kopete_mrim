/***************************************************************************
 *   Copyright (C) 2007 by netgr@am   *
 *   a@localhost   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MRACONNECTION_H
#define MRACONNECTION_H

#include <QObject>
#include <QTcpSocket>

#include "mra_proto.h"
#include "mradata.h"


#include <iostream>
#include <cstdlib>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class MRAConnection : public QObject{
    Q_OBJECT
private:


public:
    MRAConnection(QObject *parent = 0);

    class CantEstablishConnection;
    ~MRAConnection();
    bool connectToHost();
    ssize_t write(const char *buf, ssize_t size);
    ssize_t read(char* buf, ssize_t size);

    void sendMsg(mrim_msg_t msg, MRAData *data);
    // void sendMsg(mrim_msg_t msg, int message_id, MRAData *data);

    ssize_t readMessage(mrim_msg_t &msg, MRAData *data);

    void disconnect();
signals:

    void onData();
    void disconnected(const QString &reason);

private slots:
    void slotReadyRead();
    void slotDisconnected();

private:
    mrim_packet_header_t header;

    quint16 m_localPort;
    quint32 m_localAddress;
    quint16 m_remotePort;
    quint32 m_remoteAddress;
    QTcpSocket *m_socket;
    bool m_locked;

    QString getRecommendedServer();
};

#endif
