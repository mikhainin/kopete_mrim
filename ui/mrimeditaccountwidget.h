#ifndef MRIMEDITACCOUNTWIDGET_H
#define MRIMEDITACCOUNTWIDGET_H

#include <QWidget>
#include <kopete/ui/editaccountwidget.h>
#include <KConfigGroup>
#include "ui_mrimaccountpreferences.h"

class MrimEditAccountWidget : public QWidget, public KopeteEditAccountWidget
{
    Q_OBJECT
public:
    explicit MrimEditAccountWidget( QWidget* parent, Kopete::Account* account );
    ~MrimEditAccountWidget();

	/**
	 * Make an account out of the entered data
	 */
	virtual Kopete::Account* apply();
	/**
	 * Is the data correct?
	 */
	virtual bool validateData();
protected:
	Ui_MrimAccountPreferences m_preferencesWidget;
	KConfigGroup *group;
    
signals:
    
public slots:
    
};

#endif // MRIMEDITACCOUNTWIDGET_H
