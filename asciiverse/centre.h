/*
 * Asciiverse: Blue Moon centred in the terminal (compiled in with
 * -DAV_CENTRE; see PATCHES.md). The same header as Galaxis', without
 * the mouse.
 *
 * Blue Moon draws an 80x24 screen at fixed positions on stdscr. Here it
 * draws on an 80x24 pad instead: after this header, the name stdscr in
 * bluemoon.c means the pad (curses' drawing macros use that name), and
 * printw() and mvprintw(), which are functions, draw on the pad too.
 * refresh() copies the pad to the middle of the terminal, getch() does
 * the same and waits for a key on the terminal's own screen (which
 * nothing draws on), and a resized terminal centres the screen again
 * (the game never sees KEY_RESIZE). (Assigning the pad to stdscr itself
 * does not hold: curses puts its own value back.)
 *
 * SPDX-FileCopyrightText: (C) 2026 Asciiverse
 * SPDX-License-Identifier: BSD-2-Clause
 */
#ifndef AV_CENTRE_H
#define AV_CENTRE_H

#define AV_LINES 24
#define AV_COLS 80

static WINDOW *av_pad;   /* where the game draws */
static int av_oy, av_ox; /* where the pad's corner is shown */

/* Make the pad; call right after initscr(). */
static void av_centre_start(void) {
	av_pad = newpad(AV_LINES, AV_COLS);
	if (av_pad) {
		keypad(av_pad, TRUE);
		keypad(stdscr, TRUE); /* keys are read on the screen */
		/* nothing to show there: else the first getch() would
		 * refresh it, blank, over the pad */
		untouchwin(stdscr);
	}
}

/* The window the game draws on: the pad, or the screen without one. */
static WINDOW *av_win(void) { return av_pad ? av_pad : stdscr; }

/* Show the pad centred (its top left part on a smaller terminal). */
static int av_refresh(void) {
	int h, w;

	if (!av_pad) {
		return wrefresh(stdscr);
	}
	av_oy = LINES > AV_LINES ? (LINES - AV_LINES) / 2 : 0;
	av_ox = COLS > AV_COLS ? (COLS - AV_COLS) / 2 : 0;
	h = LINES < AV_LINES ? LINES : AV_LINES;
	w = COLS < AV_COLS ? COLS : AV_COLS;
	return prefresh(av_pad, 0, 0, av_oy, av_ox, av_oy + h - 1,
	                av_ox + w - 1);
}

/* Wait for a key, showing the screen first; a resized terminal is
 * cleared and the screen centred again. */
static int av_getch(void) {
	int c;

	for (;;) {
		av_refresh();
		c = wgetch(stdscr);
#ifdef KEY_RESIZE
		if (c == KEY_RESIZE && av_pad) {
#ifdef PDCURSES
			resize_term(0, 0);
#endif
			werase(stdscr);
			wnoutrefresh(stdscr);
			clearok(curscr, TRUE);
			continue;
		}
#endif
		return c;
	}
}

/* From here on, in bluemoon.c */
#undef stdscr
#define stdscr av_win()
#undef printw
#define printw(...) wprintw(av_win(), __VA_ARGS__)
#undef mvprintw
#define mvprintw(y, x, ...) mvwprintw(av_win(), y, x, __VA_ARGS__)
#undef refresh
#define refresh() av_refresh()
#undef getch
#define getch() av_getch()

#endif /* AV_CENTRE_H */
