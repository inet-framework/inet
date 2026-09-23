# Proxy Mobile IPv6 — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/pmipv6/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a document of the Proxy Mobile IPv6 family (RFC 5213, RFC
4283 — see [`standards.md`](../../protocol/pmipv6/standards.md#document-list) for why RFC 4283
is in this family's in-scope set although it is out of scope for Mobile IPv6 itself). Two of the
files that carry these claims live under `src/inet/networklayer/mipv6/`, not
`src/inet/networklayer/pmipv6/`, because PMIPv6 reuses Mobile IPv6's message classes and
serializer; each row below says so.

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 5213 | [`Pmipv6.h:31`](../../../../../src/inet/networklayer/pmipv6/Pmipv6.h) | "Implements Proxy Mobile IPv6 (RFC 5213): network-based mobility management..." The class-level documentation comment. |
| RFC 5213 | [`Pmipv6.ned:12`](../../../../../src/inet/networklayer/pmipv6/Pmipv6.ned) | "Implements Proxy Mobile IPv6 (RFC 5213): network-based mobility management." The NED documentation comment, which the module reference publishes. |
| RFC 5213 | [`LocalMobilityAnchor.ned:13`](../../../../../src/inet/node/pmipv6/LocalMobilityAnchor.ned) | "...acts as the Local Mobility Anchor (LMA) in a Proxy Mobile IPv6 (RFC 5213) domain." |
| RFC 5213 | [`MobileAccessGateway.ned:13`](../../../../../src/inet/node/pmipv6/MobileAccessGateway.ned) | "...acts as a Mobile Access Gateway (MAG) in a Proxy Mobile IPv6 (RFC 5213) domain." |
| RFC 5213 | [`Pmipv6.cc:375`](../../../../../src/inet/networklayer/pmipv6/Pmipv6.cc) | `pbu->setAccessTechnologyType(4); // IEEE 802.11 (RFC 5213 access technology type)`. The one source-level citation inside `Pmipv6.cc`. |
| RFC 5213 | [`WHATSNEW:521`](../../../../../WHATSNEW) | "Support for Proxy Mobile IPv6 (PMIPv6, RFC 5213) was added." The release notes, section 6, "Proxy Mobile IPv6". |
| RFC 5213 | [`MobilityHeader.msg:53-65,85-96,112-121`](../../../../../src/inet/networklayer/mipv6/MobilityHeader.msg) | The proxy-mobility fields of `BindingUpdate` and `BindingAcknowledgement` (`proxyRegistrationFlag`, `homeNetworkPrefix`, `handoffIndicator`, `accessTechnologyType`, `timestampValue`) and the Proxy Mobile IPv6 status codes of `BaStatus`, each commented with an RFC 5213 section number. This file is owned by the Mobile IPv6 code area; see [`model/mipv6/conformance.md`](../mipv6/conformance.md) for the citations of the RFC 6275 family in the same file. |
| RFC 5213 | [`MobilityHeaderSerializer.h:19,33`](../../../../../src/inet/networklayer/mipv6/MobilityHeaderSerializer.h) | The class comment: "Also serializes the Proxy Mobile IPv6 (RFC 5213) proxy mobility options..."; and the size helper: "Total chunk length of a Proxy Binding Update (RFC 5213)...". |
| RFC 5213 | [`MobilityHeaderSerializer.cc:23,161,164,186,189,292,294,318,321`](../../../../../src/inet/networklayer/mipv6/MobilityHeaderSerializer.cc) | The proxy-option block comment and each P-flag / option-block encode and decode site, one comment per site. |
| RFC 4283 | [`MobilityHeader.msg:60`](../../../../../src/inet/networklayer/mipv6/MobilityHeader.msg) | `string mobileNodeIdentifier; // Mobile Node Identifier option (NAI), RFC 4283`. The same field `Pmipv6.cc` populates at every Proxy Binding Update and Acknowledgement (`Pmipv6.cc:246,271,371`, no RFC citation at the call sites themselves — the citation is on the field's declaration, in the shared message file). |

The users guide (`doc/src/users-guide/*.rst`) does not mention Proxy Mobile IPv6, PMIPv6, or RFC
5213 anywhere — checked directly (`grep -rn -i 'pmipv6\|proxy mobile' doc/src/users-guide/*.rst`
returns nothing). The only claims a reader outside the source tree can find are in the release
notes (`WHATSNEW:521`).

Mapped onto the standards map, [`standards.md`](../../protocol/pmipv6/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 5213 | yes, `base` | **yes**, in the class doc, the NED doc, both node types, the release notes, and the message/serializer fields | No stale citation: every RFC 5213 mention in the tree names the current document, unlike Mobile IPv6's split claim (see [`model/mipv6/conformance.md`](../mipv6/conformance.md#part-1--the-claims)). |
| RFC 4283 | yes, `companion`, mandatory for this exchange | **yes**, once, on the shared field's declaration | The declaration is in a file `Pmipv6.cc` does not own; `Pmipv6.cc` itself never cites RFC 4283 even though it populates the field at every message it builds. |
| RFC 6543 | no, level 3 | no | Zero citations. The reserved-address gap RFC 6543 closes (RFC 5213 §6.8) is untouched: `Pmipv6.cc` never sets a link-local or link-layer address for a Mobile Access Gateway's access link at all. |
| RFC 7864 | no, level 5 | no | Zero citations, and zero mentions of a second interface or of flow mobility anywhere in `Pmipv6.cc`/`.h`. |
| RFC 5844 | no, level 5 | no | Zero citations, and zero mentions of "ipv4" anywhere in `src/inet/networklayer/pmipv6/` — checked directly. |

**The finding of the survey.** Proxy Mobile IPv6's own claim is clean: one document, cited by its
current number, everywhere the model names a standard — the class doc, the NED doc, both node
types, and the release notes. There is no obsolete citation to name, which is itself the level 1
headline for this protocol: the opposite finding from Mobile IPv6, whose behavioral half claims
the RFC 3775 that RFC 6275 obsoletes (see
[`model/mipv6/conformance.md`](../mipv6/conformance.md#part-1--the-claims)). The one gap here is
a documentation gap, not a citation gap: the users guide, which introduces Mobile IPv6 in its own
section, never mentions Proxy Mobile IPv6 at all.

For the level 2 pass, four facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. `grep -n 'TODO\|FIXME' src/inet/networklayer/pmipv6/Pmipv6.cc src/inet/networklayer/pmipv6/Pmipv6.h`
   returns nothing. Unlike every other module in this wave, PMIPv6 carries no half-written
   mechanism and no stated refusal anywhere in its own files. All five `cRuntimeError` throws
   (`Pmipv6.cc:48,81,96,270,360`) are configuration guards — a missing or contradictory
   parameter — not a protocol-content refusal.
2. `Pmipv6.h:73,82`: the Local Mobility Anchor's binding cache is
   `std::map<std::string, BindingCacheEntry> bindingCache; // key: MN identifier` — one entry per
   mobile node identifier. RFC 5213 §5.4 (Multihoming Support) needs a lookup that can hold more
   than one entry per mobile node, for its several interfaces; this data structure cannot, which
   is consistent with the level 5 placement of §5.4 and gives a level 2 pass a concrete reason a
   multihoming check cannot be built against this module as it stands.
3. `localMobilityAnchorAddress` is a single fixed address, read once from a NED parameter
   (`Pmipv6.cc`, `initialize()`, `stage == INITSTAGE_LOCAL`). Nothing in the module selects among
   more than one anchor, so the §5.7 (Local Mobility Anchor Address Discovery) area has no code
   to check.
4. `Pmipv6.cc` never sets a link-local or a link-layer address for the access link it shares with
   a mobile node (checked by the absence of any such call). RFC 5213 §6.8 leaves the exact
   method open and RFC 6543 later reserves a specific value (see the override table of
   `standards.md`); since the model has no code for either the open method or the reserved value,
   a level 3 check of RFC 6543 is, on the evidence gathered so far, an unimplemented-feature
   check and not a defect check — to be confirmed once §6.8 and §6.9.1 are read in full.
