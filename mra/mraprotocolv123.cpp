#include <kdebug.h>
#include <QCryptographicHash>

#include "mradata.h"
#include "mraconnection.h"
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
        text = data.getUnicodeString();
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
        if (!data.eof()) {
            data.getInt32(); // ??
            data.getInt32(); // ??
            QString chatTitle  = data.getUnicodeString();
            QString chatMember = data.getString();

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

    kWarning() << statusTitle << str << int1 << user << int2 << client;

    emit userStatusChanged(user, status);
}


#include "mraprotocolv123.moc"
