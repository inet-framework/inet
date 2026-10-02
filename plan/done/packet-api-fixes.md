# Packet API fixes ahead of #1155

Status: **done 2026-10-02.** Branch `topic/packet-api-fixes`, worktree
`/home/levy/workspace/inet-packet-api-fixes`, first on `master` `5da9809d19`, then rebased onto
`master` `927f2fd00d` (three 802.11 commits). It goes to `master` before #1155, and #1155 is rebased
onto it.

## Why

The owner decided on 2026-10-02 that the TCP branch of #1155 (`topic/tcp-new-audit-fixes`) carries
nothing for other protocols that the TCP protocol tests do not strictly need. A defect in shared
code that the branch exposes is repaired at its root, on a branch of its own that reaches `master`
first, and #1155 then rebases onto it. The owner allowed the `common/packet/` seal to be lifted for
this branch.

Two commits of #1155 repair the packet API, or work around a defect in it:

- `packet: fix: keep the fill byte when splitting a BitCountChunk or ByteCountChunk` repairs the
  chunk peek. It moves here unchanged.
- `ppp: fix: byte-align a packet truncated by a mid-transmission disconnect` works around a defect
  of the serializers: `Ppp` cuts a frame that a disconnect interrupts at the bit where the
  transmission stopped, and `ByteCountChunkSerializer` and `BytesChunkSerializer` cannot serialize
  a slice that starts or ends inside a byte. The run stops with "Cannot convert between integer
  units". The repair here is in the two serializers, so `Ppp` needs no change.

The other non-TCP commits of #1155 do not come here: the PPP rewrite for RFC 1661 and the pcap
link type commit that depends on it are not correct (the `Ppp` module is the whole interface and
needs the RFC 1662 framing), and the SCTP rename only follows a TCP rename.

## Steps

1. **Lift the `common/packet/` seal** — done. for the edits of steps 2 and 3.
2. **The fill byte** — done. (`chunk/BitCountChunk.cc`, `chunk/ByteCountChunk.cc`): the commit of #1155.
3. **Slices that start or end inside a byte** — done. (`serializer/ByteCountChunkSerializer.cc`,
   `serializer/BytesChunkSerializer.cc`), with a unit test in `tests/packet/UnitTest.cc`.
4. **Restore the seal** — done.
5. **Check** — done: the packet, unit, module, serializer and protocol suites, and the fingerprints.

## Evidence

- With #1155 and without the byte rounding in `Ppp`, the fingerprint run of
  `examples/inet/tcp_ppp_reconnect` (ingredients `~tND`) stops at t=3.3000001 s, the moment of the
  disconnect, with "Cannot convert between integer units". With the serializer repair it completes
  to 10 s.
- The new unit test fails with the same error without the repair, and passes with it.

## Results

At `ce71d6eeaf` on `5da9809d19`, in debug mode, and again after the rebase onto `927f2fd00d`:

- packet tests PASS; unit tests 108 PASS; module tests 346 PASS; serializer tests 4 PASS;
- protocol tests: every suite gives the counts of `master`, including master's own unexpected
  failures in igmp, ipsec, mld, mpls, nd and rip;
- fingerprints of the packet data ingredient (`~tND`), the only one that serializes packets: the
  same 638 tests run on `master` and on this branch, and each gives the same result and the same
  calculated value (262 PASS; the 364 mismatches and 10 errors are the same on `master`, because
  the runner compares against its JSON store and not against the CSV files).

The release build of the local OMNeT++ checkout crashes during network setup (its release libraries
are from different dates), so the fingerprints ran in debug mode.

`check-source-seals.sh` reports the four sealed files of steps 2 and 3, as it must: the seal owner
allowed the edits, and the merge still needs the approval of SR-PR-APPROVAL.

