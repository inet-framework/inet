
%{?ksrc: 	%{!?kernel:    %{expand: %%define kernel %(cd "%{ksrc}" &> /dev/null && echo "$(cat Makefile 2>/dev/null && echo $'\n'kernelhelper-rel:$'\n'$'\t'@echo \$\(KERNELRELEASE\)$'\n')" 2>/dev/null | make -f - kernelhelper-rel 2>/dev/null || echo "custom" ) }}}
%{!?kernel:             %{expand: %%define kernel %(uname -r)}}


%if %(echo %{kernel} | grep -c smp)
        %{expand:%%define ksmp -smp}
%endif

Name:           dsr-uu
Version:        0.1
Release:        1
Summary:        An source routed routing protocol for ad hoc networks compiled for kernel %{kernel}.

Group:          System Environment/Kernel
URL:            http://core.it.uu.se/adhoc
License:        GPL
Vendor:	        Erik Nordström, erikn[AT]it[DOT]uu[DOT]se, Uppsala University.
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description 
DSR (Dynamic Source Routing) is a routing protocol for ad hoc
networks. It uses source routing and is being developed within the
MANET Working Group of the IETF.

%package -n %{name}-%{kernel}
Summary:        DSR-UU Kernel Module
Group:          System Environment/Kernel
Provides:	%{name}
Requires(post):   /sbin/depmod
Requires(postun): /sbin/depmod
%if 0%{!?ksrc:1}
Requires:	 /boot/vmlinuz-%{kernel}
BuildRequires:  kernel-devel = %{kernel}
%endif

%description -n %{name}-%{kernel}
DSR (Dynamic Source Routing) is a routing protocol for ad hoc
networks. It uses source routing and is being developed within the
MANET Working Group of the IETF.

%prep
%setup -q

%build
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/sbin
mkdir -p $RPM_BUILD_ROOT/lib/modules/%{kernel}/%{name}

install -m 755 dsr-uu.sh $RPM_BUILD_ROOT/usr/sbin/dsr-uu.sh        
install -m 644 dsr.ko $RPM_BUILD_ROOT/lib/modules/%{kernel}/dsr-uu/dsr.ko
install -m 644 linkcache.ko $RPM_BUILD_ROOT/lib/modules/%{kernel}/dsr-uu/linkcache.ko

%clean
rm -rf $RPM_BUILD_ROOT

%post -n %{name}-%{kernel}
%if 0%{!?ksrc:1}
if [ -r /boot/System.map-%{kernel} ] ; then
  /sbin/depmod -e -F /boot/System.map-%{kernel} %{kernel} > /dev/null || :
fi
%else
if [ "$(uname -r)" = "%{kernel}" ] ; then
  /sbin/depmod -a >/dev/null || :
fi
%endif

%postun -n %{name}-%{kernel}
%if 0%{!?ksrc:1}
if [ -r /boot/System.map-%{kernel} ] ; then
  /sbin/depmod -e -F /boot/System.map-%{kernel} %{kernel} > /dev/null || :
fi
%else
if [ "$(uname -r)" = "%{kernel}" ] ; then
  /sbin/depmod -a >/dev/null || :
fi
%endif

%files -n %{name}-%{kernel}
%defattr(-,root,root)
%doc README ChangeLog
%dir /lib/modules/%{kernel}/%{name}
/usr/sbin/dsr-uu.sh
/lib/modules/%{kernel}/%{name}/dsr.ko
/lib/modules/%{kernel}/%{name}/linkcache.ko

%changelog
* Wed Aug 10 2005 Erik Nordstrom <erikn@wormhole.it.uu.se> - 0.1-1
- Created spec file

