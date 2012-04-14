#ifndef MRAPROTOCOLV123_H
#define MRAPROTOCOLV123_H

#include "mraprotocol.h"

class MRAProtocolV123 : public MRAProtocol
{
    Q_OBJECT
public:
    explicit MRAProtocolV123(QObject *parent = 0);
    virtual ~MRAProtocolV123();

    virtual bool makeConnection(const QString &login, const QString &password);

    virtual void sendLogin(const QString &login, const QString &password);

    virtual void sendUnknownBeforeLogin();

protected:
    virtual void readMessage(MRAData & data);
signals:

public slots:

};

#endif // MRAPROTOCOLV123_H
