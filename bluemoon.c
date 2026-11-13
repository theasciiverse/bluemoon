/*****************************************************************************
 *                                                                           *
 *                         B l u e   M o o n                                 *
 *                         =================                                 *
 *                   A patience game by T.A.Lister                           *
 *            Integral screen support by Eric S. Raymond                     *
 *                                                                           *
 *****************************************************************************/

/*
 * Compile this with the command `cc -O bluemoon.c -lcurses -o bluemoon'.
 * For best results, use the open-source ncurses(3) library.  On non-Intel
 * machines, SVr4 curses is just as good.
 *
 * Upon reading this code, Tim Lister wrote:

> I have come to the conclusion that it looks like a C port of a game
> I wrote many, many moons ago in GFA BASIC (one of the best versions of
> BASIC written) for the Atari ST. I am not sure whether I gave a
> copy of it to anyone and I am at a loss to explain how it
> managed to find its way onto the Internet. Ah well, may you live
> in interesting times as the Chinese curse goes...

 *
 * Debugging note:  The command `x' causes the game to terminate and write
 * a line consisting of the token `blue' followed by the random-number
 * seed used for the game, followed by a list of the moves.
 *
 * SPDX-FileCopyrightText: (C) Eric S. Raymond <esr@thyrsus.com>
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <curses.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#ifndef PDCURSES /* PDCurses has no terminfo */
#include <term.h>
#endif
#include <time.h>
#include <unistd.h>
#ifdef AV_UTF8_SUITS
#include <langinfo.h>
#endif
#ifdef AV_CENTRE
#include "centre.h"
#endif

#define NOCARD (-1)

#define ACE 0
#define KING 12
#define SUIT_LENGTH 13

#define HEARTS 0
#define SPADES 1
#define DIAMONDS 2
#define CLUBS 3
#define NSUITS 4

#define GRID_WIDTH (SUIT_LENGTH + 1)
#define GRID_LENGTH (NSUITS * GRID_WIDTH)
#define PACK_SIZE 52
#define MAX_DEALS 13
#define MAX_MOVES (PACK_SIZE * MAX_DEALS) /* assume <= 52 moves per deal */

#define BASEROW 1
#define PROMPTROW 11

static int seed, nmove;
static char journal[MAX_MOVES + 1];
static volatile sig_atomic_t got_sigint;

static int deck_size = PACK_SIZE; /* initial deck */
static int deck[PACK_SIZE];

static int grid[GRID_LENGTH]; /* card layout grid */
static int freeptr[4];        /* free card space pointers */

static int deal_number = 0;

static char *ranks[SUIT_LENGTH] = {" A", " 2", " 3", " 4", " 5", " 6", " 7",
                                   " 8", " 9", "10", " J", " Q", " K"};

static chtype letters[] = {
    'h' | COLOR_PAIR(COLOR_RED),   /* hearts */
    's' | COLOR_PAIR(COLOR_GREEN), /* spades */
    'd' | COLOR_PAIR(COLOR_RED),   /* diamonds */
    'c' | COLOR_PAIR(COLOR_GREEN), /* clubs */
};

#if defined(__i386__) && defined(A_PCCHARSET)
static chtype glyphs[] = {
    '\003' | A_PCCHARSET | COLOR_PAIR(COLOR_RED),   /* hearts */
    '\006' | A_PCCHARSET | COLOR_PAIR(COLOR_GREEN), /* spades */
    '\004' | A_PCCHARSET | COLOR_PAIR(COLOR_RED),   /* diamonds */
    '\005' | A_PCCHARSET | COLOR_PAIR(COLOR_GREEN), /* clubs */
};
#endif /* __i386__ && A_PCCHARSET */

static chtype *suits = letters; /* this may change to glyphs below */

#ifdef AV_UTF8_SUITS
/* The suit symbols, drawn when the terminal speaks UTF-8 (needs a wide
 * curses, so only in builds that ask for it). */
static const char *utf8_suits[] = {"\u2665", "\u2660", "\u2666", "\u2663"};
static int use_utf8_suits;
#endif

static void die(int onsig) {
	if (onsig) {
		signal(onsig, SIG_IGN);
	}
	endwin();
	if (seed) {
		printf("bluemoon %d %s\n", seed, journal);
	}
	exit(0);
}

static void on_sigint(int onsig) {
	(void)onsig;
	got_sigint = 1;
}

static void init_vars(void) {
	int i;

	deck_size = PACK_SIZE;
	for (i = 0; i < PACK_SIZE; i++) {
		deck[i] = i;
	}
	for (i = 0; i < 4; i++) {
		freeptr[i] = i * GRID_WIDTH;
	}
}

static void shuffle(int size) {
	int i, j, numswaps, swapnum, temp;

	numswaps = size * 10; /* an arbitrary figure */

	for (swapnum = 0; swapnum < numswaps; swapnum++) {
		i = rand() % size;
		j = rand() % size;
		temp = deck[i];
		deck[i] = deck[j];
		deck[j] = temp;
	}
}

static void deal_cards(void) {
	int ptr, card = 0, value, csuit, crank, suit, aces[4];

	for (suit = HEARTS; suit <= CLUBS; suit++) {
		ptr = freeptr[suit];
		grid[ptr++] = NOCARD; /* 1st card space is blank */
		while ((ptr % GRID_WIDTH) != 0) {
			value = deck[card++];
			crank = value % SUIT_LENGTH;
			csuit = value / SUIT_LENGTH;
			if (crank == ACE) {
				aces[csuit] = ptr;
			}
			grid[ptr++] = value;
		}
	}

	if (deal_number == 1) { /* shift the aces down to the 1st column */
		for (suit = HEARTS; suit <= CLUBS; suit++) {
			grid[suit * GRID_WIDTH] = suit * SUIT_LENGTH;
			grid[aces[suit]] = NOCARD;
			freeptr[suit] = aces[suit];
		}
	}
}

static void printcard(int value) {
	(void)addch(' ');
	if (value == NOCARD) {
		(void)addstr("   ");
	} else {
		addch(ranks[value % SUIT_LENGTH][0] | COLOR_PAIR(COLOR_BLUE));
		addch(ranks[value % SUIT_LENGTH][1] | COLOR_PAIR(COLOR_BLUE));
#ifdef AV_UTF8_SUITS
		if (use_utf8_suits) {
			attr_t pair = suits[value / SUIT_LENGTH] & A_COLOR;
			attron(pair);
			addstr(utf8_suits[value / SUIT_LENGTH]);
			attroff(pair);
		} else
#endif
			addch(suits[value / SUIT_LENGTH]);
	}
	(void)addch(' ');
}

static void display_cards(int deal) {
	int row, card;

	clear();
	(void)printw("Blue Moon %s - by Tim Lister & Eric Raymond - Deal %d.\n",
	             RELEASE, deal);
	for (row = HEARTS; row <= CLUBS; row++) {
		move(BASEROW + row + row + 2, 1);
		for (card = 0; card < GRID_WIDTH; card++) {
			printcard(grid[row * GRID_WIDTH + card]);
		}
	}

	move(PROMPTROW + 2, 0);
	refresh();
	// clang-format off
#define P(x)	(void)printw("%s\n", x)
P("   This 52-card solitaire starts with  the entire deck shuffled and dealt");
P("out in four rows.  The aces are then moved to the left end of the layout,");
P("making 4 initial free spaces.  You may move to a space only the card that");
P("matches the left neighbor in suit, and is one greater in rank.  Kings are");
P("high, so no cards may be placed to their right (they create dead spaces).");
P("  When no moves can be made,  cards still out of sequence are  reshuffled");
P("and dealt face up after the ends of the partial sequences, leaving a card");
P("space after each sequence, so that each row looks like a partial sequence");
P("followed by a space, followed by enough cards to make a row of 14.       ");
P("  A moment's reflection will show that this game cannot take more than 13");
P("deals. A good score is 1-3 deals, 4-7 is average, 8 or more is poor.     ");
#undef P
	// clang-format on
	refresh();
}

static int find(int card) {
	int i;

	if ((card < 0) || (card >= PACK_SIZE)) {
		return (NOCARD);
	}
	for (i = 0; i < GRID_LENGTH; i++) {
		if (grid[i] == card) {
			return i;
		}
	}
	return (NOCARD);
}

static void movecard(int src, int dst) {
	grid[dst] = grid[src];
	grid[src] = NOCARD;

	move(BASEROW + (dst / GRID_WIDTH) * 2 + 2, (dst % GRID_WIDTH) * 5 + 1);
	printcard(grid[dst]);

	move(BASEROW + (src / GRID_WIDTH) * 2 + 2, (src % GRID_WIDTH) * 5 + 1);
	printcard(grid[src]);

	refresh();
}

static void play_game(void) {
	int dead = 0, i, j;
	char c;
	int select[NSUITS], card;

	nmove = 0;
	memset(journal, '\0', sizeof(journal));
	while (dead < 4) {
		if (got_sigint) {
			seed = 0;
			die(0);
		}
		dead = 0;
		for (i = 0; i < 4; i++) {
			card = grid[freeptr[i] - 1];

			if (((card % SUIT_LENGTH) == KING) ||
			    (card == NOCARD)) {
				select[i] = NOCARD;
			} else {
				select[i] = find(card + 1);
			}
			if (select[i] == NOCARD) {
				dead++;
			}
		};

		if (dead < 4) {
			char live[NSUITS + 1], *lp = live;

			for (i = 0; i < 4; i++) {
				if (select[i] != NOCARD) {
					move(BASEROW +
					         (select[i] / GRID_WIDTH) * 2 +
					         3,
					     (select[i] % GRID_WIDTH) * 5);
					(void)printw("   %c ", *lp++ = 'a' + i);
				}
			};
			*lp = '\0';

			if (strlen(live) == 1) {
				move(PROMPTROW, 0);
				(void)printw("Making forced moves...           "
				             "                      ");
				refresh();
				(void)sleep(1);
				c = live[0];
			} else {
				char buf[BUFSIZ];
				int ch;

				(void)snprintf(buf, sizeof(buf),
				               "Type [%s] to move, r to redraw, "
				               "q or INTR to quit: ",
				               live);

				do {
					move(PROMPTROW, 0);
					(void)addstr(buf);
					move(PROMPTROW, strlen(buf));
					clrtoeol();
					(void)addch(' ');
					ch = getch();
					if (got_sigint) {
						seed = 0;
						die(0);
					}
				} while (ch == ERR || ch < 0 || ch > 255 ||
				         !strchr("abcdqrx", ch));
				c = (char)ch;
			}

			for (j = 0; j < 4; j++) {
				if (select[j] != NOCARD) {
					move(BASEROW +
					         (select[j] / GRID_WIDTH) * 2 +
					         3,
					     (select[j] % GRID_WIDTH) * 5);
					(void)printw("     ");
				}
			}

			if (c == 'r') {
				clearok(stdscr, TRUE);
				display_cards(deal_number);
			} else if (c == 'x') {
				die(0);
			} else if (c == 'q') {
				seed = 0;
				die(0);
			} else {
				i = c - 'a';
				if (select[i] == NOCARD) {
					beep();
				} else {
					movecard(select[i], freeptr[i]);
					freeptr[i] = select[i];
				}
			}
			if (nmove < MAX_MOVES) {
				journal[nmove++] = c;
				journal[nmove] = '\0';
			}
		}
	}

	move(PROMPTROW, 0);
	standout();
	(void)printw("Finished deal %d - type any character to continue...",
	             deal_number);
	standend();
	(void)getch();
}

static int collect_discards(void) {
	int row, col, cardno = 0, finish, gridno;

	for (row = HEARTS; row <= CLUBS; row++) {
		finish = 0;
		for (col = 1; col < GRID_WIDTH; col++) {
			gridno = row * GRID_WIDTH + col;

			if ((grid[gridno] != (grid[gridno - 1] + 1)) &&
			    (finish == 0)) {
				finish = 1;
				freeptr[row] = gridno;
			};

			if ((finish != 0) && (grid[gridno] != NOCARD)) {
				deck[cardno++] = grid[gridno];
			}
		}
	}
	return cardno;
}

static void game_finished(int deal) {
	clear();
	(void)printw("You finished the game in %d deals. This is ", deal);
	standout();
	if (deal < 2) {
		(void)addstr("excellent");
	} else if (deal < 4) {
		(void)addstr("good");
	} else if (deal < 8) {
		(void)addstr("average");
	} else {
		(void)addstr("poor");
	}
	standend();
	(void)addstr(".         ");
	move(2, 0);
	(void)addstr("Type n for a new game, any other key to quit.");
	refresh();
}

/* Wait on the result: true for another game. */
static int another_game(void) {
	int ch;

	do {
		ch = getch();
		if (got_sigint) {
			return 0;
		}
	} while (ch == ERR
#ifdef KEY_RESIZE
	         || ch == KEY_RESIZE
#endif
	);
	return ch == 'n' || ch == 'N';
}

int main(int argc, char *argv[]) {
	(void)signal(SIGINT, on_sigint);
	(void)setlocale(LC_ALL, "");
	initscr();
#ifdef AV_CENTRE
	av_centre_start();
#endif

	/*
	 * We use COLOR_GREEN because COLOR_BLACK is wired to the wrong thing.
	 */
	start_color();
	init_pair(COLOR_RED, COLOR_RED, COLOR_WHITE);
	init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_WHITE);
	init_pair(COLOR_GREEN, COLOR_BLACK, COLOR_WHITE);

#if defined(__i386__) && defined(A_PCCHARSET)
	{
		char *cap = tigetstr("smpch");
		if (cap && cap != (char *)-1) {
			suits = glyphs;
		}
	}
#endif /* __i386__ && A_PCCHARSET */

#ifdef AV_UTF8_SUITS
	{
		const char *cs = nl_langinfo(CODESET);
		use_utf8_suits = cs && strcmp(cs, "UTF-8") == 0;
	}
#endif

	cbreak();
	noecho();

	if (argc == 2) {
		srand(seed = atoi(argv[1]));
	} else {
		srand(seed = (int)time(NULL));
	}

	for (;;) {
		init_vars();
		deal_number = 0;

		do {
			deal_number++;
			shuffle(deck_size);
			deal_cards();
			display_cards(deal_number);
			play_game();
		} while ((deck_size = collect_discards()) != 0);

		game_finished(deal_number);
		if (!another_game()) {
			break;
		}
		srand(seed = rand());
	}

	die(0);
}

/* blue.c ends here */
