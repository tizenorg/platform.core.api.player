Name:       capi-media-player
Summary:    A Media Player library in Tizen Native API
Version:    0.1.58
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-player)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(capi-media-tool)
BuildRequires:  pkgconfig(libtbm)

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
A Media Player library in Tizen Native API.

%package devel
Summary:  A Media Player library in Tizen Native API (Development)
Group:    Multimedia/API
Requires: %{name} = %{version}-%{release}

%description devel
%devel_desc
%prep
%setup -q


%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
#export CFLAGS+=" -D_USE_X_DIRECT_"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/opt/usr/devel
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
cp test/player_test %{buildroot}/opt/usr/devel

%make_install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest capi-media-player.manifest
%{_libdir}/libcapi-media-player.so.*
%{_datadir}/license/%{name}
/opt/usr/devel/*

%files devel
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-player.so


