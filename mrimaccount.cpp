#include <QMessageBox>
#include <QInputDialog>

#include <kdebug.h>
#include <kinputdialog.h>
#include <klocalizedstring.h>
#include <kactionmenu.h>

#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <kopetemessage.h>
#include <kopeteaccount.h>
#include <kopeteutils.h>


#include "mra/mraprotocol.h"
#include "mra/mraprotocolv123.h"
#include "mra/mra_proto.h"
#include "mra/mracontactlist.h"
#include "mra/mraofflinemessage.h"
#include "mra/transferrequestinfo.h"

#include "ui/createconferencedialog.h"

#include "debug.h"
#include "mrimprotocol.h"
#include "mrimcontact.h"
#include "mrimaccount.h"
#include "addcontacttask.h"

struct MrimAccount::Private {
    QByteArray username;
    QByteArray password;
    QByteArray protocolVersion;
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
    mrimDebug() << __PRETTY_FUNCTION__;
    // Init the myself contact
    setMyself( new MrimContact( this, accountId(), accountId(), 0, Kopete::ContactList::self()->myself() ) );
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    parseConfig();

}

MrimAccount::~MrimAccount()
{
    mrimDebug() << __PRETTY_FUNCTION__;

    if (isConnected())
        disconnect();

    delete d;
}

MRAProtocol *MrimAccount::getMraProtocol() {
    return d->mraProto;
}


void MrimAccount::fillActionMenu( KActionMenu *actionMenu )
{
    mrimDebug() << __PRETTY_FUNCTION__;
    Kopete::Account::fillActionMenu(actionMenu);

    if (!isConnected()) {
        return;
    }

    actionMenu->addSeparator();

    KAction *addGroupAction = new KAction( i18n("Add group"), actionMenu );

    QObject::connect( addGroupAction, SIGNAL(triggered(bool)), this, SLOT(addGroup()) );

    actionMenu->addAction( addGroupAction );

    KAction *addConferenceAction = new KAction( i18n("Create new conference"), actionMenu );

    QObject::connect( addConferenceAction, SIGNAL(triggered(bool)), this, SLOT(addConference()) );

    actionMenu->addAction( addConferenceAction );
}

void MrimAccount::addGroup() {
    bool ok = false;
    QString newGroupName = KInputDialog::getText( i18n("New group"), "New group name", QString(), &ok);
    if (not ok) {
        return;
    }
    /// @todo check if the group is already exists
    AddContactTask *task = new AddContactTask(this);
    task->setGroupName(newGroupName);
    task->runAddGroupWithoutContact();
    // (void)addGroupAndReturnId(newGroupName);
}

void MrimAccount::addConference() {
    CreateConferenceDialog *c = new CreateConferenceDialog(this);
    if ( c->exec() == QDialog::Accepted ) {
        MRAProtocolV123 *p = dynamic_cast<MRAProtocolV123 *>(d->mraProto);
        p->createChat( c->getSettings() );
    }
}

bool MrimAccount::createContact(const QString& contactId, Kopete::MetaContact* parentContact)
{
    mrimDebug() << __PRETTY_FUNCTION__;

    MrimContact* newContact = new MrimContact( this, contactId, parentContact->displayName(), 0, parentContact );
    return newContact != NULL;
}

void MrimAccount::connect( const Kopete::OnlineStatus& /*initialStatus*/ )
{
    mrimDebug() << __PRETTY_FUNCTION__;


    if (d->username.isEmpty())
        return;

    d->contactList = MRAContactList();
    d->adding      = MRAContactListEntry();

    if (d->protocolVersion == "1.8") {
        d->mraProto = new MRAProtocol(this);
    } else {
        d->mraProto = new MRAProtocolV123(this);
    }

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

    QObject::connect(d->mraProto, SIGNAL(chatMembersListReceived(QString,QString,QList<QString>)),
                     this, SLOT(slotChatMembersListReceived(QString,QString,QList<QString>)));

    QObject::connect(d->mraProto, SIGNAL(chatIvitationReceived(QString,QString,QString)),
                     this, SLOT(slotChatInvitationReceived(QString,QString,QString)));

    QObject::connect(d->mraProto, SIGNAL(transferRequest(TransferRequestInfo)),
                     this, SLOT(slotTransferRequest(TransferRequestInfo)) );

    QObject::connect(d->mraProto, SIGNAL(transferRequestCancelled(TransferRequestInfo)),
                     this, SLOT(slotTransferRequestCancelled(TransferRequestInfo)) );

    if (d->mraProto->makeConnection(QString(d->username), QString(d->password)) ) {
        mrimDebug() << "connecting...";
    } else {
        mrimDebug() << "connect problems.";
    }

}

void MrimAccount::slotConnected() {
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
}

void MrimAccount::slotDisconnected(const QString &reason) {
    mrimDebug() << reason;
    /// @todo show the reason as notification
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    d->contactList = MRAContactList();
    // Kopete::Account::DisconnectReason reason;

    Kopete::Account::disconnected( Kopete::Account::OtherClient ); /// fixme

    Kopete::Utils::notifyConnectionLost(this, QString(), reason);
    // notifyConnectionLost();
}

void MrimAccount::slotLoginFailed(const QString &reason) {
    mrimDebug() << reason;
    /// @todo show the reason as notification
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );

    d->contactList = MRAContactList();
    Kopete::Utils::notifyConnectionLost(this, QString(), reason);

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


        d->mraProto->addToContactList( 0, 0, from, from, myself()->contactId(), tr("Please, authorize me."), 0 );
        d->mraProto->authorizeContact(from);



        mrimDebug() << "contact:" << d->adding.address() << d->adding.address() << d->adding.status() << d->adding.group();

    }

}

void MrimAccount::disconnect()
{
    if (d->mraProto) {
        mrimDebug() << __PRETTY_FUNCTION__;
        d->mraProto->closeConnection();
        d->mraProto->deleteLater();
        d->mraProto = 0;
    }
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );
}

void MrimAccount::setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason,
                             const OnlineStatusOptions& /*options*/ )
{
    mrimDebug() << __PRETTY_FUNCTION__;

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
    else if ( status.status() == Kopete::OnlineStatus::Busy )
        slotGoBusy();
    else
        slotGoOnline();

}

void MrimAccount::setStatusMessage(const Kopete::StatusMessage& statusMessage)
{
    /// TODO: set status message
    Q_UNUSED(statusMessage);
}

void MrimAccount::setAway(bool away, const QString& reason)
{
    mrimDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(reason); /// FIXME

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
    mrimDebug() << __PRETTY_FUNCTION__;

    if (!isConnected())
        connect();
    else {
        myself()->setOnlineStatus( MrimProtocol::protocol()->mrimOnline );
        d->mraProto->setStatus(MRAProtocol::ONLINE);
    }
}


void MrimAccount::slotGoOffline()
{
    foreach( Kopete::Contact *contact, contacts() ) {
        contact->setOnlineStatus( MrimProtocol::protocol()->mrimOffline );
    }
    disconnect();
}

void MrimAccount::slotGoAway()
{
    mrimDebug() << __PRETTY_FUNCTION__;
    if ( !isConnected() ) {
        connect();
    }
    if ( !d->mraProto ) {
        mrimDebug() << "connected but connection is not available";
    }
    d->mraProto->setStatus(MRAProtocol::AWAY);
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimAway );
}

void MrimAccount::slotGoBusy() {
    if ( !isConnected() ) {
        connect();
    }
    if ( !d->mraProto ) {
        mrimDebug() << "connected but connection is not available";
    }
    d->mraProto->setStatus(MRAProtocol::DONT_DISTRUB);
    myself()->setOnlineStatus( MrimProtocol::protocol()->mrimDontDistrub );

}

void MrimAccount::parseConfig()
{
    d->username = configGroup()->readEntry("username").toLocal8Bit();
    d->password = configGroup()->readEntry("password").toLocal8Bit();
    d->protocolVersion = configGroup()->readEntry("protocolVersion").toLocal8Bit();

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

void MrimAccount::setProtocolVersion(const QByteArray &arg) {
    d->protocolVersion = arg;
}

const QByteArray MrimAccount::getProtocolVersion() const {
    return d->protocolVersion;
}



void MrimAccount::slotReceivedContactList(const MRAContactList &list) {

    mrimDebug() << __PRETTY_FUNCTION__;

    d->contactList = list;

    // add empty groups if any
    for( int i = 0; i < list.groups().count(); ++i ) {
        QString groupName = list.groups()[i].name;
        Kopete::Group *g=Kopete::ContactList::self()->findGroup(groupName);
        Kopete::ContactList::self()->addGroup(g);
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

        mrimDebug() << "contact:" << item.address() << item.address() << item.status() << item.group();

        c->setOnlineStatus( mrimStatusToKopete(item.status()) );

        c->setFlags( item.flags() );
    }
}

int MrimAccount::getGroupIdByName( const QString &groupName ) {
    return d->contactList.groups().indexOf(groupName);
}

int MrimAccount::addGroupAndReturnId(const QString groupName) {
    d->contactList.groups().add(MRAGroup(groupName));
    return getGroupIdByName(groupName);
}

void MrimAccount::slotUserStatusChanged(const QString &user, int newStatus) {

    Kopete::Contact *c = contacts().value(user);
    if (c) {
        c->setOnlineStatus( mrimStatusToKopete(newStatus) );
    } else {
        mrimDebug() << "user was not found" << user;
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

void MrimAccount::loadChatMembersList(const QString &to) {
    d->mraProto->loadChatMembersList( to );
}

void MrimAccount::inviteMemberToChat(const QString &to, const QString &contactIdToInvite) {
    d->mraProto->inviteMemberToChat( to, contactIdToInvite );
}

void MrimAccount::slotReceivedMessage( const QString &from, const QString &text )
{
    mrimDebug() << "from=" << from;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(from) );

    if (c) {
        c->receivedMessage(text);
    } else {
        mrimDebug() << "user was not found" << from;
    }
}

void MrimAccount::slotReceivedOfflineMessage( const MRAOfflineMessage &message ) {
    mrimDebug() << "from=" << message.from();

    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(message.from()) );

    if (c) {
        c->receivedOfflineMessage(message);
    } else {
        mrimDebug() << "user was not found" << message.from();
    }

}

void MrimAccount::slotTypingAMessage( const QString &from ) {
    mrimDebug() << "from=" << from;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(from) );

    if (c) {
        c->typingMessage();
    } else {
        mrimDebug() << "user was not found" << from;
    }
}

void MrimAccount::contactTypingAMessage( const QString &to ) {
    if (d->mraProto) {
        d->mraProto->sendTypingMessage(to);
    }
}

void MrimAccount::loadAvatar( const QString &email) {
    if (d->mraProto) {
        mrimDebug() << email;
        d->mraProto->loadAvatar( email );
    }
}

void MrimAccount::loadPhoto( const QString &email, QObject *receiver, const char *member ) {
    if (d->mraProto) {
        mrimDebug() << email;
        d->mraProto->loadAvatar( email, true, receiver, member );
    } else {
        mrimDebug() << "there's undefined connection" << email;
    }
}

void MrimAccount::slotAvatarLoaded(const QString &contact, const QImage &image) {

    mrimDebug() << "contact=" << contact;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(contact) );

    if (c) {
        c->avatarLoaded(image);
    } else {
        mrimDebug() << "user was not found" << contact;
    }

}

void MrimAccount::loadUserInfo( const QString &email ) {
    d->mraProto->loadUserInfo(email);
}

void MrimAccount::slotUserInfoLoaded(const QString &contact, const MRAContactInfo &info) {
    mrimDebug() << "contact=" << contact;
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(contact) );

    if (c) {
        c->slotUserInfoLoaded( info );
    } else {
        mrimDebug() << "user was not found" << contact;
    }
}

void MrimAccount::deleteContact( const QString &email ) {
    const MRAContactListEntry *ce = d->contactList.getByAddress( email );

    if ( ce ) {
        d->mraProto->deleteContact( ce->id(), ce->address(), ce->nick() );
        d->contactList.deleteContact( email );
    }
}

void MrimAccount::renameContact( const QString &email, const QString &newName ) {
    const MRAContactListEntry *ce = d->contactList.getByAddress( email );

    if ( ce ) {
        d->mraProto->editContact( ce->id(), ce->address(), ce->group(), newName );
    }

}

void MrimAccount::moveContactToGroup( const QString &email, const QString &newGroupName ) {
    const MRAContactListEntry *ce = d->contactList.getByAddress( email );

    const int newGroupId = d->contactList.groups().indexOf( newGroupName );

    if ( newGroupId == -1 ) {
        mrimDebug() << "can't find group " << newGroupName;
        return;
    }

    if ( ce ) {
        d->mraProto->editContact( ce->id(), ce->address(), newGroupId, ce->nick() );
    }
}

void MrimAccount::slotAddContactAckReceived(int status, int contactId) {


    if (status != CONTACT_OPER_SUCCESS) {
        return;
    }

    if (d->addingMetacontact) {
        d->addingMetacontact->setDisplayName( d->adding.nick() );
        if (!addContact(d->adding.address(), d->addingMetacontact, Kopete::Account::ChangeKABC)) {
            mrimDebug() << "Can't add contact";
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

void MrimAccount::slotChatMembersListReceived(const QString &chat, const QString &title, const QList<QString> &list) {
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(chat) );

    if (c) {
        c->slotChatMembersListReceived( title, list );
    }
}

void MrimAccount::slotChatInvitationReceived(const QString &chat, const QString &title, const QString &from) {
    mrimDebug() << chat << title << from;

    d->adding = MRAContactListEntry(-1);
    d->adding.setFlags(0);
    d->adding.setNick(title);
    d->adding.setGroup(0);
    d->adding.setAddress(chat);

    d->mraProto->addToContactList( 0, 0, chat, title, myself()->contactId(), tr("Please, authorize me."), 0 );
    d->addingMetacontact = addContact(chat, title, 0, Kopete::Account::Temporary);
}

void MrimAccount::slotTransferRequest(const TransferRequestInfo &transferInfo)
{
    mrimDebug() << transferInfo.remoteContact();
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(transferInfo.remoteContact()) );

    if (c) {
        c->receiveFile(transferInfo);
    }
}

void MrimAccount::slotTransferRequestCancelled(const TransferRequestInfo &transferInfo) {
    MrimContact *c = dynamic_cast<MrimContact *>( contacts().value(transferInfo.remoteContact()) );

    if (c) {
        c->receiveFileCancel(transferInfo);
    }

}

#include "mrimaccount.moc"
