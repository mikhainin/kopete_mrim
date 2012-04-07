#ifndef MRAPROTOCOL_H
#define MRAPROTOCOL_H

#include "mra_proto.h"
#include "mradata.h"
#include "mraconnection.h"
#include "mracontactlist.h"
#include "mraofflinemessage.h"

#include <QList>
#include <QPair>
#include <QString>

#include <QObject>

class MRAAvatarLoader;
class QImage;
class MRAContactInfo;

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

    void loadAvatar(const QString &contact, bool large = false, QObject *receiver = 0, const char *member = 0);
    void loadAvatarLoop();

    void loadUserInfo(const QString &contact);

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

    void readAnketaInfo(MRAData & data);

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
};

#endif
