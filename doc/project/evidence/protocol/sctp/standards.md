# SCTP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around the Stream Control Transmission Protocol, records which document governs each
contested clause, and pins the set that the next pass tests against. SCTP has one current base
document, one immediately superseded predecessor, several extensions each in a separate
document, and a handful of relatives that touch SCTP without being about SCTP.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test. RFC 9260 is large
(133 pages); this pass pins a clause slice for level 2 and puts the rest of the document, and
the whole of every extension, in the out-of-scope table below.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 9260 §3 (packet and chunk formats), §4 (association state diagram, background), §5 (association initialization), §6 (user data transfer, including §6.8 the checksum), §9 (termination of an association) | the normal exchange: the four-way handshake that establishes an association, ordered and unordered data delivery with its acknowledgment, the wire layout of every chunk the handshake and the transfer use, and a graceful shutdown |
| level 3, Edge | nothing new | §8 (fault management) holds the validity checks: an out-of-the-blue packet, a stale or wrong verification tag, a malformed chunk, and the ABORT it produces; §10 (ICMP handling) is the error-signal companion |
| level 4, Dynamics | nothing new | §7 (congestion control: slow start, congestion avoidance, fast retransmit) and the retransmission timer of §8 (RTO, its backoff, the heartbeat) are control loops and timers, not single values |
| level 5, Complete | RFC 3758 (PR-SCTP), RFC 5061 (ASCONF), RFC 4895 (AUTH), RFC 6525 (stream reconfiguration), RFC 9260 §11 (the upper-layer interface, not wire-observable) | each extension is optional, lives in its own document, and needs its own scenario on top of the base association |

What a pass actually reached is not recorded here. It is in
[`model/sctp/coverage.md`](../../model/sctp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 9260 | Stream Control Transmission Protocol | June 2022 | Proposed Standard | `base` | [`standards/RFC/rfc9260.txt`](../../../../../../standards/RFC/rfc9260.txt), 2026-09-23 |
| RFC 4960 | Stream Control Transmission Protocol | September 2007 | Proposed Standard | `obsoleted by` RFC 9260 | no |
| RFC 2960 | Stream Control Transmission Protocol | October 2000 | Proposed Standard | `obsoleted by` RFC 4960, two generations behind RFC 9260 | no |
| RFC 4460 | Stream Control Transmission Protocol (SCTP) Specification Errata and Issues | April 2006 | Informational | `obsoleted by` RFC 9260; folded into it | no |
| RFC 6096 | Stream Control Transmission Protocol (SCTP) Chunk Flags Registration | January 2011 | Proposed Standard | `updates` RFC 4960, `obsoleted by` RFC 9260; folded into it — RFC 9260 §1 names this explicitly | no |
| RFC 7053 | SACK-IMMEDIATELY Extension for the Stream Control Transmission Protocol | November 2013 | Proposed Standard | `updates` RFC 4960, `obsoleted by` RFC 9260; folded into it — RFC 9260 §1 names this explicitly | no |
| RFC 8540 | Stream Control Transmission Protocol: Errata and Issues in RFC 4960 | February 2019 | Informational | `obsoleted by` RFC 9260; folded into it | no |
| RFC 6335 | Internet Assigned Numbers Authority (IANA) Procedures for the Management of the Service Name and Transport Protocol Port Number Registry | August 2011 | Best Current Practice | `updates` RFC 4960 (and five other RFCs); a registry procedure, not folded into RFC 9260 | no |
| RFC 8899 | Packetization Layer Path MTU Discovery for Datagram Transports | September 2020 | Proposed Standard | `updates` RFC 4960 (and RFC 4821, RFC 6951, RFC 8085, RFC 8261); not in RFC 9260's `obsoletes` list, so it stands on top of RFC 9260 as well | no |
| RFC 3758 | Stream Control Transmission Protocol (SCTP) Partial Reliability Extension | May 2004 | Proposed Standard | `companion`, an extension; current, no `obsoleted_by` | no |
| RFC 5061 | Stream Control Transmission Protocol (SCTP) Dynamic Address Reconfiguration | September 2007 | Proposed Standard | `companion`, an extension; current, no `obsoleted_by` | no |
| RFC 4895 | Authenticated Chunks for the Stream Control Transmission Protocol (SCTP) | August 2007 | Proposed Standard | `companion`, an extension; current, no `obsoleted_by` | no |
| RFC 6525 | Stream Control Transmission Protocol (SCTP) Stream Reconfiguration | March 2012 | Proposed Standard | `companion`, an extension; current, no `obsoleted_by` | no |
| RFC 6951 | UDP Encapsulation of Stream Control Transmission Protocol (SCTP) Packets for End-Host to End-Host Communication | May 2013 | Proposed Standard | `companion`, a separate encapsulation; current, `updated_by` RFC 8899 | no |
| RFC 6356 | Coupled Congestion Control for Multipath Transport Protocols | October 2011 | Experimental | `companion`, a congestion-control algorithm SCTP may adopt for multipath use; current | no |
| RFC 3649 | HighSpeed TCP for Large Congestion Windows | December 2003 | Experimental | `companion`, a TCP congestion-control algorithm; SCTP is not its subject | no |
| RFC 1982 | Serial Number Arithmetic | August 1996 | Proposed Standard | `companion`, a generic wraparound-comparison algorithm; not about SCTP | no |

Source of the text:

- `rfc9260.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc9260.txt) —
  <https://www.rfc-editor.org/rfc/rfc9260.txt>, downloaded 2026-09-23.

Every other document of this family is not downloaded in this pass: the four errata/registration
documents RFC 9260 already folds in (RFC 4460, RFC 6096, RFC 7053, RFC 8540), the two older
generations (RFC 4960, RFC 2960), the generic registry and math documents (RFC 6335,
RFC 1982), the still-standing extension RFC 8899, and the four level-5 extensions. None of them
is in the level-2 in-scope set; see the out-of-scope table.

RFC 9260 states its own lineage in its introduction: "This document obsoletes [RFC4960]. In
addition to that, it incorporates the specification of the chunk flags registry from [RFC6096]
and the specification of the I bit of DATA chunks from [RFC7053]. Therefore, [RFC6096] and
[RFC7053] are also obsoleted by this document." (`rfc9260.txt:271-276`).

RFC 9260 uses the keywords of RFC 2119 and RFC 8174, upper case, with the boilerplate at
`rfc9260.txt:827`. `MUST` is on 329 lines, `SHOULD` on 216, `MAY` on 98, `SHALL` on 2.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The SACK-IMMEDIATELY (`I`) bit of a DATA chunk | Not present in RFC 4960 (2007); it was a separate extension, RFC 7053 (2013), itself preceded by an Internet-Draft | RFC 9260 §3.3.1, `rfc9260.txt:1181` (the field) and §6.10, `rfc9260.txt:3678-3768` (the procedure); folded into the base document, `rfc9260.txt:274-276` | RFC 9260 directly — the `I` bit is part of the base chunk format now, not an extension | yes, level 2, as part of the chunk format |
| The CRC32c checksum appendix | RFC 4960 Appendix B carried the algorithm | RFC 9260 renumbers it to **Appendix A**, `rfc9260.txt:263` (table of contents) and `rfc9260.txt:6966` (the appendix itself); the procedure that uses it is §6.8, `rfc9260.txt:4308-4340` | RFC 9260 Appendix A and §6.8 | yes, level 2 |
| The chunk-flags registry | RFC 4960 left chunk-flag meaning to each chunk type, informally | RFC 6096 (2011) gave it a registry; folded into RFC 9260, `rfc9260.txt:274-276` | RFC 9260, no separate document | yes, level 2, as part of the chunk format |

The three rows above are not conflicts between documents still in force; they are cases where
RFC 9260 absorbed a separate document. A test that names "RFC 4960 Appendix B" for the checksum
targets an appendix letter that no longer exists in the governing text.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 9260 | June 2022, Proposed Standard; §3, §4, §5, §6 (incl. §6.8), §9 | none yet; level 2 writes `standard/rfc9260/catalog.md` |

Out of scope, with the reason:

| Document or clause | Level | Reason |
| --- | --- | --- |
| RFC 9260 §7 | 4 | Congestion control (slow start, congestion avoidance, fast retransmit) is a control loop, not a single value. |
| RFC 9260 §8, the retransmission timer | 4 | RTO and its exponential backoff are a timer series. |
| RFC 9260 §8, the validity checks | 3 | An out-of-the-blue packet, a stale verification tag, and a malformed chunk each need a crafted or corrupted input, not the normal exchange. |
| RFC 9260 §10 | 3 | ICMP handling is the error-signal companion to the fault-management checks above. |
| RFC 9260 §11 | not testable on the wire | The interface with the upper layer is a set of primitives between SCTP and its user, not an observable exchange between two endpoints. |
| RFC 9260 §12 to §17 | out of scope at every level | Security considerations, network-management considerations, the recommended TCB parameters, IANA considerations, and the suggested protocol parameter values state no additional wire behavior of the normal exchange. |
| RFC 3758 (PR-SCTP) | 5 | An optional partial-reliability mode on top of the base association; the normal path does not need it. |
| RFC 5061 (ASCONF) | 5 | An optional mid-association address change; the base association does not add or remove addresses. |
| RFC 4895 (AUTH) | 5 | An optional chunk-authentication mechanism; the base handshake carries no AUTH chunk. |
| RFC 6525 (stream reconfiguration) | 5 | An optional mid-association reset of a stream; the base association does not reset one. |
| RFC 8899 (DPLPMTUD) | 5 | An optional path-MTU-discovery procedure shared with UDP; it is not part of the four-way handshake or the base data transfer. |
| RFC 6951 (SCTP over UDP) | 5 | A separate encapsulation of the whole protocol for a host that cannot send SCTP directly; the base association uses SCTP directly over IP. |
| RFC 6335, RFC 1982 | out of scope at every level | Generic tools (a port-registry procedure, a wraparound-comparison algorithm) that RFC 9260 itself cites (`rfc9260.txt:751,762,6734`); neither states SCTP-specific behavior. |
| RFC 6356, RFC 3649 | 5, and only if concurrent multipath transfer (CMT) enters scope | Alternative congestion-control algorithms for a multipath association; CMT itself is an Internet-Draft family (draft-tuexen and related), not a stable RFC, and is out of scope at every level of this pass for that reason as well as the level. |
| NR-SACK (draft-natarajan-tsvwg-sctp-nrsack) | 5, and unstable | An Internet-Draft, not permanently hosted at a stable `rfc-editor.org` URL. When this area enters scope, cite the exact draft revision and quote by its own clause numbers, per the guide's rule for a text without stable line numbers. |
| CMT (draft-tuexen and related; also referenced as draft-stewart-sctp-pktdrprep in the model's own change log) | 5, and unstable | Concurrent multipath transfer is a family of Internet-Drafts, not one stable document; the same citation rule applies. |
