#include <kdebug.h>
#include <QCryptographicHash>
#include <QTextCodec>
#include <QByteArray>
#include <QApplication>

#include "../debug.h"
#include "mradata.h"
#include "mraconnection.h"
#include "mracontactlist.h"
#include "mracontactinfo.h"
#include "mraprotocolv123.h"
#include "transferrequestinfo.h"
#include "filetransferinfo.h"
#include "transfermanager.h"
#include "mraconferencesettings.h"
#include "../version.h"

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
    data.addUint32(0x00000bff);
    data.addString("client=\"kopete mrim plugin\" version=\"" + kopeteMrimVersion() + "\" build=\"5282\"");
    data.addString("ru");
    data.addUint32(0x10);
    data.addUint32(0x01);
    data.addString("geo-list");
    // data.addString("MRA 5.10 (build 5282);");
    data.addString("Kopete MRIM plugin (v" + kopeteMrimVersion() + ");");

    sendMsg(MRIM_CS_LOGIN3, &data);
}

void MRAProtocolV123::readLoginAck(MRAData & data) {
    Q_UNUSED(data);
    setStatus(ONLINE);
    emit connected();
}

void MRAProtocolV123::sendUnknownBeforeLogin() {
    MRAData data;
    data.addUint32(0x03);
    data.addUint32(0x00);
    data.addUint32(0x00);
    data.addUint32(0x01);
    data.addUint32(0x00);
    data.addUint32(0x02);
    data.addUint32(0x00);

    sendMsg(MRIM_CS_UNKNOWN2, &data);

}

void MRAProtocolV123::readMessage(MRAData & data) {
    // UL msg_id
    // UL flags
    // LPS from
    // LPS message
    // LPS rtf-formatted message (>=1.1)

    int msg_id = data.getUint32();
    int flags  = data.getUint32();
    QString from = data.getString();
    QString text;

    bool msgContainsUnicode = (flags & MESSAGE_FLAG_UNICODE) &&
                                !(flags & MESSAGE_FLAG_UNKNOWN);
    if (msgContainsUnicode) {
        if (flags & MESSAGE_FLAG_AUTHORIZE) {
            // QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");

            MRAData authMessage( QByteArray::fromBase64(data.getString().toAscii()) );
            // text = codec->toUnicode(  );
            authMessage.getUint32(); // 0x02 // ???
            mrimDebug() << authMessage.getUnicodeString();// WTF? sender?
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

    bool isSystemMessage = (flags & MESSAGE_FLAG_SYSTEM) != 0;
    /*
    messageType = 64 chatMessageType= 0: message
    messageType = 350 chatMessageType= 2: members' list
    messageType = 96 chatMessageType= 3: user invited smb to the chat
    messageType = 64 chatMessageType= 5: user has deleted itself from the chat
    messageType = 61 chatMessageType= 7: you have been invited to a chat
    messageType = 61 chatMessageType= 9: you have been kicked off from the chat

    */

    const int CHAT_MESSAGE_TEXT = 0;
    const int CHAT_MESSAGE_PARTICIPANTS_LIST = 2;
    const int CHAT_MESSAGE_USER_INVITED = 3;
    const int CHAT_MESSAGE_USER_DELETED_ITSELF = 5;
    const int CHAT_MESSAGE_YOU_HAVE_BEEN_INVITED = 7;
    const int CHAT_MESSAGE_YOU_HAVE_BEEN_KICKED_OFF = 9;

    if ( (flags & MESSAGE_FLAG_NOTIFY) != 0 ) {
        emit typingAMessage( from );
    } else if ( (flags & MESSAGE_FLAG_AUTHORIZE) != 0) {
        emit authorizeRequestReceived(from, text);
    } else {
        if ( (flags & MESSAGE_FLAG_CHAT) && !data.eof() ) {

            //  0x3d  0b00111101 -- old client (non unicode message)
            //  0x3b  0b00111011 -- normal chat message
            //  0x42  0b01000010 -- chat message (chat created by mac agent)
            //  0x43  0b01000011 -- chat message (chat created by mac agent)
            //  0x2d  0b00101101 -- ???
            //  0x35  0b00110101 -- ???

            //  0x53  0b01010011 -- chat list members
            // 0x120 0b100100000 -- updated (?) members list
            // 0x12c 0b100101100 -- chat list members, chat created by mac agent
            // 0x142 0b101000010 -- chat list members, chat created by mac agent (again!)

            int messageType = data.getUint32(); // 0x3b,0x3d ??
            int chatMessageType = data.getUint32(); // 0x00 ??
            mrimDebug() << "messageType =" << messageType << "chatMessageType="<<chatMessageType;

            if ( chatMessageType == CHAT_MESSAGE_PARTICIPANTS_LIST ) {

                isSystemMessage = true;
                receiveChatMembersList(data, from);

            } else if ( chatMessageType == CHAT_MESSAGE_YOU_HAVE_BEEN_INVITED ) {

                isSystemMessage = true;
                receiveChatInvitation(data, from);

            } else if ( chatMessageType == CHAT_MESSAGE_TEXT ) {
                mrimDebug() << "chatMessageType=" << messageType << "from=" <<from;

                QString chatTitle  = data.getUnicodeString(); // subject
                QString chatMember = data.getString();        // sender

                text = chatTitle + '(' + chatMember + ')' + '\n' + text;

                mrimDebug() << "chatMessageType=" << messageType << "from=" <<from << "sender=" << chatMember;
            } else if ( chatMessageType == CHAT_MESSAGE_USER_INVITED ) {
                // TODO
            } else if ( chatMessageType == CHAT_MESSAGE_USER_DELETED_ITSELF ) {
                // TODO
            } else if ( chatMessageType == CHAT_MESSAGE_YOU_HAVE_BEEN_KICKED_OFF ) {
                // TODO
            } else {
                mrimDebug() << "unknown messageType =" << messageType;
            }
        }

        if ( not isSystemMessage ) {
            emit messageReceived( from, text );
        }
    }

    if ( (flags & MESSAGE_FLAG_NORECV) == 0 ) {

        MRAData ackData;

        ackData.addString(from); // LPS ## from ##
        ackData.addUint32(msg_id); // UL ## msg_id ##

        sendMsg(MRIM_CS_MESSAGE_RECV, &ackData);
    }
}

bool MRAProtocolV123::isMemberListOfChat(int chatMessageType) {
    //  0x53  0b01010011 -- chat list members
    // 0x105 0b100000101 -- the same
    // 0x106 0b100000110 -- the same
    // 0x120 0b100100000 -- updated (?) members list
    // 0x12c 0b100101100 -- chat list members, chat created by mac agent
    // 0x13c 0b100111100-- chat list members
    // 0x142 0b101000010 -- chat list members, chat created by mac agent (again!)
    // 0x144
    // 0x154
    // 0x157
    // 0x15e 0b101011110
    return (chatMessageType == 0x53 ) ||
           (chatMessageType == 0x105) ||
           (chatMessageType == 0x106) ||
           (chatMessageType == 0x11f) ||
           (chatMessageType == 0x120) ||
           (chatMessageType == 0x12c) ||
           (chatMessageType == 0x13c) ||
           (chatMessageType == 0x142) ||
           (chatMessageType == 0x144) ||
           (chatMessageType == 0x154) ||
           (chatMessageType == 0x157) ||
           (chatMessageType == 0x15e);
}

bool MRAProtocolV123::isChatTextMessage(int chatMessageType) {
    const int CHAT_TEXT_MESSAGE = 0x0028;
    //  0x28  0b00101000
    //  0x41  0b01000001 -- chat message (chat created by win agent to chat that was created by a MAC agent)
    //  0x42  0b01000010 -- chat message (chat created by mac agent)
    //  0x43  0b01000011 -- chat message (chat created by mac agent)
    return (chatMessageType & CHAT_TEXT_MESSAGE) ||
            (chatMessageType == 0x40) || // from Kopete
            (chatMessageType == 0x41) ||
            (chatMessageType == 0x42) ||
            (chatMessageType == 0x43) ||
            (chatMessageType == 0x45) ||
            (chatMessageType == 0x47) ||
            (chatMessageType == 0x50) ||
            (chatMessageType == 0x54) ||
            (chatMessageType == 0x56) ||
            (chatMessageType == 0x57)
            ;
}

void MRAProtocolV123::receiveChatMembersList(MRAData & data, const QString &from) {

    QString chatTitle  = data.getUnicodeString(); // subject
    int i3 = data.getUint32();

    int numMembers = data.getUint32();

    QList<QString> membersList;

    for(; numMembers > 0; --numMembers) {
        membersList.append( data.getString() );
    }

    emit chatMembersListReceived(from, chatTitle, membersList);

    Q_UNUSED(i3);

}

void MRAProtocolV123::createChat(const MRAConferenceSettings &conferenceSettings) {

    MRAData data;

    unsigned long int flags = CONTACT_FLAG_CHAT;

    data.addUint32(flags);
    data.addUint32(0); // group id
    data.addString(""); // contact, will be generated by server
    data.addUnicodeString( conferenceSettings.title() );
    data.addString(""); // unused
    data.addString(""); // data (?)
    data.addUint32(0); // unknown parameter

    MRAData chatData;
        MRAData listData;

            QStringList membersList = conferenceSettings.contactList();
            listData.addUint32(membersList.length()); // members list length

            foreach (const QString &c, membersList) {
                listData.addString(c);
            }

        chatData.addBinaryString( listData.toByteArray() );

    data.addBinaryString( chatData.toByteArray() );


    if ( membersList.empty() ) {
        mrimWarning() << "members list is empty";
        return;
    }

    sendMsg(MRIM_CS_ADD_CONTACT, &data);

}

bool MRAProtocolV123::isYouHaveBeenAddedToTheChat(int chatMessageType) {
    return (chatMessageType == 0x07);
}

void MRAProtocolV123::receiveChatInvitation(MRAData & data, const QString &from) {
    QString chatTitle = data.getUnicodeString(); // subject
    QString whoAdd    = data.getString();

    emit chatIvitationReceived(from, chatTitle, whoAdd);
}


void MRAProtocolV123::sendText(const QString &to, const QString &text)
{
    MRAData data;

    unsigned long int flags = MESSAGE_FLAG_NORECV;

    data.addUint32(flags);
    data.addString(to);
    data.addUnicodeString(text);
    data.addString(" ");// RTF is not supported yet

    sendMsg(MRIM_CS_MESSAGE, &data);
}

void MRAProtocolV123::loadChatMembersList(const QString &to) {
    MRAData data;
    unsigned long int flags = MESSAGE_FLAG_RTF;
    data.addUint32(flags);
    data.addString(to);
    data.addString("");
    data.addString("");
    data.addUint32(0x04); // whatis 4?
    data.addUint32(0x01); // whatis 1?

    sendMsg(MRIM_CS_MESSAGE, &data);

    /*
      ack: flags: 0x00400084 = MESSAGE_FLAG_NORECV | MESSAGE_FLAG_RTF | MESSAGE_FLAG_CHAT

      chat int 1: 0x53
      chat int 2: 0x02
      lps chatTitle
      chat int 3: 0x2d
      chat int 4: 0x02 (members number?)
      lps*number

      */

}

void MRAProtocolV123::inviteMemberToChat(const QString &to, const QString &contactIdToInvite) {
    MRAData data;
    data.addUint32(  0x80 ); // TODO: move to contstants
    data.addString(to);
    data.addString("");
    data.addString("");

    MRAData chatData;
        chatData.addUint32(3); // TODO: 3 - add to the list, move to constants
        MRAData listData;
            listData.addUint32(1); // members list size
            listData.addString(contactIdToInvite);

        chatData.addBinaryString( listData.toByteArray() );

    data.addBinaryString( chatData.toByteArray() );

    sendMsg(MRIM_CS_MESSAGE, &data);
}

void MRAProtocolV123::removeMemberFromChat(const QString &to, const QString &contactIdToRemove) {
    MRAData data;
    data.addUint32(  0x80 ); // TODO: move to contstants
    data.addString(to);
    data.addString("");
    data.addString("");

    MRAData chatData;
        chatData.addUint32(8); // TODO: 8 - remove from the list, move to constants
        MRAData listData;
            listData.addUint32(1); // members list size
            listData.addString(contactIdToRemove);

        chatData.addBinaryString( listData.toByteArray() );

    data.addBinaryString( chatData.toByteArray() );

    sendMsg(MRIM_CS_MESSAGE, &data);

}

void MRAProtocolV123::setStatus(STATUS status) {
    MRAData data;

    data.addUint32( statusToInt(status) );
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

    data.addUint32(0x00); // user's client?
    data.addUint32(0x00000BFF);

    sendMsg(MRIM_CS_CHANGE_STATUS, &data);
}

void MRAProtocolV123::readUserSataus(MRAData & data) {

    // data.dumpData();

    int status  = data.getUint32();

    QString statusTitle = data.getString(); // STATUS_ONLINE
    QString str         = data.getUnicodeString(); // tr("Online")
    int int1            = data.getUint32(); // ???

    QString user        = data.getString();

    int int2            = data.getUint32(); // 0x00000BFF

    QString client      = data.getString(); // client="magent" version="5.10" build="5309"

    mrimDebug() <<status<< statusTitle << str << int1 << user << int2 << client;

    emit userStatusChanged(user, status);
}



void MRAProtocolV123::readAnketaInfo(MRAData & data) {

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
        mrimDebug() << vecInfo[i] << fieldData;

    }

    emit userInfoLoaded( info.email(), info );
}

/* void MRAProtocolV123::authorizeContact(const QString &contact) {
    Q_UNUSED(contact);
} */

void MRAProtocolV123::addToContactList(int flags, int groupId, const QString &address, const QString &nick, const QString &myAddress, const QString &authMessage, IMRAProtocolContactReceiver *contactAddReceiver) {
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

    addData.addUint32(flags);
    addData.addUint32(groupId);
    addData.addString(address);
    addData.addUnicodeString(nick);
    addData.addString(""); // unused

    MRAData authMessageData;

    authMessageData.addUint32(0x02); // ???
    authMessageData.addUnicodeString(myAddress);
    authMessageData.addUnicodeString(authMessage);


    addData.addString( authMessageData.toBase64() );

    addData.addUint32( 1 );

    sendMsg(MRIM_CS_ADD_CONTACT, &addData);

    setContactReceiver(contactAddReceiver);

}

void MRAProtocolV123::addGroupToContactList(const QString &groupName, IMRAProtocolGroupReceiver *groupAddedReveiver) {
    // UL flags (group(2) or usual(0)  = CONTACT_FLAG_GROUP
    // UL group id (unused if contact is group) = 0
    // LPS contact = groupName
    // LPS name (unicode) = groupName
    // LPS unused
    // LPS authorization message, 'please, authorize me': base64(unicode(message))
    // UL ??? (0x00000001)
    int CONTACT_FLAG_UNICODE_GROUP = 0x05000000;
    MRAData addData;
    addData.addUint32( CONTACT_FLAG_GROUP | CONTACT_FLAG_UNICODE_GROUP );
    addData.addUint32(0);
    addData.addString(""); // ??? unicode?
    addData.addUnicodeString(groupName);
    addData.addString(""); // unused

    MRAData authMessageData;

    authMessageData.addUint32(0x02); // ???
    authMessageData.addUnicodeString("");
    authMessageData.addUnicodeString("");

    addData.addString( authMessageData.toBase64() );

    addData.addUint32( 0 );

    sendMsg(MRIM_CS_ADD_CONTACT, &addData);

    setGroupReceiver(groupAddedReveiver);

}

void MRAProtocolV123::sendAuthorizationRequest(const QString &contact, const QString &myAddress, const QString &message) {
    MRAData authData;
    unsigned long int authFlags = MESSAGE_FLAG_NORECV | MESSAGE_FLAG_AUTHORIZE | MESSAGE_FLAG_UNICODE;
    authData.addUint32(authFlags);
    authData.addString(contact);

    MRAData authMessage;

    authMessage.addUint32(0x02); // ???
    authMessage.addUnicodeString(myAddress);
    authMessage.addUnicodeString(message);

    authData.addString(authMessage.toBase64());
    authData.addString("");// RTF is not supported yet

    sendMsg(MRIM_CS_MESSAGE, &authData);

}

void MRAProtocolV123::readUserInfo(MRAData & data)
{
    QString val;
    while (!data.eof()) {
        QString str = data.getString();
        if (str == "MRIM.NICKNAME" || str == "connect.xml") {
            val = data.getUnicodeString();
        } else {
            val = data.getString();
        }
        mrimDebug() << str << " " << val;
    }

}


void MRAProtocolV123::deleteContact(uint id, const QString &contact, const QString &contactName) {
    MRAData data;

    data.addUint32( id );
    data.addUint32( CONTACT_FLAG_REMOVED | CONTACT_FLAG_UNKNOWN );
    data.addUint32( 0 ); // don't care about group
    data.addString( contact );
    data.addUnicodeString( contactName );
    data.addString( QString() );

    sendMsg( MRIM_CS_MODIFY_CONTACT, &data );
}

void MRAProtocolV123::editContact(uint id, const QString &contact, uint groupId, const QString &newContactName) {
    MRAData data;

    data.addUint32( id );
    data.addUint32( CONTACT_FLAG_UNKNOWN );
    data.addUint32( groupId );
    data.addString( contact );
    data.addUnicodeString( newContactName );
    data.addString( QString() );

    sendMsg( MRIM_CS_MODIFY_CONTACT, &data );
}

void MRAProtocolV123::addTransferSession(qtmra::IFileTransferInfo *transferReceiver) {
    transferManager()
            .addSession(transferReceiver);
}

void MRAProtocolV123::startFileTransfer(qtmra::IFileTransferInfo *transferReceiver) {

    transferManager().addSession(transferReceiver);

    MRAData data;
    data.addString(transferReceiver->getContact());
    data.addUint32(transferReceiver->getSessionId());
    data.addUint32(transferReceiver->getFilesSize());
    // data.addInt32(121); // WTF?

    MRAData filesInfo;

    QString fileAndSize = buildFilesListString(transferReceiver);

        filesInfo.addString(fileAndSize);

        // data.addString(fileAndSize);

        MRAData filesDescription;
            filesDescription.addUint32(1); // files count;
            filesDescription.addUnicodeString(fileAndSize);

        filesInfo.addBinaryString(filesDescription.toByteArray());

        filesInfo.addString(transferReceiver->getHostAndPort());
        data.addBinaryString(filesInfo.toByteArray());

    sendMsg(MRIM_CS_TRANSFER_REQUEST, &data);
}

QString MRAProtocolV123::buildFilesListString(qtmra::IFileTransferInfo *transferReceiver) {
    QString fileAndSize;
    typedef QPair<QString, quint64> list_item;
    QList<list_item> filesList = transferReceiver->getFiles();
    foreach (const list_item &item, filesList) {

        fileAndSize +=
                      item.first + ';'
                    + QString::number(item.second) + ';'
                ;

    }

    return fileAndSize;
}

void MRAProtocolV123::finishFileTransfer(qtmra::IFileTransferInfo *transferReceiver) {
    //
    MRAData data;
    data.addUint32(transferReceiver->getSessionId());
    data.addUint32(0x00009800);
    data.addString(transferReceiver->getAccountId());

    sendMsg(MRIM_CS_TRANSFER_SUCCEED, &data);

    transferManager()
            .removeSession(transferReceiver->getContact(), transferReceiver->getSessionId());
}

void MRAProtocolV123::cancelFileTransfer(qtmra::IFileTransferInfo *transferReceiver) {

    MRAData data;
    data.addUint32(0); //status (?)
    data.addString(transferReceiver->getContact());
    data.addUint32(transferReceiver->getSessionId());
    data.addUint32(0); // addString("") ?

    sendMsg(MRIM_CS_TRANSFER_CANCEL, &data);

    transferManager()
            .removeSession(transferReceiver->getContact(), transferReceiver->getSessionId());

}

void MRAProtocolV123::readTransferCancel(MRAData &data) {
    TransferRequestInfo requestInfo;

    int status = data.getUint32(); // status(?)

    requestInfo.setRemoteContact(data.getString());
    requestInfo.setSessionId(data.getUint32());
    QString proxy = data.getString(); // ?

    if (not transferManager().hasSession(requestInfo.remoteContact(), requestInfo.sessionId())) {
        return;
    }

    if (status == 0) { // cancel
        transferManager()
                .session(requestInfo.remoteContact(), requestInfo.sessionId())
                ->cancel();
    } else if (status == 4) { // try to connect to me
        transferManager()
                .session(requestInfo.remoteContact(), requestInfo.sessionId())
                ->tryThisHost(proxy);
        return; // don't remove session
    } else {
        mrimWarning() << "unknown status" << status;
    }

    transferManager()
            .removeSession(requestInfo.remoteContact(), requestInfo.sessionId());

    // emit transferRequestCancelled(requestInfo);
}

void MRAProtocolV123::readTransferRequest(MRAData & data) {
    TransferRequestInfo requestInfo;
    requestInfo.setRemoteContact(data.getString());
    requestInfo.setSessionId(data.getUint32());
    requestInfo.setTotalSize(data.getUint32()); // total size

    MRAData filesInfo(data.getBinaryString());
        filesInfo.getString(); //cp1251 filename
        MRAData filesDescription(filesInfo.getBinaryString());

        (void)filesDescription.getUint32(); // files count (?)
        requestInfo.setFilesString(filesDescription.getUnicodeString());

        requestInfo.setHostsAndPortsString(filesInfo.getString());

    /// @todo: check session
    emit transferRequest(requestInfo);

}

void MRAProtocolV123::sendTransferCantLocal(qtmra::IFileTransferInfo *transferReceiver) {
    MRAData data;

    data.addString(transferReceiver->getContact());
    data.addUint32(transferReceiver->getSessionId());
    data.addUint32(2); // ???
    data.addString(buildFilesListString(transferReceiver));
    data.addString(""); // proxy
    data.addData(QByteArray(16, '\0')); // ssl key
        MRAData filesInfo;
        filesInfo.addUint32(2); // ???
        filesInfo.addUnicodeString(buildFilesListString(transferReceiver));
        filesInfo.addUint32(4); // LPS containing MRAData with files' number?
        filesInfo.addUint32(1);

    data.addBinaryString(filesInfo.toByteArray());

    sendMsg(MRIM_CS_TRANSFER_CANT_LOCAL, &data);

}

void MRAProtocolV123::readTransferCantLocal(MRAData &data) {
    QString remoteUser = data.getString();
    int sessionId = data.getUint32();
    int status = data.getUint32();// 2 // ???
    QByteArray files = data.getBinaryString(); // files list
    QString proxies = data.getString();

    // proxies = proxies.mid(proxies.indexOf(';')+1);

    if (not transferManager().hasSession(remoteUser, sessionId)) {
        mrimWarning() << "cant find session" << sessionId << "to user " << remoteUser;
        return;
    }

    MRAData sendProxy;

    sendProxy.addUint32(1); // ??? files number?
    sendProxy.addString(remoteUser);
    sendProxy.addUint32(sessionId);
    sendProxy.addUint32(status);
    sendProxy.addBinaryString(files);
    sendProxy.addString(proxies);

    // SSL key
    QByteArray proxyKey = data.getNBytes(16);
    //sendProxy.addInt32(data.getInt32());
    //sendProxy.addInt32(data.getInt32());
    //sendProxy.addInt32(data.getInt32());
    //sendProxy.addInt32(data.getInt32());

    sendProxy.addNBytes(16, proxyKey);
    // unicode files description
    sendProxy.addBinaryString(data.getBinaryString());

    sendMsg(MRIM_CS_TRANSFER_USE_THIS_PROXY, &sendProxy);

    QApplication::processEvents();

    transferManager()
            .session(remoteUser, sessionId)
            ->useThisProxy(proxies, proxyKey);



}

void MRAProtocolV123::sendTryThisHost(qtmra::IFileTransferInfo *transferReceiver) {
    MRAData data;
    data.addUint32(4); // "try this host"
    data.addString(transferReceiver->getContact());
    data.addUint32(transferReceiver->getSessionId());
    data.addString(transferReceiver->getHostAndPort());

    sendMsg(MRIM_CS_TRANSFER_CANCEL, &data);
}

void MRAProtocolV123::readTransferUseThisProxy(MRAData &data) {
    data.getUint32(); // 1 // ??? files number?

    QString remoteUser = data.getString(); // remote user
    int sessionId = data.getUint32(); // session id

    data.getUint32(); // 2 // ???

    data.getString(); // cp1251 filenames

    QString proxy =
            data.getString(); // proxy
    // ssl key - 16 bytes
    QByteArray proxyKey = data.getNBytes(16);
    // data.getInt32(); data.getInt32(); data.getInt32(); data.getInt32();

    MRAData filesInfo(data.getBinaryString());
        filesInfo.getUint32(); // files num
        filesInfo.getUnicodeString(); // filenames
        filesInfo.getUint32(); // 4 // ???
        filesInfo.getUint32(); // 1 // ???

        transferManager()
                .session(remoteUser, sessionId)
                ->useThisProxy(proxy, proxyKey);
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
            _int = data.getUint32();
            mrimDebug() << "u=" << _int;
            result.push_back(_int);
        } else if (localMask[k] == 's') {
            _string = data.getString( );
            mrimDebug() << "s=" << _string;
            result.push_back(_string);
        } else if (localMask[k] == 'S') {
            _string = data.getUnicodeString( );
            mrimDebug() << "S=" << _string;
            result.push_back(_string);
        }
    }
    mrimDebug() << "done";
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
