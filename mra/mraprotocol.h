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
#ifndef MRAPROTOCOL_H
#define MRAPROTOCOL_H
#include "mra_proto.h"
#include "mradata.h"
#include "mraconnection.h"
#include "mracontactlist.h"

#include <QObject>


class MRAProtocol : public QObject
{
    Q_OBJECT

public:
    MRAProtocol(QObject *parent = 0);

    ~MRAProtocol();
    //void sendMsg(MRAData *data);
    bool makeConnection(const std::string &login, const std::string &password);
    void closeConnection();
    //unsigned int readMessage(MRAData *data, u_long &msg);
    void sendText(QString to, QString text);
    // void sendSMS(std::string number, std::string text);


    void handleMessage(const u_long &msg, MRAData *data);
    void addToContactList(int flags, int groupId, const QString &address, const QString nick);
    void authorizeContact(const QString &contact);

private:
    MRAConnection *m_connection;
    int sec_count;

    pthread_t threadPingLoop;
    pthread_t threadMsgLoop;
    pthread_mutex_t mutex1;
    QTimer *m_keepAliveTimer;
private:
    void sendHello();
    void sendLogin(const std::string &login, const std::string &password);
    QVector<QVariant> readVectorByMask(MRAData & data, const QString &mask);

    void readContactList(MRAData & data);
    void readUserInfo(MRAData & data);
    void readMessage(MRAData & data);
    void readAuthorizeAck(MRAData & data);
    void readConnectionRejected(MRAData & data);
    void readLogoutMessage(MRAData & data);
    void readUserSataus(MRAData & data);
    void readConnectionParams(MRAData & data);

private slots:
    void slotPing();
    void slotOnDataFromServer();
    void slotDisconnected(const QString &reason);
signals:
    void contactListReceived(const MRAContactList &list);

    void messageReceived(const QString &from, const QString text);
    void typingAMessage(const QString &from);

    void authorizeAckReceived(const QString &from);
    void connected();
    void disconnected(const QString &reason);
    void loginFailed(const QString &reason);
    void userStatusChanged(const QString &user, int newStatus);
};

#endif
