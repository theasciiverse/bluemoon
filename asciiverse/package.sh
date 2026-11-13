#!/bin/sh
# Builds bluemoon with the Asciiverse changes (-DAV_CENTRE, and
# -DAV_UTF8_SUITS where CPPFLAGS asks for it with a wide curses) and packs
# dist/bluemoon-<target>.tar.gz (.zip for Windows) for av: the program at
# the top, with COPYING, README.adoc, the manual (bluemoon.adoc) and
# PATCHES.md.
#
#   CURSES_LIBS=-lncursesw CPPFLAGS=-DAV_UTF8_SUITS asciiverse/package.sh x86_64-unknown-linux-musl
#
# CC, CFLAGS, LDFLAGS, CPPFLAGS and CURSES_LIBS as usual.
set -eu
cd "$(dirname "$0")/.."
target="$1"
case "$target" in *windows*) exe=.exe ;; *) exe= ;; esac
# the version, as upstream's Makefile reads it
version=$(sed -n <NEWS.adoc '/::/s/^\([0-9][^:]*\).*/\1/p' | head -1)
${CC:-cc} ${CFLAGS:--O2} ${CPPFLAGS:-} -DAV_CENTRE -Iasciiverse -DRELEASE="\"$version\"" \
  ${LDFLAGS:-} -o "bluemoon$exe" bluemoon.c ${CURSES_LIBS:--lncurses}
stage=stage/bluemoon
rm -rf "$stage" && mkdir -p "$stage" dist
cp "bluemoon$exe" COPYING README.adoc bluemoon.adoc PATCHES.md "$stage/"
if [ -n "$exe" ]; then
  (cd "$stage" && zip -qr "../../dist/bluemoon-$target.zip" .)
else
  tar -czf "dist/bluemoon-$target.tar.gz" -C "$stage" .
fi
ls dist
