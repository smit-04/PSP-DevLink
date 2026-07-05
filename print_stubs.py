#!/usr/bin/env python3
import struct, sys

def analyze_stubs(elf_path):
    with open(elf_path, 'rb') as f:
        data = f.read()
    
    # We want to find the .lib.stub section
    # Let's read section headers
    # ELF Header:
    # shoff at 32 (4 bytes), shnum at 48 (2 bytes), shentsize at 46 (2 bytes), shstrndx at 50 (2 bytes)
    shoff = struct.unpack_from('<I', data, 32)[0]
    shentsize = struct.unpack_from('<H', data, 46)[0]
    shnum = struct.unpack_from('<H', data, 48)[0]
    shstrndx = struct.unpack_from('<H', data, 50)[0]
    
    # Read string table section header to resolve names
    shstr_offset = shoff + shstrndx * shentsize
    shstr_addr = struct.unpack_from('<I', data, shstr_offset + 16)[0] # sh_offset
    shstr_size = struct.unpack_from('<I', data, shstr_offset + 20)[0] # sh_size
    strtab = data[shstr_addr:shstr_addr+shstr_size]
    
    stub_sec = None
    for i in range(shnum):
        entry = shoff + i * shentsize
        name_off = struct.unpack_from('<I', data, entry)[0]
        sec_name = strtab[name_off:].split(b'\x00')[0].decode('ascii', errors='replace')
        if sec_name == '.lib.stub':
            stub_sec = entry
            break
            
    if not stub_sec:
        print(".lib.stub section not found!")
        return
        
    sh_offset = struct.unpack_from('<I', data, stub_sec + 16)[0]
    sh_size = struct.unpack_from('<I', data, stub_sec + 20)[0]
    
    print(f".lib.stub section offset: 0x{sh_offset:X}, size: {sh_size}")
    
    # Each entry in .lib.stub is 20 bytes (SceLibraryStubTable)
    # struct SceLibraryStubTable {
    #   char * libname;            // offset 0
    #   unsigned short version;     // offset 4
    #   unsigned short attribute;   // offset 6
    #   unsigned char len;          // offset 8
    #   unsigned char vcount;       // offset 9
    #   unsigned short stubcount;   // offset 10
    #   unsigned int * nidtable;    // offset 12
    #   void * stubtable;          // offset 16
    # }
    
    # Since addresses in ELF might be virtual, we need to map virt to file offset.
    # Let's read all program headers or section headers to map virtual addresses.
    sec_headers = []
    for i in range(shnum):
        entry = shoff + i * shentsize
        sh_addr = struct.unpack_from('<I', data, entry + 12)[0]
        sh_off = struct.unpack_from('<I', data, entry + 16)[0]
        sh_size_i = struct.unpack_from('<I', data, entry + 20)[0]
        sec_headers.append((sh_addr, sh_off, sh_size_i))
        
    def va_to_offset(va):
        for addr, offset, size in sec_headers:
            if addr <= va < addr + size:
                return offset + (va - addr)
        return None

    # Read stubs
    num_stubs = sh_size // 20
    print(f"Number of imported libraries: {num_stubs}")
    for i in range(num_stubs):
        entry_offset = sh_offset + i * 20
        libname_va = struct.unpack_from('<I', data, entry_offset)[0]
        version = struct.unpack_from('<H', data, entry_offset + 4)[0]
        attr = struct.unpack_from('<H', data, entry_offset + 6)[0]
        len_val = data[entry_offset + 8]
        vcount = data[entry_offset + 9]
        stubcount = struct.unpack_from('<H', data, entry_offset + 10)[0]
        nidtable_va = struct.unpack_from('<I', data, entry_offset + 12)[0]
        stubtable_va = struct.unpack_from('<I', data, entry_offset + 16)[0]
        
        libname_off = va_to_offset(libname_va)
        libname = data[libname_off:].split(b'\x00')[0].decode('ascii', errors='replace') if libname_off else "unknown"
        
        print(f"\nLibrary: {libname}")
        print(f"  Version: 0x{version:X}, Attributes: 0x{attr:X}")
        print(f"  Stub count: {stubcount}")
        
        nidtable_off = va_to_offset(nidtable_va)
        if nidtable_off:
            nids = []
            for j in range(stubcount):
                nid = struct.unpack_from('<I', data, nidtable_off + j * 4)[0]
                nids.append(f"0x{nid:08X}")
            print(f"  NIDs: {', '.join(nids)}")

if __name__ == '__main__':
    analyze_stubs('apps/psp/PSPDevLink.elf')
