#include <QSet>

#include "mraconferencesettings.h"

MRAConferenceSettings::MRAConferenceSettings()
{
}

const QStringList &MRAConferenceSettings::contactList() const
{
    return list;
}

void MRAConferenceSettings::setContactList(const QStringList&arg)
{
    list = arg;
}

void MRAConferenceSettings::addContact(const QString &contact) {
    list.append(contact);
}

const QString &MRAConferenceSettings::title() const
{
    return chatTitle;
}

void MRAConferenceSettings::setTitle(const QString &arg)
{
    chatTitle = arg;
}

bool MRAConferenceSettings::operator == (const MRAConferenceSettings &a) const {
    return title() == a.title() && list == a.contactList();
}

bool MRAConferenceSettings::operator != (const MRAConferenceSettings &a) const {
    return ! (*this == a);
}

MRAConferenceSettings::difference
    MRAConferenceSettings::compare(const MRAConferenceSettings &oldSettings,
                          const MRAConferenceSettings &newSettings) {

    difference result;

    if (oldSettings.title() != newSettings.title()) {
        result.newTitle = newSettings.title();
    }

    if ( oldSettings.contactList() != newSettings.contactList() ) {
        result.deletedItems =
                oldSettings.contactList().toSet().subtract(
                        newSettings.contactList().toSet()
                    ).toList();

        result.newItems =
                newSettings.contactList().toSet().subtract(
                        oldSettings.contactList().toSet()
                    ).toList();
    }

    return result;

}
