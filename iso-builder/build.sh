#!/bin/bash

set -e

echo "=========================================="
echo "  NebulaOS ISO Builder"
echo "=========================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROFILE_DIR="${SCRIPT_DIR}/archlive"
OUTPUT_DIR="${SCRIPT_DIR}/output"
WORK_DIR="${SCRIPT_DIR}/.build-tmp"

echo "[1/3] Cleaning previous builds..."
sudo rm -rf "${WORK_DIR}"
mkdir -p "${OUTPUT_DIR}"

echo "[2/3] Building ISO..."
sudo mkarchiso -v -r -w "${WORK_DIR}" -o "${OUTPUT_DIR}" "${PROFILE_DIR}"

echo "[3/3] Build complete!"
echo ""
echo "ISO location: ${OUTPUT_DIR}/nebulaos-*.iso"
echo ""
echo "To test in QEMU:"
echo "  sudo pacman -S qemu-desktop edk2-ovmf"
echo "  qemu-system-x86_64 -enable-kvm -m 4G -cdrom ${OUTPUT_DIR}/nebulaos-*.iso"
echo ""
echo "To write to USB:"
echo "  sudo dd if=${OUTPUT_DIR}/nebulaos-*.iso of=/dev/sdX bs=4M status=progress && sync"
echo ""
echo "To build NebulaOS components (optional, requires build tools):"
echo "  sudo pacman -S base-devel meson ninja pkg-config gtk4 wlroots wayland-protocols xkbcommon libinput pixman"
echo "  cd ${SCRIPT_DIR}/.."
echo "  meson setup builddir"
echo "  ninja -C builddir"
echo ""
