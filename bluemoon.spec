Name: bluemoon
Summary: Blue Moon solitaire (text mode)
Version: 2.8
Release: 1
URL: http://www.catb.org/~esr/bluemoon/
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Games
BuildRoot: %{_tmppath}/%{name}-root

%description
   This 52-card solitaire starts with  the entire deck shuffled and dealt
out in four rows.  The aces are then moved to the left end of the layout,
making 4 initial free spaces.  You may move to a space only the card that
matches the left neighbor in suit, and is one greater in rank.  Kings are
high, so no cards may be placed to their right (they create dead spaces).
  When no moves can be made,  cards still out of sequence are  reshuffled
and dealt face up after the ends of the partial sequences, leaving a card
space after each sequence, so that each row looks like a partial sequence
followed by a space, followed by enough cards to make a row of 14.
  A moment's reflection will show that this game cannot take more than 13
deals. A good score is 1-3 deals, 4-7 is average, 8 or more is poor.

%prep
%setup -q

%build
make
make bluemoon.6

%install
[ ! -z "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"
mkdir -p "$RPM_BUILD_ROOT"/usr/bin
mkdir -p "$RPM_BUILD_ROOT"/usr/share/man/man6/
cp bluemoon "$RPM_BUILD_ROOT"/usr/bin
cp bluemoon.6 "$RPM_BUILD_ROOT"/usr/share/man/man6/

%clean
[ ! -z "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"
%files
%defattr(-,root,root,-)
%{_mandir}/man6/bluemoon.6*
%{_bindir}/bluemoon
%doc README COPYING

%changelog
* Mon Dec 29 2003 Eric S. Raymond <esr@snark.thyrsus.com> 2.8-1
- Source RPMs no longer require --define myversion.

* Wed Dec 24 2003 Eric S. Raymond <esr@snark.thyrsus.com> 2.7-1
- Fixed build machinery to use rpmbuild.

