#include <kopete/kopetecontact.h>

#include "mrimaccount.h"
#include "mrimcontact.h"
#include "mra/mraconferencesettings.h"

#include "createconferencedialog.h"
#include "ui_createconferencedialog.h"

class CreateConferenceDialog::Private {
public:
    MrimAccount *account;
    MRAConferenceSettings settings;
};


CreateConferenceDialog::CreateConferenceDialog(MrimAccount *account, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateConferenceDialog),
    d(new Private)
{
    ui->setupUi(this);
    d->account = account;

    foreach( Kopete::Contact *contactItem, d->account->contacts() ) {

        MrimContact *c = dynamic_cast<MrimContact*>(contactItem);

        if ( c->isOrdinaryContact() ) {
            ui->listAvailableContacts->addItem(contactItem->contactId());
        }

    }
    ui->listSelectedContacts->sortItems();
}

CreateConferenceDialog::~CreateConferenceDialog()
{
    delete ui;
    delete d;
}

void CreateConferenceDialog::on_listAvailableContacts_doubleClicked(const QModelIndex &index)
{
    if ( index.isValid() ) {
        ui->listSelectedContacts->addItem( ui->listAvailableContacts->takeItem(index.row()) );
        ui->listSelectedContacts->sortItems();
    }
}

void CreateConferenceDialog::on_buttonDeselectContact_clicked()
{
    on_listSelectedContacts_doubleClicked( ui->listSelectedContacts->currentIndex() );
}

void CreateConferenceDialog::on_buttonSelectContact_clicked()
{
    on_listAvailableContacts_doubleClicked( ui->listSelectedContacts->currentIndex() );
}

void CreateConferenceDialog::on_listSelectedContacts_doubleClicked(const QModelIndex &index)
{
    if ( index.isValid() ) {
        ui->listAvailableContacts->addItem( ui->listSelectedContacts->takeItem(index.row()) );
        ui->listAvailableContacts->sortItems();
    }
}

void CreateConferenceDialog::on_buttonBox_accepted()
{

    QStringList list;
    QListWidgetItem * item = 0;
    int currentRow = 0;

    while( (item = ui->listSelectedContacts->item(currentRow)) != 0 ) {
        list.append( item->text() );
        ++currentRow;
    }

    d->settings.setContactList( list );
    d->settings.setTitle(ui->editConferenceTitle->text());

}

const MRAConferenceSettings &CreateConferenceDialog::getSettings() {
    return d->settings;
}
