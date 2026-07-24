import os
import serial
import sys
import time

port_name = sys.argv[1]
fname = sys.argv[2]
size = os.path.getsize(fname)

rate = 115200

with serial.Serial(port_name, rate, timeout=10) as ser:
    fd = f'FD {size}\n'

    ser.write(fd.encode())
    ser.flush()

    with open(fname, 'rb') as f:
        while size != 0:
            fd = f.read(32)

            #print(size)

            r = ser.write(fd)
            #print("write done", r)
            r = ser.flush()
            #print("flosh done", r)

            time.sleep(0.1)

            size -= len(fd)

    print(f'{size} sent')
