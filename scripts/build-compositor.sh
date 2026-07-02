#!/bin/bash
# NebulaOS Build Script - Build the compositor

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
COMPOSITOR_DIR="$PROJECT_DIR/compositor"

echo "=== Building NebulaOS Compositor ==="

cd "$COMPOSITOR_DIR"

# Clean previous build
rm -rf build

# Configure with meson
echo "Configuring..."
meson setup build

# Build
echo "Building..."
ninja -C build

echo ""
echo "=== Compositor built successfully ==="
echo "Binary: $COMPOSITOR_DIR/build/nebula-compositor"
