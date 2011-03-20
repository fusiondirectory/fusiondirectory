# Define Packagename, e.g.:
# rpmbuild --rebuild --define 'sourcename fusiondirectory' fusiondirectory.srpm
%{!?sourcename:%define sourcename %{name}-%{version}}

#
# Distribution
#
Summary: 		Web Based LDAP Administration Program 
Name:			FusionDirectory
Version: 		1.0
Release:		1
License: 		GPLv2
Source0: 		http://download.fusiondirectory.org/sources/%{sourcename}.tar.gz
URL: 			http://www.fusiondirectory.org
Group: 			System/Administration
Buildarch: 		noarch
Patch0:                 01_fix_smarty_location.patch
Patch1:                 02_fix_template_location.patch
Patch2:                 03_fix_class_mapping.patch
Patch3:                 04_fix_locale_location.patch
Patch4:                 05_fix_online_help_location.patch
Patch5:                 06_hook_password_class_plugin.patch
Patch6:                 07_hook_password_function.patch
Requires: 		httpd,php,php-ldap,php-imap,php-snmp,php-mysql,php-mbstring,ImageMagick,perl-Crypt-SmbHash
BuildRoot: 		%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: 		desktop-file-utils
BuildArch:		noarch

########################

%define confdir 	%{_sysconfdir}/%{name}
%define apacheuser apache 
%define apachegroup apache 
%define webconf	%{_sysconfdir}/httpd/conf.d/	
%define docdir /usr/share/doc/fusiondirectory-%{version}

%description
FusionDirectory is a combination of system-administrator and end-user web
interface, designed to handle LDAP based setups.
Provided is access to posix, shadow, samba, proxy, fax, and kerberos
accounts. It is able to manage the postfix/cyrus server combination
and can write user adapted sieve scripts.

%description -l fr
FusionDirectory est un ensemble d'outils WEB pour administrateurs systeme et
utilisateurs finaux permettant de gerer des configurations basees sur
un annuaire LDAP.
FusionDirectory permet de gerer des comptes de type Posix, Shadow, Samba, Proxy,
Fax et Kerberos.
Il est egalement possible de gerer des serveurs Postfix/Cyrus et 
de produire des scripts bases sur Sieve.

########################

%package dev
Group:                  System/Administration
Summary:                FusionDirectory development utilities
Requires:               php-cli,latex2html,lyx
Obsoletes:              gosa-ldap

%description dev
This package contains a couple of tools to generate
online help, extract localisations and aid developing.

########################

%package desktop
Group:                  System/Administration
Summary:                Desktop integration for FusionDirectory
Requires:               webclient
Obsoletes:              gosa-ldap

%description desktop
This package includes a menu definition for your
desktop environment.

########################

%package schema
Group: 			System/Administration
Summary: 		Schema Definitions for the FusionDirectory package
Requires:		openldap-servers	
Obsoletes:		gosa-ldap

%description schema
Contains the Schema definition files for the FusionDirectory admin package.

########################

%package help-en
Group: 			System/Administration
Summary: 		English online manual for FusionDirectory package
Requires:		fusiondirectory >= %{version}

%description help-en
English online manual page for FusionDirectory package

########################

%package help-de
Group: 			System/Administration
Summary: 		German localized online manual for FusionDirectory package
Requires:		fusiondirectory >= %{version}

%description help-de
German localized online manual page for FusionDirectory package

########################

%package help-fr
Group: 			System/Administration
Summary: 		French localized online manual for FusionDirectory package
Requires:		fusiondirectory >= %{version}

%description help-fr
French localized online manual page for FusionDirectory package

########################

%package help-nl
Group: 			System/Administration
Summary: 		Dutch localized online manual for FusionDirectory package
Requires:		fusiondirectory >= %{version}

%description help-nl
Dutch localized online manual page for FusionDirectory package

########################

%package help-es
Group: 			System/Administration
Summary: 		Spain localized online manual for FusionDirectory package
Requires:		fusiondirectory >= %{version}

%description help-es
Spain localized online manual page for FusionDirectory package

########################

%prep
%setup -q -n %{sourcename}
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3 -p1

find . -depth -name CVS -type d | xargs rm -rf

########################

%build

########################

%install
# Create buildroot
mkdir -p %{buildroot}/usr/share/fusiondirectory

# Create files for temporary stuff
for i in compile config cache; do \
  mkdir -p %{buildroot}/var/spool/fusiondirectory/$i ; \
done
mkdir -p %{buildroot}/var/cache/fusiondirectory

# Copy
DIRS="doc ihtml plugins html include locale setup"
echo `pwd`
for i in $DIRS; do \
  cp -ua $i %{buildroot}/usr/share/fusiondirectory/$i ; \
done

# Copy files for fusiondirectory
mkdir -p %{buildroot}%{_sbindir}
mkdir -p %{buildroot}%{_sysconfdir}/fusiondirectory
mkdir -p %{buildroot}/usr/share/doc/fusiondirectory
mkdir -p %{buildroot}%{webconf}

touch %{buildroot}%{_sysconfdir}/fusiondirectory/fusiondirectory.secrets
mv contrib/fusiondirectory.conf		%{buildroot}/usr/share/doc/fusiondirectory
mv update-fusiondirectory 			%{buildroot}%{_sbindir}
mv bin/fusiondirectory-encrypt-passwords 	%{buildroot}%{_sbindir}
mv debian/fusiondirectory-apache.conf 	%{buildroot}%{webconf}
mv contrib/shells 		%{buildroot}%{_sysconfdir}/fusiondirectory
mv contrib/encodings 		%{buildroot}%{_sysconfdir}/fusiondirectory
mv contrib/openldap/slapd.conf 	%{buildroot}/usr/share/doc/fusiondirectory/slapd.conf-example
mv -f doc manual

# Cleanup manual dirs
for i in admin ; do \
  rm -rf %{buildroot}/usr/share/fusiondirectory/doc/$i ; \
done

# Remove (some) unneeded files
for i in gen_locale.sh gen_online_help.sh gen_function_list.php update.sh; do \
 rm -rf %{buildroot}/usr/share/fusiondirectory/$i ; \
done

# Cleanup lyx warnings
find %{buildroot}/usr/share/fusiondirectory -name WARNINGS |xargs rm

# Cleanup guide
rm -rf %{buildroot}/usr/share/fusiondirectory/doc/guide/user/*/lyx-source

# Copy default config
mkdir -p %{buildroot}%{confdir}
mkdir -p %{buildroot}%{webconf}

# Copy file for fusiondirectory-schema
mkdir -p %{buildroot}%{_sysconfdir}/openldap/schema/fusiondirectory

mv contrib/openldap/*.schema %{buildroot}%{_sysconfdir}/openldap/schema/fusiondirectory

# Copy files for fusiondirectory-dev
mkdir -p %{buildroot}/usr/bin
mv update-locale %{buildroot}/usr/bin
mv update-online-help %{buildroot}/usr/bin
mv update-pdf-help %{buildroot}/usr/bin

# Copy files for desktop
mkdir -p %{buildroot}%{_sysconfdir}/fusiondirectory
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}%{_sbindir}
mkdir -p %{buildroot}/usr/share/pixmaps
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/man/man1/
mkdir -p %{buildroot}/usr/share/man/man5/

mv contrib/desktoprc 		%{buildroot}%{_sysconfdir}/fusiondirectory
mv contrib/fusiondirectory 		%{buildroot}/usr/bin
mv debian/fusiondirectory.xpm 		%{buildroot}/usr/share/pixmaps
mv debian/fusiondirectory-16.xpm 		%{buildroot}/usr/share/pixmaps
desktop-file-install --dir=%{buildroot}/usr/share/applications	desktop/fusiondirectory-desktop.desktop 

# Gzip manpages from source
for x in update-fusiondirectory.1 update-locale.1 update-online-help.1 update-pdf-help.1 fusiondirectory-encrypt-passwords.1
do
	gzip $x
done

# Copy manpages
mv ./*.1.gz 			%{buildroot}/usr/share/man/man1/
gzip -c contrib/fusiondirectory.1 > contrib/fusiondirectory.1.gz
mv contrib/fusiondirectory.1.gz 		%{buildroot}/usr/share/man/man1/
gzip -c contrib/fusiondirectory.conf.5 > contrib/fusiondirectory.conf.5.gz
mv contrib/fusiondirectory.conf.5.gz 		%{buildroot}/usr/share/man/man5/

mkdir -p %{buildroot}/usr/share/doc/fusiondirectory-%{version}
rm -rf %{buildroot}/usr/share/fusiondirectory/contrib

########################

%clean
rm -rf %{buildroot}

########################

%post
%{_sbindir}/update-fusiondirectory

########################

%pre
# Cleanup compile dir on updates, always exit cleanly even on errors
[ -d /var/spool/fusiondirectory ] && rm -rf /var/spool/fusiondirectory/* ; exit 0

########################

%postun
# Remove temporary files, just to be sure
[ -d /var/spool/fusiondirectory ] && rm -rf /var/spool/fusiondirectory ; exit 0
[ -d /usr/share/fusiondirectory ] && rm -rf /usr/share/fusiondirectory ; exit 0

########################

%files
%defattr(-,root,root)
%doc %attr(-,root,root) AUTHORS README README.safemode Changelog COPYING INSTALL FAQ CODING
%config %attr(-,root,root) /usr/share/doc/fusiondirectory/fusiondirectory.conf
#%attr(-,root,root) /contrib/openldap
%config %attr(-,root,root) /usr/share/doc/fusiondirectory/slapd.conf-example
%attr(755,root,root) %{_sbindir}/update-fusiondirectory
%attr(755,root,root) /usr/share/man/man1/fusiondirectory-encrypt-passwords.1.gz
%attr(755,root,root) /usr/share/man/man1/update-fusiondirectory.1.gz
%attr(755,root,root) /usr/share/man/man5/fusiondirectory.conf.5.gz
%attr(644,root,root) %{_sysconfdir}/fusiondirectory/shells
%attr(644,root,root) %{_sysconfdir}/fusiondirectory/encodings
%attr(755,root,root) %{_sbindir}/fusiondirectory-encrypt-passwords
%config(noreplace) %attr(0644,root,root) %{webconf}/fusiondirectory-apache.conf
%attr(0700, %{apacheuser}, %{apachegroup}) /var/spool/fusiondirectory
%attr(0755, root,root) /usr/share/fusiondirectory
#%attr(0755, root,root) /usr/share/fusiondirectory/html
#%attr(0755, root,root) /usr/share/fusiondirectory/ihtml
#%attr(0755, root,root) /usr/share/fusiondirectory/include
#%attr(0755, root,root) /usr/share/fusiondirectory/locale
#%attr(0755, root,root) /usr/share/fusiondirectory/plugins
#%attr(0755, root,root) /usr/share/fusiondirectory/setup
%attr(0755, root,root) /usr/share/fusiondirectory/doc/core/guide.xml
%attr(0755, root,root) /var/cache/fusiondirectory
%attr(0700, root,root) %{_sysconfdir}/fusiondirectory/fusiondirectory.secrets

########################

%files dev
%defattr(-,root,root)
/usr/bin
%attr(755,root,root) /usr/share/man/man1/update-locale.1.gz
%attr(755,root,root) /usr/share/man/man1/update-online-help.1.gz
%attr(755,root,root) /usr/share/man/man1/update-pdf-help.1.gz

########################

%files desktop
%defattr(-,root,root)
%{_sysconfdir}/fusiondirectory
/usr/bin
/usr/share/pixmaps
/usr/share/applications
%attr(755,root,root) /usr/share/man/man1/fusiondirectory.1.gz

########################

%files schema
%defattr(-,root,root)
%doc COPYING AUTHORS README contrib/openldap
%{_sysconfdir}/openldap/schema/fusiondirectory

########################

%files help-en
%defattr(-,root,root)
/usr/share/fusiondirectory/doc/core/en

########################

%files help-de
%defattr(-,root,root)
/usr/share/fusiondirectory/doc/core/de

########################

%files help-fr
%defattr(-,root,root)
/usr/share/fusiondirectory/doc/core/fr

########################

%files help-nl
%defattr(-,root,root)
/usr/share/fusiondirectory/doc/core/nl

########################

%files help-es
%defattr(-,root,root)
/usr/share/fusiondirectory/doc/core/es

########################

%changelog
* Fri Nov 17 2008 Stefan Japes <japes@GONICUS.de>
- First build of FusionDirectory 2.6 as an RPM, should work on SuSE and RedHat
