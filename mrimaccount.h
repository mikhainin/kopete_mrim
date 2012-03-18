#ifndef MRIMACCOUNT_H
#define MRIMACCOUNT_H

#include <kopeteaccount.h>

class MrimProtocol;
class MRAProtocol;
class MRAContactList;

namespace Kopete {
    class Message;
}

class MrimAccount : public Kopete::Account
{
    Q_OBJECT
public:
    explicit MrimAccount( MrimProtocol *parent, const QString& accountID );
    ~MrimAccount();
    
    /**
	 * @brief Create a New Contact
	 *
	 * Creates a protocol specific Kopete::Contact subclass and adds it to the supplied
	 * Kopete::MetaContact
	 * This is called internally only, as contacts cannot be added manually
	 *
	 * @return @c true if the contact is created, @c false otherwise
	 */
	virtual bool createContact(const QString& contactId, Kopete::MetaContact* parentContact);

	/**
	 * @brief Called when Kopete is set globally away
	 *
	 * @todo FIXME: This Doesn't Do Anything Right Now
	 */
	virtual void setAway(bool away, const QString& reason);

    /**
	 * @brief Parse The Config File
	 *
	 * This Function parses the appropriate group in the kopeterc
	 * It sets come internally used constants like @ref username, @ref emailAddress
	 */
	void parseConfig();
    
	/**
	 * @brief Called when Kopete status is changed globally
	 *
	 * @todo FIXME: This Only Makes us go online or offline, we cannot go away
	 */
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                             const OnlineStatusOptions& options = None);
	virtual void setStatusMessage(const Kopete::StatusMessage& statusMessage);
	
	/**
	 * @brief 'Connect' to the MRIM service.
	 *
	 * This the one stop call to do everything, like starting server, publishing, discovery, etc
	 * This will clear the contact list in the beginning, and start re populating it
	 * @todo Do something with initialStatus
	 *
	 * @param initialStatus
	 */
	virtual void connect( const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus() );

	/**
	 * @brief Disconnect from the service.
	 *
	 * This will disconnect from the service. It will stop everything started in connect
	 * It cleans out the contact list after it is finished
	 */
	virtual void disconnect();

    
    
    Q_PROPERTY(QByteArray username READ getUsername WRITE setUsername)
    
    void setUsername(const QByteArray &arg);
    const QByteArray getUsername() const;
    
    
    Q_PROPERTY(QByteArray password READ getPassword WRITE setPassword)
    
    void setPassword(const QByteArray &arg);
    const QByteArray getPassword() const;
    
    void sendMessage(const QString &to, const QString &text);
    
signals:
    
public slots:
    
    /**
	 * @brief Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOnline();
	/**
	 * @brief Change the account's status.  Called by KActions and internally.
	 *
	 * @todo FIXME: This Does Nothing Useful
	 */
	void slotGoAway();

	/**
	 * @brief Change the account's status.  Called by KActions and internally.
	 */
	void slotGoOffline();
    
    void slotReceivedMessage( const QString &from, const QString &text );
    
private slots:
    
    void slotContactListReceived(const MRAContactList &list);
private:
    QByteArray username;
    QByteArray password;
    Kopete::Group *group;
    MRAProtocol *m_mraProto;
    
    Kopete::OnlineStatus mrimStatusToKopete(int mrimStatus);
};

#endif // MRIMACCOUNT_H
