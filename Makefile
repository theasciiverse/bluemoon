# Makefile for bluemoon, the Blue Moon solitaire game

VERSION=$(shell sed -n <NEWS.adoc '/::/s/^\([0-9][^:]*\).*/\1/p' | head -1)

# In case the name of the installed binary and man page needs to change.
BLUEMOON_TARGET = bluemoon

SOURCES = README COPYING NEWS.adoc Makefile control bluemoon.c bluemoon.adoc bluemoon.png bluemoon.desktop
CFLAGS=-O

bluemoon: bluemoon.c
	cc $(CFLAGS) -DRELEASE=\"$(VERSION)\" bluemoon.c -lncurses -o bluemoon

# Note: to suppress the footers with timestamps being generated in HTML,
# we use "-a nofooter".
# To debug asciidoc problems, you may need to run "xmllint --nonet --noout --valid"
# on the intermediate XML that throws an error.
.SUFFIXES: .html .adoc .6

.adoc.6:
	asciidoctor -D. -a nofooter -b manpage $<
.adoc.html:
	asciidoctor -D. -a nofooter -a webfonts! $<

bluemoon-$(VERSION).tar.gz: $(SOURCES) bluemoon.6
	@ls $(SOURCES) bluemoon.6 | sed s:^:bluemoon-$(VERSION)/: >MANIFEST
	@(cd ..; ln -s bluemoon bluemoon-$(VERSION))
	@(cd ..; tar -czf bluemoon/bluemoon-$(VERSION).tar.gz `cat bluemoon/MANIFEST`)
	@(cd ..; rm bluemoon-$(VERSION))

clean:
	rm -f bluemoon *.tar.gz *.6 *.html *~

cppcheck:
	@cppcheck --quiet bluemoon.c

reflow:
	@clang-format --style="{IndentWidth: 8, UseTab: ForIndentation}" -i $$(find . -name "*.[ch]")

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
	install -m 0644 bluemoon.adoc $(DESTDIR)/usr/share/appdata/

uninstall:
	rm -f /usr/bin/$(BLUEMOON_TARGET) /usr/share/man/man6/$(BLUEMOON_TARGET).6
	rm -f /usr/share/applications/bluemoon.desktop
	rm -f /usr/share/icons/hicolor/48x48/apps/bluemoon.png
	rm -f /usr/share/appdata/bluemoon.adoc

version:
	@echo $(VERSION)

dist: bluemoon-$(VERSION).tar.gz

release: bluemoon-$(VERSION).tar.gz bluemoon.html
	shipper version=$(VERSION) | sh -e -x

refresh: bluemoon.html
	shipper -N -w version=$(VERSION) | sh -e -x
