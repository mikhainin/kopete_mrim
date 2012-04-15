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
    QString text = data.getUnicodeString();
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

}

#include "mraprotocolv123.moc"
