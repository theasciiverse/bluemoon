# Makefile for bluemoon, the Blue Moon solitaire game

VERS=2.7

SOURCES = READ.ME COPYING Makefile bluemoon.c bluemoon.xml *.spec
CFLAGS=-O

bluemoon: bluemoon.c
	cc $(CFLAGS) -DRELEASE=\"$(VERS)\" bluemoon.c -lncurses -o bluemoon

bluemoon.6: bluemoon.xml
	xmlto man bluemoon.xml

bluemoon-$(VERS).tar.gz: $(SOURCES) bluemoon.6
	@ls $(SOURCES) bluemoon.6 | sed s:^:bluemoon-$(VERS)/: >MANIFEST
	@(cd ..; ln -s bluemoon bluemoon-$(VERS))
	(cd ..; tar -czvf bluemoon/bluemoon-$(VERS).tar.gz `cat bluemoon/MANIFEST`)
	@(cd ..; rm bluemoon-$(VERS))

clean:
	rm -f bluemoon bluemoon.tar bluemoon*.gz bluemoon*.rpm bluemoon.6 *~

install: bluemoon.6 uninstall
	cp bluemoon /usr/bin
	cp bluemoon.6 /usr/share/man/man6/bluemoon.6

uninstall:
	rm -f /usr/bin/bluemoon /usr/share/man/man6/bluemoon.6

dist: bluemoon-$(VERS).tar.gz

RPMROOT=/usr/src/redhat
rpm: dist
	rpmbuild --define 'myversion $(VERS)' -ta bluemoon-$(VERS).tar.gz
	cp $(RPMROOT)/RPMS/*/bluemoon-$(VERS)*.rpm .
	cp $(RPMROOT)/SRPMS/bluemoon-$(VERS)*.src.rpm .
