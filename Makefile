# Makefile for blue, the Blue Moon solitaire game

# Note: When the version changes, you also have to change
#  * the LSM file
#  * the RPM spec file
V=2.6

SOURCES = READ.ME Makefile blue.c blue.c.orig blue.sgml blue.lsm *.spec
CFLAGS=-O

blue: blue.c
	cc $(CFLAGS) -DVERSION=\"$(V)\" blue.c -lncurses -o blue

blue.6: blue.xml
	xmlto man blue.xml

blue-$(V).tar.gz: $(SOURCES) blue.6
	@ls $(SOURCES) blue.6 | sed s:^:blue-$(V)/: >MANIFEST
	@(cd ..; ln -s blue blue-$(V))
	(cd ..; tar -czvf blue/blue-$(V).tar.gz `cat blue/MANIFEST`)
	@(cd ..; rm blue-$(V))

clean:
	rm -f blue blue.tar blue*.gz blue*.rpm blue.6 *~

install: blue.6 uninstall
	cp blue /usr/bin
	cp blue.6 /usr/share/man/man6/blue.6

uninstall:
	rm -f /usr/bin/blue /usr/share/man/man6/blue.6

dist: blue-$(V).tar.gz

RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -ba
rpm: dist
	cp blue-$(V).tar.gz $(RPMROOT)/SOURCES;
	cp blue.spec $(RPMROOT)/SPECS
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) blue.spec	
	cp $(RPMROOT)/RPMS/`arch|sed 's/i[4-9]86/i386/'`/blue-$(V)*.rpm .
	cp $(RPMROOT)/SRPMS/blue-$(V)*.src.rpm .

