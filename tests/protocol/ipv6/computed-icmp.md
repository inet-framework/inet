# Computed ICMPv6 on an IPv6 route

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [publication plan C02](../../../plan/pending/6lowpan-publication-plan.md#c02)

Publication commit C02: `icmpv6: fix: compute checksums with final IPv6 addresses and bound error size`. Test: [Ipv6ComputedIcmp.test](Ipv6ComputedIcmp.test).

Execution: **C02 debug/release PASS; identical-fixture parent runtime FAIL**. Both matching builds pass. The assigned checksum unit and four generic module controls pass in debug. This fixture extends the topology and ordinary production paths of `Rfc4443PacketTooBig.test`; it adds no optional LoWPAN dependency. The existing checksum units/modules own complementary primitive, extension-header and local-indication checks.

```mermaid
flowchart LR
 A["hostA: PingApp and UDP source"] <-->|"Ethernet MTU1500"| R["IPv6 router"]
 R <-->|"Ethernet MTU1280"| B["hostB: Echo responder and UDP sink"]
```

Diagram rendering is unverified. IPv6FlatNetworkConfigurator, StandardHost6, Router6 and Eth100M are the existing suite components. Seed 0 is fixed. Applications begin after address discovery/DAD: one 56-byte Echo payload at 5 s, followed by one 1452-byte application UDP payload at 5.5 s. The UDP packet is 1500 IPv6 bytes and cannot cross the router's 1280-byte egress. The run ends at 10 s; no injected reply, checksum helper substitution or overridden production module is used.

The checker observes actual Ethernet MAC output/input rather than ICMP construction alone. It finds the IPv6 region beneath typed link chunks and independently sums the received IPv6 source/destination addresses, 32-bit upper-layer length, Next Header 58 and ICMP bytes using one's-complement arithmetic. It does not call the production ICMP checksum routine. The received Echo request and reply must each validate; identifier/sequence/body are preserved, final source/destination pairs reverse, and the real PingApp receives one pong.

For Packet Too Big, hostA's actual MAC input must contain one direct Next Header 58 packet, complete length 1280, type2/code0, advertised MTU1280, a valid final-address checksum and a 1232-byte quote. The quote matches the actual 1500-byte original observed on link1, including its known router Hop Limit decrement 64→63. No UDP original or Fragment packet may appear on router egress link2. Thus the fixture checks both the checksum and the outer-IPv6-inclusive error budget, with full bounded quote bytes rather than only a typed MTU field.

The exact parent, C01 at `8b511b44be864d6eca4e1ea4c5cfb725adcb3ea9`, builds and executes the unchanged fixture. It fails at the destination-MAC listener's independent Echo checksum/code/length assertion, event135 at5.00003668s. The logged Echo fields reconstruct a checksum that validates over the ICMP body alone and fails with the final-address IPv6 pseudoheader. This is a runtime failure, not a missing API or test compilation error. Execution stops before the later UDP/PTB stimulus, so no executed parent PTB-size failure is claimed. Source inspection separately establishes the parent's error budget omits the outer IPv6 header.

The C02 source finalizes the checksum with the egress addresses and caps the complete IPv6 error at1280. Existing module vectors separately cover supported extension-header offsets and the loopback/local-indication bypass. This one seed checks the concrete Echo/PTB route; it is not all ICMPv6 errors, IPv6 extension processing or broad RFC conformance.

Reproduction by the sole runner, after a fresh matching debug build:

```sh
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent \
  -w '^tests/protocol/ipv6$' -f 'Ipv6ComputedIcmp\.test$'
```

One fixture/run per mode, seed0. All setup and measurements execute outside ASSERT. Both matching modes print the same unconditional row: one original, one Echo request/reply/pong, one PTB, three checksum checks, complete error1280, quote1232, MTU1280 and zero forbidden forwarding. Debug asserts the fields and exact bytes; inactive release assertions do not supply debug semantic proof. The equal release row establishes these observed counts and sizes. Exact parent, candidate-tree, fixture and library hashes, commands and raw output are retained in the C02 publication execution record; the final series index associates that tested tree with its commit.
