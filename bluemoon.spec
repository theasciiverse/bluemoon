Name: blue
Summary: Blue Moon solitaire (text mode)
Version: 2.6
Release: 1
Source: http://www.tuxedo.org/~esr/
Copyright: GPL
Group: Games
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
%setup

%build
make

%install
make install

%files
/usr/share/man/man6/blue.6
/usr/bin/blue
