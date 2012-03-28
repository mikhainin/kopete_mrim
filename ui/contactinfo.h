#ifndef CONTACTINFO_H
#define CONTACTINFO_H

#include <kdialog.h>
#include "mra/mraprotocol.h"

namespace Ui {
class ContactInfo;
}

class MrimAccount;
class MrimContact;
class MRAAvatarLoader;

class ContactInfo : public KDialog
{
    Q_OBJECT

public:
    explicit ContactInfo(MrimAccount *account, MrimContact *contact, QWidget * parent = 0);
    ~ContactInfo();
public slots:
    void slotUserInfoLoaded(const MRAContactInfo &info);
private slots:
    void slotRefreshAvatar();
    void slotAvatarLoaded(bool success, MRAAvatarLoader *loader);
private:
    class ContactInfoPrivate;
    ContactInfoPrivate *d;
};

#endif // CONTACTINFO_H
