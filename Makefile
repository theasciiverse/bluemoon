# Makefile for blue, the Blue Moon solitaire game

# Note: When the version changes, you also have to change
#  * the name of the containing directory
#  * the RPM spec file
V=2.2

SOURCES = READ.ME Makefile blue.c blue.c.orig blue.6 blue.lsm *.spec

blue: blue.c
	cc -O blue.c -lncurses -o blue

blue.tar: $(SOURCES)
	(cd ..; tar -cvf blue-$(V)/blue.tar `echo $(SOURCES) | sed "/\(^\| \)/s// blue-$(V)\//g"`)
blue.tar.gz: blue.tar
	gzip blue.tar

blue.shar: $(SOURCES)
	shar $(SOURCES) >blue.shar

clean:
	rm -f blue blue.tar blue.tar.gz blue.shar *~

rpm: blue.tar.gz
	cp blue.tar.gz /usr/src/SOURCES/blue-$(V).tar.gz
	cp blue.spec /usr/src/SPECS/blue-$(V)-1.spec
	rpm -ba blue-$(V)-1.spec
