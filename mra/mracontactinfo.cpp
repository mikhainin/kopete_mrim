#include <QString>
#include <kdatetime.h>

#include "mracontactinfo.h"

struct MRAContactInfo::MRAContactInfoPrivate {
    QString username;
    QString domain;
    QString nick;
    QString firstName;
    QString lastName;
    bool sex; // :-)
    KDateTime birthday;
    QString zodiac;
    QString country;
    QString city;
    QString location;
    QString phone;

    QString unknown;
/*
kopete(30394) MRAProtocol::readAnketaInfo: "mrim_status" "00000001"

kopete(30394) MRAProtocol::readAnketaInfo: "status_uri" "STATUS_ONLINE"

kopete(30394) MRAProtocol::readAnketaInfo: "status_title" "???"

kopete(30394) MRAProtocol::readAnketaInfo: "status_desc" ""

kopete(30394) MRAProtocol::readAnketaInfo: "ua_features" "00000bff"

*/
};


MRAContactInfo::MRAContactInfo()
    : d(new MRAContactInfoPrivate)
{
}

MRAContactInfo::~MRAContactInfo()
{
    delete d;
}


void MRAContactInfo::setParam(const QString &name, const QString &value) {
    if (name == "Username") {
        d->username = value;

    } else if (name == "Domain") {
        d->domain = value;

    } else if (name == "Nickname") {
        d->nick = value;

    } else if (name == "FirstName") {
        d->firstName = value;

    } else if (name == "LastName") {
        d->lastName = value;

    } else if (name == "Sex") {
        d->sex = (value == "1");

    } else if (name == "Birthday") {
        d->birthday = KDateTime::fromString( value, "%Y-%m-%d" );

    } else if (name == "Zodiac") {
        d->zodiac = value;

    } else if (name == "Country_id") {
        d->country = value;

    } else if (name == "City_ID") {
        d->city = value;

    } else if (name == "Location") {
        d->location = value;

    } else if (name == "Phone") {
        d->phone = value;
    } else {
        d->unknown += name + ": " + value + "\n";
    }
}

QString MRAContactInfo::email() const {
    return d->username + '@' + d->domain;
}

const QString &MRAContactInfo::nick() const {
    return d->nick;
}

const QString &MRAContactInfo::firstName() const {
    return d->firstName;
}

const QString &MRAContactInfo::lastName() const {
    return d->lastName;
}

bool MRAContactInfo::sex() const {
    return d->sex;
}

const KDateTime &MRAContactInfo::birthday() const {
    return d->birthday;
}

const QString &MRAContactInfo::zodiac() const {
    return d->zodiac;
}

const QString &MRAContactInfo::country() const {
    return d->country;
}

const QString &MRAContactInfo::city() const {
    return d->city;
}

const QString &MRAContactInfo::location() const {
    return d->location;
}

const QString &MRAContactInfo::phone() const {
    return d->phone;
}

