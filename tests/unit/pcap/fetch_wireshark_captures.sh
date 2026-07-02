#!/bin/sh
#
# Download sample packet captures from the Wireshark wiki (SampleCaptures) into
# ./wireshark/, for use as extra input to serializer.test's round-trip.
#
# The curated set targets protocols that serializer.test does not yet cover but
# that INET can actually exercise from a capture: INET must have a dissector for
# the protocol AND be able to reach it through a supported pcap link type -- the
# reader handles Ethernet (linktype 1), raw IEEE 802.11 (105), PPP (204) and
# null/loopback (0), and dispatches deeper by ethertype / IP protocol number.
# Port-based application protocols (BGP, DHCP, RIP, RTP, ...) are NOT dissected
# from a capture and are omitted -- their bytes would just fall back to a
# BytesChunk and exercise nothing.
#
# IMPORTANT: these are third-party captures from the Wireshark wiki. Review each
# file's license/provenance before committing it. They land in ./wireshark/,
# which is git-ignored; wiring a file into serializer.test (with the right hasFcs
# flag) and fixing any serializer bugs it surfaces is a separate, deliberate step.
#
# Not available from the Wireshark wiki (so not fetched):
#   - OSPFv3: no sample capture exists on the wiki or gitlab test/captures.
#   - Raw IEEE 802.11 (linktype 105): every wiki 802.11 sample is radiotap
#     (linktype 127), which the reader does not support.
#
# Usage:
#   ./fetch_wireshark_captures.sh        download + validate into ./wireshark/
#   ./fetch_wireshark_captures.sh -n     dry run: only list what would be fetched
#
set -eu

# The wiki sits behind Cloudflare, which challenges plain curl; a browser
# User-Agent gets the binary through.
UA="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
DEST="$SCRIPT_DIR/wireshark"
DRYRUN=0
[ "${1:-}" = "-n" ] && DRYRUN=1

# target | filename | url
# (filename ending in .gz is decompressed after download; each URL was verified
#  to return a real pcap whose encapsulation the reader supports and that
#  contains the target protocol)
CAPTURES=$(cat <<'EOF'
ospfv2|ospf.cap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/ospf.cap
igmp|IGMP-dataset.pcap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/IGMP-dataset.pcap
pim|pim-reg.cap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/pim-reg.cap
sctp|sctp-www.cap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/sctp-www.cap
mldv2+ipv6ext|v6-http.cap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/v6-http.cap
vlan-8021q|vlan.cap.gz|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/vlan.cap.gz
mpls|mpls-basic.cap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/mpls-basic.cap
ppp|PPP-config.cap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/PPP-config.cap
gptp-ptpv2|ptpv2.pcap|https://wiki.wireshark.org/uploads/__moin_import__/attachments/SampleCaptures/ptpv2.pcap
stp-bpdu|stp.pcap|https://wiki.wireshark.org/uploads/8d3d0627231ab1e2fa5d3fe8be2390a7/stp.pcap
EOF
)

mkdir -p "$DEST"

ok=0
fail=0
printf '%s\n' "$CAPTURES" | while IFS='|' read -r target fname url; do
    [ -n "${target:-}" ] || continue
    final="$fname"
    case "$fname" in *.gz) final="${fname%.gz}" ;; esac
    printf '=== %-14s %s\n' "$target" "$final"
    if [ "$DRYRUN" = 1 ]; then
        echo "    $url"
        continue
    fi
    out="$DEST/$fname"
    # --retry also retries HTTP 429 (Cloudflare rate limiting) with backoff;
    # --remove-on-error avoids leaving a truncated file behind on failure
    if ! curl -fsSL -m 60 --retry 4 --retry-delay 3 --retry-all-errors --remove-on-error -A "$UA" -o "$out" "$url"; then
        echo "    DOWNLOAD FAILED: $url" >&2
        fail=$((fail + 1))
        continue
    fi
    case "$fname" in *.gz) gunzip -f "$out"; out="$DEST/$final" ;; esac
    if command -v capinfos >/dev/null 2>&1; then
        enc=$(capinfos -E "$out" 2>/dev/null | awk -F': +' '/encapsulation/{print $2}')
        printf '    encapsulation: %s\n' "${enc:-unknown}"
        case "$enc" in
            Ethernet|*802.11*|PPP*|[Nn]ull*|NULL*) : ;;
            *) echo "    WARNING: unsupported encapsulation (reader supports Ethernet / raw-802.11 / PPP / Null-Loopback)" ;;
        esac
    fi
    ok=$((ok + 1))
    sleep 2 # be gentle with the wiki (Cloudflare rate limiting)
done

echo
echo "Downloaded into: $DEST"
echo "Next: review licenses, then wire the wanted files into serializer.test"
echo "      (testPcapSerialization(\"../../pcap/wireshark/<file>\", hasFcs)) and"
echo "      handle any round-trip failures they surface."
