#!/usr/bin/env python3
"""Compare internal structure of reference PRX vs our PRX"""
import struct, sys, os

def dump_prx_header(path, label):
    print(f"\n{'='*60}")
    print(f"  {label}")
    print(f"  {path}")
    print(f"{'='*60}")
    
    with open(path, 'rb') as f:
        data = f.read(128)
    
    magic = data[0:4]
    print(f"Magic: {magic} ({magic.hex()})")
    
    if magic == b'~PSP':
        # Legacy PSP PRX header (52 bytes)
        print("Format: Legacy ~PSP PRX")
        mod_attr = struct.unpack_from('<H', data, 4)[0]
        comp_attr = struct.unpack_from('<H', data, 6)[0]
        mod_ver = struct.unpack_from('<B', data, 8)[0]
        mod_name = data[10:38].split(b'\x00')[0].decode('ascii', errors='replace')
        elf_size = struct.unpack_from('<I', data, 44)[0]
        print(f"  Module Attr:    0x{mod_attr:04X}")
        print(f"  Comp Attr:      0x{comp_attr:04X}")
        print(f"  Module Version: {mod_ver}")
        print(f"  Module Name:    '{mod_name}'")
        print(f"  ELF Size:       {elf_size}")
        
        # Print raw hex of first 64 bytes
        print(f"\n  Raw hex (first 64 bytes):")
        for i in range(0, 64, 16):
            hex_str = ' '.join(f'{b:02x}' for b in data[i:i+16])
            ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[i:i+16])
            print(f"    {i:04x}: {hex_str}  |{ascii_str}|")
    
    elif magic == b'\x7fELF':
        print("Format: Modern ELF-based PRX")
        elf_type = struct.unpack_from('<H', data, 16)[0]
        print(f"  ELF Type: 0x{elf_type:04X}")
        
        # Print raw hex of first 64 bytes
        print(f"\n  Raw hex (first 64 bytes):")
        for i in range(0, 64, 16):
            hex_str = ' '.join(f'{b:02x}' for b in data[i:i+16])
            ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[i:i+16])
            print(f"    {i:04x}: {hex_str}  |{ascii_str}|")

base = r"C:\Users\smit0\.gemini\antigravity\scratch\PSP-DevLink"

# Reference PRX extracted from PRO Update
ref_data_psp = os.path.join(base, "PROUPDATE", "DATA.PSP")
dump_prx_header(ref_data_psp, "REFERENCE: PRO Update (DATA.PSP)")

# Our PRX (before patching)
# We need to extract from PBP
our_pbp = os.path.join(base, "apps", "psp", "EBOOT.PBP")
print(f"\n{'='*60}")
print(f"  OURS: PSP DevLink (extracted from EBOOT.PBP)")
print(f"{'='*60}")
with open(our_pbp, 'rb') as f:
    f.seek(28)
    off = struct.unpack('<I', f.read(4))[0]
    f.seek(off)
    data = f.read(128)
    
    magic = data[0:4]
    print(f"Magic: {magic} ({magic.hex()})")
    if magic == b'~PSP':
        print("(This was patched, showing what firmware sees)")
    
    print(f"\n  Raw hex (first 64 bytes):")
    for i in range(0, 64, 16):
        hex_str = ' '.join(f'{b:02x}' for b in data[i:i+16])
        ascii_str = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data[i:i+16])
        print(f"    {i:04x}: {hex_str}  |{ascii_str}|")
    
    elf_type = struct.unpack_from('<H', data, 16)[0]
    print(f"\n  ELF Type at offset 16: 0x{elf_type:04X}")
