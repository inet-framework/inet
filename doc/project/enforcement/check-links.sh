#!/usr/bin/env bash
#
# The link gate for doc/project/ — a T3 fitness function (see AR-QUAL-ENFORCED).
# It enforces DR-LINK-RELATIVE: every relative markdown link resolves, including its #anchor.
#
# An anchor is generated the way GitHub generates it — lowercase, punctuation removed, spaces
# turned to hyphens — which is why DR-ID-HEADING makes an identifier heading hold the identifier
# alone: the anchor is then the lowercased identifier, and it survives a rewording.
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-links.sh
#
# Exit status 0 = clean, 1 = broken links.

set -uo pipefail
cd "$(dirname "$0")/../../.." || exit 2
exec python3 doc/project/enforcement/check_links.py doc/project
