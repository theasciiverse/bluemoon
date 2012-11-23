# Makefile for bluemoon, the Blue Moon solitaire game

VERS=2.11

SOURCES = README COPYING NEWS Makefile control bluemoon.c bluemoon.xml bluemoon.png
CFLAGS=-O

bluemoon: bluemoon.c
	cc $(CFLAGS) -DRELEASE=\"$(VERS)\" bluemoon.c -lncurses -o bluemoon

bluemoon.6: bluemoon.xml
	xmlto man bluemoon.xml

bluemoon.html: bluemoon.xml
	xmlto html-nochunks bluemoon.xml 

bluemoon-$(VERS).tar.gz: $(SOURCES) bluemoon.6
	@ls $(SOURCES) bluemoon.6 | sed s:^:bluemoon-$(VERS)/: >MANIFEST
	@(cd ..; ln -s bluemoon bluemoon-$(VERS))
	(cd ..; tar -czf bluemoon/bluemoon-$(VERS).tar.gz `cat bluemoon/MANIFEST`)
	@(cd ..; rm bluemoon-$(VERS))

clean:
	rm -f bluemoon *.tar.gz *.6 *.html *~ SHIPPER.* 

install: bluemoon.6 uninstall
	cp bluemoon /usr/bin
	cp bluemoon.6 /usr/share/man/man6/bluemoon.6

uninstall:
	rm -f /usr/bin/bluemoon /usr/share/man/man6/bluemoon.6

dist: bluemoon-$(VERS).tar.gz

release: bluemoon-$(VERS).tar.gz bluemoon.html
	shipper -u -m -t; make clean 
