# Derived captures

Each file here is a **frame subset of another capture in this corpus**, cut out with
`editcap`. They exist because a capture can be partly serializable: a handful of its
frames hit a documented serializer or model gap while the rest round-trip perfectly.

The whole original capture stays in `serializer_pcap_known_failures.test`, where it
documents the gap and its pinned counts measure how far INET gets. The round-tripping
frames are salvaged into the file below and replayed by `serializer_pcap.test`, so the
serializers they exercise -- SCTP, the 802.11 block-ack and A-MSDU headers, most of the
MIPv6 mobility headers -- are held to byte-exact reproduction instead of only being
counted in an aggregate. Without this split those serializers would have no strict gate
at all: their captures could not be admitted to the clean corpus as a whole.

The frames therefore run twice, once in each test. That is intentional and cheap.

The `.gitignore` one level up re-includes `*.pcap`, which the repo-wide one drops as
simulation output; without it a newly derived capture would be silently left out of a
commit.

## How they were produced

The frame numbers dropped from each source are exactly the ones its test reports as
`differs` or `EXCEPTION`, so the derived file is the complement of the gap:

```sh
cd tests/serializer/pcap

# SCTP: drops the 9 INIT / INIT-ACK frames (on-wire parameter order not preserved)
editcap -F pcap -r wireshark/sctp-www.cap derived/sctp-www-clean.cap \
        4-22 26-30 32-76 79-84

# MIPv6: drops the 9 Binding Update / Acknowledgement frames (mobility options not modelled)
editcap -F pcap -r mipv6.pcap derived/mipv6-clean.pcap \
        1-5 15-16

# 802.11: drops the 8 VHT/HE NDP Announcement frames (control subtype not modelled)
editcap -F pcap -r wlan-blockack-reassoc.pcap derived/wlan-blockack-reassoc-clean.pcap \
        1-140 142-167 175-218

# 802.11: drops the 1 Trigger frame (control subtype not modelled)
editcap -F pcap -r wlan-aggregation.pcap derived/wlan-aggregation-clean.pcap \
        1-9 11-13
```

`dhcp.pcap` and `esp.pcap` have no derived counterpart: every frame of both hits their
gap, so there is nothing to salvage.

## When a gap is fixed

Delete the derived file, drop its `testPcapSerialization()` line, and move the **original**
capture from `serializer_pcap_known_failures.test` to `serializer_pcap.test`. Both tests
then guard the whole capture, and this file has done its job. If the fix is partial,
regenerate the derived file with the frame numbers the test still reports.
