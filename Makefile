# Makefile for blue, the Blue Moon solitaire game

VERS=2.7

SOURCES = READ.ME COPYING Makefile blue.c blue.xml *.spec
CFLAGS=-O

blue: blue.c
	cc $(CFLAGS) -DRELEASE=\"$(VERS)\" blue.c -lncurses -o blue

blue.6: blue.xml
	xmlto man blue.xml

blue-$(VERS).tar.gz: $(SOURCES) blue.6
	@ls $(SOURCES) blue.6 | sed s:^:blue-$(VERS)/: >MANIFEST
	@(cd ..; ln -s blue blue-$(VERS))
	(cd ..; tar -czvf blue/blue-$(VERS).tar.gz `cat blue/MANIFEST`)
	@(cd ..; rm blue-$(VERS))

clean:
	rm -f blue blue.tar blue*.gz blue*.rpm blue.6 *~

install: blue.6 uninstall
	cp blue /usr/bin
	cp blue.6 /usr/share/man/man6/blue.6

uninstall:
	rm -f /usr/bin/blue /usr/share/man/man6/blue.6

dist: blue-$(VERS).tar.gz

RPMROOT=/usr/src/redhat
rpm: dist
	rpmbuild --define 'myversion $(VERS)' -ta blue-$(VERS).tar.gz
	cp $(RPMROOT)/RPMS/*/blue-$(VERS)*.rpm .
	cp $(RPMROOT)/SRPMS/blue-$(VERS)*.src.rpm .
