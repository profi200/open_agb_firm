#!/usr/bin/env python3

# EEPROM Fixer v1.0
# By HTV04

import sys

with open(sys.argv[1], 'rb') as f:
    save = f.read()

length = len(save)
if length < 32 or length & (length - 1) != 0:
    print('Broken save file or incorrect size!')
    exit()

swapsave = b''
for i in range(0, length, 8):
    swapsave += save[i:i + 8][::-1]

with open(sys.argv[1], 'wb') as f:
    f.write(swapsave)
