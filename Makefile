# Makefile for blue, the Blue Moon solitaire game

# Note: When the version changes, you also have to change
#  * the LSM file
#  * the RPM spec file
V=2.4

SOURCES = READ.ME Makefile blue.c blue.c.orig blue.6 blue.lsm *.spec

blue: blue.c
	cc -O -DVERSION=\"$(V)\" blue.c -lncurses -o blue

blue-$(V).tar.gz: $(SOURCES)
	@ls $(SOURCES) | sed s:^:blue-$(V)/: >MANIFEST
	@(cd ..; ln -s blue blue-$(V))
	(cd ..; tar -czvf blue/blue-$(V).tar.gz `cat blue/MANIFEST`)
	@(cd ..; rm blue-$(V))

blue.shar: $(SOURCES)
	shar $(SOURCES) >blue.shar

clean:
	rm -f blue blue.tar blue*.gz blue*.rpm blue.shar *~

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

