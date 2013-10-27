#include <kcomponentdata.h>
#include <kactionmenu.h>
#include <klocale.h>
#include <kmenu.h>
#include <kactioncollection.h>

#include <kopete/kopetechatsessionmanager.h>
#include <kopete/kopeteaccount.h>
#include <kopete/ui/kopetecontactaction.h>

#include "ui/createconferencedialog.h"

#include "mra/mraconferencesettings.h"
#include "mrimcontact.h"
#include "mrimaccount.h"
#include "mrimprotocol.h"
#include "debug.h"

#include "mrimchatsession.h"


class MrimChatSession::Private {
public:
    MrimContact *contact;
    KAction *editConferenceAction;
    MRAConferenceSettings conferenceSettings;
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
    setComponentData( protocol->componentData() );

    d->contact = contact;
    setMayInvite( contact->isChatContact() );

    d->editConferenceAction = new KAction(KIcon("system-users"), i18n ("&Edit conference"), this);
    connect(d->editConferenceAction, SIGNAL(triggered()), this, SLOT(slotEditConference()));
    actionCollection()->addAction("mrimEditConference", d->editConferenceAction);

    setXMLFile( "kopetemrimchatui.rc" );

}


MrimChatSession::~MrimChatSession() {
    delete d;
}

void MrimChatSession::slotEditConference() {


    CreateConferenceDialog dialog(dynamic_cast<MrimAccount*>(account()));
    dialog.setSettings(d->conferenceSettings);
    if ( dialog.exec() == QDialog::Accepted ) {
        //
        MRAConferenceSettings newSettings = dialog.getSettings();

        // current contact is not present in the "list of available contacts"
        newSettings.addContact( account()->myself()->contactId() );

        if (newSettings != d->conferenceSettings) {
            MRAConferenceSettings::difference
                    diff = MRAConferenceSettings::compare(d->conferenceSettings, newSettings);

            foreach (const QString &contactId, diff.deletedItems) {
                mrimWarning() << "deleted" << contactId;
                d->contact->removeContactFromConference(contactId);
            }
            foreach (const QString &contactId, diff.newItems) {
                mrimWarning() << "new" << contactId;
                d->contact->inviteContactToConference(contactId);
            }
        }
    }

}

void MrimChatSession::inviteContact(const QString &contactId) {
    d->contact->inviteContactToConference(contactId);
}


void MrimChatSession::slotChatSettingsReceived(const MRAConferenceSettings &settings) {
    foreach(const QString &contact, settings.contactList()) {
        if ( account()->contacts().value(contact) ) {
            addContact( account()->contacts().value(contact) );
        }
    }
    setDisplayName(settings.title());
    d->conferenceSettings = settings;
}
