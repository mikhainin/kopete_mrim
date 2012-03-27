#include <kdebug.h>
#include <QList>
#include <QPair>
#include <QString>

#include "mrimaccount.h"
#include "mrimcontact.h"

#include "contactinfo.h"
#include "ui_contactinfo.h"

ContactInfo::ContactInfo(MrimAccount *account, MrimContact *contact, QWidget *parent) :
    KDialog(parent),
    ui(new Ui::ContactInfo)
{
    QWidget* w = new QWidget(this);
    ui->setupUi(w);
    setMainWidget(w);

    QObject::connect(contact, SIGNAL(userInfoLoaded(contact_info_t)),
            this, SLOT(slotUserInfoLoaded(contact_info_t)) );

    show ();
}

ContactInfo::~ContactInfo()
{
    delete ui;
}

void ContactInfo::slotUserInfoLoaded(const contact_info_t &info) {

    kWarning() << __PRETTY_FUNCTION__;

    typedef QPair<QString, QString> pair_t;
    foreach( const pair_t &item, info ) {
        ui->textInfo->append( item.first + ": " + item.second );
    }
}

#include "contactinfo.moc"
