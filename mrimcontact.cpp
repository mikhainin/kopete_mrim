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

MrimContact::MrimContact( Kopete::Account* _account, const QString &uniqueName,
                          const QString &displayName,
                          Kopete::MetaContact *parent )
    : Kopete::Contact( _account, uniqueName, parent, QString("mrim_protocol") )
    , m_msgManager(NULL)
    , m_typingTimer(0)
    , m_myselfTypingTimer(0)
    , m_infoDialog(0)
{
    kDebug()<< " uniqueName: " << uniqueName << ", displayName: " << displayName;
    kWarning() << __PRETTY_FUNCTION__;

    QTimer::singleShot( 10 * 1000, this, SLOT(slotLoadAvatar()) );

}

Kopete::ChatSession* MrimContact::manager( CanCreateFlags canCreateFlags )
{
    kWarning() << __PRETTY_FUNCTION__;

    if ( m_msgManager )
    {
        return m_msgManager;
    }
    else if ( canCreateFlags == CanCreate )
    {
        QList<Kopete::Contact*> contacts;

        contacts.append(this);

        Kopete::ChatSession::Form form = ( Kopete::ChatSession::Small );

        m_msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol(), form );

        connect(m_msgManager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
                this, SLOT(sendMessage(Kopete::Message&)) );

        connect(m_msgManager, SIGNAL(myselfTyping(bool)),
                this, SLOT(slotMyselfTyping(bool)) );

        connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));

        return m_msgManager;
    }
    else
    {
        return 0;
    }

    return 0;
}


void MrimContact::slotChatSessionDestroyed()
{
    m_msgManager = 0;
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
    if ( m_typingTimer  ) {
        m_typingTimer->stop();
        m_typingTimer->deleteLater();
        m_typingTimer = 0;
    }

    manager(CanCreate)->receivedTypingMsg( this, true );

    m_typingTimer = new QTimer(this);
    connect(m_typingTimer, SIGNAL(timeout()), this, SLOT(slotTypingTimeOut()));
    m_typingTimer->setSingleShot(true);
    m_typingTimer->setInterval(10 * 1000);
}

void MrimContact::slotTypingTimeOut() {
    manager(CanCreate)->receivedTypingMsg( this, false );
    m_typingTimer->deleteLater();

}

void MrimContact::slotMyselfTyping(bool typing) {
    if (typing && !m_myselfTypingTimer) {
        m_myselfTypingTimer = new QTimer(this);

        connect(m_myselfTypingTimer, SIGNAL(timeout()), this, SLOT(slotMyselfTypingTimeout()));
        m_myselfTypingTimer->setInterval(10 * 1000);

        slotMyselfTypingTimeout();

    } else if ( !typing && m_myselfTypingTimer ) {
        m_myselfTypingTimer->stop();
        m_myselfTypingTimer->deleteLater();
        m_myselfTypingTimer = 0;
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

#include "mrimcontact.moc"
