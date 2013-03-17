#include <kgenericfactory.h>
#include <kdebug.h>

#include <kopeteaccountmanager.h>
#include <kopeteaccount.h>

#include "ui/mrimeditaccountwidget.h"
#include "ui/mrimaddcontactpage.h"

#include "debug.h"
#include "mrimcontact.h"
#include "mrimaccount.h"
#include "mrimprotocol.h"
#include "mra/mra_proto.h"

typedef KGenericFactory<MrimProtocol> MrimProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_mrim, MrimProtocolFactory( "kopete_mrim" )  )



MrimProtocol *MrimProtocol::s_protocol = NULL;


MrimProtocol::MrimProtocol(QObject *parent, const QStringList &)
    : Kopete::Protocol(MrimProtocolFactory::componentData(), parent)

    , mrimOnline(  Kopete::OnlineStatus::Online, 25, this, STATUS_ONLINE,  QStringList(QString()),
                 i18n( "Online" ),   i18n( "O&nline" ), Kopete::OnlineStatusManager::Online )

    , mrimAway(  Kopete::OnlineStatus::Away, 25, this, STATUS_AWAY, QStringList(QLatin1String("msn_away")),
                 i18nc( "This Means the User is Away", "Away" ),   i18nc("This Means the User is Away", "&Away" ),
                 Kopete::OnlineStatusManager::Away )

    , mrimOffline(  Kopete::OnlineStatus::Offline, 25, this, STATUS_OFFLINE,  QStringList(QString()),
                 i18n( "Offline" ),   i18n( "O&ffline" ), Kopete::OnlineStatusManager::Offline )

    , mrimDontDistrub( Kopete::OnlineStatus::Busy, 25, this, STATUS_DONT_DISTRUB, QStringList( QString() ),
                       i18nc( "This Means the User is Busy", "Busy" ),   i18nc("This Means the User is Busy", "&Busy" ),
                       Kopete::OnlineStatusManager::Busy )

    , mrimUnknown( Kopete::OnlineStatus::Unknown, 25, this, STATUS_UNDETERMINATED, QStringList( "status_unknown" ),
                   i18n( "Unknown" ) )

    , propPhoto( Kopete::Global::Properties::self()->photo() )
{

    KDebug::registerArea("");

    setCapabilities( Kopete::Protocol::CanSendOffline );

    s_protocol = this;
}

MrimProtocol::~MrimProtocol()
{
}

Kopete::Contact *MrimProtocol::deserializeContact(
    Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
    const QMap<QString, QString> &/* addressBookData */)
{
    QString contactId = serializedData[ "contactId" ];
    QString accountId = serializedData[ "accountId" ];
    QString displayName = serializedData[ "displayName" ];
    int flags  =  serializedData[ "flags" ].toInt();

    QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( this );
    Kopete::Account* account = 0;
    foreach( Kopete::Account* acct, accounts )
    {
        if ( acct->accountId() == accountId )
            account = acct;
    }

    if ( !account )
    {
        kDebug(kdeDebugArea()) << "Account doesn't exist, skipping";
        return 0;
    }

    MrimContact * contact = new MrimContact(account, contactId, displayName, flags, metaContact);
    return contact;
}

AddContactPage * MrimProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * account )
{
    return new MrimAddContactPage( parent, dynamic_cast<MrimAccount *>(account) );
}

KopeteEditAccountWidget * MrimProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
    return new MrimEditAccountWidget( parent, account );
}

Kopete::Account *MrimProtocol::createNewAccount( const QString &accountId )
{
    return new MrimAccount( this, accountId );
}

MrimProtocol *MrimProtocol::protocol()
{
    return s_protocol;
}



#include "mrimprotocol.moc"
