#include <kcomponentdata.h>
#include <kopete/kopetechatsessionmanager.h>

#include "mrimcontact.h"
#include "mrimprotocol.h"
#include "debug.h"

#include "mrimchatsession.h"


class MrimChatSession::Private {
public:
    MrimContact *contact;
};

MrimChatSession::MrimChatSession(const Kopete::Contact *user,
                                 MrimContact *contact,
                                 Kopete::ContactPtrList others,
                                 MrimProtocol *protocol,
                                 Form form = Small ) :
    Kopete::ChatSession(user, others, protocol, form),
    d(new Private)
{

    Kopete::ChatSessionManager::self()->registerChatSession( this );
    setComponentData(protocol->componentData());

    d->contact = contact;
    setMayInvite( true || contact->isChatContact() );

}


MrimChatSession::~MrimChatSession() {
    delete d;
}

void MrimChatSession::inviteContact(const QString &contactId) {
    mrimWarning() << contactId;
    d->contact->inviteContact(contactId);
}
