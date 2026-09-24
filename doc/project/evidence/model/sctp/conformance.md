# SCTP — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/sctp/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the SCTP family:

| Claim | Where | What it says |
| --- | --- | --- |
| none | [`Sctp.ned:15`](../../../../../src/inet/transportlayer/sctp/Sctp.ned) | "Implements the SCTP protocol." The documentation comment of the module, which the module reference publishes, names no document at all. |
| none | [`ch-transport.rst:285-287`](../../../../../doc/src/users-guide/ch-transport.rst) | "The :ned:`Sctp` module implements the Stream Control Transmission Protocol (SCTP)." The user's guide repeats the same silence. |
| RFC 4960 | [`Sctp.ned:61`](../../../../../src/inet/transportlayer/sctp/Sctp.ned) | The `ccModule` parameter: "RFC4960=0". |
| RFC 4960 | [`SctpAssociation.h:187-189`](../../../../../src/inet/transportlayer/sctp/SctpAssociation.h) | `enum SctpCcModules { RFC4960 = 0 };` — the document name is the identifier of the default congestion-control module, used throughout the congestion-control code (for example `SctpAssociationBase.cc:1675`, `case RFC4960:`). |
| RFC 4960 | [`Sctp.ned:138`](../../../../../src/inet/transportlayer/sctp/Sctp.ned) | The CMT congestion-control variant comment: "off = use over every path the default RFC4960 CC (New Reno)". |
| RFC 6356 | [`Sctp.ned:139`](../../../../../src/inet/transportlayer/sctp/Sctp.ned) | "lia = First resource pooling congestion control RFC6356 adapted for SCTP". |
| RFC 3649 | [`SctpAssociation.h:736`](../../../../../src/inet/transportlayer/sctp/SctpAssociation.h) | "bool highSpeedCC; // HighSpeed CC (RFC 3649)". |
| RFC 3649 | [`SctpCcFunctions.cc:29`](../../../../../src/inet/transportlayer/sctp/SctpCcFunctions.cc) | "High-Speed CC cwnd adjustment table from RFC 3649 appendix B". |
| RFC 1982 | [`SctpAssociation.h:1158`](../../../../../src/inet/transportlayer/sctp/SctpAssociation.h) | "Defines see RFC1982 for details", above the TSN comparison functions. |
| RFC 2960 | [`SctpAssociationUtil.cc:2851-2852`](../../../../../src/inet/transportlayer/sctp/SctpAssociationUtil.cc) | "RFC 2960, sect. 6.3.1: new RTT measurements SHOULD be made no more than once per round-trip." |
| RFC 4960 | [`SctpAssociationSendAll.cc:581-582`](../../../../../src/inet/transportlayer/sctp/SctpAssociationSendAll.cc) | "RFC4960: When a Fast Retransmit is being performed, the sender SHOULD ignore the value of cwnd and SHOULD NOT delay retransmission for this single packet." |
| RFC 4960 §3.3.2.1 | [`SctpHeaderSerializer.cc:43`](../../../../../src/inet/transportlayer/sctp/SctpHeaderSerializer.cc) | "Writes an address parameter (RFC 4960 3.3.2.1) of either family". |
| RFC 4960 §4.2 | [`SctpHeaderSerializer.cc:101`](../../../../../src/inet/transportlayer/sctp/SctpHeaderSerializer.cc) | "the parameter is opaque by RFC 4960 4.2, so a peer may fill it with anything." |
| RFC 4960 appendix B | [`SctpHeaderSerializer.cc:1116-1119`, `SctpHeaderSerializer.cc:1151`](../../../../../src/inet/transportlayer/sctp/SctpHeaderSerializer.cc) | Two comments on the checksum byte order: "RFC 4960 appendix B folds a byte swap into the end of the CRC routine..." and "the field carries the CRC32c byte swapped (RFC 4960 appendix B)". |
| RFC 4960 §3.3.3 | [`SctpHeaderSerializer.cc:1586-1587`](../../../../../src/inet/transportlayer/sctp/SctpHeaderSerializer.cc) | "RFC 4960 3.3.3: the INIT-ACK reports the parameters the peer's INIT carried and this end did not recognize". |
| RFC 6525, RFC 4895, RFC 5061, RFC 3758 | [`WHATSNEW:5205-5214`](../../../../../WHATSNEW) | The changelog entry "5. STCP received several new features" (sic, a misspelling of SCTP) lists, one item per line: "SCTP Stream Reset (RFC 6525)", "SCTP Authentication (RFC 4895)", "Add-IP feature for SCTP (RFC 5061)", "NR\_SACK feature to SCTP" (no document), "Partial Reliability SCTP (RFC 3758)", "SCTP packet drop feature (draft-stewart-sctp-pktdrprep-15.txt)", and "SCTP \"sack immediately\" feature (draft-ietf-tsvwg-sctp-sack-immediately)". |

No `.msg` file, and none of `SctpAssociationStreamReset.cc`, `SctpAssociationAddIp.cc`, or
`sctpapp/`, names a document. The four level-5 extensions the changelog names are claimed only
there, not at the place a reader of the module reference would look.

Mapped onto the standards map, [`standards.md`](../../protocol/sctp/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 9260 | yes, `base` | **no** | Not one comment, parameter, `.msg` field, or line of the user's guide names it. |
| RFC 4960 | no, obsoleted by RFC 9260 | **yes, pervasively** | Eight of the sixteen claim rows above name it; one of them is not a comment but an **enum identifier**, `RFC4960 = 0`, baked into the parameter surface (`ccModule`) and switched on throughout the congestion-control code. |
| RFC 2960 | no, obsoleted by RFC 4960, two generations behind RFC 9260 | **yes, once** | `SctpAssociationUtil.cc:2851`, a clause citation for the RTT-sampling rule. |
| RFC 4460, RFC 6096, RFC 7053, RFC 8540 | no, folded into RFC 9260 | no, except RFC 7053 by a draft name (see below) | Nothing names RFC 4460, RFC 6096 or RFC 8540. The WHATSNEW entry for "sack immediately" names the pre-RFC draft of what became RFC 7053, not the RFC. |
| RFC 6335 | no, out of scope | no | — |
| RFC 8899 | no, level 5 | no | — |
| RFC 3758, RFC 5061, RFC 4895, RFC 6525 | no, level 5 | **yes, in the changelog only** | See the row above; a claimed document outside the in-scope set is a scope gap for a later pass, not a verdict. |
| RFC 6951 | no, level 5 | no | The survey calls the NAT/UDP-hook code "RFC 6951-shaped"; the model itself names no document there. |
| RFC 6356 | no, level 5 | yes, in a comment | Sctp.ned:139, for the `lia` congestion-control variant. |
| RFC 3649 | no, level 5 | yes, in two comments | The HighSpeed CC variant names it twice. |
| RFC 1982 | out of scope always | yes, in a comment | Used correctly for a generic wraparound comparison, not a protocol-behavior claim. |
| NR-SACK, CMT | no, level 5, unstable | code exists, named informally | The changelog names "NR\_SACK feature to SCTP" with no document; CMT is named only through the model's own parameter and variant names (`CUCVariant`, `BufferSplitVariant`), never through a draft string in the source. |

**The finding of the survey.** The model's most repeated citation is the most obsolete one in
active use. `RFC4960` is not only quoted in comments; it is an **identifier**,
`enum SctpCcModules { RFC4960 = 0 }`, so the name of a document RFC 9260 obsoleted in 2022
is load-bearing in the parameter surface a user sets (`ccModule`,
[`Sctp.ned:61`](../../../../../src/inet/transportlayer/sctp/Sctp.ned)) and in the switch
statement the congestion-control code branches on
([`SctpAssociationBase.cc:1675`](../../../../../src/inet/transportlayer/sctp/SctpAssociationBase.cc)).
One citation reaches back a further generation, to RFC 2960 (obsoleted twice over,
`SctpAssociationUtil.cc:2851`). RFC 9260, the document that has governed since 2022, is named
nowhere: not in the module documentation, not in the user's guide, not in one comment among the
sixteen this survey found. A reader of the model has no way to learn that SCTP's actual base
document today is RFC 9260, and the checksum comments name an appendix letter, "appendix B",
that RFC 9260 renumbered to Appendix A when it folded the errata documents in
([`standards.md`](../../protocol/sctp/standards.md#override-table)).

The second half of the finding concerns the four extensions. RFC 3758, RFC 5061, RFC 4895 and
RFC 6525 are all implemented in depth — dedicated files, dedicated enums, and a matching
`.test` file each in `tests/module/` for three of the four — yet the only place any of the four
numbers appears is one changelog entry from when the feature first landed
([`WHATSNEW:5205-5214`](../../../../../WHATSNEW)). The changelog entry for "sack immediately"
names a pre-standardization draft; that feature is now part of the base document itself, as the
override table of `standards.md` records, so even the changelog's own citation is out of date
by two steps: draft, then RFC 7053, then folded into RFC 9260.

For the level 2 pass, three facts from the code (the guide permits this look to decide the
claim question, and the pass must check each one):

1. **`RFC4960` is an enum identifier, not only a comment.** A level 2 catalog that quotes
   "RFC 4960" from the code quotes a name chosen for a constant, and the constant's value
   (`New Reno`, the base congestion-control algorithm) is what RFC 9260 §7 also specifies.
   The naming is stale; nothing suggests the algorithm itself has drifted from the current
   text, and a later pass should check that directly against RFC 9260 §7, not assume it from
   the name.
2. **The I bit and the chunk-flags registry are base-document material now, not extensions.**
   A check of `RFC9260-CHUNK-*` for a DATA chunk's `I` bit belongs in the level 2 catalog
   under §3/§6.10, not in a level 5 extension catalog, even though the model's own changelog
   still calls "sack immediately" a feature added on top of the base.
3. **NR-SACK and CMT have no citable stable document.** Any later pass that reaches level 5
   for either one must cite an exact Internet-Draft revision and quote by its own clause
   numbers, per the guide's rule for a text without stable line numbers
   ([`standards.md`](../../protocol/sctp/standards.md#in-scope-set)).
