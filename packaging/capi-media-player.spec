Name:       capi-media-player
Summary:    A Media Player library in Tizen Native API
Version:    0.1.0
Release:    12
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: packaging/capi-media-player.manifest 
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-player)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-sound-manager)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description


%package devel
Summary:  A Media Player library in Tizen Native API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel

%prep
%setup -q


%build
cp %{SOURCE1001} .
FULLVER=%{version}
MAJORVER=`echo ${FULLVER} | cut -d '.' -f 1`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=${FULLVER} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest capi-media-player.manifest
%{_libdir}/libcapi-media-player.so*

%files devel
%manifest capi-media-player.manifest
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/*.pc


