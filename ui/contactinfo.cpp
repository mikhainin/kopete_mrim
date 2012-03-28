#include <kdebug.h>
#include <QList>
#include <QPair>
#include <QString>

#include "mrimaccount.h"
#include "mrimcontact.h"
#include "mra/mraavatarloader.h"

#include "contactinfo.h"
#include "ui_contactinfo.h"


struct ContactInfo::ContactInfoPrivate {
    QWidget * mainWidget;
    MrimAccount *account;
    MrimContact *contact;
    Ui::ContactInfo ui;
};

ContactInfo::ContactInfo(MrimAccount *account, MrimContact *contact, QWidget *parent) :
    KDialog(parent),
    d(new ContactInfoPrivate)
{
    d->account = account;
    d->contact = contact;

    QWidget* w = new QWidget(this);
    d->ui.setupUi(w);
    setMainWidget(w);

    QObject::connect(contact, SIGNAL(userInfoLoaded(contact_info_t)),
            this, SLOT(slotUserInfoLoaded(contact_info_t)) );

    QObject::connect(d->ui.buttonRefreshPhoto, SIGNAL(clicked()), this, SLOT(slotRefreshAvatar()));

    slotRefreshAvatar();

    show ();
}

ContactInfo::~ContactInfo()
{
    delete d;
}

void ContactInfo::slotUserInfoLoaded(const contact_info_t &info) {

    kWarning() << __PRETTY_FUNCTION__;

    typedef QPair<QString, QString> pair_t;
    foreach( const pair_t &item, info ) {
        d->ui.textInfo->append( item.first + ": " + item.second );
    }
}

void ContactInfo::slotRefreshAvatar() {
    d->account->loadPhoto( d->contact->contactId(), this, SLOT(slotAvatarLoaded(bool,MRAAvatarLoader*)) );
}

void ContactInfo::slotAvatarLoaded(bool success, MRAAvatarLoader *loader) {
    if (success) {
        d->ui.labelPhoto->setPixmap( QPixmap::fromImage( loader->image() ) );
    }
}

#include "contactinfo.moc"
