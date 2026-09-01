#!/usr/bin/env bash
#
# Architecture check for INET — a T3 fitness function (see AR-QUAL-ENFORCED).
# Enforces the mechanical parts of four rules from doc/project/rule/architecture.md:
#
#   AR-ORG-DOMAINS   — the shared 'common' package must not depend on any protocol layer
#                      (dependencies point protocols -> infrastructure, never the reverse)
#   AR-ORG-VIS-SPLIT — model/protocol code must not depend on the visualizer package
#   AR-COM-SOCKETS   — applications must include transport contracts, not implementation
#                      modules, when they use socket-style protocol APIs
#   AR-QUAL-DETERMINISM — model code must not use process-, clock-, or libc-global randomness
#
# Usage (from the INET repository root):
#   doc/project/enforcement/check-architecture.sh            # full check
#   doc/project/enforcement/check-architecture.sh <SUBTREE>  # scope checks to a subset,
#                                                     # e.g. src/inet/common/packet
#
# With no argument, AR-ORG-DOMAINS covers src/inet/common, AR-ORG-VIS-SPLIT and
# AR-QUAL-DETERMINISM cover src/inet, and AR-COM-SOCKETS covers src/inet/applications. A
# SUBTREE argument restricts all applicable checks to that directory — useful for a focused,
# per-package audit report.
#
# Exit status 0 = clean, 1 = candidates found, 2 = invalid scope or usage. Reconcile candidates
# with doc/project/audit/architecture-exceptions.md before classifying them as violations.

set -uo pipefail
if [ "$#" -gt 1 ]; then
  echo "usage: $0 [src/inet/<subtree>]" >&2
  exit 2
fi

SCOPE="${1:-src/inet}"
SCOPE="${SCOPE#./}"
SCOPE="${SCOPE%/}"
if [[ "$SCOPE" == *"//"* || "$SCOPE" == *"/./"* || "$SCOPE" == */. || \
      "$SCOPE" == *"/../"* || "$SCOPE" == */.. ]]; then
  echo "error: scope must be a normalized src/inet path without '.' or '..' components: '$SCOPE'" >&2
  exit 2
fi
if [[ "$SCOPE" != "src/inet" && "$SCOPE" != src/inet/* ]]; then
  echo "error: scope must be src/inet or one of its subtrees: '$SCOPE'" >&2
  exit 2
fi
if [ ! -d "$SCOPE" ]; then
  echo "error: '$SCOPE' not found (run from the INET repo root)" >&2
  exit 2
fi

intersect_scope() {
  local canonical="$1"
  local requested="$2"
  if [[ "$requested" == "$canonical" || "$requested" == "$canonical"/* ]]; then
    printf '%s\n' "$requested"
  elif [[ "$canonical" == "$requested"/* ]]; then
    printf '%s\n' "$canonical"
  fi
}

DOMAIN_SCOPE="$(intersect_scope "src/inet/common" "$SCOPE")"
VIS_SCOPE="$SCOPE"
status=0

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

if [ -n "$DOMAIN_SCOPE" ]; then
  echo "== AR-ORG-DOMAINS: $DOMAIN_SCOPE must not #include a protocol layer (foundational value types allowlisted) =="
  hits=$(grep -rEn "#include \"inet/(${LAYERS})/" "$DOMAIN_SCOPE" 2>/dev/null | grep -vE "$ALLOW")
  if [ -n "$hits" ]; then
    echo "$hits" | sed 's/^/  VIOLATION: /'
    echo "  ^ common/ reaches up into a protocol layer — invert the dependency (AR-EXT-ATTACH),"
    echo "    or record a sanctioned exception in architecture-exceptions.md."
    status=1
  else
    echo "  ok"
  fi
else
  echo "== AR-ORG-DOMAINS: N/A (scope does not intersect src/inet/common) =="
fi

echo
echo "== AR-ORG-VIS-SPLIT: non-visualizer code must not #include visualizer/ =="
if hits=$(grep -rEln "#include \"inet/visualizer/" "$VIS_SCOPE" 2>/dev/null | grep -v "/visualizer/"); then
  echo "$hits" | sed 's/^/  VIOLATION: /'
  echo "  ^ model/protocol code depends on the visualizer — visualizers must subscribe from outside."
  status=1
else
  echo "  ok"
fi

echo
APP_SCOPE="$(intersect_scope "src/inet/applications" "$SCOPE")"
if [ -n "$APP_SCOPE" ]; then
  echo "== AR-COM-SOCKETS: $APP_SCOPE must include transport contracts, not implementations =="
  app_hits=$(grep -rEn --include='*.h' --include='*.cc' --include='*.icc' \
    '^[[:space:]]*#[[:space:]]*include[[:space:]]+"inet/transportlayer/[^/"[:space:]]+/' \
    "$APP_SCOPE" 2>/dev/null | grep -vE '/transportlayer/(contract|common)/' || true)
  if [ -n "$app_hits" ]; then
    echo "$app_hits" | sed 's/^/  CANDIDATE: /'
    echo "  ^ applications may depend on transport contracts, but not transport implementations."
    status=1
  else
    echo "  ok"
  fi
else
  echo "== AR-COM-SOCKETS: N/A (scope does not intersect src/inet/applications) =="
fi

echo
echo "== AR-QUAL-DETERMINISM: $SCOPE =="
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
DET_GATE="$SCRIPT_DIR/check-determinism.py"
if [ ! -f "$DET_GATE" ] || ! command -v python3 >/dev/null 2>&1; then
  echo "error: canonical determinism checker or python3 is unavailable: $DET_GATE" >&2
  exit 2
fi
det_hits=$(python3 "$DET_GATE" "$SCOPE")
det_status=$?
if [ "$det_status" -eq 2 ]; then
  exit 2
elif [ "$det_status" -ne 0 ]; then
  echo "$det_hits" | sed 's/^/  CANDIDATE: /'
  echo "  ^ route stochasticity through the OMNeT++ RNG and derive ordering from model state."
  status=1
else
  echo "  ok"
fi

echo
if [ "$status" -eq 0 ]; then
  echo "PASS: architecture checks clean."
else
  echo "FAIL: architecture candidates found (record permanent exceptions, fix the rest)."
fi
exit "$status"
