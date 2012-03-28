#ifndef MRACONTACTINFO_H
#define MRACONTACTINFO_H

class QString;

class MRAContactInfo
{
public:
    MRAContactInfo();
    ~MRAContactInfo();

    void setParam(const QString &name, const QString &value);

    QString email() const;

    const QString &nick() const;

    const QString &firstName() const;

    const QString &lastName() const;

    bool sex() const; // :-)

    const KDateTime &birthday() const;

    const QString &zodiac() const;

    const QString &country() const;

    const QString &city() const;

    const QString &location() const;

    const QString &phone() const;

private:
   class MRAContactInfoPrivate;
   MRAContactInfoPrivate *d;
};

#endif // MRACONTACTINFO_H
