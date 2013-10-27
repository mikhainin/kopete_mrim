#ifndef MRIMCHATSESSION_H
#define MRIMCHATSESSION_H

#include <kopete/kopetechatsession.h>

class MrimProtocol;
class MrimContact;

class MrimChatSession : public Kopete::ChatSession
{
    Q_OBJECT
public:
    explicit MrimChatSession(const Kopete::Contact *user,
                             MrimContact *contact,
                             Kopete::ContactPtrList others,
                             MrimProtocol *protocol,
                             Kopete::ChatSession::Form form );

    virtual ~MrimChatSession();

    /**
     * Called when contact is dragged to the chat's contact list
     * @see Kopete::ChatSession::inviteContact(QString) and Kopete::ChatSession::mayInvite(bool)
     * @brief inviteContact
     * @param contactId
     */
    virtual void inviteContact(const QString &contactId);

    void slotChatSettingsReceived(const MRAConferenceSettings &settings);
signals:

public slots:
    void slotEditConference();

private:
    class Private;
    Private *d;
};

#endif // MRIMCHATSESSION_H
