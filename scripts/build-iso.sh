#!/bin/bash
# NebulaOS ISO Build Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ISO_DIR="$PROJECT_DIR/iso"
OUTPUT_DIR="$HOME/nebulaos-output"

echo "=== Building NebulaOS ISO ==="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (sudo)"
    exit 1
fi

# Check for archiso
if ! command -v mkarchiso &> /dev/null; then
    echo "Error: archiso not found. Install with: sudo pacman -S archiso"
    exit 1
fi

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Clean previous build
rm -rf /tmp/nebulaos-work

echo ""
echo "Building ISO..."
echo "This may take several minutes..."
echo ""

mkarchiso -v \
    -w /tmp/nebulaos-work \
    -o "$OUTPUT_DIR" \
    "$ISO_DIR"

echo ""
echo "=== ISO Build Complete ==="
echo "Output: $OUTPUT_DIR/nebulaos-*.iso"
echo ""
echo "To test in QEMU:"
echo "  qemu-system-x86_64 -cdrom $OUTPUT_DIR/nebulaos-*.iso -m 4G -enable-kvm"
echo ""
echo "To write to USB:"
echo "  sudo dd if=$OUTPUT_DIR/nebulaos-*.iso of=/dev/sdX bs=4M status=progress"
