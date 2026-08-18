# Generated captures

The captures here were **recorded from INET simulations** rather than taken off a real
network, because no public capture of the protocol exists (or none that carries the
message types the serializers need). Each one is verified with `tshark` before it enters
the corpus: Wireshark has to decode every frame as the protocol it claims to be, with no
`[Malformed Packet]`.

That check is what makes a self-recorded capture worth anything. On its own it would be
circular -- INET reading back what INET wrote proves only that it is self-consistent --
but a second, independent dissector agreeing on the bytes is real evidence about the wire
format. It has already paid for itself: the same check rejected two other candidate
captures (see "Rejected" below).

What a generated capture is good for afterwards: it is a permanent regression gate on the
serializer, and because `serializer_pcap` drops the byte cache before re-serializing, it
still catches a deserialize/serialize asymmetry. What it cannot do is notice later that
the format itself was wrong all along -- only the `tshark` check at generation time does
that, so re-run it when a capture is regenerated.

## mrp-ring.pcap, mrp-linkcheck.pcap

MRP (Media Redundancy Protocol, IEC 62439-2, EtherType 0x88E3 inside LLC/SNAP), recorded
on `node0` of the 8-node two-ring example, whose scripted link failure at t=1s and repair
at t=2s produce the topology- and link-change messages. Between them the two files cover
nine message types: Test, TopologyChange, LinkUp, LinkDown, InTest, InTopologyChange,
InLinkUp, InLinkDown and (from the link-check configuration) InLinkStatusPoll.

```sh
cd examples/mrp

# the ring and interconnection messages (ring-check interconnection mode)
inet_dbg -u Cmdenv -c MediumNetworkRC -r 0 --sim-time-limit=3s \
    --*.node0.numPcapRecorders=1 \
    '--*.node0.pcapRecorder[0].pcapFile="/tmp/mrp-rc.pcap"' \
    '--*.node0.pcapRecorder[0].fileFormat="pcap"' \
    '--**.fcsMode="computed"' '--**.crcMode="computed"'

# InLinkStatusPoll, which only the link-check interconnection mode sends
inet_dbg -u Cmdenv -c MediumNetworkLC -r 0 --sim-time-limit=3s ...same overrides...

# three frames of every message type, and only those (the link-check run also carries
# CFM continuity checks, which INET cannot round-trip -- see REMAINING_GAPS.md)
editcap -F pcap -r /tmp/mrp-rc.pcap mrp-ring.pcap <frame numbers>
editcap -F pcap -r /tmp/mrp-lc.pcap mrp-linkcheck.pcap <frame numbers>

# verification: every frame must be PN-MRP and nothing may be malformed
tshark -r mrp-ring.pcap -T fields -e _ws.col.Protocol -e _ws.col.Info | sort | uniq -c
tshark -r mrp-ring.pcap -Y "_ws.malformed" | wc -l    # must print 0
```

The two `--**.fcsMode`/`--**.crcMode` overrides are what makes the frames serializable at
all: with the default declared-correct FCS the recorder cannot write them.

Verified beyond the structure: the domain UUID of each frame is one of the two the ini
configures (111 for the first ring, 222 for the second), the sequence ids increase, and
`MRP_SA` is the address of the node that sent the frame -- which it was not when these
captures were first recorded. Comparing that field against the Ethernet source is what
uncovered it: the module read the bridge address from the relay in the initialization
stage the relay assigns it in, so every frame carried an all-zero source address.

## mld-query.pcap, mld-v2.pcap

MLD, recorded on the router of `examples/ipv6/mld`. The real capture in the corpus
(`mld.pcap`) carries only MLDv1 reports and a done, so the queries had nowhere to come
from: `mld-query.pcap` holds a general query, a multicast-address-specific query (the one
a router sends after a listener leaves) and the done that triggers it, `mld-v2.pcap` an
MLDv2 query and two MLDv2 reports of different record counts.

```sh
cd examples/ipv6/mld
inet_dbg -u Cmdenv -c MldDemo  -r 0 ...recorder overrides on *.router...
inet_dbg -u Cmdenv -c MldV2Ssm -r 0 ...recorder overrides on *.router...
editcap -F pcap -r /tmp/mld-v1.pcap mld-query.pcap 2 10 16 17
editcap -F pcap -r /tmp/mld-v2.pcap mld-v2.pcap 1 2 4 11
```

A third override is needed here on top of the FCS one: `--**.checksumMode="computed"`,
without which the run stops at the first UDP packet it has to serialize.

## mrp-mra.pcap

The option TLV an auto-manager appends to its frames, with all three of its sub-TLVs
(`MRP_AutoMgr`, `MRP_TestMgrNAck`, `MRP_TestPropagate`), recorded on `node0` of
`SmallNetworkMRA` -- three frames of each. It took three fixes to INET before Wireshark
would decode these frames at all, one of which was a crash in INET's own receive path;
until then no capture of them could be admitted.

Verified beyond the structure: `MRP_SA` is the node that sent the frame, the priority is
the documented auto-manager default (0xa000), and the addresses a TestMgrNack names are
those of the two managers contending for the ring.

## tsn-rtag.pcap

The IEEE 802.1CB R-TAG, recorded on switch `s2a` of
`showcases/tsn/framereplication/manualconfiguration`, where frames are replicated and
therefore tagged. Wireshark decodes the frames as `eth:ethertype:rtag:ethertype:vlan:
ethertype:ip:udp:data`, i.e. the R-TAG in front of a VLAN tag.

## rtp-rtcp.pcap

RTCP receiver reports and RTP with an MPEG payload, recorded on the `receiver` of
`examples/rtp/unicast` (RTP on UDP 5004, RTCP on 5005 -- the test registers both ports,
since neither protocol has a fixed one). The SIP capture already in the corpus carries a
sender report, a source description and a bye, but no receiver report.

The hosts of that example are connected by a PPP link, so this is a link-type-204 capture
and it exercises `PppHeader` and `PppTrailer` as well -- the only PPP frames in the corpus.

## Rejected

- **MRA (auto-manager) MRP frames** were refused three times before they were admitted as
  `mrp-mra.pcap`; the story is in REMAINING_GAPS.md, and it is the best example
  of what this check is for.
- **PPP** was refused as a capture of its own -- and then arrived anyway: the RTP example
  runs over a PPP link, so `rtp-rtcp.pcap` is a link-type-204 capture and PPP frames do
  round-trip in the corpus. What the refusal was about is the framing, and that stands as
  a documented gap: `PppHeader` carries the HDLC flag octet inside the header while link
  type 204 begins with a direction indicator, and Wireshark decodes the result only
  because the two line up. See REMAINING_GAPS.md.
- **PIM State Refresh.** The shipped PIM-DM example never originates one, so there was
  nothing to record.
- **CFM continuity checks** carried by the same link-check run: Wireshark reports them
  malformed as well (the MEG ID is not the 802.1ag MAID), which is the gap `cfm.pcap`
  already documents on the known-failure side.
