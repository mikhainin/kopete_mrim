#include <kdebug.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetemessage.h>

#include "mra/mraprotocol.h"
#include "mrimprotocol.h"
#include "mrimcontact.h"
#include "mrimaccount.h"

MrimAccount::MrimAccount( MrimProtocol *parent, const QString& accountID )
    : Kopete::Account(parent, accountID)
    , group(NULL)
{
    kWarning() << __PRETTY_FUNCTION__;
    // Init the myself contact
	setMyself( new MrimContact( this, accountId(), accountId(), Kopete::ContactList::self()->myself() ) );
	myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

	group = Kopete::ContactList::self()->findGroup("MRIM");

	// Clean out Contacts from last time when kopete starts up
	// wipeOutAllContacts();

	parseConfig();

}

MrimAccount::~MrimAccount()
{
    kWarning() << __PRETTY_FUNCTION__;
    
    if (isConnected())
		disconnect();
}

bool MrimAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
    kWarning() << __PRETTY_FUNCTION__;
    
    MrimContact* newContact = new MrimContact( this, contactId, parentContact->displayName(), parentContact );
	return newContact != NULL;
}

void MrimAccount::connect( const Kopete::OnlineStatus& /*initialStatus*/ )
{
    kWarning() << __PRETTY_FUNCTION__;
    
    
    if (username.isEmpty())
		return;

    m_mraProto = new MRAProtocol(this);
    
    QObject::connect(m_mraProto, SIGNAL(contactListReceived(MRAContactList)), 
            this, SLOT(slotContactListReceived(MRAContactList)) );
    
    QObject::connect(m_mraProto, SIGNAL( messageReceived(QString,QString)), 
            this, SLOT( slotReceivedMessage(QString,QString) ) ) ;
    
    if (m_mraProto->makeConnection(QString(username).toStdString(), QString(password).toStdString()) ) {
        kWarning() << "connected!";
        myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
    } else {
        kWarning() << "not connected";
    	myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );
    }
    
    // group = Kopete::ContactList::self()->findGroup("MRIM");
    // Kopete::ContactList::self()->addGroup( new Kopete::Group() );
    
    // Kopete::Group *g=Kopete::ContactList::self()->findGroup(group);
    // addContact("test", "display name", group, Kopete::Account::Temporary);
    // addContact(
//	startBrowse();
}

void MrimAccount::disconnect()
{
    kWarning() << __PRETTY_FUNCTION__;
    m_mraProto->closeConnection();
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );
}

void MrimAccount::setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason,
                             const OnlineStatusOptions& options )
{
    kWarning() << __PRETTY_FUNCTION__;
    
    if ( status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
		slotGoOnline();
	else if (status.status() == Kopete::OnlineStatus::Online &&
			(myself()->onlineStatus().status() == Kopete::OnlineStatus::Away ||
				myself()->onlineStatus().status() == Kopete::OnlineStatus::Away) )
		setAway( false, reason.message() );
	else if ( status.status() == Kopete::OnlineStatus::Offline )
		slotGoOffline();
	else if ( status.status() == Kopete::OnlineStatus::Away )
		slotGoAway( /* reason */ );
    else
        slotGoOnline();
    
}

void MrimAccount::setStatusMessage(const Kopete::StatusMessage& statusMessage)
{
    kWarning() << __PRETTY_FUNCTION__;
    
}

void MrimAccount::setAway(bool away, const QString& reason)
{
    kWarning() << __PRETTY_FUNCTION__;
    
}

void MrimAccount::slotGoOnline ()
{
    kWarning() << __PRETTY_FUNCTION__;

	if (!isConnected())
		connect();
	else {
		/*if (service) {
			QMap <QString, QByteArray> map = service->textData();
			map["status"] = AvailabilityStatusAvailId;
			service->setTextData(map);
		}
        */
		myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
	}
}


void MrimAccount::slotGoOffline() 
{
    kWarning() << __PRETTY_FUNCTION__;
}

void MrimAccount::slotGoAway() 
{
    kWarning() << __PRETTY_FUNCTION__;
}

void MrimAccount::parseConfig()
{
	username = configGroup()->readEntry("username").toLocal8Bit();
	password = configGroup()->readEntry("password").toLocal8Bit();
//	lastName = configGroup()->readEntry("lastName").toLocal8Bit();
//	emailAddress = configGroup()->readEntry("emailAddress").toLocal8Bit();
}

void MrimAccount::setUsername(const QByteArray &arg)
{
	username = arg;
}

const QByteArray MrimAccount::getUsername() const
{
	return username;
}

void MrimAccount::setPassword(const QByteArray &arg)
{
	password = arg;
}

const QByteArray MrimAccount::getPassword() const
{
	return password;
}


void MrimAccount::slotContactListReceived(const MRAContactList &list) {
    
    kWarning() << __PRETTY_FUNCTION__;
    
    for( int i = 0; i < list.count(); ++i ) {
        MRAContactListEntry item = list[i];
        
        QString groupName = list.groups()[ item.group() ].name;
        
        Kopete::Group *g=Kopete::ContactList::self()->findGroup(groupName);
        
        Kopete::MetaContact *mc = addContact(item.address(), item.nick(), g, Kopete::Account::Temporary /* ChangeKABC */);
        
        MrimContact *c = (MrimContact *) mc->findContact( // ???
                        protocol()->pluginId(),
                        accountId(),
                        item.address()
                    );
        
        kWarning() << "contact:" << item.address() << item.address() << item.status();
                      
        c->setOnlineStatus( mrimStatusToKopete(item.status()) );
    }
}

Kopete::OnlineStatus MrimAccount::mrimStatusToKopete(int mrimStatus) {
    
    switch(mrimStatus) {
    
    case STATUS_ONLINE:
        return Kopete::OnlineStatus::Online;
        
    case STATUS_OFFLINE:
        return Kopete::OnlineStatus::Offline;
        
    case STATUS_AWAY:
        return Kopete::OnlineStatus::Away;
        
    case STATUS_FLAG_INVISIBLE:
        return Kopete::OnlineStatus::Invisible;
        
    case STATUS_UNDETERMINATED:
        return Kopete::OnlineStatus::Unknown;
        
    default:
        return Kopete::OnlineStatus::Unknown;
    }
}

void MrimAccount::sendMessage(const QString &to, const QString &text) {
    m_mraProto->sendText(to, text);
}

void MrimAccount::slotReceivedMessage( const QString &from, const QString &text )
{
    Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact *>( contacts().value(from) );
    
    MrimContact *c = (MrimContact *) mc->findContact(
                    protocol()->pluginId(),
                    accountId(),
                    from
                );
    
    c->receivedMessage(text);
}

#include "mrimaccount.moc"
