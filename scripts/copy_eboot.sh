#!/bin/bash
# PSP DevLink — Copy EBOOT.PBP to PSP Memory Stick
# Run this from inside WSL after attaching your PSP via usbipd

set -e

EBOOT_SRC="$(dirname "$0")/apps/psp/EBOOT.PBP"

echo "============================================"
echo "   PSP DevLink — EBOOT Copy Helper"
echo "============================================"
echo ""

if [ ! -f "$EBOOT_SRC" ]; then
    echo "[ERROR] EBOOT.PBP not found at: $EBOOT_SRC"
    echo "        Run 'make -C apps/psp' first."
    exit 1
fi

# Try to find PSP drive letter via lsblk (when attached via usbipd)
PSP_MOUNT=""

# First check if PSP shows as a mounted drive in /mnt
for drive in /mnt/[d-z]; do
    if [ -d "$drive/PSP/GAME" ]; then
        PSP_MOUNT="$drive"
        break
    fi
done

if [ -n "$PSP_MOUNT" ]; then
    echo "[INFO] Found PSP Memory Stick at: $PSP_MOUNT"
    GAME_DIR="$PSP_MOUNT/PSP/GAME/PSPDevLink"
    mkdir -p "$GAME_DIR"
    cp "$EBOOT_SRC" "$GAME_DIR/EBOOT.PBP"
    echo "[OK]  Copied EBOOT.PBP -> $GAME_DIR/EBOOT.PBP"
    echo ""
    echo "Done! Safely eject your PSP and launch PSPDevLink from the XMB Game Menu."
else
    echo "[WARN] Could not auto-detect PSP Memory Stick mount point."
    echo ""
    echo "Please copy manually:"
    echo "  Source : $EBOOT_SRC"
    echo "  Dest   : [PSP Drive]\PSP\GAME\PSPDevLink\EBOOT.PBP"
    echo ""
    echo "On Windows, you can also find the EBOOT at:"
    echo "  <repo_root>\\apps\\psp\\EBOOT.PBP"
fi
