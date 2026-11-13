# Changes from upstream

This is the Asciiverse edition of Blue Moon (BSD-2-Clause), Tim Lister's
patience from the Atari ST given its curses screen by Eric S. Raymond in
1994, from ESR's own repository, gitlab.com/esr/bluemoon (the `upstream`
remote), at release 2.15 (`91ff5db`, 2026-03-29), with its full history.
The `asciiverse` branch carries the changes below.

## Code

- **The result stays on the screen, and you can play again.** At the end
  of a game, "You finished the game in N deals. This is good." was drawn
  and the program ended at once, so in a terminal with a separate full
  screen (most of them, and under av) the result vanished before it could
  be read. It now waits: n deals a new game, any other key quits. A new
  game takes a fresh seed from the old one.
- **Keys are not echoed** (noecho()). A key typed while the forced moves
  played out could land on the board.
- **Suit symbols in UTF-8 terminals** (under `AV_UTF8_SUITS`, for builds
  linked with a wide curses): the cards show ♥ ♠ ♦ ♣ in their colours
  instead of the letters h s d c, when the locale is UTF-8. The game
  already used the PC character set's suit glyphs where an i386 console
  offered them; this does the same for today's terminals. Other locales,
  and builds without the option, keep the letters.
- **Centred, and following a resized terminal** (under `AV_CENTRE`,
  `asciiverse/centre.h`, Galaxis' header without the mouse): bluemoon.c
  draws on an 80x24 pad that the name stdscr and printw point at,
  refresh() shows it in the middle of the terminal, keys are read on the
  terminal's own screen, and a resize centres it again.
- **Windows** (MinGW and PDCurses): `<term.h>`, which PDCurses does not
  have, is only included for other curses; it served the i386 glyph
  check, which does not apply there.
- setlocale() is called at start, for the suit symbols and wide curses.

Blue Moon keeps no files, so nothing moves to a data folder.

## Packaging

- `asciiverse/package.sh` builds with the changes above and packs
  `bluemoon-<target>` with the program, COPYING, README.adoc, the manual
  (bluemoon.adoc) and this file. Upstream's Makefile, which builds the
  man page with asciidoctor, is left as it is; the version comes from
  NEWS.adoc the way it reads it.
- `.github/workflows/release.yml`: static musl Linux (Alpine, x86-64 and
  ARM64, ncursesw, suit symbols), macOS (Intel and Apple silicon, system
  ncurses, letters) and Windows x86-64 (MinGW, PDCurses 3.9 wincon), on
  our tags (`*-av.*`), plus .deb, .rpm, .apk and Arch packages of the
  static Linux binary by nfpm (`asciiverse/nfpm.yaml`).
- `asciiverse/record.sh` records the catalog preview,
  `asciiverse/bluemoon.cast`, dealing game 2 while `asciiverse/drive.py`
  plays the first move offered at a person's pace: three deals, "good".

## Checked

Linux, native and static (Alpine): whole games played through at 100x30
by a driver (four seeds, 3 to 7 deals), the result screen, a new game
with n, quitting, letters in the C locale, symbols in C.UTF-8, 80x24 and
a resize to 120x40. Windows: cross-built in Docker, started under Wine
(deals, takes a move, quits); its drawing under Wine inside tmux not
judged. macOS: not built here.
