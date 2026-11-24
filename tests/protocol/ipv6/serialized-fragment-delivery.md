# Serialized fragments through normal IPv6 delivery

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [publication plan C03](../../../plan/pending/6lowpan-publication-plan.md#c03)

Publication commit C03: `ipv6: fix: retain decoded Fragment header during local delivery`. Test: [Ipv6SerializedFragmentDelivery.test](Ipv6SerializedFragmentDelivery.test).

Execution: **C03 debug/release PASS; identical-fixture parent runtime FAIL (exit139)**. Both matching builds pass. PR01's assigned checksum unit, seven module controls and six fingerprint comparisons in two QoS simulations also pass. This fixture reuses the existing `Rfc8200SourceFragmentation.test` topology, configurator and `PacketTap` support. It complements `tests/module/Ipv6SerializedFragment.test`, whose overridden local continuation isolates ownership. This protocol fixture overrides no IPv6 method: receiver continuation reaches actual UDP and UdpSink.

```mermaid
flowchart LR
 A["hostA: UDP source, MTU1280"] --> R["IPv6 router"]
 R --> T["Existing PacketTap: representation only"]
 T --> B["hostB: full IPv6 → UDP → UdpSink"]
```

Diagram rendering is unverified. One seed-0 1452-byte UDP application payload starts at 5 s after IPv6 discovery/DAD. Source IPv6 creates the ordinary two-fragment representation for the 1500-byte packet, with one identification and offsets0/1232. The router forwards both through Ethernet and the existing store-and-forward tap. The simulation runs to10 s to detect extra deliveries; no forged fragments, direct reassembly call or private helper substitutes for this path.

A test-local rule on the existing tap selects protocol44 packets. It measures the source/destination, identification, offset and M flag from actual serialized bytes. Using the public equal-length `Packet::replaceDataAt`, it replaces only the complete IPv6 region with its byte-identical BytesChunk representation, retaining Ethernet prefix, padding/FCS suffix, packet identity/front/back offsets, packet tags and region-tag ownership. Ethernet FCS is explicitly computed because the serializer rejects declared-only FCS when reading the whole frame as bytes. The current IPv6 base header is40 bytes with a separate8-byte Fragment chunk; the checker peeks the full typed base header without imposing a peek length, then replaces40 plus the wire payload length. It validates whole-frame byte equality and observes two conversions. Nothing is removed/reinserted into gates, and the existing tap retains frame queues/transmission timing and lifecycle ownership. The normal receiving MAC/IPv6 stack must now decode the IPv6/Fragment representation from bytes instead of borrowing an existing typed fragment chunk.

The source app's actual payload bytes are captured before transport processing. After normal receiver IPv6 reassembly, UDP must observe exactly one1460-byte datagram, source/destination ports4000/5000, the source/target IPv6 identities from the actual fragments and the same1452 payload bytes. UdpSink must receive those exact bytes once. These independent transport/application counts prohibit duplicate delivery. Computed UDP/ICMP checksums retain ordinary stack behavior; no fabricated success state is installed.

The parent holds a raw Fragment-header pointer after the scoped decoded owner has expired during `Ipv6::localDeliver`; the C03 repair retains that owner until reassembly finishes using it. Serialized chunks make receive-time decode reachable through the full protocol path. On exact parent C02 at `bf2feaae570ed6cc7c118f2d9effdd56965c12c9`, this unchanged fixture compiles and reaches reception, then terminates with exit139. Its retained backtrace identifies `Ipv6.cc:771`, reading `fh->getFragmentOffset()` in `Ipv6::localDeliver` after the owner's scope has ended. This is an observed runtime crash, not a missing API or compilation failure. Freed-pointer behavior can also appear to pass under another allocator/run; this single observation does not establish deterministic failure. This case checks valid source fragmentation and normal continuation, not malformed/overlap policy, arbitrary allocator schedules or comprehensive RFC8200 behavior.

Reproduction by the sole runner, after a fresh matching debug build:

```sh
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent \
  -w '^tests/protocol/ipv6$' -f 'Ipv6SerializedFragmentDelivery\.test$'
```

One fixture/run per mode, seed0. Setup, mutation and counters execute outside ASSERT. Both matching modes print the same unconditional row: one source datagram, two conversions, two byte-equality checks, preserved identity, identification0, offsets0/1232, one UDP delivery and one application delivery with1452payload bytes. Debug verifies exact bytes and identity; inactive release assertions do not supply debug semantic proof. The earlier computed-ICMP protocol control also passes in both modes with equal observations. Exact parent/candidate trees, fixture/library hashes, commands and raw output are retained in the C03 publication execution record; the final series index associates the tested tree with its commit. No memory-sanitizer proof is claimed.
