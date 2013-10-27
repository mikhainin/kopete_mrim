#ifndef MRIMCONTACT_H
#define MRIMCONTACT_H
#include <QList>
#include <QPair>
#include <QString>

#include <kopetecontact.h>
#include <kopetechatsession.h>
#include "mra/mraprotocol.h"

class MRAOfflineMessage;
class ContactInfo;
class TransferRequestInfo;

class MrimContact : public Kopete::Contact
{
    Q_OBJECT
public:
    explicit MrimContact( Kopete::Account* _account,
                          const QString &uniqueName,
                          const QString &displayName,
                          int flags,
                          Kopete::MetaContact *parent );

    virtual ~MrimContact();

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

    virtual bool isReachable() { return true; }


    void loadUserInfo();
    void slotUserInfoLoaded(const MRAContactInfo &info);
    void setFlags(int arg);

    void loadChatMembersList();
    void slotChatSettingsReceived(const MRAConferenceSettings &settings);

    virtual QList<KAction *> *customContextMenuActions(  );
    virtual QList<KAction *> *customContextMenuActions( Kopete::ChatSession* );

    void sendFile( const KUrl &sourceURL = KUrl(),
                       const QString &fileName = QString(), uint fileSize = 0L );

    void receiveFile(const TransferRequestInfo &transferInfo);
    void receiveFileCancel(const TransferRequestInfo &transferInfo);

    /**
     * @brief isOrdinaryContact: neither chat nor phone number
     * @return
     */
    bool isOrdinaryContact() const;

    bool isChatContact() const;

    void inviteContactToConference(const QString &contactIdToInvite);
    void removeContactFromConference(const QString &contactIdToRemove);

signals:
    void userInfoLoaded(const MRAContactInfo &info);

public slots:

    void deleteContact();

    void sendMessage( Kopete::Message &message );
    void slotTypingTimeOut();
    void slotChatSessionDestroyed();
    void slotMyselfTyping(bool typing);
    void slotMyselfTypingTimeout();

    void slotLoadAvatar();
    void slotPerformRequestForAuthorization();

    void slotTransferFinished();


    /**
     * Show "info dialog" for the contact
     */
    virtual void slotUserInfo ();

    virtual void sync(unsigned int changed = 0xFF);

private:
    class Private;
    Private *d;
};

#endif // MRIMCONTACT_H
