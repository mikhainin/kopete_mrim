#include <kdebug.h>
#include <kopeteaccount.h>
#include <kuser.h>
#include <kpimidentities/identity.h>
#include <kpimidentities/identitymanager.h>

#include "mrimaccount.h"
#include "mrimprotocol.h"
#include "mrimeditaccountwidget.h"

MrimEditAccountWidget::MrimEditAccountWidget( QWidget* parent, Kopete::Account* account )
    : QWidget(parent)
    , KopeteEditAccountWidget(account)
{
    kWarning() << __PRETTY_FUNCTION__;
    
    
    QVBoxLayout *layout = new QVBoxLayout( this );
    QWidget *widget = new QWidget( this );
    
	m_preferencesWidget.setupUi( widget );

	if (account) {
		group = account->configGroup();
	
		m_preferencesWidget.kcfg_username->setText(group->readEntry("username"));
		m_preferencesWidget.kcfg_password->setText(group->readEntry("password"));
//		m_preferencesWidget.kcfg_lastName->setText(group->readEntry("lastName"));
//		m_preferencesWidget.kcfg_emailAddress->setText(group->readEntry("emailAddress"));
	} else {

		// In this block, we populate the default values
		QString password, login;
		QStringList names;

		// Create a KUser object with default values
		// We May be able to get username and Real Name from here
		KUser user = KUser();

		if (user.isValid()) {
			// Get the login name from KUser
			login = user.loginName();

			// First Get the Names from KUser
			names = user.property(KUser::FullName).toString().split(' ');
		}

		// Next try via the default identity
		KPIMIdentities::IdentityManager manager(true);
		const KPIMIdentities::Identity & ident = manager.defaultIdentity();

		if (! ident.isNull()) {
			// Get the full name from identity (only if not available via KUser)
			if ( names.isEmpty() )
				names = ident.fullName().split(' ');

			// Get the email address
			// emailAddress = ident.emailAddr();
		}

		// Split the names array into firstName and lastName
		/*
        if (! names.isEmpty()) {
			firstName = names.takeFirst();
			lastName = names.join(" ");
		}
        */

		if (! login.isEmpty())
			m_preferencesWidget.kcfg_username->setText(login);
		if (! password.isEmpty())
			m_preferencesWidget.kcfg_password->setText(password);
        /*
		if (! lastName.isEmpty())
			m_preferencesWidget.kcfg_lastName->setText(lastName);
		if (! emailAddress.isEmpty())
			m_preferencesWidget.kcfg_emailAddress->setText(emailAddress);
         */

	}
    layout->addWidget( widget );
}

MrimEditAccountWidget::~MrimEditAccountWidget()
{
    kWarning() << __PRETTY_FUNCTION__;
}

Kopete::Account* MrimEditAccountWidget::apply() {
    kWarning() << __PRETTY_FUNCTION__;
    
    if (! account() ) {
		setAccount( new MrimAccount ( MrimProtocol::protocol(), m_preferencesWidget.kcfg_username->text()));
        kWarning() << "Write Group!";
	}

	account()->configGroup()->writeEntry("username", m_preferencesWidget.kcfg_username->text());
	account()->configGroup()->writeEntry("password", m_preferencesWidget.kcfg_password->text());
//	group->writeEntry("lastName", m_preferencesWidget->kcfg_lastName->text());
//	group->writeEntry("emailAddress", m_preferencesWidget->kcfg_emailAddress->text());

	((MrimAccount *)account())->parseConfig();

	return account();
}

bool MrimEditAccountWidget::validateData() {
    kWarning() << __PRETTY_FUNCTION__;
    return true;
}
