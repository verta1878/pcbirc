#!/bin/bash
# setup.sh — Fix PCBOARD.DAT paths for current directory
# Usage: ./setup.sh [dos_drive_letter]
# Example: ./setup.sh C    → paths become C:\path\to\file
#          ./setup.sh      → defaults to C:

DRIVE="${1:-C}"
DIR="$(cd "$(dirname "$0")" && pwd)"

echo "PCBoard Data Directory Setup"
echo "  Data dir: $DIR"
echo "  DOS drive: ${DRIVE}:"
echo ""
echo "NOTE: PCBOARD.DAT contains absolute DOS paths."
echo "When running in DOSBox, mount this directory as ${DRIVE}:"
echo "  mount ${DRIVE} ${DIR}"
echo ""
echo "hexadecimal needs to provide the full data directory."
echo "This is a placeholder with sample PCBOARD.DAT from Clark's test config."
echo ""
echo "See DATA_DIRECTORY.md for the complete file list."
