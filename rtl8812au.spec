# Conditional build:
%bcond_with	verbose		# verbose build (V=1)

# nothing to be placed to debuginfo package
%define		_enable_debug_packages	0

%define		rel	14
%define		snap	20140901
%define		pname	rtl8812au
Summary:	Driver for AC1200 (802.11ac) Wireless Dual-Band USB Adapter
Name:		%{pname}%{_alt_kernel}
Version:	4.3.2_11100.20140411
Release:	0.%{snap}.%{rel}%{?_pld_builder:@%{_kernel_ver_str}}
License:	GPL
Group:		Base/Kernel
Source0: https://github.com/astsam/rtl8812au/archive/master/%{name}-%{version}-%{snap}.tar.gz
#Source0:	https://github.com/abperiasamy/rtl8812AU_8821AU_linux/archive/master/%{name}-%{version}-%{snap}.tar.gz
#Source0:	https://github.com/austinmarton/rtl8812au_linux/archive/master/%{pname}-%{version}-%{snap}.tar.gz
# Source0-md5:	693825ab344b68a1217f20ab8dd98b82
# good luck finding this chip on Realtek website :/
#URL:		http://www.realtek.com.tw/
URL:		https://github.com/astsam/rtl8812au
Patch0:		linux-3.11.patch
Patch1:		disable-debug.patch
Patch2:		enable-cfg80211-support.patch
Patch3:		update-cfg80211-support.patch
Patch4:		warnings.patch
Patch5:		gcc-4.9.patch
Patch6:		linux-3.18.patch
Patch7:		linux-4.0.patch
Patch8:		linux-4.1.patch
Patch9:		linux-4.2.patch
Patch10:	linux-4.3.patch
Patch11:	linux-4.6.patch
Patch12:	linux-4.7.patch
Patch13:	linux-4.8.patch
Patch14:	linux-4.11.patch
BuildRequires:	rpmbuild(macros) >= 1.701
%{expand:%buildrequires_kernel kernel%%{_alt_kernel}-module-build >= 3:2.6.20.2}
BuildRoot:	%{tmpdir}/%{pname}-%{version}-root-%(id -u -n)

%description
Driver for AC1200 (802.11ac) Wireless Dual-Band USB Adapter.

%define	kernel_pkg()\
%package -n kernel%{_alt_kernel}-net-rtl8812au\
Summary:	Driver for AC1200 (802.11ac) Wireless Dual-Band USB Adapter\
Release:	%{rel}@%{_kernel_ver_str}\
Group:		Base/Kernel\
Requires(post,postun):	/sbin/depmod\
%requires_releq_kernel\
Requires(postun):	%releq_kernel\
\
%description -n kernel%{_alt_kernel}-net-rtl8812au\
Driver for AC1200 (802.11ac) Wireless Dual-Band USB Adapter\
\
%files -n kernel%{_alt_kernel}-net-rtl8812au\
%defattr(644,root,root,755)\
/lib/modules/%{_kernel_ver}/kernel/drivers/net/wireless/*.ko*\
\
%post	-n kernel%{_alt_kernel}-net-rtl8812au\
%depmod %{_kernel_ver}\
\
%postun	-n kernel%{_alt_kernel}-net-rtl8812au\
%depmod %{_kernel_ver}\
%{nil}

%define build_kernel_pkg()\
%{__make} clean KVER=%{_kernel_ver} KSRC=%{_kernelsrcdir}\
%{__make} modules KVER=%{_kernel_ver} KSRC=%{_kernelsrcdir}\
%install_kernel_modules -D installed -m 8812au -d kernel/drivers/net/wireless\
%{nil}

%{expand:%create_kernel_packages}

%prep
#%setup -q -n %{pname}-%{version}
%setup -q -n rtl8812au_linux-master
%patch0 -p1
%patch1 -p1
%patch2 -p1
%patch3 -p1
%patch4 -p1
%patch5 -p1
%patch6 -p1
%patch7 -p1
%patch8 -p1
%patch9 -p1
%patch10 -p1
%patch11 -p1
%patch12 -p1
%patch13 -p1
%patch14 -p1

%build
%{expand:%build_kernel_packages}

%install
rm -rf $RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT

cp -a installed/* $RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT
