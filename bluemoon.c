/*****************************************************************************
 *                                                                           *
 *                         B l u e   M o o n                                 *
 *                         =================                                 *
 *                               V2.1                                        *
 *                   A patience game by T.A.Lister                           *
 *            Integral screen support by Eric S. Raymond                     *
 *                                                                           *
 *****************************************************************************/

/*
 * Compile this with the command `cc -O blue.c -ltermcap -o blue`. You might
 * have to plug in a different library name for -ltermcap; -lcurses and
 * -ltermlib are common alternatives.
 *
 * If your terminal type has IBMPREFIX as a prefix, this program will assume
 * your terminal can display the IBM card-suit graphics using the strings given
 * here. These strings work with the generic AT&T System V console(7) driver
 * on AT-compatible boxes; if you have something else (like a pre-merge XENIX)
 * they might have to change. This ugly kluge is needed because as of SVr3.2
 * termcap/terminfo doesn't yet have a capability for displaying the `literal'
 * values of control characters.
 *
 * Warning: due to a bug in tgetent() on the 6386WGS (and possibly elsewhere)
 * under SVr3.2, running this program may trash eof, tab3 and possibly other
 * pieces of your tty control info. If this happens, recompile with -DBROKEN.
 */
#define IBMPREFIX	"AT"
#define HEART		"\033[11m\003\033[10m"
#define SPADE		"\033[11m\006\033[10m"
#define DIAMOND		"\033[11m\004\033[10m"
#define CLUB		"\033[11m\005\033[10m"

#include <signal.h>
#include <stdio.h>
#include <ctype.h>	/* for isdigit() */
#ifdef BROKEN
#include <termio.h>

static struct termio savetty;
#endif /* BROKEN */

extern long time();
extern char *malloc();

#define NOCARD		(-1)

#define ACE		0
#define KING		12
#define SUIT_LENGTH	13

#define HEARTS		0
#define SPADES		1
#define DIAMONDS	2
#define CLUBS		3
#define NSUITS		4

#define GRID_WIDTH	14	/*    13+1  */
#define GRID_LENGTH	56	/* 4*(13+1) */
#define PACK_SIZE	52

#define BASEROW		1
#define PROMPTROW	11

/* these are the terminfo indexes for the basic colors it guarantees */
#define COLOR_BLACK	0
#define COLOR_BLUE	1
#define COLOR_GREEN	2
#define COLOR_CYAN	3
#define COLOR_RED	4
#define COLOR_MAGENTA	5
#define COLOR_YELLOW	6
#define COLOR_WHITE	7

static int deck_size = PACK_SIZE;	/* initial deck */
static int deck[PACK_SIZE];

static int grid[GRID_LENGTH];	/* card layout grid */
static int freeptr[4];		/* free card space pointers */

static int deal_number=0;

static char *suits[] = {"h", "s", "d", "c" };

/*
 * Here goes the screen support. We don't use terminfo/termcap because of
 * the incredible braindamage they manifest when you try to output control
 * characters -- on IBM-compatible machines this leads to serious lossage.
 *
 * The only functions this code requires from curses(3) are:
 *
 *	cbreak(), nocbreak(), tgetstr(), tgetnum(), tputs(), tparm(), tgoto()
 *
 * so a port to MS-DOS using PC-Curses or some variant thereof ought to be
 * fairly easy.
 */

static int CO, LI;
static char *HO, *CL, *CM, *CE, *SO, *SE, *VS, *VE, *HI, *HE, *SF, *SB;

/* lowest-level character output */

static void xputc(c)
char c;
{
    putchar(c);
}

void xputs(s)
char *s;
{
    tputs(s, 1, (int (*)())xputc);
}

static void initscr()
{
    extern char *tgetstr();
    extern char *tparm(), *tgetstr();
    extern char *getenv();
    extern int tgetnum();
    void clear();

    register char *term;
    register char *tptr;
    register int i;

    if((term = getenv("TERM")) == (char *)NULL)
    {
	(void) fprintf(stderr, "Can't get TERM.\n");
	exit(1);
    }

    tptr = malloc(10000);
    if (tgetent(tptr, term) < 1)
    {
	(void) fprintf(stderr, "Unknown terminal type: %s.\n", term);
	exit(1);
    }

    HO = tgetstr("ho", (char *)NULL);
    CO = tgetnum("co");
    LI = tgetnum("li");
    if(!(CL = tgetstr("cl", (char *)NULL)) || !(CM = tgetstr("cm", (char *)NULL)))
    {
	(void) fprintf(stderr, "Blue needs CL and CM\n");
	exit(1);
    }
    CE = tgetstr("ce", (char *)NULL);
    SO = tgetstr("so", (char *)NULL);
    SE = tgetstr("se", (char *)NULL);
    VS = tgetstr("vs", (char *)NULL);
    VE = tgetstr("ve", (char *)NULL);
    HI = tgetstr("hi", (char *)NULL);
    HE = tgetstr("he", (char *)NULL);
    SF = SB = "";
    if (tgetnum("Co") >= 8)
    {
	SF = tgetstr("Sf", (char *)NULL);
	SB = tgetstr("Sb", (char *)NULL);
    }

    free(tptr);

    if (strncmp(term, IBMPREFIX, sizeof(IBMPREFIX) - 1) == 0)
    {
	suits[HEARTS] = HEART;
	suits[SPADES] = SPADE;
	suits[DIAMONDS] = DIAMOND;
	suits[CLUBS] = CLUB;
    }

    tputs(VS);
}

static void endwin()
{
    xputs(VE);
}

/* Cursor movements */

static void home()
{
    if(HO)
	xputs(HO);
    else
	xputs(tgoto(CM, 0, 0));
}

static void clear()
{
    xputs(CL);
    home();
}

static void clrtoeol()
{
    xputs(CE);
}

static void refresh()
{
    (void) fflush(stdout);
}

static void move(y, x)
register int y, x;
{
    xputs(tgoto(CM, x - 1, y - 1));
}

static void standout()
{
    if(SO) xputs(SO);
}

static void standend()
{
    if(SE) xputs(SE);
}

static void setfore(color)
int color;
{
    extern char *tparm();

    if (SF)
	xputs(tparm(SF, color));
}

static void setback(color)
int color;
{
    extern char *tparm();

    if (SB)
	xputs(tparm(SB, color));
}

/*
 * OK, here is the game proper
 */

static void die(onsig)
int onsig;
{
    signal(SIGINT, SIG_IGN);
    setfore(COLOR_WHITE);
    setback(COLOR_BLACK);
    if (onsig)
	clear();
    endwin();
#ifdef BSD
    nocrmode();
#else
    nocbreak();
#endif /* BSD */
#ifdef BROKEN
    (void) ioctl(0, TCSETA, &savetty);
#endif /* BROKEN */
    exit(0);
}

static void init_vars()
{
    int i;

    deck_size=PACK_SIZE;
    for(i=0;i<PACK_SIZE;i++)	deck[i]=i;
    for(i=0;i<4;i++)		freeptr[i]=i * GRID_WIDTH;
}

static void shuffle(size)
int size;
{
	int i,j,numswaps,swapnum,temp;

	numswaps=size*10;	/* an arbitrary figure */

	for (swapnum=0;swapnum<numswaps;swapnum++)
	{
		i=rand() % size;
		j=rand() % size;
		temp=deck[i];
		deck[i]=deck[j];
		deck[j]=temp;
	}
}

static void deal_cards()
{
    int ptr, card=0, value, csuit, crank, suit, aces[4];

    for (suit=HEARTS;suit<=CLUBS;suit++)
    {
	ptr=freeptr[suit];
	grid[ptr++]=NOCARD;	/* 1st card space is blank */
	while ((ptr % GRID_WIDTH) != 0)
	{
	    value=deck[card++];
	    crank=value % SUIT_LENGTH;
	    csuit=value / SUIT_LENGTH;
	    if (crank==ACE)
		aces[csuit]=ptr;
	    grid[ptr++]=value;
	}
    }

    if (deal_number==1)		/* shift the aces down to the 1st column */
	for (suit=HEARTS;suit<=CLUBS;suit++)
	{
	    grid[suit * GRID_WIDTH] = suit * SUIT_LENGTH;
	    grid[aces[suit]]=NOCARD;
	    freeptr[suit]=aces[suit];
	}
}

static void printcard(value)
int value;
{
    static char *ranks[SUIT_LENGTH] =
	{" A"," 2"," 3"," 4"," 5"," 6"," 7"," 8"," 9","10"," J"," Q"," K"};

    (void)putchar(' ');
    if (value == NOCARD)
	(void)printf("   ");
    else
    {
	setback(COLOR_WHITE);
	setfore(COLOR_BLUE);
	xputs(ranks[value % SUIT_LENGTH]);
	setfore((value / SUIT_LENGTH) % 2 ? COLOR_BLACK : COLOR_RED);
	xputs(suits[value / SUIT_LENGTH]);
	setfore(COLOR_WHITE);
	setback(COLOR_BLACK);
    }
    (void)putchar(' ');
}

static void display_cards(deal)
int deal;
{
    int row, card;

    clear();
    (void)printf(
		 "Blue Moon 2.1 - by Tim Lister & Eric Raymond - Deal %d.\n",
		 deal);
    for(row=HEARTS;row<=CLUBS;row++)
    {
	move(BASEROW + row + row + 2, 0);
	for(card=0;card<GRID_WIDTH;card++)
	    printcard(grid[row * GRID_WIDTH + card]);
    }

    move(PROMPTROW + 2, 0); refresh();
#define P(x)	(void)printf("%s\n", x)
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
    refresh();
}

static int find(card)
int card;
{
    int i;

    if ((card<0)||(card>=PACK_SIZE))	return NOCARD;
    for(i=0;i<GRID_LENGTH;i++)
	if (grid[i]==card)		return i;
    return NOCARD;
}

static void movecard(src, dst)
int src, dst;
{
    grid[dst]=grid[src];
    grid[src]=NOCARD;

    move( BASEROW + (dst / GRID_WIDTH)*2+2, (dst % GRID_WIDTH)*5 + 1);
    printcard(grid[dst]);

    move( BASEROW + (src / GRID_WIDTH)*2+2, (src % GRID_WIDTH)*5 + 1);
    printcard(grid[src]);

    refresh();
}

static void play_game()
{
    int dead=0, i, j;
    char c;
    int select[4], card;

    while (dead<4)
    {
	dead=0;
	for (i=0;i<4;i++)
	{
	    card=grid[freeptr[i]-1];

	    if (	((card % SUIT_LENGTH)==KING)
		||
		(card==NOCARD)	)
		select[i]=NOCARD;
	    else
		select[i]=find(card+1);

	    if (select[i]==NOCARD)
		dead++;
	};

	if (dead<4)
	{
	    char	live[NSUITS], *lp = live;

	    for (i=0;i<4;i++)
	    {
		if (select[i] != NOCARD)
		{
		    move( BASEROW + (select[i] / GRID_WIDTH)*2+3,
			 (select[i] % GRID_WIDTH)*5);
		    (void)printf("   %c ", *lp++ = 'a' + i);
		}
	    };
	    *lp = '\0';

	    if (strlen(live) == 1)
	    {
		move(PROMPTROW,0);
		(void)printf(
		    "Making forced moves...                                 ");
		(void) fflush(stdout);
		(void) sleep(1);
		c = live[0];
	    }
	    else
	    {
		char	buf[BUFSIZ];

		(void)sprintf(buf,
			"Type [%s] to move, r to redraw, q or DEL to quit: ",
			live);

		do {
		    move(PROMPTROW,0);
		    (void) fputs(buf, stdout);
		    move(PROMPTROW, strlen(buf));
		    clrtoeol();
		    (void) putchar(' ');
		} while
		    (((c = getchar())<'a' || c>'d') && (c!='r') && (c!='q'));
	    }

	    for (j = 0; j < 4; j++)
		if (select[j]!=NOCARD)
		{
		    move(BASEROW + (select[j] / GRID_WIDTH)*2+3,
			 (select[j] % GRID_WIDTH)*5);
		    (void)printf("     ");
		}

	    if (c == 'r')
		display_cards(deal_number);
	    else if (c == 'q')
		die(0);
	    else
	    {
		i = c-'a';
		if (select[i] == NOCARD)
		    (void) putchar('\007');
		else
		{
		    movecard(select[i], freeptr[i]);
		    freeptr[i]=select[i];
		}
	    }
	}
    }

    move(PROMPTROW, 0);
    standout();
    (void)printf("Finished deal %d - type any character to continue...", deal_number);
    standend();
    (void) getchar();
}

static int collect_discards()
{
    int row, col, cardno=0, finish, gridno;

    for (row=HEARTS;row<=CLUBS;row++)
    {
	finish=0;
	for (col=1;col<GRID_WIDTH;col++)
	{
	    gridno=row * GRID_WIDTH + col;

	    if ((grid[gridno]!=(grid[gridno-1]+1))&&(finish==0))
	    {
		finish=1;
		freeptr[row]=gridno;
	    };

	    if ((finish!=0)&&(grid[gridno]!=NOCARD))
		deck[cardno++]=grid[gridno];
	}
    }
    return cardno;
}

static void game_finished(deal)
int deal;
{
    clear();
    (void)printf("You finished the game in %d deals. This is ",deal);
    standout();
    if (deal<2)
	(void)printf("excellent");
    else if (deal<4)
	(void)printf("good");
    else if (deal<8)
	(void)printf("average");
    else
	(void)printf("poor");
    standend();
    (void) printf(".         \n");
    die(0);
}

main()
{
#ifdef BROKEN
    (void) ioctl(0, TCGETA, &savetty);
#endif /* BROKEN */

    signal(SIGINT, die);
    initscr();

#ifdef BSD
    crmode();
#else
    cbreak();
#endif /* BSD */

    srand((int)time((long *)0));

    init_vars();

    do{
	deal_number++;
	shuffle(deck_size);
	deal_cards();
	display_cards(deal_number);
	play_game();
    }
    while
	((deck_size=collect_discards()) != 0);

    game_finished(deal_number);
}

/* blue.c ends here */
