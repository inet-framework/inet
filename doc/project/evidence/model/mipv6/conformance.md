# Mobile IPv6 — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/mipv6/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a document of the Mobile IPv6 family (RFC 6275, RFC 3775,
RFC 4877, RFC 5095, RFC 4283). `src/inet/networklayer/xmipv6/` no longer exists: its last commit,
`8ca5680904`, renamed the module `xMIPv6` to `Mipv6` and flattened the `xMIPv6Support` wrapper
into `Ipv6NetworkLayer`; the live code is entirely under `src/inet/networklayer/mipv6/`.

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 3775 | [`Mipv6.h:60`](../../../../../src/inet/networklayer/mipv6/Mipv6.h) | "Implements RFC 3775 Mobility Support in Ipv6." The class-level documentation comment of the `Mipv6` module. |
| RFC 3775 | [`Mipv6.ned:17`](../../../../../src/inet/networklayer/mipv6/Mipv6.ned) | "Implements Mobile IPv6 (RFC 3775): the mobility signalling and binding management logic..." The NED documentation comment, which the module reference publishes. |
| RFC 3775 | [`Mipv6.ned:35`](../../../../../src/inet/networklayer/mipv6/Mipv6.ned) | The `useRouteOptimization` parameter comment: "whether a mobile node route-optimizes with correspondent nodes (RFC 3775)". |
| RFC 3775 | [`BindingCache.ned:17`](../../../../../src/inet/networklayer/mipv6/BindingCache.ned) | "Holds the Binding Cache of a home agent or correspondent node (RFC 3775)..." |
| RFC 3775 | [`BindingUpdateList.ned:17`](../../../../../src/inet/networklayer/mipv6/BindingUpdateList.ned) | "Holds the Binding Update List of a mobile node (RFC 3775)..." |
| RFC 3775 | [`HomeAgent6.ned:23`](../../../../../src/inet/node/mipv6/HomeAgent6.ned) | "MIPv6 routers need faster Router Advertisements... (RFC 3775 Section 7.5)." |
| RFC 3775 | [`Mipv6.cc:288,920,1874,1910,2350`](../../../../../src/inet/networklayer/mipv6/Mipv6.cc) | Five source comments that cite RFC 3775 by section: §11.6.1 (care-of address reuse), §9.5.1 (Binding Update validation), the segments-left decrement rule, §6.4 (Home Address destination option processing), and §10.4.5 (verification of a reverse-tunneled datagram). |
| RFC 3775 | [`ch-ipv6.rst:46`](../../../../../doc/src/users-guide/ch-ipv6.rst) | "The network layer can optionally include Mobile IPv6 (RFC 3775); see the `Mobile IPv6`_ section." |
| RFC 3775 | [`ch-ipv6.rst:181`](../../../../../doc/src/users-guide/ch-ipv6.rst) | "Mobile IPv6 (MIPv6, RFC 3775) lets a node remain reachable under a stable address while it moves..." The users-guide chapter that introduces the protocol. |
| RFC 6275 | [`MobilityHeaderSerializer.h:17`](../../../../../src/inet/networklayer/mipv6/MobilityHeaderSerializer.h) | "MIPv6 Mobility Header as defined in RFC 6275 Section 6.1." |
| RFC 6275 | [`MobilityHeaderSerializer.cc:17,36,81,114-205`](../../../../../src/inet/networklayer/mipv6/MobilityHeaderSerializer.cc) | Section-numbered comments through the whole wire-format encoder and decoder: §6.1.1 to §6.1.9, one per message type. |
| RFC 6275 | [`MobilityHeader.msg:40,104,142,148,149,156,162,163`](../../../../../src/inet/networklayer/mipv6/MobilityHeader.msg) | Field comments on the lifetime and cookie/token fields: "serialized in RFC 6275 units of 4 seconds", "8 octets on the wire (RFC 6275)". |
| RFC 4283 | [`MobilityHeader.msg:60`](../../../../../src/inet/networklayer/mipv6/MobilityHeader.msg) | The `mobileNodeIdentifier` field: "Mobile Node Identifier option (NAI), RFC 4283". This field is shared with Proxy Mobile IPv6; see [`model/pmipv6/conformance.md`](../pmipv6/conformance.md) for the RFC 5213 fields of the same message classes. |

`MobilityHeader.msg` and `MobilityHeaderSerializer.*` also carry the Proxy Mobile IPv6 option
fields (`proxyRegistrationFlag`, `homeNetworkPrefix`, and the rest), cited to RFC 5213 throughout
— RFC 5213 is not of the Mobile IPv6 family, so those citations are not rows of this table; they
are mapped in [`model/pmipv6/conformance.md`](../pmipv6/conformance.md). `Mipv6.h:75` and
`Mipv6.cc:2104` cite RFC 2473 for the tunnel management that this module took over from the
former `Ipv6Tunneling` module; RFC 2473 is not of this family either, and that claim is mapped in
[`model/ipv6tunnel/conformance.md`](../ipv6tunnel/conformance.md).

Two more citations turned up in the same grep and are not claims of a document of any family:
`Mipv6.cc:526` cites "RFC 3041" inside a block comment that quotes RFC 6275 §11.7.1 verbatim (RFC
3041 is the old number for the IPv6 privacy-addresses extension, mentioned by the quoted clause
itself, not by the model); `Mipv6.cc:2058` cites "RFC 8200" for the extension-header length rule
inside an unrelated size computation. Neither is counted as a claim above.

Mapped onto the standards map, [`standards.md`](../../protocol/mipv6/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 6275 | yes, `base` | **yes**, but only in the wire-format layer | `MobilityHeaderSerializer.*` cites it by section number throughout. No NED documentation comment, no class comment, and neither users-guide citation names it. |
| RFC 3775 | no, obsoleted | **yes**, in every place a reader looks first | The `Mipv6` class comment, the `Mipv6.ned` module comment, both data-store modules' comments, `HomeAgent6.ned`, five comments in `Mipv6.cc`, and both users-guide citations. **This is the headline of level 1: the behavioral half of the model claims an obsolete document, while its own serializer already claims the current one — one module, two RFC numbers, split exactly along the same line as its two halves.** |
| RFC 4877 | no, level 5 | no | Zero citations anywhere in `src/inet/networklayer/mipv6/` or `src/inet/node/mipv6/`. |
| RFC 5095 | no, out of the register family | no | Zero citations. |
| RFC 4283 | no, level 2/5 companion | yes, once, for the field it defines | `MobilityHeader.msg:60`, exactly where the field is declared. |

**The finding of the survey.** The model claims Mobile IPv6 twice, under two different RFC
numbers, in the same code area (5,360 non-generated lines under
`src/inet/networklayer/mipv6/`). Every comment a reader meets first — the class
doc of `Mipv6.h`, the NED doc of `Mipv6.ned`, the two data-store modules, the home-agent node
type, and both mentions in the users guide — names RFC 3775, obsoleted by RFC 6275 since 2011.
The one place that already claims RFC 6275 is the part of the module a behavioral reader never
opens: the serializer, added later (2024) and cited section by section against the current
document. RFC 4283 is claimed exactly once, at the one field it defines, and nowhere else; RFC
4877 and RFC 5095 are not claimed at all, which matches their absence from the register's
relations to RFC 6275.

For the level 2 pass, four facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. The return-routability cookie and token fields exist and are populated in every message, but
   the nonce input to `generateHomeToken`/`generateCareOfToken` is hardcoded to `0` at all four
   call sites (`Mipv6.cc:982,983,1546,1567`), each marked `// TODO nonce`. The message sequence,
   the field presence, and the cookie echo are real; the cryptographic content behind them is
   not.
2. `MobilityHeader.msg:46,106,136` mark "Mobility Options not defined" for `BindingUpdate`,
   `BindingAcknowledgement` and `BindingError`: no field exists for Pad1, PadN, Binding Refresh
   Advice, Alternate Care-of Address or Nonce Indices (RFC 6275 §6.2). One option has a
   placeholder: `bindingAuthorizationData` (`MobilityHeader.msg:50,109`) is a plain `int`, not
   the MAC-bearing option format of §6.2.7 — a field that exists with a placeholder value, which
   the guide treats as a claim regardless of what a comment says.
3. `Mipv6.cc:1122` calls `ipv6nd->sendUnsolicitedNa(ie)` after a de-registration Binding
   Acknowledgement from the home agent, next to a comment that quotes RFC 6275 §11.5.4 verbatim
   (`Mipv6.cc:1116-1121`). This is the only Neighbor-Advertisement-related call found under
   `src/inet/networklayer/mipv6/`; a level 2 pass that builds a return-home check should read
   this call in full first, and not assume what it covers.
4. `Mipv6.cc:645`: "// TODO solve the HA DAD problem in a different way", next to a `sendTime`
   parameter that already delays a message send. This is a stated deferral without a stated
   reason — by the guide's table, a bare TODO is a claim, so a check that exercises this path and
   fails is a defect, not an unimplemented feature, unless the level 2 pass finds a reason
   elsewhere in the same function.
