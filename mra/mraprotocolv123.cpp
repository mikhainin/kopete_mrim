#include <kdebug.h>
#include <QCryptographicHash>
#include <QTextCodec>
#include <QByteArray>

#include "mradata.h"
#include "mraconnection.h"
#include "mracontactlist.h"
#include "mracontactinfo.h"
#include "mraprotocolv123.h"

MRAProtocolV123::MRAProtocolV123(QObject *parent) :
    MRAProtocol(parent)
{
}

MRAProtocolV123::~MRAProtocolV123() {

}

bool MRAProtocolV123::makeConnection(const QString &login, const QString &password)
{
    if (!MRAProtocol::makeConnection(login, password)) {
        return false;
    }

    return true;
}

void MRAProtocolV123::sendLogin(const QString &login, const QString &password)
{
    sendUnknownBeforeLogin();

    MRAData data;

    // proto v1.23
    data.addString(login);
    data.addBinaryString(QCryptographicHash::hash(password.toAscii(), QCryptographicHash::Md5) );
    data.addInt32(0x00000bff);
    data.addString("client=\"magent\" version=\"5.10\" build=\"5282\"");
    data.addString("ru");
    data.addInt32(0x10);
    data.addInt32(0x01);
    data.addString("geo-list");
    data.addString("MRA 5.10 (build 5282);");

    connection()->sendMsg(MRIM_CS_LOGIN3, &data);
}

void MRAProtocolV123::readLoginAck(MRAData & data) {
    Q_UNUSED(data);
    setStatus(ONLINE);
    emit connected();
}

void MRAProtocolV123::sendUnknownBeforeLogin() {
    MRAData data;
    data.addInt32(0x03);
    data.addInt32(0x00);
    data.addInt32(0x00);
    data.addInt32(0x01);
    data.addInt32(0x00);
    data.addInt32(0x02);
    data.addInt32(0x00);

    connection()->sendMsg(MRIM_CS_UNKNOWN2, &data);

}

void MRAProtocolV123::readMessage(MRAData & data) {
    // UL msg_id
    // UL flags
    // LPS from
    // LPS message
    // LPS rtf-formatted message (>=1.1)

    int msg_id = data.getInt32();
    int flags  = data.getInt32();
    QString from = data.getString();
    QString text;
    if (flags & MESSAGE_FLAG_UNICODE) {
        if (flags & MESSAGE_FLAG_AUTHORIZE) {
            // QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");

            MRAData authMessage( QByteArray::fromBase64(data.getString().toAscii()) );
            // text = codec->toUnicode(  );
            authMessage.getInt32(); // 0x02 // ???
            kWarning() << authMessage.getUnicodeString();// WTF? sender?
            text = authMessage.getUnicodeString();

        } else {
            text = data.getUnicodeString();
        }
    } else {
        text = data.getString();
    }
    if ( (flags & MESSAGE_FLAG_RTF) && !data.eof() ) {
        QString rtf  = data.getString(); // ignore
        Q_UNUSED(rtf);
    }

    if ( (flags & MESSAGE_FLAG_NOTIFY) != 0 ) {
        emit typingAMessage( from );
    } else if ( (flags & MESSAGE_FLAG_AUTHORIZE) != 0) {
        emit authorizeRequestReceived(from, text);
    } else {
        if ( (flags & MESSAGE_FLAG_CHAT) && !data.eof() ) {
            data.getInt32(); // ??
            data.getInt32(); // ??
            QString chatTitle  = data.getUnicodeString(); // subject
            QString chatMember = data.getString();        // sender

            text = chatTitle + '(' + chatMember + ')' + '\n' + text;
        }

        emit messageReceived( from, text );
    }

    if ( (flags & MESSAGE_FLAG_NORECV) == 0 ) {

        MRAData ackData;

        ackData.addString(from); // LPS ## from ##
        ackData.addInt32(msg_id); // UL ## msg_id ##

        connection()->sendMsg(MRIM_CS_MESSAGE_RECV, &ackData);
    }
}
/*!
    \fn MRAMsg::sendMessage()
 */
void MRAProtocolV123::sendText(const QString &to, const QString &text)
{
    MRAData data;

    unsigned long int flags = MESSAGE_FLAG_NORECV;

    data.addInt32(flags);
    data.addString(to);
    data.addUnicodeString(text);
    data.addString(" ");// RTF is not supported yet

    connection()->sendMsg(MRIM_CS_MESSAGE, &data);
}



void MRAProtocolV123::setStatus(STATUS status) {
    MRAData data;

    data.addInt32( statusToInt(status) );
    if (status == ONLINE) {
        data.addString("STATUS_ONLINE");
        data.addUnicodeString(tr("Online")); /// @todo make phrases configurable
    } else if (status == AWAY) {
        data.addString("STATUS_AWAY");
        data.addUnicodeString(tr("Away"));
    } else if (status == DONT_DISTRUB) {
        data.addString("STATUS_DND");
        data.addUnicodeString(tr("Don't distrub"));
    } else if (status == CHATTY) {
        data.addString("STATUS_CHAT");
        data.addUnicodeString(tr("Ready to talk"));
    } else {
        /// @fixme
        data.addString("STATUS_ONLINE");
        data.addUnicodeString(tr("Online"));
    }

    data.addInt32(0x00); // user's client?
    data.addInt32(0x00000BFF);

    connection()->sendMsg(MRIM_CS_CHANGE_STATUS, &data);
}

void MRAProtocolV123::readUserSataus(MRAData & data) {

    // data.dumpData();

    int status  = data.getInt32();

    QString statusTitle = data.getString(); // STATUS_ONLINE
    QString str         = data.getUnicodeString(); // tr("Online")
    int int1            = data.getInt32(); // ???

    QString user        = data.getString();

    int int2            = data.getInt32(); // 0x00000BFF

    QString client      = data.getString(); // client="magent" version="5.10" build="5309"

    kWarning() <<status<< statusTitle << str << int1 << user << int2 << client;

    emit userStatusChanged(user, status);
}



void MRAProtocolV123::readAnketaInfo(MRAData & data) {

    MRAContactInfo info;

    uint status     = data.getInt32();
    kWarning() << "status=" << status;
    uint fields_num = data.getInt32();
    uint max_rows   = data.getInt32();
    uint server_time= data.getInt32();
    Q_UNUSED(max_rows); /// @fixme: use this fields
    Q_UNUSED(server_time); /// @fixme: use this fields

    QVector<QString> vecInfo;
    vecInfo.reserve(fields_num);

    for( uint i = 0; i < fields_num; ++i ) {
        QString field = data.getString();
        kWarning() << field;
        vecInfo.append( field );
    }

    for( uint i = 0; i < fields_num; ++i ) {


        QString fieldData;
        bool isUnicodeAttribute =
            vecInfo[i] == "Location" ||
            vecInfo[i] == "Nickname" ||
            vecInfo[i] == "FirstName" ||
            vecInfo[i] == "LastName" ||
            vecInfo[i] == "status_title";

        if (isUnicodeAttribute) {
            fieldData = data.getUnicodeString();
        } else {
            fieldData = data.getString();
        }

        info.setParam(vecInfo[i], fieldData);
        kWarning() << vecInfo[i] << fieldData;

    }

    this->emit userInfoLoaded( info.email(), info );
}

/* void MRAProtocolV123::authorizeContact(const QString &contact) {
    Q_UNUSED(contact);
} */

void MRAProtocolV123::addToContactList(int flags, int groupId, const QString &address, const QString &nick, const QString &myAddress, const QString &authMessage) {
/*
#define MRIM_CS_ADD_CONTACT			0x1019	// C -> S
    // added by negram. since v1.23:
    //
    // UL flags (group(2) or usual(0)
    // UL group id (unused if contact is group)
    // LPS contact
    // LPS name (unicode)
    // LPS unused
    // LPS authorization message, 'please, authorize me': base64(unicode(message))
    // UL ??? (0x00000001)
*/
    MRAData addData;

    addData.addInt32(flags);
    addData.addInt32(groupId);
    addData.addString(address);
    addData.addUnicodeString(nick);
    addData.addString(""); // unused

    MRAData authMessageData;

    authMessageData.addInt32(0x02); // ???
    authMessageData.addUnicodeString(myAddress);
    authMessageData.addUnicodeString(authMessage);


    addData.addString( authMessageData.toBase64() );

    addData.addInt32( 1 );

    connection()->sendMsg(MRIM_CS_ADD_CONTACT, &addData);

}

void MRAProtocolV123::sendAuthorizationRequest(const QString &contact, const QString &myAddress, const QString &message) {
    MRAData authData;
    unsigned long int authFlags = MESSAGE_FLAG_NORECV | MESSAGE_FLAG_AUTHORIZE | MESSAGE_FLAG_UNICODE;
    authData.addInt32(authFlags);
    authData.addString(contact);

    MRAData authMessage;

    authMessage.addInt32(0x02); // ???
    authMessage.addUnicodeString(myAddress);
    authMessage.addUnicodeString(message);

    authData.addString(authMessage.toBase64());
    authData.addString("");// RTF is not supported yet

    connection()->sendMsg(MRIM_CS_MESSAGE, &authData);

}

void MRAProtocolV123::deleteContact(uint id, const QString &contact, const QString &contactName) {
    kWarning() << __PRETTY_FUNCTION__;
/*

#define MRIM_CS_MODIFY_CONTACT			0x101B	// C -> S
    // UL id
    // UL flags - same as for MRIM_CS_ADD_CONTACT
    // UL group id (unused if contact is group)
    // LPS contact
    // LPS name
    // LPS unused
    */
    MRAData data;

    data.addInt32( id );
    data.addInt32( CONTACT_FLAG_REMOVED | CONTACT_FLAG_UNKNOWN );
    data.addInt32( 0 ); // don't care about group
    data.addString( contact );
    data.addUnicodeString( contactName );
    data.addString( QString() );

    connection()->sendMsg( MRIM_CS_MODIFY_CONTACT, &data );
}

QVector<QVariant> MRAProtocolV123::readVectorByMask(MRAData & data, const QString &mask)
{
    QVector<QVariant> result;

    quint32 _int;
    QString _string;
    QString localMask = mask;
    if (localMask.length() > 5) {
        // user's mask
        localMask[3] = 'S';
        localMask[8] = 'S';
        localMask[15] = 'S';
    } else {
        // group's mask
        localMask[1] = 'S';
    }

    for (int k = 0; k < localMask.length(); ++k) {
        if (localMask[k] == 'u') {
            _int = data.getInt32();
            kWarning() << "u=" << _int;
            result.push_back(_int);
        } else if (localMask[k] == 's') {
            _string = data.getString( );
            kWarning() << "s=" << _string;
            result.push_back(_string);
        } else if (localMask[k] == 'S') {
            _string = data.getUnicodeString( );
            kWarning() << "S=" << _string;
            result.push_back(_string);
        }
    }
    kWarning() << "done";
    return result;
}


void MRAProtocolV123::fillUserInfo(QVector<QVariant> &protoData, MRAContactListEntry &item) {
    item.setFlags(   protoData[0].toUInt() );
    item.setGroup(   protoData[1].toUInt() );
    item.setAddress( protoData[2].toString() );
    item.setNick(    protoData[3].toString() );
    item.setServerFlags( protoData[4].toUInt() );
    item.setStatus(  protoData[5].toUInt() );
}

#include "mraprotocolv123.moc"
