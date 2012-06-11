#include <kdebug.h>
#include <kopeteaccount.h>
#include <kopetechatsessionmanager.h>
#include <QTimer>
#include <QMessageBox>
#include <kopeteavatarmanager.h>
#include <kopeteuiglobal.h>

#include "ui/contactinfo.h"
#include "mra/mraofflinemessage.h"
#include "mrimaccount.h"
#include "mrimprotocol.h"
#include "mrimcontact.h"

struct MrimContact::Private {
    Kopete::ChatSession* msgManager;
    QTimer *typingTimer;
    QTimer *myselfTypingTimer;
    ContactInfo *infoDialog;
    KAction *requestForAuthorization;

    Private()
        : msgManager(NULL)
        , typingTimer(0)
        , myselfTypingTimer(0)
        , infoDialog(0) {
    }

};

MrimContact::MrimContact( Kopete::Account* _account, const QString &uniqueName,
                          const QString &displayName,
                          Kopete::MetaContact *parent )
    : Kopete::Contact( _account, uniqueName, parent, QString("mrim_protocol") )
    , d(new Private)
{
    kDebug()<< " uniqueName: " << uniqueName << ", displayName: " << displayName;

    QTimer::singleShot( 10 * 1000, this, SLOT(slotLoadAvatar()) );

    d->requestForAuthorization = new KAction( KIcon("mail-reply-sender"), tr( "(Re)request Authorization From" ), this );
    connect( d->requestForAuthorization, SIGNAL(triggered(bool)), this, SLOT(slotPerformRequestForAuthorization()) );

}

MrimContact::~MrimContact() {
    delete d;
}

void MrimContact::slotPerformRequestForAuthorization() {
    kWarning() << __PRETTY_FUNCTION__;
    MrimAccount *a = dynamic_cast<MrimAccount*>( account() );
    a->requestForAuthorization(contactId() );
}

QList<KAction *> *MrimContact::customContextMenuActions( Kopete::ChatSession *manager ) {
    kWarning() << __PRETTY_FUNCTION__;
    Q_UNUSED(manager)
    QList<KAction *> *list = new QList<KAction *>();
    list->append(d->requestForAuthorization);

    return list;
}

Kopete::ChatSession* MrimContact::manager( CanCreateFlags canCreateFlags )
{
    kWarning() << __PRETTY_FUNCTION__;

    if ( d->msgManager )
    {
        return d->msgManager;
    }
    else if ( canCreateFlags == CanCreate )
    {
        QList<Kopete::Contact*> contacts;

        contacts.append(this);

        Kopete::ChatSession::Form form = ( Kopete::ChatSession::Small );

        d->msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol(), form );

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
    kDebug();

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

    msg.setManager( manager(CanCreate) );

    Kopete::ChatSession *session = manager(CanCreate);
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

    kWarning() << __PRETTY_FUNCTION__;

    loadUserInfo();
}

void MrimContact::slotUserInfoLoaded(const MRAContactInfo &info) {

    kWarning() << __PRETTY_FUNCTION__;

    emit userInfoLoaded(info);
}

void MrimContact::loadUserInfo() {

    kWarning() << __PRETTY_FUNCTION__;

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
        kWarning() << "empty!" << contactId();
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
    a->deleteContact( contactId() );

    Kopete::Contact::deleteContact();
}

#include "mrimcontact.moc"
