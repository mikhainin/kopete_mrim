#include <QMessageBox>

#include <kdebug.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetemessage.h>
#include <kopeteaccount.h>

#include "mra/mraprotocol.h"
#include "mra/mra_proto.h"
#include "mra/mracontactlist.h"
#include "mra/mraofflinemessage.h"

#include "mrimprotocol.h"
#include "mrimcontact.h"
#include "mrimaccount.h"

MrimAccount::MrimAccount( MrimProtocol *parent, const QString& accountID )
    : Kopete::Account(parent, accountID)
    , m_mraProto(0)
{
    kWarning() << __PRETTY_FUNCTION__;
    // Init the myself contact
    setMyself( new MrimContact( this, accountId(), accountId(), Kopete::ContactList::self()->myself() ) );
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

        // group = Kopete::ContactList::self()->findGroup("MRIM");

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

    m_groups.clear();

    m_mraProto = new MRAProtocol(this);

    QObject::connect(m_mraProto, SIGNAL(contactListReceived(MRAContactList)),
            this, SLOT(slotContactListReceived(MRAContactList)) );

    QObject::connect(m_mraProto, SIGNAL( messageReceived(QString,QString)),
            this, SLOT( slotReceivedMessage(QString,QString) ) ) ;

    QObject::connect(m_mraProto, SIGNAL(connected()),
            this, SLOT( slotConnected() ) ) ;

    QObject::connect(m_mraProto, SIGNAL(loginFailed(QString)),
            this, SLOT( slotLoginFailed(QString) ) ) ;

    QObject::connect(m_mraProto, SIGNAL(disconnected(QString)),
                     this, SLOT( slotDisconnected(QString) ) ) ;
/*
    QObject::connect(m_mraProto, SIGNAL(authorizeAckReceived(QString)),
                     this, SLOT( slotAuthorizeAckReceived(QString) ) );
*/
    QObject::connect(m_mraProto, SIGNAL(authorizeRequestReceived(QString,QString)),
                     this, SLOT(authorizeRequestReceived(QString,QString)) );

    QObject::connect(m_mraProto, SIGNAL(userStatusChanged(QString,int)),
                     this, SLOT( slotUserStatusChanged(QString,int) ) );

    QObject::connect(m_mraProto, SIGNAL(typingAMessage(QString)),
                     this, SLOT( slotTypingAMessage(QString)) );

    QObject::connect(m_mraProto, SIGNAL(offlineReceived(MRAOfflineMessage)),
                     this, SLOT(slotReceivedOfflineMessage(MRAOfflineMessage)) );

    QObject::connect(m_mraProto, SIGNAL(avatarLoaded(QString,QImage)),
                     this, SLOT(slotAvatarLoaded(QString,QImage)) );

    QObject::connect(m_mraProto, SIGNAL(userInfoLoaded(QString,MRAContactInfo)),
                     this, SLOT(slotUserInfoLoaded(QString,MRAContactInfo)) );

    if (m_mraProto->makeConnection(QString(username).toStdString(), QString(password).toStdString()) ) {
        kWarning() << "connecting...";
    } else {
        kWarning() << "connect problems.";
    }

//	startBrowse();
}

void MrimAccount::slotConnected() {
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
}

void MrimAccount::slotDisconnected(const QString &reason) {
    kWarning() << reason;
    /// @todo show the reason as notification
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    m_groups.clear();
    // Kopete::Account::DisconnectReason reason;

    Kopete::Account::disconnected( Kopete::Account::OtherClient ); /// fixme
}

void MrimAccount::slotLoginFailed(const QString &reason) {
    kWarning() << reason;
    /// @todo show the reason as notification
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    m_groups.clear();

}

void MrimAccount::authorizeRequestReceived(const QString &from, const QString &text) {
    QMessageBox::StandardButton answer =
        QMessageBox::question( 0,
                               "Authorization request",
                               "Would you like to authorize " + from + "?\n\n" + text,
                               QMessageBox::Yes | QMessageBox::No );

    if ( answer == QMessageBox::Yes ) {
        m_mraProto->authorizeContact(from);
        m_mraProto->addToContactList(0, 0, from, from);
    }

}

void MrimAccount::disconnect()
{
    if (m_mraProto) {
        kWarning() << __PRETTY_FUNCTION__;
        m_mraProto->closeConnection();
        m_mraProto->deleteLater();
        m_mraProto = 0;
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
    if ( m_mraProto ) {
        if (away) {
            m_mraProto->setStatus(STATUS_AWAY);
        } else {
            m_mraProto->setStatus(STATUS_ONLINE);
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
        m_mraProto->setStatus(STATUS_ONLINE);
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
    if ( m_mraProto ) {
        m_mraProto->setStatus(STATUS_AWAY);
    }
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimAway );
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

    for( int i = 0; i < list.groups().count(); ++i ) {
        m_groups.push_back( list.groups()[i].name );
    }

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

void MrimAccount::addNewContactToServerList(const QString &email, const QString &nick, const QString &groupName) {
    int flags = 0;

    int gid = m_groups.indexOf(groupName);

    kWarning() << flags << gid << email << groupName;

    m_mraProto->addToContactList( flags, gid, email, nick );
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
        return  p->mrimOnline; // Kopete::OnlineStatus::Online;

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
    m_mraProto->sendTypingMessage(to);
}

void MrimAccount::loadAvatar( const QString &email) {
    if (m_mraProto) {
        kWarning() << email;
        m_mraProto->loadAvatar( email );
    }
}

void MrimAccount::loadPhoto( const QString &email, QObject *receiver, const char *member ) {
    if (m_mraProto) {
        kWarning() << email;
        m_mraProto->loadAvatar( email, true, receiver, member );
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
    m_mraProto->loadUserInfo(email);
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

#include "mrimaccount.moc"
