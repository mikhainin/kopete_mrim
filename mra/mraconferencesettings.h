#ifndef MRACONFERENCESETTINGS_H
#define MRACONFERENCESETTINGS_H

#include <QStringList>

class MRAConferenceSettings
{
public:
    MRAConferenceSettings();

    const QStringList &getContactList() const;
    void setContactList(const QStringList&arg);

    const QString &title() const;
    void setTitle(const QString &arg);

private:
    QStringList list;
    QString chatTitle;
};

#endif // MRACONFERENCESETTINGS_H
