Name: zokete
Version: 1.0.0
Release: 1%{?dist}
Summary: Simple SOCKS5 server
Group: System Environment/Daemons
License: GPLv3+
URL: https://github.com/matricali/zokete
Source0: https://github.com/matricali/zokete/archive/%{version}/%{name}-%{version}.tar.gz
BuildRequires: gcc
BuildRequires: make

%description
Simple SOCKS5 server.

%prep
%setup -q -n zokete-%{version}

%build
make %{?_smp_mflags} clean all

%install
make install DESTDIR="%{buildroot}" PREFIX="" BINDIR="%{_bindir}" MANDIR="%{_mandir}/man8" DATADIR="%{_datadir}/%{name}"
mkdir -p %{buildroot}%{_mandir}/man8
cp docs/man/zoketed.8 %{buildroot}%{_mandir}/man8/zoketed.8

%post
# Register the zokete service
/sbin/chkconfig --add zokete

%files
%defattr(-,root,root)

%{_bindir}/zoketed
%{_mandir}/man8/zoketed.8*

%doc README.md LICENSE.txt
%{!?_licensedir:%global license %doc}
%license LICENSE.txt

%changelog
* Wed Dec 25 2019 Jorge Matricali <jorgematricali@gmail.com> 0:1.0-0
- Initial release
