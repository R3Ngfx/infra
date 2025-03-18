import sys
import re

for line in sys.stdin:
    m = re.search(r"[^=>] => ([^\s]+)", line)
    if m:
        l = m.groups()[0]
        print(l, end=" ")
