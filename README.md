# Blue Moon

A patience for one: all 52 cards dealt in four rows, the aces moved to the
left, and every move fills a gap with the next card of its suit. Get every
row in order in as few deals as you can; it never takes more than 13.

## Play

With [av](https://asciiverse.sh), the Asciiverse client:

```
av install bluemoon
av play bluemoon
```

Each card you can move into a gap is marked with a letter; press it. When
no move is left, the cards out of order are dealt again. 1 to 3 deals is
good.

## Credits

Tim Lister wrote the game in GFA BASIC for the Atari ST; Eric S. Raymond
gave it its curses screen in 1994 and still maintains it (2.15, 2026). This
build is his latest, centred in the window, with suit symbols, the result
kept on screen and a new game from there; it also builds for Windows.
