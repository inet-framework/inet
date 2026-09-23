# IPv4 — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/ipv4/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/ipv4/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan: 2026-09-09, worktree at commit `da7ac0d5bf`. `Ipv4.ned`, `Icmp.ned` and the
  IPv4 tree were unchanged since the scan of pass 2, and the scan gave the same result. The
  scan was extended to `RFC 1122`, `RFC1122`, `RFC 6864` and `RFC6864` over
  `src/inet/networklayer/` and `src/inet/transportlayer/udp/`: no line names either. The
  claim scan did not run again on 2026-09-23.
- Support values, from this run:
  - Date: 2026-09-23 18:26 +0200
  - INET: branch `topic/standards-tests-wave0`, commit `28536bd0a5` (on `master`), tree clean
  - Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
  - OMNeT++: 6.4.0, commit `cf58891643`
  - Build: debug, built from this commit
  - Compiler: Ubuntu clang version 23.0.0
  - Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
  - Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/ipv4$'`
- Ledger state: [`coverage.md`](coverage.md#feature-support), from the run above. Three
  commits repaired four defects since the pass that first found them: `f77a73d5a1`,
  `37ddbfa7a8` and `40c9f04e21`.

This is the one document of the workflow whose first part reads the model documentation on
purpose. The claims must not travel back into the catalogs, the feature map, or the check
descriptions.

## Part 1 — what the model claims

Part 1 was read on 2026-09-09 and this refresh did not read the claims again.

### The claims of the active modules

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [Ipv4.ned:30-31](../../../../../src/inet/networklayer/ipv4/Ipv4.ned#L30-L31) | "Implements the IPv4 protocol. The protocol header is represented by the ~Ipv4Header message class." | protocol claim, no document |
| [Icmp.ned:12](../../../../../src/inet/networklayer/ipv4/Icmp.ned#L12) | "ICMP implementation." | protocol claim, no document |

These two comments are the whole module-level claim. Neither names a document, a number, or
a year. The only two document references in the tree sit in the BSD-derived definition
headers `headers/ip.h:44` and `headers/ip_icmp.h:44`, which the simulation does not use for
the datagram (see the pass 2 scan).

### How this pass reads the claim

The reading of pass 2 stands: "Implements the IPv4 protocol" is an implicit claim on the
Internet Standard for IPv4, and "ICMP implementation" on RFC 792. Two documents entered the
in-scope set since, and the reading has to say where each stands:

- **RFC 1122** is the Internet Standard for hosts (STD 3). A host model that claims IPv4
  claims the behavior of an IPv4 host, and the host requirements are where that behavior is
  written down with keywords. This pass reads RFC 1122 §3 as **implicitly claimed**, on the
  same footing as RFC 791.
- **RFC 6864** is a Proposed Standard of 2013 that changes the identification rule. A model
  that names no document and predates the RFC cannot be read as claiming it. This pass
  reads RFC 6864 as **not claimed**. The consequence is visible in the matrix: two features
  whose governing statements are RFC 6864 keywords read `undocumented`, which means that the
  model does what the newer document asks without saying so.

The missing version statement is not lost by either choice: it is the first finding below,
and it now has a price.

## Part 2 — conformance matrix

The verdict combines the claim on the governing source document, the support value of the
ledger, and the level of the feature, by the table of step 8.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| IPV4-F-DELIVERY | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-HEADER | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-TTL | mandatory | yes (implicit, RFC 791 and RFC 1122) | supported | **confirmed** |
| IPV4-F-FRAGMENTATION | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-REASSEMBLY | mandatory | yes (implicit, RFC 1122) | supported | **confirmed** |
| IPV4-F-DONT-FRAGMENT | mandatory | no (RFC 6864 governs; the RFC 791 base text is claimed) | supported | **undocumented** |
| IPV4-F-IDENTIFICATION | mandatory | no (RFC 6864 governs) | supported | **undocumented** |
| IPV4-F-HEADER-CHECKSUM | mandatory | yes (implicit, RFC 791 and RFC 1122) | supported | **confirmed** |
| IPV4-F-MIN-SIZE | mandatory | yes (implicit, RFC 791 and RFC 1122) | supported | **confirmed** |
| IPV4-F-ERROR-REPORT | mandatory for a host, optional for a gateway | yes (implicit, RFC 792 and RFC 1122) | supported | **confirmed** |
| IPV4-F-INPUT-VALIDATION | mandatory | yes (implicit, RFC 1122) | partial | **partial** — RFC1122-VER-1, ADDR-3: two of five silent-discard rules fail |
| IPV4-F-ERROR-SUPPRESSION | mandatory | yes (implicit, RFC 1122) | supported | **confirmed** |
| IPV4-F-ERROR-DELIVERY | mandatory | yes (implicit, RFC 1122) | untested | **unverified** — every core statement is internal; module tests |

Nine features `confirmed`, one `partial`, two `undocumented`, one `unverified`. No
`defect` in the strict sense of the table: the one partial feature also has a core check
that passed, so the table says `partial` and lists the gap. No `declined`. Three verdicts
moved from `partial` to `confirmed` since the pass that wrote them — IPV4-F-REASSEMBLY,
IPV4-F-HEADER-CHECKSUM, IPV4-F-ERROR-SUPPRESSION — repaired by `f77a73d5a1` and
`37ddbfa7a8`.

## Findings

### 1. The model states no version for either protocol, and it now costs two verdicts

No active module names RFC 791, RFC 792, RFC 1122 or RFC 6864. At level 2 this cost
nothing. At level 3 the identification and don't-fragment features are governed by RFC 6864
keywords, the model satisfies every RFC 6864 check that ran, and the matrix still cannot say
`confirmed`, because nothing in the model claims the document. A sentence in `Ipv4.ned` in
the house style of `Igmpv2.ned` ("implements ... as specified in RFC 2236") would turn both
`undocumented` rows into `confirmed` and every `implicit` into an explicit claim.

### 2. Input validation: two of the five silent-discard rules fail, down from four

RFC 1122 §3.2.1 and §3.2.2 require a host to discard, silently, a datagram with a wrong
version, a bad checksum, or an invalid source address, and an ICMP message of unknown type.
The model discards a datagram that is not addressed to it, and now also a bad checksum and
an unknown ICMP type. Two gaps remain, with the code in
[`results.md`](results.md#model-analysis--where-inet-implements-the-checked-behavior):

- the version field is never read on receipt;
- the source address is never validated, at the network layer or at UDP.

Two gaps are repaired since the pass that found them: the header checksum guard, by
`f77a73d5a1`; the unknown ICMP type, by `40c9f04e21`. The unknown-type gap was more than a
conformance gap: it was a robustness problem for any scenario that carries crafted,
external or fuzzed traffic. Within a simulation, the source address gap left is the same
kind of shortcut as the declared checksum mode: the model trusts its own senders.

### 3. A report about a link-layer broadcast — repaired by `37ddbfa7a8`

RFC 1122 §3.2.2 forbids an ICMP error about a datagram that arrived as a link-layer
broadcast, and its implementation note says why the rule is separate from the IP-broadcast
rule. The model's ICMP decided from the IP addresses only, and a unicast datagram inside a
broadcast frame, to a closed port, was answered with Destination Unreachable.
`Icmp::maySendErrorMessage` now also reads the `MacAddressInd` tag of the arriving frame,
and the report is suppressed.

### 4. The reassembly key had three fields — repaired by `f77a73d5a1`

RFC 791 §2.3 combines fragments that agree in identification, source, destination and
protocol. The model's buffer key omitted the protocol, so two datagrams of different
protocols with the same identification shared a buffer; in the check, the echo request was
reassembled out of the UDP datagram's buffer and the UDP datagram was lost. The key carries
the protocol too now. The exposure was small in the model's own traffic, whose
identifications come from one counter for all protocols; it was real for forged or wrapped
identifications.

### 5. The handoff of ICMP errors to the transport layer is unverified

IPV4-F-ERROR-DELIVERY is mandatory and untested, because every one of its statements is
about a handoff inside the host. The model has the mechanism (an indication with the quoted
datagram travels from ICMP through IPv4 to the transport protocol), and no protocol test
can see it. This is the headline for a module-test pass, not for the next protocol pass.

### 6. A should the model declines

RFC 1122 §3.3.3 says a host should send at most 576 octets to an off-net destination unless
it knows the path MTU. The model sends the datagram whole and lets the gateway fragment it.
Recorded as `declined` in the ledger; a should, not a defect.

## What this document does not establish

- It says nothing about RFC 1812: the gateway of the mockup is judged by RFC 791 and RFC 792
  alone, whose error reports are a `may`.
- It says nothing about the optional statements that no check selected (local
  fragmentation, Parameter Problem), about the reassembly timer (level 4), or about the
  options and the type of service (level 5).
- A `confirmed` verdict means the checks of the feature passed. It does not mean the
  feature is complete; the ledger records the reach of every check.
