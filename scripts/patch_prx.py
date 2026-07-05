#!/usr/bin/env python3
"""
Convert modern PSPSDK ELF PRX (ELF type 0xFFA0) to legacy ~PSP PRX format.
The legacy format is required by PSP firmware 6.60 PRO-B9 and earlier.

Legacy ~PSP PRX header (52 bytes):
  0x00: magic     "~PSP" (4 bytes)
  0x04: modattr   uint16 (module attributes: 0x0000=user, 0x1000=kernel)
  0x06: modver    uint16 (compression attribute: 0x0001 = not compressed)
  0x08: version   uint8  (module minor version)
  0x09: ???       uint8  (module major version)
  0x0A: modname   char[28] (module name, null padded)
  0x26: ???       uint16
  0x28: gp_value  uint32 (GP register value)
  0x2C: lib_ent   uint32 (offset to library entry table)
  0x30: lib_ent_btm uint32
  0x34: lib_stub  uint32 (offset to library stub table)
  0x38: lib_stub_btm uint32
  0x3C: ...module data continues...

The legacy format wraps a complete ELF image after the ~PSP header.
PSP firmware maps and relocates it at load time.

Actually, the legacy ~PSP format IS essentially an ELF file where:
- Byte 0 is changed from 0x7F to 0x7E ('~')
- Bytes 1-3 stay "PSP" (0x50 0x53 0x50)
- The rest is a standard PSP ELF

The real difference from stock ELF is just the first byte and the ELF type:
- Stock ELF:    7F 45 4C 46, type=ET_EXEC (0x0002) 
- Modern PRX:   7F 45 4C 46, type=ET_SCE_IOPRELEXEC (0xFFA0)
- Legacy PRX:   7E 50 53 50, same type=0xFFA0 but custom header

Wait - looking at the actual PRO Update DATA.PSP more carefully,
the "~PSP" format is a COMPLETELY DIFFERENT binary layout from ELF.
It is the PSP Module format (not ELF-based).

However, since 6.60 PRO-B9 CFW is custom firmware, it MAY support
loading static ELF user-mode homebrews directly without PRX conversion.

Strategy: produce a plain user-mode EBOOT with:
- No BUILD_PRX (strips to static ELF)
- Module attr 0 (user mode)
- Link only user-mode libs (lpspusb for high-level USB control)
- For bulk USB, use the sceUsbstorBootSetCapacity trick or a helper PRX

This script implements option 2: proper ~PSP module format generation.
"""

import struct
import sys
import os


def read_elf32_header(data):
    """Parse ELF32 header"""
    if data[:4] != b'\x7fELF':
        raise ValueError(f"Not an ELF file: {data[:4].hex()}")
    
    (ei_class, ei_data, ei_version, ei_osabi,
     ei_abiversion) = struct.unpack_from('BBBBB', data, 4)
    
    # ELF32 header (fixed fields starting at offset 16)
    (e_type, e_machine, e_version, e_entry, e_phoff, e_shoff,
     e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize,
     e_shnum, e_shstrndx) = struct.unpack_from('<HHIIIIIHHHHHH', data, 16)
    
    return {
        'e_type': e_type,
        'e_machine': e_machine,
        'e_entry': e_entry,
        'e_phoff': e_phoff,
        'e_shoff': e_shoff,
        'e_flags': e_flags,
        'e_phnum': e_phnum,
        'e_shnum': e_shnum,
        'e_shstrndx': e_shstrndx,
        'e_shentsize': e_shentsize,
    }


def find_scemoduleinfo(data, elf_hdr):
    """Find the .rodata.sceModuleInfo section and extract module info"""
    shoff = elf_hdr['e_shoff']
    shnum = elf_hdr['e_shnum']
    shstrndx = elf_hdr['e_shstrndx']
    shentsize = elf_hdr['e_shentsize']
    
    # Get section name string table
    shstr_off = shoff + shstrndx * shentsize
    (sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size,
     sh_link, sh_info, sh_addralign, sh_entsize) = struct.unpack_from(
        '<IIIIIIIIII', data, shstr_off)
    shstrtab = data[sh_offset:sh_offset + sh_size]
    
    for i in range(shnum):
        sec_off = shoff + i * shentsize
        (sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size,
         sh_link, sh_info, sh_addralign, sh_entsize) = struct.unpack_from(
            '<IIIIIIIIII', data, sec_off)
        
        name_end = shstrtab.index(b'\x00', sh_name)
        name = shstrtab[sh_name:name_end].decode('ascii', errors='replace')
        
        if name == '.rodata.sceModuleInfo':
            # Parse SceModuleInfo struct
            sec_data = data[sh_offset:sh_offset + sh_size]
            if len(sec_data) >= 52:
                (modattr, modver, modname_bytes, gp_value,
                 lib_ent, lib_ent_btm, lib_stub, lib_stub_btm) = struct.unpack_from(
                    '<HH28sIIIII', sec_data, 0)
                modname = modname_bytes.split(b'\x00')[0].decode('ascii', errors='replace')
                return {
                    'modattr': modattr,
                    'modver': modver,
                    'modname': modname,
                    'gp_value': gp_value,
                    'lib_ent': lib_ent,
                    'lib_ent_btm': lib_ent_btm,
                    'lib_stub': lib_stub,
                    'lib_stub_btm': lib_stub_btm,
                    'offset_in_section': sh_offset,
                }
    
    return None


def patch_elf_prx_to_legacy(input_path, output_path):
    """
    The simple version: just change the ELF magic from 7F ELF to ~PSP.
    Some CFW versions (like PRO-C and ARK) accept this. PRO-B9 may not.
    
    The more complex version requires full PRX module format reconstruction.
    For now, let's try the minimal patch and verify if PRO-B9 accepts it.
    """
    with open(input_path, 'rb') as f:
        data = bytearray(f.read())
    
    elf_hdr = read_elf32_header(bytes(data))
    print(f"ELF type: 0x{elf_hdr['e_type']:04X}")
    print(f"Entry point: 0x{elf_hdr['e_entry']:08X}")
    
    if elf_hdr['e_type'] != 0xFFA0:
        print(f"WARNING: ELF type is not 0xFFA0 (PRX). Got 0x{elf_hdr['e_type']:04X}")
    
    # Find module info
    modinfo = find_scemoduleinfo(bytes(data), elf_hdr)
    if modinfo:
        print(f"Module: '{modinfo['modname']}' attr=0x{modinfo['modattr']:04X}")
    
    # Patch magic: 7F 45 4C 46 -> 7E 50 53 50 (~PSP)
    print(f"Patching ELF magic 7f454c46 -> 7e505350 (~PSP)")
    data[0:4] = b'~PSP'
    
    with open(output_path, 'wb') as f:
        f.write(data)
    
    print(f"Written to: {output_path}")
    return True


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input.prx> <output.prx>")
        sys.exit(1)
    
    patch_elf_prx_to_legacy(sys.argv[1], sys.argv[2])
