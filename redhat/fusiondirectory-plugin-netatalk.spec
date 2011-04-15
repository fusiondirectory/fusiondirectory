
# Some sort of "detection" of suse
%{?suse_version:%define suse 1}
%{!?suse_version:%define suse 0}

# Define Packagename, e.g.:
# rpmbuild --rebuild --define 'sourcename gosa' gosa.srpm
%{!?sourcename:%define sourcename %{name}-%{version}}

#
# Distribution
#
Summary:                Apple account management
Name:                   gosa-plugin-netatalk
Version:                2.6.11
Release:                0
License:                GPL
Source:                 ftp://oss.GONICUS.de/pub/gosa/%{sourcename}.tar.bz2
URL:                    https://oss.gonicus.de/labs/gosa/
Group:                  System/Administration
Vendor:                 GONICUS GmbH
Packager:               Cajus Pollmeier <pollmeier@gonicus.de>
Buildarch:              noarch
%if %{suse}
Requires:               gosa >= 2.6.0, gosa-plugin-systems
%else
Requires:               gosa >= 2.6.0, gosa-plugin-systems
%endif
BuildRoot:              %{_tmppath}/%{name}-%{version}-root
BuildArch:              noarch

%if %{suse}
        %{echo:Building SuSE rpm}
        %define docdir /usr/share/doc/packages/gosa
%else
        %{echo:Building other rpm}
        %define webconf /etc/httpd/conf.d/
        %define docdir /usr/share/doc/gosa-%{version}
%endif

%description
Apple account management

%prep
%setup -q -n %{sourcename}

%build

%install

# Move extra HTML and images
mkdir -p %{buildroot}/usr/share/gosa/html/plugins/netatalk

echo "%{buildroot} <<---------" 

if ls ./html/* &> /dev/null ;then
	mv ./html/* %{buildroot}/usr/share/gosa/html/plugins/netatalk
fi

# Move contrib if available
mkdir -p %{buildroot}/usr/share/doc/%{name}
if ls ./contrib/* &> /dev/null ;then
	mv ./contrib %{buildroot}/usr/share/doc/%{name}
fi

# Move help
mkdir -p %{buildroot}/usr/share/gosa/doc/plugins/netatalk
if ls ./help/* &> /dev/null ;then
	mv ./help/* %{buildroot}/usr/share/gosa/doc/plugins/netatalk
fi

# Copy etc
mkdir -p %{buildroot}/etc/gosa
if ls ./etc/* &> /dev/null ;then
	mv ./etc/* %{buildroot}/etc/gosa
fi

# Move localization
mkdir -p %{buildroot}/usr/share/gosa/locale/plugins/netatalk
if ls ./locale/* &> /dev/null ;then
	mv ./locale/* %{buildroot}/usr/share/gosa/locale/plugins/netatalk
fi

# Move plugin itself
mkdir -p %{buildroot}/usr/share/gosa/plugins/
mv ./* %{buildroot}/usr/share/gosa/plugins/

%post
/usr/sbin/update-gosa

%postun
/usr/sbin/update-gosa

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%attr (-,root,root) /usr/share/gosa/plugins/
%attr (-,root,root) /usr/share/gosa/doc/plugins/netatalk
%attr (-,root,root) /usr/share/gosa/locale/plugins/netatalk
%attr (-,root,root) /usr/share/gosa/html/plugins/netatalk
%attr (-,root,root) /usr/share/doc/%{name}
%attr (-,root,root) /etc/gosa/
