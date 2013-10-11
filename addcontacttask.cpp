#include <kdebug.h>
#include <kopete/kopetemetacontact.h>
#include <kopete/kopetecontact.h>

#include "mrimaccount.h"
#include "debug.h"

#include "addcontacttask.h"


struct AddContactTask::Private {

    QString groupName;
    Kopete::MetaContact* m;
    MrimAccount *account;
    MRAProtocol *proto;
    int groupId;
    QString email;
    QString nickName;

    Private() : m(0), account(0), proto(0), groupId(0) {
    }

};

AddContactTask::AddContactTask(MrimAccount *account)
    : QObject(static_cast<QObject*>(account))
    , d(new Private)
{
    d->account = account;

    d->proto = account->getMraProtocol();
}

AddContactTask::~AddContactTask() {
   delete d;
}

void AddContactTask::setGroupName( const QString &groupName ) {
    d->groupName = groupName;
}

void AddContactTask::setMetaContact( Kopete::MetaContact* m ) {
    d->m = m;
}

void AddContactTask::setEmail( const QString &email ) {
    d->email = email;
}

void AddContactTask::setNickName( const QString &nickName ) {
    d->nickName = nickName;
}

void AddContactTask::run() {

    mrimDebug() << __PRETTY_FUNCTION__;

    if ( !d->m || d->groupName.isEmpty() ) {
        mrimWarning() << "neither metacontact nor groupName is set";
        return;
    }


    int gid = d->account->getGroupIdByName( d->groupName );

    if ( gid == -1 ) {
        d->proto->addGroupToContactList(d->groupName, this);
    } else {

        d->groupId = gid;
        d->proto->addToContactList(
                        0,
                        d->groupId,
                        d->email,
                        d->nickName,
                        d->account->myself()->contactId(),
                        tr("Please, authorize me."),
                        this
                    );
    }

}

void AddContactTask::runAddGroupWithoutContact() {

    if ( d->groupName.isEmpty() ) {
        mrimWarning() << "neither metacontact nor groupName is set";
        return;
    }

    int gid = d->account->getGroupIdByName( d->groupName );

    if ( gid == -1 ) {
        d->proto->addGroupToContactList(d->groupName, this);
    } else {
        mrimDebug() << "the group " << d->groupName << " is already exists";
    }
}

void AddContactTask::groupAddedSuccessfully() {

    d->groupId = d->account->addGroupAndReturnId(d->groupName);

    if ( d->nickName.isEmpty() ) {
        mrimDebug() << "nickname is empty => return here";
        return;
    }
    d->proto->addToContactList(
                    0,
                    d->groupId,
                    d->email,
                    d->nickName,
                    d->account->myself()->contactId(),
                    tr("Please, authorize me."),
                    this
                );
}

void AddContactTask::groupAddFailed(int status) {
    Q_UNUSED(status);
    /// TODO: implement me
}


void AddContactTask::contactAddedSuccessfully() {
    d->account->addContact(d->email, d->m, Kopete::Account::ChangeKABC);
}

void AddContactTask::contactAddFailed(int status) {
    Q_UNUSED(status);
    /// TODO: implement me
}

