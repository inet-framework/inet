#!/usr/bin/env bash
#
# Starter architecture check for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# Enforces two dependency-direction rules from architectural-requirements.md over the
# C++ #include graph:
#
#   AR-ORG-DOMAINS   — the shared 'common' package must not depend on any protocol layer
#                      (dependencies point protocols -> infrastructure, never the reverse)
#   AR-ORG-VIS-SPLIT — model/protocol code must not depend on the visualizer package
#
# Usage (from the INET repository root):
#   doc/tmp/enforcement/check-architecture.sh [SRC_DIR]     # default SRC_DIR=src/inet
#
# Exit status 0 = clean, 1 = violations found. Wire it into CI to make the rule a gate.
# This is intentionally a grep-level starter; a robust version would parse the full
# include graph (e.g. dependency-cruiser / a small Python tool) and check for cycles.

set -uo pipefail
SRC="${1:-src/inet}"
status=0

if [ ! -d "$SRC" ]; then
  echo "error: source dir '$SRC' not found (run from the INET repo root)" >&2
  exit 2
fi

LAYERS='physicallayer|linklayer|networklayer|transportlayer|routing|applications'

# Foundational value types that are depended on framework-wide. These are sanctioned
# exceptions (AS-* in architecture-exceptions.md) — ideally they would live in common/,
# but until they are moved, coupling to them is accepted rather than flagged.
ALLOW='networklayer/contract/ipv4/Ipv4Address\.h'
ALLOW+='|networklayer/contract/ipv6/Ipv6Address\.h'
ALLOW+='|networklayer/common/L3Address(Resolver)?\.h'
ALLOW+='|linklayer/common/MacAddress\.h'
ALLOW+='|linklayer/common/EtherType_m\.h'
ALLOW+='|networklayer/common/IpProtocolId_m\.h'

echo "== AR-ORG-DOMAINS: common/ must not #include a protocol layer (foundational value types allowlisted) =="
hits=$(grep -rEn "#include \"inet/(${LAYERS})/" "$SRC/common/" 2>/dev/null | grep -vE "$ALLOW")
if [ -n "$hits" ]; then
  echo "$hits" | sed 's/^/  VIOLATION: /'
  echo "  ^ common/ reaches up into a protocol layer — invert the dependency (AR-EXT-ATTACH),"
  echo "    or record a sanctioned exception in architecture-exceptions.md."
  status=1
else
  echo "  ok"
fi

echo
echo "== AR-ORG-VIS-SPLIT: non-visualizer code must not #include visualizer/ =="
if hits=$(grep -rEln "#include \"inet/visualizer/" "$SRC" 2>/dev/null | grep -v "/visualizer/"); then
  echo "$hits" | sed 's/^/  VIOLATION: /'
  echo "  ^ model/protocol code depends on the visualizer — visualizers must subscribe from outside."
  status=1
else
  echo "  ok"
fi

echo
if [ "$status" -eq 0 ]; then
  echo "PASS: architecture checks clean."
else
  echo "FAIL: architecture violations found (record permanent exceptions, fix the rest)."
fi
exit "$status"
