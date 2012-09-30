%define name    kopete-mrim
%define version 0.2.0
%define release 00beta2

# Macros for in the menu-file.
%define section Networking
%define title   Chat

# SUMMARY is a SHORT one-line description of the rpm (max. 60
# characters).  Do NOT include the package name (or version number of
# the software) in the comment.  Do NOT start with an uppercase letter
# unless semantically significant, and do NOT end with a period.  DON'T
# EVER START WITH AN INDETERMINATE ARTICLE SUCH AS `a' or `as'; remove
# the article altogether.
%define Summary MRIM (aka mail.ru agent) protocol support for Kopete

Summary:        %Summary
Name:           %name
Version:        %version
Release:        %release
License:        BSD
Group:          Chat 
URL:            https://github.com/negram/kopete_mrim

# https://github.com/downloads/negram/kopete_mrim/kopete-mrim_0.2.0.orig.tar.gz
Source0:        kopete-mrim_0.2.0.orig.tar.gz

BuildRoot:      %_tmppath/%name-buildroot

# Use ldd afterwards to check for missing Requires/Buildrequires.
# DO NOT ADD IMPLICIT DEPENDENCIES; emample:
# Buildrequires: gtk+-devel XFree86-devel <- wrong!
# gtk+-devel depends on XFree86-devel so you don't have to mention XFree86-devel.
Buildrequires:       kdenetwork4-devel cmake

# rpmbuild finds requirements automatically
#Requires:            kdenetwork4

%description
# Put the description here.  Most of the time you can get a good
# description from the homepage.  Make sure it is terse and to the
# point.
Mail.ru@agent protocol support for Kopete


%prep
%setup -q


%build
%cmake_kde4
%make


%install
rm -rf %buildroot
%makeinstall_std -C build

%post
%update_menus

%postun
%clean_menus


%clean
rm -rf %buildroot


%files
%defattr(0755,root,root,0755)
%{_kde_libdir}/kde4/*
%defattr(0644,root,root,0755)
%{_kde_services}/*.desktop
%{_kde_appsdir}/*
# Add all documentation-files that are usefull for people who install
# the package.  INSTALL may not be usefull since the rpm already
# installs everything.
# %doc COPYING LICENSE README INSTALL Changelog AUTHORS


%changelog
* Sun Sep 30 2012 Mikhail Galanin <bespoleznyak@narod.ru> 0.2.0-00beta2
- first RPM for Mandriva
