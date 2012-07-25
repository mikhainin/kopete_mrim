#ifndef MRAPROTOCOL_H
#define MRAPROTOCOL_H

#include <QObject>

class QImage;
class QString;

class MRAContactInfo;
class MRAAvatarLoader;
class MRAData;
class MRAOfflineMessage;
class MRAContactList;
class MRAConnection;
class MRAContactListEntry;

class MRAProtocol : public QObject
{
    Q_OBJECT

public:

    enum STATUS {
        OFFLINE,
        ONLINE,
        AWAY,
        DONT_DISTRUB,
        CHATTY,
        INVISIBLE
    };

    MRAProtocol(QObject *parent = 0);

    ~MRAProtocol();

    virtual bool makeConnection(const QString &login, const QString &password);
    virtual void closeConnection();

    virtual void sendText(const QString &to, const QString &text);



    virtual void handleMessage(const ulong &msg, MRAData *data);
    virtual void addToContactList(int flags, int groupId, const QString &address, const QString &nick, const QString &myAddress, const QString &authMessage);
    virtual void authorizeContact(const QString &contact);
    virtual void sendAuthorizationRequest(const QString &contact, const QString &myAddress, const QString &message);
    virtual void removeContact(const QString &contact);

    virtual void sendTypingMessage(const QString &contact);

    virtual void loadAvatar(const QString &contact, bool large = false, QObject *receiver = 0, const char *member = 0);
    virtual void loadAvatarLoop();

    virtual void loadUserInfo(const QString &contact);

    virtual void setStatus(STATUS status);

    virtual void deleteContact(uint id, const QString &contact, const QString &contactName);

    virtual void renameContact(uint id, const QString &contact, uint groupId, const QString &newContactName);

    virtual void loadChatMembersList(const QString &to);

private:
    class MRAProtocolPrivate;
    MRAProtocolPrivate *d;

protected:
    MRAConnection *connection();
    virtual void sendHello();
    virtual void sendLogin(const QString &login, const QString &password);
    virtual void readLoginAck(MRAData & data);

    virtual QVector<QVariant> readVectorByMask(MRAData & data, const QString &mask);
    virtual void fillUserInfo(QVector<QVariant> &protoData, MRAContactListEntry &item);

    virtual void readContactList(MRAData & data);
    virtual void readUserInfo(MRAData & data);
    virtual void readMessage(MRAData & data);
    virtual void readAuthorizeAck(MRAData & data);
    virtual void readConnectionRejected(MRAData & data);
    virtual void readLogoutMessage(MRAData & data);
    virtual void readUserSataus(MRAData & data);
    virtual void readConnectionParams(MRAData & data);
    virtual void readOfflineMessage(MRAData & data);
    virtual void emitOfflineMessagesReceived();

    virtual void readAnketaInfo(MRAData & data);
    int statusToInt(STATUS status);

    virtual void readAddContactAck(MRAData & data);

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
    void userInfoLoaded(const QString &contact, const MRAContactInfo &info);

    void addContactAckReceived(int status, int contactId);

    void chatMembersListReceived(const QString &chat, const QString &title, const QList<QString> &list);
};

#endif
