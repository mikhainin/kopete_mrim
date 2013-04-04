#ifndef MRAPROTOCOLV123_H
#define MRAPROTOCOLV123_H

#include "mraprotocol.h"
#include <QVector>
#include <QPair>

class MRAContactListEntry;

class IFileTransferInfo {
public:
    /*FileTransferInfo(
                  const QString &contact
                , int sessionId
                , const QString &filepath
            );
            */
    virtual ~IFileTransferInfo() {}
    virtual QString getContact() = 0;
    virtual QList<QPair<QString, int> > getFiles() = 0;
    virtual int getFilesSize() = 0;
    virtual int getSessionId() = 0;
    virtual QString getHostAndPort() = 0;
    virtual QString getAccountId() = 0;

};

class MRAProtocolV123 : public MRAProtocol
{
    Q_OBJECT
public:
    explicit MRAProtocolV123(QObject *parent = 0);
    virtual ~MRAProtocolV123();

    virtual bool makeConnection(const QString &login, const QString &password);

    virtual void sendLogin(const QString &login, const QString &password);

    virtual void readLoginAck(MRAData & data);

    virtual void sendUnknownBeforeLogin();

    virtual void setStatus(STATUS status);

    virtual void sendText(const QString &to, const QString &text);

    virtual void deleteContact(uint id, const QString &contact, const QString &contactName);

    void editContact(uint id, const QString &contact, uint groupId, const QString &newContactName);

    virtual void addToContactList(int flags, int groupId, const QString &address, const QString &nick, const QString &myAddress, const QString &authMessage, IMRAProtocolContactReceiver *contactAddReceiver);

    virtual void sendAuthorizationRequest(const QString &contact, const QString &myAddress, const QString &message);

    virtual void readUserInfo(MRAData & data);

    virtual void loadChatMembersList(const QString &to);

    virtual void addGroupToContactList(const QString &groupName, IMRAProtocolGroupReceiver *groupAddedReveiver);

    virtual void startFileTransfer(IFileTransferInfo *transferReceiver);
    virtual void finishFileTransfer(IFileTransferInfo *transferReceiver);
    virtual void cancelFileTransfer(IFileTransferInfo *transferReceiver);

protected:
    virtual void readMessage(MRAData & data);
    virtual void readUserSataus(MRAData & data);
    virtual void readAnketaInfo(MRAData & data);
    virtual void readTransferRequest(MRAData & data);
    virtual void readTransferCancel(MRAData &data);

    virtual QVector<QVariant> readVectorByMask(MRAData & data, const QString &mask);

    virtual void fillUserInfo(QVector<QVariant> &protoData, MRAContactListEntry &item);
signals:

public slots:

private:
    bool isMemberListOfChat(int chatMessageType);
    bool isChatTextMessage(int chatMessageType);
    void receiveChatMembersList(MRAData & data, const QString &from);

    bool isYouHaveBeenAddedToTheChat(int chatMessageType);
    void receiveChatInvitation(MRAData & data, const QString &from);
};

#endif // MRAPROTOCOLV123_H
