#ifndef MRAPROTOCOLV123_H
#define MRAPROTOCOLV123_H

#include "mraprotocol.h"
#include <QVector>

class MRAContactListEntry;

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

    virtual void addToContactList(int flags, int groupId, const QString &address, const QString &nick, const QString &myAddress, const QString &authMessage);
//    virtual void authorizeContact(const QString &contact);
    virtual void sendAuthorizationRequest(const QString &contact, const QString &myAddress, const QString &message);

protected:
    virtual void readMessage(MRAData & data);
    virtual void readUserSataus(MRAData & data);
    virtual void readAnketaInfo(MRAData & data);

    virtual QVector<QVariant> readVectorByMask(MRAData & data, const QString &mask);

    virtual void fillUserInfo(QVector<QVariant> &protoData, MRAContactListEntry &item);
signals:

public slots:

};

#endif // MRAPROTOCOLV123_H
