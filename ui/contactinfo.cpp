#include <kdebug.h>
#include <QList>
#include <QPair>
#include <QString>

#include "mrimaccount.h"
#include "mrimcontact.h"
#include "mra/mraavatarloader.h"
#include "mra/mracontactinfo.h"

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

    QObject::connect(contact, SIGNAL(userInfoLoaded(MRAContactInfo)),
            this, SLOT(slotUserInfoLoaded(MRAContactInfo)) );

    QObject::connect(d->ui.buttonRefreshPhoto, SIGNAL(clicked()), this, SLOT(slotRefreshAvatar()));

    slotRefreshAvatar();

    show ();
}

ContactInfo::~ContactInfo()
{
    delete d;
}

void ContactInfo::slotUserInfoLoaded(const MRAContactInfo &info) {

    kWarning() << __PRETTY_FUNCTION__;

    typedef QPair<QString, QString> pair_t;
    /*
    foreach( const pair_t &item, info ) {
        d->ui.textInfo->append( item.first + ": " + item.second );
    }
    */

    d->ui.textInfo->append( QObject::tr("E-mail") + ": " + info.email() );
    d->ui.textInfo->append( QObject::tr("Nick")   + ": " + info.nick() );
    d->ui.textInfo->append( QObject::tr("First name") + ": " + info.firstName() );
    d->ui.textInfo->append( QObject::tr("Last name") + ": " + info.lastName() );
    d->ui.textInfo->append( QObject::tr("Location") + ": " + info.location() );
    d->ui.textInfo->append( QObject::tr("Birthday") + ": " + info.birthday().toString() );
    d->ui.textInfo->append( QObject::tr("Phone number") + ": " + info.phone());
    d->ui.textInfo->append( QObject::tr("Gender") + ": " + (info.sex() ? QObject::tr("male") : QObject::tr("female")) );
    // d->ui.textInfo->append( QObject::tr("E-mail") + ": " + info.email() );
    // d->ui.textInfo->append( QObject::tr("E-mail") + ": " + info.email() );

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
