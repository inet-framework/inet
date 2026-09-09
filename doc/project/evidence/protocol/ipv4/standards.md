# IPv4 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around IPv4, records which document governs each contested clause, and pins the exact set
that the current pass tests against. Every later artifact of this folder is bound to the
in-scope set below.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-08 and checked again 2026-09-09. The relationship words are the ones of
the workflow: `base`, `updates`, `obsoletes`, `amends`, `companion`.

## Target level

**Level 3 — Edge** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
Level 3 adds the negative requirements, the error reports, and the crafted or corrupted
input, and it brings in the documents where those rules live: the host requirements and
the identification errata. The in-scope set below therefore holds four documents.

The level decides which documents the next pass must add:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 4, Dynamics | nothing new: the reassembly timer clauses of RFC 791 §3.2 and RFC 1122 §3.3.2 are already in the catalogs and wait for the statistical toolset | a timer is a distribution, not a value |
| level 5, Complete | RFC 2474, RFC 1191, RFC 950, RFC 1812, RFC 4884, RFC 6633, RFC 6918 | the options, the type of service byte, the router requirements, and the ICMP messages the catalogs leave out |

What the pass actually reached is not recorded here. It is in
[`model/ipv4/coverage.md`](../../model/ipv4/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 791 | Internet Protocol | September 1981 | Internet Standard | `base` | [`standard/rfc791/`](../../standard/rfc791/rfc791.txt), 2026-09-02 |
| RFC 792 | Internet Control Message Protocol | September 1981 | Internet Standard | `companion` of RFC 791 (error reports) | [`standard/rfc792/`](../../standard/rfc792/rfc792.txt), 2026-09-02 |
| RFC 1122 | Requirements for Internet Hosts — Communication Layers | October 1989 | Internet Standard | `companion` (see the note below) | [`standard/rfc1122/`](../../standard/rfc1122/rfc1122.txt), 2026-09-09 |
| RFC 6864 | Updated Specification of the IPv4 ID Field | February 2013 | Proposed Standard | `updates` RFC 791, RFC 1122, RFC 2003 | [`standard/rfc6864/`](../../standard/rfc6864/rfc6864.txt), 2026-09-09 |
| RFC 1349 | Type of Service in the Internet Protocol Suite | July 1992 | Proposed Standard | `updates` RFC 791; obsoleted by RFC 2474 | no |
| RFC 2474 | Definition of the Differentiated Services Field | December 1998 | Proposed Standard | `updates` RFC 791; `obsoletes` RFC 1349 | no |
| RFC 950 | Internet Standard Subnetting Procedure | August 1985 | Internet Standard | `updates` RFC 792 | no |
| RFC 4884 | Extended ICMP to Support Multi-Part Messages | May 2007 | Proposed Standard | `updates` RFC 792 | no |
| RFC 6633 | Deprecation of ICMP Source Quench Messages | May 2012 | Proposed Standard | `updates` RFC 792, RFC 1122 | no |
| RFC 6918 | Formally Deprecating Some ICMPv4 Message Types | April 2013 | Proposed Standard | `updates` RFC 792, RFC 1122 | no |
| RFC 1191 | Path MTU Discovery | November 1990 | Draft Standard | `companion` (redefines one RFC 792 field) | no |
| RFC 1812 | Requirements for IP Version 4 Routers | June 1995 | Proposed Standard | `companion` (the router counterpart of RFC 1122) | no |
| RFC 815 | IP Datagram Reassembly Algorithms | July 1982 | informational, legacy | `companion` (an algorithm, not a requirement) | no |

Sources of the four cached texts. Each one lives in the folder of its own document,
beside the catalog of that document, and every quote in this tree cites it by file name
and line number:

- `rfc791.txt` in [`evidence/standard/rfc791/`](../../standard/rfc791) —
  <https://www.rfc-editor.org/rfc/rfc791.txt>, downloaded 2026-09-02.
- `rfc792.txt` in [`evidence/standard/rfc792/`](../../standard/rfc792) —
  <https://www.rfc-editor.org/rfc/rfc792.txt>, downloaded 2026-09-02.
- `rfc1122.txt` in [`evidence/standard/rfc1122/`](../../standard/rfc1122) —
  <https://www.rfc-editor.org/rfc/rfc1122.txt>, downloaded 2026-09-09.
- `rfc6864.txt` in [`evidence/standard/rfc6864/`](../../standard/rfc6864) —
  <https://www.rfc-editor.org/rfc/rfc6864.txt>, downloaded 2026-09-09.

Three relationship notes, because the register alone gives the wrong picture:

- **RFC 1122 is not a formal update of RFC 791.** Its `Updates` header names RFC 793 only.
  But its §3 restates the IPv4 host rules of RFC 791 with explicit MUST and SHOULD
  keywords, so it governs the strength of several statements for a host. The workflow
  therefore records it as a companion with override rows, not as an unrelated document.
  RFC 1122 speaks for **hosts**. Its counterpart for routers is RFC 1812, which is out of
  scope; where a check observes a gateway, the RFC 791 and RFC 792 text still governs the
  gateway.
- **RFC 6864 is a formal update** of RFC 791 and of RFC 1122. It replaces the
  identification rule of RFC 791 §2.3 and the retransmission permission of RFC 1122
  §3.2.1.5, and it restates the don't-fragment rule with a keyword.
- **RFC 1191 is not a formal update of RFC 792.** It redefines the field that RFC 792
  labels "unused" in the destination unreachable message (§4, `Router specification`), so
  it governs the layout of exactly the message that check `dont-fragment` observes.

## Override table

One row per clause-level conflict. `Governs` names the document a test must follow when the
two texts disagree. The references into the four cached files are line numbers; the other
references are clause numbers, because those texts are not cached.

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Identification uniqueness | RFC 791 §2.3, `rfc791.txt:684-687` (unique per source, destination, protocol) | RFC 6864 §4.3, `rfc6864.txt:438-440` (unique per tuple within one MDL, for non-atomic datagrams only); §4.1, `rfc6864.txt:375-376` (the field of an atomic datagram is ignored) | RFC 6864 | **yes** |
| Identification of a retransmitted copy | RFC 1122 §3.2.1.5, `rfc1122.txt:1878-1880` (a copy MAY keep the identification) | RFC 6864 §4.2, `rfc6864.txt:421-422` (a non-atomic copy MUST NOT reuse it) | RFC 6864 | **yes** |
| Don't fragment | RFC 791 §2.3, `rfc791.txt:662-666` (prose, no keyword) | RFC 6864 §4.3, `rfc6864.txt:458-460` (MUST NOT fragment; transit devices MUST NOT clear DF) | RFC 6864 | **yes** |
| Header checksum verification | RFC 791 §1.4, `rfc791.txt:365-366` (prose: "is discarded at once") | RFC 1122 §3.2.1.2, `rfc1122.txt:1691-1693` (MUST verify, silently discard) | RFC 1122 | **yes** |
| Reassembly, the obligation | RFC 791 §2.3, `rfc791.txt:723-729` (a procedure, no obligation) | RFC 1122 §3.2.1.4 and §3.3.2, `rfc1122.txt:3281` (MUST implement reassembly) | RFC 1122 | **yes** |
| Reassembly buffer size for a host | RFC 791 §3.1, `rfc791.txt:961-963` (accept 576 octets) | RFC 1122 §3.3.2, `rfc1122.txt:3285-3286` (EMTU_R MUST be at least 576) | RFC 1122 | **yes** |
| Strength of the IPv4 host rules | RFC 791 prose, no keywords | RFC 1122 §3.2.1, `rfc1122.txt:1680-2160` (MUST and SHOULD keywords) | RFC 1122 | **yes** |
| Error reports of a host | RFC 792, "may" in every error message | RFC 1122 §3.3.8, `rfc1122.txt:4021-4023` (MUST return ICMP errors wherever practical); §3.2.2, `rfc1122.txt:2249-2267` (five cases where an error MUST NOT be sent) | RFC 1122, for a host; RFC 792 keeps governing a gateway (RFC 1812 is out of scope) | **yes** |
| Type of service byte | RFC 791 §3.1 Type of Service | RFC 1349 (obsolete), then RFC 2474 §3 `Differentiated Services Field Definition`, §4.2.2 | RFC 2474 | no |
| ICMP destination unreachable, unused field | RFC 792, `rfc792.txt:215` (code 4), unused word of the message | RFC 1191 §4 `Router specification` (low 16 bits become the next-hop MTU) | RFC 1191 | no |
| ICMP source quench | RFC 792, source quench message; RFC 1122 §3.2.2.3 | RFC 6633 (deprecated; a host MUST silently discard) | RFC 6633 | no |

Every in-scope row is an `Overridden by` cross reference in the catalog of the base
document: the RFC 791 entries name the RFC 1122 or RFC 6864 entry that governs, and the
RFC 1122 identification entry names the RFC 6864 entry. The RFC 792 entries carry no
`Overridden by`, because the two messages they describe are gateway reports and RFC 1122
governs hosts only; a note under each entry says so.

## In-scope set

The current pass tests against these exact documents:

| Document | Version | Sections in scope | Catalog file |
| --- | --- | --- | --- |
| RFC 791 | September 1981, Internet Standard, no revision since | all but the options, the type of service, and the security annex | [`rfc791/catalog.md`](../../standard/rfc791/catalog.md) |
| RFC 792 | September 1981, Internet Standard, no revision since | the error reports of the RFC 791 mechanisms | [`rfc792/catalog.md`](../../standard/rfc792/catalog.md) |
| RFC 1122 | October 1989, Internet Standard, no revision since | §3.2.1 (IP), §3.2.2 (ICMP: the general rules and the error messages of §3.2.2.1, §3.2.2.4, §3.2.2.5), §3.3.2 (reassembly), §3.3.3 (fragmentation), §3.3.8 (error reporting) | [`rfc1122/catalog.md`](../../standard/rfc1122/catalog.md) |
| RFC 6864 | February 2013, Proposed Standard, no revision since | the whole document; §4 holds the requirements | [`rfc6864/catalog.md`](../../standard/rfc6864/catalog.md) |

The four documents revise by replacement, never in place, so the document identity pins
the version by itself. A standard that revises in place (an IEEE edition, for example)
needs the year in this table and in the file name of the cached text.

RFC 1122 is a multi-protocol document: its §3 covers IP, ICMP and IGMP, and its §4 covers
UDP and TCP. Only the sections named above enter the IPv4 set. The parts of §3 that stay
out, with the reason:

| RFC 1122 part | Reason it is out of the IPv4 set |
| --- | --- |
| §3.2.1.6 Type of service | the area is out of scope in the RFC 791 catalog; RFC 2474 replaced the byte; level 5 |
| §3.2.1.8 Options, §3.3.5 Source route forwarding | the options are level 5 by the level table of the guide |
| §3.2.1.3, the subnet requirement; §3.3.6 Broadcasts | the addressing family (RFC 919, RFC 922, RFC 950) is a scope of its own. The rules of §3.2.1.3 about the source address of a sent datagram and about the validation of a received datagram **are** in scope, because they are edge rules of the base mechanisms |
| §3.2.2.2 Redirect, §3.3.1 Routing outbound datagrams, §3.3.4 Multihoming | host routing is a scope of its own, and the redirect message serves it |
| §3.2.2.3 Source quench | deprecated by RFC 6633, which is out of scope |
| §3.2.2.6 to §3.2.2.9, the ICMP query messages | ICMP as a protocol of its own (echo, timestamp, address mask) belongs to an `icmp` protocol folder; this set takes from ICMP only what reports the failures of IPv4 |
| §3.2.3 IGMP, §3.3.7 Multicasting | a protocol of its own |
| §3.4 Internet/transport layer interface | an interface description, not a wire behavior. The interface `MUST`s that §3.2.1, §3.3.2 and §3.3.3 state are in the catalog with class `internal` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 1812 | The router counterpart of RFC 1122. The mockup observes a gateway in several checks, and RFC 1812 would give the gateway rules their keywords. Bring it in at level 5, together with a strength review of the gateway-side statements. |
| RFC 1191 | It gives a meaning to the word that RFC 792 leaves unused in the very message that the `dont-fragment` check observes. No catalog entry claims that field yet, so bring the document in together with an entry for the next-hop MTU. |
| RFC 2474, RFC 1349 | The type of service area is out of scope in the catalog itself. |
| RFC 950, RFC 4884, RFC 6633, RFC 6918 | These change ICMP message types that the catalog does not cover. |
| RFC 815 | An algorithm description, not a source of checkable requirements. |
