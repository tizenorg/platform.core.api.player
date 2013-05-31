Name:       capi-media-player
Summary:    A Media Player library in Tizen Native API
Version:    0.1.0
Release:    57
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-player)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(mm-ta)

%description
A Media Player library in Tizen Native API.


%package devel
Summary:  A Media Player library in Tizen Native API (Development)
Group:    Development/Multimedia
Requires: %{name} = %{version}-%{release}

%description devel
%devel_desc

%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%license LICENSE.APLv2
%manifest capi-media-player.manifest
%{_libdir}/libcapi-media-player.so.*
%{_datadir}/license/%{name}

%files devel
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-player.so


