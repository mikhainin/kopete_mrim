#ifndef MRACONFERENCESETTINGS_H
#define MRACONFERENCESETTINGS_H

#include <QStringList>

class MRAConferenceSettings
{
public:

    struct difference {
        QString newTitle; // empty if wasn't changed
        QStringList newItems;
        QStringList deletedItems;
    };
    static difference compare(const MRAConferenceSettings &oldSettings,
                              const MRAConferenceSettings &newSettings);

    MRAConferenceSettings();

    const QStringList &contactList() const;
    void setContactList(const QStringList&arg);
    void addContact(const QString &contact);

    const QString &title() const;
    void setTitle(const QString &arg);

    bool operator != (const MRAConferenceSettings &a) const;
    bool operator == (const MRAConferenceSettings &a) const;

private:
    QStringList list;
    QString chatTitle;
};



#endif // MRACONFERENCESETTINGS_H
