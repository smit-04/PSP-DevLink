#!/usr/bin/env python3
import struct, sys

path = sys.argv[1]
with open(path, 'rb') as f:
    f.seek(28)
    off = struct.unpack('<I', f.read(4))[0]
    f.seek(off)
    magic = f.read(4)
    print(f'Slot 6 offset: {off}, magic bytes: {magic.hex()}, ascii: {magic}')
    if magic == b'~PSP':
        print('FORMAT: PRX (correct for CFW homebrew)')
    elif magic == b'\x7fELF':
        print('FORMAT: Static ELF (WRONG - will cause 8002013C)')
    else:
        print('FORMAT: UNKNOWN')
