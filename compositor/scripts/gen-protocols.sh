#!/bin/sh
# Generate wayland protocol C and H files
# Usage: gen-wayland-protocols.sh <wayland-scanner> <input.xml> <output.c> <output.h>
SCANNER="$1"
INPUT="$2"
OUTPUT_C="$3"
OUTPUT_H="$4"

"$SCANNER" private-code "$INPUT" "$OUTPUT_C"
"$SCANNER" server-header "$INPUT" "$OUTPUT_H"
