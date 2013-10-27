#include <kdebug.h>
#include <QTimer>
#include <QStringList>
// #include <QHttp>

#include "../debug.h"
#include "mracontactlist.h"
#include "mraavatarloader.h"
#include "mraprotocol.h"
#include "mracontactinfo.h"
#include "mraconnection.h"
#include "mraofflinemessage.h"
#include "transferrequestinfo.h"
#include "mra_proto.h"
#include "filetransferinfo.h"
#include "transfermanager.h"
#include "../version.h"

struct MRAProtocol::MRAProtocolPrivate {
    MRAConnection *connection;
    int secCount;

    QTimer *keepAliveTimer;
    QTimer *offlineMessagesTimer;
    QList<MRAOfflineMessage*> offlineMessages;
    QList<MRAAvatarLoader*> avatarLoaders;
    int avatarLoadersCount;
    IMRAProtocolGroupReceiver *grouReceiver;
    IMRAProtocolContactReceiver *contactReceiver;
    qtmra::TransferManager transferManager;


    MRAProtocolPrivate()
      : connection(0)
      , secCount(0)
      , keepAliveTimer(0)
      , offlineMessagesTimer(0)
      , avatarLoadersCount(0)
      , grouReceiver(0)
      , contactReceiver(0)
    {
    }
};

MRAProtocol::MRAProtocol(QObject*parent)
    : QObject(parent)
    , d(new MRAProtocolPrivate)
{
}


MRAProtocol::~MRAProtocol()
{
    closeConnection();

    if (d->keepAliveTimer) {
        d->keepAliveTimer->stop();
        disconnect(d->keepAliveTimer, SIGNAL(timeout()) , this , SLOT(slotPing()));
        delete d->keepAliveTimer;
        d->keepAliveTimer = 0;
    }
    delete d;
}

MRAConnection *MRAProtocol::connection() {
    return d->connection;
}

void MRAProtocol::sendMsg(const mrim_msg_t &msg, MRAData *data) {
    if (d->connection) {
        d->connection->sendMsg(msg, data);
    } else {
        mrimWarning() << "sending message to a closed connection" << msg;
    }
}

bool MRAProtocol::makeConnection(const QString &login, const QString &password)
{
    d->connection = new MRAConnection(this);
    if ( !d->connection->connectToHost() ) {
        delete d->connection;
        return false;
    }

    sendHello();

    connect( d->connection, SIGNAL(onData()), this, SLOT(slotOnDataFromServer()) );
    connect( d->connection, SIGNAL(disconnected(QString)), this, SLOT(slotDisconnected(QString)) );
    sendLogin(login, password);

    d->keepAliveTimer = new QTimer(this);
    connect(d->keepAliveTimer, SIGNAL(timeout()) , this , SLOT(slotPing()));
    d->keepAliveTimer->start(d->secCount*1000);

    return true;
}

void MRAProtocol::loadChatMembersList(const QString &to) {
    Q_UNUSED(to); // proto v1.23
}

void MRAProtocol::inviteMemberToChat(const QString &to, const QString &contactIdToInvite) {
    Q_UNUSED(to);
    Q_UNUSED(contactIdToInvite);
}

void MRAProtocol::removeMemberFromChat(const QString &to, const QString &contactIdToRemove) {
    Q_UNUSED(to);
    Q_UNUSED(contactIdToRemove);
}

void MRAProtocol::addGroupToContactList(const QString &groupName, IMRAProtocolGroupReceiver *groupAddedReveiver) {

    MRAData addData;

    addData.addUint32(CONTACT_FLAG_GROUP);
    addData.addUint32(0);
    addData.addString("");
    addData.addString(groupName);
    addData.addString(""); // unused LPS

    sendMsg(MRIM_CS_ADD_CONTACT, &addData);

    setGroupReceiver(groupAddedReveiver);

}


void MRAProtocol::closeConnection() {

    // d->contactListReceived = false;
    d->transferManager.removeAllSessions();
    if (d->connection) {
        d->connection->disconnect();
        d->connection->deleteLater();
        d->connection = 0;
    }

    if (d->keepAliveTimer) {
        d->keepAliveTimer->stop();
        d->keepAliveTimer->deleteLater();
        d->keepAliveTimer = 0;
    }

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
    sendMsg(MRIM_CS_HELLO, NULL);
    MRAData data;
    mrim_msg_t msg;

    d->connection->readMessage(msg, &data);

    d->secCount = data.getUint32();
    mrimDebug() << "HELLO ACK received, timeout sec:" << d->secCount ;

}

void MRAProtocol::readConnectionParams(MRAData & data) {

    d->secCount = data.getUint32();

    d->keepAliveTimer->deleteLater();
    d->keepAliveTimer = 0;

    d->keepAliveTimer = new QTimer(this);
    connect(d->keepAliveTimer, SIGNAL(timeout()) , this , SLOT(slotPing()));
    d->keepAliveTimer->start(d->secCount*1000);

}

/*!
    \fn MRAMsg::sendLogin()
 */
void MRAProtocol::sendLogin(const QString &login, const QString &password)
{
    MRAData data;

    // proto v1.07/1.08
    data.addString(login);
    data.addString(password);
    data.addUint32(STATUS_ONLINE);
    data.addString("Kopete MRIM plugin v" + kopeteMrimVersion() );

    sendMsg(MRIM_CS_LOGIN2, &data);
}

void MRAProtocol::readLoginAck(MRAData & data) {
    Q_UNUSED(data);
    emit connected();
}

/*!
    \fn MRAMsg::sendMessage()
 */
void MRAProtocol::sendText(const QString &to, const QString &text)
{
    MRAData data;

    unsigned long int flags = MESSAGE_FLAG_NORECV;

    data.addUint32(flags);
    data.addString(to);
    data.addString(text);
    data.addString(" ");// RTF is not supported yet

    sendMsg(MRIM_CS_MESSAGE, &data);
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
            _int = data.getUint32();
            result.push_back(_int);
        } else if (mask[k] == 's') {
            _string = data.getString();
            result.push_back(_string);
        }
    }
    return result;
}

void MRAProtocol::readContactList(MRAData & data)
{
    MRAContactList list;

    int status = data.getUint32();
    int group_num = data.getUint32();

    list.setStatus(status);

    std::cout << "group_num: " << group_num << std::endl;

    QString gmask;
    QString umask;

    gmask = data.getString(); // "us" - flags and name
    umask = data.getString(); // uussuus (flags, group num, email address, nickname, server flags, current status)

    mrimDebug() << "gmask=" << gmask << " umask=" << umask;
    ulong flags;
    QString gname;

    MRAGroup g;

    for (int i = 0; i < group_num; ++i) {
        QVector<QVariant> protoData = readVectorByMask(data, gmask);
        flags = protoData[0].toInt(); // data.getInt32();
        gname = protoData[1].toString(); //data.getString();

        g.flags = flags;
        g.name = gname;

        mrimDebug() << "added group " << flags << gname;

        list.groups().add(g);
    }

    ulong userNumber = 20;
    while( !data.eof() ) {
        MRAContactListEntry item(userNumber++);

        QVector<QVariant> protoData = readVectorByMask(data, umask);

        fillUserInfo(protoData, item);

        list.addEntry(item);

        mrimDebug() << "added contact" << item.flags() << item.group() << item.nick() << item.address() << protoData[6].toString();
    }

    emit contactListReceived(list);

    // d->contactListReceived = true;

    emitOfflineMessagesReceived();
}

void MRAProtocol::fillUserInfo(QVector<QVariant> &protoData, MRAContactListEntry &item) {
    item.setFlags(   protoData[0].toUInt() );
    item.setGroup(   protoData[1].toUInt() );
    item.setAddress( protoData[2].toString() );
    item.setNick(    protoData[3].toString() );
    item.setServerFlags( protoData[4].toUInt() );
    item.setStatus(  protoData[5].toUInt() );
}

void MRAProtocol::readUserInfo(MRAData & data)
{
    QString str;
    QString val;
    while (!data.eof()) {
        str = data.getString();
        val = data.getString();
        mrimDebug() << str << " " << val;
    }

}

void MRAProtocol::readMessage(MRAData & data) {
    // UL msg_id
    // UL flags
    // LPS from
    // LPS message
    // LPS rtf-formatted message (>=1.1)

    int msg_id = data.getUint32();
    int flags  = data.getUint32();
    QString from = data.getString();
    QString text = data.getString();
    // QString rtf  = data.getString(); // ignore

    if ( (flags & MESSAGE_FLAG_NOTIFY) != 0 ) {
        emit typingAMessage( from );
    } else if ( (flags & MESSAGE_FLAG_AUTHORIZE) != 0) {
        emit authorizeRequestReceived(from, text);
    } else {
        emit messageReceived( from, text );
    }

    if ( (flags & MESSAGE_FLAG_NORECV) == 0 ) {

        MRAData ackData;

        ackData.addString(from); // LPS ## from ##
        ackData.addUint32(msg_id); // UL ## msg_id ##

        sendMsg(MRIM_CS_MESSAGE_RECV, &ackData);
    }
}

void MRAProtocol::sendTypingMessage(const QString &contact) {
    MRAData data;

    data.addUint32( MESSAGE_FLAG_NORECV | MESSAGE_FLAG_NOTIFY ); // flags
    data.addString(contact); // to
    data.addString(" "); // message
    data.addString(" "); // rtf

    sendMsg(MRIM_CS_MESSAGE, &data);
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

    sendMsg( MRIM_CS_AUTHORIZE, &authData );
}

void MRAProtocol::sendAuthorizationRequest(const QString &contact, const QString &myAddress, const QString &message) {

    Q_UNUSED(myAddress); // proto v1.23

    MRAData authData;
    unsigned long int authFlags = MESSAGE_FLAG_NORECV | MESSAGE_FLAG_AUTHORIZE | MESSAGE_FLAG_UNICODE;
    authData.addUint32(authFlags);
    authData.addString(contact);
    authData.addString(message);
    authData.addString("");// RTF is not supported yet

    connection()->sendMsg(MRIM_CS_MESSAGE, &authData);

}

void MRAProtocol::addToContactList(int flags, int groupId, const QString &address, const QString &nick, const QString &myAddress, const QString &authMessage, IMRAProtocolContactReceiver *contactAddReceiver) {

    Q_UNUSED(authMessage); // in proto v1.23
    Q_UNUSED(myAddress); // in proto v1.23

    MRAData addData;

    addData.addUint32(flags);
    addData.addUint32(groupId);
    addData.addString(address);
    addData.addString(nick);
    addData.addString(" "); // unused LPS

    sendMsg(MRIM_CS_ADD_CONTACT, &addData);

    setContactReceiver(contactAddReceiver);
}

void MRAProtocol::removeContact(const QString &contact) {
    addToContactList(CONTACT_FLAG_REMOVED, 0, contact, contact, "", "", 0);
}


void MRAProtocol::readUserSataus(MRAData & data) {
    int status   = data.getUint32();
    QString user = data.getString();

    emit userStatusChanged(user, status);
}

void MRAProtocol::setStatus(STATUS status) {
    MRAData data;
    data.addUint32(statusToInt(status));

    sendMsg(MRIM_CS_CHANGE_STATUS, &data);
}

int MRAProtocol::statusToInt(STATUS status) {
    if (status == ONLINE) {
        return STATUS_ONLINE;
    } else if (status == OFFLINE) {
        return STATUS_OFFLINE;
    } else if (status == AWAY) {
        return STATUS_AWAY;
    }
    return STATUS_UNDETERMINATED;
}

void MRAProtocol::readOfflineMessage(MRAData & data) {
    QByteArray msgId      = data.getUIDL();
    QString rfc822message = data.getString();

    MRAOfflineMessage *message = new MRAOfflineMessage (this, msgId);
    message->parse(rfc822message);

    d->offlineMessages.push_back(message);

    mrimDebug() << "offline message pushed" << d->offlineMessages.size();

    if (d->offlineMessagesTimer == 0) {
        d->offlineMessagesTimer = new QTimer(this);

        connect(d->offlineMessagesTimer,SIGNAL(timeout()),
                this, SLOT(slotOfflineMessagesReceived())
                );

        d->offlineMessagesTimer->setSingleShot(true);
        d->offlineMessagesTimer->start(5000);

    } else {
        d->offlineMessagesTimer->start(5000);
    }
//    if (d->contactListReceived) {
//        emitOfflineMessagesReceived();
//    }


}

void MRAProtocol::slotOfflineMessagesReceived() {
    d->offlineMessagesTimer->deleteLater();
    d->offlineMessagesTimer = 0;
    emitOfflineMessagesReceived();
}

bool MessageDateLessThan(MRAOfflineMessage *a, MRAOfflineMessage *b) {
     return a->date() < b->date();
}

void MRAProtocol::emitOfflineMessagesReceived() {

    // sort the list

    mrimDebug() << "offline message emmiting" << d->offlineMessages.size();

    qSort(d->offlineMessages.begin(), d->offlineMessages.end(), MessageDateLessThan);

    mrimDebug() << "offline message emmiting2" << d->offlineMessages.size();

    foreach( MRAOfflineMessage *message, d->offlineMessages ) {

        if ( (message->flags() & MESSAGE_FLAG_AUTHORIZE) != 0) {
            emit authorizeRequestReceived(message->from(), message->text());
/*        } else if ( (message->flags() & MESSAGE_FLAG_NOTIFY) != 0 ) {
            ;*/
        } else {
            emit offlineReceived(*message);
        }

        MRAData ackData;
        ackData.addUIDL(message->id());

        sendMsg(MRIM_CS_DELETE_OFFLINE_MESSAGE, &ackData);

        message->deleteLater();
    }

    d->offlineMessages.clear();

}

void MRAProtocol::loadAvatar(const QString &contact, bool large, QObject *receiver, const char *member) {

    mrimDebug() << contact;
    d->avatarLoaders.push_back(
                    new MRAAvatarLoader(contact, this, large, receiver, member)
                );

    loadAvatarLoop();
}

void MRAProtocol::loadAvatarLoop() {

    mrimDebug() << __PRETTY_FUNCTION__ << "loaders" << d->avatarLoadersCount;

    if ( d->avatarLoadersCount > 3 || d->avatarLoaders.empty() ) {
        return;
    }

    MRAAvatarLoader * loader = d->avatarLoaders.front();
    d->avatarLoaders.pop_front();

    QObject::connect(loader, SIGNAL(done(bool,MRAAvatarLoader*)),
                     this, SLOT(slotAvatarLoaded(bool,MRAAvatarLoader*)) );

    mrimDebug() << loader->contact();

    d->avatarLoadersCount++;

    loader->run();

}

void MRAProtocol::slotAvatarLoaded(bool success, MRAAvatarLoader *loader) {

    mrimDebug() << loader->contact() << success;

    if (success) {
        if ( !loader->receiver() ) {
            emit avatarLoaded( loader->contact(), loader->image() );
        }
    }

    d->avatarLoadersCount--;

    loader->deleteLater();

    loadAvatarLoop();
}

void MRAProtocol::loadUserInfo(const QString &contact) {

    QStringList items = contact.split('@');
    if (items.size() != 2) {
        return; /// todo raise error
    }

    MRAData anketaData;
    anketaData.addUint32(MRIM_CS_WP_REQUEST_PARAM_USER);
    anketaData.addString(items[0]);

    anketaData.addUint32(MRIM_CS_WP_REQUEST_PARAM_DOMAIN);
    anketaData.addString(items[1]);

    sendMsg( MRIM_CS_WP_REQUEST, &anketaData );
}

void MRAProtocol::readAnketaInfo(MRAData & data) {

    MRAContactInfo info;

    uint status     = data.getUint32();
    mrimDebug() << "status=" << status;
    uint fields_num = data.getUint32();
    uint max_rows   = data.getUint32();
    uint server_time= data.getUint32();
    Q_UNUSED(max_rows); /// @fixme: use this fields
    Q_UNUSED(server_time); /// @fixme: use this fields

    QVector<QString> vecInfo;
    vecInfo.reserve(fields_num);

    for( uint i = 0; i < fields_num; ++i ) {
        QString field = data.getString();
        mrimDebug() << field;
        vecInfo.append( field );
    }

    for( uint i = 0; i < fields_num; ++i ) {

        QString fieldData = data.getString();

        info.setParam(vecInfo[i], fieldData);

    }

    this->emit userInfoLoaded( info.email(), info );
}

void MRAProtocol::deleteContact(uint id, const QString &contact, const QString &contactName) {
    MRAData data;

    data.addUint32( id );
    data.addUint32( CONTACT_FLAG_REMOVED );
    data.addUint32( 0 ); // don't care about group
    data.addString( contact );
    data.addString( contactName );
    data.addString( QString() );

    connection()->sendMsg( MRIM_CS_MODIFY_CONTACT, &data );
}

void MRAProtocol::editContact(uint id, const QString &contact, uint groupId, const QString &newContactName) {
    MRAData data;

    data.addUint32( id );
    data.addUint32( 0 );
    data.addUint32( groupId ); // don't care about group
    data.addString( contact );
    data.addString( newContactName );
    data.addString( QString() );

    connection()->sendMsg( MRIM_CS_MODIFY_CONTACT, &data );
}

void MRAProtocol::readAddContactAck(MRAData & data) {
    int status    = data.getUint32();
    int contactId = data.getUint32();

    if (d->grouReceiver) {
        // TODO: it may be not only successfully
        d->grouReceiver->groupAddedSuccessfully();
        d->grouReceiver = 0;
    } else if (d->contactReceiver) {
        // TODO: it may be not only successfully
        d->contactReceiver->contactAddedSuccessfully();
        d->contactReceiver = 0;
    } else {
        emit addContactAckReceived(status, contactId);
    }
}

void MRAProtocol::setGroupReceiver(IMRAProtocolGroupReceiver*groupReceiver) {
    d->grouReceiver = groupReceiver;
}

void MRAProtocol::setContactReceiver(IMRAProtocolContactReceiver *contactReceiver) {
    d->contactReceiver = contactReceiver;
}

void MRAProtocol::readTransferRequest(MRAData & data) {
    Q_UNUSED(data);
}
void MRAProtocol::readTransferCancel(MRAData & data) {
    Q_UNUSED(data);
}
void MRAProtocol::readTransferUseThisProxy(MRAData &data) {
    Q_UNUSED(data);
}

void MRAProtocol::readTransferCantLocal(MRAData &data) {
    Q_UNUSED(data);
}

qtmra::TransferManager &MRAProtocol::transferManager() {
    return d->transferManager;
}


void MRAProtocol::handleMessage(const ulong &msg, MRAData *data)
{
    mrimDebug() << "Accepting message " << hex << msg;
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
            readLoginAck(*data);
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

        case MRIM_CS_CONNECTION_PARAMS:
            readConnectionParams(*data);
            return;

        case MRIM_CS_AUTHORIZE_ACK:
            readAuthorizeAck(*data);
            break;

        case MRIM_CS_OFFLINE_MESSAGE_ACK:
            readOfflineMessage(*data);
            break;

        case MRIM_CS_ANKETA_INFO:
            readAnketaInfo(*data);
            break;

        case MRIM_CS_ADD_CONTACT_ACK:
            readAddContactAck(*data);
            break;

        case MRIM_CS_TRANSFER_REQUEST:
            readTransferRequest(*data);
            break;
        case MRIM_CS_TRANSFER_CANCEL:
            readTransferCancel(*data);
            break;

        case MRIM_CS_TRANSFER_USE_THIS_PROXY:
            readTransferUseThisProxy(*data);
            break;

        case MRIM_CS_TRANSFER_CANT_LOCAL:
            readTransferCantLocal(*data);
            break;

        case MRIM_CS_MESSAGE_STATUS:

        case MRIM_CS_MPOP_SESSION:
        // case MRIM_CS_FILE_TRANSFER_ACK:
            mrimDebug() << "there is no handler for " << msg;
            break;


        default: {
            mrimDebug()  << "unknown message " << msg;
        }
    }
}

void MRAProtocol::slotPing() {
    //mrimDebug() << "sending ping";
    sendMsg(MRIM_CS_PING, NULL);

}

void MRAProtocol::slotOnDataFromServer() {
    mrimDebug() << __PRETTY_FUNCTION__;
    MRAData *data = new MRAData(this);
    mrim_msg_t msg;
    d->connection->readMessage(msg, data);

    handleMessage(msg, data);

    data->deleteLater();
}


#include "mraprotocol.moc"
