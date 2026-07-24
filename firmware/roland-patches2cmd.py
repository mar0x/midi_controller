def parse(f):
    for l in f:
        l = l.rstrip()
        c = l.split(';')
        try:
            patch_no = int(c[0])
        except ValueError:
            continue

        program = patch_no - 1
        title = c[1].strip()

        print(f'PR 1 {program} "{title}" {program}')

import sys

parse(sys.stdin)
