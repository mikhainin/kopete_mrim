#include <kdebug.h>
#include <kopeteaccount.h>
#include <kopetemetacontact.h>
#include <kopetegroup.h>

#include "mra/mracontactlistentry.h"

#include "addcontacttask.h"
#include "mrimaccount.h"

#include "mrimaddcontactpage.h"

MrimAddContactPage::MrimAddContactPage(QWidget *parent, MrimAccount *a) :
    AddContactPage(parent)
  , m_account(a)
{
    QVBoxLayout* l = new QVBoxLayout( this );
    QWidget* w = new QWidget();
    m_mrimAddUI.setupUi( w );
    l->addWidget( w );
}

MrimAddContactPage::~MrimAddContactPage()
{
}

bool MrimAddContactPage::apply( Kopete::Account* /*a*/, Kopete::MetaContact* m )
{

    MRAContactListEntry ce;

    ce.setAddress( m_mrimAddUI.editEmail->text() );
    ce.setNick( m_mrimAddUI.editNick->text() );


    AddContactTask *task = new AddContactTask(m_account);

    task->setGroupName( m->groups()[0]->displayName() );
    task->setNickName( m_mrimAddUI.editNick->text() );
    task->setEmail( m_mrimAddUI.editEmail->text() );
    task->setMetaContact( m );
    task->run();

    return true;
}

bool MrimAddContactPage::validateData()
{
    if (m_mrimAddUI.editEmail->text().isEmpty()) {
        return false;
    }

    if (m_mrimAddUI.editNick->text().isEmpty()) {
        /// TODO: resolve nickname
        m_mrimAddUI.editNick->setText(m_mrimAddUI.editEmail->text());
    }

    return true;
}


#include "mrimaddcontactpage.moc"
