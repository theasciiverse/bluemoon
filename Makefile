# Makefile for blue, the Blue Moon solitaire game

SOURCES = READ.ME Makefile blue.c blue.c.orig blue.6

blue: blue.c
	cc -O blue.c -lcurses -o blue

blue.tar: $(SOURCES)
	tar -cvf blue.tar $(SOURCES)
blue.tar.gz: blue.tar
	gzip blue.tar

blue.shar: $(SOURCES)
	shar $(SOURCES) >blue.shar

clean:
	rm -f blue blue.tar blue.tar.gz blue.shar *~
