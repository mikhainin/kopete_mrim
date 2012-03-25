#include <kgenericfactory.h>
#include <kdebug.h>

#include <kopete/kopeteaccountmanager.h>
#include <kopete/kopeteaccount.h>

#include "mrimcontact.h"
#include "mrimaddcontactpage.h"
#include "mrimeditaccountwidget.h"
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

    , mrimUnknown( Kopete::OnlineStatus::Unknown, 25, this, STATUS_UNDETERMINATED, QStringList( "status_unknown" ),
                   i18n( "Unknown" ) )
{
    kWarning() << __PRETTY_FUNCTION__;
    s_protocol = this;
}

MrimProtocol::~MrimProtocol()
{
    kWarning() << __PRETTY_FUNCTION__;
}

Kopete::Contact *MrimProtocol::deserializeContact(
    Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
    const QMap<QString, QString> &/* addressBookData */)
{
    kWarning() << __PRETTY_FUNCTION__;
    QString contactId = serializedData[ "contactId" ];
    QString accountId = serializedData[ "accountId" ];
    QString displayName = serializedData[ "displayName" ];

    QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( this );
    Kopete::Account* account = 0;
    foreach( Kopete::Account* acct, accounts )
    {
        if ( acct->accountId() == accountId )
            account = acct;
    }

    if ( !account )
    {
        kWarning() << "Account doesn't exist, skipping";
        return 0;
    }

    MrimContact * contact = new MrimContact(account, contactId, displayName, metaContact);
    return contact;
}

AddContactPage * MrimProtocol::createAddContactWidget( QWidget *parent, Kopete::Account * account )
{
    kWarning()<< "Creating Add Contact Page";
    return new MrimAddContactPage( parent, dynamic_cast<MrimAccount *>(account) );
}

KopeteEditAccountWidget * MrimProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
    kWarning() << "Creating Edit Account Page";
    return new MrimEditAccountWidget( parent, account );
}

Kopete::Account *MrimProtocol::createNewAccount( const QString &accountId )
{
    kWarning() << __PRETTY_FUNCTION__;
    return new MrimAccount( this, accountId );
}

MrimProtocol *MrimProtocol::protocol()
{
    kWarning() << __PRETTY_FUNCTION__;
    return s_protocol;
}



#include "mrimprotocol.moc"
