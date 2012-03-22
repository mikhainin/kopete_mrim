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
#include <kdebug.h>
#include <qtimer.h>

#include "mraprotocol.h"
#include <pthread.h>

#include "mracontactlist.h"


unsigned long int sec_count;


MRAProtocol::MRAProtocol(QObject*parent)
    : QObject(parent)
    , m_connection(0)
    , sec_count(0)
{

    pthread_mutex_init(&this->mutex1, NULL);
}


MRAProtocol::~MRAProtocol()
{
    if (m_keepAliveTimer) {
        m_keepAliveTimer->stop();
        disconnect(m_keepAliveTimer, SIGNAL(timeout()) , this , SLOT(slotPing()));
        delete m_keepAliveTimer;
        m_keepAliveTimer = 0;
    }

}

bool MRAProtocol::makeConnection(const std::string &login, const std::string &password)
{
    m_connection = new MRAConnection(this);
    if ( !m_connection->connectToHost() ) {
        delete m_connection;
        return false;
    }

    sendHello();

    connect( m_connection, SIGNAL(onData()), this, SLOT(slotOnDataFromServer()) );
    connect( m_connection, SIGNAL(disconnected(QString)), this, SLOT(slotDisconnected(QString)) );
    sendLogin(login, password);

    m_keepAliveTimer = new QTimer(this);
    connect(m_keepAliveTimer, SIGNAL(timeout()) , this , SLOT(slotPing()));
    m_keepAliveTimer->start(sec_count*1000);

    return true;
}

void MRAProtocol::closeConnection() {
    if (!m_connection) {
        return;
    }

    m_connection->disconnect();
    m_connection->deleteLater();
    m_connection = 0;

    m_keepAliveTimer->stop();
    delete m_keepAliveTimer;
    m_keepAliveTimer = 0;

}

void MRAProtocol::slotDisconnected(const QString &reason) {
    closeConnection();
    emit disconnected(reason);
}

/*!
    \fn MRAMsg::sendHello()
 */
void MRAProtocol::sendHello()
{
    m_connection->sendMsg(MRIM_CS_HELLO, NULL);
    MRAData data;
    mrim_msg_t msg;
    kWarning() << "HELLO sent";

    m_connection->readMessage(msg, &data);

    sec_count = data.getInt32();
    kWarning() << "HELLO ACK received, timeout sec:" << sec_count ;

}


/*!
    \fn MRAMsg::sendLogin()
 */
void MRAProtocol::sendLogin(const std::string &login, const std::string &password)
{
    MRAData data;

    data.addString(login.c_str());
    data.addString(password.c_str());
    data.addInt32(STATUS_ONLINE);
    data.addString("Kopete MRIM plugin v0.0.1");


    m_connection->sendMsg(MRIM_CS_LOGIN2, &data);
/*
    mrim_msg_t msg;

    m_connection->readMessage(msg, &data);

    if ( msg == MRIM_CS_LOGIN_ACK ) {
        return true;
    } else {
        /// @todo: read reason
        return false;
    }
*/
}


/*!
    \fn MRAMsg::sendMessage()
 */
void MRAProtocol::sendText(QString to, QString text)
{
    MRAData data;

    unsigned long int flags = MESSAGE_FLAG_NORECV;

    data.addInt32(flags);
    data.addString(to);
    data.addString(text);
    data.addString(" ");// RTF has not supported yet

    m_connection->sendMsg(MRIM_CS_MESSAGE, &data);
}


/*!
    \fn MRAProtocol::sendSMS(std::string number, std::string text)
 */
/*
void MRAProtocol::sendSMS(std::string number, std::string text)
{
    MRAData data;
    data.addInt32(0);
    data.addString(number.c_str());
    data.addString(text.c_str());
    this->conn->sendMsg(MRIM_CS_SMS, &data);

    //u_long msg;
    //this->conn->readMessage(msg, &data);
}
    */

QVector<QVariant> MRAProtocol::readVectorByMask(MRAData & data, const QString &mask)
{
    QVector<QVariant> result;

    quint32 _int;
    QString _string;

    for (int k = 0; k < mask.length(); ++k) {
        if (mask[k] == 'u') {
            _int = data.getInt32();
            // kWarning() << "u=" << null_int;
            result.push_back(_int);
        } else if (mask[k] == 's') {
            _string = data.getString();
            // kWarning() << "s=" << null_string;
            result.push_back(_string);
        }
    }
    return result;
}

void MRAProtocol::readContactList(MRAData & data)
{
    MRAContactList list;

    int status = data.getInt32();
    int group_num = data.getInt32();

    list.setStatus(status);

    std::cout << "group_num: " << group_num << std::endl;

    QString gmask;
    QString umask;

    gmask = data.getString(); // "us" - флаги и название
    umask = data.getString(); // uussuus (флаги, группа, адрес, ник, серверные флаги, текущий статус в сети)

    kWarning() << "gmask=" << gmask << " umask=" << umask;
    ulong flags;
    QString gname;

    MRAGroup g;

    for (int i = 0; i < group_num; ++i) {
        QVector<QVariant> protoData = readVectorByMask(data, gmask);
        flags = protoData[0].toInt(); // data.getInt32();
        gname = protoData[1].toString(); //data.getString();

        g.flags = flags;
        g.name = gname;

        kWarning() << "added group " << flags << gname;

        list.groups().add(g);
    }

    while( !data.eof() ) {
        MRAContactListEntry item;

        QVector<QVariant> protoData = readVectorByMask(data, umask);
        item.setFlags(   protoData[0].toInt() );
        item.setGroup(   protoData[1].toInt() );
        item.setAddress( protoData[2].toString() );
        item.setNick(    protoData[3].toString() );
        item.setServerFlags( protoData[4].toInt() );
        item.setStatus(  protoData[5].toInt() );

        list.addEntry(item);

        kWarning() << "added contact" << item.flags() << item.nick() << item.address();
    }

    emit contactListReceived(list);
}

void MRAProtocol::readUserInfo(MRAData & data)
{
    QString str;
    QString val;
    while (!data.eof()) {
        str = data.getString();
        val = data.getString();
        kWarning() << str << " " << val;
    }

}

void MRAProtocol::readMessage(MRAData & data) {
    // UL msg_id
    // UL flags
    // LPS from
    // LPS message
    // LPS rtf-formatted message (>=1.1)

    int msg_id = data.getInt32();
    int flags  = data.getInt32();
    QString from = data.getString();
    QString text = data.getString();
    // QString rtf  = data.getString(); // ignore

    if ( (flags & MESSAGE_FLAG_NOTIFY) != 0 ) {
        emit typingAMessage( from );
    } else {
        emit messageReceived( from, text );
    }

    if ( (flags & MESSAGE_FLAG_NORECV) == 0 ) {

        MRAData ackData;

        ackData.addString(from); // LPS ## from ##
        ackData.addInt32(msg_id); // UL ## msg_id ##

        m_connection->sendMsg(MRIM_CS_MESSAGE_RECV, &ackData);
    }
}

void MRAProtocol::readConnectionRejected(MRAData & data) {
    /// TODO: handle this event
    QString reason = data.getString();

    emit loginFailed(reason);
}

void MRAProtocol::readLogoutMessage(MRAData & data) {
    /// TODO: handle this event
    QString reason = data.getString();

    emit disconnected(reason);
}

void MRAProtocol::readAuthorizeAck(MRAData & data) {
    QString from = data.getString(); /// TODO: handle this event

    emit authorizeAckReceived(from);
}

void MRAProtocol::authorizeContact(const QString &contact) {
    MRAData authData;
    authData.addString(contact);

    m_connection->sendMsg( MRIM_CS_AUTHORIZE, &authData );

}

void MRAProtocol::addToContactList(int flags, int groupId, const QString &address, const QString nick) {
    MRAData addData; /// TODO: use this function

    addData.addInt32(flags);
    addData.addInt32(groupId);
    addData.addString(address);
    addData.addString(nick);
    addData.addString(" "); // unused LPS

    m_connection->sendMsg(MRIM_CS_ADD_CONTACT, &addData);
}

void MRAProtocol::readUserSataus(MRAData & data) {
    int status   = data.getInt32();
    QString user = data.getString();

    emit userStatusChanged(user, status);
}

void MRAProtocol::handleMessage(const u_long &msg, MRAData *data)
{
    kWarning() << "Accepting message " << msg;
    switch (msg) {
        case MRIM_CS_USER_INFO:

            readUserInfo(*data);
            break;

        case MRIM_CS_CONTACT_LIST2:
            readContactList(*data);
            break;

        case MRIM_CS_MESSAGE_ACK:
            readMessage(*data);
            break;

        case MRIM_CS_LOGIN_ACK:
            emit connected();/// TODO: handle this event
            return;

        case MRIM_CS_LOGIN_REJ:
            readConnectionRejected(*data);
            return;

        case MRIM_CS_LOGOUT:
            readLogoutMessage(*data);
            return;

        case MRIM_CS_USER_STATUS:
            readUserSataus(*data);
            return;

        case MRIM_CS_MESSAGE_STATUS:
        case MRIM_CS_CONNECTION_PARAMS:
        case MRIM_CS_ADD_CONTACT_ACK:
        case MRIM_CS_OFFLINE_MESSAGE_ACK:
        case MRIM_CS_AUTHORIZE_ACK:
            readAuthorizeAck(*data);
            break;
        case MRIM_CS_MPOP_SESSION:
//	case MRIM_CS_FILE_TRANSFER_ACK:
        case MRIM_CS_ANKETA_INFO: {
            kWarning() << "there is no handler for " << msg;
            break;
        }
        default: {
            kWarning()  << "unknown message " << msg;
        }
    }
}

void MRAProtocol::slotPing() {
    kWarning() << "sending ping";
    m_connection->sendMsg(MRIM_CS_PING, NULL);
    kDebug() << "ping sent";
}

void MRAProtocol::slotOnDataFromServer() {
    kWarning() << __PRETTY_FUNCTION__;
    MRAData data;
    mrim_msg_t msg;
    m_connection->readMessage(msg, &data);

    handleMessage(msg, &data);
}

#include "mraprotocol.moc"
