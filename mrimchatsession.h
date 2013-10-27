#ifndef MRIMCHATSESSION_H
#define MRIMCHATSESSION_H

#include <kopete/kopetechatsession.h>

class MrimProtocol;
class MrimContact;

class MrimChatSession : public Kopete::ChatSession
{
    Q_OBJECT
public:
    explicit MrimChatSession(const Kopete::Contact *user,
                             MrimContact *contact,
                             Kopete::ContactPtrList others,
                             MrimProtocol *protocol,
                             Kopete::ChatSession::Form form );

    virtual ~MrimChatSession();

    virtual void inviteContact(const QString &contactId);

signals:

public slots:

private:
    class Private;
    Private *d;
};

#endif // MRIMCHATSESSION_H
