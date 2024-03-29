Summary: Spec file for xpms2d
Name: xpms2d
Version: 3.0
Release: 2
License: GPL
Group: System Environment/Daemons
Url: http://www.eol.ucar.edu/
Packager: Chris Webster <cjw@ucar.edu>
# becomes RPM_BUILD_ROOT
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Vendor: UCAR
BuildArch: x86_64

BuildRequires: python3-scons
Requires: libpng

%if 0%{?rhel} <= 7
Requires: openmotif
%else
Requires: motif
%endif

Source: %{name}.tar.gz

%description
Configuration for NCAR-EOL xpms2d display for OAP probes.

%prep
%setup -n %{name}

%build
scons

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_bindir}
cp xpms2d/src/%{name} %{buildroot}%{_bindir}

%post


%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/%{name}

%changelog

* Sat Mar 13 2021 Chris Webster <cjw@ucar.edu> - 3.0-2
- Try and get rpmbuild to work.  Help Menu refactor.

* Mon Jul 27 2015 Chris Webster <cjw@ucar.edu> - 2.7-1
- Merge in 2DS branch.

* Fri Dec 05 2014 Chris Webster <cjw@ucar.edu> - 2.7-0
- Add support for CIP & PIP probes.  Files still need to meet OAP format.  I would call this beta support.

* Thu Dec 12 2013 Chris Webster <cjw@ucar.edu> - 2.6-2
- bug fixes.
- Magnify box issue - XButtonEvent needed update, wasn't detecting if NumLock was off (or on).
- Merge in branch file positioning.
- Replace fopen64() freado64(), fseeko64(), etc with fopen() fread(), fseeko()
- Clean up Mac build for Mountian Lion
* Wed Jul 13 2011 Chris Webster <cjw@ucar.edu> - 2.6-1
- initial version
