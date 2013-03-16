#ifndef ADDCONTACTTASK_H
#define ADDCONTACTTASK_H

#include <QObject>
#include "mra/mraprotocolv123.h"

class MrimAccount;
namespace Kopete {
    class MetaContact;
}

class AddContactTask :
        public QObject,
        public IMRAProtocolGroupReceiver,
        public IMRAProtocolContactReceiver
{
    Q_OBJECT
public:
    explicit AddContactTask(MrimAccount *account);

    ~AddContactTask();

    void setGroupName( const QString &groupName );
    void setEmail( const QString &email );
    void setNickName( const QString &nickName );
    void setMetaContact( Kopete::MetaContact* m );
    void run();

private:
    void groupAddedSuccessfully();
    void groupAddFailed(int status);

    void contactAddedSuccessfully();
    void contactAddFailed(int status);

signals:

public slots:

private slots:
    void slotGroupRegistred();

private:
    class Private;
    Private *d;

};

#endif // ADDCONTACTTASK_H
