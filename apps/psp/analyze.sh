#!/bin/bash
export PSPDEV="${PSPDEV:-$HOME/pspdev}"
export PATH="$PSPDEV/bin:$PATH"

cd "$(dirname "$0")"

echo "=== PBP file info ==="
md5sum EBOOT.PBP
file EBOOT.PBP

echo ""
echo "=== Checking PBP structure ==="
# First 4 bytes should be \x00PBP
xxd -l 40 EBOOT.PBP

echo ""
echo "=== Checking .gitattributes for binary handling ==="
cat ../../.gitattributes 2>/dev/null || echo "No .gitattributes found"

echo ""
echo "=== Git LF/CRLF autocrlf setting ==="
cd ../../
git config core.autocrlf

echo ""
echo "=== Checking if EBOOT.PBP is tracked as binary by git ==="
git diff --numstat HEAD~1 -- apps/psp/EBOOT.PBP 2>/dev/null || echo "cannot diff"
git check-attr -a apps/psp/EBOOT.PBP 2>/dev/null
