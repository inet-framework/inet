# RTP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around the Real-time Transport Protocol and its companion control protocol, RTCP, records
which document governs each contested clause, and pins the set that the next pass tests
against. RTP has one base document and one profile document that together define the normal
exchange; the base document also has ten later documents that update it, and the profile
document has three.

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 3550 §5 (the RTP header), §6.1 (the RTCP packet format), §6.4 (Sender and Receiver Reports), §6.5 (SDES, at least the CNAME item), §6.6 (BYE); RFC 3551 §6, the payload type 10 row (L16, 44,100 Hz, 2 channels) | the normal exchange: a data stream with a correct header, and the RTCP reports, source description and leave message that go with it, packetized with one payload format both base documents name |
| level 3, Edge | nothing new | §6.4.4 holds the fraction-lost and cumulative-lost computation, which needs an induced loss to check; a crafted or truncated RTCP packet exercises the packet-format rules of §6.1 |
| level 4, Dynamics | nothing new | §6.2 and §6.3 hold the RTCP transmission-interval algorithm, the SSRC timeout, and the reduced-interval rules — a control loop with several time constants |
| level 5, Complete | RFC 3550 §6.7 (APP), §7 (translators and mixers); every document that updates RFC 3550 or RFC 3551; RFC 2250 (a second payload format) | an application-defined packet, a topology with more than two endpoints, each later extension, and a payload format the normal path does not need |

What a pass actually reached is not recorded here. It is in
[`model/rtp/coverage.md`](../../model/rtp/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 3550 | RTP: A Transport Protocol for Real-Time Applications | July 2003 | Draft Standard at publication, Internet Standard now | `base` | [`standards/RFC/rfc3550.txt`](../../../../../../standards/RFC/rfc3550.txt), 2026-09-23 |
| RFC 3551 | RTP Profile for Audio and Video Conferences with Minimal Control | July 2003 | Draft Standard at publication, Internet Standard now | `base`, the profile RFC 3550 needs to be usable | [`standards/RFC/rfc3551.txt`](../../../../../../standards/RFC/rfc3551.txt), 2026-09-23 |
| RFC 1889 | RTP: A Transport Protocol for Real-Time Applications | January 1996 | Proposed Standard | `obsoleted by` RFC 3550 | no |
| RFC 1890 | RTP Profile for Audio and Video Conferences with Minimal Control | January 1996 | Proposed Standard | `obsoleted by` RFC 3551 | no |
| RFC 2250 | RTP Payload Format for MPEG1/MPEG2 Video | January 1998 | Proposed Standard | `companion`, one specific payload format; obsoletes RFC 2038, no `updated_by` | no |
| RFC 5506 | Support for Reduced-Size Real-Time Transport Control Protocol (RTCP): Opportunities and Consequences | April 2009 | Proposed Standard | `updates` RFC 3550 | no |
| RFC 5761 | Multiplexing RTP Data and Control Packets on a Single Port | April 2010 | Proposed Standard | `updates` RFC 3550, RFC 3551 | no |
| RFC 6051 | Rapid Synchronisation of RTP Flows | November 2010 | Proposed Standard | `updates` RFC 3550 | no |
| RFC 6222 | Guidelines for Choosing RTP Control Protocol (RTCP) Canonical Names (CNAMEs) | April 2011 | Proposed Standard | `updates` RFC 3550; `obsoleted by` RFC 7022 | no |
| RFC 7022 | Guidelines for Choosing RTP Control Protocol (RTCP) Canonical Names (CNAMEs) | September 2013 | Proposed Standard | `updates` RFC 3550; obsoletes RFC 6222 | no |
| RFC 7160 | Support for Multiple Clock Rates in an RTP Session | April 2014 | Proposed Standard | `updates` RFC 3550 | no |
| RFC 7164 | RTP and Leap Seconds | March 2014 | Proposed Standard | `updates` RFC 3550 | no |
| RFC 8083 | Multimedia Congestion Control: Circuit Breakers for Unicast RTP Sessions | March 2017 | Proposed Standard | `updates` RFC 3550 | no |
| RFC 8108 | Sending Multiple RTP Streams in a Single RTP Session | March 2017 | Proposed Standard | `updates` RFC 3550 | no |
| RFC 8860 | Sending Multiple Types of Media in a Single RTP Session | January 2021 | Proposed Standard | `updates` RFC 3550, RFC 3551 | no |
| RFC 7007 | Update to Remove DVI4 from the Recommended Codecs for the RTP Profile for Audio and Video Conferences with Minimal Control (RTP/AVP) | August 2013 | Proposed Standard | `updates` RFC 3551 | no |

Source of the texts:

- `rfc3550.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3550.txt) —
  <https://www.rfc-editor.org/rfc/rfc3550.txt>, downloaded 2026-09-23.
- `rfc3551.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc3551.txt) —
  <https://www.rfc-editor.org/rfc/rfc3551.txt>, downloaded 2026-09-23.

Both base documents were published as Draft Standard, one step short of full Internet
Standard under the process in force in 2003; the register's current `status` field
(distinct from the `pub_status` field this pass fetched) shows both have since reached
Internet Standard, without a text change. RFC 1889 and
RFC 1890 — the documents this protocol's own model names, see
[`conformance.md`](../../model/rtp/conformance.md#part-1--the-claims) — were both obsoleted in
2003 by RFC 3550 and RFC 3551, over twenty years before this pass.

RFC 3550 uses the keywords of RFC 2119, with the boilerplate at `rfc3550.txt:267`. `MUST` is on
82 lines, `SHOULD` on 87, `MAY` on 85, `SHALL` on 1. RFC 3551 also uses them, with the
boilerplate at `rfc3551.txt:156`. `MUST` is on 21 lines, `SHOULD` on 30, `MAY` on 31, `SHALL`
on 12.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The RTP header fields and the RTP/RTCP packet formats | RFC 1889 (1996) defined them first | RFC 3550 §5 and §6, `rfc3550.txt:679-2936`, restates and obsoletes RFC 1889 in full | RFC 3550 | yes, level 2 |
| The audio/video profile and its payload type table | RFC 1890 (1996) defined it first | RFC 3551 §6, `rfc3551.txt:1756-1830`, restates and obsoletes RFC 1890 in full | RFC 3551 | yes, level 2, for payload type 10 |
| The RTCP CNAME construction rule | Neither base document mandates a specific algorithm to derive the CNAME SDES item; RFC 3550 §6.5.1 gives examples only | RFC 6222 (2011), then RFC 7022 (2013) after RFC 6222 was itself obsoleted, gives a normative algorithm | RFC 7022, when this area enters scope | no; level 5 |

No clause of any later document contradicts a clause of RFC 3550 or RFC 3551 within the level-2
slice above; every one of the ten later documents adds an optional mechanism instead.

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 3550 | July 2003, Draft Standard at publication, Internet Standard now; §5, §6.1, §6.4, §6.5 (at least §6.5.1), §6.6 | none yet; level 2 writes `standard/rfc3550/catalog.md` |
| RFC 3551 | July 2003, Draft Standard at publication, Internet Standard now; §6, the payload type 10 row | none yet; level 2 writes `standard/rfc3551/catalog.md` |

Out of scope, with the reason:

| Document or clause | Level | Reason |
| --- | --- | --- |
| RFC 3550 §6.2, §6.3 | 4 | The RTCP transmission-interval algorithm, the SSRC timeout, and the reduced-interval rules are a control loop with several time constants, not a single value. |
| RFC 3550 §6.7 | 5 | An application-defined RTCP packet is an optional extension point, not part of the normal exchange. |
| RFC 3550 §7 | 5 | A translator or a mixer is a third kind of node in the topology; the normal exchange of level 2 needs only a sender and a receiver. |
| RFC 2250 | 5 | MPEG1/MPEG2 video is one specific payload encoding among the many RFC 3551 and its companions allow. The normal exchange of level 2 needs one payload format, and RFC 3551 §6 already supplies one (payload type 10) without it. |
| RFC 5506, RFC 6051, RFC 7160, RFC 7164, RFC 8083, RFC 8108, RFC 8860 | 5 | Each adds one optional mechanism on top of the base exchange: a reduced-size RTCP packet, faster cross-stream synchronization, more than one clock rate in a session, a leap-second adjustment, a congestion-control circuit breaker, more than one RTP stream in a session, and more than one media type in a session. None of the seven is needed to send and receive one RTP stream with its RTCP reports. |
| RFC 5761, RFC 8860 (the RFC 3551 half) | 5 | Multiplexing RTP and RTCP on one port, and sending more than one media type in one session, are both optional changes to the profile, not the default it defines. |
| RFC 6222, RFC 7022 | 5 | A normative CNAME-construction algorithm is a refinement of one SDES item; the base documents already define the item and give example algorithms. See the override table. |
| RFC 7007 | out of scope always | Removes one codec, DVI4, from the profile's recommended list; it changes no wire format and states no requirement the level 2 slice needs. |
