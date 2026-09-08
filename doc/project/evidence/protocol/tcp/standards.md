# TCP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around TCP, records which document governs each contested clause, and pins the exact set
that the current pass tests against.

The relationships come from the RFC-editor metadata (`https://www.rfc-editor.org/rfc/rfcNNN.json`),
retrieved 2026-09-08, and from the text of RFC 9293 itself.

TCP differs from IPv4 in one important way. The 1981 base document, RFC 793, is **obsolete**.
RFC 9293 replaced it in August 2022. A test that targets the 1981 text tests a document that
no longer governs.

## Target level

**Level 2 — Core** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
The in-scope set below holds exactly the document that level 1 and level 2 need. TCP needs
no separate error-report companion: RFC 9293 carries the reset and the error paths itself.

The level decides which documents the next pass must add:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 3, Edge | none | RFC 9293 already folds in RFC 1122 and the errata, so the edge rules — the receiver's checksum check, reset handling on a live connection, a shrunk window — are in the document that is in scope |
| level 4, Dynamics | RFC 6298, RFC 5681 | the retransmission timer and the congestion window are the two control loops of TCP |
| level 5, Complete | RFC 7323, RFC 2018, RFC 2883, RFC 3517, RFC 8257 | the options and the loss-recovery extensions |

TCP shows a property of the level system worth noting: a well-collected base document can
carry a whole level on its own. RFC 9293 replaced a family of update documents, so level 3
needs no new text, while IPv4 must bring in RFC 1122 to reach the same depth.

What the pass actually reached is not recorded here. It is in
[`model/tcp/coverage.md`](../../model/tcp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Cached |
| --- | --- | --- | --- | --- | --- |
| RFC 9293 | Transmission Control Protocol (TCP) | August 2022 | Internet Standard | `base` | [`standard/rfc9293/`](../../standard/rfc9293/rfc9293.txt), 2026-09-08 |
| RFC 793 | Transmission Control Protocol | September 1981 | Internet Standard | **obsoleted by RFC 9293** | no |
| RFC 1122 | Requirements for Internet Hosts | October 1989 | Internet Standard | `updates` RFC 793; RFC 9293 in turn updates and replaces its TCP part | no |
| RFC 5681 | TCP Congestion Control | September 2009 | Draft Standard | `companion` (congestion control) | no |
| RFC 6298 | Computing TCP's Retransmission Timer | June 2011 | Proposed Standard | `companion`; `updates` RFC 1122 | no |
| RFC 7323 | TCP Extensions for High Performance | September 2014 | Proposed Standard | `companion` (window scale, timestamps) | no |
| RFC 2018 | TCP Selective Acknowledgment Options | October 1996 | Proposed Standard | `companion` (SACK) | no |

Source of the cached text:

- `rfc9293.txt` in [`evidence/standard/rfc9293/`](../../standard/rfc9293) —
  <https://www.rfc-editor.org/rfc/rfc9293.txt>, downloaded 2026-09-08.

RFC 9293 states its own place in the family, and this is the authority for the rows above:

> "This document obsoletes RFC 793, as well as RFCs 879, 2873, 6093, 6429, 6528, and 6691
> that updated parts of RFC 793. It updates RFCs 1011 and 1122, and it should be considered
> as a replacement for the portions of those documents dealing with TCP requirements."
> — `rfc9293.txt:26-30`

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The whole protocol specification | RFC 793 (September 1981) | RFC 9293 (August 2022), `rfc9293.txt:26` | **RFC 9293** | **yes** |
| TCP host requirements | RFC 1122 §4.2 | RFC 9293, `rfc9293.txt:27-29`, replaces the TCP part | RFC 9293 | yes, through RFC 9293 |
| Retransmission timer | RFC 793 timeout prose | RFC 6298 | RFC 6298 | no |
| Congestion control | absent from RFC 793 | RFC 5681 | RFC 5681 | no |
| Selective acknowledgment | absent from RFC 793 | RFC 2018 | RFC 2018 | no |
| Window scale and timestamps | RFC 793 16-bit window | RFC 7323 | RFC 7323 | no |

The first two rows are in scope and they are already resolved: the catalog of this pass
quotes RFC 9293, so no entry needs an `Overridden by` field. The value of the row is that it
records **why** the catalog does not quote RFC 793, which is the document almost every text
about TCP still cites.

## In-scope set

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 9293 | August 2022, Internet Standard, the current text | [`standard/rfc9293/catalog.md`](../../standard/rfc9293/catalog.md) |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 793 | Obsolete. Kept in the list because the model, the literature, and most course material still cite it. A test must target RFC 9293. |
| RFC 5681, RFC 6298 | Congestion control and the retransmission timer are the natural second pass. Both need statistical or timing checks rather than single-segment wire checks. |
| RFC 7323, RFC 2018 | Options that a basic pass does not exercise. |
| RFC 1122 | RFC 9293 replaces its TCP part, so it adds nothing while RFC 9293 is in scope. |
