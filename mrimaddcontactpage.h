#ifndef MRIMADDCONTACTPAGE_H
#define MRIMADDCONTACTPAGE_H

#include <QObject>
#include <ui/addcontactpage.h>
#include "ui_mrimaddui.h"

class MrimAccount;
class MrimAddContactPage : public AddContactPage
{
    Q_OBJECT
public:
    explicit MrimAddContactPage(QWidget *parent, MrimAccount *a);
    ~MrimAddContactPage();


    /**
     * Make a contact out of the entered data
     */
    virtual bool apply(Kopete::Account* a, Kopete::MetaContact* m);

    /**
     * Is the data correct?
     */
    virtual bool validateData();

protected:
    Ui_MrimAddUI m_mrimAddUI;
    MrimAccount *m_account;

signals:

public slots:

};

#endif // MRIMADDCONTACTPAGE_H
