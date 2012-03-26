#ifndef MRIMCONTACT_H
#define MRIMCONTACT_H

#include <kopetecontact.h>
#include <kopetechatsession.h>

class MRAOfflineMessage;

class MrimContact : public Kopete::Contact
{
    Q_OBJECT
public:
    explicit MrimContact( Kopete::Account* _account, const QString &uniqueName,
                          const QString &displayName,
                          Kopete::MetaContact *parent );

    /**
     * @brief Returns a Kopete::ChatSession associated with this contact
     *
     * @param canCreate If @c true, then a new manager may be created
     * @return The Contats's ChatSession Manager
     */
    virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );

    void receivedMessage( const QString &text );

    void receivedOfflineMessage( const MRAOfflineMessage &message );

    void typingMessage();

    void avatarLoaded(const QImage &avatar);

signals:

public slots:

    void sendMessage( Kopete::Message &message );
    void slotTypingTimeOut();
    void slotChatSessionDestroyed();
    void slotMyselfTyping(bool typing);
    void slotMyselfTypingTimeout();

    void slotLoadAvatar();

    /**
     * Show "info dialog" for the contact
     */
    virtual void slotUserInfo ();

private:
    Kopete::ChatSession* m_msgManager;
    QTimer *m_typingTimer;

    QTimer *m_myselfTypingTimer;
};

#endif // MRIMCONTACT_H
