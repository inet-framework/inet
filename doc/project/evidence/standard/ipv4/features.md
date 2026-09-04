# IPv4 — feature map and support matrix

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `IPV4-F-*` · **Stands on:** [standards.md](standards.md), [rfc791-checklist.md](rfc791-checklist.md), [rfc792-checklist.md](rfc792-checklist.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and
per document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level
of each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. The support column comes from the
  run verdicts of [`rfc791-results.md`](rfc791-results.md), by the rule of step 7.

The feature list itself comes from the standard texts only. Only the support column comes
from a run. The comparison of the support column against the standards that the model
claims to implement is [`conformance.md`](conformance.md).

## Summary table

Support state: run of 2026-09-02, INET tree at docs commit `6208e77255`.

| Feature | Level | Sources | Core checks | Support |
| --- | --- | --- | --- | --- |
| [IPV4-F-TTL](#ipv4-f-ttl) | mandatory | RFC 791 §3.1, §3.2 | RFC791-TTL-1, RFC791-TTL-2 | **partial** |
| [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation) | mandatory | RFC 791 §2.3, §3.2 | RFC791-FRAG-1..4 | **supported** |
| [IPV4-F-REASSEMBLY](#ipv4-f-reassembly) | unstated | RFC 791 §2.3 | RFC791-REASM-1 | **supported** |
| [IPV4-F-DONT-FRAGMENT](#ipv4-f-dont-fragment) | mandatory | RFC 791 §2.3 | RFC791-FRAG-5 | **supported** |
| [IPV4-F-IDENTIFICATION](#ipv4-f-identification) | mandatory | RFC 791 §2.3 | RFC791-ID-1, RFC791-FRAG-3 | **partial** |
| [IPV4-F-HEADER-CHECKSUM](#ipv4-f-header-checksum) | unstated | RFC 791 §1.4, §3.1 | RFC791-CKSUM-1, RFC791-CKSUM-2 | **untested** |
| [IPV4-F-MIN-SIZE](#ipv4-f-min-size) | mandatory | RFC 791 §3.1, §3.2 | RFC791-FRAG-6, RFC791-REASM-2 | **untested** |
| [IPV4-F-ERROR-REPORT](#ipv4-f-error-report) | optional | RFC 792 | RFC792-DU-4, RFC792-TE-1 | **partial** |

Three of eight features are `supported`, three are `partial`, and two are `untested`. No
feature is `not supported`: pass 1 found no model gap.

Two levels read `unstated`, and both point at the same thing. RFC 791 predates RFC 2119 and
writes those two capabilities as plain description. RFC 1122 §3.2.1 restates the IPv4 host
rules with MUST and SHOULD keywords, so the level of these two features will change on the
day RFC 1122 enters the in-scope set. The rows wait in
[`standards.md`](standards.md#override-table).

## IPV4-F-TTL

**A datagram carries a time to live, every hop decreases it, and a datagram that reaches
zero dies.**

- **Sources** — RFC 791 §3.2 Time to Live, `rfc791.txt:1976-1984`; §3.1 field definition,
  `rfc791.txt:1013-1014`. No later in-scope document changes them.
- **Level** — mandatory. "This field must be decreased at each point that the internet
  header is processed" and "If this field contains the value zero, then the datagram must
  be destroyed" (`rfc791.txt:1981-1982`, `rfc791.txt:1013-1014`).
- **Description** — the field bounds the lifetime of a datagram in the network and stops a
  datagram that circulates in a loop.
- **Checks** — core: RFC791-TTL-1 (the decrease per hop), RFC791-TTL-2 (the death at
  zero). Supporting: RFC791-TTL-3 (the sender sets the field).
- **Support** — **partial**. RFC791-TTL-1 passed and RFC791-TTL-3 passed; RFC791-TTL-2 is
  a catalog candidate and no test ran it. The death at zero is therefore unproven, not
  disproven. The next pass closes this with a TTL 1 variant of the same mockup.

## IPV4-F-FRAGMENTATION

**A module that must forward a datagram larger than the MTU of the next link splits it
into fragments that carry the fragment fields of the original datagram.**

- **Sources** — RFC 791 §2.3 Fragmentation, `rfc791.txt:662-731`; §3.2,
  `rfc791.txt:1656-1671`.
- **Level** — mandatory. "If an internet datagram is fragmented, its data portion must be
  broken on 8 octet boundaries" (`rfc791.txt:1658-1659`).
- **Description** — the split preserves the identification, marks every fragment but the
  last with the more-fragments flag, and places each piece with an offset in 8-octet
  units.
- **Checks** — core: RFC791-FRAG-1 (zero fragmentation information when whole),
  RFC791-FRAG-2 (the 8-octet boundary), RFC791-FRAG-3 (the identification is copied),
  RFC791-FRAG-4 (the offset and flag pattern of the first and last fragment).
- **Support** — **supported**. All four core checks ran in `Rfc791FragmentReassembly.test`
  and passed. The bound of the statement: the check uses one datagram size and one MTU. A
  size sweep, and the total-length assertions that
  [`rfc791-results.md`](rfc791-results.md#sharpening-candidates-for-the-next-pass) lists,
  would widen it.

## IPV4-F-REASSEMBLY

**The destination collects the fragments of one datagram and delivers the original
datagram upward.**

- **Sources** — RFC 791 §2.3, `rfc791.txt:723-729`.
- **Level** — unstated. The RFC writes the procedure as description: "The combination is
  done by placing the data portion of each fragment in the relative position indicated by
  the fragment offset". RFC 1122 §3.3.2 `Reassembly` states the host obligation with a
  keyword, but RFC 1122 is out of scope today.
- **Description** — the four fields identification, source, destination, and protocol
  select the fragments that belong together.
- **Checks** — core: RFC791-REASM-1. Supporting: RFC791-REASM-3 (fragments of different
  datagrams do not mix).
- **Support** — **supported**. RFC791-REASM-1 passed: the destination delivered the whole
  1008-octet datagram upward, and the sink application received the 1000-octet payload.
  RFC791-REASM-3 has status `later` and did not run; it is supporting, so it does not
  lower the value. The separation of two interleaved fragment trains is therefore not
  established.

## IPV4-F-DONT-FRAGMENT

**A datagram marked don't fragment is never fragmented; a module that would have to
fragment it discards it instead.**

- **Sources** — RFC 791 §2.3, `rfc791.txt:662-666`.
- **Level** — mandatory. The RFC uses obligation words without a keyword: "Any internet
  datagram so marked is not to be internet fragmented under any circumstances" and "it is
  to be discarded instead".
- **Description** — the flag lets a sender forbid the split, at the price of the datagram
  when the path cannot carry it whole.
- **Checks** — core: RFC791-FRAG-5. The error report that follows the discard belongs to
  [IPV4-F-ERROR-REPORT](#ipv4-f-error-report), not to this feature: the discard is
  mandatory and the report is not.
- **Support** — **supported**. RFC791-FRAG-5 passed in `Rfc791DontFragment.test`: no
  fragment appeared after the gateway and the destination received nothing.

## IPV4-F-IDENTIFICATION

**The sender gives each datagram an identification value that is unique for its source,
destination, and protocol while the datagram lives, and every fragment keeps it.**

- **Sources** — RFC 791 §2.3, `rfc791.txt:683-687` (uniqueness) and
  `rfc791.txt:692-694` (the copy into each fragment).
- **Level** — mandatory. "sets the identification field to a value that must be unique for
  that source-destination pair and protocol for the time the datagram will be active".
- **Description** — the field is what makes reassembly possible; without uniqueness the
  destination mixes the fragments of two datagrams.
- **Checks** — core: RFC791-ID-1 (uniqueness across datagrams), RFC791-FRAG-3 (the value
  survives the split). RFC791-FRAG-3 is core for this feature and for
  [IPV4-F-FRAGMENTATION](#ipv4-f-fragmentation); one check may serve two features.
- **Support** — **partial**. RFC791-FRAG-3 passed: all fragments of the datagram showed
  one identification value. RFC791-ID-1 is a candidate and did not run, so uniqueness
  across two successive datagrams is unproven.
- **Note for a later pass** — RFC 6864 §4.1 and §4.3 rewrite this requirement. Bring the
  document into scope together with the RFC791-ID-1 test, so the test targets the
  governing text and not the 1981 text.

## IPV4-F-HEADER-CHECKSUM

**The header carries a checksum, every module that processes the header recomputes and
verifies it, and a header that fails is discarded.**

- **Sources** — RFC 791 §3.1, `rfc791.txt:1031-1033`; §1.4, `rfc791.txt:365-366`.
- **Level** — unstated. Both sentences are description without a keyword: "this is
  recomputed and verified at each point that the internet header is processed" and "the
  internet datagram is discarded at once by the entity which detects the error". RFC 1122
  §3.2.1 gives the IPv4 host rules their keywords.
- **Description** — the checksum protects the header only, and it must change whenever a
  field of the header changes, for example the time to live.
- **Checks** — core: RFC791-CKSUM-1 (the recompute per hop), RFC791-CKSUM-2 (the discard
  of a bad header).
- **Support** — **untested**. No core check ran. RFC791-CKSUM-1 is a candidate that pairs
  well with the existing TTL mockup: the checksum must change across the same hop where
  the TTL changes. RFC791-CKSUM-2 needs the corruption of a datagram in flight, which the
  toolset does not yet do.

## IPV4-F-MIN-SIZE

**Every module forwards a 68-octet datagram without fragmentation, and every host accepts
a 576-octet datagram.**

- **Sources** — RFC 791 §3.2, `rfc791.txt:1669-1671` (the 68-octet floor); §3.1,
  `rfc791.txt:961-963` (the 576-octet floor).
- **Level** — mandatory. Both sentences use "must".
- **Description** — the two floors are what let a sender assume a minimum service from an
  unknown path.
- **Checks** — core: RFC791-FRAG-6 (68 octets pass without a split), RFC791-REASM-2 (576
  octets arrive, whole or in fragments).
- **Support** — **untested**. Both checks are catalog candidates. The run of pass 1 did
  carry a 1008-octet datagram end to end, which exceeds the 576-octet floor, but no check
  claims that observation, so the feature stays untested. This is the rule working as
  intended: a value in the support column must stand on a check, not on a side effect of
  another scenario.

## IPV4-F-ERROR-REPORT

**A module that discards a datagram may report the failure to the source with an ICMP
message.**

- **Sources** — RFC 792, `rfc792.txt:261-264` and the code list at `rfc792.txt:215`
  (destination unreachable, code 4); `rfc792.txt:344-356` (time exceeded).
- **Level** — optional. In both messages the discard is a must and the report is a may:
  "the gateway must discard the datagram and may return a destination unreachable
  message". The mandatory half of each pair belongs to the RFC 791 feature that owns the
  discard.
- **Description** — the report turns a silent drop into a signal that the source can act
  on. The RFC permits silence, so the absence of a report is not a violation.
- **Checks** — core: RFC792-DU-4 (the report after a don't-fragment discard), RFC792-TE-1
  (the report after a TTL-zero discard).
- **Support** — **partial**. RFC792-DU-4 passed: the model sent type 3, code 4 toward the
  source. RFC792-TE-1 did not run.
- **Note on the strength** — a `partial` value on an optional feature is not a defect. The
  model may stay silent and still conform. The value states what the checks saw, and
  [`conformance.md`](conformance.md) reads it together with the level.

## Coverage of the catalogs

Step 4 requires that every area of every in-scope catalog appears in at least one feature,
or that this map marks the area as out of scope.

| Catalog area | Feature |
| --- | --- |
| RFC 791, Time to live | IPV4-F-TTL |
| RFC 791, Fragmentation | IPV4-F-FRAGMENTATION, IPV4-F-DONT-FRAGMENT, IPV4-F-MIN-SIZE, IPV4-F-IDENTIFICATION |
| RFC 791, Reassembly | IPV4-F-REASSEMBLY, IPV4-F-MIN-SIZE |
| RFC 791, Header checksum | IPV4-F-HEADER-CHECKSUM |
| RFC 791, Identification | IPV4-F-IDENTIFICATION |
| RFC 792, Error signals | IPV4-F-ERROR-REPORT |

All 17 catalog entries of the in-scope set appear in the map, and no entry appears in none.

Out of scope in the map, because the catalogs put them out of scope: options, type of
service and precedence, the security annex, the reassembly timer, and the ICMP messages
other than the two error reports. Each becomes a feature on the day its catalog entries
exist.
