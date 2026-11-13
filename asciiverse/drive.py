"""drive.py SESSION: plays Blue Moon in a 100x30 tmux pane for the catalog
preview (asciiverse/record.sh).

Reads the prompt ("Type [abd] to move") and plays the first row offered,
at a brisk person's pace, presses a key at the end of each deal, and stops
at the result or after SECONDS_LIMIT seconds (default 70). record.sh deals
game 2, which this way ends in three deals."""
import os, re, subprocess, sys, time

S = sys.argv[1]
secs = float(os.environ.get("SECONDS_LIMIT", "70"))
TMUX = ["tmux", "-L", os.environ.get("TMUX_SOCKET", "default")]


def key(k):
    subprocess.run(TMUX + ["send-keys", "-t", S, "-l", k])


def screen():
    return subprocess.run(TMUX + ["capture-pane", "-p", "-t", S],
                          capture_output=True, text=True).stdout


time.sleep(2.0)
start = time.time()
while time.time() - start < secs:
    s = screen()
    if "You finished" in s:
        break
    if "type any character" in s:
        time.sleep(1.5)
        key(" ")
        time.sleep(0.8)
        continue
    m = re.search(r"Type \[([abcd]+)\]", s)
    if m:
        time.sleep(0.22)
        key(m.group(1)[0])
        time.sleep(0.12)
    else:
        time.sleep(0.05)
time.sleep(3.0)
