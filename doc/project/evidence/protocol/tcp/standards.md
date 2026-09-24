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

**Level 4 — Dynamics** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
Levels 1 to 3 ran against RFC 9293 alone, which carries the reset and the error paths
itself, so TCP needed no error-report companion. Level 4 adds the two control loops, and
each one has its own document: RFC 6298 for the retransmission timer and RFC 5681 for
congestion control.

The level decides which documents the next pass must add:

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 3, Edge | none | RFC 9293 already folds in RFC 1122 and the errata, so the edge rules — the receiver's checksum check, reset handling on a live connection, a shrunk window — are in the document that is in scope |
| level 4, Dynamics | RFC 6298, RFC 5681 | in scope now — the retransmission timer and the congestion window are the two control loops of TCP |
| level 5, Complete | RFC 7323, RFC 2018, RFC 2883, **RFC 6675**, RFC 8257, RFC 3168 | the options, the loss-recovery extensions, and explicit congestion notification. RFC 6675 replaces RFC 3517, which an earlier version of this row named |

TCP shows a property of the level system worth noting: a well-collected base document can
carry a whole level on its own. RFC 9293 replaced a family of update documents, so level 3
needs no new text, while IPv4 must bring in RFC 1122 to reach the same depth.

What the pass actually reached is not recorded here. It is in
[`model/tcp/coverage.md`](../../model/tcp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 9293 | Transmission Control Protocol (TCP) | August 2022 | Internet Standard | `base` | [`standards/RFC/rfc9293.txt`](../../../../../../standards/RFC/rfc9293.txt), 2026-09-08 |
| RFC 793 | Transmission Control Protocol | September 1981 | Internet Standard | **obsoleted by RFC 9293** | no |
| RFC 1122 | Requirements for Internet Hosts | October 1989 | Internet Standard | `updates` RFC 793; RFC 9293 in turn updates and replaces its TCP part | no |
| RFC 5681 | TCP Congestion Control | September 2009 | Draft Standard | `companion` (congestion control); `obsoletes` RFC 2581 | [`standards/RFC/rfc5681.txt`](../../../../../../standards/RFC/rfc5681.txt), 2026-09-11 |
| RFC 6298 | Computing TCP's Retransmission Timer | June 2011 | Proposed Standard | `companion`; `obsoletes` RFC 2988; `updates` RFC 1122 | [`standards/RFC/rfc6298.txt`](../../../../../../standards/RFC/rfc6298.txt), 2026-09-11 |
| RFC 7323 | TCP Extensions for High Performance | September 2014 | Proposed Standard | `companion` (window scale, timestamps); `obsoletes` RFC 1323 | no |
| RFC 2018 | TCP Selective Acknowledgment Options | October 1996 | Proposed Standard | `companion` (SACK) | no |
| RFC 6675 | A Conservative Loss Recovery Algorithm Based on SACK | August 2012 | Proposed Standard | `companion` (SACK loss recovery); `obsoletes` RFC 3517 | no |
| RFC 6582 | The NewReno Modification to TCP's Fast Recovery Algorithm | April 2012 | Proposed Standard | `companion` (NewReno); `obsoletes` RFC 3782 | no |
| RFC 8201 | Path MTU Discovery for IP version 6 | July 2017 | Internet Standard | `companion` (PMTUD); `obsoletes` RFC 1981 | no |
| RFC 2581 | TCP Congestion Control | April 1999 | Proposed Standard | **obsoleted by RFC 5681**; the model cites it 48 times | no |
| RFC 2988 | Computing TCP's Retransmission Timer | November 2000 | Proposed Standard | **obsoleted by RFC 6298**; the model cites it 8 times | no |
| RFC 3517 | A Conservative SACK-based Loss Recovery Algorithm | April 2003 | Proposed Standard | **obsoleted by RFC 6675**; the model cites it 52 times | no |
| RFC 3782 | The NewReno Modification to TCP's Fast Recovery Algorithm | April 2004 | Proposed Standard | **obsoleted by RFC 6582**; the model cites it 17 times | no |
| RFC 1323 | TCP Extensions for High Performance | May 1992 | Proposed Standard | **obsoleted by RFC 7323**; the model cites it 29 times | no |
| RFC 1981 | Path MTU Discovery for IP version 6 | August 1996 | Draft Standard | **obsoleted by RFC 8201**; the model cites it 9 times | no |
| RFC 2001 | TCP Slow Start, Congestion Avoidance, Fast Retransmit | January 1997 | Proposed Standard | **obsoleted by RFC 2581**, then by RFC 5681; the model cites it 5 times | no |

The seven superseded rows are the outcome of a sweep of the model's own citations. The
sweep and its counts are in
[`model/tcp/conformance.md`](../../model/tcp/conformance.md#the-claimed-set-is-one-standards-generation-behind).
Each `obsoletes` relation comes from the header of the replacing text.

Source of the texts:

- `rfc9293.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc9293.txt) —
  <https://www.rfc-editor.org/rfc/rfc9293.txt>, downloaded 2026-09-08.
- `rfc6298.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc6298.txt) —
  <https://www.rfc-editor.org/rfc/rfc6298.txt>, downloaded 2026-09-11.
- `rfc5681.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc5681.txt) —
  <https://www.rfc-editor.org/rfc/rfc5681.txt>, downloaded 2026-09-11.

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
| Retransmission timer | RFC 793 timeout prose | RFC 6298, `rfc6298.txt:1-619` | **RFC 6298** | **yes** |
| Retransmission timer | RFC 2988 | RFC 6298, `rfc6298.txt:7` (`Obsoletes: 2988`) | **RFC 6298** | **yes** |
| Congestion control | absent from RFC 793 | RFC 5681, `rfc5681.txt:1-1011` | **RFC 5681** | **yes** |
| Congestion control | RFC 2581 | RFC 5681, `rfc5681.txt:7` (`Obsoletes: 2581`) | **RFC 5681** | **yes** |
| Congestion control | RFC 2001 | RFC 2581, then RFC 5681 | **RFC 5681** | **yes** |
| SACK loss recovery | RFC 3517 | RFC 6675 | RFC 6675 | no |
| NewReno fast recovery | RFC 3782 | RFC 6582 | RFC 6582 | no |
| Path MTU discovery, IPv6 | RFC 1981 | RFC 8201 | RFC 8201 | no |
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
| RFC 6298 | June 2011, Proposed Standard, the current text | [`standard/rfc6298/catalog.md`](../../standard/rfc6298/catalog.md) |
| RFC 5681 | September 2009, Draft Standard, the current text | [`standard/rfc5681/catalog.md`](../../standard/rfc5681/catalog.md) |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 793 | Obsolete. Kept in the list because the model, the literature, and most course material still cite it. A test must target RFC 9293. |
| RFC 7323, RFC 2018 | Options that a basic pass does not exercise. |
| RFC 1122 | RFC 9293 replaces its TCP part, so it adds nothing while RFC 9293 is in scope. |
