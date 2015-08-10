%bcond_with wayland
%bcond_with x

Name:       legacy-capi-media-player
Summary:    A Media Player library in Tizen Native API
Version:    0.2.5
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: 	legacy-capi-media-player.manifest
BuildRequires:  cmake
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-player)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(evas)
%if %{with x}
BuildRequires:  pkgconfig(ecore-x)
%endif
%if %{with wayland}
BuildRequires:  pkgconfig(ecore-wayland)
%endif
BuildRequires:  pkgconfig(capi-media-tool)
BuildRequires:  pkgconfig(libtbm)
#BuildRequires:  pkgconfig(ttrace)
BuildRequires:  pkgconfig(capi-system-info)

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

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
cp %{SOURCE1001} .


%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
#export CFLAGS+=" -D_USE_X_DIRECT_"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER} \
%if "%{?profile}" == "wearable"
	-DTIZEN_WEARABLE=YES \
    %else
    -DTIZEN_MOBILE=YES \
    %endif
%if %{with wayland}
    -DWAYLAND_SUPPORT=On \
%else
    -DWAYLAND_SUPPORT=Off \
%endif
%if %{with x}
    -DX11_SUPPORT=On \
%else
    -DX11_SUPPORT=Off \
%endif
    -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER} \
    -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/usr/bin
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
cp test/legacy_player_test %{buildroot}/usr/bin
cp test/legacy_player_media_packet_test %{buildroot}/usr/bin
cp test/legacy_player_es_push_test %{buildroot}/usr/bin

%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%license LICENSE.APLv2
%manifest legacy-capi-media-player.manifest
%{_libdir}/liblegacy-capi-media-player.so.*
%{_datadir}/license/%{name}
%{_bindir}/*

%files devel
%manifest %{name}.manifest
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/liblegacy-capi-media-player.so


