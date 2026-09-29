# RIP — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-24 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/rip/standards.md), [features.md](../../protocol/rip/features.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow. Part 1 records what the model intends: which
standards it claims to implement, mapped onto the standards map. Part 2 crosses the claims
with the feature support of the ledger, feature by feature.

Run record of the ledger state that part 2 comes from:

- Date: 2026-09-29 17:29 +0200
- INET: branch `master`, commit `24675c3a37`, tree clean
- Trees: src `8b4f86968e`, tests/protocol `1f1d62beca`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/rip$'`

Part 1 was first read by the level 1 pass, at src `16dc528e10`. The RIP code is the same in
both trees; the two trees differ only in seven files of IEEE 802.11 and one line of
`src/inet/common/InitStages.cc`.

## Part 1 — the claims

Every place in the model that names a standard of the RIP family:

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 2453, RFC 2080 | [`Rip.ned:16-17`](../../../../../src/inet/routing/rip/Rip.ned) | "Implements distance vector routing as specified in RFC 2453 (RIPv2) and RFC 2080 (RIPng)." This is the documentation comment of the module, which the module reference publishes. |
| RFC 2453, RFC 2080 | [`Rip.ned:34`](../../../../../src/inet/routing/rip/Rip.ned) | The `mode` parameter: "either "RIPv2" (RFC 2453) or "RIPng" (RFC 2080)". |
| RFC 2453, RFC 2080 | [`ch-routing.rst:40-41`](../../../../../doc/src/users-guide/ch-routing.rst) | The user's guide repeats the claim of the NED documentation. |
| RFC 2453, RFC 2080 | [`Rip.h:60`](../../../../../src/inet/routing/rip/Rip.h) | "This module supports RIPv2 (RFC 2453) and RIPng (RFC 2080)." |
| RFC 2453 §3.7 | [`Rip.h:77-78`](../../../../../src/inet/routing/rip/Rip.h) | A TODO: "There is no merging of subnet routes. RFC 2453 3.7 suggests that subnetted network routes should not be advertised outside the subnetted network." |
| RFC 2453 §3.9.1 | [`Rip.cc:457`](../../../../../src/inet/routing/rip/Rip.cc) | "The request processing follows the guidelines described in RFC 2453 3.9.1." |
| RFC 2453 §3.9.2 | [`Rip.cc:793`, `Rip.cc:829`](../../../../../src/inet/routing/rip/Rip.cc) | Two comments that quote the response processing of §3.9.2. |
| RFC 2453 §3.6, §4 | [`RipPacket.msg:39`](../../../../../src/inet/routing/rip/RipPacket.msg) | "see RFC 2453 3.6 and 4", above the route entry. |
| none (a refusal) | [`RipPacket.msg:33`, `RipPacket.msg:55`](../../../../../src/inet/routing/rip/RipPacket.msg) | The authentication family `0xFFFF` is commented out, and the packet says "note: Authentication entry is not allowed". |
| RFC 2453 | [`RipPacketSerializer.cc:29`](../../../../../src/inet/routing/rip/RipPacketSerializer.cc) | The "must be zero" field of the header. |
| RFC 1058, RFC 2080 | [`RipPacketSerializer.cc:15-18`](../../../../../src/inet/routing/rip/RipPacketSerializer.cc) | A TODO: "The inet::Rip uses RipPacket and RipEntry for IPv4 (RIPv2, see RFC 1058) and for IPv6 (RIPng, see RFC 2080). The serializer accepts only RFC1058 packets with IPv4 addresses." |

The documentation comment also states three limits (`Rip.ned:27-30`): the hop-count metric
only, a diameter below 16, and the "counting to infinity" recovery. These are the limits of
the protocol itself, as RFC 2453 §3.2 states them, and not limits of the model.

Mapped onto the standards map, [`standards.md`](../../protocol/rip/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 2453 | yes, `base` | **yes**, in the published documentation and at the level of clauses | The module documentation, the user's guide, and comments that name §3.6, §3.7, §3.9.1 and §3.9.2. |
| RFC 2080 | yes, `base` | **yes**, in the published documentation | No comment names a clause of RFC 2080. |
| RFC 1058 | no, Historic | named, for the IPv4 wire format | **An obsolete claim.** The serializer comment says "RIPv2, see RFC 1058" and "accepts only RFC1058 packets". RFC 1058 is RIP version 1. The layout that the serializer writes — a route tag, a subnet mask and a next hop in each entry — is the version 2 layout of RFC 2453 §4. |
| RFC 1723 | no, obsoleted | no | — |
| RFC 4822, RFC 2453 §4.1 | no, level 5 | **declined** | The packet definition refuses the authentication entry in words. A stated refusal is not a claim. |
| RFC 2453 §5, §6 | no, level 5 | no | Nothing names the compatibility switch or RIP version 1. |
| RFC 2091 | no, level 5 | no | — |

### The claims that the code makes

The guide counts code as a claim: an effort for a behavior claims it. The level 2 pass read
the code only to decide the status of each statement in the ledger, and these are the claims
it found beyond the named documents. None enters part 2 on its own, because part 2 works on
documents, not on lines of code; they decide the `owed`, `later` and `no check` rows of
[`coverage.md`](coverage.md#statement-coverage).

| Behavior | Code | What it decides |
| --- | --- | --- |
| the validation of a response: port, own address, neighbor or link-local source, hop limit, entries | [`Rip.cc:724-790`](../../../../../src/inet/routing/rip/Rip.cc) | RIP-F-RESPONSE-VALIDATION is claimed; its statements are `later`, level 3 |
| the specific request | [`Rip.cc:474-523`](../../../../../src/inet/routing/rip/Rip.cc) | RIP-F-SPECIFIC-QUERY is claimed, level 3 |
| the route tag kept and advertised | [`Rip.cc:587`, `Rip.cc:850`](../../../../../src/inet/routing/rip/Rip.cc) | RFC2453-TAG-1 and RFC2080-TAG-1 are claimed, level 3 |
| the route tag and the metric of an import, parameters that every caller leaves at the default | [`Rip.cc:266`](../../../../../src/inet/routing/rip/Rip.cc) | TAG-2 of both documents and RFC2080-ADDR-3 are claimed, `owed` |
| the switch to an equal route at half the timeout, a bare TODO | [`Rip.cc:712`](../../../../../src/inet/routing/rip/Rip.cc) | RFC2453-RESP-11 and RFC2080-RESP-13 are claimed, `owed` |
| the suppression of a triggered update before a periodic one, and the clearing of the change flags | [`Rip.cc:895`, `Rip.cc:452`](../../../../../src/inet/routing/rip/Rip.cc) | TRIG-4 and TRIG-8 of RFC 2453, TRIG-3 and TRIG-7 of RFC 2080 are claimed, `owed` |
| the import of a default route | [`Rip.cc:200-201`](../../../../../src/inet/routing/rip/Rip.cc) | RIP-F-DEFAULT-ROUTE is claimed, `owed` |
| no version 1: no code reads the version of a message, and every message has version 2 | [`RipPacket.msg:60`](../../../../../src/inet/routing/rip/RipPacket.msg) | RFC2453-MASK-2 and QRY-1 are not claimed, `no check`; RFC2453-ADDR-2, ADDR-3 and ADDR-4 are claimed by the TODO of `Rip.h:77-78` and wait for level 5 |

## Part 2 — the conformance matrix

A feature is `claimed` when the claims of part 1 cover its governing source document. The
model claims RFC 2453 and RFC 2080 as wholes, so every feature of the map is claimed. The
support comes from [`coverage.md`](coverage.md#feature-support).

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [RIP-F-MESSAGE-FORMAT](../../protocol/rip/features.md#rip-f-message-format) | mandatory | yes | partial | `partial` — RFC2080-GEN-3: RIPng carries version 2 |
| [RIP-F-TRANSPORT](../../protocol/rip/features.md#rip-f-transport) | mandatory | yes | partial | `partial` — RFC2080-MSG-4: the RIPng answer to a request never arrives |
| [RIP-F-UPDATE-ADDRESSING](../../protocol/rip/features.md#rip-f-update-addressing) | mandatory | yes | supported | `confirmed` |
| [RIP-F-METRIC](../../protocol/rip/features.md#rip-f-metric) | mandatory | yes | supported | `confirmed` |
| [RIP-F-DESTINATION-PREFIX](../../protocol/rip/features.md#rip-f-destination-prefix) | mandatory | yes | supported | `confirmed` |
| [RIP-F-PERIODIC-UPDATE](../../protocol/rip/features.md#rip-f-periodic-update) | mandatory | yes | supported | `confirmed` |
| [RIP-F-ROUTE-LEARNING](../../protocol/rip/features.md#rip-f-route-learning) | mandatory | yes | supported | `confirmed` |
| [RIP-F-SPLIT-HORIZON](../../protocol/rip/features.md#rip-f-split-horizon) | mandatory | yes | supported | `confirmed` |
| [RIP-F-TRIGGERED-UPDATE](../../protocol/rip/features.md#rip-f-triggered-update) | mandatory | yes | partial | `partial` — RFC2080-TRIG-8: the triggered update of RIPng carries version 2 |
| [RIP-F-ROUTE-EXPIRY](../../protocol/rip/features.md#rip-f-route-expiry) | mandatory | yes | partial | `partial` — TIMER-4, TIMER-5, TIMER-6 of both documents fail and RFC2080-TIMER-7 is not reached; only RFC2453-TIMER-7 holds |
| [RIP-F-RESPONSE-CONTENTS](../../protocol/rip/features.md#rip-f-response-contents) | mandatory | yes | supported | `confirmed` |
| [RIP-F-RESPONSE-VALIDATION](../../protocol/rip/features.md#rip-f-response-validation) | mandatory | yes | untested | `unverified` — level 3 |
| [RIP-F-TABLE-REQUEST](../../protocol/rip/features.md#rip-f-table-request) | unstated | yes | partial | `partial` — RFC2080-REQ-4, REQ-6, OUT-1: the RIPng answer never arrives |
| [RIP-F-SPECIFIC-QUERY](../../protocol/rip/features.md#rip-f-specific-query) | unstated | yes | untested | `unverified` — level 3 |
| [RIP-F-NEXT-HOP](../../protocol/rip/features.md#rip-f-next-hop) | optional | yes | partial | `partial` — the RIPng next hop entry, level 3 |
| [RIP-F-ROUTE-TAG](../../protocol/rip/features.md#rip-f-route-tag) | mandatory | yes | untested | `unverified` — level 3 |
| [RIP-F-DEFAULT-ROUTE](../../protocol/rip/features.md#rip-f-default-route) | unstated | yes | untested | `unverified` — owed at level 2 |
| [RIP-F-HOST-ROUTES](../../protocol/rip/features.md#rip-f-host-routes) | optional | yes | untested | `unverified` — owed at level 2 |
| [RIP-F-VERSION-1-INTERWORKING](../../protocol/rip/features.md#rip-f-version-1-interworking) | mandatory | yes | untested | `unverified` — level 5; the code makes no effort for version 1 beyond the TODO of `Rip.h:77-78` |

Seven features are `confirmed`, six `partial` and six `unverified`. No feature is a `defect`
in the sense of the matrix: every mandatory feature that failed a check passed another one.
The statement-level defects sit inside features that otherwise work, which is the case the
guide describes; [`results.md`](results.md#the-model-gaps) holds all six.

### How to read the matrix

- **RIP version 2 alone** would read differently: of the six `partial` verdicts, only
  RIP-F-ROUTE-EXPIRY holds a failure of RFC 2453. The other five `partial` verdicts are RIPng
  failures, gaps 1 to 3, and the RIPng next hop entry of level 3.
- **RIP-F-ROUTE-EXPIRY** is the feature where both documents fail, and the reason is the same
  code: the timeout is looked at only when an update goes out (gap 4), a network the router
  itself lost is never purged (gap 5), and a learned route is purged 300 s after the last
  entry from its next hop, not 120 s after the deletion (gap 6). Together, gaps 5 and 6 keep
  every withdrawn network in every update of the RIP domain for good.
- **The `unverified` verdicts** are the level 3 work and the owed checks, not verdicts on the
  model.

## Headlines for the next pass

1. **Two mandatory features are `unverified` and claimed in code**: RIP-F-RESPONSE-VALIDATION
   and RIP-F-ROUTE-TAG. Level 3 reaches both with crafted responses.
2. **The six gaps** of [`results.md`](results.md#the-model-gaps) are repairs, not tests: none is
   declared, so the suite stays red until they are repaired. After a repair of gap 3, the
   check of RFC2080-TIMER-7 reaches its own observation for the first time.
3. **The obsolete claim of level 1 stands**: the serializer names RFC 1058 for the version 2
   layout.
