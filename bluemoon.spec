Description: Blue Moon solitaire (text mode)
Name: blue
Version: 2.2
Release: 1
Source: locke.ccil.org:/pub/esr/blue-2.2.tar.gz
Copyright: BSD-like
Group: Games

%prep
%setup

%build
make

%install
rm -f /usr/bin/blue
cp blue /usr/bin
cp blue.6 /usr/man/man1/blue.6

%files
%doc /usr/man/man1/blue.6
/usr/bin/blue
