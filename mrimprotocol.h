#ifndef kopetemrim_H
#define kopetemrim_H

// #include <QtCore/QObject>
#include <kopete/kopeteprotocol.h>

/**
 * @brief This Represents the MRIM (Mail.Ru Instant Mmessaging) Protocol
 *
 * Encapsulates the generic actions associated with this protocol
 * Usually, there is only a single instance of this class at any time
 *
 * @author Galanin Mikhail <bespoleznyak\@narod.ru>
 */

class MrimProtocol : public Kopete::Protocol
{
Q_OBJECT
public:
    MrimProtocol(QObject *parent, const QStringList &args);
    virtual ~MrimProtocol();
    
    /**
	 * Convert the serialised data back into a MrimContact and add this
	 * to its Kopete::MetaContact
	 */
	virtual Kopete::Contact *deserializeContact(
			Kopete::MetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);

	/**
	 * @brief Generate an Add Contact Page (not actually useful)
	 *
	 * As you cannot actually add an contact, this basically brings up an ugly message
	 *
	 * @param account is the account to add contact to
	 * @param parent The parent of the 'to be returned' widget
	 *
	 * @return The Add Contact Page Widget
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );

	/**
	 * @brief Generate an Edit Account Page
	 *
	 * Generate the widget needed to add/edit accounts for this protocol
	 *
	 * @param account is the account to edit. If @c NULL, a new account is made
	 * @param parent The parent of the 'to be returned' widget
	 *
	 * @return The Edit Account Page Widget
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );

	/**
	 * @brief Create a New Account
	 *
	 * This will Generate a MrimAccount
	 * @param accountId A Unique String to identify the Account
	 * @return The Newly Created Account
	 */
	virtual Kopete::Account * createNewAccount( const QString &accountId );

	/**
	 * @brief Access the instance of this protocol
	 * @return The Instance of this protocol
	 */
	static MrimProtocol *protocol();

	/**
	 * Represents contacts that are Online
	 */
	const Kopete::OnlineStatus mrimOnline;
	/**
	 * Represents contacts that are Away
	 */
	const Kopete::OnlineStatus mrimAway;
	/**
	 * Represents contacts that are Offline
	 */
	const Kopete::OnlineStatus mrimOffline;
    /**
	 * Represents contacts that are Away
	 */
	const Kopete::OnlineStatus mrimUnknown;
protected:
	static MrimProtocol *s_protocol;
};

#endif // kopetemrim_H
