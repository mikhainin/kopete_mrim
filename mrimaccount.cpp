#include <QMessageBox>

#include <kdebug.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetemessage.h>
#include <kopeteaccount.h>

#include "mra/mraprotocol.h"
#include "mra/mraprotocolv123.h"
#include "mra/mra_proto.h"
#include "mra/mracontactlist.h"
#include "mra/mraofflinemessage.h"

#include "mrimprotocol.h"
#include "mrimcontact.h"
#include "mrimaccount.h"

struct MrimAccount::Private {
    QByteArray username;
    QByteArray password;
    MRAProtocol *mraProto;
    MRAContactListEntry adding;
    MRAContactList contactList;
    Kopete::MetaContact *addingMetacontact;
    Private() : mraProto(0), addingMetacontact(0) {
    }
};

MrimAccount::MrimAccount( MrimProtocol *parent, const QString& accountID )
    : Kopete::Account(parent, accountID)
    , d(new Private)
{
    kWarning() << __PRETTY_FUNCTION__;
    // Init the myself contact
    setMyself( new MrimContact( this, accountId(), accountId(), Kopete::ContactList::self()->myself() ) );
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    parseConfig();

}

MrimAccount::~MrimAccount()
{
    kWarning() << __PRETTY_FUNCTION__;

    if (isConnected())
        disconnect();

    delete d;
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


    if (d->username.isEmpty())
        return;

    d->contactList = MRAContactList();
    d->adding      = MRAContactListEntry();

    d->mraProto = new MRAProtocolV123(this); /// @todo: make the protocol's version optional

    QObject::connect(d->mraProto, SIGNAL(contactListReceived(MRAContactList)),
            this, SLOT(slotReceivedContactList(MRAContactList)) );

    QObject::connect(d->mraProto, SIGNAL( messageReceived(QString,QString)),
            this, SLOT( slotReceivedMessage(QString,QString) ) ) ;

    QObject::connect(d->mraProto, SIGNAL(connected()),
            this, SLOT( slotConnected() ) ) ;

    QObject::connect(d->mraProto, SIGNAL(loginFailed(QString)),
            this, SLOT( slotLoginFailed(QString) ) ) ;

    QObject::connect(d->mraProto, SIGNAL(disconnected(QString)),
                     this, SLOT( slotDisconnected(QString) ) ) ;
/*
    QObject::connect(d->mraProto, SIGNAL(authorizeAckReceived(QString)),
                     this, SLOT( slotAuthorizeAckReceived(QString) ) );
*/
    QObject::connect(d->mraProto, SIGNAL(authorizeRequestReceived(QString,QString)),
                     this, SLOT(authorizeRequestReceived(QString,QString)) );

    QObject::connect(d->mraProto, SIGNAL(userStatusChanged(QString,int)),
                     this, SLOT( slotUserStatusChanged(QString,int) ) );

    QObject::connect(d->mraProto, SIGNAL(typingAMessage(QString)),
                     this, SLOT( slotTypingAMessage(QString)) );

    QObject::connect(d->mraProto, SIGNAL(offlineReceived(MRAOfflineMessage)),
                     this, SLOT(slotReceivedOfflineMessage(MRAOfflineMessage)) );

    QObject::connect(d->mraProto, SIGNAL(avatarLoaded(QString,QImage)),
                     this, SLOT(slotAvatarLoaded(QString,QImage)) );

    QObject::connect(d->mraProto, SIGNAL(userInfoLoaded(QString,MRAContactInfo)),
                     this, SLOT(slotUserInfoLoaded(QString,MRAContactInfo)) );

    QObject::connect(d->mraProto, SIGNAL(addContactAckReceived(int,int)),
                     this, SLOT(slotAddContactAckReceived(int,int)) );

    if (d->mraProto->makeConnection(QString(d->username), QString(d->password)) ) {
        kWarning() << "connecting...";
    } else {
        kWarning() << "connect problems.";
    }

}

void MrimAccount::slotConnected() {
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
}

void MrimAccount::slotDisconnected(const QString &reason) {
    kWarning() << reason;
    /// @todo show the reason as notification
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    d->contactList = MRAContactList();
    // Kopete::Account::DisconnectReason reason;

    Kopete::Account::disconnected( Kopete::Account::OtherClient ); /// fixme
}

void MrimAccount::slotLoginFailed(const QString &reason) {
    kWarning() << reason;
    /// @todo show the reason as notification
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    d->contactList = MRAContactList();

}

void MrimAccount::authorizeRequestReceived(const QString &from, const QString &text) {

    if (d->contactList.getByAddress(from) || (d->adding.address() == from) )  {
        d->mraProto->authorizeContact(from);
        return;
    }

    QMessageBox::StandardButton answer =
        QMessageBox::question( 0,
                               "Authorization request",
                               "Would you like to authorize " + from + "?\n\n" + text,
                               QMessageBox::Yes | QMessageBox::No );

    if ( answer == QMessageBox::Yes ) {

        d->adding = MRAContactListEntry(-1);

        d->adding.setFlags(0);
        d->adding.setGroup(0);
        d->adding.setNick(from);
        d->adding.setAddress(from);


        QString groupName = d->contactList.groups()[ d->adding.group() ].name;

        Kopete::Group *g=Kopete::ContactList::self()->findGroup(groupName);

        Kopete::MetaContact *mc = addContact(d->adding.address(), d->adding.nick(), g, Kopete::Account::ChangeKABC);
        (MrimContact *) mc->findContact(
                        protocol()->pluginId(),
                        accountId(),
                        d->adding.address()
                    );


        d->mraProto->addToContactList( 0, 0, from, from, myself()->contactId(), tr("Please, authorize me.") );
        d->mraProto->authorizeContact(from);



        kWarning() << "contact:" << d->adding.address() << d->adding.address() << d->adding.status() << d->adding.group();

    }

}

void MrimAccount::disconnect()
{
    if (d->mraProto) {
        kWarning() << __PRETTY_FUNCTION__;
        d->mraProto->closeConnection();
        d->mraProto->deleteLater();
        d->mraProto = 0;
    }
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );
}

void MrimAccount::setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason,
                             const OnlineStatusOptions& /*options*/ )
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

    if (!isConnected()) {
        connect();
    }
    if ( d->mraProto ) {
        if (away) {
            d->mraProto->setStatus(MRAProtocol::AWAY);
        } else {
            d->mraProto->setStatus(MRAProtocol::ONLINE);
        }
    }
    if (away) {
        myself()->setOnlineStatus( MrimProtocol::protocol()->mrimAway );
    } else {
        myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
    }


}

void MrimAccount::slotGoOnline ()
{
    kWarning() << __PRETTY_FUNCTION__;

    if (!isConnected())
        connect();
    else {
        myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
        d->mraProto->setStatus(MRAProtocol::ONLINE);
    }
}


void MrimAccount::slotGoOffline()
{
    kWarning() << __PRETTY_FUNCTION__;

    foreach( Kopete::Contact *contact, contacts() ) {
        contact->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );
    }
    disconnect();
}

void MrimAccount::slotGoAway()
{
    kWarning() << __PRETTY_FUNCTION__;
    if ( !isConnected() ) {
        connect();
    }
    if ( !d->mraProto ) {
        kWarning() << "connected but connection is not available";
    }
    d->mraProto->setStatus(MRAProtocol::AWAY);
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimAway );
}

void MrimAccount::parseConfig()
{
    d->username = configGroup()->readEntry("username").toLocal8Bit();
    d->password = configGroup()->readEntry("password").toLocal8Bit();

}

void MrimAccount::setUsername(const QByteArray &arg)
{
    d->username = arg;
}

const QByteArray MrimAccount::getUsername() const
{
    return d->username;
}

void MrimAccount::setPassword(const QByteArray &arg)
{
    d->password = arg;
}

const QByteArray MrimAccount::getPassword() const
{
    return d->password;
}


void MrimAccount::slotReceivedContactList(const MRAContactList &list) {

    kDebug() << __PRETTY_FUNCTION__;

    d->contactList = list;

    for( int i = 0; i < list.count(); ++i ) {
        const MRAContactListEntry &item = list[i];

        QString groupName = list.groups()[ item.group() ].name;

        Kopete::Group *g=Kopete::ContactList::self()->findGroup(groupName);

        Kopete::MetaContact *mc = addContact(item.address(), item.nick(), g, Kopete::Account::ChangeKABC);

        MrimContact *c = (MrimContact *) mc->findContact( // ???
                        protocol()->pluginId(),
                        accountId(),
                        item.address()
                    );

        kWarning() << "contact:" << item.address() << item.address() << item.status() << item.group();

        c->setOnlineStatus( mrimStatusToKopete(item.status()) );
    }
}

void MrimAccount::addNewContactToServerList(const QString &email, const QString &nick, const QString &groupName, Kopete::MetaContact *m ) {
    int flags = 0;

    int gid = d->contactList.groups().indexOf(groupName);

    kWarning() << flags << gid << email << groupName;

    d->adding = MRAContactListEntry(-1);
    d->adding.setFlags(flags);
    d->adding.setNick(nick);
    d->adding.setGroup(gid);
    d->adding.setAddress(email);

    d->mraProto->addToContactList( flags, gid, email, nick, myself()->contactId(), tr("Please, authorize me.") );

    d->addingMetacontact = m;
}

void MrimAccount::slotUserStatusChanged(const QString &user, int newStatus) {

    Kopete::Contact *c = contacts().value(user);
    if (c) {
        c->setOnlineStatus( mrimStatusToKopete(newStatus) );
    } else {
        kWarning() << "user was not found" << user;
    }

}

Kopete::OnlineStatus MrimAccount::mrimStatusToKopete(int mrimStatus) {

    MrimProtocol *p = dynamic_cast<MrimProtocol *>(protocol());

    switch(mrimStatus) {

    case STATUS_ONLINE:
        return  p->mrimOnline;

    case STATUS_OFFLINE:
        return Kopete::OnlineStatus::Offline;

    case STATUS_AWAY:
        return Kopete::OnlineStatus::Away;

    case STATUS_DONT_DISTRUB:
        return Kopete::OnlineStatus::Busy;

    case STATUS_FLAG_INVISIBLE:
        return Kopete::OnlineStatus::Invisible;

    case STATUS_UNDETERMINATED:
        return Kopete::OnlineStatus::Unknown;

    default:
        return Kopete::OnlineStatus::Unknown;
    }
}

void MrimAccount::sendMessage(const QString &to, const QString &text) {
    d->mraProto->sendText(to, text);
}

void MrimAccount::slotReceivedMessage( const QString &from, const QString &text )
{
    kWarning() << "from=" << from;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(from) );

    if (c) {
        c->receivedMessage(text);
    } else {
        kWarning() << "user was not found" << from;
    }
}

void MrimAccount::slotReceivedOfflineMessage( const MRAOfflineMessage &message ) {
    kWarning() << "from=" << message.from();

    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(message.from()) );

    if (c) {
        c->receivedOfflineMessage(message);
    } else {
        kWarning() << "user was not found" << message.from();
    }

}

void MrimAccount::slotTypingAMessage( const QString &from ) {
    kWarning() << "from=" << from;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(from) );

    if (c) {
        c->typingMessage();
    } else {
        kWarning() << "user was not found" << from;
    }
}

void MrimAccount::contactTypingAMessage( const QString &to ) {
    if (d->mraProto) {
        d->mraProto->sendTypingMessage(to);
    }
}

void MrimAccount::loadAvatar( const QString &email) {
    if (d->mraProto) {
        kWarning() << email;
        d->mraProto->loadAvatar( email );
    }
}

void MrimAccount::loadPhoto( const QString &email, QObject *receiver, const char *member ) {
    if (d->mraProto) {
        kWarning() << email;
        d->mraProto->loadAvatar( email, true, receiver, member );
    } else {
        kWarning() << "there's undefined connection" << email;
    }
}

void MrimAccount::slotAvatarLoaded(const QString &contact, const QImage &image) {

    kWarning() << "contact=" << contact;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(contact) );

    if (c) {
        c->avatarLoaded(image);
    } else {
        kWarning() << "user was not found" << contact;
    }

}

void MrimAccount::loadUserInfo( const QString &email ) {
    d->mraProto->loadUserInfo(email);
}

void MrimAccount::slotUserInfoLoaded(const QString &contact, const MRAContactInfo &info) {
    kWarning() << "contact=" << contact;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(contact) );

    if (c) {
        c->slotUserInfoLoaded( info );
    } else {
        kWarning() << "user was not found" << contact;
    }
}

void MrimAccount::deleteContact( const QString &email ) {
    const MRAContactListEntry *ce = d->contactList.getByAddress( email );

    if ( ce ) {
        d->mraProto->deleteContact( ce->id(), ce->address(), ce->nick() );
    }
}


void MrimAccount::slotAddContactAckReceived(int status, int contactId) {


    if (status != CONTACT_OPER_SUCCESS) {
        return;
    }

    if (d->addingMetacontact) {
        if (!addContact(d->adding.address(), d->addingMetacontact, Kopete::Account::ChangeKABC)) {
            kWarning() << "Can't add contact";
            return;
        }
    }

    d->adding.setId(contactId);

    d->contactList.addEntry(d->adding);

    d->adding = MRAContactListEntry();
    d->addingMetacontact = 0;
}

void MrimAccount::requestForAuthorization( const QString &contact ) {
    d->mraProto->sendAuthorizationRequest(contact, myself()->contactId(), tr("Please, authorize me."));
}

#include "mrimaccount.moc"
