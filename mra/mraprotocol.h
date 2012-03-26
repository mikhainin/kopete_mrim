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
#include "mraofflinemessage.h"

#include <QObject>

class MRAAvatarLoader;
class QImage;

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
    void removeContact(const QString &contact);

    void sendTypingMessage(const QString &contact);

    void loadAvatar(const QString &contact);
    void loadAvatarLoop();

    /**
     * Return whether the protocol supports offline messages.
     */
    bool canSendOffline() const { return true; }

private:
    MRAConnection *m_connection;
    int sec_count;

    QTimer *m_keepAliveTimer;
    bool m_contactListReceived;
    QList<MRAOfflineMessage*> m_offlineMessages;
    QList<MRAAvatarLoader*> m_avatarLoaders;
    int m_avatarLoadersCount;

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
    void readOfflineMessage(MRAData & data);
    void emitOfflineMessagesReceived();

private slots:
    void slotPing();
    void slotOnDataFromServer();
    void slotDisconnected(const QString &reason);
    void slotAvatarLoaded(bool success, MRAAvatarLoader *loader);

signals:
    void contactListReceived(const MRAContactList &list);

    void messageReceived(const QString &from, const QString text);
    void typingAMessage(const QString &from);
    void offlineReceived(const MRAOfflineMessage &message);

    void authorizeRequestReceived(const QString &from, const QString text);
    void authorizeAckReceived(const QString &from);

    void connected();
    void disconnected(const QString &reason);
    void loginFailed(const QString &reason);
    void userStatusChanged(const QString &user, int newStatus);

    void avatarLoaded(const QString &contact, const QImage &image);
};

#endif
