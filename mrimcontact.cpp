#include <kdebug.h>
#include <kopete/kopeteaccount.h>
#include <kopete/kopetechatsessionmanager.h>
#include <kopete/kopetetransfermanager.h>
#include <QTimer>
#include <QMessageBox>
#include <kopete/kopeteavatarmanager.h>
#include <kopete/kopeteuiglobal.h>
#include <kopete/kopetemetacontact.h>
#include <kopete/kopetegroup.h>
#include <kfiledialog.h>

#include "ui/contactinfo.h"
#include "mra/mraofflinemessage.h"
#include "mra/mra_proto.h"
#include "mra/transferrequestinfo.h"
#include "filetransfertask.h"
#include "debug.h"
#include "mrimaccount.h"
#include "mrimprotocol.h"
#include "mrimchatsession.h"
#include "mrimcontact.h"

struct MrimContact::Private {
    Kopete::ChatSession* msgManager;
    QTimer *typingTimer;
    QTimer *myselfTypingTimer;
    ContactInfo *infoDialog;
    KAction *requestForAuthorization;
    int flags;
    QMap<int, FileTransferTask*> transferTasks;

    Private()
        : msgManager(NULL)
        , typingTimer(0)
        , myselfTypingTimer(0)
        , infoDialog(0)
        , flags(0) {
    }

};

MrimContact::MrimContact( Kopete::Account* _account,
                          const QString &uniqueName,
                          const QString &displayName,
                          int flags,
                          Kopete::MetaContact *parent )
    : Kopete::Contact( _account, uniqueName, parent, QString("mrim_protocol") )
    , d(new Private)
{
    mrimDebug()<< " uniqueName: " << uniqueName << ", displayName: " << displayName << "flags: " << flags;

    QTimer::singleShot( 10 * 1000, this, SLOT(slotLoadAvatar()) );

    d->requestForAuthorization = new KAction( KIcon("mail-reply-sender"), tr( "(Re)request Authorization From" ), this );
    connect( d->requestForAuthorization, SIGNAL(triggered(bool)), this, SLOT(slotPerformRequestForAuthorization()) );

    d->flags = flags;

    if ( not isChatContact() ) {
        setFileCapable( true );
    }

}

MrimContact::~MrimContact() {
    delete d;
}

void MrimContact::sendFile( const KUrl &sourceURL,
                   const QString &fileName, uint fileSize ) {

    kDebug(kdeDebugArea()) << sourceURL << fileName << fileSize;

    QStringList fileNames;
    //If the file location is null, then get it from a file open dialog
    if( !sourceURL.isValid() ) {
        fileNames = KFileDialog::getOpenFileNames( KUrl() ,"*", 0l  , tr( "Kopete File Transfer" ));
    } else {
        fileNames << sourceURL.path(KUrl::RemoveTrailingSlash);
    }

    kDebug(kdeDebugArea()) << "start transfer";

    FileTransferTask *task = new FileTransferTask(
                  dynamic_cast<MrimAccount*>( account() )
                , this
                , fileNames
                , FileTransferTask::Outgoing
                , 0
                , this);

    connect(task, SIGNAL(transferComplete()),
            this, SLOT(slotTransferFinished()) );

    connect(task, SIGNAL(transferFailed()),
            this, SLOT(slotTransferFinished()) );

    d->transferTasks[task->getSessionId()] = task;

}

void MrimContact::receiveFile(const TransferRequestInfo &transferInfo) {
    /// @todo ask user's confirmation
    kDebug(kdeDebugArea());
    FileTransferTask *task = new FileTransferTask(
                  dynamic_cast<MrimAccount*>( account() )
                , this
                , QStringList()
                , FileTransferTask::Incoming
                , &transferInfo
                , this);

    connect(task, SIGNAL(transferComplete()),
            this, SLOT(slotTransferFinished()) );

    connect(task, SIGNAL(transferFailed()),
            this, SLOT(slotTransferFinished()) );

    d->transferTasks[task->getSessionId()] = task;
}

void MrimContact::receiveFileCancel(const TransferRequestInfo &transferInfo) {
    if (d->transferTasks.contains(transferInfo.sessionId())) {
        d->transferTasks[transferInfo.sessionId()]->cancel();
    }
}

void MrimContact::slotTransferFinished() {
    FileTransferTask *task = (FileTransferTask *)sender();
    d->transferTasks.remove(task->getSessionId());
}

void MrimContact::setFlags(int arg) {
    d->flags = arg;
    if ( isChatContact() ) {
        setFileCapable( false );
    } else {
        setFileCapable( true );
    }
}

bool MrimContact::isOrdinaryContact() const {
    return (d->flags == CONTACT_FLAG_UNKNOWN) || (d->flags == 0);
}

bool MrimContact::isChatContact() const {
    return (d->flags & CONTACT_FLAG_CHAT) != 0;
}

void MrimContact::slotPerformRequestForAuthorization() {
    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );
    a->requestForAuthorization(contactId() );
}

QList<KAction *> *MrimContact::customContextMenuActions( Kopete::ChatSession* ) {

    QList<KAction *> *list = new QList<KAction *>();
    list->append(d->requestForAuthorization);

    return list;
}

QList<KAction *> *MrimContact::customContextMenuActions(  ) {
    return Kopete::Contact::customContextMenuActions();
}


Kopete::ChatSession* MrimContact::manager( CanCreateFlags canCreateFlags )
{

    if ( d->msgManager )
    {
        return d->msgManager;
    }
    else if ( canCreateFlags == CanCreate )
    {
        QList<Kopete::Contact*> contacts;

        contacts.append(this);

        Kopete::ChatSession::Form form = Kopete::ChatSession::Small;
        if ( isChatContact() ) {

            form = Kopete::ChatSession::Chatroom;

            loadChatMembersList();

            d->msgManager = new MrimChatSession(account()->myself(), this, contacts, dynamic_cast<MrimProtocol*>(protocol()), form);

        } else {
            d->msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol(), form );
        }

        connect(d->msgManager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
                this, SLOT(sendMessage(Kopete::Message&)) );

        connect(d->msgManager, SIGNAL(myselfTyping(bool)),
                this, SLOT(slotMyselfTyping(bool)) );

        connect(d->msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));

        return d->msgManager;
    }
    else
    {
        return 0;
    }

    return 0;
}


void MrimContact::slotChatSessionDestroyed()
{
    d->msgManager = 0;
}

void MrimContact::sendMessage( Kopete::Message &message )
{
    mrimDebug();

    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );

    a->sendMessage( contactId(), message.plainBody() );

    // give it back to the manager to display
    manager()->appendMessage( message );
    // tell the manager it was sent successfully
    manager()->messageSucceeded();
}

void MrimContact::receivedMessage( const QString &text ) {

    Kopete::Message msg( this, account()->myself() );

    msg.setDirection( Kopete::Message::Inbound );

    msg.setPlainBody( text );

    Kopete::ChatSession *session = manager(CanCreate);

    msg.setManager( session );

    session->appendMessage(msg);
}

void MrimContact::receivedOfflineMessage( const MRAOfflineMessage &message ) {

    Kopete::Message msg( this, account()->myself() );

    msg.setDirection( Kopete::Message::Inbound );

    msg.setPlainBody( message.text() );

    msg.setManager( manager(CanCreate) );

    msg.setTimestamp( QDateTime::fromTime_t( message.date().toTime_t() ) );

    Kopete::ChatSession *session = manager(CanCreate);
    session->appendMessage(msg);
}

void MrimContact::typingMessage() {
    if ( d->typingTimer  ) {
        d->typingTimer->stop();
        d->typingTimer->deleteLater();
        d->typingTimer = 0;
    }

    manager(CanCreate)->receivedTypingMsg( this, true );

    d->typingTimer = new QTimer(this);
    connect(d->typingTimer, SIGNAL(timeout()), this, SLOT(slotTypingTimeOut()));
    d->typingTimer->setSingleShot(true);
    d->typingTimer->setInterval(10 * 1000);
}

void MrimContact::slotTypingTimeOut() {
    manager(CanCreate)->receivedTypingMsg( this, false );
    d->typingTimer->deleteLater();
    d->typingTimer = 0;
}

void MrimContact::slotMyselfTyping(bool typing) {
    if (typing && !d->myselfTypingTimer) {
        d->myselfTypingTimer = new QTimer(this);

        connect(d->myselfTypingTimer, SIGNAL(timeout()), this, SLOT(slotMyselfTypingTimeout()));
        d->myselfTypingTimer->setInterval(10 * 1000);

        slotMyselfTypingTimeout();

    } else if ( !typing && d->myselfTypingTimer ) {
        d->myselfTypingTimer->stop();
        d->myselfTypingTimer->deleteLater();
        d->myselfTypingTimer = 0;
    }
}

void MrimContact::slotMyselfTypingTimeout() {
    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );
    a->contactTypingAMessage( contactId() );
}

void MrimContact::slotUserInfo() {

    if ( !account()->isConnected() ) {
        return;
    }

    new ContactInfo( dynamic_cast<MrimAccount*>( account() ), this, Kopete::UI::Global::mainWidget () );

    mrimDebug() << __PRETTY_FUNCTION__;

    loadUserInfo();
}

void MrimContact::slotUserInfoLoaded(const MRAContactInfo &info) {

    mrimDebug() << __PRETTY_FUNCTION__;

    emit userInfoLoaded(info);
}

void MrimContact::loadUserInfo() {

    mrimDebug() << __PRETTY_FUNCTION__;

    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );
    a->loadUserInfo( contactId() );
}

void MrimContact::slotLoadAvatar() {

    if ( !contactId().isEmpty() )
    {
        if ( !property(MrimProtocol::protocol()->propPhoto).isNull() ) {
            return ;
        }
        MrimAccount *a = dynamic_cast<MrimAccount*>( account() );
        a->loadAvatar( contactId() );

    } else {
        mrimDebug() << "empty!" << contactId();
    }

}

void MrimContact::avatarLoaded(const QImage &avatar) {

    // add the entry using the avatar manager
    Kopete::AvatarManager::AvatarEntry entry;
    entry.name = contactId();
    entry.image = avatar;
    entry.category = Kopete::AvatarManager::Contact;
    entry.contact = this;
    entry = Kopete::AvatarManager::self()->add(entry);

    // Save the image to the disk, then set the property.
    if(!entry.dataPath.isNull())
    {
        setProperty( MrimProtocol::protocol()->propPhoto, entry.dataPath );
    }

}

void MrimContact::deleteContact() {

    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );

    if ( this == a->myself() ) {
        mrimDebug() << "can't delete myself";
        return;
    }

    a->deleteContact( contactId() );

    Kopete::Contact::deleteContact();
}

void MrimContact::loadChatMembersList() {
    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );
    a->loadChatMembersList( contactId() );
}

void MrimContact::slotChatMembersListReceived(const QString &title, const QList<QString> &list) {
    foreach(const QString &contact, list) {
        mrimDebug() << contact;
        if ( account()->contacts().value(contact) ) {
            manager()->addContact( account()->contacts().value(contact) );
        }
    }
    manager()->setDisplayName(title);
}

void MrimContact::sync(unsigned int changed) {
    //
    mrimDebug() << metaContact()->displayName();
    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );

    if (changed & MovedBetweenGroup) {
        const QString &newGroup = metaContact()->groups().first()->displayName();
        a->moveContactToGroup( contactId(), newGroup );
    } else if (changed & DisplayNameChanged) {
        a->renameContact( contactId(), metaContact()->displayName() );
    } else {
        mrimDebug() << "unknown change action:" << changed;
    }
}

void MrimContact::inviteContact(const QString &contactIdToInvite) {

    if ( not isChatContact() ) {
        return;
    }

    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );
    a->inviteMemberToChat(contactId(), contactIdToInvite);
}

#include "mrimcontact.moc"
