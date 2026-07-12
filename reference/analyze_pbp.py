import struct, sys, os

def analyze_pbp(path, label):
    print(f"\n{'='*50}")
    print(f"  {label}")
    print(f"  {path}")
    print(f"{'='*50}")
    
    with open(path, 'rb') as f:
        magic = f.read(4)
        print(f"PBP Magic: {magic}")
        
        version = struct.unpack('<I', f.read(4))[0]
        print(f"PBP Version: 0x{version:08X}")
        
        offsets = []
        for i in range(8):
            offsets.append(struct.unpack('<I', f.read(4))[0])
        
        names = ['PARAM.SFO', 'ICON0.PNG', 'ICON1.PMF', 'PIC0.PNG', 'PIC1.PNG', 'SND0.AT3', 'DATA.PSP/ELF', 'DATA.PSAR']
        
        file_size = os.path.getsize(path)
        for i, (name, off) in enumerate(zip(names, offsets)):
            next_off = offsets[i+1] if i+1 < len(offsets) else file_size
            size = next_off - off
            print(f"  [{i}] {name:15s} offset=0x{off:06X}  size={size:>8d} bytes")
        
        # Read first 16 bytes of executable (slot 6)
        exe_offset = offsets[6]
        f.seek(exe_offset)
        exe_header = f.read(16)
        
        if exe_header[:4] == b'~PSP':
            print(f"\n  Executable format: PRX (~PSP)")
            mod_attr = struct.unpack('<H', exe_header[6:8])[0]
            print(f"  Module attributes: 0x{mod_attr:04X}", end="")
            if mod_attr & 0x1000:
                print(" (KERNEL MODE)")
            else:
                print(" (USER MODE)")
        elif exe_header[:4] == b'\x7fELF':
            print(f"\n  Executable format: Static ELF")
        else:
            print(f"\n  Executable format: UNKNOWN (magic: {exe_header[:4].hex()})")
        
        # Read PARAM.SFO for CATEGORY
        sfo_offset = offsets[0]
        sfo_end = offsets[1]
        f.seek(sfo_offset)
        sfo_data = f.read(sfo_end - sfo_offset)
        
        # Parse SFO to find CATEGORY
        if sfo_data[:4] == b'\x00PSF':
            key_offset = struct.unpack('<I', sfo_data[8:12])[0]
            data_offset = struct.unpack('<I', sfo_data[12:16])[0]
            num_entries = struct.unpack('<I', sfo_data[16:20])[0]
            
            print(f"\n  PARAM.SFO entries ({num_entries}):")
            for i in range(num_entries):
                entry_off = 20 + i * 16
                key_off = struct.unpack('<H', sfo_data[entry_off:entry_off+2])[0]
                fmt = struct.unpack('<H', sfo_data[entry_off+4:entry_off+6])[0]
                val_len = struct.unpack('<I', sfo_data[entry_off+6:entry_off+10])[0]
                val_max = struct.unpack('<I', sfo_data[entry_off+10:entry_off+14])[0]
                val_off = struct.unpack('<I', sfo_data[entry_off+12:entry_off+16])[0]
                
                key = sfo_data[key_offset + key_off:].split(b'\x00')[0].decode('ascii', errors='replace')
                
                if fmt == 0x0204:  # UTF-8 string
                    val = sfo_data[data_offset + val_off:data_offset + val_off + val_len].split(b'\x00')[0].decode('ascii', errors='replace')
                    print(f"    {key:20s} = {val}")
                elif fmt == 0x0404:  # uint32
                    val = struct.unpack('<I', sfo_data[data_offset + val_off:data_offset + val_off + 4])[0]
                    print(f"    {key:20s} = 0x{val:08X} ({val})")

# Analyze all three
base = r"C:\Users\smit0\.gemini\antigravity\scratch\PSP-DevLink"

analyze_pbp(os.path.join(base, "PROUPDATE", "EBOOT.PBP"), "REFERENCE: PRO Update CFW")
analyze_pbp(os.path.join(base, "apps", "psp", "EBOOT.PBP"), "OURS: PSP DevLink")
analyze_pbp(os.path.join(base, "test_hello", "EBOOT.PBP"), "TEST: Hello World")
