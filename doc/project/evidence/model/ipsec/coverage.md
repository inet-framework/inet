# IPsec — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc4301/catalog.md](../../standard/rfc4301/catalog.md), [rfc4302/catalog.md](../../standard/rfc4302/catalog.md), [rfc4303/catalog.md](../../standard/rfc4303/catalog.md), [features.md](../../protocol/ipsec/features.md)

The single place that holds the changing state of the IPsec workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-24 19:35 +0200
- INET: branch `topic/standards-tests-ipsec-level2`, commit `829ba07bae`, tree clean
- Trees: src `5c4f41c600`, tests/protocol `3ffda5ff0f`
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0 (++20260325083105+68994554ea12-1~exp1~20260325203127.404)
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/ipsec$'`
- Suite: 31 tests, 15 PASS, 10 FAIL (unexpected), 6 FAIL (expected), so the suite reports FAIL
- Target level: 2

The analysis of every failure is in [`results.md`](results.md#the-model-gaps).

## Statement coverage

`Status` is the state of the workflow, not of the standard:

| Status | Means |
| --- | --- |
| `selected` | a check targets it |
| `covered` | a selected check establishes it as a side effect |
| `owed` | the model claims the behavior and **no check exists yet**; the note names what a check would need |
| `later` | it needs a toolset of a higher level — a crafted or replayed packet at level 3, the state of a node or a counter near 2^32 at level 4, a key management protocol or the algorithm itself at level 5 |
| `no check` | the model does not claim the behavior, or a permission that no observation can fail, so [the third principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test) asks for no test |

A verdict belongs to the test. Where a test fails at a later observation and the observation
of a statement held, the row says PASS and names the observation in the note; where a failure
kept a test from the observation a statement needs, the row says FAIL and says so. A PASS
"with no weight" is a pass that the model cannot fail, because the receiver never checks the
ICV; level 3 gives it weight.

| Status | RFC 4301 | RFC 4302 | RFC 4303 | Together |
| --- | --- | --- | --- | --- |
| `selected` | 69 | 40 | 61 | 170 |
| `covered` | 72 | 41 | 42 | 155 |
| `owed` | 4 | 1 | 1 | 6 |
| `later` | 105 | 59 | 96 | 260 |
| `no check` | 120 | 49 | 37 | 206 |
| all | 370 | 190 | 237 | 797 |

### RFC 4301

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC4301-OVW-1](../../standard/rfc4301/catalog.md#rfc4301-ovw-1) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OVW-2](../../standard/rfc4301/catalog.md#rfc4301-ovw-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-OVW-3](../../standard/rfc4301/catalog.md#rfc4301-ovw-3) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-OVW-4](../../standard/rfc4301/catalog.md#rfc4301-ovw-4) | no check | — | — | — | not claimed: one SPD for each host, no SPD selection function |
| [RFC4301-OVW-5](../../standard/rfc4301/catalog.md#rfc4301-ovw-5) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-OVW-6](../../standard/rfc4301/catalog.md#rfc4301-ovw-6) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4301-OVW-7](../../standard/rfc4301/catalog.md#rfc4301-ovw-7) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4301-OVW-8](../../standard/rfc4301/catalog.md#rfc4301-ovw-8) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4301-OVW-9](../../standard/rfc4301/catalog.md#rfc4301-ovw-9) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4301-OVW-10](../../standard/rfc4301/catalog.md#rfc4301-ovw-10) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-OVW-11](../../standard/rfc4301/catalog.md#rfc4301-ovw-11) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-OVW-12](../../standard/rfc4301/catalog.md#rfc4301-ovw-12) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SA-1](../../standard/rfc4301/catalog.md#rfc4301-sa-1) | selected | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | `Rfc4301SaPerDirection` | PASS |  |
| [RFC4301-SA-2](../../standard/rfc4301/catalog.md#rfc4301-sa-2) | selected | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | `Rfc4301SaPerDirection` | PASS |  |
| [RFC4301-SA-3](../../standard/rfc4301/catalog.md#rfc4301-sa-3) | covered | [AH and ESP on one packet](../../protocol/ipsec/checks/modes.md#ah-and-esp-on-one-packet) | `Rfc4301AhAndEsp` | PASS | each SA has one protection; the model does not have to support nested SAs |
| [RFC4301-SA-4](../../standard/rfc4301/catalog.md#rfc4301-sa-4) | selected | [AH and ESP on one packet](../../protocol/ipsec/checks/modes.md#ah-and-esp-on-one-packet) | `Rfc4301AhAndEsp` | FAIL | observation 1: the configuration cannot give one flow both protections, [gap 7](results.md#gap-7-untestable-claim--ah-and-esp-cannot-protect-one-packet) |
| [RFC4301-SA-5](../../standard/rfc4301/catalog.md#rfc4301-sa-5) | selected | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | `Rfc4301SaPerDirection` | PASS |  |
| [RFC4301-SA-6](../../standard/rfc4301/catalog.md#rfc4301-sa-6) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-7](../../standard/rfc4301/catalog.md#rfc4301-sa-7) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-8](../../standard/rfc4301/catalog.md#rfc4301-sa-8) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SA-9](../../standard/rfc4301/catalog.md#rfc4301-sa-9) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SA-10](../../standard/rfc4301/catalog.md#rfc4301-sa-10) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SA-11](../../standard/rfc4301/catalog.md#rfc4301-sa-11) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SA-12](../../standard/rfc4301/catalog.md#rfc4301-sa-12) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SA-13](../../standard/rfc4301/catalog.md#rfc4301-sa-13) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-14](../../standard/rfc4301/catalog.md#rfc4301-sa-14) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-15](../../standard/rfc4301/catalog.md#rfc4301-sa-15) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-16](../../standard/rfc4301/catalog.md#rfc4301-sa-16) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-17](../../standard/rfc4301/catalog.md#rfc4301-sa-17) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-18](../../standard/rfc4301/catalog.md#rfc4301-sa-18) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-19](../../standard/rfc4301/catalog.md#rfc4301-sa-19) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SA-20](../../standard/rfc4301/catalog.md#rfc4301-sa-20) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SA-21](../../standard/rfc4301/catalog.md#rfc4301-sa-21) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SA-22](../../standard/rfc4301/catalog.md#rfc4301-sa-22) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SA-23](../../standard/rfc4301/catalog.md#rfc4301-sa-23) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SA-24](../../standard/rfc4301/catalog.md#rfc4301-sa-24) | covered | [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | `Rfc4301ParallelSas` | FAIL (expected) | observation 2: the packets with DSCP 46 carry SPI 101, [gap 10](results.md#gap-10-unimplemented-feature--no-choice-of-an-sa-by-dscp) |
| [RFC4301-SA-25](../../standard/rfc4301/catalog.md#rfc4301-sa-25) | selected | [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | `Rfc4301ParallelSas` | FAIL (expected) | observation 2: the packets with DSCP 46 carry SPI 101, [gap 10](results.md#gap-10-unimplemented-feature--no-choice-of-an-sa-by-dscp) |
| [RFC4301-SA-26](../../standard/rfc4301/catalog.md#rfc4301-sa-26) | selected | [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | `Rfc4301ParallelSas` | FAIL (expected) | not reached: the test stopped at observation 2, [gap 10](results.md#gap-10-unimplemented-feature--no-choice-of-an-sa-by-dscp) |
| [RFC4301-SA-27](../../standard/rfc4301/catalog.md#rfc4301-sa-27) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-28](../../standard/rfc4301/catalog.md#rfc4301-sa-28) | covered | [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | `Rfc4301ParallelSas` | FAIL (expected) | not reached: the test stopped at observation 2, [gap 10](results.md#gap-10-unimplemented-feature--no-choice-of-an-sa-by-dscp) |
| [RFC4301-SA-29](../../standard/rfc4301/catalog.md#rfc4301-sa-29) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-SA-30](../../standard/rfc4301/catalog.md#rfc4301-sa-30) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-SA-31](../../standard/rfc4301/catalog.md#rfc4301-sa-31) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-32](../../standard/rfc4301/catalog.md#rfc4301-sa-32) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-33](../../standard/rfc4301/catalog.md#rfc4301-sa-33) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-34](../../standard/rfc4301/catalog.md#rfc4301-sa-34) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-SA-35](../../standard/rfc4301/catalog.md#rfc4301-sa-35) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-SA-36](../../standard/rfc4301/catalog.md#rfc4301-sa-36) | owed | — | — | — | a later level 2 pass: a packet with a Hop-by-Hop or Destination Options header, to see where AH or ESP goes and which headers the selectors skip |
| [RFC4301-SA-37](../../standard/rfc4301/catalog.md#rfc4301-sa-37) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-SA-38](../../standard/rfc4301/catalog.md#rfc4301-sa-38) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-SA-39](../../standard/rfc4301/catalog.md#rfc4301-sa-39) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | with no weight: the model computes no ICV |
| [RFC4301-SA-40](../../standard/rfc4301/catalog.md#rfc4301-sa-40) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-41](../../standard/rfc4301/catalog.md#rfc4301-sa-41) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-SA-42](../../standard/rfc4301/catalog.md#rfc4301-sa-42) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-43](../../standard/rfc4301/catalog.md#rfc4301-sa-43) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-44](../../standard/rfc4301/catalog.md#rfc4301-sa-44) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4301-SA-45](../../standard/rfc4301/catalog.md#rfc4301-sa-45) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4301-SA-46](../../standard/rfc4301/catalog.md#rfc4301-sa-46) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4301-SA-47](../../standard/rfc4301/catalog.md#rfc4301-sa-47) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-SA-48](../../standard/rfc4301/catalog.md#rfc4301-sa-48) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-49](../../standard/rfc4301/catalog.md#rfc4301-sa-49) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SA-50](../../standard/rfc4301/catalog.md#rfc4301-sa-50) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-SA-51](../../standard/rfc4301/catalog.md#rfc4301-sa-51) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-FUNC-1](../../standard/rfc4301/catalog.md#rfc4301-func-1) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-FUNC-2](../../standard/rfc4301/catalog.md#rfc4301-func-2) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-FUNC-3](../../standard/rfc4301/catalog.md#rfc4301-func-3) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4301-FUNC-4](../../standard/rfc4301/catalog.md#rfc4301-func-4) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-FUNC-5](../../standard/rfc4301/catalog.md#rfc4301-func-5) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-FUNC-6](../../standard/rfc4301/catalog.md#rfc4301-func-6) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-FUNC-7](../../standard/rfc4301/catalog.md#rfc4301-func-7) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-FUNC-8](../../standard/rfc4301/catalog.md#rfc4301-func-8) | covered | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS |  |
| [RFC4301-FUNC-9](../../standard/rfc4301/catalog.md#rfc4301-func-9) | selected | [An ESP SA without encryption and without integrity](../../protocol/ipsec/checks/esp.md#an-esp-sa-without-encryption-and-without-integrity) | `Rfc4303EspBothNull` | PASS |  |
| [RFC4301-FUNC-10](../../standard/rfc4301/catalog.md#rfc4301-func-10) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-FUNC-11](../../standard/rfc4301/catalog.md#rfc4301-func-11) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-FUNC-12](../../standard/rfc4301/catalog.md#rfc4301-func-12) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-COMB-1](../../standard/rfc4301/catalog.md#rfc4301-comb-1) | covered | [AH and ESP on one packet](../../protocol/ipsec/checks/modes.md#ah-and-esp-on-one-packet) | `Rfc4301AhAndEsp` | PASS | each SA has one protection; the model does not have to support nested SAs |
| [RFC4301-COMB-2](../../standard/rfc4301/catalog.md#rfc4301-comb-2) | no check | — | — | — | not claimed: no nested SAs |
| [RFC4301-DB-1](../../standard/rfc4301/catalog.md#rfc4301-db-1) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-DB-2](../../standard/rfc4301/catalog.md#rfc4301-db-2) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-DB-3](../../standard/rfc4301/catalog.md#rfc4301-db-3) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-DB-4](../../standard/rfc4301/catalog.md#rfc4301-db-4) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-DB-5](../../standard/rfc4301/catalog.md#rfc4301-db-5) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-DB-6](../../standard/rfc4301/catalog.md#rfc4301-db-6) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-DB-7](../../standard/rfc4301/catalog.md#rfc4301-db-7) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-DB-8](../../standard/rfc4301/catalog.md#rfc4301-db-8) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-DB-9](../../standard/rfc4301/catalog.md#rfc4301-db-9) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-DB-10](../../standard/rfc4301/catalog.md#rfc4301-db-10) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-DB-11](../../standard/rfc4301/catalog.md#rfc4301-db-11) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPD-1](../../standard/rfc4301/catalog.md#rfc4301-spd-1) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SPD-2](../../standard/rfc4301/catalog.md#rfc4301-spd-2) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-3](../../standard/rfc4301/catalog.md#rfc4301-spd-3) | no check | — | — | — | not claimed: one SPD for each host, no SPD selection function |
| [RFC4301-SPD-4](../../standard/rfc4301/catalog.md#rfc4301-spd-4) | no check | — | — | — | not claimed: one SPD for each host, no SPD selection function |
| [RFC4301-SPD-5](../../standard/rfc4301/catalog.md#rfc4301-spd-5) | no check | — | — | — | not claimed: one SPD for each host, no SPD selection function |
| [RFC4301-SPD-6](../../standard/rfc4301/catalog.md#rfc4301-spd-6) | selected | [The first matching entry decides](../../protocol/ipsec/checks/policy.md#the-first-matching-entry-decides) | `Rfc4301FirstMatchDecides` | PASS |  |
| [RFC4301-SPD-7](../../standard/rfc4301/catalog.md#rfc4301-spd-7) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-8](../../standard/rfc4301/catalog.md#rfc4301-spd-8) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-9](../../standard/rfc4301/catalog.md#rfc4301-spd-9) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-10](../../standard/rfc4301/catalog.md#rfc4301-spd-10) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-11](../../standard/rfc4301/catalog.md#rfc4301-spd-11) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-12](../../standard/rfc4301/catalog.md#rfc4301-spd-12) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-13](../../standard/rfc4301/catalog.md#rfc4301-spd-13) | selected | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-SPD-14](../../standard/rfc4301/catalog.md#rfc4301-spd-14) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-15](../../standard/rfc4301/catalog.md#rfc4301-spd-15) | covered | [The first matching entry decides](../../protocol/ipsec/checks/policy.md#the-first-matching-entry-decides) | `Rfc4301FirstMatchDecides` | PASS |  |
| [RFC4301-SPD-16](../../standard/rfc4301/catalog.md#rfc4301-spd-16) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-17](../../standard/rfc4301/catalog.md#rfc4301-spd-17) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPD-18](../../standard/rfc4301/catalog.md#rfc4301-spd-18) | selected | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-SPD-19](../../standard/rfc4301/catalog.md#rfc4301-spd-19) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-20](../../standard/rfc4301/catalog.md#rfc4301-spd-20) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-21](../../standard/rfc4301/catalog.md#rfc4301-spd-21) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPD-22](../../standard/rfc4301/catalog.md#rfc4301-spd-22) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPD-23](../../standard/rfc4301/catalog.md#rfc4301-spd-23) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPD-24](../../standard/rfc4301/catalog.md#rfc4301-spd-24) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-SPD-25](../../standard/rfc4301/catalog.md#rfc4301-spd-25) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-SPD-26](../../standard/rfc4301/catalog.md#rfc4301-spd-26) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPD-27](../../standard/rfc4301/catalog.md#rfc4301-spd-27) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPD-28](../../standard/rfc4301/catalog.md#rfc4301-spd-28) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPD-29](../../standard/rfc4301/catalog.md#rfc4301-spd-29) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPD-30](../../standard/rfc4301/catalog.md#rfc4301-spd-30) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPD-31](../../standard/rfc4301/catalog.md#rfc4301-spd-31) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPD-32](../../standard/rfc4301/catalog.md#rfc4301-spd-32) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPD-33](../../standard/rfc4301/catalog.md#rfc4301-spd-33) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-SPD-34](../../standard/rfc4301/catalog.md#rfc4301-spd-34) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPD-35](../../standard/rfc4301/catalog.md#rfc4301-spd-35) | selected | [The first matching entry decides](../../protocol/ipsec/checks/policy.md#the-first-matching-entry-decides) | `Rfc4301FirstMatchDecides` | PASS |  |
| [RFC4301-SPD-36](../../standard/rfc4301/catalog.md#rfc4301-spd-36) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-SPD-37](../../standard/rfc4301/catalog.md#rfc4301-spd-37) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-SPD-38](../../standard/rfc4301/catalog.md#rfc4301-spd-38) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPD-39](../../standard/rfc4301/catalog.md#rfc4301-spd-39) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SPD-40](../../standard/rfc4301/catalog.md#rfc4301-spd-40) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPD-41](../../standard/rfc4301/catalog.md#rfc4301-spd-41) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPD-42](../../standard/rfc4301/catalog.md#rfc4301-spd-42) | selected | [The first matching entry decides](../../protocol/ipsec/checks/policy.md#the-first-matching-entry-decides) | `Rfc4301FirstMatchDecides` | PASS |  |
| [RFC4301-SPD-43](../../standard/rfc4301/catalog.md#rfc4301-spd-43) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SPD-44](../../standard/rfc4301/catalog.md#rfc4301-spd-44) | no check | — | — | — | not claimed: the SPD does not change while the model runs |
| [RFC4301-SPD-45](../../standard/rfc4301/catalog.md#rfc4301-spd-45) | no check | — | — | — | not claimed: the SPD does not change while the model runs |
| [RFC4301-SEL-1](../../standard/rfc4301/catalog.md#rfc4301-sel-1) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-2](../../standard/rfc4301/catalog.md#rfc4301-sel-2) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-3](../../standard/rfc4301/catalog.md#rfc4301-sel-3) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SEL-4](../../standard/rfc4301/catalog.md#rfc4301-sel-4) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-5](../../standard/rfc4301/catalog.md#rfc4301-sel-5) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-6](../../standard/rfc4301/catalog.md#rfc4301-sel-6) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SEL-7](../../standard/rfc4301/catalog.md#rfc4301-sel-7) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-8](../../standard/rfc4301/catalog.md#rfc4301-sel-8) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-9](../../standard/rfc4301/catalog.md#rfc4301-sel-9) | owed | — | — | — | a later level 2 pass: a packet with a Hop-by-Hop or Destination Options header, to see where AH or ESP goes and which headers the selectors skip |
| [RFC4301-SEL-10](../../standard/rfc4301/catalog.md#rfc4301-sel-10) | owed | — | — | — | a later level 2 pass: a packet with a Hop-by-Hop or Destination Options header, to see where AH or ESP goes and which headers the selectors skip |
| [RFC4301-SEL-11](../../standard/rfc4301/catalog.md#rfc4301-sel-11) | owed | — | — | — | a later level 2 pass: a packet with a Hop-by-Hop or Destination Options header, to see where AH or ESP goes and which headers the selectors skip |
| [RFC4301-SEL-12](../../standard/rfc4301/catalog.md#rfc4301-sel-12) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-13](../../standard/rfc4301/catalog.md#rfc4301-sel-13) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SEL-14](../../standard/rfc4301/catalog.md#rfc4301-sel-14) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SEL-15](../../standard/rfc4301/catalog.md#rfc4301-sel-15) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SEL-16](../../standard/rfc4301/catalog.md#rfc4301-sel-16) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4301-SEL-17](../../standard/rfc4301/catalog.md#rfc4301-sel-17) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SEL-18](../../standard/rfc4301/catalog.md#rfc4301-sel-18) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-19](../../standard/rfc4301/catalog.md#rfc4301-sel-19) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-20](../../standard/rfc4301/catalog.md#rfc4301-sel-20) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SEL-21](../../standard/rfc4301/catalog.md#rfc4301-sel-21) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SEL-22](../../standard/rfc4301/catalog.md#rfc4301-sel-22) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SEL-23](../../standard/rfc4301/catalog.md#rfc4301-sel-23) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SEL-24](../../standard/rfc4301/catalog.md#rfc4301-sel-24) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SEL-25](../../standard/rfc4301/catalog.md#rfc4301-sel-25) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SEL-26](../../standard/rfc4301/catalog.md#rfc4301-sel-26) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SEL-27](../../standard/rfc4301/catalog.md#rfc4301-sel-27) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SEL-28](../../standard/rfc4301/catalog.md#rfc4301-sel-28) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SEL-29](../../standard/rfc4301/catalog.md#rfc4301-sel-29) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-1](../../standard/rfc4301/catalog.md#rfc4301-spde-1) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-2](../../standard/rfc4301/catalog.md#rfc4301-spde-2) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-3](../../standard/rfc4301/catalog.md#rfc4301-spde-3) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPDE-4](../../standard/rfc4301/catalog.md#rfc4301-spde-4) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPDE-5](../../standard/rfc4301/catalog.md#rfc4301-spde-5) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-6](../../standard/rfc4301/catalog.md#rfc4301-spde-6) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-7](../../standard/rfc4301/catalog.md#rfc4301-spde-7) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-8](../../standard/rfc4301/catalog.md#rfc4301-spde-8) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-9](../../standard/rfc4301/catalog.md#rfc4301-spde-9) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-10](../../standard/rfc4301/catalog.md#rfc4301-spde-10) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SPDE-11](../../standard/rfc4301/catalog.md#rfc4301-spde-11) | selected | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPDE-12](../../standard/rfc4301/catalog.md#rfc4301-spde-12) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-SPDE-13](../../standard/rfc4301/catalog.md#rfc4301-spde-13) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPDE-14](../../standard/rfc4301/catalog.md#rfc4301-spde-14) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPDE-15](../../standard/rfc4301/catalog.md#rfc4301-spde-15) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPDE-16](../../standard/rfc4301/catalog.md#rfc4301-spde-16) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPDE-17](../../standard/rfc4301/catalog.md#rfc4301-spde-17) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4301-SPDE-18](../../standard/rfc4301/catalog.md#rfc4301-spde-18) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPDE-19](../../standard/rfc4301/catalog.md#rfc4301-spde-19) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPDE-20](../../standard/rfc4301/catalog.md#rfc4301-spde-20) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SPDE-21](../../standard/rfc4301/catalog.md#rfc4301-spde-21) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-SPDE-22](../../standard/rfc4301/catalog.md#rfc4301-spde-22) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-NLP-1](../../standard/rfc4301/catalog.md#rfc4301-nlp-1) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-NLP-2](../../standard/rfc4301/catalog.md#rfc4301-nlp-2) | covered | [Selectors](../../protocol/ipsec/checks/policy.md#selectors) | `Rfc4301Selectors` | PASS |  |
| [RFC4301-NLP-3](../../standard/rfc4301/catalog.md#rfc4301-nlp-3) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-NLP-4](../../standard/rfc4301/catalog.md#rfc4301-nlp-4) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-NLP-5](../../standard/rfc4301/catalog.md#rfc4301-nlp-5) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-NLP-6](../../standard/rfc4301/catalog.md#rfc4301-nlp-6) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4301-SAD-1](../../standard/rfc4301/catalog.md#rfc4301-sad-1) | covered | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | `Rfc4301SaPerDirection` | PASS |  |
| [RFC4301-SAD-2](../../standard/rfc4301/catalog.md#rfc4301-sad-2) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SAD-3](../../standard/rfc4301/catalog.md#rfc4301-sad-3) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-SAD-4](../../standard/rfc4301/catalog.md#rfc4301-sad-4) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SAD-5](../../standard/rfc4301/catalog.md#rfc4301-sad-5) | covered | [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | `Rfc4301InboundSelectorCheck` | FAIL | the SAD entry holds the selectors, and nothing reads them, [gap 5](results.md#gap-5-defect--no-selector-check-after-ah-or-esp-processing) |
| [RFC4301-SAD-6](../../standard/rfc4301/catalog.md#rfc4301-sad-6) | selected | [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | `Rfc4301InboundSelectorCheck` | FAIL | observation 3: B delivers flow 3000, which SA 101 does not cover, [gap 5](results.md#gap-5-defect--no-selector-check-after-ah-or-esp-processing) |
| [RFC4301-SAD-7](../../standard/rfc4301/catalog.md#rfc4301-sad-7) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SAD-8](../../standard/rfc4301/catalog.md#rfc4301-sad-8) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SAD-9](../../standard/rfc4301/catalog.md#rfc4301-sad-9) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SAD-10](../../standard/rfc4301/catalog.md#rfc4301-sad-10) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SAD-11](../../standard/rfc4301/catalog.md#rfc4301-sad-11) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SAD-12](../../standard/rfc4301/catalog.md#rfc4301-sad-12) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SAD-13](../../standard/rfc4301/catalog.md#rfc4301-sad-13) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-SADI-1](../../standard/rfc4301/catalog.md#rfc4301-sadi-1) | covered | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | `Rfc4301SaPerDirection` | PASS |  |
| [RFC4301-SADI-2](../../standard/rfc4301/catalog.md#rfc4301-sadi-2) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers` | PASS | observation 3: each SA counts on its own |
| [RFC4301-SADI-3](../../standard/rfc4301/catalog.md#rfc4301-sadi-3) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4301-SADI-4](../../standard/rfc4301/catalog.md#rfc4301-sadi-4) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SADI-5](../../standard/rfc4301/catalog.md#rfc4301-sadi-5) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4301-SADI-6](../../standard/rfc4301/catalog.md#rfc4301-sadi-6) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-SADI-7](../../standard/rfc4301/catalog.md#rfc4301-sadi-7) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-SADI-8](../../standard/rfc4301/catalog.md#rfc4301-sadi-8) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-SADI-9](../../standard/rfc4301/catalog.md#rfc4301-sadi-9) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4301-SADI-10](../../standard/rfc4301/catalog.md#rfc4301-sadi-10) | selected | [The lifetime of an SA](../../protocol/ipsec/checks/security-associations.md#the-lifetime-of-an-sa) | `Rfc4301SaLifetime` | FAIL (expected) | observations 2 and 3: the SAs have no lifetime, [gap 9](results.md#gap-9-unimplemented-feature--no-sa-lifetime) |
| [RFC4301-SADI-11](../../standard/rfc4301/catalog.md#rfc4301-sadi-11) | selected | [The lifetime of an SA](../../protocol/ipsec/checks/security-associations.md#the-lifetime-of-an-sa) | `Rfc4301SaLifetime` | FAIL (expected) | observations 2 and 3: the SAs have no lifetime, [gap 9](results.md#gap-9-unimplemented-feature--no-sa-lifetime) |
| [RFC4301-SADI-12](../../standard/rfc4301/catalog.md#rfc4301-sadi-12) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-SADI-13](../../standard/rfc4301/catalog.md#rfc4301-sadi-13) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-SADI-14](../../standard/rfc4301/catalog.md#rfc4301-sadi-14) | covered | [The lifetime of an SA](../../protocol/ipsec/checks/security-associations.md#the-lifetime-of-an-sa) | `Rfc4301SaLifetime` | FAIL (expected) | observations 2 and 3: the SAs have no lifetime, [gap 9](results.md#gap-9-unimplemented-feature--no-sa-lifetime) |
| [RFC4301-SADI-15](../../standard/rfc4301/catalog.md#rfc4301-sadi-15) | covered | [The lifetime of an SA](../../protocol/ipsec/checks/security-associations.md#the-lifetime-of-an-sa) | `Rfc4301SaLifetime` | FAIL (expected) | observations 2 and 3: the SAs have no lifetime, [gap 9](results.md#gap-9-unimplemented-feature--no-sa-lifetime) |
| [RFC4301-SADI-16](../../standard/rfc4301/catalog.md#rfc4301-sadi-16) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SADI-17](../../standard/rfc4301/catalog.md#rfc4301-sadi-17) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SADI-18](../../standard/rfc4301/catalog.md#rfc4301-sadi-18) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SADI-19](../../standard/rfc4301/catalog.md#rfc4301-sadi-19) | covered | [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | `Rfc4301ParallelSas` | FAIL (expected) | observation 2: the packets with DSCP 46 carry SPI 101, [gap 10](results.md#gap-10-unimplemented-feature--no-choice-of-an-sa-by-dscp) |
| [RFC4301-SADI-20](../../standard/rfc4301/catalog.md#rfc4301-sadi-20) | covered | [Parallel SAs for classes of traffic](../../protocol/ipsec/checks/security-associations.md#parallel-sas-for-classes-of-traffic) | `Rfc4301ParallelSas` | FAIL (expected) | not reached: the test stopped at observation 2, [gap 10](results.md#gap-10-unimplemented-feature--no-choice-of-an-sa-by-dscp) |
| [RFC4301-SADI-21](../../standard/rfc4301/catalog.md#rfc4301-sadi-21) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-SADI-22](../../standard/rfc4301/catalog.md#rfc4301-sadi-22) | covered | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | `Rfc4301PathMtuIpv6` | FAIL (expected) | observation 2: A sends packets of 1410 octets after the Packet Too Big message, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu) |
| [RFC4301-SADI-23](../../standard/rfc4301/catalog.md#rfc4301-sadi-23) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-PFP-1](../../standard/rfc4301/catalog.md#rfc4301-pfp-1) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-2](../../standard/rfc4301/catalog.md#rfc4301-pfp-2) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-3](../../standard/rfc4301/catalog.md#rfc4301-pfp-3) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-4](../../standard/rfc4301/catalog.md#rfc4301-pfp-4) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-5](../../standard/rfc4301/catalog.md#rfc4301-pfp-5) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-6](../../standard/rfc4301/catalog.md#rfc4301-pfp-6) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-7](../../standard/rfc4301/catalog.md#rfc4301-pfp-7) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-8](../../standard/rfc4301/catalog.md#rfc4301-pfp-8) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-9](../../standard/rfc4301/catalog.md#rfc4301-pfp-9) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-10](../../standard/rfc4301/catalog.md#rfc4301-pfp-10) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-11](../../standard/rfc4301/catalog.md#rfc4301-pfp-11) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-12](../../standard/rfc4301/catalog.md#rfc4301-pfp-12) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-13](../../standard/rfc4301/catalog.md#rfc4301-pfp-13) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-14](../../standard/rfc4301/catalog.md#rfc4301-pfp-14) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-15](../../standard/rfc4301/catalog.md#rfc4301-pfp-15) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-16](../../standard/rfc4301/catalog.md#rfc4301-pfp-16) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-17](../../standard/rfc4301/catalog.md#rfc4301-pfp-17) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-18](../../standard/rfc4301/catalog.md#rfc4301-pfp-18) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-19](../../standard/rfc4301/catalog.md#rfc4301-pfp-19) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-20](../../standard/rfc4301/catalog.md#rfc4301-pfp-20) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-21](../../standard/rfc4301/catalog.md#rfc4301-pfp-21) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-22](../../standard/rfc4301/catalog.md#rfc4301-pfp-22) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-23](../../standard/rfc4301/catalog.md#rfc4301-pfp-23) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-24](../../standard/rfc4301/catalog.md#rfc4301-pfp-24) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-25](../../standard/rfc4301/catalog.md#rfc4301-pfp-25) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-26](../../standard/rfc4301/catalog.md#rfc4301-pfp-26) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-27](../../standard/rfc4301/catalog.md#rfc4301-pfp-27) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-28](../../standard/rfc4301/catalog.md#rfc4301-pfp-28) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-29](../../standard/rfc4301/catalog.md#rfc4301-pfp-29) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-30](../../standard/rfc4301/catalog.md#rfc4301-pfp-30) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-31](../../standard/rfc4301/catalog.md#rfc4301-pfp-31) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-32](../../standard/rfc4301/catalog.md#rfc4301-pfp-32) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PFP-33](../../standard/rfc4301/catalog.md#rfc4301-pfp-33) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-PROC-1](../../standard/rfc4301/catalog.md#rfc4301-proc-1) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-PROC-2](../../standard/rfc4301/catalog.md#rfc4301-proc-2) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-PROC-3](../../standard/rfc4301/catalog.md#rfc4301-proc-3) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OUT-1](../../standard/rfc4301/catalog.md#rfc4301-out-1) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OUT-2](../../standard/rfc4301/catalog.md#rfc4301-out-2) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OUT-3](../../standard/rfc4301/catalog.md#rfc4301-out-3) | selected | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OUT-4](../../standard/rfc4301/catalog.md#rfc4301-out-4) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-OUT-5](../../standard/rfc4301/catalog.md#rfc4301-out-5) | covered | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | `Rfc4301PathMtuIpv6` | FAIL (expected) | observation 2: A sends packets of 1410 octets after the Packet Too Big message, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu) |
| [RFC4301-OUT-6](../../standard/rfc4301/catalog.md#rfc4301-out-6) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OUT-7](../../standard/rfc4301/catalog.md#rfc4301-out-7) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OUT-8](../../standard/rfc4301/catalog.md#rfc4301-out-8) | covered | [A PROTECT entry without an SA](../../protocol/ipsec/checks/security-associations.md#a-protect-entry-without-an-sa) | `Rfc4301ProtectWithoutSa` | FAIL | no key management, and no discard: the packet leaves in clear, [gap 6](results.md#gap-6-defect--a-protect-entry-without-an-sa-sends-the-packet-in-clear) |
| [RFC4301-OUT-9](../../standard/rfc4301/catalog.md#rfc4301-out-9) | covered | [A PROTECT entry without an SA](../../protocol/ipsec/checks/security-associations.md#a-protect-entry-without-an-sa) | `Rfc4301ProtectWithoutSa` | FAIL | no key management, and no discard: the packet leaves in clear, [gap 6](results.md#gap-6-defect--a-protect-entry-without-an-sa-sends-the-packet-in-clear) |
| [RFC4301-OUT-10](../../standard/rfc4301/catalog.md#rfc4301-out-10) | selected | [A PROTECT entry without an SA](../../protocol/ipsec/checks/security-associations.md#a-protect-entry-without-an-sa) | `Rfc4301ProtectWithoutSa` | FAIL | observation 2: flow 2000 leaves in clear, [gap 6](results.md#gap-6-defect--a-protect-entry-without-an-sa-sends-the-packet-in-clear) |
| [RFC4301-OUT-11](../../standard/rfc4301/catalog.md#rfc4301-out-11) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-OUT-12](../../standard/rfc4301/catalog.md#rfc4301-out-12) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-OUT-13](../../standard/rfc4301/catalog.md#rfc4301-out-13) | covered | [Outbound dispositions](../../protocol/ipsec/checks/policy.md#outbound-dispositions) | `Rfc4301OutboundDispositions` | PASS |  |
| [RFC4301-OUT-14](../../standard/rfc4301/catalog.md#rfc4301-out-14) | no check | — | — | — | not claimed: no nested SAs |
| [RFC4301-OUT-15](../../standard/rfc4301/catalog.md#rfc4301-out-15) | no check | — | — | — | not claimed: no nested SAs |
| [RFC4301-OUT-16](../../standard/rfc4301/catalog.md#rfc4301-out-16) | no check | — | — | — | not claimed: no nested SAs |
| [RFC4301-OUT-17](../../standard/rfc4301/catalog.md#rfc4301-out-17) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-OUT-18](../../standard/rfc4301/catalog.md#rfc4301-out-18) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-OUT-19](../../standard/rfc4301/catalog.md#rfc4301-out-19) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-OUT-20](../../standard/rfc4301/catalog.md#rfc4301-out-20) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-OUT-21](../../standard/rfc4301/catalog.md#rfc4301-out-21) | selected | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | `Rfc4301PathMtuIpv6` | FAIL (expected) | observation 2: A sends packets of 1410 octets after the Packet Too Big message, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu) |
| [RFC4301-DISC-1](../../standard/rfc4301/catalog.md#rfc4301-disc-1) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-2](../../standard/rfc4301/catalog.md#rfc4301-disc-2) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-DISC-3](../../standard/rfc4301/catalog.md#rfc4301-disc-3) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-DISC-4](../../standard/rfc4301/catalog.md#rfc4301-disc-4) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-5](../../standard/rfc4301/catalog.md#rfc4301-disc-5) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-6](../../standard/rfc4301/catalog.md#rfc4301-disc-6) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-7](../../standard/rfc4301/catalog.md#rfc4301-disc-7) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-8](../../standard/rfc4301/catalog.md#rfc4301-disc-8) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-9](../../standard/rfc4301/catalog.md#rfc4301-disc-9) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-10](../../standard/rfc4301/catalog.md#rfc4301-disc-10) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-DISC-11](../../standard/rfc4301/catalog.md#rfc4301-disc-11) | no check | — | — | — | not claimed: no ICMP report of an outbound discard |
| [RFC4301-TUN-1](../../standard/rfc4301/catalog.md#rfc4301-tun-1) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-2](../../standard/rfc4301/catalog.md#rfc4301-tun-2) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-3](../../standard/rfc4301/catalog.md#rfc4301-tun-3) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-4](../../standard/rfc4301/catalog.md#rfc4301-tun-4) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-5](../../standard/rfc4301/catalog.md#rfc4301-tun-5) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-6](../../standard/rfc4301/catalog.md#rfc4301-tun-6) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-7](../../standard/rfc4301/catalog.md#rfc4301-tun-7) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-8](../../standard/rfc4301/catalog.md#rfc4301-tun-8) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-9](../../standard/rfc4301/catalog.md#rfc4301-tun-9) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-10](../../standard/rfc4301/catalog.md#rfc4301-tun-10) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-11](../../standard/rfc4301/catalog.md#rfc4301-tun-11) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-12](../../standard/rfc4301/catalog.md#rfc4301-tun-12) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-13](../../standard/rfc4301/catalog.md#rfc4301-tun-13) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-14](../../standard/rfc4301/catalog.md#rfc4301-tun-14) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-15](../../standard/rfc4301/catalog.md#rfc4301-tun-15) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-16](../../standard/rfc4301/catalog.md#rfc4301-tun-16) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-17](../../standard/rfc4301/catalog.md#rfc4301-tun-17) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-18](../../standard/rfc4301/catalog.md#rfc4301-tun-18) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-19](../../standard/rfc4301/catalog.md#rfc4301-tun-19) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-20](../../standard/rfc4301/catalog.md#rfc4301-tun-20) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-21](../../standard/rfc4301/catalog.md#rfc4301-tun-21) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-22](../../standard/rfc4301/catalog.md#rfc4301-tun-22) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-23](../../standard/rfc4301/catalog.md#rfc4301-tun-23) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-24](../../standard/rfc4301/catalog.md#rfc4301-tun-24) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-25](../../standard/rfc4301/catalog.md#rfc4301-tun-25) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-26](../../standard/rfc4301/catalog.md#rfc4301-tun-26) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4301-TUN-27](../../standard/rfc4301/catalog.md#rfc4301-tun-27) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-28](../../standard/rfc4301/catalog.md#rfc4301-tun-28) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-29](../../standard/rfc4301/catalog.md#rfc4301-tun-29) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-30](../../standard/rfc4301/catalog.md#rfc4301-tun-30) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-31](../../standard/rfc4301/catalog.md#rfc4301-tun-31) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-32](../../standard/rfc4301/catalog.md#rfc4301-tun-32) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-TUN-33](../../standard/rfc4301/catalog.md#rfc4301-tun-33) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4301-IN-1](../../standard/rfc4301/catalog.md#rfc4301-in-1) | covered | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-2](../../standard/rfc4301/catalog.md#rfc4301-in-2) | covered | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-3](../../standard/rfc4301/catalog.md#rfc4301-in-3) | covered | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-4](../../standard/rfc4301/catalog.md#rfc4301-in-4) | selected | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-5](../../standard/rfc4301/catalog.md#rfc4301-in-5) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4301-IN-6](../../standard/rfc4301/catalog.md#rfc4301-in-6) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-IN-7](../../standard/rfc4301/catalog.md#rfc4301-in-7) | no check | — | — | — | not claimed: one SPD for each host, no SPD selection function |
| [RFC4301-IN-8](../../standard/rfc4301/catalog.md#rfc4301-in-8) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-IN-9](../../standard/rfc4301/catalog.md#rfc4301-in-9) | no check | — | — | — | not claimed: one SPD for each host, no SPD selection function |
| [RFC4301-IN-10](../../standard/rfc4301/catalog.md#rfc4301-in-10) | covered | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-11](../../standard/rfc4301/catalog.md#rfc4301-in-11) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-IN-12](../../standard/rfc4301/catalog.md#rfc4301-in-12) | no check | — | — | — | not claimed: one SPD for each host, no SPD selection function |
| [RFC4301-IN-13](../../standard/rfc4301/catalog.md#rfc4301-in-13) | selected | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-14](../../standard/rfc4301/catalog.md#rfc4301-in-14) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-IN-15](../../standard/rfc4301/catalog.md#rfc4301-in-15) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-IN-16](../../standard/rfc4301/catalog.md#rfc4301-in-16) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4301-IN-17](../../standard/rfc4301/catalog.md#rfc4301-in-17) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-IN-18](../../standard/rfc4301/catalog.md#rfc4301-in-18) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-IN-19](../../standard/rfc4301/catalog.md#rfc4301-in-19) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-IN-20](../../standard/rfc4301/catalog.md#rfc4301-in-20) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4301-IN-21](../../standard/rfc4301/catalog.md#rfc4301-in-21) | covered | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-22](../../standard/rfc4301/catalog.md#rfc4301-in-22) | selected | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-23](../../standard/rfc4301/catalog.md#rfc4301-in-23) | covered | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-24](../../standard/rfc4301/catalog.md#rfc4301-in-24) | covered | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-25](../../standard/rfc4301/catalog.md#rfc4301-in-25) | selected | [Inbound dispositions](../../protocol/ipsec/checks/policy.md#inbound-dispositions) | `Rfc4301InboundDispositions` | PASS |  |
| [RFC4301-IN-26](../../standard/rfc4301/catalog.md#rfc4301-in-26) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-IN-27](../../standard/rfc4301/catalog.md#rfc4301-in-27) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-IN-28](../../standard/rfc4301/catalog.md#rfc4301-in-28) | later | — | — | — | level 3: ICMP error messages at the IPsec boundary (RFC 4301 §6) |
| [RFC4301-IN-29](../../standard/rfc4301/catalog.md#rfc4301-in-29) | later | — | — | — | level 3: ICMP error messages at the IPsec boundary (RFC 4301 §6) |
| [RFC4301-IN-30](../../standard/rfc4301/catalog.md#rfc4301-in-30) | later | — | — | — | level 3: ICMP error messages at the IPsec boundary (RFC 4301 §6) |
| [RFC4301-IN-31](../../standard/rfc4301/catalog.md#rfc4301-in-31) | selected | [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | `Rfc4301InboundSelectorCheck` | PASS | observation 2: B delivers flow 2000 |
| [RFC4301-IN-32](../../standard/rfc4301/catalog.md#rfc4301-in-32) | selected | [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | `Rfc4301InboundSelectorCheck` | FAIL | observation 3: B delivers flow 3000, which SA 101 does not cover, [gap 5](results.md#gap-5-defect--no-selector-check-after-ah-or-esp-processing) |
| [RFC4301-IN-33](../../standard/rfc4301/catalog.md#rfc4301-in-33) | selected | [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | `Rfc4301InboundSelectorCheck` | FAIL | observation 3: B delivers flow 3000, which SA 101 does not cover, [gap 5](results.md#gap-5-defect--no-selector-check-after-ah-or-esp-processing) |
| [RFC4301-IN-34](../../standard/rfc4301/catalog.md#rfc4301-in-34) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-IN-35](../../standard/rfc4301/catalog.md#rfc4301-in-35) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4301-IN-36](../../standard/rfc4301/catalog.md#rfc4301-in-36) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-IN-37](../../standard/rfc4301/catalog.md#rfc4301-in-37) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-IN-38](../../standard/rfc4301/catalog.md#rfc4301-in-38) | later | — | — | — | level 5: a key management protocol |
| [RFC4301-IN-39](../../standard/rfc4301/catalog.md#rfc4301-in-39) | covered | [Inbound selector check](../../protocol/ipsec/checks/security-associations.md#inbound-selector-check) | `Rfc4301InboundSelectorCheck` | PASS |  |
| [RFC4301-IN-40](../../standard/rfc4301/catalog.md#rfc4301-in-40) | no check | — | — | — | not claimed: no nested SAs |
| [RFC4301-IN-41](../../standard/rfc4301/catalog.md#rfc4301-in-41) | no check | — | — | — | not claimed: no nested SAs |
| [RFC4301-CONF-1](../../standard/rfc4301/catalog.md#rfc4301-conf-1) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4301-CONF-2](../../standard/rfc4301/catalog.md#rfc4301-conf-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |

### RFC 4302

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC4302-FMT-1](../../standard/rfc4302/catalog.md#rfc4302-fmt-1) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-FMT-2](../../standard/rfc4302/catalog.md#rfc4302-fmt-2) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-FMT-3](../../standard/rfc4302/catalog.md#rfc4302-fmt-3) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-FMT-4](../../standard/rfc4302/catalog.md#rfc4302-fmt-4) | later | — | — | — | level 5: a key management protocol |
| [RFC4302-NH-1](../../standard/rfc4302/catalog.md#rfc4302-nh-1) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-NH-2](../../standard/rfc4302/catalog.md#rfc4302-nh-2) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-LEN-1](../../standard/rfc4302/catalog.md#rfc4302-len-1) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | FAIL | observation 6: the Payload Length is 0, [gap 2](results.md#gap-2-defect--the-ah-payload-length-is-always-0) |
| [RFC4302-LEN-2](../../standard/rfc4302/catalog.md#rfc4302-len-2) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-RSV-1](../../standard/rfc4302/catalog.md#rfc4302-rsv-1) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-RSV-2](../../standard/rfc4302/catalog.md#rfc4302-rsv-2) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-RSV-3](../../standard/rfc4302/catalog.md#rfc4302-rsv-3) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-RSV-4](../../standard/rfc4302/catalog.md#rfc4302-rsv-4) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-SPI-1](../../standard/rfc4302/catalog.md#rfc4302-spi-1) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-2](../../standard/rfc4302/catalog.md#rfc4302-spi-2) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-3](../../standard/rfc4302/catalog.md#rfc4302-spi-3) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-4](../../standard/rfc4302/catalog.md#rfc4302-spi-4) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-5](../../standard/rfc4302/catalog.md#rfc4302-spi-5) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-6](../../standard/rfc4302/catalog.md#rfc4302-spi-6) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SPI-7](../../standard/rfc4302/catalog.md#rfc4302-spi-7) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SPI-8](../../standard/rfc4302/catalog.md#rfc4302-spi-8) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SPI-9](../../standard/rfc4302/catalog.md#rfc4302-spi-9) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SPI-10](../../standard/rfc4302/catalog.md#rfc4302-spi-10) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-11](../../standard/rfc4302/catalog.md#rfc4302-spi-11) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-12](../../standard/rfc4302/catalog.md#rfc4302-spi-12) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-13](../../standard/rfc4302/catalog.md#rfc4302-spi-13) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-14](../../standard/rfc4302/catalog.md#rfc4302-spi-14) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-15](../../standard/rfc4302/catalog.md#rfc4302-spi-15) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-16](../../standard/rfc4302/catalog.md#rfc4302-spi-16) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-SPI-17](../../standard/rfc4302/catalog.md#rfc4302-spi-17) | later | — | — | — | level 5: a key management protocol |
| [RFC4302-SPI-18](../../standard/rfc4302/catalog.md#rfc4302-spi-18) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SPI-19](../../standard/rfc4302/catalog.md#rfc4302-spi-19) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SPI-20](../../standard/rfc4302/catalog.md#rfc4302-spi-20) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-SPI-21](../../standard/rfc4302/catalog.md#rfc4302-spi-21) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-SEQ-1](../../standard/rfc4302/catalog.md#rfc4302-seq-1) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS | observation 2: each packet carries the number before it plus one |
| [RFC4302-SEQ-2](../../standard/rfc4302/catalog.md#rfc4302-seq-2) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS | observation 2: each packet carries the number before it plus one |
| [RFC4302-SEQ-3](../../standard/rfc4302/catalog.md#rfc4302-seq-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SEQ-4](../../standard/rfc4302/catalog.md#rfc4302-seq-4) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-SEQ-5](../../standard/rfc4302/catalog.md#rfc4302-seq-5) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-SEQ-6](../../standard/rfc4302/catalog.md#rfc4302-seq-6) | covered | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS |  |
| [RFC4302-SEQ-7](../../standard/rfc4302/catalog.md#rfc4302-seq-7) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-SEQ-8](../../standard/rfc4302/catalog.md#rfc4302-seq-8) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4302SequenceNumbers` | FAIL | observation 4: the first packet carries 0, [gap 1](results.md#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0) |
| [RFC4302-SEQ-9](../../standard/rfc4302/catalog.md#rfc4302-seq-9) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4302-SEQ-10](../../standard/rfc4302/catalog.md#rfc4302-seq-10) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4302-SEQ-11](../../standard/rfc4302/catalog.md#rfc4302-seq-11) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-SEQ-12](../../standard/rfc4302/catalog.md#rfc4302-seq-12) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-SEQ-13](../../standard/rfc4302/catalog.md#rfc4302-seq-13) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-SEQ-14](../../standard/rfc4302/catalog.md#rfc4302-seq-14) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-SEQ-15](../../standard/rfc4302/catalog.md#rfc4302-seq-15) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-ICV-1](../../standard/rfc4302/catalog.md#rfc4302-icv-1) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-ICV-2](../../standard/rfc4302/catalog.md#rfc4302-icv-2) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-ICV-3](../../standard/rfc4302/catalog.md#rfc4302-icv-3) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-ICV-4](../../standard/rfc4302/catalog.md#rfc4302-icv-4) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-ICV-5](../../standard/rfc4302/catalog.md#rfc4302-icv-5) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-LOC-1](../../standard/rfc4302/catalog.md#rfc4302-loc-1) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-LOC-2](../../standard/rfc4302/catalog.md#rfc4302-loc-2) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-LOC-3](../../standard/rfc4302/catalog.md#rfc4302-loc-3) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-LOC-4](../../standard/rfc4302/catalog.md#rfc4302-loc-4) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-LOC-5](../../standard/rfc4302/catalog.md#rfc4302-loc-5) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-LOC-6](../../standard/rfc4302/catalog.md#rfc4302-loc-6) | owed | — | — | — | a later level 2 pass: a packet with a Hop-by-Hop or Destination Options header, to see where AH or ESP goes and which headers the selectors skip |
| [RFC4302-LOC-7](../../standard/rfc4302/catalog.md#rfc4302-loc-7) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-LOC-8](../../standard/rfc4302/catalog.md#rfc4302-loc-8) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-LOC-9](../../standard/rfc4302/catalog.md#rfc4302-loc-9) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4302-LOC-10](../../standard/rfc4302/catalog.md#rfc4302-loc-10) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-LOC-11](../../standard/rfc4302/catalog.md#rfc4302-loc-11) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4302-LOC-12](../../standard/rfc4302/catalog.md#rfc4302-loc-12) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4302-LOC-13](../../standard/rfc4302/catalog.md#rfc4302-loc-13) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4302-LOC-14](../../standard/rfc4302/catalog.md#rfc4302-loc-14) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4302-LOC-15](../../standard/rfc4302/catalog.md#rfc4302-loc-15) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-ALG-1](../../standard/rfc4302/catalog.md#rfc4302-alg-1) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-ALG-2](../../standard/rfc4302/catalog.md#rfc4302-alg-2) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OSA-1](../../standard/rfc4302/catalog.md#rfc4302-osa-1) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-OSA-2](../../standard/rfc4302/catalog.md#rfc4302-osa-2) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-OSA-3](../../standard/rfc4302/catalog.md#rfc4302-osa-3) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4302-OSEQ-1](../../standard/rfc4302/catalog.md#rfc4302-oseq-1) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4302SequenceNumbers` | FAIL | observation 4: the first packet carries 0, [gap 1](results.md#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0) |
| [RFC4302-OSEQ-2](../../standard/rfc4302/catalog.md#rfc4302-oseq-2) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS | observation 2: each packet carries the number before it plus one |
| [RFC4302-OSEQ-3](../../standard/rfc4302/catalog.md#rfc4302-oseq-3) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4302-OSEQ-4](../../standard/rfc4302/catalog.md#rfc4302-oseq-4) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4302-OSEQ-5](../../standard/rfc4302/catalog.md#rfc4302-oseq-5) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4302-OSEQ-6](../../standard/rfc4302/catalog.md#rfc4302-oseq-6) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4302-OSEQ-7](../../standard/rfc4302/catalog.md#rfc4302-oseq-7) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4302-OSEQ-8](../../standard/rfc4302/catalog.md#rfc4302-oseq-8) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4302-OSEQ-9](../../standard/rfc4302/catalog.md#rfc4302-oseq-9) | covered | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS |  |
| [RFC4302-OSEQ-10](../../standard/rfc4302/catalog.md#rfc4302-oseq-10) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-OSEQ-11](../../standard/rfc4302/catalog.md#rfc4302-oseq-11) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-OSEQ-12](../../standard/rfc4302/catalog.md#rfc4302-oseq-12) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-OICV-1](../../standard/rfc4302/catalog.md#rfc4302-oicv-1) | selected | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-2](../../standard/rfc4302/catalog.md#rfc4302-oicv-2) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-3](../../standard/rfc4302/catalog.md#rfc4302-oicv-3) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-4](../../standard/rfc4302/catalog.md#rfc4302-oicv-4) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OICV-5](../../standard/rfc4302/catalog.md#rfc4302-oicv-5) | selected | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-6](../../standard/rfc4302/catalog.md#rfc4302-oicv-6) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-7](../../standard/rfc4302/catalog.md#rfc4302-oicv-7) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-8](../../standard/rfc4302/catalog.md#rfc4302-oicv-8) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-9](../../standard/rfc4302/catalog.md#rfc4302-oicv-9) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-10](../../standard/rfc4302/catalog.md#rfc4302-oicv-10) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-11](../../standard/rfc4302/catalog.md#rfc4302-oicv-11) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-12](../../standard/rfc4302/catalog.md#rfc4302-oicv-12) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-13](../../standard/rfc4302/catalog.md#rfc4302-oicv-13) | selected | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-14](../../standard/rfc4302/catalog.md#rfc4302-oicv-14) | covered | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4302-OICV-15](../../standard/rfc4302/catalog.md#rfc4302-oicv-15) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-16](../../standard/rfc4302/catalog.md#rfc4302-oicv-16) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-17](../../standard/rfc4302/catalog.md#rfc4302-oicv-17) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-18](../../standard/rfc4302/catalog.md#rfc4302-oicv-18) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-19](../../standard/rfc4302/catalog.md#rfc4302-oicv-19) | selected | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-OICV-20](../../standard/rfc4302/catalog.md#rfc4302-oicv-20) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-21](../../standard/rfc4302/catalog.md#rfc4302-oicv-21) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-22](../../standard/rfc4302/catalog.md#rfc4302-oicv-22) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-23](../../standard/rfc4302/catalog.md#rfc4302-oicv-23) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-24](../../standard/rfc4302/catalog.md#rfc4302-oicv-24) | no check | — | — | — | not claimed: the model computes no ICV, so no field of an option or extension header counts in one; the Mobility Header is not a selector |
| [RFC4302-OICV-25](../../standard/rfc4302/catalog.md#rfc4302-oicv-25) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-OICV-26](../../standard/rfc4302/catalog.md#rfc4302-oicv-26) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-OICV-27](../../standard/rfc4302/catalog.md#rfc4302-oicv-27) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-OICV-28](../../standard/rfc4302/catalog.md#rfc4302-oicv-28) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | observation 5: the ICV follows the payload, and has no padding in IPv6, [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-OICV-29](../../standard/rfc4302/catalog.md#rfc4302-oicv-29) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-OICV-30](../../standard/rfc4302/catalog.md#rfc4302-oicv-30) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-OICV-31](../../standard/rfc4302/catalog.md#rfc4302-oicv-31) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OICV-32](../../standard/rfc4302/catalog.md#rfc4302-oicv-32) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OICV-33](../../standard/rfc4302/catalog.md#rfc4302-oicv-33) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OICV-34](../../standard/rfc4302/catalog.md#rfc4302-oicv-34) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OICV-35](../../standard/rfc4302/catalog.md#rfc4302-oicv-35) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OICV-36](../../standard/rfc4302/catalog.md#rfc4302-oicv-36) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-OICV-37](../../standard/rfc4302/catalog.md#rfc4302-oicv-37) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-FRAG-1](../../standard/rfc4302/catalog.md#rfc4302-frag-1) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4302-FRAG-2](../../standard/rfc4302/catalog.md#rfc4302-frag-2) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4302-FRAG-3](../../standard/rfc4302/catalog.md#rfc4302-frag-3) | covered | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4302-FRAG-4](../../standard/rfc4302/catalog.md#rfc4302-frag-4) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-FRAG-5](../../standard/rfc4302/catalog.md#rfc4302-frag-5) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-FRAG-6](../../standard/rfc4302/catalog.md#rfc4302-frag-6) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-FRAG-7](../../standard/rfc4302/catalog.md#rfc4302-frag-7) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4302-FRAG-8](../../standard/rfc4302/catalog.md#rfc4302-frag-8) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4302-FRAG-9](../../standard/rfc4302/catalog.md#rfc4302-frag-9) | covered | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | `Rfc4301PathMtuIpv6` | FAIL (expected) | observation 2: A sends packets of 1410 octets after the Packet Too Big message, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu) |
| [RFC4302-FRAG-10](../../standard/rfc4302/catalog.md#rfc4302-frag-10) | selected | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | `Rfc4301PathMtuIpv6` | FAIL (expected) | observation 2: A sends packets of 1410 octets after the Packet Too Big message, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu) |
| [RFC4302-REAS-1](../../standard/rfc4302/catalog.md#rfc4302-reas-1) | covered | [AH and ESP on one packet](../../protocol/ipsec/checks/modes.md#ah-and-esp-on-one-packet) | `Rfc4301AhAndEsp` | FAIL | not reached: no packet with two IPsec headers, [gap 7](results.md#gap-7-untestable-claim--ah-and-esp-cannot-protect-one-packet) |
| [RFC4302-REAS-2](../../standard/rfc4302/catalog.md#rfc4302-reas-2) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4302-REAS-3](../../standard/rfc4302/catalog.md#rfc4302-reas-3) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-REAS-4](../../standard/rfc4302/catalog.md#rfc4302-reas-4) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4302-REAS-5](../../standard/rfc4302/catalog.md#rfc4302-reas-5) | covered | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4302-ISA-1](../../standard/rfc4302/catalog.md#rfc4302-isa-1) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-ISA-2](../../standard/rfc4302/catalog.md#rfc4302-isa-2) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-ISA-3](../../standard/rfc4302/catalog.md#rfc4302-isa-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-ISA-4](../../standard/rfc4302/catalog.md#rfc4302-isa-4) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-ISA-5](../../standard/rfc4302/catalog.md#rfc4302-isa-5) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-ISA-6](../../standard/rfc4302/catalog.md#rfc4302-isa-6) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4302-ISA-7](../../standard/rfc4302/catalog.md#rfc4302-isa-7) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4302-ISA-8](../../standard/rfc4302/catalog.md#rfc4302-isa-8) | later | — | — | — | level 5: a key management protocol |
| [RFC4302-ISEQ-1](../../standard/rfc4302/catalog.md#rfc4302-iseq-1) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-2](../../standard/rfc4302/catalog.md#rfc4302-iseq-2) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-3](../../standard/rfc4302/catalog.md#rfc4302-iseq-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-ISEQ-4](../../standard/rfc4302/catalog.md#rfc4302-iseq-4) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-ISEQ-5](../../standard/rfc4302/catalog.md#rfc4302-iseq-5) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-ISEQ-6](../../standard/rfc4302/catalog.md#rfc4302-iseq-6) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-7](../../standard/rfc4302/catalog.md#rfc4302-iseq-7) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-8](../../standard/rfc4302/catalog.md#rfc4302-iseq-8) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-9](../../standard/rfc4302/catalog.md#rfc4302-iseq-9) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-10](../../standard/rfc4302/catalog.md#rfc4302-iseq-10) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-11](../../standard/rfc4302/catalog.md#rfc4302-iseq-11) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-12](../../standard/rfc4302/catalog.md#rfc4302-iseq-12) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-13](../../standard/rfc4302/catalog.md#rfc4302-iseq-13) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-14](../../standard/rfc4302/catalog.md#rfc4302-iseq-14) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-15](../../standard/rfc4302/catalog.md#rfc4302-iseq-15) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-16](../../standard/rfc4302/catalog.md#rfc4302-iseq-16) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-ISEQ-17](../../standard/rfc4302/catalog.md#rfc4302-iseq-17) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-ISEQ-18](../../standard/rfc4302/catalog.md#rfc4302-iseq-18) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-ISEQ-19](../../standard/rfc4302/catalog.md#rfc4302-iseq-19) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-ISEQ-20](../../standard/rfc4302/catalog.md#rfc4302-iseq-20) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-21](../../standard/rfc4302/catalog.md#rfc4302-iseq-21) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-22](../../standard/rfc4302/catalog.md#rfc4302-iseq-22) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4302-ISEQ-23](../../standard/rfc4302/catalog.md#rfc4302-iseq-23) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-24](../../standard/rfc4302/catalog.md#rfc4302-iseq-24) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-25](../../standard/rfc4302/catalog.md#rfc4302-iseq-25) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-26](../../standard/rfc4302/catalog.md#rfc4302-iseq-26) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-27](../../standard/rfc4302/catalog.md#rfc4302-iseq-27) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-28](../../standard/rfc4302/catalog.md#rfc4302-iseq-28) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-ISEQ-29](../../standard/rfc4302/catalog.md#rfc4302-iseq-29) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-IICV-1](../../standard/rfc4302/catalog.md#rfc4302-iicv-1) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-IICV-2](../../standard/rfc4302/catalog.md#rfc4302-iicv-2) | selected | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6` | PASS | observations 1 to 4; the tests fail at observation 6 |
| [RFC4302-IICV-3](../../standard/rfc4302/catalog.md#rfc4302-iicv-3) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-IICV-4](../../standard/rfc4302/catalog.md#rfc4302-iicv-4) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4302-IICV-5](../../standard/rfc4302/catalog.md#rfc4302-iicv-5) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-IICV-6](../../standard/rfc4302/catalog.md#rfc4302-iicv-6) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-IICV-7](../../standard/rfc4302/catalog.md#rfc4302-iicv-7) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4302-IICV-8](../../standard/rfc4302/catalog.md#rfc4302-iicv-8) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-IICV-9](../../standard/rfc4302/catalog.md#rfc4302-iicv-9) | covered | [AH across a router](../../protocol/ipsec/checks/ah.md#ah-across-a-router) | `Rfc4302AhAcrossRouter`, `Rfc4302AhAcrossRouterIpv6` | PASS | with no weight: the receiver never checks the ICV (`IPsec.cc:832`), so a changed TTL cannot make it reject the packet |
| [RFC4302-CONF-1](../../standard/rfc4302/catalog.md#rfc4302-conf-1) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6`, `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | AH is not fully implemented: [gap 1](results.md#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0), [gap 2](results.md#gap-2-defect--the-ah-payload-length-is-always-0), [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-CONF-2](../../standard/rfc4302/catalog.md#rfc4302-conf-2) | covered | [AH format in transport mode](../../protocol/ipsec/checks/ah.md#ah-format-in-transport-mode) | `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6`, `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6` | FAIL | AH is not fully implemented: [gap 1](results.md#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0), [gap 2](results.md#gap-2-defect--the-ah-payload-length-is-always-0), [gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet) |
| [RFC4302-CONF-3](../../standard/rfc4302/catalog.md#rfc4302-conf-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4302-CONF-4](../../standard/rfc4302/catalog.md#rfc4302-conf-4) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-CONF-5](../../standard/rfc4302/catalog.md#rfc4302-conf-5) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4302-CONF-6](../../standard/rfc4302/catalog.md#rfc4302-conf-6) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4302-CONF-7](../../standard/rfc4302/catalog.md#rfc4302-conf-7) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |

### RFC 4303

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC4303-FMT-1](../../standard/rfc4303/catalog.md#rfc4303-fmt-1) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-2](../../standard/rfc4303/catalog.md#rfc4303-fmt-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-3](../../standard/rfc4303/catalog.md#rfc4303-fmt-3) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-4](../../standard/rfc4303/catalog.md#rfc4303-fmt-4) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-FMT-5](../../standard/rfc4303/catalog.md#rfc4303-fmt-5) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-FMT-6](../../standard/rfc4303/catalog.md#rfc4303-fmt-6) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-FMT-7](../../standard/rfc4303/catalog.md#rfc4303-fmt-7) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-8](../../standard/rfc4303/catalog.md#rfc4303-fmt-8) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-FMT-9](../../standard/rfc4303/catalog.md#rfc4303-fmt-9) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-FMT-10](../../standard/rfc4303/catalog.md#rfc4303-fmt-10) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-FMT-11](../../standard/rfc4303/catalog.md#rfc4303-fmt-11) | covered | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-FMT-12](../../standard/rfc4303/catalog.md#rfc4303-fmt-12) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4303-FMT-13](../../standard/rfc4303/catalog.md#rfc4303-fmt-13) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-14](../../standard/rfc4303/catalog.md#rfc4303-fmt-14) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-15](../../standard/rfc4303/catalog.md#rfc4303-fmt-15) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-16](../../standard/rfc4303/catalog.md#rfc4303-fmt-16) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-FMT-17](../../standard/rfc4303/catalog.md#rfc4303-fmt-17) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-FMT-18](../../standard/rfc4303/catalog.md#rfc4303-fmt-18) | later | — | — | — | level 5: a key management protocol |
| [RFC4303-SPI-1](../../standard/rfc4303/catalog.md#rfc4303-spi-1) | covered | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | `Rfc4301SaPerDirection` | PASS |  |
| [RFC4303-SPI-2](../../standard/rfc4303/catalog.md#rfc4303-spi-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-SPI-3](../../standard/rfc4303/catalog.md#rfc4303-spi-3) | covered | [One SA for each direction](../../protocol/ipsec/checks/security-associations.md#one-sa-for-each-direction) | `Rfc4301SaPerDirection` | PASS |  |
| [RFC4303-SPI-4](../../standard/rfc4303/catalog.md#rfc4303-spi-4) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-SPI-5](../../standard/rfc4303/catalog.md#rfc4303-spi-5) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SPI-6](../../standard/rfc4303/catalog.md#rfc4303-spi-6) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SPI-7](../../standard/rfc4303/catalog.md#rfc4303-spi-7) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SPI-8](../../standard/rfc4303/catalog.md#rfc4303-spi-8) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SPI-9](../../standard/rfc4303/catalog.md#rfc4303-spi-9) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SPI-10](../../standard/rfc4303/catalog.md#rfc4303-spi-10) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-SPI-11](../../standard/rfc4303/catalog.md#rfc4303-spi-11) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-SPI-12](../../standard/rfc4303/catalog.md#rfc4303-spi-12) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-SPI-13](../../standard/rfc4303/catalog.md#rfc4303-spi-13) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-SPI-14](../../standard/rfc4303/catalog.md#rfc4303-spi-14) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-SPI-15](../../standard/rfc4303/catalog.md#rfc4303-spi-15) | later | — | — | — | level 5: a key management protocol |
| [RFC4303-SPI-16](../../standard/rfc4303/catalog.md#rfc4303-spi-16) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SPI-17](../../standard/rfc4303/catalog.md#rfc4303-spi-17) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SPI-18](../../standard/rfc4303/catalog.md#rfc4303-spi-18) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-SEQ-1](../../standard/rfc4303/catalog.md#rfc4303-seq-1) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS | observation 2: each packet carries the number before it plus one |
| [RFC4303-SEQ-2](../../standard/rfc4303/catalog.md#rfc4303-seq-2) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS | observation 2: each packet carries the number before it plus one |
| [RFC4303-SEQ-3](../../standard/rfc4303/catalog.md#rfc4303-seq-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SEQ-4](../../standard/rfc4303/catalog.md#rfc4303-seq-4) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-SEQ-5](../../standard/rfc4303/catalog.md#rfc4303-seq-5) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-SEQ-6](../../standard/rfc4303/catalog.md#rfc4303-seq-6) | covered | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS |  |
| [RFC4303-SEQ-7](../../standard/rfc4303/catalog.md#rfc4303-seq-7) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers` | FAIL | observation 4: the first packet carries 0, [gap 1](results.md#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0) |
| [RFC4303-SEQ-8](../../standard/rfc4303/catalog.md#rfc4303-seq-8) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-SEQ-9](../../standard/rfc4303/catalog.md#rfc4303-seq-9) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4303-SEQ-10](../../standard/rfc4303/catalog.md#rfc4303-seq-10) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-SEQ-11](../../standard/rfc4303/catalog.md#rfc4303-seq-11) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-SEQ-12](../../standard/rfc4303/catalog.md#rfc4303-seq-12) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-SEQ-13](../../standard/rfc4303/catalog.md#rfc4303-seq-13) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-SEQ-14](../../standard/rfc4303/catalog.md#rfc4303-seq-14) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-SEQ-15](../../standard/rfc4303/catalog.md#rfc4303-seq-15) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-SEQ-16](../../standard/rfc4303/catalog.md#rfc4303-seq-16) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-PAY-1](../../standard/rfc4303/catalog.md#rfc4303-pay-1) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-PAY-2](../../standard/rfc4303/catalog.md#rfc4303-pay-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-PAY-3](../../standard/rfc4303/catalog.md#rfc4303-pay-3) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-PAY-4](../../standard/rfc4303/catalog.md#rfc4303-pay-4) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-PAY-5](../../standard/rfc4303/catalog.md#rfc4303-pay-5) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-PAD-1](../../standard/rfc4303/catalog.md#rfc4303-pad-1) | selected | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS | observations 2 to 4; the test fails at observation 5 |
| [RFC4303-PAD-2](../../standard/rfc4303/catalog.md#rfc4303-pad-2) | selected | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS | observations 2 to 4; the test fails at observation 5 |
| [RFC4303-PAD-3](../../standard/rfc4303/catalog.md#rfc4303-pad-3) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4303-PAD-4](../../standard/rfc4303/catalog.md#rfc4303-pad-4) | covered | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS |  |
| [RFC4303-PAD-5](../../standard/rfc4303/catalog.md#rfc4303-pad-5) | selected | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS | observations 2 to 4; the test fails at observation 5 |
| [RFC4303-PAD-6](../../standard/rfc4303/catalog.md#rfc4303-pad-6) | selected | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS | observations 2 to 4; the test fails at observation 5 |
| [RFC4303-PAD-7](../../standard/rfc4303/catalog.md#rfc4303-pad-7) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-PAD-8](../../standard/rfc4303/catalog.md#rfc4303-pad-8) | selected | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS | observations 2 to 4; the test fails at observation 5 |
| [RFC4303-PAD-9](../../standard/rfc4303/catalog.md#rfc4303-pad-9) | selected | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | FAIL | observation 5: the padding octets are 63, 63, 63, [gap 4](results.md#gap-4-defect--the-esp-padding-octets-are-63-not-1-2-3) |
| [RFC4303-PAD-10](../../standard/rfc4303/catalog.md#rfc4303-pad-10) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-PADL-1](../../standard/rfc4303/catalog.md#rfc4303-padl-1) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-PADL-2](../../standard/rfc4303/catalog.md#rfc4303-padl-2) | covered | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-PADL-3](../../standard/rfc4303/catalog.md#rfc4303-padl-3) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-NH-1](../../standard/rfc4303/catalog.md#rfc4303-nh-1) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-NH-2](../../standard/rfc4303/catalog.md#rfc4303-nh-2) | selected | [Dummy packets](../../protocol/ipsec/checks/esp.md#dummy-packets) | `Rfc4303DummyPackets` | FAIL (expected) | observation 1: no dummy packet, [gap 11](results.md#gap-11-unimplemented-feature--no-dummy-packets) |
| [RFC4303-NH-3](../../standard/rfc4303/catalog.md#rfc4303-nh-3) | selected | [Dummy packets](../../protocol/ipsec/checks/esp.md#dummy-packets) | `Rfc4303DummyPackets` | FAIL (expected) | observation 1: no dummy packet, [gap 11](results.md#gap-11-unimplemented-feature--no-dummy-packets) |
| [RFC4303-NH-4](../../standard/rfc4303/catalog.md#rfc4303-nh-4) | selected | [Dummy packets](../../protocol/ipsec/checks/esp.md#dummy-packets) | `Rfc4303DummyPackets` | FAIL (expected) | not reached: no dummy packet reaches B, [gap 11](results.md#gap-11-unimplemented-feature--no-dummy-packets) |
| [RFC4303-NH-5](../../standard/rfc4303/catalog.md#rfc4303-nh-5) | selected | [Dummy packets](../../protocol/ipsec/checks/esp.md#dummy-packets) | `Rfc4303DummyPackets` | FAIL (expected) | observation 1: no dummy packet, [gap 11](results.md#gap-11-unimplemented-feature--no-dummy-packets) |
| [RFC4303-NH-6](../../standard/rfc4303/catalog.md#rfc4303-nh-6) | covered | [Dummy packets](../../protocol/ipsec/checks/esp.md#dummy-packets) | `Rfc4303DummyPackets` | FAIL (expected) | observation 1: no dummy packet, [gap 11](results.md#gap-11-unimplemented-feature--no-dummy-packets) |
| [RFC4303-TFC-1](../../standard/rfc4303/catalog.md#rfc4303-tfc-1) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4303-TFC-2](../../standard/rfc4303/catalog.md#rfc4303-tfc-2) | selected | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-TFC-3](../../standard/rfc4303/catalog.md#rfc4303-tfc-3) | covered | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-TFC-4](../../standard/rfc4303/catalog.md#rfc4303-tfc-4) | covered | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-TFC-5](../../standard/rfc4303/catalog.md#rfc4303-tfc-5) | selected | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-TFC-6](../../standard/rfc4303/catalog.md#rfc4303-tfc-6) | covered | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-TFC-7](../../standard/rfc4303/catalog.md#rfc4303-tfc-7) | later | — | — | — | level 5: a key management protocol |
| [RFC4303-TFC-8](../../standard/rfc4303/catalog.md#rfc4303-tfc-8) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4303-ICV-1](../../standard/rfc4303/catalog.md#rfc4303-icv-1) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-ICV-2](../../standard/rfc4303/catalog.md#rfc4303-icv-2) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-ICV-3](../../standard/rfc4303/catalog.md#rfc4303-icv-3) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-ICV-4](../../standard/rfc4303/catalog.md#rfc4303-icv-4) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-LOC-1](../../standard/rfc4303/catalog.md#rfc4303-loc-1) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4303-LOC-2](../../standard/rfc4303/catalog.md#rfc4303-loc-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-LOC-3](../../standard/rfc4303/catalog.md#rfc4303-loc-3) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-LOC-4](../../standard/rfc4303/catalog.md#rfc4303-loc-4) | selected | [AH and ESP on one packet](../../protocol/ipsec/checks/modes.md#ah-and-esp-on-one-packet) | `Rfc4301AhAndEsp` | FAIL | observation 1: the configuration cannot give one flow both protections, [gap 7](results.md#gap-7-untestable-claim--ah-and-esp-cannot-protect-one-packet) |
| [RFC4303-LOC-5](../../standard/rfc4303/catalog.md#rfc4303-loc-5) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-LOC-6](../../standard/rfc4303/catalog.md#rfc4303-loc-6) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-LOC-7](../../standard/rfc4303/catalog.md#rfc4303-loc-7) | owed | — | — | — | a later level 2 pass: a packet with a Hop-by-Hop or Destination Options header, to see where AH or ESP goes and which headers the selectors skip |
| [RFC4303-LOC-8](../../standard/rfc4303/catalog.md#rfc4303-loc-8) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4303-LOC-9](../../standard/rfc4303/catalog.md#rfc4303-loc-9) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4303-LOC-10](../../standard/rfc4303/catalog.md#rfc4303-loc-10) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4303-LOC-11](../../standard/rfc4303/catalog.md#rfc4303-loc-11) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4303-LOC-12](../../standard/rfc4303/catalog.md#rfc4303-loc-12) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4303-LOC-13](../../standard/rfc4303/catalog.md#rfc4303-loc-13) | covered | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4303-ALG-1](../../standard/rfc4303/catalog.md#rfc4303-alg-1) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-ALG-2](../../standard/rfc4303/catalog.md#rfc4303-alg-2) | selected | [An ESP SA without encryption and without integrity](../../protocol/ipsec/checks/esp.md#an-esp-sa-without-encryption-and-without-integrity) | `Rfc4303EspBothNull` | PASS |  |
| [RFC4303-ALG-3](../../standard/rfc4303/catalog.md#rfc4303-alg-3) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-ALG-4](../../standard/rfc4303/catalog.md#rfc4303-alg-4) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-ALG-5](../../standard/rfc4303/catalog.md#rfc4303-alg-5) | selected | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-ALG-6](../../standard/rfc4303/catalog.md#rfc4303-alg-6) | selected | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-ALG-7](../../standard/rfc4303/catalog.md#rfc4303-alg-7) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-ALG-8](../../standard/rfc4303/catalog.md#rfc4303-alg-8) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-ALG-9](../../standard/rfc4303/catalog.md#rfc4303-alg-9) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-ALG-10](../../standard/rfc4303/catalog.md#rfc4303-alg-10) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-ALG-11](../../standard/rfc4303/catalog.md#rfc4303-alg-11) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-ALG-12](../../standard/rfc4303/catalog.md#rfc4303-alg-12) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OSA-1](../../standard/rfc4303/catalog.md#rfc4303-osa-1) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-OSA-2](../../standard/rfc4303/catalog.md#rfc4303-osa-2) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-OENC-1](../../standard/rfc4303/catalog.md#rfc4303-oenc-1) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-OENC-2](../../standard/rfc4303/catalog.md#rfc4303-oenc-2) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp`, `Rfc4301TunnelModeAh` | FAIL (expected) | observation 3: no inner IPv4 header, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4303-OENC-3](../../standard/rfc4303/catalog.md#rfc4303-oenc-3) | covered | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS |  |
| [RFC4303-OENC-4](../../standard/rfc4303/catalog.md#rfc4303-oenc-4) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-OENC-5](../../standard/rfc4303/catalog.md#rfc4303-oenc-5) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-6](../../standard/rfc4303/catalog.md#rfc4303-oenc-6) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-7](../../standard/rfc4303/catalog.md#rfc4303-oenc-7) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-8](../../standard/rfc4303/catalog.md#rfc4303-oenc-8) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-9](../../standard/rfc4303/catalog.md#rfc4303-oenc-9) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-OENC-10](../../standard/rfc4303/catalog.md#rfc4303-oenc-10) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-OENC-11](../../standard/rfc4303/catalog.md#rfc4303-oenc-11) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-12](../../standard/rfc4303/catalog.md#rfc4303-oenc-12) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-13](../../standard/rfc4303/catalog.md#rfc4303-oenc-13) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-14](../../standard/rfc4303/catalog.md#rfc4303-oenc-14) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-15](../../standard/rfc4303/catalog.md#rfc4303-oenc-15) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-16](../../standard/rfc4303/catalog.md#rfc4303-oenc-16) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-17](../../standard/rfc4303/catalog.md#rfc4303-oenc-17) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OENC-18](../../standard/rfc4303/catalog.md#rfc4303-oenc-18) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-OSEQ-1](../../standard/rfc4303/catalog.md#rfc4303-oseq-1) | selected | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers` | FAIL | observation 4: the first packet carries 0, [gap 1](results.md#gap-1-defect--the-first-packet-of-an-sa-carries-sequence-number-0) |
| [RFC4303-OSEQ-2](../../standard/rfc4303/catalog.md#rfc4303-oseq-2) | covered | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS |  |
| [RFC4303-OSEQ-3](../../standard/rfc4303/catalog.md#rfc4303-oseq-3) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4303-OSEQ-4](../../standard/rfc4303/catalog.md#rfc4303-oseq-4) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-OSEQ-5](../../standard/rfc4303/catalog.md#rfc4303-oseq-5) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-OSEQ-6](../../standard/rfc4303/catalog.md#rfc4303-oseq-6) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4303-OSEQ-7](../../standard/rfc4303/catalog.md#rfc4303-oseq-7) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4303-OSEQ-8](../../standard/rfc4303/catalog.md#rfc4303-oseq-8) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4303-OSEQ-9](../../standard/rfc4303/catalog.md#rfc4303-oseq-9) | later | — | — | — | level 4: a counter near 2^32 |
| [RFC4303-OSEQ-10](../../standard/rfc4303/catalog.md#rfc4303-oseq-10) | covered | [Sequence numbers of an SA](../../protocol/ipsec/checks/sequence-numbers.md#sequence-numbers-of-an-sa) | `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers` | PASS |  |
| [RFC4303-OSEQ-11](../../standard/rfc4303/catalog.md#rfc4303-oseq-11) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-OSEQ-12](../../standard/rfc4303/catalog.md#rfc4303-oseq-12) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-OSEQ-13](../../standard/rfc4303/catalog.md#rfc4303-oseq-13) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-FRAG-1](../../standard/rfc4303/catalog.md#rfc4303-frag-1) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4303-FRAG-2](../../standard/rfc4303/catalog.md#rfc4303-frag-2) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4303-FRAG-3](../../standard/rfc4303/catalog.md#rfc4303-frag-3) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4303-FRAG-4](../../standard/rfc4303/catalog.md#rfc4303-frag-4) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4303-FRAG-5](../../standard/rfc4303/catalog.md#rfc4303-frag-5) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4303-FRAG-6](../../standard/rfc4303/catalog.md#rfc4303-frag-6) | no check | — | — | — | not claimed: the model is a host in transport mode only (`IPsec.ned:40`); a gateway, a tunnel beyond one host pair, or a bump-in-the-stack |
| [RFC4303-FRAG-7](../../standard/rfc4303/catalog.md#rfc4303-frag-7) | no check | — | — | — | a permission or a description that no observation can fail |
| [RFC4303-FRAG-8](../../standard/rfc4303/catalog.md#rfc4303-frag-8) | covered | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | `Rfc4301PathMtuIpv6` | FAIL (expected) | observation 2: A sends packets of 1410 octets after the Packet Too Big message, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu) |
| [RFC4303-FRAG-9](../../standard/rfc4303/catalog.md#rfc4303-frag-9) | selected | [Path MTU of a protected flow](../../protocol/ipsec/checks/fragmentation.md#path-mtu-of-a-protected-flow) | `Rfc4301PathMtuIpv6` | FAIL (expected) | observation 2: A sends packets of 1410 octets after the Packet Too Big message, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu) |
| [RFC4303-REAS-1](../../standard/rfc4303/catalog.md#rfc4303-reas-1) | selected | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4303-REAS-2](../../standard/rfc4303/catalog.md#rfc4303-reas-2) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-REAS-3](../../standard/rfc4303/catalog.md#rfc4303-reas-3) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-REAS-4](../../standard/rfc4303/catalog.md#rfc4303-reas-4) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-REAS-5](../../standard/rfc4303/catalog.md#rfc4303-reas-5) | covered | [Fragments of a protected datagram](../../protocol/ipsec/checks/fragmentation.md#fragments-of-a-protected-datagram) | `Rfc4301Fragments`, `Rfc4301FragmentsIpv6` | PASS |  |
| [RFC4303-ISA-1](../../standard/rfc4303/catalog.md#rfc4303-isa-1) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-ISA-2](../../standard/rfc4303/catalog.md#rfc4303-isa-2) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-ISA-3](../../standard/rfc4303/catalog.md#rfc4303-isa-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-ISA-4](../../standard/rfc4303/catalog.md#rfc4303-isa-4) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-ISA-5](../../standard/rfc4303/catalog.md#rfc4303-isa-5) | covered | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-ISA-6](../../standard/rfc4303/catalog.md#rfc4303-isa-6) | selected | [Inbound SA lookup](../../protocol/ipsec/checks/security-associations.md#inbound-sa-lookup) | `Rfc4301InboundSaLookup` | PASS |  |
| [RFC4303-ISA-7](../../standard/rfc4303/catalog.md#rfc4303-isa-7) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-ISA-8](../../standard/rfc4303/catalog.md#rfc4303-isa-8) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-ISEQ-1](../../standard/rfc4303/catalog.md#rfc4303-iseq-1) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-2](../../standard/rfc4303/catalog.md#rfc4303-iseq-2) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-3](../../standard/rfc4303/catalog.md#rfc4303-iseq-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-ISEQ-4](../../standard/rfc4303/catalog.md#rfc4303-iseq-4) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-ISEQ-5](../../standard/rfc4303/catalog.md#rfc4303-iseq-5) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-6](../../standard/rfc4303/catalog.md#rfc4303-iseq-6) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-7](../../standard/rfc4303/catalog.md#rfc4303-iseq-7) | later | — | — | — | level 5: a key management protocol |
| [RFC4303-ISEQ-8](../../standard/rfc4303/catalog.md#rfc4303-iseq-8) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-9](../../standard/rfc4303/catalog.md#rfc4303-iseq-9) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-10](../../standard/rfc4303/catalog.md#rfc4303-iseq-10) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-11](../../standard/rfc4303/catalog.md#rfc4303-iseq-11) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-12](../../standard/rfc4303/catalog.md#rfc4303-iseq-12) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-13](../../standard/rfc4303/catalog.md#rfc4303-iseq-13) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-14](../../standard/rfc4303/catalog.md#rfc4303-iseq-14) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-15](../../standard/rfc4303/catalog.md#rfc4303-iseq-15) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-16](../../standard/rfc4303/catalog.md#rfc4303-iseq-16) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-17](../../standard/rfc4303/catalog.md#rfc4303-iseq-17) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-18](../../standard/rfc4303/catalog.md#rfc4303-iseq-18) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-ISEQ-19](../../standard/rfc4303/catalog.md#rfc4303-iseq-19) | no check | — | — | — | not claimed: the counter of an SA is 32 bits, with no extended sequence number |
| [RFC4303-ISEQ-20](../../standard/rfc4303/catalog.md#rfc4303-iseq-20) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-21](../../standard/rfc4303/catalog.md#rfc4303-iseq-21) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-22](../../standard/rfc4303/catalog.md#rfc4303-iseq-22) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-23](../../standard/rfc4303/catalog.md#rfc4303-iseq-23) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-24](../../standard/rfc4303/catalog.md#rfc4303-iseq-24) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-ISEQ-25](../../standard/rfc4303/catalog.md#rfc4303-iseq-25) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-ISEQ-26](../../standard/rfc4303/catalog.md#rfc4303-iseq-26) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-27](../../standard/rfc4303/catalog.md#rfc4303-iseq-27) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-28](../../standard/rfc4303/catalog.md#rfc4303-iseq-28) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-29](../../standard/rfc4303/catalog.md#rfc4303-iseq-29) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-30](../../standard/rfc4303/catalog.md#rfc4303-iseq-30) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-ISEQ-31](../../standard/rfc4303/catalog.md#rfc4303-iseq-31) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-IICV-1](../../standard/rfc4303/catalog.md#rfc4303-iicv-1) | covered | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-IICV-2](../../standard/rfc4303/catalog.md#rfc4303-iicv-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-IICV-3](../../standard/rfc4303/catalog.md#rfc4303-iicv-3) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-IICV-4](../../standard/rfc4303/catalog.md#rfc4303-iicv-4) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-IICV-5](../../standard/rfc4303/catalog.md#rfc4303-iicv-5) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-IICV-6](../../standard/rfc4303/catalog.md#rfc4303-iicv-6) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-7](../../standard/rfc4303/catalog.md#rfc4303-iicv-7) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-8](../../standard/rfc4303/catalog.md#rfc4303-iicv-8) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-IICV-9](../../standard/rfc4303/catalog.md#rfc4303-iicv-9) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-10](../../standard/rfc4303/catalog.md#rfc4303-iicv-10) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-11](../../standard/rfc4303/catalog.md#rfc4303-iicv-11) | covered | [ESP padding](../../protocol/ipsec/checks/esp.md#esp-padding) | `Rfc4303EspPadding` | PASS |  |
| [RFC4303-IICV-12](../../standard/rfc4303/catalog.md#rfc4303-iicv-12) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-IICV-13](../../standard/rfc4303/catalog.md#rfc4303-iicv-13) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-IICV-14](../../standard/rfc4303/catalog.md#rfc4303-iicv-14) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-IICV-15](../../standard/rfc4303/catalog.md#rfc4303-iicv-15) | selected | [Tunnel mode between two hosts](../../protocol/ipsec/checks/modes.md#tunnel-mode-between-two-hosts) | `Rfc4301TunnelModeEsp` | FAIL (expected) | not reached: the test stopped at observation 3, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [RFC4303-IICV-16](../../standard/rfc4303/catalog.md#rfc4303-iicv-16) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-17](../../standard/rfc4303/catalog.md#rfc4303-iicv-17) | covered | [Traffic flow confidentiality padding](../../protocol/ipsec/checks/esp.md#traffic-flow-confidentiality-padding) | `Rfc4303TfcPadding` | PASS |  |
| [RFC4303-IICV-18](../../standard/rfc4303/catalog.md#rfc4303-iicv-18) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-IICV-19](../../standard/rfc4303/catalog.md#rfc4303-iicv-19) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-20](../../standard/rfc4303/catalog.md#rfc4303-iicv-20) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-21](../../standard/rfc4303/catalog.md#rfc4303-iicv-21) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-22](../../standard/rfc4303/catalog.md#rfc4303-iicv-22) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-IICV-23](../../standard/rfc4303/catalog.md#rfc4303-iicv-23) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-IICV-24](../../standard/rfc4303/catalog.md#rfc4303-iicv-24) | later | — | — | — | level 4: state inside a node, or an audit log |
| [RFC4303-IICV-25](../../standard/rfc4303/catalog.md#rfc4303-iicv-25) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-26](../../standard/rfc4303/catalog.md#rfc4303-iicv-26) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-IICV-27](../../standard/rfc4303/catalog.md#rfc4303-iicv-27) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-CONF-1](../../standard/rfc4303/catalog.md#rfc4303-conf-1) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-CONF-2](../../standard/rfc4303/catalog.md#rfc4303-conf-2) | selected | [ESP packet format in transport mode](../../protocol/ipsec/checks/esp.md#esp-packet-format-in-transport-mode) | `Rfc4303EspFormat`, `Rfc4303EspFormatIpv6` | PASS |  |
| [RFC4303-CONF-3](../../standard/rfc4303/catalog.md#rfc4303-conf-3) | no check | — | — | — | not claimed: multicast bypasses IPsec (`IPsec.ned:41`) |
| [RFC4303-CONF-4](../../standard/rfc4303/catalog.md#rfc4303-conf-4) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-CONF-5](../../standard/rfc4303/catalog.md#rfc4303-conf-5) | later | — | — | — | level 3: a crafted, corrupted or replayed packet |
| [RFC4303-CONF-6](../../standard/rfc4303/catalog.md#rfc4303-conf-6) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-CONF-7](../../standard/rfc4303/catalog.md#rfc4303-conf-7) | later | — | — | — | level 5: the algorithm itself (RFC 8221 and the algorithm documents) |
| [RFC4303-CONF-8](../../standard/rfc4303/catalog.md#rfc4303-conf-8) | selected | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-CONF-9](../../standard/rfc4303/catalog.md#rfc4303-conf-9) | covered | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-CONF-10](../../standard/rfc4303/catalog.md#rfc4303-conf-10) | selected | [ESP services](../../protocol/ipsec/checks/esp.md#esp-services) | `Rfc4303EspServices` | PASS |  |
| [RFC4303-CONF-11](../../standard/rfc4303/catalog.md#rfc4303-conf-11) | selected | [An ESP SA without encryption and without integrity](../../protocol/ipsec/checks/esp.md#an-esp-sa-without-encryption-and-without-integrity) | `Rfc4303EspBothNull` | PASS |  |

## The coverage debt: the checks this pass owes

Six statements are `owed`: the model claims the behavior, and no check of this pass reaches it.
All six need the same thing, a packet with an IPv6 extension header before the IPsec header, and
one later level 2 check serves them all. The model says that it inserts AH and ESP "after the
base header" (`IPsec.ned:44-46`), so the check would show where they go when a Hop-by-Hop or a
Destination Options header is there, and which headers the selectors skip to find the next
layer protocol.

| Statement | Strength | What a check needs |
| --- | --- | --- |
| [RFC4301-SA-36](../../standard/rfc4301/catalog.md#rfc4301-sa-36) | may (lower case) | a packet with a Hop-by-Hop or Destination Options header |
| [RFC4301-SEL-9](../../standard/rfc4301/catalog.md#rfc4301-sel-9) | should | a packet with a Hop-by-Hop or Destination Options header |
| [RFC4301-SEL-10](../../standard/rfc4301/catalog.md#rfc4301-sel-10) | should | a packet with a Hop-by-Hop or Destination Options header |
| [RFC4301-SEL-11](../../standard/rfc4301/catalog.md#rfc4301-sel-11) | description | a packet with a Hop-by-Hop or Destination Options header |
| [RFC4302-LOC-6](../../standard/rfc4302/catalog.md#rfc4302-loc-6) | description | a packet with a Hop-by-Hop or Destination Options header |
| [RFC4303-LOC-7](../../standard/rfc4303/catalog.md#rfc4303-loc-7) | description | a packet with a Hop-by-Hop or Destination Options header |

The `later` statements are not debt of this level: 260 need a crafted or replayed packet, a
counter near 2^32, the state of a node, a key management protocol or the algorithm itself.
The 206 `no check` statements are behaviors that the model does not claim — multicast, a
gateway and tunnel mode beyond one host pair, extended sequence numbers, nested SAs, the ICMP
report of a discard, more than one SPD, a change of the SPD at run time, the ICV over options —
and permissions that no observation can fail.

## Feature support

The rule of the guide: `supported` when every core statement passed, `partial` when some passed
and some failed or have no check, `not supported` when every core statement that ran failed,
`untested` when no core statement has a check.

| Feature | Level | Support | Why |
| --- | --- | --- | --- |
| [IPSEC-F-ARCHITECTURE](../../protocol/ipsec/features.md#ipsec-f-architecture) | mandatory | supported | every core statement passed |
| [IPSEC-F-SECURITY-ASSOCIATION](../../protocol/ipsec/features.md#ipsec-f-security-association) | mandatory | supported | every core statement passed |
| [IPSEC-F-SPD-PROCESSING](../../protocol/ipsec/features.md#ipsec-f-spd-processing) | mandatory | supported | every core statement passed |
| [IPSEC-F-SPD-MANAGEMENT](../../protocol/ipsec/features.md#ipsec-f-spd-management) | mandatory | supported | every core statement passed |
| [IPSEC-F-SELECTORS](../../protocol/ipsec/features.md#ipsec-f-selectors) | mandatory | partial | 11 core statements passed, 0 failed, 2 have no check |
| [IPSEC-F-NAMED-SPD-ENTRIES](../../protocol/ipsec/features.md#ipsec-f-named-spd-entries) | mandatory | untested | no core statement has a check: later |
| [IPSEC-F-SA-CREATION](../../protocol/ipsec/features.md#ipsec-f-sa-creation) | mandatory | not supported | every core statement that ran failed (6) |
| [IPSEC-F-SA-LOOKUP](../../protocol/ipsec/features.md#ipsec-f-sa-lookup) | mandatory | supported | every core statement passed |
| [IPSEC-F-INBOUND-SELECTOR-CHECK](../../protocol/ipsec/features.md#ipsec-f-inbound-selector-check) | mandatory | partial | 1 core statements passed, 3 failed |
| [IPSEC-F-TRANSPORT-MODE](../../protocol/ipsec/features.md#ipsec-f-transport-mode) | mandatory | partial | 14 core statements passed, 1 failed |
| [IPSEC-F-TUNNEL-MODE](../../protocol/ipsec/features.md#ipsec-f-tunnel-mode) | mandatory | not supported | every core statement that ran failed (17); 3 have no check |
| [IPSEC-F-PARALLEL-SAS](../../protocol/ipsec/features.md#ipsec-f-parallel-sas) | mandatory | not supported | every core statement that ran failed (3) |
| [IPSEC-F-SA-COMBINATION](../../protocol/ipsec/features.md#ipsec-f-sa-combination) | optional | not supported | every core statement that ran failed (2) |
| [IPSEC-F-FRAGMENTATION](../../protocol/ipsec/features.md#ipsec-f-fragmentation) | mandatory | partial | 9 core statements passed, 0 failed, 2 have no check |
| [IPSEC-F-PATH-MTU](../../protocol/ipsec/features.md#ipsec-f-path-mtu) | mandatory | not supported | every core statement that ran failed (3) |
| [IPSEC-F-OUTBOUND-DISCARD-REPORT](../../protocol/ipsec/features.md#ipsec-f-outbound-discard-report) | optional | untested | no core statement has a check: no check |
| [IPSEC-F-AUDIT](../../protocol/ipsec/features.md#ipsec-f-audit) | unstated | untested | no core statement has a check: later |
| [IPSEC-F-AH-FORMAT](../../protocol/ipsec/features.md#ipsec-f-ah-format) | optional | partial | 5 core statements passed, 5 failed |
| [IPSEC-F-AH-INTEGRITY](../../protocol/ipsec/features.md#ipsec-f-ah-integrity) | optional | partial | 7 core statements passed, 0 failed, 2 have no check |
| [IPSEC-F-ESP-FORMAT](../../protocol/ipsec/features.md#ipsec-f-esp-format) | mandatory | supported | every core statement passed |
| [IPSEC-F-ESP-PADDING](../../protocol/ipsec/features.md#ipsec-f-esp-padding) | mandatory | partial | 5 core statements passed, 1 failed |
| [IPSEC-F-ESP-SERVICES](../../protocol/ipsec/features.md#ipsec-f-esp-services) | mandatory | supported | every core statement passed |
| [IPSEC-F-ESP-PROCESSING](../../protocol/ipsec/features.md#ipsec-f-esp-processing) | mandatory | partial | 5 core statements passed, 0 failed, 4 have no check |
| [IPSEC-F-SEQUENCE-NUMBERS](../../protocol/ipsec/features.md#ipsec-f-sequence-numbers) | mandatory | partial | 6 core statements passed, 4 failed, 2 have no check |
| [IPSEC-F-ANTI-REPLAY](../../protocol/ipsec/features.md#ipsec-f-anti-replay) | mandatory | untested | no core statement has a check: later |
| [IPSEC-F-EXTENDED-SEQUENCE-NUMBERS](../../protocol/ipsec/features.md#ipsec-f-extended-sequence-numbers) | optional | untested | no core statement has a check: no check |
| [IPSEC-F-ESP-TFC-PADDING](../../protocol/ipsec/features.md#ipsec-f-esp-tfc-padding) | optional | supported | every core statement passed |
| [IPSEC-F-ESP-DUMMY-PACKETS](../../protocol/ipsec/features.md#ipsec-f-esp-dummy-packets) | mandatory | not supported | every core statement that ran failed (4) |
| [IPSEC-F-MULTICAST](../../protocol/ipsec/features.md#ipsec-f-multicast) | optional | untested | no core statement has a check: no check |

- **IPSEC-F-TRANSPORT-MODE is partial because of one statement**, RFC4301-SA-50: a host supports
  transport mode and tunnel mode, and the model has only the first ([gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode)).
  Every rule of transport mode itself passed.
- **IPSEC-F-AH-INTEGRITY is partial with no failure**, and its passes have no weight: the
  receiver never checks the ICV, and the two core statements of a failed ICV are level 3.
- **Four mandatory features are untested or not supported only because a statement needs a later
  level**: the named SPD entries and anti-replay have no core check at level 2, as
  [`checks.md`](../../protocol/ipsec/checks.md#statements-this-pass-wrote-no-check-for) says.

## Achieved level

**Level 2, reached, for the normal path.** Target: level 2, from
[`standards.md`](../../protocol/ipsec/standards.md#target-level).

| Half of the criterion | State | Evidence |
| --- | --- | --- |
| Every normal-path mandatory mechanism of the base documents appears as a feature | holds | the catalogs hold every normative statement of RFC 4301 §3.1 to §5.2 and §10, and of RFC 4302 and RFC 4303 §2 to §3.4 and §5, 797 entries; the feature map has 29 features and places every entry |
| Every mandatory feature has a core check that ran and has a verdict | holds for the normal path | 19 of the 21 mandatory features have core checks that ran: 31 tests, 15 PASS, 16 FAIL. The two others need a key management protocol (the named SPD entries) or a replayed packet (anti-replay); neither is a mechanism of the normal path of a manually keyed host |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | **reached** | the 6 `owed` statements are the debt of the level; see [the coverage debt](#the-coverage-debt-the-checks-this-pass-owes) |
| 3, Edge | not started | a failed ICV on AH and ESP (defects by the TODOs of `IPsec.cc:832, 885`), a fragment offered to AH or ESP, a non-zero Reserved field, wrong padding, a replayed packet, ICMP errors at the IPsec boundary |
| 4, Dynamics | not started | the anti-replay window, a counter near 2^32, the state of the SPD and the SAD, the audit log |
| 5, Complete | not started | key management, the algorithms of RFC 8221, and multicast, gateways and tunnel mode when the model claims them |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 4301, RFC 4302, RFC 4303 downloaded; the standards map; the claims of the model | no run; no obsolete claim; five stated refusals in the model's own words; one bare TODO (ICV verification) that is a claim, not a refusal |
| 2 | 2026-09-24 | **2, reached** | catalogs of RFC 4301 (370), RFC 4302 (190) and RFC 4303 (237); 29 features; 23 checks; 31 tests; the conformance matrix | 15 PASS, 16 FAIL, 6 of them declared expected; twelve gaps of the model: six defects, one untestable claim and five unimplemented features; 6 statements owed |

## Out of scope

The sections of the three documents outside the in-scope set are listed at the end of each
catalog: [RFC 4301](../../standard/rfc4301/catalog.md#out-of-scope-in-this-catalog),
[RFC 4302](../../standard/rfc4302/catalog.md#out-of-scope-in-this-catalog) and
[RFC 4303](../../standard/rfc4303/catalog.md#out-of-scope-in-this-catalog).
