Name:		qosmic
Version:	1.6.0
Release:	1%{?dist}
Summary:	A flam3 fractal editor

Group:		Applications/Multimedia
License:	GPLv3
URL:		http://github.com/bitsed/qosmic
Source0:	qosmic-%{version}.tar.bz2
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	flam3-devel >= 3.1.1
BuildRequires:	qt-devel >= 5.5
BuildRequires:	lua-devel >= 5.2
Requires:	flam3 >= 3.1.1
Requires:	qt >= 5.5
Requires:	lua >= 5.2

%description
A graphical interface to create, edit, and render flam3 fractal images.

%prep
%setup -q


%build
%{_qt4_qmake}
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install INSTALL_ROOT=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc README README-LUA changes.txt COPYING scripts/
%{_bindir}/%{name}
%{_pixmapsdir}/qosmicicon.xpm
%{_desktopdir}/%{name}.desktop


%changelog
* Thu Mar 10 2011 Nathaniel Clark <utopiabound@sourceforge.net> - 1.4.8-1
- Initial Spec file
* Thu Jul 21 2011 David Bitseff <bitsed@gmail.com> - 1.5.0-1
- Version 1.5.0 Spec file
* Thu Jun 23 2016 David Bitseff <bitsed@gmail.com> - 1.6.0-1
- Version 1.6.0 Spec file
