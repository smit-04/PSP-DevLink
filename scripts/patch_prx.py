#!/usr/bin/env python3
"""
Patch a modern PSPSDK PRX (ELF type 0xFFA0) to use the legacy ~PSP magic
that older PSP CFW versions (like PRO-B9) require.
"""
import struct, sys, os

def patch_prx_in_pbp(pbp_path, output_path):
    with open(pbp_path, 'rb') as f:
        data = bytearray(f.read())
    
    # Read PBP header offset for slot 6 (DATA.PSP executable)
    exe_offset = struct.unpack_from('<I', data, 28)[0]
    
    # Check current magic
    current_magic = bytes(data[exe_offset:exe_offset+4])
    print(f"Current executable magic at offset {exe_offset}: {current_magic.hex()} ({current_magic})")
    
    if current_magic == b'\x7fELF':
        # Check ELF type field (offset +16 from start = e_type, 2 bytes)
        elf_type = struct.unpack_from('<H', data, exe_offset + 16)[0]
        print(f"ELF type: 0x{elf_type:04X}")
        
        if elf_type == 0xFFA0:
            print("ELF type is 0xFFA0 (PRX) - patching magic to ~PSP")
            data[exe_offset:exe_offset+4] = b'~PSP'
            
            with open(output_path, 'wb') as f:
                f.write(data)
            print(f"Patched EBOOT written to: {output_path}")
            
            # Verify
            print(f"New magic: {bytes(data[exe_offset:exe_offset+4]).hex()} ({bytes(data[exe_offset:exe_offset+4])})")
        else:
            print(f"ELF type is 0x{elf_type:04X} (not PRX) - cannot patch")
            sys.exit(1)
    elif current_magic == b'~PSP':
        print("Already in ~PSP format, no patching needed")
    else:
        print(f"Unknown format: {current_magic.hex()}")
        sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <EBOOT.PBP> [output.PBP]")
        sys.exit(1)
    
    input_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else input_path
    patch_prx_in_pbp(input_path, output_path)
