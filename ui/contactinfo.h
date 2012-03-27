#ifndef CONTACTINFO_H
#define CONTACTINFO_H

#include <kdialog.h>
#include "mra/mraprotocol.h"
namespace Ui {
class ContactInfo;
}

class MrimAccount;
class MrimContact;

class ContactInfo : public KDialog
{
    Q_OBJECT

public:
    explicit ContactInfo(MrimAccount *account, MrimContact *contact, QWidget * parent = 0);
    ~ContactInfo();
public slots:
    void slotUserInfoLoaded(const contact_info_t &info);
private:
    Ui::ContactInfo *ui;
    QWidget * m_mainWidget;
};

#endif // CONTACTINFO_H
