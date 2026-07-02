#!/bin/bash
# NebulaOS QEMU Test Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$HOME/nebulaos-output"

# Find the ISO
ISO_FILE=$(ls -1 "$OUTPUT_DIR"/nebulaos-*.iso 2>/dev/null | head -1)

if [ -z "$ISO_FILE" ]; then
    echo "Error: No ISO found in $OUTPUT_DIR"
    echo "Run build-iso.sh first"
    exit 1
fi

echo "=== Running NebulaOS in QEMU ==="
echo "ISO: $ISO_FILE"
echo ""
echo "Controls:"
echo "  Ctrl+Alt+G  - Release mouse"
echo "  Ctrl+Alt+F2 - Switch to TTY"
echo "  Power off   - Close QEMU window"
echo ""

# Check for KVM
KVM_FLAG=""
if [ -e /dev/kvm ]; then
    KVM_FLAG="-enable-kvm"
    echo "Using KVM acceleration"
else
    echo "Warning: KVM not available, running without acceleration (slow)"
fi

qemu-system-x86_64 \
    -cdrom "$ISO_FILE" \
    -m 4G \
    -smp 4 \
    -cpu host \
    $KVM_FLAG \
    -vga virtio \
    -display gtk,gl=on \
    -device virtio-net-pci,netdev=net0 \
    -netdev user,id=net0 \
    -boot d
