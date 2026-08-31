#!/usr/bin/env bash
#
# The seal gate for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# It checks the seal flags in doc/project/ against the SR-* rules of rule/sealing.md:
#
#   SR-FLAG-VALUES     — only the two flags exist
#   SR-FLAG-PLACEMENT  — a flag never appears in a heading line
#   SR-FLAG-COVERAGE   — a flag matches the unit the header declares; "complete" means every unit
#   SR-PROMOTE         — a by-unit document whose units are all closed should be promoted
#   SR-STATE-WHERE     — the generated index in audit/seal-list.md matches the tree
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-seals.sh            # check
#   doc/project/enforcement/check-seals.sh --write    # check, and rewrite the generated index
#
# Exit status 0 = clean, 1 = findings.

set -uo pipefail
cd "$(dirname "$0")/../../.." || exit 2
exec python3 doc/project/enforcement/check_seals.py "$@"
