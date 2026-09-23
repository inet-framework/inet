# RIP — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/rip/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `97f4559eee`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

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

**The finding of the survey.** The model claims the two current base documents, by number,
in the place that a reader looks first. One citation is obsolete: the serializer names RFC 1058 for a layout that RFC 2453
defines. Authentication is declined in words, so a level 5 pass declares its failures expected.

For the level 2 pass, three facts from the code (the guide permits this look to decide the
claim question, and the pass must check each one):

1. The serializer writes IPv4 addresses only (`toIpv4()` for the address, the mask and the next
   hop). A RIPng packet has no byte form, so every statement about the RIPng message format of
   RFC 2080 §2.1 meets a model that never encodes it.
2. The subnet-route rule of RFC 2453 §3.7 is a TODO without a reason. By the table of the guide,
   a bare TODO is a claim, so a check of §3.7 that fails is a defect and not an unimplemented
   feature.
3. The address family identifier of a received entry is not checked (`RipPacketSerializer.cc:60`,
   "TODO Valid addressFamilyId values: 0, 2, 0xFFFF").
