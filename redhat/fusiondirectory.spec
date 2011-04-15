# Some sort of "detection" of suse
%{?suse_version:%define suse 1}
%{!?suse_version:%define suse 0}

# Define Packagename, e.g.:
# rpmbuild --rebuild --define 'sourcename fusiondirectory' fusiondirectory.srpm
%{!?sourcename:%define sourcename %{name}-%{version}}

#
# Distribution
#
Summary: 		Web Based LDAP Administration Program 
Name:			fusiondirectory
Version: 		1.0
Release:		1
License: 		GPL
Source0:                http://download.fusiondirectory.org/sources/%{sourcename}.tar.gz
URL:                    http://www.fusiondirectory.org
Group: 			System/Administration
Vendor:			FusionDirectory project
Packager:		FusionDirectory packages maintainers group <fusiondirectory-packages@lists.fusiondirectory.org>
Buildarch: 		noarch
Patch0:                 01_fix_smarty_location.patch
Patch1:                 02_fix_template_location.patch
Patch2:                 03_fix_class_mapping.patch
Patch3:                 04_fix_locale_location.patch
Patch4:                 05_fix_online_help_location.patch
%if %{suse}
Requires:		apache2,apache2-mod_php5,php5,php5-gd,php5-ldap,php5-mcrypt,php5-mysql,php5-imap,php5-iconv,php5-hash,php5-posix,php5-mbstring,php5-gettext,ImageMagick,gettext-tools
%else
Requires: 		httpd,php,php-ldap,php-imap,php-snmp,php-mysql,php-mbstring,ImageMagick,perl-Crypt-SmbHash
%endif
BuildRoot: 		%{_tmppath}/%{name}-%{version}-root
BuildArch:		noarch

########################

%define confdir 	/etc/%{name}

%if %{suse}
	%{echo:Building SuSE rpm}
	%define apacheuser wwwrun
	%define apachegroup root
	%define webconf	/etc/apache2/conf.d/
	%define docdir /usr/share/doc/packages/fusiondirectory
%else
	%{echo:Building other rpm}
	%define apacheuser apache 
	%define apachegroup apache 
	%define webconf	/etc/httpd/conf.d/	
	%define docdir /usr/share/doc/fusiondirectory-%{version}
%endif

%description
FusionDirectory is a combination of system-administrator and end-user web
interface, designed to handle LDAP based setups.
Provided is access to posix, shadow, samba, proxy, fax, and kerberos
accounts. It is able to manage the postfix/cyrus server combination
and can write user adapted sieve scripts.

########################

%package dev
Group:                  System/Administration
Summary:                fusiondirectory development utilities
%if %{suse}
Requires:               lyx
%else
Requires:               php-cli,latex2html,lyx
%endif
Obsoletes:              gosa-ldap

%description dev
This package contains a couple of tools to generate
online help, extract localisations and aid developing.

########################

%package desktop
Group:                  System/Administration
Summary:                Desktop integration for fusiondirectory
%if %{suse}
Requires:               firefox
%else
Requires:               firefox
%endif
Obsoletes:              gosa-ldap

%description desktop
This package includes a menu definition for your
desktop environment.

########################

%package schema
Group: 			System/Administration
Summary: 		Schema Definitions for the fusiondirectory package
%if %{suse}
Requires:		openldap2	
%else
Requires:		openldap-servers
%endif
Obsoletes:		gosa-ldap

%description schema
Contains the Schema definition files for the fusiondirectory admin package.

########################

%package help-en
Group: 			System/Administration
Summary: 		English online manual for fusiondirectory package
Requires:		fusiondirectory >= %{version}

%description help-en
English online manual page for fusiondirectory package

########################

%package help-de
Group: 			System/Administration
Summary: 		German localized online manual for fusiondirectory package
Requires:		fusiondirectory >= %{version}

%description help-de
German localized online manual page for fusiondirectory package

########################

%package help-fr
Group: 			System/Administration
Summary: 		French localized online manual for fusiondirectory package
Requires:		fusiondirectory >= %{version}

%description help-fr
French localized online manual page for fusiondirectory package

########################

%package help-nl
Group: 			System/Administration
Summary: 		Dutch localized online manual for fusiondirectory package
Requires:		fusiondirectory >= %{version}

%description help-nl
Dutch localized online manual page for fusiondirectory package

########################

%package help-es
Group: 			System/Administration
Summary: 		Spain localized online manual for fusiondirectory package
Requires:		fusiondirectory >= %{version}

%description help-es
Spain localized online manual page for fusiondirectory package

########################

%prep
%setup -q -n %{sourcename}
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3 -p1
%patch4 -p1

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
mkdir -p %{buildroot}/usr/sbin
mkdir -p %{buildroot}/etc/fusiondirectory
mkdir -p %{buildroot}/usr/share/doc/fusiondirectory
mkdir -p %{buildroot}%{webconf}

touch %{buildroot}/etc/fusiondirectory/fusiondirectory.secrets
mv contrib/fusiondirectory.conf			%{buildroot}/usr/share/doc/fusiondirectory
mv update-fusiondirectory 			%{buildroot}/usr/sbin
mv bin/fusiondirectory-encrypt-passwords 	%{buildroot}/usr/sbin
mv debian/fusiondirectory-apache.conf 		%{buildroot}%{webconf}
mv contrib/shells 				%{buildroot}/etc/fusiondirectory
mv contrib/encodings 				%{buildroot}/etc/fusiondirectory
mv contrib/openldap/slapd.conf 			%{buildroot}/usr/share/doc/fusiondirectory/slapd.conf-example
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
mkdir -p %{buildroot}/etc/openldap/schema/fusiondirectory

mv contrib/openldap/*.schema %{buildroot}/etc/openldap/schema/fusiondirectory

# Copy files for fusiondirectory-dev
mkdir -p %{buildroot}/usr/bin
mv update-locale %{buildroot}/usr/bin
mv update-online-help %{buildroot}/usr/bin
mv update-pdf-help %{buildroot}/usr/bin

# Copy files for desktop
mkdir -p %{buildroot}/etc/fusiondirectory
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/sbin
mkdir -p %{buildroot}/usr/share/pixmaps
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/man/man1/
mkdir -p %{buildroot}/usr/share/man/man5/

mv contrib/desktoprc 				%{buildroot}/etc/fusiondirectory
mv contrib/fusiondirectory 			%{buildroot}/usr/bin
mv debian/fusiondirectory.xpm 			%{buildroot}/usr/share/pixmaps
mv debian/fusiondirectory-16.xpm 		%{buildroot}/usr/share/pixmaps
mv debian/fusiondirectory-desktop.desktop 	%{buildroot}/usr/share/applications

# Gzip manpages from source
for x in update-fusiondirectory.1 update-locale.1 update-online-help.1 update-pdf-help.1 fusiondirectory-encrypt-passwords.1
do
	gzip $x
done

%if %{suse}
	sed -i 's#/usr/bin/php#/usr/bin/php5#' %{buildroot}/usr/sbin/update-fusiondirectory
	sed -i 's#/usr/bin/php#/usr/bin/php5#' %{buildroot}/usr/sbin/fusiondirectory-encrypt-passwords
	cat <<-EOF >> %{buildroot}%{webconf}/fusiondirectory-apache.conf
	
	<Directory /usr/share/fusiondirectory/html>
	    Options None
	    AllowOverride None
	    Order deny,allow
	    Allow from all
	</Directory>
	EOF
%endif

# Copy manpages
mv ./*.1.gz 					%{buildroot}/usr/share/man/man1/
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
/usr/sbin/update-fusiondirectory

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
%attr(755,root,root) /usr/sbin/update-fusiondirectory
%attr(755,root,root) /usr/share/man/man1/fusiondirectory-encrypt-passwords.1.gz
%attr(755,root,root) /usr/share/man/man1/update-fusiondirectory.1.gz
%attr(755,root,root) /usr/share/man/man5/fusiondirectory.conf.5.gz
%attr(644,root,root) /etc/fusiondirectory/shells
%attr(644,root,root) /etc/fusiondirectory/encodings
%attr(755,root,root) /usr/sbin/fusiondirectory-encrypt-passwords
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
%attr(0700, root,root) /etc/fusiondirectory/fusiondirectory.secrets

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
/etc/fusiondirectory
/usr/bin
/usr/share/pixmaps
/usr/share/applications
%attr(755,root,root) /usr/share/man/man1/fusiondirectory.1.gz

########################

%files schema
%defattr(-,root,root)
%doc COPYING AUTHORS README contrib/openldap
/etc/openldap/schema/fusiondirectory

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
* Fri Apr 15 2011 Benoit Mortier <benoit.mortier@opensides.be> 
- First build of FusionDirectory 1.0 as an RPM, should work on SuSE and RedHat
