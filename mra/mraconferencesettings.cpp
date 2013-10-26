#include "mraconferencesettings.h"

MRAConferenceSettings::MRAConferenceSettings()
{
}

const QStringList &MRAConferenceSettings::getContactList() const
{
    return list;
}

void MRAConferenceSettings::setContactList(const QStringList&arg)
{
    list = arg;
}

const QString &MRAConferenceSettings::title() const
{
    return chatTitle;
}

void MRAConferenceSettings::setTitle(const QString &arg)
{
    chatTitle = arg;
}
