# IPv4 — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around IPv4, records which document governs each contested clause, and pins the exact set
that the current pass tests against. Every later artifact of this folder is bound to the
in-scope set below.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-04. The relationship words are the ones of the workflow: `base`,
`updates`, `obsoletes`, `amends`, `companion`.

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 791 | Internet Protocol | September 1981 | Internet Standard | `base` | `rfc791.txt`, 2026-09-02 |
| RFC 792 | Internet Control Message Protocol | September 1981 | Internet Standard | `companion` of RFC 791 (error reports) | `rfc792.txt`, 2026-09-02 |
| RFC 1349 | Type of Service in the Internet Protocol Suite | July 1992 | Proposed Standard | `updates` RFC 791; obsoleted by RFC 2474 | no |
| RFC 2474 | Definition of the Differentiated Services Field | December 1998 | Proposed Standard | `updates` RFC 791; `obsoletes` RFC 1349 | no |
| RFC 6864 | Updated Specification of the IPv4 ID Field | February 2013 | Proposed Standard | `updates` RFC 791 | no |
| RFC 950 | Internet Standard Subnetting Procedure | August 1985 | Internet Standard | `updates` RFC 792 | no |
| RFC 4884 | Extended ICMP to Support Multi-Part Messages | May 2007 | Proposed Standard | `updates` RFC 792 | no |
| RFC 6633 | Deprecation of ICMP Source Quench Messages | May 2012 | Proposed Standard | `updates` RFC 792 | no |
| RFC 6918 | Formally Deprecating Some ICMPv4 Message Types | April 2013 | Proposed Standard | `updates` RFC 792 | no |
| RFC 1122 | Requirements for Internet Hosts — Communication Layers | October 1989 | Internet Standard | `companion` (see the note below) | no |
| RFC 1191 | Path MTU Discovery | November 1990 | Draft Standard | `companion` (redefines one RFC 792 field) | no |
| RFC 815 | IP Datagram Reassembly Algorithms | July 1982 | informational, legacy | `companion` (an algorithm, not a requirement) | no |

Sources of the two cached texts:

- `rfc791.txt` — <https://www.rfc-editor.org/rfc/rfc791.txt>, downloaded 2026-09-02.
- `rfc792.txt` — <https://www.rfc-editor.org/rfc/rfc792.txt>, downloaded 2026-09-02.

Two relationship notes, because the register alone gives the wrong picture:

- **RFC 1122 is not a formal update of RFC 791.** Its `Updates` header names RFC 793 only.
  But its §3 restates the IPv4 host rules of RFC 791 with explicit MUST and SHOULD
  keywords, so it governs the strength of several statements for a host. The workflow
  therefore records it as a companion with override rows, not as an unrelated document.
- **RFC 1191 is not a formal update of RFC 792.** It redefines the field that RFC 792
  labels "unused" in the destination unreachable message (§4, `Router specification`), so
  it governs the layout of exactly the message that check `dont-fragment` observes.

## Override table

One row per clause-level conflict. `Governs` names the document a test must follow when the
two texts disagree. The RFC 791 and RFC 792 references are line numbers of the cached
files; the other references are clause numbers, because those texts are not cached.

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Identification uniqueness | RFC 791 §2.3, `rfc791.txt:684-687` (unique per source, destination, protocol) | RFC 6864 §4.1 `IPv4 ID Used Only for Fragmentation`, §4.3 `IPv4 ID Requirements That Persist` | RFC 6864 | no |
| Type of service byte | RFC 791 §3.1 Type of Service | RFC 1349 (obsolete), then RFC 2474 §3 `Differentiated Services Field Definition`, §4.2.2 | RFC 2474 | no |
| ICMP destination unreachable, unused field | RFC 792, `rfc792.txt:215` (code 4), unused word of the message | RFC 1191 §4 `Router specification` (low 16 bits become the next-hop MTU) | RFC 1191 | no |
| Reassembly buffer size for a host | RFC 791 §3.1, `rfc791.txt:961-963` (accept 576 octets) | RFC 1122 §3.3.2 `Reassembly` (EMTU_R MUST be at least 576) | RFC 1122 | no |
| Strength of the IPv4 host rules | RFC 791 prose, no keywords | RFC 1122 §3.2.1 `Internet Protocol -- IP` (MUST and SHOULD keywords) | RFC 1122 | no |
| ICMP source quench | RFC 792, source quench message | RFC 6633 (deprecated; a host MUST silently discard) | RFC 6633 | no |

No row is in scope in the current pass, so no catalog entry carries an `Overridden by`
field yet. Every row above becomes an `Overridden by` cross reference on the day its
document enters the in-scope set.

## In-scope set

The current pass tests against these exact documents:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 791 | September 1981, Internet Standard, no revision since | [`rfc791-checklist.md`](rfc791-checklist.md) |
| RFC 792 | September 1981, Internet Standard, no revision since | [`rfc792-checklist.md`](rfc792-checklist.md) |

RFC 791 and RFC 792 revise by replacement, never in place, so the document identity pins
the version by itself. A standard that revises in place (an IEEE edition, for example)
needs the year in this table and in the file name of the cached text.

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 1122 | The largest and most valuable next step. It changes the strength of many RFC 791 statements, so it needs its own catalog file and a strength review of the whole existing catalog. |
| RFC 6864 | Changes one selected area (identification). Bring it in together with the identification checks, which are candidates today. |
| RFC 1191 | The model already sends the next-hop MTU that RFC 1191 defines (see [`rfc791-results.md`](rfc791-results.md)), so the observation exists but no catalog entry claims it yet. |
| RFC 2474, RFC 1349 | The type of service area is out of scope in the catalog itself. |
| RFC 950, RFC 4884, RFC 6633, RFC 6918 | These change ICMP message types that the catalog does not cover. |
| RFC 815 | An algorithm description, not a source of checkable requirements. |
