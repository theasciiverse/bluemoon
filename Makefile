# Makefile for bluemoon, the Blue Moon solitaire game
#
# SPDX-FileCopyrightText: (C) Eric S. Raymond <esr@thyrsus.com>
# SPDX-License-Identifier: BSD-2-Clause

PREFIX      ?= /usr/local
BINDIR      ?= $(PREFIX)/bin
DATADIR     ?= $(PREFIX)/share
MANDIR      ?= $(DATADIR)/man

# In case the name of the installed binary and man page needs to change.
BLUEMOON_TARGET = bluemoon

SOURCES = README.adoc COPYING NEWS.adoc Makefile control bluemoon.c bluemoon.adoc bluemoon.png bluemoon.desktop
CFLAGS=-O

VERSION=$(shell sed -n <NEWS.adoc '/::/s/^\([0-9][^:]*\).*/\1/p' | head -1)

# Rules

# Note: to suppress the footers with timestamps being generated in HTML,
# we use "-a nofooter".
# To debug asciidoc problems, you may need to run "xmllint --nonet --noout --valid"
# on the intermediate XML that throws an error.
.SUFFIXES: .html .adoc .6

.adoc.6:
	asciidoctor -D. -a nofooter -b manpage $<
.adoc.html:
	asciidoctor -D. -a nofooter -a webfonts! $<

.PHONY: clean cppcheck spellcheck reflow
.PHONY: install uninstall version dist release refresh

# Build

bluemoon: bluemoon.c
	cc $(CFLAGS) -DRELEASE=\"$(VERSION)\" bluemoon.c -lncurses -o bluemoon

clean:
	rm -f bluemoon *.tar.gz *.6 *.html *~

# Validate

cppcheck:
	@cppcheck --quiet bluemoon.c

spellcheck:
	@spellcheck bluemoon.adoc

reflow:
	@clang-format --style="{IndentWidth: 8, UseTab: ForIndentation}" -i $$(find . -name "*.[ch]")

# Install/uninstall

install:
	install -m 0755 -d $(DESTDIR)$(BINDIR)
	install -m 0755 -d $(DESTDIR)$(MANDIR)/man6
	install -m 0755 -d $(DESTDIR)$(DATADIR)/applications/
	install -m 0755 -d $(DESTDIR)$(DATADIR)/icons/hicolor/48x48/apps/
	install -m 0755 -d $(DESTDIR)$(DATADIR)/appdata/
	install -m 0755 bluemoon $(DESTDIR)$(BINDIR)/$(BLUEMOON_TARGET)
	install -m 0644 bluemoon.6 $(DESTDIR)$(MANDIR)/man6/$(BLUEMOON_TARGET).6
	install -m 0644 bluemoon.desktop $(DESTDIR)$(DATADIR)/applications/
	install -m 0644 bluemoon.png $(DESTDIR)$(DATADIR)/icons/hicolor/48x48/apps/
	install -m 0644 bluemoon.adoc $(DESTDIR)$(DATADIR)/appdata/

uninstall:
	rm -f $(BINDIR)/$(BLUEMOON_TARGET) $(MANDIR)/man6/$(BLUEMOON_TARGET).6
	rm -f $(DATADIR)/applications/bluemoon.desktop
	rm -f $(DATADIR)/icons/hicolor/48x48/apps/bluemoon.png
	rm -f $(DATADIR)/appdata/bluemoon.adoc

# Export

version:
	@echo $(VERSION)

bluemoon-$(VERSION).tar.gz: $(SOURCES) bluemoon.6
	@ls $(SOURCES) bluemoon.6 | sed s:^:bluemoon-$(VERSION)/: >MANIFEST
	@(cd ..; ln -s bluemoon bluemoon-$(VERSION))
	@(cd ..; tar -czf bluemoon/bluemoon-$(VERSION).tar.gz `cat bluemoon/MANIFEST`)
	@(cd ..; rm bluemoon-$(VERSION))

dist: bluemoon-$(VERSION).tar.gz

release: bluemoon-$(VERSION).tar.gz bluemoon.html
	shipper version=$(VERSION) | sh -e -x

refresh: bluemoon.html
	shipper -N -w version=$(VERSION) | sh -e -x

# end
