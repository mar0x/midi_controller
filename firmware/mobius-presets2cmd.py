bank_ab = { 'A': 0, 'B': 1 }

def parse(f):
    progs = [ "" ] * 100

    for l in f:
        l = l.rstrip()
        c = l.split(';')
        bank_str = c[0]
        bank_no = int(bank_str[4:6])
        program = bank_no * 2 + bank_ab[bank_str[7]]
        title = c[-1].strip()

        progs[program] = title

        print(f'PR 0 {program} "{title}" {program}')

    for i, title in enumerate(progs):
        program = 100 + i
        print(f'PR 0 {program} "{title}" {program}')

import sys

parse(sys.stdin)
