#ifndef CREATECONFERENCEDIALOG_H
#define CREATECONFERENCEDIALOG_H

#include <QDialog>

namespace Ui {
class CreateConferenceDialog;
}
class QModelIndex;
class MrimAccount;
class MRAConferenceSettings;

class CreateConferenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateConferenceDialog(MrimAccount *account, QWidget *parent = 0);
    ~CreateConferenceDialog();

    const MRAConferenceSettings &getSettings();

    void setSettings(const MRAConferenceSettings &arg);

private slots:
    void on_listAvailableContacts_doubleClicked(const QModelIndex &index);

    void on_buttonDeselectContact_clicked();

    void on_buttonSelectContact_clicked();

    void on_listSelectedContacts_doubleClicked(const QModelIndex &index);

    void on_buttonBox_accepted();


private:
    Ui::CreateConferenceDialog *ui;
    class Private;
    Private *d;
};

#endif // CREATECONFERENCEDIALOG_H
