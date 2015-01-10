# Makefile for bluemoon, the Blue Moon solitaire game

VERS=2.12

# In case the name of the installed binary and man page needs to change.
BLUEMOON_TARGET = bluemoon

SOURCES = README COPYING NEWS Makefile control bluemoon.c bluemoon.xml bluemoon.png bluemoon.desktop
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
	@(cd ..; tar -czf bluemoon/bluemoon-$(VERS).tar.gz `cat bluemoon/MANIFEST`)
	@(cd ..; rm bluemoon-$(VERS))

clean:
	rm -f bluemoon *.tar.gz *.6 *.html *~

install: bluemoon.6 uninstall
	install -m 0755 -d $(DESTDIR)/usr/bin
	install -m 0755 -d $(DESTDIR)/usr/share/man/man6
	install -m 0755 -d $(DESTDIR)//usr/share/applications/
	install -m 0755 -d $(DESTDIR)/usr/share/icons/hicolor/48x48/apps/
	install -m 0755 -d $(DESTDIR)/usr/share/appdata/
	install -m 0755 bluemoon $(DESTDIR)/usr/bin/$(BLUEMOON_TARGET)
	install -m 0644 bluemoon.6 $(DESTDIR)/usr/share/man/man6/$(BLUEMOON_TARGET).6
	install -m 0644 bluemoon.desktop $(DESTDIR)/usr/share/applications/
	install -m 0644 bluemoon.png $(DESTDIR)/usr/share/icons/hicolor/48x48/apps/
	install -m 0644 bluemoon.xml $(DESTDIR)/usr/share/appdata/

uninstall:
	rm -f /usr/bin/$(BLUEMOON_TARGET) /usr/share/man/man6/$(BLUEMOON_TARGET).6
	rm -f /usr/share/applications/bluemoon.desktop
	rm -f /usr/share/icons/hicolor/48x48/apps/bluemoon.png
	rm -f /usr/share/appdata/bluemoon.xml

dist: bluemoon-$(VERS).tar.gz

release: bluemoon-$(VERS).tar.gz bluemoon.html
	shipper version=$(VERS) | sh -e -x
