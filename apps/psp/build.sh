#!/bin/bash
export PSPDEV=/home/smit0/pspdev
export PATH="$PSPDEV/bin:$PATH"

cd /mnt/c/Users/smit0/.gemini/antigravity/scratch/PSP-DevLink/apps/psp

echo "=== Clean build EBOOT ==="
make clean
make 2>&1

echo ""
echo "=== Clean build driver PRX ==="
cd driver
make clean
make 2>&1
cd ..

echo ""
echo "=== Verify binaries ==="
echo "EBOOT.PBP:"
md5sum EBOOT.PBP
ls -la EBOOT.PBP

echo ""
echo "pspdl_driver.prx:"
md5sum driver/pspdl_driver.prx
ls -la driver/pspdl_driver.prx

echo ""
echo "=== Verify PBP header (first 8 bytes should be 00 50 42 50 00 00 01 00) ==="
xxd -l 8 EBOOT.PBP
