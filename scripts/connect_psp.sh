#!/bin/bash
# PSP DevLink — Start Companion and verify PSP USB visibility
# Run this from WSL AFTER you have run 'usbipd attach --wsl --busid X-X' in Windows PowerShell

set -e

COMPANION="$(dirname "$0")/../apps/desktop/build/PSPDevLinkDesktop"

echo "============================================"
echo "   PSP DevLink — Companion Launcher"
echo "============================================"
echo ""

# Check if companion binary exists
if [ ! -f "$COMPANION" ]; then
    echo "[ERROR] PSPDevLinkDesktop binary not found."
    echo "        Build it first with:"
    echo "          cd apps/desktop && cmake -B build && cmake --build build"
    exit 1
fi

# Check if usbip attached any device
echo "[INFO] Checking for USB devices visible to WSL..."
if command -v lsusb &>/dev/null; then
    echo ""
    echo "--- USB Devices ---"
    lsusb
    echo "-------------------"
    echo ""

    # Look for PSP specifically (VID 0xFFFF)
    if lsusb | grep -i "ffff"; then
        echo "[OK]  PSP DevLink device detected! (VID: 0xFFFF)"
    else
        echo "[WARN] PSP not found in USB device list."
        echo "       Make sure you ran in Windows PowerShell (as Admin):"
        echo "         usbipd bind --busid X-X"
        echo "         usbipd attach --wsl --busid X-X"
        echo ""
        echo "       Replace X-X with your PSP's BUSID from: usbipd list"
        echo ""
        read -p "Continue anyway? (y/N): " confirm
        if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
            exit 1
        fi
    fi
else
    echo "[WARN] lsusb not found, skipping USB check."
fi

echo ""
echo "[INFO] Starting PSP DevLink Desktop Companion..."
echo "[INFO] Dashboard will be available at: http://localhost:8080"
echo ""
exec "$COMPANION"
