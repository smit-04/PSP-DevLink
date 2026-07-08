#!/bin/bash
export PSPDEV=/home/smit0/pspdev
export PATH="$PSPDEV/bin:$PATH"

cd /mnt/c/Users/smit0/.gemini/antigravity/scratch/PSP-DevLink/apps/psp

# Dump the default specs and modify them to remove network libs
psp-gcc -dumpspecs > psp_no_net.specs

# Remove -lpspnet_inet and -lpspnet_resolver from the spec file
sed -i 's/-lpspnet_inet -lpspnet_resolver//g' psp_no_net.specs

echo "=== Modified spec lib line ==="
grep -A2 '^\*lib:' psp_no_net.specs
