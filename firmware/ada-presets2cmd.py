def parse(f):
    for l in f:
        l = l.rstrip()
        sp = l.find(" ")
        try:
            preset_no = int(l[:sp])
        except ValueError:
            continue

        program = preset_no - 1
        title = l[sp + 1:]

        print(f'PR 2 {program} "{title}" {program}')

import sys

parse(sys.stdin)
