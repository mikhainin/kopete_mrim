#include <kdebug.h>
#include <kopeteaccount.h>
#include <kopetechatsessionmanager.h>
#include <QTimer>

#include "mrimaccount.h"
#include "mrimcontact.h"

MrimContact::MrimContact( Kopete::Account* _account, const QString &uniqueName,
                          const QString &displayName,
                          Kopete::MetaContact *parent )
    : Kopete::Contact( _account, uniqueName, parent, QString("mrim_protocol") )
    , m_msgManager(NULL)
    , m_typingTimer(0)
{
    kDebug()<< " uniqueName: " << uniqueName << ", displayName: " << displayName;
    kWarning() << __PRETTY_FUNCTION__;
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

        connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));

        return m_msgManager;
    }
    else
    {
        return 0;
    }

    return 0;
}


void MrimContact::sendMessage( Kopete::Message &message )
{
    kDebug();

    // Blocking Again. Upto another 3 seconds
    // connection->sendMessage(message);

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

void MrimContact::typingMessage() {
    delete m_typingTimer;

    manager(CanCreate)->receivedTypingMsg( this, true );

    m_typingTimer = new QTimer(this);
    connect(m_typingTimer, SIGNAL(timeout()), this, SLOT(slotTypingTimeOut()));
    m_typingTimer->setSingleShot(true);
    m_typingTimer->setInterval(10 * 1000);
}

void MrimContact::slotTypingTimeOut() {
    manager(CanCreate)->receivedTypingMsg( this, false );
}
