# RFC 9776 (Internet Group Management Protocol, Version 3) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC9776-*` · **Stands on:** [standards.md](../../protocol/igmp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 9776, Internet Group Management Protocol, Version 3, of
March 2025, a document of the in-scope set of IGMP. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc9776.txt`](../../../../../../standards/RFC/rfc9776.txt) —
  Internet Group Management Protocol, Version 3, March 2025. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc9776.txt>.

The other document of the in-scope set has a catalog of its own:
[`rfc2236/catalog.md`](../rfc2236/catalog.md) (IGMPv2, whose host and router state diagrams the
compatibility rules of §7 fall back to).

The in-scope sections are §4 to §8. RFC 9776 is the 2025 revision of RFC 3376; its Appendix C
names the changes, two of which change timer formulas that §8 of this catalog quotes.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/igmp/standards.md). The features these statements build are in
[`features.md`](../../protocol/igmp/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`igmp/coverage.md`](../../model/igmp/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc9776.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC9776-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 9776 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them, and
  `description` for a normative sentence with no keyword. Where the text writes a keyword in
  lower case, the entry records the word and says so, for example `must (lower case)`; a
  "might" stays `description`. An entry with two keywords names what each one covers. A row of
  a state table that states an action is an entry of its own.
- **Class** — how a test can observe the statement: `wire` (fields of messages on a link, or
  the presence or absence of a message), `end-to-end` (what a node accepts and acts on, seen
  in what it sends or forwards afterwards), `error-signal` (an error report),
  `internal` (state inside a node), `encoding` (the exact bit layout).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC9776-GEN-1](#rfc9776-gen-1) | An IGMP message is encapsulated in an IPv4 datagram with IP protocol number 2. |
| [RFC9776-GEN-2](#rfc9776-gen-2) | A system sends every IGMP message with an IP Time-to-Live of 1. |
| [RFC9776-GEN-3](#rfc9776-gen-3) | A system sends every IGMP message with IP Precedence Internetwork Control. |
| [RFC9776-GEN-4](#rfc9776-gen-4) | A system sends every IGMP message with an IP Router Alert option. |
| [RFC9776-GEN-5](#rfc9776-gen-5) | A Membership Query message carries type value 0x11. |
| [RFC9776-GEN-6](#rfc9776-gen-6) | An IGMPv3 Membership Report message carries type value 0x22. |
| [RFC9776-GEN-7](#rfc9776-gen-7) | A system MUST support the IGMPv1 Membership Report message, type 0x12, for interoperation with an older IGMP version. |
| [RFC9776-GEN-8](#rfc9776-gen-8) | A system MUST support the IGMPv2 Membership Report message, type 0x16, for interoperation with an older IGMP version. |
| [RFC9776-GEN-9](#rfc9776-gen-9) | A system MUST support the IGMPv2 Leave Group message, type 0x17, for interoperation with an older IGMP version. |
| [RFC9776-GEN-10](#rfc9776-gen-10) | A system MUST silently ignore an unrecognized IGMP message type. |
| [RFC9776-QRY-1](#rfc9776-qry-1) | A multicast router sends a Membership Query to learn the multicast reception state of a neighboring interface. |
| [RFC9776-QRY-2](#rfc9776-qry-2) | The Max Resp Code field states the maximum time allowed before a system sends its responding report. |
| [RFC9776-QRY-3](#rfc9776-qry-3) | A Max Resp Code below 128 equals the Max Response Time directly. |
| [RFC9776-QRY-4](#rfc9776-qry-4) | A Max Resp Code of 128 or more codes the Max Response Time as a floating-point value: the mantissa combined with 0x10, shifted left by the exponent plus 3. |
| [RFC9776-QRY-5](#rfc9776-qry-5) | The Checksum field is the 16-bit one's complement of the one's complement sum of the whole IGMP message, computed with the Checksum field itself set to zero. |
| [RFC9776-QRY-6](#rfc9776-qry-6) | A system MUST verify the checksum of a received Query before it processes the packet. |
| [RFC9776-QRY-7](#rfc9776-qry-7) | The Group Address field is zero when a router sends a General Query. |
| [RFC9776-QRY-8](#rfc9776-qry-8) | The Group Address field holds the queried multicast address when a router sends a Group-Specific or Group-and-Source Specific Query. |
| [RFC9776-QRY-9](#rfc9776-qry-9) | The Flags field is a bitstring managed by the IGMP Type Numbers registry. |
| [RFC9776-QRY-10](#rfc9776-qry-10) | When the S flag is set to one, a receiving router suppresses its normal timer updates for that Query. |
| [RFC9776-QRY-11](#rfc9776-qry-11) | The S flag does not suppress Querier election or a router's own host-side processing of the Query. |
| [RFC9776-QRY-12](#rfc9776-qry-12) | The QRV field carries the querier's Robustness Variable, when the field is non-zero. |
| [RFC9776-QRY-13](#rfc9776-qry-13) | A querier whose Robustness Variable exceeds 7 sets the QRV field to zero. |
| [RFC9776-QRY-14](#rfc9776-qry-14) | A router adopts the QRV value of the most recently received Query as its own Robustness Variable, or the default value when that QRV was zero. |
| [RFC9776-QRY-15](#rfc9776-qry-15) | The QQIC field states the Query Interval used by the querier. |
| [RFC9776-QRY-16](#rfc9776-qry-16) | A QQIC below 128 equals the QQI directly. |
| [RFC9776-QRY-17](#rfc9776-qry-17) | A QQIC of 128 or more codes the Querier's Query Interval as a floating-point value: the mantissa combined with 0x10, shifted left by the exponent plus 3. |
| [RFC9776-QRY-18](#rfc9776-qry-18) | A multicast router that is not the current querier adopts the QQI of the most recently received Query as its own Query Interval, or the default value when that QQI was zero. |
| [RFC9776-QRY-19](#rfc9776-qry-19) | The Number of Sources field states how many source addresses are present in the Query. |
| [RFC9776-QRY-20](#rfc9776-qry-20) | The Number of Sources field is zero in a General Query or a Group Specific Query, and non-zero in a Group-and-Source Specific Query. |
| [RFC9776-QRY-21](#rfc9776-qry-21) | The Number of Sources is limited by the MTU of the network over which the Query is transmitted. |
| [RFC9776-QRY-22](#rfc9776-qry-22) | The Source Address fields of a Query are a vector of source addresses, one for each count in the Number of Sources field. |
| [RFC9776-QRY-23](#rfc9776-qry-23) | On a received Query with extra octets beyond the defined fields, a system MUST include those octets when it verifies the IGMP Checksum, but MUST otherwise ignore them. |
| [RFC9776-QRY-24](#rfc9776-qry-24) | When a system sends a Query, it MUST NOT include additional octets beyond the fields described in the format. |
| [RFC9776-QRY-25](#rfc9776-qry-25) | A General Query is sent to learn the complete multicast reception state of the neighboring interfaces, with the Group Address and Number of Sources fields both zero. |
| [RFC9776-QRY-26](#rfc9776-qry-26) | A Group Specific Query is sent to learn the reception state of one multicast address, with the Group Address field holding that address and the Number of Sources field zero. |
| [RFC9776-QRY-27](#rfc9776-qry-27) | A Group-and-Source Specific Query is sent to learn if any neighboring interface wants packets sent to a specified address from a specified list of sources, and it carries the address and the sources. |
| [RFC9776-QRY-28](#rfc9776-qry-28) | A General Query is sent to the IP destination address 224.0.0.1, the all-systems multicast address. |
| [RFC9776-QRY-29](#rfc9776-qry-29) | A Group Specific or Group-and-Source Specific Query is sent to an IP destination address equal to the multicast address of interest. |
| [RFC9776-QRY-30](#rfc9776-qry-30) | A system MUST accept and process a Query whose IP Destination Address field contains any address assigned to the interface on which the Query arrives. |
| [RFC9776-REP-1](#rfc9776-rep-1) | A system sends an IGMPv3 Membership Report to tell neighboring routers its current or changed multicast reception state. |
| [RFC9776-REP-2](#rfc9776-rep-2) | A system sets the Reserved field of a Report to zero when it sends the Report. |
| [RFC9776-REP-3](#rfc9776-rep-3) | A system ignores the Reserved field of a Report it receives. |
| [RFC9776-REP-4](#rfc9776-rep-4) | The Checksum field is the 16-bit one's complement of the one's complement sum of the whole IGMP message, computed with the Checksum field itself set to zero. |
| [RFC9776-REP-5](#rfc9776-rep-5) | A system MUST verify the checksum of a received Report before it processes the message. |
| [RFC9776-REP-6](#rfc9776-rep-6) | The Flags field is a bitstring managed by the IGMP Type Numbers registry. |
| [RFC9776-REP-7](#rfc9776-rep-7) | The Number of Group Records field states how many Group Records are present in the Report. |
| [RFC9776-REP-8](#rfc9776-rep-8) | A Group Record holds information about the sender's membership in one multicast group on the interface from which the Report is sent. |
| [RFC9776-REP-9](#rfc9776-rep-9) | The Aux Data Len field states the length of the Auxiliary Data field in 32-bit words, and zero means there is no auxiliary data. |
| [RFC9776-REP-10](#rfc9776-rep-10) | The Number of Sources field states how many source addresses are present in this Group Record. |
| [RFC9776-REP-11](#rfc9776-rep-11) | The Multicast Address field holds the IP multicast address the Group Record pertains to. |
| [RFC9776-REP-12](#rfc9776-rep-12) | The Source Address fields of a Group Record are a vector of source addresses, one for each count in this record's Number of Sources field. |
| [RFC9776-REP-13](#rfc9776-rep-13) | The Auxiliary Data field, when present, holds extra information about the Group Record, but IGMPv3 defines no auxiliary data. |
| [RFC9776-REP-14](#rfc9776-rep-14) | A system MUST NOT include auxiliary data in a Group Record it transmits, and MUST set the Aux Data Len field to zero. |
| [RFC9776-REP-15](#rfc9776-rep-15) | A system MUST ignore any auxiliary data present in a Group Record it receives. |
| [RFC9776-REP-16](#rfc9776-rep-16) | On a received Report with extra octets beyond the last Group Record, a system MUST include those octets when it verifies the IGMP Checksum, but MUST otherwise ignore them. |
| [RFC9776-REP-17](#rfc9776-rep-17) | When a system sends a Report, it MUST NOT include additional octets beyond the last Group Record. |
| [RFC9776-REP-18](#rfc9776-rep-18) | A system sends a Current-State Record in a Report sent in response to a Query, to state its current reception state for one multicast address. |
| [RFC9776-REP-19](#rfc9776-rep-19) | A Current-State Record of type MODE_IS_INCLUDE means the interface's filter-mode is INCLUDE for the address, and the Source Address fields list the interface's non-empty source list. |
| [RFC9776-REP-20](#rfc9776-rep-20) | A Current-State Record of type MODE_IS_EXCLUDE means the interface's filter-mode is EXCLUDE for the address, and the Source Address fields list the interface's non-empty source list. |
| [RFC9776-REP-21](#rfc9776-rep-21) | An SSM-aware host SHOULD NOT send a MODE_IS_EXCLUDE record for a multicast address within the SSM address range, because SSM-aware routers ignore it. |
| [RFC9776-REP-22](#rfc9776-rep-22) | A system sends a Filter-Mode-Change Record, in the Report from the interface where the change occurred, whenever a local request changes the interface's filter-mode between INCLUDE and EXCLUDE. |
| [RFC9776-REP-23](#rfc9776-rep-23) | CHANGE_TO_INCLUDE_MODE means the interface has changed to INCLUDE filter-mode for the address, and the Source Address fields list the new non-empty source list. |
| [RFC9776-REP-24](#rfc9776-rep-24) | CHANGE_TO_EXCLUDE_MODE means the interface has changed to EXCLUDE filter-mode for the address, and the Source Address fields list the new non-empty source list. |
| [RFC9776-REP-25](#rfc9776-rep-25) | An SSM-aware host SHOULD NOT send a CHANGE_TO_EXCLUDE_MODE record for a multicast address within the SSM address range. |
| [RFC9776-REP-26](#rfc9776-rep-26) | A system sends a Source-List-Change Record, in the Report from the interface where the change occurred, whenever a local request changes the interface's source list without changing its filter-mode. |
| [RFC9776-REP-27](#rfc9776-rep-27) | ALLOW_NEW_SOURCES lists, in the Source Address fields, the sources newly added to an INCLUDE list or removed from an EXCLUDE list. |
| [RFC9776-REP-28](#rfc9776-rep-28) | BLOCK_OLD_SOURCES lists, in the Source Address fields, the sources removed from an INCLUDE list or added to an EXCLUDE list. |
| [RFC9776-REP-29](#rfc9776-rep-29) | When a source-list change both allows new sources and blocks old sources for the same address, a system sends two Group Records for it: one ALLOW_NEW_SOURCES and one BLOCK_OLD_SOURCES. |
| [RFC9776-REP-30](#rfc9776-rep-30) | A system MUST silently ignore an unrecognized Group Record Type value. |
| [RFC9776-REP-31](#rfc9776-rep-31) | A Report is sent with a valid unicast IPv4 source address for the destination subnet. |
| [RFC9776-REP-32](#rfc9776-rep-32) | A system that has not yet acquired an IP address MAY use 0.0.0.0 as its Report's source address. |
| [RFC9776-REP-33](#rfc9776-rep-33) | A router MUST accept a Report with a source address of 0.0.0.0. |
| [RFC9776-REP-34](#rfc9776-rep-34) | A Version 3 Report is sent to the IP destination address 224.0.0.22, to which every IGMPv3-capable multicast router listens. |
| [RFC9776-REP-35](#rfc9776-rep-35) | A system operating in v1 or v2 compatibility mode sends its Report to the multicast group named in the Report's Group Address field. |
| [RFC9776-REP-36](#rfc9776-rep-36) | A system MUST accept and process a v1 or v2 Report whose IP Destination Address field contains any address assigned to the interface on which the Report arrives. |
| [RFC9776-REP-37](#rfc9776-rep-37) | When the Group Records required in a Report do not fit within the size limit of a single Report message, a system sends them in as many Report messages as needed. |
| [RFC9776-REP-38](#rfc9776-rep-38) | When a single Group Record with a type other than MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE has too many source addresses for one message, a system splits it into several Group Records, each sent in a separate Report message. |
| [RFC9776-REP-39](#rfc9776-rep-39) | When a single MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE Group Record has too many source addresses for one message, a system sends one Group Record with as many source addresses as fit and leaves out the rest. |
| [RFC9776-HOST-1](#rfc9776-host-1) | A group member runs the group-member protocol on every interface that supports multicast reception, even when two interfaces reach the same network. |
| [RFC9776-HOST-2](#rfc9776-host-2) | A group member keeps a MulticastRouterVersion variable for each interface that supports multicast reception. |
| [RFC9776-HOST-3](#rfc9776-host-3) | Every system permanently accepts packets to 224.0.0.1 from any source, on every interface that supports multicast reception. |
| [RFC9776-HOST-4](#rfc9776-host-4) | A group member never sends an IGMP message about the all-systems multicast address. |
| [RFC9776-HOST-5](#rfc9776-host-5) | A group member silently ignores a received IGMP message that is not a Query, except as older-version interoperation requires. |
| [RFC9776-HOST-6](#rfc9776-host-6) | A change in an interface's multicast reception state makes the host send a State-Change Report from that interface right away. |
| [RFC9776-HOST-7](#rfc9776-host-7) | A missing interface state, before or after a change, counts as INCLUDE mode with an empty source list. |
| [RFC9776-HOST-8](#rfc9776-host-8) | A change from INCLUDE(A) to INCLUDE(B) reports ALLOW of the added sources and BLOCK of the removed sources. |
| [RFC9776-HOST-9](#rfc9776-host-9) | A change from EXCLUDE(A) to EXCLUDE(B) reports ALLOW of the sources removed from exclusion and BLOCK of the sources added to exclusion. |
| [RFC9776-HOST-10](#rfc9776-host-10) | A change from INCLUDE(A) to EXCLUDE(B) reports one TO_EX record with source list B. |
| [RFC9776-HOST-11](#rfc9776-host-11) | A change from EXCLUDE(A) to INCLUDE(B) reports one TO_IN record with source list B. |
| [RFC9776-HOST-12](#rfc9776-host-12) | A host leaves an ALLOW or BLOCK record with an empty source list out of the Report message. |
| [RFC9776-HOST-13](#rfc9776-host-13) | A host retransmits a State-Change Report [Robustness Variable]-1 more times, at random intervals within [Unsolicited Report Interval]. |
| [RFC9776-HOST-14](#rfc9776-host-14) | A further change to the same interface-state entry, before earlier retransmissions finish, sends a new State-Change Report right away. |
| [RFC9776-HOST-15](#rfc9776-host-15) | The difference records for a later change are merged into the still-pending report instead of being sent as their own message. |
| [RFC9776-HOST-16](#rfc9776-host-16) | Sending the merged State-Change Report stops the earlier report's retransmissions and starts a fresh count of [Robustness Variable] transmissions. |
| [RFC9776-HOST-17](#rfc9776-host-17) | A host keeps retransmission state for a source until it has sent [Robustness Variable] State-Change Reports that include the source. |
| [RFC9776-HOST-18](#rfc9776-host-18) | A filter-mode change makes the next [Robustness Variable] State-Change Reports carry a Filter-Mode-Change Record, whatever source-list changes happen meanwhile. |
| [RFC9776-HOST-19](#rfc9776-host-19) | A host keeps retransmission state for the group until it has sent the [Robustness Variable] State-Change Reports for the filter-mode change. |
| [RFC9776-HOST-20](#rfc9776-host-20) | Once the Filter-Mode-Change Records are exhausted, a still-pending source-list change makes the next report carry Source-List-Change Records instead. |
| [RFC9776-HOST-21](#rfc9776-host-21) | A retransmitted Filter-Mode-Change Record is a TO_IN record when the current filter-mode is INCLUDE, and a TO_EX record otherwise. |
| [RFC9776-HOST-22](#rfc9776-host-22) | A retransmitted Source-List-Change Record carries an ALLOW record and a BLOCK record. |
| [RFC9776-HOST-23](#rfc9776-host-23) | A retransmitted TO_IN record lists every source in the current interface state that must be forwarded. |
| [RFC9776-HOST-24](#rfc9776-host-24) | A retransmitted TO_EX record lists every source in the current interface state that must be blocked. |
| [RFC9776-HOST-25](#rfc9776-host-25) | A retransmitted ALLOW record lists every source with retransmission state that must be forwarded. |
| [RFC9776-HOST-26](#rfc9776-host-26) | A retransmitted BLOCK record lists every source with retransmission state that must be blocked. |
| [RFC9776-HOST-27](#rfc9776-host-27) | An empty ALLOW or BLOCK record is left out of a retransmitted State-Change Report. |
| [RFC9776-HQRY-1](#rfc9776-hqry-1) | A group member delays its response to a Query by a random time bounded by the Max Response Time derived from the Max Resp Code. |
| [RFC9776-HQRY-2](#rfc9776-hqry-2) | A group member keeps a per-interface timer for General-Query responses, a per-group-and-interface timer for Group and Group-and-Source Specific Query responses, and a per-group-and-interface list of sources for a pending Group-and-Source response. |
| [RFC9776-HQRY-3](#rfc9776-hqry-3) | On receiving a Query with the Router Alert option, and only when it has state to report, a member picks a random response delay from (0, Max Response Time). |
| [RFC9776-HQRY-4](#rfc9776-hqry-4) | When an earlier General-Query response is already due sooner, a member schedules no extra response for a new Query. |
| [RFC9776-HQRY-5](#rfc9776-hqry-5) | A General Query schedules its response on the Interface Timer after the selected delay, canceling any earlier pending General-Query response. |
| [RFC9776-HQRY-6](#rfc9776-hqry-6) | A Group Specific or Group-and-Source Specific Query with no pending response for the group schedules a report on the Group Timer. |
| [RFC9776-HQRY-7](#rfc9776-hqry-7) | A Group-and-Source Specific Query with no pending group response records its queried sources for use when the response is built. |
| [RFC9776-HQRY-8](#rfc9776-hqry-8) | A new Group Specific Query, or an empty recorded source list, clears the group's source list and reschedules a single Group-Timer response at the earlier of the two delays. |
| [RFC9776-HQRY-9](#rfc9776-hqry-9) | A Group-and-Source Specific Query with a pending non-empty-source-list response adds its sources to the recorded list and reschedules a single response at the earlier delay. |
| [RFC9776-HQRY-10](#rfc9776-hqry-10) | Expiry of the Interface Timer sends one Current-State Record, carrying filter-mode and source list, for every address with reception state on that interface, packed into as few Report messages as possible. |
| [RFC9776-HQRY-11](#rfc9776-hqry-11) | An implementation should spread General-Query Report transmissions across the interval (0, Max Response Time) rather than using a single timer. |
| [RFC9776-HQRY-12](#rfc9776-hqry-12) | An implementation must not send a Report immediately after receiving a General Query. |
| [RFC9776-HQRY-13](#rfc9776-hqry-13) | Expiry of a Group Timer with no recorded sources sends one Current-State Record for that address, only if the interface still has reception state for it. |
| [RFC9776-HQRY-14](#rfc9776-hqry-14) | Expiry of a Group Timer with recorded sources builds the Current-State Record from Table 5, only if the interface still has reception state for the address. |
| [RFC9776-HQRY-15](#rfc9776-hqry-15) | When the interface state is INCLUDE(A), the Current-State Record for pending sources B is IS_IN(A intersect B). |
| [RFC9776-HQRY-16](#rfc9776-hqry-16) | When the interface state is EXCLUDE(A), the Current-State Record for pending sources B is IS_IN(B minus A). |
| [RFC9776-HQRY-17](#rfc9776-hqry-17) | A Current-State Record with an empty source-address set is not sent. |
| [RFC9776-HQRY-18](#rfc9776-hqry-18) | After sending the required Report messages, a member clears the recorded source lists of the groups it reported. |
| [RFC9776-RQ-1](#rfc9776-rq-1) | A multicast router runs the router protocol on each directly attached network, using only one interface when several interfaces reach the same network. |
| [RFC9776-RQ-2](#rfc9776-rq-2) | A router must enable reception of 224.0.0.22 from all sources on every interface running this protocol. |
| [RFC9776-RQ-3](#rfc9776-rq-3) | A router must also perform the group-member part of IGMPv3 for 224.0.0.22 on that interface. |
| [RFC9776-RQ-4](#rfc9776-rq-4) | A router needs to know only that some system wants a source's packets; it need not track each system's interest individually. |
| [RFC9776-RQ-5](#rfc9776-rq-5) | An IGMPv3 router must also implement IGMPv1 and IGMPv2. |
| [RFC9776-RQ-6](#rfc9776-rq-6) | A router sends General Queries periodically to build and refresh the group membership state of an attached network. |
| [RFC9776-RQ-7](#rfc9776-rq-7) | Before a router deletes a group or source and prunes its traffic, it queries for other members or listeners. |
| [RFC9776-RQ-8](#rfc9776-rq-8) | A router sends a Group Specific Query to confirm no system still wants a group, once it receives a State-Change Record for a system leaving that group. |
| [RFC9776-RQ-9](#rfc9776-rq-9) | A router sends a Group-and-Source Specific Query, listing the sources that were asked to stop being forwarded, to check whether any system still wants them. |
| [RFC9776-RQ-10](#rfc9776-rq-10) | A router sends a Group-and-Source Specific Query only after a State-Change Record, never after a Current-State Record. |
| [RFC9776-RST-1](#rfc9776-rst-1) | A router keeps, per group per attached network, a record of the multicast address, Group Timer, Router Filter Mode, and source records. |
| [RFC9776-RST-2](#rfc9776-rst-2) | Each source record of a router's group state holds a source address and a Source Timer. |
| [RFC9776-RST-3](#rfc9776-rst-3) | Wanting all sources of a group is recorded as EXCLUDE mode with an empty source-record list. |
| [RFC9776-RST-4](#rfc9776-rst-4) | Once a router receives any Group Record with EXCLUDE filter-mode for a group, the Router Filter Mode for that group becomes EXCLUDE. |
| [RFC9776-RST-5](#rfc9776-rst-5) | In EXCLUDE mode, a router's source-record list holds two kinds of sources: ones that conflict between systems and must be forwarded, and ones every host asked to block. |
| [RFC9776-RST-6](#rfc9776-rst-6) | In INCLUDE mode, a router's source-record list is exactly the sources some system wants forwarded, and each must be forwarded by some router. |
| [RFC9776-RST-7](#rfc9776-rst-7) | The Group Timer is a per-group, per-network decrementing timer, bounded below at zero, used only in EXCLUDE mode. |
| [RFC9776-RST-8](#rfc9776-rst-8) | A group with INCLUDE filter-mode and a Group Timer of zero or more means all its members are in INCLUDE mode. |
| [RFC9776-RST-9](#rfc9776-rst-9) | A group with EXCLUDE filter-mode and a Group Timer greater than zero means at least one member is in EXCLUDE mode. |
| [RFC9776-RST-10](#rfc9776-rst-10) | A Group Timer at zero in EXCLUDE mode deletes the Group Record if all source timers have also expired, or else switches to INCLUDE mode using the sources with running timers. |
| [RFC9776-RST-11](#rfc9776-rst-11) | A Source Timer is a per-source-record decrementing timer with a lower bound of zero. |
| [RFC9776-RST-12](#rfc9776-rst-12) | A router updates a source's Source Timer whenever that source appears in a received record for the group. |
| [RFC9776-RST-13](#rfc9776-rst-13) | In INCLUDE mode, a source record with a running Source Timer means some system currently wants that source. |
| [RFC9776-RST-14](#rfc9776-rst-14) | In INCLUDE mode, an expired Source Timer makes the router delete that source record as no longer wanted. |
| [RFC9776-RST-15](#rfc9776-rst-15) | In EXCLUDE mode, a source record with a running Source Timer means at least one system wants that source forwarded. |
| [RFC9776-RST-16](#rfc9776-rst-16) | In EXCLUDE mode, an expired Source Timer makes the router tell the routing protocol that no receiver wants that source. |
| [RFC9776-RST-17](#rfc9776-rst-17) | In EXCLUDE mode, a router deletes source records only when the Group Timer expires. |
| [RFC9776-FWD-1](#rfc9776-fwd-1) | The multicast routing protocol should use IGMPv3 information so that every source or group wanted on a subnetwork is forwarded there. |
| [RFC9776-FWD-2](#rfc9776-fwd-2) | IGMPv3 information does not override multicast routing information; a router may still forward excluded-source traffic onto a transit subnet. |
| [RFC9776-FWD-3](#rfc9776-fwd-3) | In INCLUDE mode with a positive Source Timer, IGMP suggests forwarding traffic from that source. |
| [RFC9776-FWD-4](#rfc9776-fwd-4) | In INCLUDE mode with a Source Timer at zero, IGMP suggests stopping forwarding, removes the source record, and deletes the Group Record if none remain. |
| [RFC9776-FWD-5](#rfc9776-fwd-5) | In INCLUDE mode with no source elements, IGMP suggests not forwarding that source. |
| [RFC9776-FWD-6](#rfc9776-fwd-6) | In EXCLUDE mode with a positive Source Timer, IGMP suggests forwarding traffic from that source. |
| [RFC9776-FWD-7](#rfc9776-fwd-7) | In EXCLUDE mode with a Source Timer at zero, IGMP suggests not forwarding that source, but keeps the source record. |
| [RFC9776-FWD-8](#rfc9776-fwd-8) | In EXCLUDE mode with no source elements, IGMP suggests forwarding traffic from that source. |
| [RFC9776-RREP-1](#rfc9776-rrep-1) | An SSM-aware router should ignore a Group Record for an SSM address whose type is MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE. |
| [RFC9776-RREP-2](#rfc9776-rrep-2) | An SSM-aware router should ignore an IGMPv1 or IGMPv2 Report, or an IGMPv2 Done message, for an SSM address. |
| [RFC9776-RREP-3](#rfc9776-rrep-3) | An SSM-aware router should not use such an older-version message for an SSM address to build IP forwarding state. |
| [RFC9776-RREP-4](#rfc9776-rrep-4) | An SSM-aware router may log an error when it receives such a message. |
| [RFC9776-RREP-5](#rfc9776-rrep-5) | On receiving a Current-State Record, a router updates its group and source timers for that group. |
| [RFC9776-RREP-6](#rfc9776-rrep-6) | GMI, the Group Membership Interval, is the time in which a group membership will time out. |
| [RFC9776-RREP-7](#rfc9776-rrep-7) | LMQT, the Last Member Query Time, is the total time spent after [Last Member Query Count] retransmissions, and represents the router's leave latency. |
| [RFC9776-RREP-8](#rfc9776-rrep-8) | A router in INCLUDE(A) that gets an IS_IN(B) Current-State Record moves to INCLUDE(A+B) and sets B's Source Timer to GMI. |
| [RFC9776-RREP-9](#rfc9776-rrep-9) | A router in INCLUDE(A) that gets an IS_EX(B) Current-State Record moves to EXCLUDE(A*B, B-A), zeroes the (B-A) timers, deletes (A-B), and sets the Group Timer to GMI. |
| [RFC9776-RREP-10](#rfc9776-rrep-10) | A router in EXCLUDE(X,Y) that gets an IS_IN(A) Current-State Record moves to EXCLUDE(X+A, Y-A) and sets A's Source Timer to GMI. |
| [RFC9776-RREP-11](#rfc9776-rrep-11) | A router in EXCLUDE(X,Y) that gets an IS_EX(A) Current-State Record moves to EXCLUDE(A-Y, Y*A), sets (A-X-Y) timers to GMI, deletes (X-A) and (Y-A), and sets the Group Timer to GMI. |
| [RFC9776-RREP-12](#rfc9776-rrep-12) | A router queries the sources that a system asked to stop forwarding to a group. |
| [RFC9776-RREP-13](#rfc9776-rrep-13) | Sending or receiving a query for specific sources lowers their Source Timers to [Last Member Query Time]. |
| [RFC9776-RREP-14](#rfc9776-rrep-14) | A Group Record received during the query period that expresses interest in a queried source updates that source's timer. |
| [RFC9776-RREP-15](#rfc9776-rrep-15) | Querying a specific group lowers its Group Timer to [Last Member Query Time]. |
| [RFC9776-RREP-16](#rfc9776-rrep-16) | An EXCLUDE-mode Group Record received during the query interval updates the Group Timer, and the router keeps suggesting the group be forwarded without interruption. |
| [RFC9776-RREP-17](#rfc9776-rrep-17) | A router keeps suggesting to forward a queried group or source throughout [Last Member Query Time], and may prune it only after that time passes with no interested record. |
| [RFC9776-RREP-18](#rfc9776-rrep-18) | A query that a router state table triggers is sent [Last Member Query Count] times, once every [Last Member Query Interval]. |
| [RFC9776-RREP-19](#rfc9776-rrep-19) | A newly scheduled query for a group merges with any already-pending query retransmissions for that group. |
| [RFC9776-RREP-20](#rfc9776-rrep-20) | A router in INCLUDE(A) that gets an ALLOW(B) record moves to INCLUDE(A+B) and sets B's Source Timer to GMI. |
| [RFC9776-RREP-21](#rfc9776-rrep-21) | A router in INCLUDE(A) that gets a BLOCK(B) record stays at INCLUDE(A) and sends a Group-and-Source Query for A intersect B. |
| [RFC9776-RREP-22](#rfc9776-rrep-22) | A router in INCLUDE(A) that gets a TO_EX(B) record moves to EXCLUDE(A*B, B-A), zeroes (B-A), deletes (A-B), sends a Group-and-Source Query for A*B, and sets the Group Timer to GMI. |
| [RFC9776-RREP-23](#rfc9776-rrep-23) | A router in INCLUDE(A) that gets a TO_IN(B) record moves to INCLUDE(A+B), sets B's timer to GMI, and sends a Group-and-Source Query for A-B. |
| [RFC9776-RREP-24](#rfc9776-rrep-24) | A router in EXCLUDE(X,Y) that gets an ALLOW(A) record moves to EXCLUDE(X+A, Y-A) and sets A's Source Timer to GMI. |
| [RFC9776-RREP-25](#rfc9776-rrep-25) | A router in EXCLUDE(X,Y) that gets a BLOCK(A) record moves to EXCLUDE(X+(A-Y), Y), sets (A-X-Y) to the Group Timer value, and sends a Group-and-Source Query for A-Y. |
| [RFC9776-RREP-26](#rfc9776-rrep-26) | A router in EXCLUDE(X,Y) that gets a TO_EX(A) record moves to EXCLUDE(A-Y, Y*A), sets (A-X-Y) to the Group Timer value, deletes (X-A) and (Y-A), sends a Group-and-Source Query for A-Y, and sets the Group Timer to GMI. |
| [RFC9776-RREP-27](#rfc9776-rrep-27) | A router in EXCLUDE(X,Y) that gets a TO_IN(A) record moves to EXCLUDE(X+A, Y-A), sets A's timer to GMI, sends a Group-and-Source Query for X-A, and sends a Group Specific Query. |
| [RFC9776-RSW-1](#rfc9776-rsw-1) | When a group's Group Timer expires while the Router Filter Mode is EXCLUDE, the router assumes no more EXCLUDE-mode systems remain and switches the mode to INCLUDE. |
| [RFC9776-RSW-2](#rfc9776-rsw-2) | On switching to INCLUDE, a router keeps the source records whose Source Timer is still running and deletes those at zero. |
| [RFC9776-RQRY-1](#rfc9776-rqry-1) | Sending or receiving a query with the Suppress Router-Side Processing flag clear makes a router update its timers for the queried group or sources. |
| [RFC9776-RQRY-2](#rfc9776-rqry-2) | Receiving or sending Q(G,A) lowers the Source Timer of every source in A to LMQT. |
| [RFC9776-RQRY-3](#rfc9776-rqry-3) | Receiving or sending Q(G) lowers the group's Group Timer to LMQT. |
| [RFC9776-RQRY-4](#rfc9776-rqry-4) | Sending or receiving a query with the S flag set leaves the router's timers unchanged. |
| [RFC9776-RQRY-5](#rfc9776-rqry-5) | IGMPv3 elects one querier per subnet by comparing IP addresses, the same way as IGMPv2. |
| [RFC9776-RQRY-6](#rfc9776-rqry-6) | Receiving a General Query from a lower IP address sets the Other-Querier-Present Timer to [Other Querier Present Interval] and stops the router sending General Queries if it had been the querier. |
| [RFC9776-RQRY-7](#rfc9776-rqry-7) | A non-querier router should start sending General Queries once its Other-Querier-Present Timer expires. |
| [RFC9776-RQRY-8](#rfc9776-rqry-8) | A router that receives an older-version General Query must switch to the oldest IGMP version present on the network. |
| [RFC9776-RQRY-9](#rfc9776-rqry-9) | A "Send Q(G)" table action lowers the Group Timer to LMQT. |
| [RFC9776-RQRY-10](#rfc9776-rqry-10) | A "Send Q(G)" action sends a Group Specific Query at once and schedules [Last Member Query Count]-1 retransmissions every [Last Member Query Interval]. |
| [RFC9776-RQRY-11](#rfc9776-rqry-11) | A Group Specific Query sets the Suppress Router-Side Processing bit when the Group Timer exceeds LMQT. |
| [RFC9776-RQRY-12](#rfc9776-rqry-12) | A "Send Q(G,X)" action sets the retransmission count to [Last Member Query Count] and lowers the Source Timer to LMQT, for each source in X with a timer above LMQT. |
| [RFC9776-RQRY-13](#rfc9776-rqry-13) | A "Send Q(G,X)" action sends a Group-and-Source Specific Query at once and schedules [Last Member Query Count]-1 retransmissions every [Last Member Query Interval]. |
| [RFC9776-RQRY-14](#rfc9776-rqry-14) | Building a Group-and-Source Specific Query sends two messages: one with the Suppress bit set for sources with timers above LMQT, one with the bit clear for sources with timers at or below LMQT. |
| [RFC9776-RQRY-15](#rfc9776-rqry-15) | A Group-and-Source Specific Query message with no matching sources is not sent. |
| [RFC9776-VER-1](#rfc9776-ver-1) | A Membership Query of length 8 octets with Max Resp Code zero is an IGMPv1 Query. |
| [RFC9776-VER-2](#rfc9776-ver-2) | A Membership Query of length 8 octets with a non-zero Max Resp Code is an IGMPv2 Query. |
| [RFC9776-VER-3](#rfc9776-ver-3) | A Membership Query of length 12 octets or more is an IGMPv3 Query. |
| [RFC9776-VER-4](#rfc9776-ver-4) | A Query Message that matches none of the three length and code conditions must be silently ignored. |
| [RFC9776-COMPH-1](#rfc9776-comph-1) | An IGMPv3 host must be able to operate in IGMPv1 and IGMPv2 compatibility modes. |
| [RFC9776-COMPH-2](#rfc9776-comph-2) | An IGMPv3 host must keep per-interface state for the compatibility mode of each attached network. |
| [RFC9776-COMPH-3](#rfc9776-comph-3) | The Host Compatibility Mode variable, kept per interface, is IGMPv1, IGMPv2, or IGMPv3, driven by received General Queries and the Older-Version-Querier-Present Timer. |
| [RFC9776-COMPH-4](#rfc9776-comph-4) | A host keeps an IGMPv1-Querier-Present Timer and an IGMPv2-Querier-Present Timer, one per interface. |
| [RFC9776-COMPH-5](#rfc9776-comph-5) | Receiving an IGMPv1 Membership Query sets the host's IGMPv1-Querier-Present Timer to [Older Version Querier Present Interval]. |
| [RFC9776-COMPH-6](#rfc9776-comph-6) | Receiving an IGMPv2 General Query sets the host's IGMPv2-Querier-Present Timer to [Older Version Querier Present Interval]. |
| [RFC9776-COMPH-7](#rfc9776-comph-7) | A host's compatibility mode changes on receiving an older-version query than its current mode, or on certain timer events. |
| [RFC9776-COMPH-8](#rfc9776-comph-8) | When the IGMPv1-Querier-Present Timer expires, a host switches to IGMPv2 mode if the IGMPv2 timer is still running, or to IGMPv3 mode if it is not. |
| [RFC9776-COMPH-9](#rfc9776-comph-9) | When the IGMPv2-Querier-Present Timer expires, a host switches to IGMPv3 mode. |
| [RFC9776-COMPH-10](#rfc9776-comph-10) | The Host Compatibility Mode reflects whether an older-version General Query arrived in the last [Older Version Querier Present Interval] seconds. |
| [RFC9776-COMPH-11](#rfc9776-comph-11) | An older-version Group Specific Query must not change the Host Compatibility Mode variable. |
| [RFC9776-COMPH-12](#rfc9776-comph-12) | Host Compatibility Mode is IGMPv3, the default, when neither the IGMPv1 nor the IGMPv2 Querier Present timer is running. |
| [RFC9776-COMPH-13](#rfc9776-comph-13) | Host Compatibility Mode is IGMPv2 when the IGMPv2 Querier Present timer runs and the IGMPv1 one does not. |
| [RFC9776-COMPH-14](#rfc9776-comph-14) | Host Compatibility Mode is IGMPv1 whenever the IGMPv1 Querier Present timer is running. |
| [RFC9776-COMPH-15](#rfc9776-comph-15) | A host should switch compatibility mode right away once a received query updates its Querier Present timers. |
| [RFC9776-COMPH-16](#rfc9776-comph-16) | In IGMPv3 Host Compatibility Mode, a host uses the IGMPv3 protocol on that interface. |
| [RFC9776-COMPH-17](#rfc9776-comph-17) | In IGMPv2 Host Compatibility Mode, a host uses only the IGMPv2 protocol on that interface. |
| [RFC9776-COMPH-18](#rfc9776-comph-18) | In IGMPv1 Host Compatibility Mode, a host uses only the IGMPv1 protocol on that interface. |
| [RFC9776-COMPH-19](#rfc9776-comph-19) | An IGMPv1 router sends General Queries with the Max Resp Code field set to 0. |
| [RFC9776-COMPH-20](#rfc9776-comph-20) | A host must interpret a Max Resp Code of 0 as a Max Response Time of 100 (10 seconds). |
| [RFC9776-COMPH-21](#rfc9776-comph-21) | An IGMPv2 router sets Max Resp Code directly to the desired Max Response Time, without the IGMPv3 exponential encoding. |
| [RFC9776-COMPH-22](#rfc9776-comph-22) | A host cancels all pending response and retransmission timers when it changes compatibility mode. |
| [RFC9776-COMPH-23](#rfc9776-comph-23) | An SSM-aware host should log an error on receiving an IGMPv1 Query, an IGMPv2 General Query, or an IGMPv2 Group Specific Query for an SSM address. |
| [RFC9776-COMPH-24](#rfc9776-comph-24) | An implementation should offer a configuration option to disable Host Compatibility Mode for SSM-only operation. |
| [RFC9776-COMPH-25](#rfc9776-comph-25) | This SSM-only configuration option should be disabled by default. |
| [RFC9776-COMPH-26](#rfc9776-comph-26) | A host may let an IGMPv1 or IGMPv2 Membership Report suppress its own IGMPv3 Membership Record. |
| [RFC9776-COMPH-27](#rfc9776-comph-27) | An SSM-aware host must not let its IGMPv3 Membership Record be suppressed. |
| [RFC9776-COMPR-1](#rfc9776-compr-1) | The querier must use the lowest IGMP version present among the network's routers. |
| [RFC9776-COMPR-2](#rfc9776-compr-2) | A router that wants IGMPv1 or IGMPv2 compatibility must have a configuration option for IGMPv1 or IGMPv2 mode. |
| [RFC9776-COMPR-3](#rfc9776-compr-3) | In IGMPv1 mode, a router must send Periodic Queries with Max Resp Code 0, truncated to 8 bytes at the Group Address field. |
| [RFC9776-COMPR-4](#rfc9776-compr-4) | In IGMPv1 mode, a router must ignore Leave Group messages. |
| [RFC9776-COMPR-5](#rfc9776-compr-5) | In IGMPv1 mode, a router should warn on receiving an IGMPv2 or IGMPv3 query, with the warnings rate-limited. |
| [RFC9776-COMPR-6](#rfc9776-compr-6) | In IGMPv2 mode, a router must send Periodic Queries truncated to 8 bytes at the Group Address field. |
| [RFC9776-COMPR-7](#rfc9776-compr-7) | In IGMPv2 mode, a router should warn on receiving an IGMPv3 query, with the warnings rate-limited. |
| [RFC9776-COMPR-8](#rfc9776-compr-8) | In IGMPv2 mode, a router must fill Max Resp Code with the literal Max Response Time, not the IGMPv3 exponential encoding. |
| [RFC9776-COMPR-9](#rfc9776-compr-9) | A router not configured for IGMPv1 or IGMPv2 should log a warning on receiving an IGMPv1 Query or an IGMPv2 General Query. |
| [RFC9776-COMPR-10](#rfc9776-compr-10) | These warnings about an unexpected older-version query must be rate-limited. |
| [RFC9776-COMPR-11](#rfc9776-compr-11) | An implementation should offer a router configuration option to disable compatibility mode for SSM-only operation. |
| [RFC9776-COMPR-12](#rfc9776-compr-12) | This SSM-only configuration option should be disabled by default. |
| [RFC9776-COMPR-13](#rfc9776-compr-13) | An IGMPv3 router must be able to operate in IGMPv1 and IGMPv2 compatibility modes for older hosts. |
| [RFC9776-COMPR-14](#rfc9776-compr-14) | A router keeps a per-Group-Record Group Compatibility Mode, one of IGMPv1, IGMPv2, or IGMPv3, driven by received Membership Report versions and the Older-Version-Host-Present Timer. |
| [RFC9776-COMPR-15](#rfc9776-compr-15) | A router keeps an IGMPv1-Host-Present Timer and an IGMPv2-Host-Present Timer per Group Record. |
| [RFC9776-COMPR-16](#rfc9776-compr-16) | Receiving an IGMPv1 Membership Report sets the group's IGMPv1-Host-Present Timer to [Older Version Host Present Interval]. |
| [RFC9776-COMPR-17](#rfc9776-compr-17) | Receiving an IGMPv2 Membership Report sets the group's IGMPv2-Host-Present Timer to [Older Version Host Present Interval]. |
| [RFC9776-COMPR-18](#rfc9776-compr-18) | A group's Compatibility Mode changes on an older-version report than its current mode, or on certain timer events. |
| [RFC9776-COMPR-19](#rfc9776-compr-19) | When the IGMPv1-Host-Present Timer expires, a router switches the group to IGMPv2 mode if the IGMPv2 timer is running, else to IGMPv3 mode. |
| [RFC9776-COMPR-20](#rfc9776-compr-20) | When the IGMPv2-Host-Present Timer expires while the IGMPv1 timer is not running, a router switches the group to IGMPv3 mode. |
| [RFC9776-COMPR-21](#rfc9776-compr-21) | After a group switches back to IGMPv3 mode, blocking of sources marked to be blocked is delayed until [Group Membership Interval] after regaining source-specific state. |
| [RFC9776-COMPR-22](#rfc9776-compr-22) | Group Compatibility Mode is IGMPv3, the default, when neither the IGMPv1 nor the IGMPv2 Host Present timer is running. |
| [RFC9776-COMPR-23](#rfc9776-compr-23) | Group Compatibility Mode is IGMPv2 when the IGMPv2 Host Present timer runs and the IGMPv1 one does not. |
| [RFC9776-COMPR-24](#rfc9776-compr-24) | Group Compatibility Mode is IGMPv1 whenever the IGMPv1 Host Present timer is running. |
| [RFC9776-COMPR-25](#rfc9776-compr-25) | A router should switch a group's Compatibility Mode right away once a received report updates its Host Present timers. |
| [RFC9776-COMPR-26](#rfc9776-compr-26) | In IGMPv3 Group Compatibility Mode, a router uses the IGMPv3 protocol for that group. |
| [RFC9776-COMPR-27](#rfc9776-compr-27) | In IGMPv2 Group Compatibility Mode, a router treats an IGMPv2 Report as IS_EX({}). |
| [RFC9776-COMPR-28](#rfc9776-compr-28) | In IGMPv2 Group Compatibility Mode, a router treats an IGMPv2 Leave as TO_IN({}). |
| [RFC9776-COMPR-29](#rfc9776-compr-29) | In IGMPv2 Group Compatibility Mode, a router ignores IGMPv3 BLOCK messages. |
| [RFC9776-COMPR-30](#rfc9776-compr-30) | In IGMPv2 Group Compatibility Mode, a router ignores the source list of a TO_EX() message, treating it as TO_EX({}). |
| [RFC9776-COMPR-31](#rfc9776-compr-31) | In IGMPv1 Group Compatibility Mode, a router treats a v1 Report as IS_EX({}). |
| [RFC9776-COMPR-32](#rfc9776-compr-32) | In IGMPv1 Group Compatibility Mode, a router treats a v2 Report as IS_EX({}). |
| [RFC9776-COMPR-33](#rfc9776-compr-33) | In IGMPv1 Group Compatibility Mode, a router also ignores IGMPv2 Leave messages and IGMPv3 TO_IN() messages, besides what it already ignores in IGMPv2 mode. |
| [RFC9776-TIMER-1](#rfc9776-timer-1) | If a link uses non-default timer or counter settings, every system on that link MUST use the same settings. |
| [RFC9776-TIMER-2](#rfc9776-timer-2) | The Robustness Variable tunes IGMP for expected packet loss; IGMP tolerates one less than the Robustness Variable packet losses. Default: 2. |
| [RFC9776-TIMER-3](#rfc9776-timer-3) | The Robustness Variable MUST NOT be zero and SHOULD NOT be one. |
| [RFC9776-TIMER-4](#rfc9776-timer-4) | The Query Interval is the interval between General Queries sent by the Querier. Default: 125 seconds. |
| [RFC9776-TIMER-5](#rfc9776-timer-5) | The Query Response Interval uses the Max Response Time to calculate the Max Resp Code inserted into periodic General Queries. Default: 100, that is 10 seconds. |
| [RFC9776-TIMER-6](#rfc9776-timer-6) | The number of seconds of the Query Response Interval must be less than the Query Interval. |
| [RFC9776-TIMER-7](#rfc9776-timer-7) | The Group Membership Interval is the time a multicast router waits before it decides there are no more members of a group or a source on a network. |
| [RFC9776-TIMER-8](#rfc9776-timer-8) | The Group Membership Interval MUST be the Robustness Variable times the Query Interval, plus twice the Query Response Interval. |
| [RFC9776-TIMER-9](#rfc9776-timer-9) | The Other Querier Present Interval is the time a multicast router waits before it decides there is no longer another router that should be the querier. |
| [RFC9776-TIMER-10](#rfc9776-timer-10) | The Other Querier Present Interval MUST be the Robustness Variable times the Query Interval, plus 0.5 times the Query Response Interval. |
| [RFC9776-TIMER-11](#rfc9776-timer-11) | The Startup Query Interval is the interval between General Queries a Querier sends on startup. Default: 1/4 times the Query Interval. |
| [RFC9776-TIMER-12](#rfc9776-timer-12) | The Startup Query Count is the number of Queries a Querier sends out on startup, separated by the Startup Query Interval. Default: the Robustness Variable. |
| [RFC9776-TIMER-13](#rfc9776-timer-13) | The Last Member Query Interval is the Max Response Time used for the Max Resp Code of Group Specific Queries sent after a Leave Group message, and of Group-and-Source Specific Queries. Default: 10, that is 1 second. |
| [RFC9776-TIMER-14](#rfc9776-timer-14) | For a Last Member Query Interval greater than 12.8 seconds, only specific values are representable, matching the sequential Max Resp Code values. |
| [RFC9776-TIMER-15](#rfc9776-timer-15) | When a system converts a configured time to a Max Resp Code, it uses the exact value if possible, or the next lower representable value otherwise. |
| [RFC9776-TIMER-16](#rfc9776-timer-16) | The Last Member Query Count is the number of Group Specific Queries, and also the number of Group-and-Source Specific Queries, a router sends before it assumes there are no more listeners. Default: the Robustness Variable. |
| [RFC9776-TIMER-17](#rfc9776-timer-17) | The Last Member Query Time is the Last Member Query Interval times the Last Member Query Count; it is not itself a tunable value, only its components are. |
| [RFC9776-TIMER-18](#rfc9776-timer-18) | The Unsolicited Report Interval is the time between repetitions of a host's initial report of membership in a group. Default: 1 second. |
| [RFC9776-TIMER-19](#rfc9776-timer-19) | The Older Version Querier Present Interval is the timeout for a host to transition back to IGMPv3 mode after it receives an older version query. |
| [RFC9776-TIMER-20](#rfc9776-timer-20) | When a host receives an older version query, it sets its Older-Version-Querier-Present Timer to the Older Version Querier Present Interval. |
| [RFC9776-TIMER-21](#rfc9776-timer-21) | A host SHOULD use the default values to calculate the Older Version Querier Present Interval, because it does not know the querying router's configured values. |
| [RFC9776-TIMER-22](#rfc9776-timer-22) | The Older Version Querier Present Interval SHOULD be the Robustness Variable times the Query Interval, plus 10 times the Max Response Time of the last received Query. |
| [RFC9776-TIMER-23](#rfc9776-timer-23) | The Older Host Present Interval is the timeout for a group to transition back to IGMPv3 mode after an older version report is sent for it. |
| [RFC9776-TIMER-24](#rfc9776-timer-24) | When a router receives an older version report, it sets its Older-Host-Present Timer to the Older Host Present Interval. |
| [RFC9776-TIMER-25](#rfc9776-timer-25) | The Older Host Present Interval MUST be the Robustness Variable times the Query Interval, plus the Query Response Interval. |
| [RFC9776-TIMER-26](#rfc9776-timer-26) | The Query Interval MUST be equal to or longer than the Max Response Time inserted in General Query messages. |

## Message formats: general rules

### RFC9776-GEN-1

**An IGMP message is encapsulated in an IPv4 datagram with IP protocol number 2.**

> "IGMP messages are encapsulated in IPv4 datagrams, with an IP protocol number of 2." — §4,
> `rfc9776.txt:446-447`

- Strength: description. Class: wire.
- Check idea: capture an IGMP message on the link and check that its enclosing IPv4 header
  carries protocol number 2.

### RFC9776-GEN-2

**A system sends every IGMP message with an IP Time-to-Live of 1.**

> "Every IGMP message described in this document is sent with an IP Time-to-Live of 1" — §4,
> `rfc9776.txt:447-448`

- Strength: description. Class: wire.
- Check idea: capture an IGMP message on the link and check that the IP header Time-to-Live
  field is 1.

### RFC9776-GEN-3

**A system sends every IGMP message with IP Precedence Internetwork Control.**

> "IP Precedence of Internetwork Control (e.g., Type of Service 0xc0)" — §4,
> `rfc9776.txt:448-449`

- Strength: description. Class: wire.
- Check idea: capture an IGMP message on the link and check that the IP header Type-of-Service
  octet is 0xc0.

### RFC9776-GEN-4

**A system sends every IGMP message with an IP Router Alert option.**

> "carries an IP Router Alert option [RFC2113] in its IP header" — §4, `rfc9776.txt:449-450`

- Strength: description. Class: wire.
- Check idea: capture an IGMP message on the link and check that its IP header options include
  the Router Alert option.

### RFC9776-GEN-5

**A Membership Query message carries type value 0x11.**

> "| Type Number (hex) | Message Name             |" and "| 0x11              | Membership
> Query         |" — §4, `rfc9776.txt:457`, `rfc9776.txt:459`

- Strength: description. Class: wire.
- Check idea: capture a Membership Query sent on the link and check that its Type field is
  0x11.

### RFC9776-GEN-6

**An IGMPv3 Membership Report message carries type value 0x22.**

> "| Type Number (hex) | Message Name             |" and "| 0x22              | IGMPv3
> Membership Report |" — §4, `rfc9776.txt:457`, `rfc9776.txt:461`

- Strength: description. Class: wire.
- Check idea: capture an IGMPv3 Membership Report sent on the link and check that its Type
  field is 0x22.

### RFC9776-GEN-7

**A system MUST support the IGMPv1 Membership Report message, type 0x12, for interoperation
with an older IGMP version.**

> "An implementation of IGMPv3 MUST also support the following three message types, for
> interoperation with previous versions of IGMP (see Section 7):" and "| Type Number (hex) |
> Message Name             | Reference |" and "| 0x12              | IGMPv1 Membership Report |
> [RFC1112] |" — §4, `rfc9776.txt:466-468`, `rfc9776.txt:471`, `rfc9776.txt:473`

- Strength: must. Class: end-to-end.
- Check idea: put a system in an IGMPv1 compatibility scenario and check that it recognizes and
  processes a message of type 0x12.

### RFC9776-GEN-8

**A system MUST support the IGMPv2 Membership Report message, type 0x16, for interoperation
with an older IGMP version.**

> "An implementation of IGMPv3 MUST also support the following three message types, for
> interoperation with previous versions of IGMP (see Section 7):" and "| 0x16              |
> IGMPv2 Membership Report | [RFC2236] |" — §4, `rfc9776.txt:466-468`, `rfc9776.txt:475`

- Strength: must. Class: end-to-end.
- Check idea: put a system in an IGMPv2 compatibility scenario and check that it recognizes and
  processes a message of type 0x16.

### RFC9776-GEN-9

**A system MUST support the IGMPv2 Leave Group message, type 0x17, for interoperation with an
older IGMP version.**

> "An implementation of IGMPv3 MUST also support the following three message types, for
> interoperation with previous versions of IGMP (see Section 7):" and "| 0x17              |
> IGMPv2 Leave Group       | [RFC2236] |" — §4, `rfc9776.txt:466-468`, `rfc9776.txt:477`

- Strength: must. Class: end-to-end.
- Check idea: put a router in an IGMPv2 compatibility scenario and check that it recognizes and
  processes a message of type 0x17.

### RFC9776-GEN-10

**A system MUST silently ignore an unrecognized IGMP message type.**

> "Unrecognized message types MUST be silently ignored." — §4, `rfc9776.txt:482`

- Strength: must. Class: end-to-end.
- Check idea: send a system an IGMP message of an undefined type and check that the system
  gives no error report and no visible reaction.
## Membership Query message

### RFC9776-QRY-1

**A multicast router sends a Membership Query to learn the multicast reception state of a
neighboring interface.**

> "Membership Queries are sent by IP multicast routers to query the multicast reception state
> of neighboring interfaces." — §4.1, `rfc9776.txt:492-493`

- Strength: description. Class: wire.
- Check idea: observe the link during normal operation and check that Membership Query messages
  originate only from a multicast router.

### RFC9776-QRY-2

**The Max Resp Code field states the maximum time allowed before a system sends its
responding report.**

> "The Max Resp Code field specifies the maximum time allowed before sending a responding
> report." — §4.1.1, `rfc9776.txt:519-520`

- Strength: description. Class: wire.
- Check idea: send a Query with a given Max Resp Code and check that every responding report
  arrives within the time the code represents.

### RFC9776-QRY-3

**A Max Resp Code below 128 equals the Max Response Time directly.**

> "If Max Resp Code < 128, Max Response Time = Max Resp Code" — §4.1.1, `rfc9776.txt:524`

- Strength: description. Class: encoding.
- Check idea: build a Query with a Max Resp Code under 128 and check that the resulting Max
  Response Time equals the same number.

### RFC9776-QRY-4

**A Max Resp Code of 128 or more codes the Max Response Time as a floating-point value: the
mantissa combined with 0x10, shifted left by the exponent plus 3.**

> "If Max Resp Code >= 128, Max Resp Code represents a floating-point value as follows:" and
> "|1| exp | mant  |" and "Max Response Time = (mant | 0x10) << (exp + 3)" — §4.1.1,
> `rfc9776.txt:526-527`, `rfc9776.txt:531`, `rfc9776.txt:534`

- Strength: description. Class: encoding.
- Check idea: build a Query with a Max Resp Code of 128 or more and check that the decoded Max
  Response Time matches the exponent/mantissa formula.

### RFC9776-QRY-5

**The Checksum field is the 16-bit one's complement of the one's complement sum of the whole
IGMP message, computed with the Checksum field itself set to zero.**

> "The Checksum field is the 16-bit one's complement of the one's complement sum of the whole
> IGMP message (the entire IP payload)." and "For computing the checksum, the Checksum field is
> set to zero." — §4.1.2, `rfc9776.txt:546-547`, `rfc9776.txt:548`

- Strength: description. Class: wire.
- Check idea: capture a Query, recompute the checksum over the message with the Checksum field
  zeroed, and check the result equals the field on the wire.

### RFC9776-QRY-6

**A system MUST verify the checksum of a received Query before it processes the packet.**

> "When receiving packets, the checksum MUST be verified before processing a packet [RFC1071]."
> — §4.1.2, `rfc9776.txt:548-550`

- Strength: must. Class: end-to-end.
- Check idea: send a Query with a wrong checksum and check that the system does not act on its
  content.

### RFC9776-QRY-7

**The Group Address field is zero when a router sends a General Query.**

> "The Group Address field is set to zero when sending a General Query" — §4.1.3,
> `rfc9776.txt:554-555`

- Strength: description. Class: wire.
- Check idea: trigger a General Query and check that its Group Address field is zero.

### RFC9776-QRY-8

**The Group Address field holds the queried multicast address when a router sends a
Group-Specific or Group-and-Source Specific Query.**

> "set to the IP multicast address being queried when sending a Group-Specific Query or
> Group-and-Source Specific Query" — §4.1.3, `rfc9776.txt:555-557`

- Strength: description. Class: wire.
- Check idea: trigger a Group-Specific Query for a given address and check that the Group
  Address field carries that address.

### RFC9776-QRY-9

**The Flags field is a bitstring managed by the IGMP Type Numbers registry.**

> "The Flags field is a bitstring managed by the" and "registry defined in [BCP57]." — §4.1.4,
> `rfc9776.txt:561-562`

- Strength: description. Class: encoding.
- Check idea: capture a Query and check that the Flags bits match the current IGMP Type Numbers
  registry allocation.

### RFC9776-QRY-10

**When the S flag is set to one, a receiving router suppresses its normal timer updates for
that Query.**

> "When set to one, the S flag indicates to any receiving multicast routers that they are to
> suppress the normal timer updates they perform upon receiving a Query." — §4.1.5,
> `rfc9776.txt:566-568`

- Strength: description. Class: end-to-end.
- Check idea: send a Query with the S flag set to one to a router and check that the router
  does not update its normal timers in response.

### RFC9776-QRY-11

**The S flag does not suppress Querier election or a router's own host-side processing of the
Query.**

> "It does not, however, suppress the Querier election or the" and "processing of a Query that
> a router may be required" and "to perform as a consequence of itself being a group member." —
> §4.1.5, `rfc9776.txt:568`, `rfc9776.txt:569-570`, `rfc9776.txt:570-571`

- Strength: may (lower case). Class: end-to-end.
- Check idea: send a Query with the S flag set to one to a router that is itself a group
  member and check that the router still runs Querier election and its own group-member
  response.

### RFC9776-QRY-12

**The QRV field carries the querier's Robustness Variable, when the field is non-zero.**

> "If non-zero, the QRV field contains the [Robustness Variable] value" and "used by the
> querier, i.e., the sender of the Query." — §4.1.6, `rfc9776.txt:575`, `rfc9776.txt:575-576`

- Strength: description. Class: wire.
- Check idea: configure a querier's Robustness Variable and check that the QRV field of its
  Query carries the same value.

### RFC9776-QRY-13

**A querier whose Robustness Variable exceeds 7 sets the QRV field to zero.**

> "If the querier's" and "[Robustness Variable] exceeds 7, the maximum value of the QRV field,"
> and "the QRV is set to zero." — §4.1.6, `rfc9776.txt:576`, `rfc9776.txt:577`,
> `rfc9776.txt:578`

- Strength: description. Class: wire.
- Check idea: configure a querier's Robustness Variable above 7 and check that its Query
  carries QRV zero.

### RFC9776-QRY-14

**A router adopts the QRV value of the most recently received Query as its own Robustness
Variable, or the default value when that QRV was zero.**

> "Routers adopt the QRV value from the most" and "recently received Query as their own
> [Robustness Variable] value," and "unless that most recently received QRV was zero, in which
> case the" and "receivers use the default [Robustness Variable] value specified in" and
> "Section 8.1 or a statically configured value." — §4.1.6, `rfc9776.txt:578`,
> `rfc9776.txt:579`, `rfc9776.txt:580`, `rfc9776.txt:581`, `rfc9776.txt:582`

- Strength: description. Class: internal.
- Check idea: send a router a Query with a non-zero QRV and check that the router's Robustness
  Variable becomes that value; repeat with QRV zero and check the router uses its default or
  configured value instead.

### RFC9776-QRY-15

**The QQIC field states the Query Interval used by the querier.**

> "The QQIC field specifies the [Query Interval] used by the querier." — §4.1.7,
> `rfc9776.txt:586`

- Strength: description. Class: wire.
- Check idea: configure a querier's Query Interval and check that the QQIC field of its Query
  encodes that interval.

### RFC9776-QRY-16

**A QQIC below 128 equals the QQI directly.**

> "If QQIC < 128, QQI = QQIC" — §4.1.7, `rfc9776.txt:591`

- Strength: description. Class: encoding.
- Check idea: build a Query with a QQIC under 128 and check that the decoded Querier's Query
  Interval equals the same number.

### RFC9776-QRY-17

**A QQIC of 128 or more codes the Querier's Query Interval as a floating-point value: the
mantissa combined with 0x10, shifted left by the exponent plus 3.**

> "If QQIC >= 128, QQIC represents a floating-point value as follows:" and "|1| exp | mant  |"
> and "QQI = (mant | 0x10) << (exp + 3)" — §4.1.7, `rfc9776.txt:593`, `rfc9776.txt:597`,
> `rfc9776.txt:600`

- Strength: description. Class: encoding.
- Check idea: build a Query with a QQIC of 128 or more and check that the decoded QQI matches
  the exponent/mantissa formula.

### RFC9776-QRY-18

**A multicast router that is not the current querier adopts the QQI of the most recently
received Query as its own Query Interval, or the default value when that QQI was zero.**

> "Multicast routers that are not the current querier adopt the QQI" and "value from the most
> recently received Query as their own [Query" and "Interval] value, unless that most recently
> received QQI was zero, in" and "which case the receiving routers use the default [Query
> Interval]" and "value specified in Section 8.2." — §4.1.7, `rfc9776.txt:604`,
> `rfc9776.txt:605`, `rfc9776.txt:606`, `rfc9776.txt:607`, `rfc9776.txt:608`

- Strength: description. Class: internal.
- Check idea: send a non-querier router a Query with a non-zero QQIC and check that the
  router's Query Interval becomes the decoded value; repeat with QQIC zero and check the router
  uses its default instead.

### RFC9776-QRY-19

**The Number of Sources field states how many source addresses are present in the Query.**

> "The Number of Sources (N) field specifies how many source addresses" and "are present in the
> Query." — §4.1.8, `rfc9776.txt:612`, `rfc9776.txt:613`

- Strength: description. Class: wire.
- Check idea: build a Query with a chosen count of source addresses and check that the Number
  of Sources field equals that count.

### RFC9776-QRY-20

**The Number of Sources field is zero in a General Query or a Group Specific Query, and
non-zero in a Group-and-Source Specific Query.**

> "This number is zero in a General Query or" and "a Group Specific Query and non-zero in a
> Group-and-Source Specific" and "Query." — §4.1.8, `rfc9776.txt:613`, `rfc9776.txt:614`,
> `rfc9776.txt:615`

- Strength: description. Class: wire.
- Check idea: trigger a General Query and a Group Specific Query and check the Number of
  Sources field is zero in each; trigger a Group-and-Source Specific Query and check the field
  is non-zero.

### RFC9776-QRY-21

**The Number of Sources is limited by the MTU of the network over which the Query is
transmitted.**

> "This number is limited by the MTU of the network over which" and "the Query is transmitted."
> — §4.1.8, `rfc9776.txt:615`, `rfc9776.txt:616`

- Strength: description. Class: wire.
- Check idea: request a Query with a source-address count that would exceed the link MTU and
  check that no such oversized Query appears on the link.

### RFC9776-QRY-22

**The Source Address fields of a Query are a vector of source addresses, one for each count in
the Number of Sources field.**

> "The Source Address [i] fields are a vector of n IP unicast addresses," and "where n is the
> value in the Number of Sources (N) field." — §4.1.9, `rfc9776.txt:625`, `rfc9776.txt:626`

- Strength: description. Class: wire.
- Check idea: build a Group-and-Source Specific Query with n sources and check that the message
  carries exactly n Source Address fields, matching the Number of Sources field.

### RFC9776-QRY-23

**On a received Query with extra octets beyond the defined fields, a system MUST include those
octets when it verifies the IGMP Checksum, but MUST otherwise ignore them.**

> "If the Packet Length field in the IP header of a received Query" and "indicates that there
> are additional octets of data present, beyond" and "the fields described here, IGMPv3
> implementations MUST include those" and "octets in the computation to verify the received
> IGMP Checksum but" and "MUST otherwise ignore those additional octets." — §4.1.10,
> `rfc9776.txt:630`, `rfc9776.txt:631`, `rfc9776.txt:632`, `rfc9776.txt:633`,
> `rfc9776.txt:634`

- Strength: must. Class: end-to-end.
- Check idea: send a Query padded with trailing octets and a checksum computed over them, and
  check the system accepts it while treating the trailing octets as having no other effect.

### RFC9776-QRY-24

**When a system sends a Query, it MUST NOT include additional octets beyond the fields
described in the format.**

> "When sending a Query," and "an IGMPv3 implementation MUST NOT include additional octets
> beyond" and "the fields described here." — §4.1.10, `rfc9776.txt:634`, `rfc9776.txt:635`,
> `rfc9776.txt:636`

- Strength: must. Class: wire.
- Check idea: capture a Query a system sends and check that its length matches exactly the
  defined fields, with no trailing octets.

### RFC9776-QRY-25

**A General Query is sent to learn the complete multicast reception state of the neighboring
interfaces, with the Group Address and Number of Sources fields both zero.**

> "A General Query is sent by a multicast router to learn the" and "complete multicast
> reception state of the neighboring interfaces" and "In a General Query, both the Group
> Address field and the" and "Number of Sources (N) field are zero." — §4.1.11,
> `rfc9776.txt:642-643`, `rfc9776.txt:644`, `rfc9776.txt:645`, `rfc9776.txt:646`

- Strength: description. Class: wire.
- Check idea: trigger a General Query and check that both the Group Address and Number of
  Sources fields are zero.

### RFC9776-QRY-26

**A Group Specific Query is sent to learn the reception state of one multicast address, with
the Group Address field holding that address and the Number of Sources field zero.**

> "A Group Specific Query is sent by a multicast router to learn the" and "reception state,
> with respect to a single multicast address, of" and "the neighboring interfaces." and "In a
> Group Specific Query, the Group" and "Address field contains the multicast address of
> interest, and the" and "Number of Sources (N) field contains zero." — §4.1.11,
> `rfc9776.txt:648-649`, `rfc9776.txt:649-650`, `rfc9776.txt:650`, `rfc9776.txt:650-651`,
> `rfc9776.txt:651`, `rfc9776.txt:652`

- Strength: description. Class: wire.
- Check idea: trigger a Group Specific Query for a chosen address and check the Group Address
  field carries it while the Number of Sources field is zero.

### RFC9776-QRY-27

**A Group-and-Source Specific Query is sent to learn if any neighboring interface wants
packets sent to a specified address from a specified list of sources, and it carries the
address and the sources.**

> "A Group-and-Source Specific Query is sent by a multicast router" and "to learn if any
> neighboring interface desires reception of" and "packets sent to a specified multicast
> address, from any of a" and "specified list of sources." and "the" and "Group Address field
> contains the multicast address of" and "interest, and the Source Address [i] fields contain
> the source" and "address(es) of interest." — §4.1.11, `rfc9776.txt:654`, `rfc9776.txt:655`,
> `rfc9776.txt:656`, `rfc9776.txt:657`, `rfc9776.txt:658`, `rfc9776.txt:658-659`,
> `rfc9776.txt:659-660`, `rfc9776.txt:660`

- Strength: description. Class: wire.
- Check idea: trigger a Group-and-Source Specific Query for a chosen address and source list
  and check the Group Address and Source Address fields carry them.

### RFC9776-QRY-28

**A General Query is sent to the IP destination address 224.0.0.1, the all-systems multicast
address.**

> "In IGMPv3, General Queries are sent with an IP destination address of" and "224.0.0.1, the
> all-systems multicast address." — §4.1.12, `rfc9776.txt:664`, `rfc9776.txt:664-665`

- Strength: description. Class: wire.
- Check idea: trigger a General Query and check its IP destination address is 224.0.0.1.

### RFC9776-QRY-29

**A Group Specific or Group-and-Source Specific Query is sent to an IP destination address
equal to the multicast address of interest.**

> "Group Specific and" and "Group-and-Source Specific Queries are sent with an IP destination"
> and "address equal to the multicast address of interest." — §4.1.12, `rfc9776.txt:665`,
> `rfc9776.txt:666`, `rfc9776.txt:667`

- Strength: description. Class: wire.
- Check idea: trigger a Group Specific Query for a chosen address and check the IP destination
  address equals it.

### RFC9776-QRY-30

**A system MUST accept and process a Query whose IP Destination Address field contains any
address assigned to the interface on which the Query arrives.**

> "a" and "system MUST accept and process any Query whose IP Destination Address" and "field
> contains any of the addresses (unicast or multicast) assigned" and "to the interface on which
> the Query arrives." — §4.1.12, `rfc9776.txt:667`, `rfc9776.txt:668`, `rfc9776.txt:669`,
> `rfc9776.txt:670`

- Strength: must. Class: end-to-end.
- Check idea: send a system a Query addressed to a unicast address assigned to its receiving
  interface and check the system still processes the Query.
## Version 3 Membership Report message

### RFC9776-REP-1

**A system sends an IGMPv3 Membership Report to tell neighboring routers its current or
changed multicast reception state.**

> "IGMPv3 Membership Reports are sent by IP systems to report (to" and "neighboring routers)
> the current multicast reception state, or" and "changes in the multicast reception state, of
> their interfaces." — §4.2, `rfc9776.txt:674`, `rfc9776.txt:675`, `rfc9776.txt:676`

- Strength: description. Class: wire.
- Check idea: observe the link during normal operation and check that IGMPv3 Membership
  Reports originate only from a system reporting its own reception state.

### RFC9776-REP-2

**A system sets the Reserved field of a Report to zero when it sends the Report.**

> "The Reserved field is set to zero on transmission" — §4.2.1, `rfc9776.txt:739-740`

- Strength: description. Class: wire.
- Check idea: capture a Report a system sends and check the Reserved field is zero.

### RFC9776-REP-3

**A system ignores the Reserved field of a Report it receives.**

> "ignored on" and "reception." — §4.2.1, `rfc9776.txt:739`, `rfc9776.txt:740`

- Strength: description. Class: end-to-end.
- Check idea: send a system a Report with a non-zero Reserved field and check the system
  processes the Report as if the field were zero.

### RFC9776-REP-4

**The Checksum field is the 16-bit one's complement of the one's complement sum of the whole
IGMP message, computed with the Checksum field itself set to zero.**

> "The Checksum field is the 16-bit one's complement of the one's" and "complement sum of the
> whole IGMP message (the entire IP payload)." and "For computing the checksum, the Checksum
> field is set to zero." — §4.2.2, `rfc9776.txt:744`, `rfc9776.txt:745`, `rfc9776.txt:746`

- Strength: description. Class: wire.
- Check idea: capture a Report, recompute the checksum over the message with the Checksum
  field zeroed, and check the result equals the field on the wire.

### RFC9776-REP-5

**A system MUST verify the checksum of a received Report before it processes the message.**

> "When" and "receiving packets, the checksum MUST be verified before processing a" and
> "message." — §4.2.2, `rfc9776.txt:746`, `rfc9776.txt:747`, `rfc9776.txt:748`

- Strength: must. Class: end-to-end.
- Check idea: send a router a Report with a wrong checksum and check the router does not act
  on its content.

### RFC9776-REP-6

**The Flags field is a bitstring managed by the IGMP Type Numbers registry.**

> "The Flags field is a bitstring managed by the" and "registry defined in [BCP57]." — §4.2.3,
> `rfc9776.txt:752-753`

- Strength: description. Class: encoding.
- Check idea: capture a Report and check that the Flags bits match the current IGMP Type
  Numbers registry allocation.

### RFC9776-REP-7

**The Number of Group Records field states how many Group Records are present in the Report.**

> "The Number of Group Records (M) field specifies how many Group" and "Records are present in
> this Report." — §4.2.4, `rfc9776.txt:757`, `rfc9776.txt:758`

- Strength: description. Class: wire.
- Check idea: build a Report with a chosen number of Group Records and check that the Number of
  Group Records field equals that count.

### RFC9776-REP-8

**A Group Record holds information about the sender's membership in one multicast group on the
interface from which the Report is sent.**

> "Each Group Record is a block of fields containing information" and "pertaining to the
> sender's membership in a single multicast group on" and "the interface from which the Report
> is sent." — §4.2.5, `rfc9776.txt:762`, `rfc9776.txt:763`, `rfc9776.txt:764`

- Strength: description. Class: wire.
- Check idea: capture a Report and check that each Group Record it carries pertains to a single
  multicast address on the sending interface.

### RFC9776-REP-9

**The Aux Data Len field states the length of the Auxiliary Data field in 32-bit words, and
zero means there is no auxiliary data.**

> "The Aux Data Len field contains the length of the Auxiliary Data" and "field in this Group
> Record, in units of 32-bit words." and "It may contain" and "zero, to indicate the absence of
> any auxiliary data." — §4.2.7, `rfc9776.txt:772`, `rfc9776.txt:773`, `rfc9776.txt:773`,
> `rfc9776.txt:774`

- Strength: may (lower case). Class: encoding.
- Check idea: build a Group Record with no auxiliary data and check that its Aux Data Len field
  is zero.

### RFC9776-REP-10

**The Number of Sources field states how many source addresses are present in this Group
Record.**

> "The Number of Sources (N) field specifies how many source addresses" and "are present in
> this Group Record." — §4.2.8, `rfc9776.txt:778`, `rfc9776.txt:779`

- Strength: description. Class: wire.
- Check idea: build a Group Record with a chosen count of source addresses and check the Number
  of Sources field equals that count.

### RFC9776-REP-11

**The Multicast Address field holds the IP multicast address the Group Record pertains to.**

> "The Multicast Address field contains the IP multicast address to" and "which this Group
> Record pertains." — §4.2.9, `rfc9776.txt:783`, `rfc9776.txt:784`

- Strength: description. Class: wire.
- Check idea: build a Group Record for a chosen multicast address and check the Multicast
  Address field carries it.

### RFC9776-REP-12

**The Source Address fields of a Group Record are a vector of source addresses, one for each
count in this record's Number of Sources field.**

> "The Source Address [i] fields are a vector of n IP unicast addresses," and "where n is the
> value in this record's Number of Sources (N) field." — §4.2.10, `rfc9776.txt:788`,
> `rfc9776.txt:789`

- Strength: description. Class: wire.
- Check idea: build a Group Record with n sources and check it carries exactly n Source Address
  fields, matching its Number of Sources field.

### RFC9776-REP-13

**The Auxiliary Data field, when present, holds extra information about the Group Record, but
IGMPv3 defines no auxiliary data.**

> "The Auxiliary Data field, if present, contains additional information" and "pertaining to
> this Group Record." and "The protocol specified in this" and "document, IGMPv3, does not
> define any auxiliary data." — §4.2.11, `rfc9776.txt:793`, `rfc9776.txt:794`,
> `rfc9776.txt:794`, `rfc9776.txt:795`

- Strength: description. Class: wire.
- Check idea: capture a Group Record and check that any Auxiliary Data it carries has no
  defined meaning under this document.

### RFC9776-REP-14

**A system MUST NOT include auxiliary data in a Group Record it transmits, and MUST set the
Aux Data Len field to zero.**

> "implementations of IGMPv3 MUST NOT include any auxiliary data (i.e.," and "MUST set the Aux
> Data Len field to zero) in any transmitted Group" — §4.2.11, `rfc9776.txt:796`,
> `rfc9776.txt:797`

- Strength: must. Class: wire.
- Check idea: capture a Group Record a system sends and check the Aux Data Len field is zero
  and no auxiliary data octets follow.

### RFC9776-REP-15

**A system MUST ignore any auxiliary data present in a Group Record it receives.**

> "MUST ignore any auxiliary data present in any received" and "Group Record." — §4.2.11,
> `rfc9776.txt:798`, `rfc9776.txt:798-799`

- Strength: must. Class: end-to-end.
- Check idea: send a system a Group Record that carries auxiliary data and check the system's
  processing of the record is unaffected by that data.

### RFC9776-REP-16

**On a received Report with extra octets beyond the last Group Record, a system MUST include
those octets when it verifies the IGMP Checksum, but MUST otherwise ignore them.**

> "If the Packet Length field in the IP header of a received Report" and "indicates that there
> are additional octets of data present, beyond" and "the last Group Record, IGMPv3
> implementations MUST include those" and "octets in the computation to verify the received
> IGMP Checksum but" and "MUST otherwise ignore those additional octets." — §4.2.12,
> `rfc9776.txt:805`, `rfc9776.txt:806`, `rfc9776.txt:807`, `rfc9776.txt:808`,
> `rfc9776.txt:809`

- Strength: must. Class: end-to-end.
- Check idea: send a Report padded with trailing octets and a checksum computed over them, and
  check the system accepts it while treating the trailing octets as having no other effect.

### RFC9776-REP-17

**When a system sends a Report, it MUST NOT include additional octets beyond the last Group
Record.**

> "When sending a" and "Report, an IGMPv3 implementation MUST NOT include additional octets"
> and "beyond the last Group Record." — §4.2.12, `rfc9776.txt:809`, `rfc9776.txt:809-810`,
> `rfc9776.txt:811`

- Strength: must. Class: wire.
- Check idea: capture a Report a system sends and check its length ends exactly at the last
  Group Record, with no trailing octets.

### RFC9776-REP-18

**A system sends a Current-State Record in a Report sent in response to a Query, to state its
current reception state for one multicast address.**

> "A Current-State Record is sent by a system in response to a Query" and "received on an
> interface.  It reports the current reception state" and "of that interface, with respect to a
> single multicast address." — §4.2.13, `rfc9776.txt:818`, `rfc9776.txt:819`,
> `rfc9776.txt:820`

- Strength: description. Class: wire.
- Check idea: send a system a Query and check that its responding Report carries a
  Current-State Record for each multicast address of interest on that interface.

### RFC9776-REP-19

**A Current-State Record of type MODE_IS_INCLUDE means the interface's filter-mode is INCLUDE
for the address, and the Source Address fields list the interface's non-empty source list.**

> "MODE_IS_INCLUDE - indicates that the interface has a filter-" and "mode of INCLUDE for the
> specified multicast address." and "The" and "Source Address [i] fields in this Group Record
> contain the" and "interface's source-list for the specified multicast address," and "if it is
> non-empty." — §4.2.13, `rfc9776.txt:824`, `rfc9776.txt:825`, `rfc9776.txt:825`,
> `rfc9776.txt:826`, `rfc9776.txt:827`, `rfc9776.txt:828`

- Strength: description. Class: wire.
- Check idea: set an interface to INCLUDE filter-mode with a non-empty source list for an
  address and check its Current-State Record is MODE_IS_INCLUDE with that source list.

### RFC9776-REP-20

**A Current-State Record of type MODE_IS_EXCLUDE means the interface's filter-mode is EXCLUDE
for the address, and the Source Address fields list the interface's non-empty source list.**

> "MODE_IS_EXCLUDE - indicates that the interface has a filter-" and "mode of EXCLUDE for the
> specified multicast address." and "The" and "Source Address [i] fields in this Group Record
> contain the" and "interface's source-list for the specified multicast address," — §4.2.13,
> `rfc9776.txt:830`, `rfc9776.txt:831`, `rfc9776.txt:831`, `rfc9776.txt:832`,
> `rfc9776.txt:833`

- Strength: description. Class: wire.
- Check idea: set an interface to EXCLUDE filter-mode with a non-empty source list for an
  address and check its Current-State Record is MODE_IS_EXCLUDE with that source list.

### RFC9776-REP-21

**An SSM-aware host SHOULD NOT send a MODE_IS_EXCLUDE record for a multicast address within the
SSM address range, because SSM-aware routers ignore it.**

> "An SSM-aware host" and "SHOULD NOT send a" and "MODE_IS_EXCLUDE record type for multicast
> addresses that fall" and "within the SSM address range as they will be ignored by SSM-" and
> "aware routers [RFC4604]." — §4.2.13, `rfc9776.txt:834`, `rfc9776.txt:834`,
> `rfc9776.txt:835`, `rfc9776.txt:836`, `rfc9776.txt:837`

- Strength: should not. Class: wire.
- Check idea: set an SSM-aware host to EXCLUDE filter-mode for an address in the SSM range and
  check that its Report does not carry a MODE_IS_EXCLUDE record for that address.

### RFC9776-REP-22

**A system sends a Filter-Mode-Change Record, in the Report from the interface where the
change occurred, whenever a local request changes the interface's filter-mode between INCLUDE
and EXCLUDE.**

> "A Filter-Mode-Change Record is sent by a system whenever a local" and "invocation of
> IPMulticastListen causes a change of the filter-mode" and "(i.e., a change from INCLUDE to
> EXCLUDE, or from EXCLUDE to" and "INCLUDE) of the interface-level state entry for a particular"
> and "multicast address." and "The Record is included in a Report sent from" and "the
> interface on which the change occurred." — §4.2.13, `rfc9776.txt:839`, `rfc9776.txt:840`,
> `rfc9776.txt:841`, `rfc9776.txt:842`, `rfc9776.txt:843`, `rfc9776.txt:843`,
> `rfc9776.txt:844`

- Strength: description. Class: wire.
- Check idea: change an interface's filter-mode for an address from INCLUDE to EXCLUDE and
  check that the next Report from that interface carries a Filter-Mode-Change Record for it.

### RFC9776-REP-23

**CHANGE_TO_INCLUDE_MODE means the interface has changed to INCLUDE filter-mode for the
address, and the Source Address fields list the new non-empty source list.**

> "CHANGE_TO_INCLUDE_MODE - indicates that the interface has" and "changed to INCLUDE
> filter-mode for the specified multicast" and "address." and "The Source Address [i] fields in
> this Group Record" and "contain the interface's new source-list for the specified" and
> "multicast address, if it is non-empty." — §4.2.13, `rfc9776.txt:847`, `rfc9776.txt:848`,
> `rfc9776.txt:849`, `rfc9776.txt:849`, `rfc9776.txt:850`, `rfc9776.txt:851`

- Strength: description. Class: wire.
- Check idea: change an interface from EXCLUDE to INCLUDE filter-mode for an address with a
  non-empty source list and check the Report carries a CHANGE_TO_INCLUDE_MODE record with that
  list.

### RFC9776-REP-24

**CHANGE_TO_EXCLUDE_MODE means the interface has changed to EXCLUDE filter-mode for the
address, and the Source Address fields list the new non-empty source list.**

> "CHANGE_TO_EXCLUDE_MODE - indicates that the interface has" and "changed to EXCLUDE
> filter-mode for the specified multicast" and "address." and "The Source Address [i] fields in
> this Group Record" and "contain the interface's new source-list for the specified" —
> §4.2.13, `rfc9776.txt:853`, `rfc9776.txt:854`, `rfc9776.txt:855`, `rfc9776.txt:855`,
> `rfc9776.txt:856`

- Strength: description. Class: wire.
- Check idea: change an interface from INCLUDE to EXCLUDE filter-mode for an address with a
  non-empty source list and check the Report carries a CHANGE_TO_EXCLUDE_MODE record with that
  list.

### RFC9776-REP-25

**An SSM-aware host SHOULD NOT send a CHANGE_TO_EXCLUDE_MODE record for a multicast address
within the SSM address range.**

> "An SSM-aware host" and "SHOULD NOT send a CHANGE_TO_EXCLUDE_MODE record type for" and
> "multicast addresses that fall within the SSM address range." — §4.2.13,
> `rfc9776.txt:857`, `rfc9776.txt:858`, `rfc9776.txt:859`

- Strength: should not. Class: wire.
- Check idea: change an SSM-aware host to EXCLUDE filter-mode for an address in the SSM range
  and check that no CHANGE_TO_EXCLUDE_MODE record for it appears in the Report.

### RFC9776-REP-26

**A system sends a Source-List-Change Record, in the Report from the interface where the
change occurred, whenever a local request changes the interface's source list without
changing its filter-mode.**

> "A Source-List-Change Record is sent by a system whenever a local" and "invocation of
> IPMulticastListen causes a change of the source-list" and "that is not coincident with a
> change of the filter-mode, of the" and "interface-level state entry for a particular
> multicast address." and "The Record is included in a Report sent from the interface on" and
> "which the change occurred." — §4.2.13, `rfc9776.txt:861`, `rfc9776.txt:862`,
> `rfc9776.txt:863`, `rfc9776.txt:864`, `rfc9776.txt:865`, `rfc9776.txt:866`

- Strength: description. Class: wire.
- Check idea: add a source to an interface's INCLUDE source list without changing filter-mode
  and check the Report carries a Source-List-Change Record from that interface.

### RFC9776-REP-27

**ALLOW_NEW_SOURCES lists, in the Source Address fields, the sources newly added to an INCLUDE
list or removed from an EXCLUDE list.**

> "ALLOW_NEW_SOURCES - indicates that the Source Address [i]" and "fields in this Group Record
> contain a list of the additional" and "sources that the system wishes to receive, for packets
> sent to" and "the specified multicast address." and "If the change was to an" and "INCLUDE
> source-list, these are the addresses that were added" and "to the list; if the change was to
> an EXCLUDE source-list," and "these are the addresses that were deleted from the list." —
> §4.2.13, `rfc9776.txt:869`, `rfc9776.txt:870`, `rfc9776.txt:871`, `rfc9776.txt:872`,
> `rfc9776.txt:872`, `rfc9776.txt:873`, `rfc9776.txt:874`, `rfc9776.txt:875`

- Strength: description. Class: wire.
- Check idea: add a source to an interface's INCLUDE list and check the Report carries an
  ALLOW_NEW_SOURCES record listing that source.

### RFC9776-REP-28

**BLOCK_OLD_SOURCES lists, in the Source Address fields, the sources removed from an INCLUDE
list or added to an EXCLUDE list.**

> "BLOCK_OLD_SOURCES - indicates that the Source Address [i]" and "fields in this Group Record
> contain a list of the sources that" and "the system no longer wishes to receive, for packets
> sent to" and "the specified multicast address." and "If the change was to an" and "INCLUDE
> source-list, these are the addresses that were deleted" and "from the list; if the change was
> to an EXCLUDE source-list," and "these are the addresses that were added to the list." —
> §4.2.13, `rfc9776.txt:877`, `rfc9776.txt:878`, `rfc9776.txt:879`, `rfc9776.txt:880`,
> `rfc9776.txt:880`, `rfc9776.txt:881`, `rfc9776.txt:882`, `rfc9776.txt:883`

- Strength: description. Class: wire.
- Check idea: remove a source from an interface's INCLUDE list and check the Report carries a
  BLOCK_OLD_SOURCES record listing that source.

### RFC9776-REP-29

**When a source-list change both allows new sources and blocks old sources for the same
address, a system sends two Group Records for it: one ALLOW_NEW_SOURCES and one
BLOCK_OLD_SOURCES.**

> "If a change of source-list results in both allowing new sources and" and "blocking old
> sources, then two Group Records are sent for the same" and "multicast address, one of type
> ALLOW_NEW_SOURCES and one of type" and "BLOCK_OLD_SOURCES." — §4.2.13, `rfc9776.txt:885`,
> `rfc9776.txt:886`, `rfc9776.txt:887`, `rfc9776.txt:888`

- Strength: description. Class: wire.
- Check idea: change an interface's INCLUDE source list by adding one source and removing
  another at once, and check the Report carries both an ALLOW_NEW_SOURCES and a
  BLOCK_OLD_SOURCES record for the address.

### RFC9776-REP-30

**A system MUST silently ignore an unrecognized Group Record Type value.**

> "Unrecognized Record Type values MUST be silently ignored." — §4.2.13, `rfc9776.txt:893`

- Strength: must. Class: end-to-end.
- Check idea: send a router a Report that carries a Group Record with an undefined Record Type
  and check that the router gives no error report and does not act on the record's content.

### RFC9776-REP-31

**A Report is sent with a valid unicast IPv4 source address for the destination subnet.**

> "An IGMP report is sent with a valid unicast IPv4 source address for" and "the destination
> subnet." — §4.2.14, `rfc9776.txt:897`, `rfc9776.txt:897-898`

- Strength: description. Class: wire.
- Check idea: capture a Report a system sends and check the IP source address is a valid
  unicast address of the sending subnet.

### RFC9776-REP-32

**A system that has not yet acquired an IP address MAY use 0.0.0.0 as its Report's source
address.**

> "The 0.0.0.0 source address may be used by a" and "system that has not yet acquired an IP
> address." — §4.2.14, `rfc9776.txt:898`, `rfc9776.txt:898-899`

- Strength: may (lower case). Class: wire.
- Check idea: have a system without an assigned IP address send a Report and check the IP
  source address is 0.0.0.0.

### RFC9776-REP-33

**A router MUST accept a Report with a source address of 0.0.0.0.**

> "Routers MUST accept a report with a source address of" and "0.0.0.0." — §4.2.14,
> `rfc9776.txt:901`, `rfc9776.txt:901-902`

- Strength: must. Class: end-to-end.
- Check idea: send a router a Report with source address 0.0.0.0 and check the router still
  processes it.

### RFC9776-REP-34

**A Version 3 Report is sent to the IP destination address 224.0.0.22, to which every
IGMPv3-capable multicast router listens.**

> "Version 3 Reports are sent with an IP destination address of" and "224.0.0.22, to which all
> IGMPv3-capable multicast routers listen." — §4.2.15, `rfc9776.txt:906`, `rfc9776.txt:906-907`

- Strength: description. Class: wire.
- Check idea: capture a Version 3 Report a system sends and check its IP destination address is
  224.0.0.22.

### RFC9776-REP-35

**A system operating in v1 or v2 compatibility mode sends its Report to the multicast group
named in the Report's Group Address field.**

> "A" and "system that is operating in v1 or v2 compatibility modes sends v1 or" and "v2
> Reports to the multicast group specified in the Group Address" and "field of the Report." —
> §4.2.15, `rfc9776.txt:907`, `rfc9776.txt:908`, `rfc9776.txt:909`, `rfc9776.txt:909-910`

- Strength: description. Class: wire.
- Check idea: put a system in v2 compatibility mode for a group and check that its v2 Report's
  IP destination address equals the group address in the Report.

### RFC9776-REP-36

**A system MUST accept and process a v1 or v2 Report whose IP Destination Address field
contains any address assigned to the interface on which the Report arrives.**

> "a system MUST accept and process" and "any v1 or v2 Report whose IP Destination Address
> field contains any" and "of the addresses (unicast or multicast) assigned to the interface
> on" and "which the Report arrives." — §4.2.15, `rfc9776.txt:910`, `rfc9776.txt:911`,
> `rfc9776.txt:912`, `rfc9776.txt:913`

- Strength: must. Class: end-to-end.
- Check idea: send a system a v2 Report addressed to a unicast address assigned to its
  receiving interface and check the system still processes the Report.

### RFC9776-REP-37

**When the Group Records required in a Report do not fit within the size limit of a single
Report message, a system sends them in as many Report messages as needed.**

> "If the set of Group Records required in a report does not fit within" and "the size limit of
> a single Report message (as determined by the MTU" and "of the network on which it will be
> sent), the Group Records are sent" and "in as many Report messages as needed to report the
> entire set." — §4.2.17, `rfc9776.txt:939`, `rfc9776.txt:940`, `rfc9776.txt:941`,
> `rfc9776.txt:942`

- Strength: description. Class: wire.
- Check idea: give a system more Group Records than fit in one message and check that it sends
  them split across several Report messages, together covering the whole set.

### RFC9776-REP-38

**When a single Group Record with a type other than MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE
has too many source addresses for one message, a system splits it into several Group Records,
each sent in a separate Report message.**

> "If a single Group Record contains so many source addresses that it" and "does not fit within
> the size limit of a single Report message, and if" and "its Type is not MODE_IS_EXCLUDE or
> CHANGE_TO_EXCLUDE_MODE, it is" and "split into multiple Group Records, each containing a
> different subset" and "of the source addresses and each sent in a separate Report message." —
> §4.2.17, `rfc9776.txt:944`, `rfc9776.txt:945`, `rfc9776.txt:946`, `rfc9776.txt:947`,
> `rfc9776.txt:948`

- Strength: description. Class: wire.
- Check idea: build an ALLOW_NEW_SOURCES record with more sources than fit in one message and
  check that the system splits it across several Group Records, each in its own Report message.

### RFC9776-REP-39

**When a single MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE Group Record has too many source
addresses for one message, a system sends one Group Record with as many source addresses as
fit and leaves out the rest.**

> "If its Type is MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE, a single" and "Group Record is
> sent, containing as many source addresses as can fit," and "and the remaining source
> addresses are not reported" — §4.2.17, `rfc9776.txt:949`, `rfc9776.txt:950`,
> `rfc9776.txt:951`

- Strength: description. Class: wire.
- Check idea: build a MODE_IS_EXCLUDE record with more sources than fit in one message and
  check the system sends a single Group Record holding as many sources as fit, with the rest
  absent.

## Group member: action on change of interface state

### RFC9776-HOST-1

**A group member runs the group-member protocol on every interface that supports multicast reception, even when two interfaces reach the same network.**

> "A system performs the protocol described in this section over all
> interfaces on which multicast reception is supported, even if more
> than one of those interfaces is connected to the same network." — §5, `rfc9776.txt:967-969`

- Strength: description. Class: internal.
- Check idea: Give a host two interfaces attached to the same network. Confirm the host applies the group-member protocol on both interfaces independently.

### RFC9776-HOST-2

**A group member keeps a MulticastRouterVersion variable for each interface that supports multicast reception.**

> "systems maintain a MulticastRouterVersion variable for each
> interface on which multicast reception is supported." — §5, `rfc9776.txt:972-973`

- Strength: description. Class: internal.
- Check idea: Inspect a host's per-interface state. Confirm a MulticastRouterVersion value exists for every interface that supports multicast reception.

### RFC9776-HOST-3

**Every system permanently accepts packets to 224.0.0.1 from any source, on every interface that supports multicast reception.**

> "On all systems -- that is, all hosts and routers including
> multicast routers -- reception of packets destined to the all-systems
> multicast address, from all sources, is permanently enabled on all
> interfaces on which multicast reception is supported." — §5, `rfc9776.txt:980-983`

- Strength: description. Class: end-to-end.
- Check idea: Send a packet to 224.0.0.1 from an arbitrary source to a host or router that took no join action. Confirm the node accepts the packet.

### RFC9776-HOST-4

**A group member never sends an IGMP message about the all-systems multicast address.**

> "No IGMP
> messages are ever sent regarding the all-systems multicast address." — §5, `rfc9776.txt:983-984`

- Strength: description. Class: wire.
- Check idea: Observe a group member's IGMP traffic over time. Confirm no Group Record ever names 224.0.0.1 as its multicast address.

### RFC9776-HOST-5

**A group member silently ignores a received IGMP message that is not a Query, except as older-version interoperation requires.**

> "Received IGMP messages of types other than Query are silently
> ignored, except as required for interoperation with earlier versions
> of IGMP." — §5, `rfc9776.txt:994-996`

- Strength: description. Class: end-to-end.
- Check idea: Send a group member an IGMP message of a type other than Query. Confirm the member takes no protocol action beyond what older-version interoperation requires.

### RFC9776-HOST-6

**A change in an interface's multicast reception state makes the host send a State-Change Report from that interface right away.**

> "A change of interface state causes the system to immediately transmit
> a State-Change Report from that interface." — §5.1, `rfc9776.txt:1010-1011`

- Strength: description. Class: wire.
- Check idea: Trigger a reception-state change on an interface. Confirm a State-Change Report leaves that interface without delay.

### RFC9776-HOST-7

**A missing interface state, before or after a change, counts as INCLUDE mode with an empty source list.**

> "If no interface state
> existed for that multicast address before the change (i.e., the
> change consisted of creating a new per-interface record), or if no
> state exists after the change (i.e., the change consisted of deleting
> a per-interface record), then the "non-existent" state is considered
> to have a filter-mode of INCLUDE and an empty source-list." — §5.1, `rfc9776.txt:1014-1019`

- Strength: description. Class: internal.
- Check idea: Create a brand-new per-interface record for a multicast address, and separately delete one. Confirm the "before" or "after" state used to build the report is INCLUDE with no sources.

### RFC9776-HOST-8

**A change from INCLUDE(A) to INCLUDE(B) reports ALLOW of the added sources and BLOCK of the removed sources.**

> "| Old State   | New State   | State-Change Record Sent |" — §5.1, `rfc9776.txt:1022`, and
> "| INCLUDE (A) | INCLUDE (B) | ALLOW (B-A), BLOCK (A-B) |" — §5.1, `rfc9776.txt:1024`

- Strength: description. Class: wire.
- Check idea: Change a group's interface state from INCLUDE(A) to INCLUDE(B). Confirm the State-Change Report carries an ALLOW record for B-A and a BLOCK record for A-B.

### RFC9776-HOST-9

**A change from EXCLUDE(A) to EXCLUDE(B) reports ALLOW of the sources removed from exclusion and BLOCK of the sources added to exclusion.**

> "| EXCLUDE (A) | EXCLUDE (B) | ALLOW (A-B), BLOCK (B-A) |" — §5.1, `rfc9776.txt:1026`

- Strength: description. Class: wire.
- Check idea: Change a group's interface state from EXCLUDE(A) to EXCLUDE(B). Confirm the State-Change Report carries an ALLOW record for A-B and a BLOCK record for B-A.

### RFC9776-HOST-10

**A change from INCLUDE(A) to EXCLUDE(B) reports one TO_EX record with source list B.**

> "| INCLUDE (A) | EXCLUDE (B) | TO_EX (B)                |" — §5.1, `rfc9776.txt:1028`

- Strength: description. Class: wire.
- Check idea: Change a group's interface state from INCLUDE(A) to EXCLUDE(B). Confirm the State-Change Report carries a single TO_EX(B) record.

### RFC9776-HOST-11

**A change from EXCLUDE(A) to INCLUDE(B) reports one TO_IN record with source list B.**

> "| EXCLUDE (A) | INCLUDE (B) | TO_IN (B)                |" — §5.1, `rfc9776.txt:1030`

- Strength: description. Class: wire.
- Check idea: Change a group's interface state from EXCLUDE(A) to INCLUDE(B). Confirm the State-Change Report carries a single TO_IN(B) record.

### RFC9776-HOST-12

**A host leaves an ALLOW or BLOCK record with an empty source list out of the Report message.**

> "If the computed source-list for either an ALLOW or a BLOCK State-
> Change Record is empty, that record is omitted from the Report
> message." — §5.1, `rfc9776.txt:1035-1037`

- Strength: description. Class: wire.
- Check idea: Trigger a state change whose computed ALLOW or BLOCK source list is empty. Confirm that record is missing from the transmitted Report.

### RFC9776-HOST-13

**A host retransmits a State-Change Report [Robustness Variable]-1 more times, at random intervals within [Unsolicited Report Interval].**

> "To cover the possibility of the State-Change Report being missed by
> one or more multicast routers, it is retransmitted [Robustness
> Variable] - 1 more times, at intervals chosen at random from the
> range (0, [Unsolicited Report Interval])." — §5.1, `rfc9776.txt:1039-1042`

- Strength: description. Class: wire.
- Check idea: Trigger one state change. Count the retransmitted State-Change Reports and measure the gaps between them.

### RFC9776-HOST-14

**A further change to the same interface-state entry, before earlier retransmissions finish, sends a new State-Change Report right away.**

> "If more changes to the same interface state entry occur before all
> the retransmissions of the State-Change Report for the first change
> have been completed, each such additional change triggers the
> immediate transmission of a State-Change Report." — §5.1, `rfc9776.txt:1044-1047`

- Strength: description. Class: wire.
- Check idea: Trigger a second state change on the same entry while the first change's retransmissions are still pending. Confirm a new report leaves at once.

### RFC9776-HOST-15

**The difference records for a later change are merged into the still-pending report instead of being sent as their own message.**

> "The report records expressing the difference are built according to
> Table 3.  However, these records are not transmitted in a message but
> instead are merged with the contents of the pending report to create
> the new State-Change report." — §5.1, `rfc9776.txt:1051-1055`

- Strength: description. Class: internal.
- Check idea: Trigger a second interface-state change while a State-Change Report retransmission is still pending. Confirm the new difference is folded into that pending report rather than sent by itself.

### RFC9776-HOST-16

**Sending the merged State-Change Report stops the earlier report's retransmissions and starts a fresh count of [Robustness Variable] transmissions.**

> "The transmission of the merged State-Change Report terminates
> retransmissions of the earlier State-Change Reports for the same
> multicast address, and becomes the first of [Robustness Variable]
> transmissions of State-Change Reports." — §5.1, `rfc9776.txt:1059-1062`

- Strength: description. Class: wire.
- Check idea: After a merged report is sent, count remaining retransmissions. Confirm the earlier report's series stopped and a new series of [Robustness Variable] reports started.

### RFC9776-HOST-17

**A host keeps retransmission state for a source until it has sent [Robustness Variable] State-Change Reports that include the source.**

> "Each time a source is included in the difference report calculated
> above, retransmission state for that source needs to be maintained
> until [Robustness Variable] State-Change Reports have been sent by
> the host." — §5.1, `rfc9776.txt:1064-1067`

- Strength: description. Class: internal.
- Check idea: Add a source to a difference report. Count how many further State-Change Reports still name that source before its retransmission state is dropped.

### RFC9776-HOST-18

**A filter-mode change makes the next [Robustness Variable] State-Change Reports carry a Filter-Mode-Change Record, whatever source-list changes happen meanwhile.**

> "If the interface reception-state change that triggers the new report
> is a filter-mode change, then the next [Robustness Variable] State-
> Change Reports will include a Filter-Mode-Change Record.  This
> applies even if any number of source-list changes occur in that
> period." — §5.1, `rfc9776.txt:1070-1074`

- Strength: description. Class: wire.
- Check idea: Trigger a filter-mode change, then several source-list changes before the retransmissions finish. Confirm all [Robustness Variable] reports still carry a Filter-Mode-Change Record.

### RFC9776-HOST-19

**A host keeps retransmission state for the group until it has sent the [Robustness Variable] State-Change Reports for the filter-mode change.**

> "The host has to maintain retransmission state for the group
> until the [Robustness Variable] State-Change Reports have been sent." — §5.1, `rfc9776.txt:1074-1075`

- Strength: description. Class: internal.
- Check idea: After a filter-mode change, inspect the host's group retransmission state across the [Robustness Variable] reports and confirm it persists until the last one is sent.

### RFC9776-HOST-20

**Once the Filter-Mode-Change Records are exhausted, a still-pending source-list change makes the next report carry Source-List-Change Records instead.**

> "When [Robustness Variable] State-Change Reports with Filter-Mode-
> Change Records have been transmitted after the last filter-mode
> change, and if source-list changes to the interface reception have
> scheduled additional reports, then the next State-Change Report will
> include Source-List-Change Records." — §5.1, `rfc9776.txt:1076-1080`

- Strength: description. Class: wire.
- Check idea: Make a filter-mode change followed by a source-list change. After the [Robustness Variable] Filter-Mode-Change reports are sent, confirm the next report carries Source-List-Change Records.

### RFC9776-HOST-21

**A retransmitted Filter-Mode-Change Record is a TO_IN record when the current filter-mode is INCLUDE, and a TO_EX record otherwise.**

> "If the report should contain a Filter-Mode-
> Change Record, and if the current filter-mode of the interface is
> INCLUDE, a TO_IN record is included in the report; otherwise, a TO_EX
> record is included." — §5.1, `rfc9776.txt:1083-1086`

- Strength: should (lower case). Class: wire.
- Check idea: Schedule a Filter-Mode-Change report while the interface is INCLUDE, and separately while it is EXCLUDE. Confirm the record type is TO_IN in the first case and TO_EX in the second.

### RFC9776-HOST-22

**A retransmitted Source-List-Change Record carries an ALLOW record and a BLOCK record.**

> "If instead the report should contain a Source-
> List-Change Record, an ALLOW and a BLOCK record are included." — §5.1, `rfc9776.txt:1086-1088`

- Strength: should (lower case). Class: wire.
- Check idea: Schedule a Source-List-Change report. Confirm it carries both an ALLOW record and a BLOCK record.

### RFC9776-HOST-23

**A retransmitted TO_IN record lists every source in the current interface state that must be forwarded.**

> "| Record | Sources Included             |" — §5.1, `rfc9776.txt:1091`, and
> "| TO_IN  | All in the current interface |
> |        | state that must be forwarded |" — §5.1, `rfc9776.txt:1093-1094`

- Strength: must (lower case). Class: wire.
- Check idea: Build a retransmitted TO_IN record. Confirm it lists exactly the sources of the current interface state that must be forwarded.

### RFC9776-HOST-24

**A retransmitted TO_EX record lists every source in the current interface state that must be blocked.**

> "| TO_EX  | All in the current interface |
> |        | state that must be blocked   |" — §5.1, `rfc9776.txt:1096-1097`

- Strength: must (lower case). Class: wire.
- Check idea: Build a retransmitted TO_EX record. Confirm it lists exactly the sources of the current interface state that must be blocked.

### RFC9776-HOST-25

**A retransmitted ALLOW record lists every source with retransmission state that must be forwarded.**

> "| ALLOW  | All with retransmission      |
> |        | state that must be forwarded |" — §5.1, `rfc9776.txt:1099-1100`

- Strength: must (lower case). Class: wire.
- Check idea: Build a retransmitted ALLOW record. Confirm it lists exactly the sources with retransmission state that must be forwarded.

### RFC9776-HOST-26

**A retransmitted BLOCK record lists every source with retransmission state that must be blocked.**

> "| BLOCK  | All with retransmission      |
> |        | state that must be blocked   |" — §5.1, `rfc9776.txt:1102-1103`

- Strength: must (lower case). Class: wire.
- Check idea: Build a retransmitted BLOCK record. Confirm it lists exactly the sources with retransmission state that must be blocked.

### RFC9776-HOST-27

**An empty ALLOW or BLOCK record is left out of a retransmitted State-Change Report.**

> "If the computed source-list for either an ALLOW or a BLOCK record is
> empty, that record is omitted from the State-Change Report." — §5.1, `rfc9776.txt:1108-1109`

- Strength: description. Class: wire.
- Check idea: Build a retransmitted report whose computed ALLOW or BLOCK source list is empty. Confirm that record is missing from the report.

## Group member: action on reception of a query

### RFC9776-HQRY-1

**A group member delays its response to a Query by a random time bounded by the Max Response Time derived from the Max Resp Code.**

> "When a system receives a Query, it does not respond immediately.
> Instead, it delays its response by a random amount of time, bounded
> by the Max Response Time value derived from the Max Resp Code in the
> received Query Message." — §5.2, `rfc9776.txt:1118-1121`

- Strength: description. Class: end-to-end.
- Check idea: Send a group member a Query with a known Max Resp Code. Measure the delay before its response and confirm it is random and bounded by the derived Max Response Time.

### RFC9776-HQRY-2

**A group member keeps a per-interface timer for General-Query responses, a per-group-and-interface timer for Group and Group-and-Source Specific Query responses, and a per-group-and-interface list of sources for a pending Group-and-Source response.**

> "the system must be able
> to maintain the following state:" — §5.2, `rfc9776.txt:1128-1129`, and
> "A timer per interface for scheduling responses to General Queries." — §5.2, `rfc9776.txt:1131`, and
> "A per-group and Interface Timer for scheduling responses to Group
> Specific and Group-and-Source Specific Queries." — §5.2, `rfc9776.txt:1133-1134`, and
> "A per-group and interface list of sources to be reported in the
> response to a Group-and-Source Specific Query." — §5.2, `rfc9776.txt:1136-1137`

- Strength: must (lower case). Class: internal.
- Check idea: Inspect a group member's per-interface and per-group state. Confirm each of the three state items exists.

### RFC9776-HQRY-3

**On receiving a Query with the Router Alert option, and only when it has state to report, a member picks a random response delay from (0, Max Response Time).**

> "When a Query with the Router Alert option arrives on an interface,
> provided the system has state to report, a delay for a response is
> randomly selected in the range (0, [Max Response Time]) where Max
> Response Time is derived from Max Resp Code in the received Query
> Message." — §5.2, `rfc9776.txt:1139-1143`

- Strength: description. Class: end-to-end.
- Check idea: Send a Query with the Router Alert option to a member that has no state to report, and separately to one that has state. Confirm a delay is scheduled only in the second case.

### RFC9776-HQRY-4

**When an earlier General-Query response is already due sooner, a member schedules no extra response for a new Query.**

> "If there is a pending response to a previous General Query
> scheduled sooner than the selected delay, no additional response
> needs to be scheduled." — §5.2, `rfc9776.txt:1147-1149`

- Strength: description. Class: internal.
- Check idea: Send two General Queries close together so the second one's selected delay is later than the first's pending response. Confirm no second response is scheduled.

### RFC9776-HQRY-5

**A General Query schedules its response on the Interface Timer after the selected delay, canceling any earlier pending General-Query response.**

> "If the received Query is a General Query, the Interface Timer is
> used to schedule a response to the General Query after the selected
> delay.  Any previously pending response to a General Query is
> canceled." — §5.2, `rfc9776.txt:1151-1154`

- Strength: description. Class: internal.
- Check idea: Send a General Query while an earlier General-Query response is still pending with a later due time. Confirm the Interface Timer is reset to the new delay and the old pending response is gone.

### RFC9776-HQRY-6

**A Group Specific or Group-and-Source Specific Query with no pending response for the group schedules a report on the Group Timer.**

> "If the received Query is a Group Specific Query or a Group-and-
> Source Specific Query and there is no pending response to a previous
> Query for this group, then the Group Timer is used to schedule a
> report." — §5.2, `rfc9776.txt:1156-1159`

- Strength: description. Class: internal.
- Check idea: Send a member a Group Specific Query for a group with no pending response. Confirm the Group Timer for that group is set to schedule a report.

### RFC9776-HQRY-7

**A Group-and-Source Specific Query with no pending group response records its queried sources for use when the response is built.**

> "If the received Query is a Group-and-Source
> Specific Query, the list of queried sources is recorded to be
> used when generating a response." — §5.2, `rfc9776.txt:1159-1161`

- Strength: description. Class: internal.
- Check idea: Send a member a Group-and-Source Specific Query with no pending response for the group. Confirm the queried source list is recorded for the scheduled response.

### RFC9776-HQRY-8

**A new Group Specific Query, or an empty recorded source list, clears the group's source list and reschedules a single Group-Timer response at the earlier of the two delays.**

> "If there already is a pending response to a previous Query
> scheduled for this group, and either the new Query is a Group
> Specific Query or the recorded source-list associated with the
> group is empty, then the group source-list is cleared and a
> single response is scheduled using the Group Timer.  The new
> response is scheduled to be sent at the earliest of the remaining
> time for the pending report and the selected delay." — §5.2, `rfc9776.txt:1163-1169`

- Strength: description. Class: internal.
- Check idea: With a pending group response already scheduled, send a Group Specific Query for the same group. Confirm the source list is cleared and one response remains, timed at the earlier delay.

### RFC9776-HQRY-9

**A Group-and-Source Specific Query with a pending non-empty-source-list response adds its sources to the recorded list and reschedules a single response at the earlier delay.**

> "If the received Query is a Group-and-Source Specific Query and
> there is a pending response for this group with a non-empty
> source-list, then the group source-list is augmented to contain
> the list of sources in the new Query and a single response is
> scheduled using the Group Timer.  The new response is scheduled
> to be sent at the earliest of the remaining time for the pending
> report and the selected delay." — §5.2, `rfc9776.txt:1171-1177`

- Strength: description. Class: internal.
- Check idea: With a pending non-empty group-source response, send a Group-and-Source Specific Query for the same group. Confirm the new sources are added and one response remains, timed at the earlier delay.

### RFC9776-HQRY-10

**Expiry of the Interface Timer sends one Current-State Record, carrying filter-mode and source list, for every address with reception state on that interface, packed into as few Report messages as possible.**

> "If the expired timer is the Interface Timer (i.e., it is a
> pending response to a General Query), then one Current-State
> Record is sent for each multicast address for which the specified
> interface has reception state, as described in Section 3.2.  The
> Current-State Record carries the multicast address and its
> associated filter-mode (MODE_IS_INCLUDE or MODE_IS_EXCLUDE) and
> source-list.  Multiple Current-State Records are packed into
> individual Report messages, to the extent possible." — §5.2, `rfc9776.txt:1184-1191`

- Strength: description. Class: wire.
- Check idea: Let a member join several groups, then let its Interface Timer expire. Confirm one Current-State Record appears for each group with the correct filter-mode and source list, packed across as few Report messages as possible.

### RFC9776-HQRY-11

**An implementation should spread General-Query Report transmissions across the interval (0, Max Response Time) rather than using a single timer.**

> "Instead of using
> a single Interface Timer, implementations are recommended to
> spread transmission of such Report messages over the interval (0,
> [Max Response Time])." — §5.2, `rfc9776.txt:1194-1197`

- Strength: description. Class: wire.
- Check idea: Let a member with many joined groups respond to a General Query. Measure whether its Report messages are spread across the response interval instead of sent in a single burst.

### RFC9776-HQRY-12

**An implementation must not send a Report immediately after receiving a General Query.**

> "Note that any such implementation MUST
> avoid the "ack-implosion" problem, i.e., MUST NOT send a Report
> immediately on reception of a General Query." — §5.2, `rfc9776.txt:1197-1199`

- Strength: must not. Class: wire.
- Check idea: Send a member a General Query. Confirm no Report leaves immediately in response.

### RFC9776-HQRY-13

**Expiry of a Group Timer with no recorded sources sends one Current-State Record for that address, only if the interface still has reception state for it.**

> "If the expired timer is a Group Timer and the list of recorded
> sources for that group is empty (i.e., it is a pending response
> to a Group Specific Query), then if and only if the interface has
> reception state for that group address, a single Current-State
> Record is sent for that address.  The Current-State Record
> carries the multicast address and its associated filter-mode
> (MODE_IS_INCLUDE or MODE_IS_EXCLUDE) and source-list." — §5.2, `rfc9776.txt:1201-1207`

- Strength: description. Class: wire.
- Check idea: Let a Group Timer with no recorded sources expire, once while the interface still has reception state for the group and once after it was dropped. Confirm a Current-State Record is sent only in the first case.

### RFC9776-HQRY-14

**Expiry of a Group Timer with recorded sources builds the Current-State Record from Table 5, only if the interface still has reception state for the address.**

> "If the expired timer is a Group Timer and the list of recorded
> sources for that group is non-empty (i.e., it is a pending
> response to a Group-and-Source Specific Query), then if and only
> if the interface has reception state for that group address, the
> contents of the responding Current-State Record is determined
> from the interface state and the pending response record, as
> specified in Table 5." — §5.2, `rfc9776.txt:1209-1215`

- Strength: description. Class: wire.
- Check idea: Let a Group Timer with recorded sources expire while the interface still has reception state. Confirm the Current-State Record content follows Table 5.

### RFC9776-HQRY-15

**When the interface state is INCLUDE(A), the Current-State Record for pending sources B is IS_IN(A intersect B).**

> "| Per-Interface State | Set of Sources in the   | Current-State |" — §5.2, `rfc9776.txt:1218`, and
> "| INCLUDE (A)         | B                       | IS_IN (A*B)   |" — §5.2, `rfc9776.txt:1221`

- Strength: description. Class: wire.
- Check idea: Set an interface's state to INCLUDE(A) with a pending response listing sources B. Confirm the resulting Current-State Record is IS_IN(A*B).

### RFC9776-HQRY-16

**When the interface state is EXCLUDE(A), the Current-State Record for pending sources B is IS_IN(B minus A).**

> "| EXCLUDE (A)         | B                       | IS_IN (B-A)   |" — §5.2, `rfc9776.txt:1223`

- Strength: description. Class: wire.
- Check idea: Set an interface's state to EXCLUDE(A) with a pending response listing sources B. Confirm the resulting Current-State Record is IS_IN(B-A).

### RFC9776-HQRY-17

**A Current-State Record with an empty source-address set is not sent.**

> "If the resulting Current-State Record has an empty set of source
> addresses, then no response is sent." — §5.2, `rfc9776.txt:1228-1229`

- Strength: description. Class: wire.
- Check idea: Build a Current-State Record whose computed source-address set is empty. Confirm no response is sent for it.

### RFC9776-HQRY-18

**After sending the required Report messages, a member clears the recorded source lists of the groups it reported.**

> "Finally, after any required Report messages have been generated, the
> source-lists associated with any reported groups are cleared." — §5.2, `rfc9776.txt:1231-1232`

- Strength: description. Class: internal.
- Check idea: Let a member send its scheduled Report messages. Inspect its per-group recorded source lists afterward and confirm they are cleared.

## Router: conditions for queries

### RFC9776-RQ-1

**A multicast router runs the router protocol on each directly attached network, using only one interface when several interfaces reach the same network.**

> "A multicast router performs the protocol described in this section
> over each of its directly attached networks.  If a multicast router
> has more than one interface to the same network, it only needs to
> operate this protocol over one of those interfaces." — §6, `rfc9776.txt:1251-1254`

- Strength: description. Class: internal.
- Check idea: Give a router two interfaces to the same network. Confirm the router protocol runs over only one of them.

### RFC9776-RQ-2

**A router must enable reception of 224.0.0.22 from all sources on every interface running this protocol.**

> "On each
> interface over which this protocol is being run, the router MUST
> enable reception of multicast address 224.0.0.22 from all sources" — §6, `rfc9776.txt:1254-1256`

- Strength: must. Class: end-to-end.
- Check idea: Send a router a packet to 224.0.0.22 from an arbitrary source, on an interface running the protocol. Confirm the router accepts it.

### RFC9776-RQ-3

**A router must also perform the group-member part of IGMPv3 for 224.0.0.22 on that interface.**

> "(and
> MUST perform the group member part of IGMPv3 for that address on
> that interface)." — §6, `rfc9776.txt:1256-1258`

- Strength: must. Class: internal.
- Check idea: Inspect a router's interface running IGMPv3. Confirm it also carries out the group-member protocol for 224.0.0.22.

### RFC9776-RQ-4

**A router needs to know only that some system wants a source's packets; it need not track each system's interest individually.**

> "Multicast routers need to know only that at least one system on an
> attached network is interested in packets to a particular multicast
> address from a particular source; a multicast router is not required
> to keep track of the interests of each individual neighboring system." — §6, `rfc9776.txt:1260-1263`

- Strength: description. Class: internal.
- Check idea: Let two systems on a network want the same source and group. Confirm the router's kept state does not distinguish which system asked.

### RFC9776-RQ-5

**An IGMPv3 router must also implement IGMPv1 and IGMPv2.**

> "IGMPv3 multicast routers MUST also implement versions 1 and
> 2 of the protocol" — §6, `rfc9776.txt:1268-1269`

- Strength: must. Class: internal.
- Check idea: Configure a router as an IGMPv3 router. Confirm it can also act as an IGMPv1 router and as an IGMPv2 router.

### RFC9776-RQ-6

**A router sends General Queries periodically to build and refresh the group membership state of an attached network.**

> "Multicast routers send General Queries periodically to request group
> membership information from an attached network.  These Queries are
> used to build and refresh the group membership state of systems on
> attached networks." — §6.1, `rfc9776.txt:1273-1276`

- Strength: description. Class: wire.
- Check idea: Observe a router's interface over time. Confirm General Queries leave it at a periodic rate.

### RFC9776-RQ-7

**Before a router deletes a group or source and prunes its traffic, it queries for other members or listeners.**

> "When a group membership is terminated at a system or traffic from a
> particular source is no longer desired, a multicast router must query
> for other members of the group or listeners of the source before
> deleting the group (or source) and pruning its traffic." — §6.1, `rfc9776.txt:1286-1289`

- Strength: must (lower case). Class: wire.
- Check idea: Let the last interested system on a network leave a group or source. Confirm the router sends a query before it deletes the group or source record.

### RFC9776-RQ-8

**A router sends a Group Specific Query to confirm no system still wants a group, once it receives a State-Change Record for a system leaving that group.**

> "A Group
> Specific Query is sent to verify there are no systems that desire
> reception of the specified group or to "rebuild" the desired
> reception state for a particular group.  Group Specific Queries are
> sent when a router receives a State-Change Record indicating a system
> is leaving a group." — §6.1, `rfc9776.txt:1292-1297`

- Strength: description. Class: wire.
- Check idea: Send a router a State-Change Record showing a system leaving a group. Confirm the router sends a Group Specific Query for that group.

### RFC9776-RQ-9

**A router sends a Group-and-Source Specific Query, listing the sources that were asked to stop being forwarded, to check whether any system still wants them.**

> "A Group-and-Source Specific Query is used to verify there are no
> systems on a network that desire receiving traffic from a set of
> sources.  Group-and-Source Specific Queries list sources for a
> particular group that have been requested to no longer be forwarded.
> This query is sent by a multicast router to learn if any systems
> desire reception of packets to the specified group address from the
> specified source addresses." — §6.1, `rfc9776.txt:1299-1305`

- Strength: description. Class: wire.
- Check idea: Send a router a record asking to stop forwarding a set of sources for a group. Confirm the router sends a Group-and-Source Specific Query listing exactly those sources.

### RFC9776-RQ-10

**A router sends a Group-and-Source Specific Query only after a State-Change Record, never after a Current-State Record.**

> "Group-and-Source Specific Queries are
> only sent in response to State-Change Records and never in response
> to Current-State Records." — §6.1, `rfc9776.txt:1305-1307`

- Strength: description. Class: wire.
- Check idea: Send a router a Current-State Record naming sources to block. Confirm no Group-and-Source Specific Query results, unlike the case of an equivalent State-Change Record.

## Router state: filter mode, group timers and source timers

### RFC9776-RST-1

**A router keeps, per group per attached network, a record of the multicast address, Group Timer, Router Filter Mode, and source records.**

> "This group state
> consists of a filter-mode, a list of sources, and various timers." — §6.2, `rfc9776.txt:1313-1314`, and
> "(multicast address, Group Timer, Router Filter Mode, (source records))" — §6.2, `rfc9776.txt:1319`

- Strength: description. Class: internal.
- Check idea: Inspect a router's kept state for one group on one attached network. Confirm it holds a multicast address, a Group Timer, a Router Filter Mode, and a set of source records.

### RFC9776-RST-2

**Each source record of a router's group state holds a source address and a Source Timer.**

> "Each source record is of the form:

>     (source address, Source Timer)" — §6.2, `rfc9776.txt:1321-1323`

- Strength: description. Class: internal.
- Check idea: Inspect one source record in a router's group state. Confirm it holds a source address and a Source Timer.

### RFC9776-RST-3

**Wanting all sources of a group is recorded as EXCLUDE mode with an empty source-record list.**

> "If all sources within a given group are desired, an empty source
> record list is kept with filter-mode set to EXCLUDE." — §6.2, `rfc9776.txt:1325-1326`

- Strength: description. Class: internal.
- Check idea: Let every system on a network want all sources of a group. Confirm the router's state for that group is EXCLUDE with no source records.

### RFC9776-RST-4

**Once a router receives any Group Record with EXCLUDE filter-mode for a group, the Router Filter Mode for that group becomes EXCLUDE.**

> "As a rule, once a Group Record with a
> filter-mode of EXCLUDE is received, the Router Filter Mode for that
> group will be EXCLUDE." — §6.2.1, `rfc9776.txt:1345-1347`

- Strength: description. Class: internal.
- Check idea: Let the router's Filter Mode for a group be INCLUDE, then send it one Group Record with EXCLUDE filter-mode. Confirm the Router Filter Mode changes to EXCLUDE.

### RFC9776-RST-5

**In EXCLUDE mode, a router's source-record list holds two kinds of sources: ones that conflict between systems and must be forwarded, and ones every host asked to block.**

> "When a Router Filter Mode for a group is EXCLUDE, the source record
> list contains two types of sources.  The first type is the set that
> represents conflicts in the desired reception state; this set must be
> forwarded by some router on the network.  The second type is the set
> of sources that hosts have requested to not be forwarded." — §6.2.1, `rfc9776.txt:1349-1353`

- Strength: must (lower case). Class: internal.
- Check idea: Put a group's Router Filter Mode into EXCLUDE with conflicting and blocked sources present. Confirm the source-record list separates the two kinds.

### RFC9776-RST-6

**In INCLUDE mode, a router's source-record list is exactly the sources some system wants forwarded, and each must be forwarded by some router.**

> "When a Router Filter Mode for a group is INCLUDE, the source record
> list is the list of sources desired for the group.  This is the total
> desired set of sources for that group.  Each source in the source
> record list must be forwarded by some router on the network." — §6.2.1, `rfc9776.txt:1357-1360`

- Strength: must (lower case). Class: internal.
- Check idea: Put a group's Router Filter Mode into INCLUDE with a known set of wanted sources. Confirm the source-record list equals exactly that set.

### RFC9776-RST-7

**The Group Timer is a per-group, per-network decrementing timer, bounded below at zero, used only in EXCLUDE mode.**

> "The Group Timer is only used when a group is in EXCLUDE mode and it
> represents the time for the filter-mode of the group to expire and
> switch to INCLUDE mode.  We define a Group Timer as a decrementing
> timer with a lower bound of zero kept per group per attached network." — §6.2.2, `rfc9776.txt:1373-1376`

- Strength: description. Class: internal.
- Check idea: Inspect a group's Group Timer while the group is INCLUDE, and again while it is EXCLUDE. Confirm the timer runs only in the EXCLUDE case and never goes below zero.

### RFC9776-RST-8

**A group with INCLUDE filter-mode and a Group Timer of zero or more means all its members are in INCLUDE mode.**

> "| Group       | Group | Actions/Comments                        |" — §6.2.2, `rfc9776.txt:1391`, and
> "| INCLUDE     | Timer | All members in INCLUDE mode.            |
> |             | >= 0  |                                         |" — §6.2.2, `rfc9776.txt:1395-1396`

- Strength: description. Class: internal.
- Check idea: Put a group into INCLUDE filter-mode. Confirm the Group Timer holds a non-negative value and every member is treated as INCLUDE.

### RFC9776-RST-9

**A group with EXCLUDE filter-mode and a Group Timer greater than zero means at least one member is in EXCLUDE mode.**

> "| EXCLUDE     | Timer | At least one member in EXCLUDE mode.    |
> |             | > 0   |                                         |" — §6.2.2, `rfc9776.txt:1398-1399`

- Strength: description. Class: internal.
- Check idea: Put a group into EXCLUDE filter-mode with a positive Group Timer. Confirm at least one member is treated as EXCLUDE.

### RFC9776-RST-10

**A Group Timer at zero in EXCLUDE mode deletes the Group Record if all source timers have also expired, or else switches to INCLUDE mode using the sources with running timers.**

> "| EXCLUDE     | Timer | No more listeners to group.  If all     |
> |             | == 0  | source timers have expired, then delete |
> |             |       | Group Record.  If there are still       |
> |             |       | source record timers running, switch to |
> |             |       | INCLUDE filter-mode using those source  |
> |             |       | records with running timers as the      |
> |             |       | INCLUDE source record state.            |" — §6.2.2, `rfc9776.txt:1401-1407`

- Strength: description. Class: internal.
- Check idea: Let a group's Group Timer reach zero while EXCLUDE, once with all source timers also expired and once with some still running. Confirm the group is deleted in the first case and switched to INCLUDE with the running sources in the second.

### RFC9776-RST-11

**A Source Timer is a per-source-record decrementing timer with a lower bound of zero.**

> "A Source Timer is kept per source record and is a decrementing timer
> with a lower bound of zero." — §6.2.3, `rfc9776.txt:1414-1415`

- Strength: description. Class: internal.
- Check idea: Inspect a source record's Source Timer over time. Confirm it only decreases and never goes below zero.

### RFC9776-RST-12

**A router updates a source's Source Timer whenever that source appears in a received record for the group.**

> "Source timers
> are always updated (for a particular group) whenever the source is
> present in a received record for that group." — §6.2.3, `rfc9776.txt:1416-1419`

- Strength: description. Class: internal.
- Check idea: Send a router two records naming the same source for a group, spaced apart in time. Confirm the Source Timer is refreshed by the second record.

### RFC9776-RST-13

**In INCLUDE mode, a source record with a running Source Timer means some system currently wants that source.**

> "A source record with a running timer with a Router Filter Mode for
> the group of INCLUDE means that there is currently one or more
> systems (in INCLUDE filter-mode) that desire to receive that source." — §6.2.3, `rfc9776.txt:1421-1423`

- Strength: description. Class: internal.
- Check idea: Put a group in INCLUDE mode with a source record whose timer is running. Confirm at least one system still wants that source.

### RFC9776-RST-14

**In INCLUDE mode, an expired Source Timer makes the router delete that source record as no longer wanted.**

> "If a Source Timer expires with a Router Filter Mode for the group of
> INCLUDE, the router concludes that traffic from this particular
> source is no longer desired on the attached network and deletes the
> associated source record." — §6.2.3, `rfc9776.txt:1424-1427`

- Strength: description. Class: internal.
- Check idea: Let a source record's timer expire while the group is INCLUDE. Confirm the router deletes that source record.

### RFC9776-RST-15

**In EXCLUDE mode, a source record with a running Source Timer means at least one system wants that source forwarded.**

> "If a source record has a running timer with a
> Router Filter Mode for the group of EXCLUDE, it means that at least
> one system desires the source." — §6.2.3, `rfc9776.txt:1430-1432`

- Strength: description. Class: internal.
- Check idea: Put a group in EXCLUDE mode with a source record whose timer is running. Confirm at least one system wants that source forwarded.

### RFC9776-RST-16

**In EXCLUDE mode, an expired Source Timer makes the router tell the routing protocol that no receiver wants that source.**

> "If a Source Timer expires with a Router Filter Mode for the group of
> EXCLUDE, the router informs the routing protocol that there is no
> longer a receiver on the network interested in traffic from this
> source." — §6.2.3, `rfc9776.txt:1437-1440`

- Strength: description. Class: internal.
- Check idea: Let a source record's timer expire while the group is EXCLUDE. Confirm the router reports to the routing protocol that no receiver wants that source.

### RFC9776-RST-17

**In EXCLUDE mode, a router deletes source records only when the Group Timer expires.**

> "When a Router Filter Mode for a group is EXCLUDE, source records are
> only deleted when the Group Timer expires." — §6.2.3, `rfc9776.txt:1442-1443`

- Strength: description. Class: internal.
- Check idea: Let a source record's Source Timer expire while the group is EXCLUDE and the Group Timer is still running. Confirm the source record is not deleted until the Group Timer later expires.

## Source-specific forwarding rules

### RFC9776-FWD-1

**The multicast routing protocol should use IGMPv3 information so that every source or group wanted on a subnetwork is forwarded there.**

> "The multicast routing
> protocol in use is in charge of this decision and should use the
> IGMPv3 information to ensure that all sources/groups desired on a
> subnetwork are forwarded to that subnetwork." — §6.3, `rfc9776.txt:1451-1454`

- Strength: should (lower case). Class: internal.
- Check idea: Record a group's desired sources in IGMPv3 state on a subnetwork. Confirm the routing protocol forwards those sources to that subnetwork.

### RFC9776-FWD-2

**IGMPv3 information does not override multicast routing information; a router may still forward excluded-source traffic onto a transit subnet.**

> "IGMPv3 information does not override multicast routing information; for
> example, if the IGMPv3 filter-mode group for G is EXCLUDE, a router
> may still forward packets for excluded sources to a transit subnet." — §6.3, `rfc9776.txt:1454-1457`

- Strength: may (lower case). Class: internal.
- Check idea: Set a group's IGMPv3 filter-mode to EXCLUDE for a source, on a subnet that is transit for the routing protocol. Confirm the router may still forward that source's traffic there.

### RFC9776-FWD-3

**In INCLUDE mode with a positive Source Timer, IGMP suggests forwarding traffic from that source.**

> "| Group       | Group    | Action                                |" — §6.3, `rfc9776.txt:1466`, and
> "| INCLUDE     | TIMER >  | Suggest to forward traffic from       |
> |             | 0        | source.                               |" — §6.3, `rfc9776.txt:1470-1471`

- Strength: description. Class: internal.
- Check idea: Put a group in INCLUDE mode with a source whose timer is above zero. Confirm IGMP's suggestion to the routing protocol is to forward that source.

### RFC9776-FWD-4

**In INCLUDE mode with a Source Timer at zero, IGMP suggests stopping forwarding, removes the source record, and deletes the Group Record if none remain.**

> "| INCLUDE     | TIMER == | Suggest to stop forwarding traffic    |
> |             | 0        | from source and remove source record. |
> |             |          | If there are no more source records   |
> |             |          | for the group, delete Group Record.   |" — §6.3, `rfc9776.txt:1473-1476`

- Strength: description. Class: internal.
- Check idea: Let a source's timer reach zero while the group is INCLUDE, once with other sources remaining and once with none. Confirm the suggestion stops, the source record is removed, and the Group Record is deleted only when no sources remain.

### RFC9776-FWD-5

**In INCLUDE mode with no source elements, IGMP suggests not forwarding that source.**

> "| INCLUDE     | No       | Suggest to not forward source.        |
> |             | Source   |                                       |
> |             | Elements |                                       |" — §6.3, `rfc9776.txt:1478-1480`

- Strength: description. Class: internal.
- Check idea: Put a group in INCLUDE mode with no source elements for a given source. Confirm IGMP's suggestion is not to forward that source.

### RFC9776-FWD-6

**In EXCLUDE mode with a positive Source Timer, IGMP suggests forwarding traffic from that source.**

> "| EXCLUDE     | TIMER >  | Suggest to forward traffic from       |
> |             | 0        | source.                               |" — §6.3, `rfc9776.txt:1482-1483`

- Strength: description. Class: internal.
- Check idea: Put a group in EXCLUDE mode with a source whose timer is above zero. Confirm IGMP's suggestion is to forward that source.

### RFC9776-FWD-7

**In EXCLUDE mode with a Source Timer at zero, IGMP suggests not forwarding that source, but keeps the source record.**

> "| EXCLUDE     | TIMER == | Suggest to not forward traffic from   |
> |             | 0        | source (DO NOT remove record).        |" — §6.3, `rfc9776.txt:1485-1486`

- Strength: description. Class: internal.
- Check idea: Let a source's timer reach zero while the group is EXCLUDE. Confirm the suggestion stops but the source record is not removed.

### RFC9776-FWD-8

**In EXCLUDE mode with no source elements, IGMP suggests forwarding traffic from that source.**

> "| EXCLUDE     | No       | Suggest to forward traffic from       |
> |             | Source   | source.                               |
> |             | Elements |                                       |" — §6.3, `rfc9776.txt:1488-1490`

- Strength: description. Class: internal.
- Check idea: Put a group in EXCLUDE mode with no source elements for a given source. Confirm IGMP's suggestion is to forward that source.

## Router: reception of reports

### RFC9776-RREP-1

**An SSM-aware router should ignore a Group Record for an SSM address whose type is MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE.**

> "SSM-aware routers SHOULD ignore records that contain multicast
> addresses in the SSM address range if the record type is
> MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE." — §6.4, `rfc9776.txt:1497-1499`

- Strength: should. Class: end-to-end.
- Check idea: Send an SSM-aware router a Group Record of type MODE_IS_EXCLUDE for an SSM address. Confirm the router does not act on it.

### RFC9776-RREP-2

**An SSM-aware router should ignore an IGMPv1 or IGMPv2 Report, or an IGMPv2 Done message, for an SSM address.**

> "SSM-aware routers SHOULD
> ignore IGMPv1/IGMPv2 Report and IGMPv2 DONE messages that contain
> multicast addresses in the SSM address range" — §6.4, `rfc9776.txt:1499-1501`

- Strength: should. Class: end-to-end.
- Check idea: Send an SSM-aware router an IGMPv2 Report for an SSM address. Confirm the router does not act on it.

### RFC9776-RREP-3

**An SSM-aware router should not use such an older-version message for an SSM address to build IP forwarding state.**

> "SHOULD NOT use such
> Reports to establish IP forwarding state" — §6.4, `rfc9776.txt:1501-1502`

- Strength: should not. Class: internal.
- Check idea: Send an SSM-aware router an IGMPv1 or IGMPv2 Report for an SSM address. Confirm the router's IP forwarding state is unaffected.

### RFC9776-RREP-4

**An SSM-aware router may log an error when it receives such a message.**

> "and
> MAY log an error if it
> receives such a message." — §6.4, `rfc9776.txt:1502-1503`

- Strength: may. Class: internal.
- Check idea: Send an SSM-aware router an older-version message for an SSM address. Confirm an error log entry is an allowed outcome.

### RFC9776-RREP-5

**On receiving a Current-State Record, a router updates its group and source timers for that group.**

> "When receiving Current-State Records, a router updates both its group
> and source timers." — §6.4.1, `rfc9776.txt:1507-1509`

- Strength: description. Class: internal.
- Check idea: Send a router a Current-State Record for a known group. Confirm the group's timer and the named sources' timers change.

### RFC9776-RREP-6

**GMI, the Group Membership Interval, is the time in which a group membership will time out.**

> "The variable GMI is
> an abbreviation for the Group Membership Interval, which is the time
> in which group memberships will time out." — §6.4.1, `rfc9776.txt:1530-1532`

- Strength: description. Class: internal.
- Check idea: Set a source timer to GMI after a report. Confirm the group membership times out after that interval with no further reports.

### RFC9776-RREP-7

**LMQT, the Last Member Query Time, is the total time spent after [Last Member Query Count] retransmissions, and represents the router's leave latency.**

> "The variable LMQT is
> an abbreviation for the Last Member Query Time, which is the total time
> spent after [Last Member Query Count] retransmissions.  LMQT
> represents the leave latency or the difference between the
> transmission of a membership change and the change in the information
> given to the routing protocol." — §6.4.1, `rfc9776.txt:1532-1537`

- Strength: description. Class: internal.
- Check idea: Trigger a leave and measure the time from the state change to the routing protocol being told to stop forwarding. Confirm it equals LMQT.

### RFC9776-RREP-8

**A router in INCLUDE(A) that gets an IS_IN(B) Current-State Record moves to INCLUDE(A+B) and sets B's Source Timer to GMI.**

> "| Router  | Report | New       | Actions         |" — §6.4.1, `rfc9776.txt:1546`, and
> "| INCLUDE | IS_IN  | INCLUDE   | (B)=GMI         |
> | (A)     | (B)    | (A+B)     |                 |" — §6.4.1, `rfc9776.txt:1550-1551`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at INCLUDE(A), then send an IS_IN(B) Current-State Record. Confirm the state becomes INCLUDE(A+B) and B's Source Timer is set to GMI.

### RFC9776-RREP-9

**A router in INCLUDE(A) that gets an IS_EX(B) Current-State Record moves to EXCLUDE(A*B, B-A), zeroes the (B-A) timers, deletes (A-B), and sets the Group Timer to GMI.**

> "| INCLUDE | IS_EX  | EXCLUDE   | (B-A)=0         |
> | (A)     | (B)    | (A*B,B-A) | Delete (A-B)    |
> |         |        |           | Group Timer=GMI |" — §6.4.1, `rfc9776.txt:1553-1555`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at INCLUDE(A), then send an IS_EX(B) Current-State Record. Confirm the state becomes EXCLUDE(A*B, B-A), (A-B) is deleted, and the Group Timer is GMI.

### RFC9776-RREP-10

**A router in EXCLUDE(X,Y) that gets an IS_IN(A) Current-State Record moves to EXCLUDE(X+A, Y-A) and sets A's Source Timer to GMI.**

> "| EXCLUDE | IS_IN  | EXCLUDE   | (A)=GMI         |
> | (X,Y)   | (A)    | (X+A,Y-A) |                 |" — §6.4.1, `rfc9776.txt:1557-1558`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at EXCLUDE(X,Y), then send an IS_IN(A) Current-State Record. Confirm the state becomes EXCLUDE(X+A, Y-A) with A's timer set to GMI.

### RFC9776-RREP-11

**A router in EXCLUDE(X,Y) that gets an IS_EX(A) Current-State Record moves to EXCLUDE(A-Y, Y*A), sets (A-X-Y) timers to GMI, deletes (X-A) and (Y-A), and sets the Group Timer to GMI.**

> "| EXCLUDE | IS_EX  | EXCLUDE   | (A-X-Y)=GMI     |
> | (X,Y)   | (A)    | (A-Y,Y*A) | Delete (X-A)    |
> |         |        |           | Delete (Y-A)    |
> |         |        |           | Group Timer=GMI |" — §6.4.1, `rfc9776.txt:1560-1563`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at EXCLUDE(X,Y), then send an IS_EX(A) Current-State Record. Confirm the state becomes EXCLUDE(A-Y, Y*A), (X-A) and (Y-A) are deleted, and the Group Timer is GMI.

### RFC9776-RREP-12

**A router queries the sources that a system asked to stop forwarding to a group.**

> "Routers must query sources that are requested to be no longer
> forwarded to a group." — §6.4.2, `rfc9776.txt:1576-1577`

- Strength: must (lower case). Class: wire.
- Check idea: Send a router a record asking to stop forwarding a source. Confirm the router sends a query naming that source.

### RFC9776-RREP-13

**Sending or receiving a query for specific sources lowers their Source Timers to [Last Member Query Time].**

> "When a router queries or receives a query for a specific set of
> sources, it lowers its source timers for those sources to a small
> interval of [Last Member Query Time] seconds." — §6.4.2, `rfc9776.txt:1577-1579`

- Strength: description. Class: internal.
- Check idea: Send or receive a query naming a set of sources. Confirm each named source's timer drops to [Last Member Query Time].

### RFC9776-RREP-14

**A Group Record received during the query period that expresses interest in a queried source updates that source's timer.**

> "If Group Records are received in response to the queries which express
> interest in receiving traffic from the queried sources, the
> corresponding timers are updated." — §6.4.2, `rfc9776.txt:1579-1582`

- Strength: description. Class: internal.
- Check idea: Query a source, then send a Group Record expressing interest in it before the query period ends. Confirm the source's timer is updated.

### RFC9776-RREP-15

**Querying a specific group lowers its Group Timer to [Last Member Query Time].**

> "when a router queries a specific group, it lowers its
> Group Timer for that group to a small interval of [Last Member Query
> Time] seconds." — §6.4.2, `rfc9776.txt:1584-1586`

- Strength: description. Class: internal.
- Check idea: Send a Group Specific Query for a group. Confirm the group's Group Timer drops to [Last Member Query Time].

### RFC9776-RREP-16

**An EXCLUDE-mode Group Record received during the query interval updates the Group Timer, and the router keeps suggesting the group be forwarded without interruption.**

> "If any Group Records expressing EXCLUDE mode interest
> in the group are received within the interval, the Group Timer for
> the group is updated and the suggestion to the routing protocol to
> forward the group stands without any interruption." — §6.4.2, `rfc9776.txt:1586-1589`

- Strength: description. Class: internal.
- Check idea: Query a group, then send an EXCLUDE-mode Group Record for it within the query interval. Confirm the Group Timer is updated and the router's forwarding suggestion never lapses.

### RFC9776-RREP-17

**A router keeps suggesting to forward a queried group or source throughout [Last Member Query Time], and may prune it only after that time passes with no interested record.**

> "During a query period (i.e., [Last Member Query Time] seconds), the
> IGMP component in the router continues to suggest to the routing
> protocol that it forwards traffic from the groups or sources that it
> is querying.  It is not until after [Last Member Query Time] seconds
> without receiving a record expressing interest in the queried group
> or sources that the router may prune the group or sources from the
> network." — §6.4.2, `rfc9776.txt:1591-1597`

- Strength: may (lower case). Class: internal.
- Check idea: Query a group with no interested record arriving during [Last Member Query Time]. Confirm the router keeps suggesting to forward until that time elapses, then may prune.

### RFC9776-RREP-18

**A query that a router state table triggers is sent [Last Member Query Count] times, once every [Last Member Query Interval].**

> "queries sent by actions in
> Table 9 need to be transmitted [Last Member Query Count] times, once
> every [Last Member Query Interval]." — §6.4.2, `rfc9776.txt:1611-1613`

- Strength: description. Class: wire.
- Check idea: Trigger a Table 9 query action. Count the transmitted queries and the interval between them.

### RFC9776-RREP-19

**A newly scheduled query for a group merges with any already-pending query retransmissions for that group.**

> "If while scheduling new queries there are already pending queries to
> be retransmitted for the same group, the new and pending queries have
> to be merged." — §6.4.2, `rfc9776.txt:1615-1617`

- Strength: description. Class: internal.
- Check idea: Trigger a second query action for a group while an earlier query's retransmissions are still pending. Confirm the two merge into one retransmission schedule.

### RFC9776-RREP-20

**A router in INCLUDE(A) that gets an ALLOW(B) record moves to INCLUDE(A+B) and sets B's Source Timer to GMI.**

> "| Router  | Report | New Router  | Actions             |" — §6.4.2, `rfc9776.txt:1623`, and
> "| INCLUDE | ALLOW  | INCLUDE     | (B)=GMI             |
> | (A)     | (B)    | (A+B)       |                     |" — §6.4.2, `rfc9776.txt:1626-1627`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at INCLUDE(A), then send an ALLOW(B) record. Confirm the state becomes INCLUDE(A+B) and B's Source Timer is GMI.

### RFC9776-RREP-21

**A router in INCLUDE(A) that gets a BLOCK(B) record stays at INCLUDE(A) and sends a Group-and-Source Query for A intersect B.**

> "| INCLUDE | BLOCK  | INCLUDE (A) | Send Q(G,A*B)       |
> | (A)     | (B)    |             |                     |" — §6.4.2, `rfc9776.txt:1629-1630`

- Strength: description. Class: wire.
- Check idea: Put a router's group state at INCLUDE(A), then send a BLOCK(B) record. Confirm the state stays INCLUDE(A) and a Group-and-Source Specific Query for A*B is sent.

### RFC9776-RREP-22

**A router in INCLUDE(A) that gets a TO_EX(B) record moves to EXCLUDE(A*B, B-A), zeroes (B-A), deletes (A-B), sends a Group-and-Source Query for A*B, and sets the Group Timer to GMI.**

> "| INCLUDE | TO_EX  | EXCLUDE     | (B-A)=0             |
> | (A)     | (B)    | (A*B,B-A)   | Delete (A-B)        |
> |         |        |             | Send Q(G,A*B)       |
> |         |        |             | Group Timer=GMI     |" — §6.4.2, `rfc9776.txt:1632-1635`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at INCLUDE(A), then send a TO_EX(B) record. Confirm the state becomes EXCLUDE(A*B, B-A), (A-B) is deleted, a query for A*B is sent, and the Group Timer is GMI.

### RFC9776-RREP-23

**A router in INCLUDE(A) that gets a TO_IN(B) record moves to INCLUDE(A+B), sets B's timer to GMI, and sends a Group-and-Source Query for A-B.**

> "| INCLUDE | TO_IN  | INCLUDE     | (B)=GMI             |
> | (A)     | (B)    | (A+B)       | Send Q(G,A-B)       |" — §6.4.2, `rfc9776.txt:1637-1638`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at INCLUDE(A), then send a TO_IN(B) record. Confirm the state becomes INCLUDE(A+B), B's timer is GMI, and a query for A-B is sent.

### RFC9776-RREP-24

**A router in EXCLUDE(X,Y) that gets an ALLOW(A) record moves to EXCLUDE(X+A, Y-A) and sets A's Source Timer to GMI.**

> "| EXCLUDE | ALLOW  | EXCLUDE     | (A)=GMI             |
> | (X,Y)   | (A)    | (X+A,Y-A)   |                     |" — §6.4.2, `rfc9776.txt:1640-1641`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at EXCLUDE(X,Y), then send an ALLOW(A) record. Confirm the state becomes EXCLUDE(X+A, Y-A) with A's timer set to GMI.

### RFC9776-RREP-25

**A router in EXCLUDE(X,Y) that gets a BLOCK(A) record moves to EXCLUDE(X+(A-Y), Y), sets (A-X-Y) to the Group Timer value, and sends a Group-and-Source Query for A-Y.**

> "| EXCLUDE | BLOCK  | EXCLUDE     | (A-X-Y)=Group Timer |
> | (X,Y)   | (A)    | (X+(A-Y),Y) | Send Q(G,A-Y)       |" — §6.4.2, `rfc9776.txt:1643-1644`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at EXCLUDE(X,Y), then send a BLOCK(A) record. Confirm the state becomes EXCLUDE(X+(A-Y), Y) and a query for A-Y is sent.

### RFC9776-RREP-26

**A router in EXCLUDE(X,Y) that gets a TO_EX(A) record moves to EXCLUDE(A-Y, Y*A), sets (A-X-Y) to the Group Timer value, deletes (X-A) and (Y-A), sends a Group-and-Source Query for A-Y, and sets the Group Timer to GMI.**

> "| EXCLUDE | TO_EX  | EXCLUDE     | (A-X-Y)=Group Timer |
> | (X,Y)   | (A)    | (A-Y,Y*A)   | Delete (X-A)        |
> |         |        |             | Delete (Y-A)        |
> |         |        |             | Send Q(G,A-Y)       |
> |         |        |             | Group Timer=GMI     |" — §6.4.2, `rfc9776.txt:1646-1650`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at EXCLUDE(X,Y), then send a TO_EX(A) record. Confirm the state becomes EXCLUDE(A-Y, Y*A), (X-A) and (Y-A) are deleted, a query for A-Y is sent, and the Group Timer is GMI.

### RFC9776-RREP-27

**A router in EXCLUDE(X,Y) that gets a TO_IN(A) record moves to EXCLUDE(X+A, Y-A), sets A's timer to GMI, sends a Group-and-Source Query for X-A, and sends a Group Specific Query.**

> "| EXCLUDE | TO_IN  | EXCLUDE     | (A)=GMI             |
> | (X,Y)   | (A)    | (X+A,Y-A)   | Send Q(G,X-A)       |
> |         |        |             | Send Q(G)           |" — §6.4.2, `rfc9776.txt:1652-1654`

- Strength: description. Class: internal.
- Check idea: Put a router's group state at EXCLUDE(X,Y), then send a TO_IN(A) record. Confirm the state becomes EXCLUDE(X+A, Y-A), A's timer is GMI, and both a Group-and-Source Query for X-A and a Group Specific Query are sent.

## Switching router filter modes

### RFC9776-RSW-1

**When a group's Group Timer expires while the Router Filter Mode is EXCLUDE, the router assumes no more EXCLUDE-mode systems remain and switches the mode to INCLUDE.**

> "When a Group Timer expires with a Router Filter Mode of EXCLUDE, a
> router assumes that there are no systems with a filter-mode of
> EXCLUDE present on the attached network.  When a router's filter-mode
> for a group is EXCLUDE and the Group Timer expires, the Router Filter
> Mode for the group transitions to INCLUDE." — §6.5, `rfc9776.txt:1664-1668`

- Strength: description. Class: internal.
- Check idea: Put a group's Router Filter Mode at EXCLUDE and let its Group Timer expire. Confirm the mode changes to INCLUDE.

### RFC9776-RSW-2

**On switching to INCLUDE, a router keeps the source records whose Source Timer is still running and deletes those at zero.**

> "A router uses source records with running source timers as its state
> for the switch to a filter-mode of INCLUDE.  If there are any source
> records with source timers greater than zero (i.e., requested to be
> forwarded), a router switches to filter-mode of INCLUDE using those
> source records.  Source records whose timers are zero (from the
> previous EXCLUDE mode) are deleted." — §6.5, `rfc9776.txt:1670-1675`

- Strength: description. Class: internal.
- Check idea: Let a group's Group Timer expire in EXCLUDE mode with some source timers still running and some at zero. Confirm the new INCLUDE state keeps only the running-timer sources.

## Router: reception of queries, querier election and specific queries

### RFC9776-RQRY-1

**Sending or receiving a query with the Suppress Router-Side Processing flag clear makes a router update its timers for the queried group or sources.**

> "When a router sends or receives a query with a clear Suppress Router-
> Side Processing flag, it must update its timers to reflect the
> correct timeout values for the group or sources being queried." — §6.6.1, `rfc9776.txt:1685-1687`

- Strength: must (lower case). Class: internal.
- Check idea: Send or receive a query with the Suppress Router-Side Processing flag clear. Confirm the router's timer for the queried group or sources is updated.

### RFC9776-RQRY-2

**Receiving or sending Q(G,A) lowers the Source Timer of every source in A to LMQT.**

> "| Query  | Action                                            |" — §6.6.1, `rfc9776.txt:1693`, and
> "| Q(G,A) | Source Timer for sources in A are lowered to LMQT |" — §6.6.1, `rfc9776.txt:1695`

- Strength: description. Class: internal.
- Check idea: Send or receive a Q(G,A) query. Confirm every source named in A has its Source Timer lowered to LMQT.

### RFC9776-RQRY-3

**Receiving or sending Q(G) lowers the group's Group Timer to LMQT.**

> "| Q(G)   | Group Timer is lowered to LMQT                    |" — §6.6.1, `rfc9776.txt:1697`

- Strength: description. Class: internal.
- Check idea: Send or receive a Q(G) query. Confirm the group's Group Timer is lowered to LMQT.

### RFC9776-RQRY-4

**Sending or receiving a query with the S flag set leaves the router's timers unchanged.**

> "When a router sends or receives a query with the S flag set, it will
> not update its timers." — §6.6.1, `rfc9776.txt:1702-1703`

- Strength: description. Class: internal.
- Check idea: Send or receive a query with the S flag set. Confirm the group and source timers are unchanged afterward.

### RFC9776-RQRY-5

**IGMPv3 elects one querier per subnet by comparing IP addresses, the same way as IGMPv2.**

> "IGMPv3 elects a single querier per subnet using the same Querier
> election mechanism as IGMPv2, namely by IP address." — §6.6.2, `rfc9776.txt:1707-1709`

- Strength: description. Class: internal.
- Check idea: Put two routers with different IP addresses on the same subnet. Confirm the one with the lower address becomes and stays querier.

### RFC9776-RQRY-6

**Receiving a General Query from a lower IP address sets the Other-Querier-Present Timer to [Other Querier Present Interval] and stops the router sending General Queries if it had been the querier.**

> "When a router
> receives a General Query with a lower IP address, it sets the Other-
> Querier-Present Timer to [Other Querier Present Interval] and ceases
> to send general queries on the network if it was the previously
> elected querier." — §6.6.2, `rfc9776.txt:1708-1712`

- Strength: description. Class: internal.
- Check idea: Let a querier receive a General Query from a router with a lower IP address. Confirm its Other-Querier-Present Timer is set and it stops sending General Queries.

### RFC9776-RQRY-7

**A non-querier router should start sending General Queries once its Other-Querier-Present Timer expires.**

> "After its Other-Querier-Present Timer expires, it
> should begin sending General Queries." — §6.6.2, `rfc9776.txt:1712-1713`

- Strength: should (lower case). Class: wire.
- Check idea: Let a non-querier router's Other-Querier-Present Timer expire with no new General Query received. Confirm the router starts sending General Queries.

### RFC9776-RQRY-8

**A router that receives an older-version General Query must switch to the oldest IGMP version present on the network.**

> "If a router receives an older version General Query, it MUST use the
> oldest version of IGMP on the network." — §6.6.2, `rfc9776.txt:1715-1716`

- Strength: must. Class: internal.
- Check idea: Send a router an IGMPv1 General Query while it also observes IGMPv2 traffic on the network. Confirm the router uses IGMPv1 as the oldest version present.

### RFC9776-RQRY-9

**A "Send Q(G)" table action lowers the Group Timer to LMQT.**

> "When a table action "Send Q(G)" is encountered, the Group Timer must
> be lowered to LMQT." — §6.6.3.1, `rfc9776.txt:1723-1725`

- Strength: must (lower case). Class: internal.
- Check idea: Trigger a table action of "Send Q(G)". Confirm the Group Timer is lowered to LMQT.

### RFC9776-RQRY-10

**A "Send Q(G)" action sends a Group Specific Query at once and schedules [Last Member Query Count]-1 retransmissions every [Last Member Query Interval].**

> "The router must then immediately send a Group
> Specific Query as well as schedule [Last Member Query Count] - 1
> query retransmission(s) to be sent every [Last Member Query Interval]
> over [Last Member Query Time]." — §6.6.3.1, `rfc9776.txt:1724-1727`

- Strength: must (lower case). Class: wire.
- Check idea: Trigger a "Send Q(G)" action. Count the Group Specific Queries sent and the interval between them.

### RFC9776-RQRY-11

**A Group Specific Query sets the Suppress Router-Side Processing bit when the Group Timer exceeds LMQT.**

> "When transmitting a Group Specific Query, if the Group Timer is
> larger than LMQT, the "Suppress Router-Side Processing" bit is set in
> the Query Message." — §6.6.3.1, `rfc9776.txt:1729-1731`

- Strength: description. Class: wire.
- Check idea: Send a Group Specific Query while the Group Timer is above LMQT, and again while it is at or below LMQT. Confirm the Suppress Router-Side Processing bit is set only in the first case.

### RFC9776-RQRY-12

**A "Send Q(G,X)" action sets the retransmission count to [Last Member Query Count] and lowers the Source Timer to LMQT, for each source in X with a timer above LMQT.**

> "the following actions must be performed for each of the sources in X of group G, with the Source Timer larger
> than LMQT:

>    *  Set the number of retransmissions for each source to [Last Member
>       Query Count].

>    *  Lower the Source Timer to LMQT." — §6.6.3.2, `rfc9776.txt:1736-1743`

- Strength: must (lower case). Class: internal.
- Check idea: Trigger a "Send Q(G,X)" action for a set X where some sources have timers above LMQT and some do not. Confirm only the above-LMQT sources get their retransmission count set and their timer lowered to LMQT.

### RFC9776-RQRY-13

**A "Send Q(G,X)" action sends a Group-and-Source Specific Query at once and schedules [Last Member Query Count]-1 retransmissions every [Last Member Query Interval].**

> "The router must then immediately send a Group-and-Source Specific
> Query as well as schedule [Last Member Query Count] - 1 query
> retransmission(s) to be sent every [Last Member Query Interval] over
> [Last Member Query Time]." — §6.6.3.2, `rfc9776.txt:1745-1748`

- Strength: must (lower case). Class: wire.
- Check idea: Trigger a "Send Q(G,X)" action. Count the Group-and-Source Specific Queries sent and the interval between them.

### RFC9776-RQRY-14

**Building a Group-and-Source Specific Query sends two messages: one with the Suppress bit set for sources with timers above LMQT, one with the bit clear for sources with timers at or below LMQT.**

> "When building a Group-and-source Specific Query for group G, two
> separate Query Messages are sent for the group.  The first one has
> the "Suppress Router-Side Processing" bit set and contains all the
> sources with retransmission state and timers greater than LMQT.  The
> second has the "Suppress Router-Side Processing" bit clear and
> contains all the sources with retransmission state and timers lower
> or equal to LMQT." — §6.6.3.2, `rfc9776.txt:1751-1757`

- Strength: description. Class: wire.
- Check idea: Build a Group-and-Source Specific Query for a group with sources on both sides of LMQT. Confirm two messages result, split by the Suppress bit as specified.

### RFC9776-RQRY-15

**A Group-and-Source Specific Query message with no matching sources is not sent.**

> "If either of the two calculated messages does not
> contain any sources, then its transmission is suppressed." — §6.6.3.2, `rfc9776.txt:1757-1758`

- Strength: description. Class: wire.
- Check idea: Build a Group-and-Source Specific Query where all sources fall on one side of LMQT. Confirm the other message is not sent.

## Query version distinctions

### RFC9776-VER-1

**A Membership Query of length 8 octets with Max Resp Code zero is an IGMPv1 Query.**

> "IGMPv1 Query: length = 8 octets AND Max Resp Code field is zero" — §7.1, `rfc9776.txt:1779`

- Strength: description. Class: wire.
- Check idea: Send a Membership Query of length 8 octets with Max Resp Code zero. Confirm the receiver classifies it as an IGMPv1 Query.

### RFC9776-VER-2

**A Membership Query of length 8 octets with a non-zero Max Resp Code is an IGMPv2 Query.**

> "IGMPv2 Query: length = 8 octets AND Max Resp Code field is non-
> zero" — §7.1, `rfc9776.txt:1781-1782`

- Strength: description. Class: wire.
- Check idea: Send a Membership Query of length 8 octets with a non-zero Max Resp Code. Confirm the receiver classifies it as an IGMPv2 Query.

### RFC9776-VER-3

**A Membership Query of length 12 octets or more is an IGMPv3 Query.**

> "IGMPv3 Query: length >= 12 octets" — §7.1, `rfc9776.txt:1784`

- Strength: description. Class: wire.
- Check idea: Send a Membership Query of length 12 octets or more. Confirm the receiver classifies it as an IGMPv3 Query.

### RFC9776-VER-4

**A Query Message that matches none of the three length and code conditions must be silently ignored.**

> "Query Messages that do not match any of the above conditions (e.g., a
> Query of length 10 octets) MUST be silently ignored." — §7.1, `rfc9776.txt:1786-1787`

- Strength: must. Class: end-to-end.
- Check idea: Send a Query Message of length 10 octets. Confirm the receiver takes no protocol action on it.

## Group member behavior with older versions

### RFC9776-COMPH-1

**An IGMPv3 host must be able to operate in IGMPv1 and IGMPv2 compatibility modes.**

> "IGMPv3 hosts
> MUST operate in v1 and v2 compatibility modes." — §7.2.1, `rfc9776.txt:1793-1794`

- Strength: must. Class: internal.
- Check idea: Configure an IGMPv3 host on a network with an older-version querier. Confirm the host can operate in IGMPv1 and in IGMPv2 compatibility mode.

### RFC9776-COMPH-2

**An IGMPv3 host must keep per-interface state for the compatibility mode of each attached network.**

> "IGMPv3 hosts MUST
> keep state per local interface regarding the compatibility mode of
> each attached network." — §7.2.1, `rfc9776.txt:1794-1796`

- Strength: must. Class: internal.
- Check idea: Give a host two interfaces on two different networks. Confirm each interface carries its own compatibility-mode state.

### RFC9776-COMPH-3

**The Host Compatibility Mode variable, kept per interface, is IGMPv1, IGMPv2, or IGMPv3, driven by received General Queries and the Older-Version-Querier-Present Timer.**

> "A host's compatibility mode is determined
> from the Host Compatibility Mode variable, which can be in one of
> three states: IGMPv1, IGMPv2, or IGMPv3.  This variable is kept per
> interface and is dependent on the version of General Queries received
> on that interface as well as the Older-Version-Querier-Present Timer
> for the interface." — §7.2.1, `rfc9776.txt:1796-1801`

- Strength: description. Class: internal.
- Check idea: Inspect a host's per-interface Host Compatibility Mode variable. Confirm its value is always one of IGMPv1, IGMPv2, or IGMPv3.

### RFC9776-COMPH-4

**A host keeps an IGMPv1-Querier-Present Timer and an IGMPv2-Querier-Present Timer, one per interface.**

> "hosts keep
> both an IGMPv1-Querier-Present Timer and an IGMPv2-Querier-Present
> Timer per interface." — §7.2.1, `rfc9776.txt:1803-1805`

- Strength: description. Class: internal.
- Check idea: Inspect a host's interface state. Confirm both an IGMPv1-Querier-Present Timer and an IGMPv2-Querier-Present Timer exist for it.

### RFC9776-COMPH-5

**Receiving an IGMPv1 Membership Query sets the host's IGMPv1-Querier-Present Timer to [Older Version Querier Present Interval].**

> "IGMPv1-Querier-Present Timer is set to [Older
> Version Querier Present Interval] seconds whenever an IGMPv1
> Membership Query is received." — §7.2.1, `rfc9776.txt:1805-1807`

- Strength: description. Class: internal.
- Check idea: Send a host an IGMPv1 Membership Query. Confirm its IGMPv1-Querier-Present Timer is set to [Older Version Querier Present Interval].

### RFC9776-COMPH-6

**Receiving an IGMPv2 General Query sets the host's IGMPv2-Querier-Present Timer to [Older Version Querier Present Interval].**

> "IGMPv2-Querier-Present Timer is set to
> [Older Version Querier Present Interval] seconds whenever an IGMPv2
> General Query is received." — §7.2.1, `rfc9776.txt:1807-1809`

- Strength: description. Class: internal.
- Check idea: Send a host an IGMPv2 General Query. Confirm its IGMPv2-Querier-Present Timer is set to [Older Version Querier Present Interval].

### RFC9776-COMPH-7

**A host's compatibility mode changes on receiving an older-version query than its current mode, or on certain timer events.**

> "The Host Compatibility Mode of an interface changes whenever an older
> version query (than the current compatibility mode) is received or
> when certain timer conditions occur." — §7.2.1, `rfc9776.txt:1811-1813`

- Strength: description. Class: internal.
- Check idea: Put a host in IGMPv3 compatibility mode, then send it an IGMPv2 General Query. Confirm the compatibility mode changes.

### RFC9776-COMPH-8

**When the IGMPv1-Querier-Present Timer expires, a host switches to IGMPv2 mode if the IGMPv2 timer is still running, or to IGMPv3 mode if it is not.**

> "When the IGMPv1-Querier-Present
> Timer expires, a host switches to Host Compatibility Mode of IGMPv2
> if it has a running IGMPv2 Querier Present timer.  If it does not
> have a running IGMPv2 Querier Present timer, then it switches to Host
> Compatibility of IGMPv3." — §7.2.1, `rfc9776.txt:1813-1817`

- Strength: description. Class: internal.
- Check idea: Let a host's IGMPv1-Querier-Present Timer expire, once with the IGMPv2 timer still running and once without. Confirm it switches to IGMPv2 mode in the first case and IGMPv3 mode in the second.

### RFC9776-COMPH-9

**When the IGMPv2-Querier-Present Timer expires, a host switches to IGMPv3 mode.**

> "When the IGMPv2 Querier Present timer
> expires, a host switches to Host Compatibility Mode of IGMPv3." — §7.2.1, `rfc9776.txt:1817-1818`

- Strength: description. Class: internal.
- Check idea: Let a host's IGMPv2-Querier-Present Timer expire. Confirm its Host Compatibility Mode becomes IGMPv3.

### RFC9776-COMPH-10

**The Host Compatibility Mode reflects whether an older-version General Query arrived in the last [Older Version Querier Present Interval] seconds.**

> "The Host Compatibility Mode variable is based on whether an older
> version General Query was received in the last [Older Version Querier
> Present Interval] seconds." — §7.2.1, `rfc9776.txt:1820-1822`

- Strength: description. Class: internal.
- Check idea: Stop sending older-version General Queries to a host and wait past [Older Version Querier Present Interval]. Confirm its Host Compatibility Mode reflects the absence.

### RFC9776-COMPH-11

**An older-version Group Specific Query must not change the Host Compatibility Mode variable.**

> "The Host Compatibility Mode variable value MUST NOT be changed by an older version Group Specific Query." — §7.2.1, `rfc9776.txt:1822-1823`

- Strength: must not. Class: internal.
- Check idea: Send a host an IGMPv2 Group Specific Query while it is in IGMPv3 compatibility mode. Confirm the Host Compatibility Mode does not change.

### RFC9776-COMPH-12

**Host Compatibility Mode is IGMPv3, the default, when neither the IGMPv1 nor the IGMPv2 Querier Present timer is running.**

> "| Host Compatibility Mode | Timer State                            |" — §7.2.1, `rfc9776.txt:1827`, and
> "| IGMPv3 (default)        | IGMPv2 Querier Present not running and |
> |                         | IGMPv1 Querier Present not running     |" — §7.2.1, `rfc9776.txt:1829-1830`

- Strength: description. Class: internal.
- Check idea: Let both the IGMPv1-Querier-Present and IGMPv2-Querier-Present timers of a host's interface expire. Confirm the Host Compatibility Mode is IGMPv3.

### RFC9776-COMPH-13

**Host Compatibility Mode is IGMPv2 when the IGMPv2 Querier Present timer runs and the IGMPv1 one does not.**

> "| IGMPv2                  | IGMPv2 Querier Present running and     |
> |                         | IGMPv1 Querier Present not running     |" — §7.2.1, `rfc9776.txt:1832-1833`

- Strength: description. Class: internal.
- Check idea: Let a host's IGMPv2-Querier-Present timer run while the IGMPv1 one is not running. Confirm the Host Compatibility Mode is IGMPv2.

### RFC9776-COMPH-14

**Host Compatibility Mode is IGMPv1 whenever the IGMPv1 Querier Present timer is running.**

> "| IGMPv1                  | IGMPv1 Querier Present running         |" — §7.2.1, `rfc9776.txt:1835`

- Strength: description. Class: internal.
- Check idea: Let a host's IGMPv1-Querier-Present timer run. Confirm the Host Compatibility Mode is IGMPv1, whatever the IGMPv2 timer state.

### RFC9776-COMPH-15

**A host should switch compatibility mode right away once a received query updates its Querier Present timers.**

> "If a host receives a query that causes its Querier Present timers to
> be updated and correspondingly its compatibility mode, it should
> switch compatibility modes immediately." — §7.2.1, `rfc9776.txt:1840-1842`

- Strength: should (lower case). Class: internal.
- Check idea: Send a host a query that updates a Querier Present timer and changes its compatibility mode. Measure how soon the host's behavior reflects the new mode.

### RFC9776-COMPH-16

**In IGMPv3 Host Compatibility Mode, a host uses the IGMPv3 protocol on that interface.**

> "When Host Compatibility Mode is IGMPv3, a host acts using the IGMPv3
> protocol on that interface." — §7.2.1, `rfc9776.txt:1844-1845`

- Strength: description. Class: end-to-end.
- Check idea: Put a host's interface in IGMPv3 Host Compatibility Mode. Confirm its Reports use the IGMPv3 message format.

### RFC9776-COMPH-17

**In IGMPv2 Host Compatibility Mode, a host uses only the IGMPv2 protocol on that interface.**

> "When Host Compatibility Mode is IGMPv2,
> a host acts in IGMPv2 compatibility mode, using only the IGMPv2
> protocol, on that interface." — §7.2.1, `rfc9776.txt:1845-1847`

- Strength: description. Class: end-to-end.
- Check idea: Put a host's interface in IGMPv2 Host Compatibility Mode. Confirm its messages on that interface are IGMPv2, never IGMPv3.

### RFC9776-COMPH-18

**In IGMPv1 Host Compatibility Mode, a host uses only the IGMPv1 protocol on that interface.**

> "When Host Compatibility Mode is IGMPv1,
> a host acts in IGMPv1 compatibility mode, using only the IGMPv1
> protocol on that interface." — §7.2.1, `rfc9776.txt:1847-1849`

- Strength: description. Class: end-to-end.
- Check idea: Put a host's interface in IGMPv1 Host Compatibility Mode. Confirm its messages on that interface are IGMPv1, never IGMPv2 or IGMPv3.

### RFC9776-COMPH-19

**An IGMPv1 router sends General Queries with the Max Resp Code field set to 0.**

> "An IGMPv1 router
> will send General Queries with the Max Resp Code set
> to 0." — §7.2.1, `rfc9776.txt:1851-1852`

- Strength: description. Class: wire.
- Check idea: Observe General Queries sent by an IGMPv1 router. Confirm the Max Resp Code field is 0.

### RFC9776-COMPH-20

**A host must interpret a Max Resp Code of 0 as a Max Response Time of 100 (10 seconds).**

> "This MUST be interpreted as a value of 100 (10 seconds)." — §7.2.1, `rfc9776.txt:1852-1853`

- Strength: must. Class: end-to-end.
- Check idea: Send a host a General Query with Max Resp Code 0. Confirm the host uses a Max Response Time of 100 (10 seconds).

### RFC9776-COMPH-21

**An IGMPv2 router sets Max Resp Code directly to the desired Max Response Time, without the IGMPv3 exponential encoding.**

> "An IGMPv2 router will send General Queries with the Max Resp Code set
> to the desired Max Response Time, i.e., the full range of this field
> is linear and the exponential algorithm described in Section 4.1.1 is
> not used." — §7.2.1, `rfc9776.txt:1854-1857`

- Strength: description. Class: wire.
- Check idea: Observe a General Query sent by an IGMPv2 router with a chosen Max Response Time. Confirm the Max Resp Code field holds that value directly, not the IGMPv3 exponential encoding.

### RFC9776-COMPH-22

**A host cancels all pending response and retransmission timers when it changes compatibility mode.**

> "Whenever a host changes its compatibility mode, it cancels all its
> pending response and retransmission timers." — §7.2.1, `rfc9776.txt:1859-1860`

- Strength: description. Class: internal.
- Check idea: Give a host a pending response timer, then trigger a compatibility-mode change. Confirm the pending timer is canceled.

### RFC9776-COMPH-23

**An SSM-aware host should log an error on receiving an IGMPv1 Query, an IGMPv2 General Query, or an IGMPv2 Group Specific Query for an SSM address.**

> "An SSM-aware host that receives an IGMPv1 Query, an IGMPv2 General
> Query, or an IGMPv2 Group Specific Query for a multicast address in
> the SSM address range SHOULD log an error." — §7.2.1, `rfc9776.txt:1862-1864`

- Strength: should. Class: internal.
- Check idea: Send an SSM-aware host an IGMPv1 Query for an SSM address. Confirm an error log entry results.

### RFC9776-COMPH-24

**An implementation should offer a configuration option to disable Host Compatibility Mode for SSM-only operation.**

> "It is RECOMMENDED that
> implementations provide a configuration option to disable use of the
> Host Compatibility Mode to allow networks to operate only in SSM
> mode." — §7.2.1, `rfc9776.txt:1864-1867`

- Strength: should. Class: internal.
- Check idea: Inspect an implementation's configuration options. Confirm one exists to disable Host Compatibility Mode for SSM-only operation.

### RFC9776-COMPH-25

**This SSM-only configuration option should be disabled by default.**

> "This configuration option SHOULD be disabled by default." — §7.2.1, `rfc9776.txt:1867`

- Strength: should. Class: internal.
- Check idea: Inspect a fresh implementation's default configuration. Confirm the SSM-only option is off by default.

### RFC9776-COMPH-26

**A host may let an IGMPv1 or IGMPv2 Membership Report suppress its own IGMPv3 Membership Record.**

> "A host MAY allow its IGMPv3
> Membership Record to be suppressed by either an IGMPv1 Membership
> Report or an IGMPv2 Membership Report." — §7.2.2, `rfc9776.txt:1872-1874`

- Strength: may. Class: end-to-end.
- Check idea: Have another host send an IGMPv2 Membership Report for a group just before an IGMPv3 host's own Membership Record for it. Confirm the IGMPv3 host is allowed to suppress its own record.

### RFC9776-COMPH-27

**An SSM-aware host must not let its IGMPv3 Membership Record be suppressed.**

> "SSM-aware hosts MUST NOT
> allow its IGMPv3 Membership Record to be suppressed." — §7.2.2, `rfc9776.txt:1874-1875`

- Strength: must not. Class: end-to-end.
- Check idea: Have another host send an older-version Membership Report just before an SSM-aware host's own IGMPv3 record. Confirm the SSM-aware host still sends its record.

## Router behavior with older versions

### RFC9776-COMPR-1

**The querier must use the lowest IGMP version present among the network's routers.**

> "If any older versions of IGMP are present on routers, the querier
> MUST use the lowest version of IGMP present on the network." — §7.3.1, `rfc9776.txt:1885-1886`

- Strength: must. Class: internal.
- Check idea: Put an IGMPv1 router and an IGMPv3 querier on the same network. Confirm the querier switches to IGMPv1.

### RFC9776-COMPR-2

**A router that wants IGMPv1 or IGMPv2 compatibility must have a configuration option for IGMPv1 or IGMPv2 mode.**

> "routers that desire to be
> compatible with IGMPv1 and IGMPv2 MUST have a configuration option to
> act in IGMPv1 or IGMPv2 compatibility modes." — §7.3.1, `rfc9776.txt:1887-1889`

- Strength: must. Class: internal.
- Check idea: Inspect a router's configuration options. Confirm one exists to force IGMPv1 or IGMPv2 compatibility mode.

### RFC9776-COMPR-3

**In IGMPv1 mode, a router must send Periodic Queries with Max Resp Code 0, truncated to 8 bytes at the Group Address field.**

> "When in IGMPv1
> mode, routers MUST send Periodic Queries with a Max Resp Code of 0
> and truncated at the Group Address field (i.e., 8 bytes long)" — §7.3.1, `rfc9776.txt:1889-1891`

- Strength: must. Class: wire.
- Check idea: Put a router into IGMPv1 mode. Observe its Periodic Queries and confirm Max Resp Code is 0 and the message ends at the Group Address field.

### RFC9776-COMPR-4

**In IGMPv1 mode, a router must ignore Leave Group messages.**

> "and
> MUST ignore Leave Group messages." — §7.3.1, `rfc9776.txt:1891-1892`

- Strength: must. Class: end-to-end.
- Check idea: Send a router in IGMPv1 mode a Leave Group message. Confirm it takes no action on it.

### RFC9776-COMPR-5

**In IGMPv1 mode, a router should warn on receiving an IGMPv2 or IGMPv3 query, with the warnings rate-limited.**

> "They SHOULD also warn about
> receiving an IGMPv2 or IGMPv3 query, although such warnings MUST
> be rate-limited." — §7.3.1, `rfc9776.txt:1892-1894`

- Strength: should (the warning); must (the rate limit). Class: internal.
- Check idea: Send a router in IGMPv1 mode repeated IGMPv2 or IGMPv3 queries. Confirm warnings appear and their rate is limited.

### RFC9776-COMPR-6

**In IGMPv2 mode, a router must send Periodic Queries truncated to 8 bytes at the Group Address field.**

> "When in IGMPv2 mode, routers MUST send Periodic
> Queries truncated at the Group Address field (i.e., 8 bytes long)" — §7.3.1, `rfc9776.txt:1894-1895`

- Strength: must. Class: wire.
- Check idea: Put a router into IGMPv2 mode. Observe its Periodic Queries and confirm the message ends at the Group Address field.

### RFC9776-COMPR-7

**In IGMPv2 mode, a router should warn on receiving an IGMPv3 query, with the warnings rate-limited.**

> "and SHOULD also warn about receiving an IGMPv3 query (such
> warnings MUST be rate-limited)." — §7.3.1, `rfc9776.txt:1895-1897`

- Strength: should (the warning); must (the rate limit). Class: internal.
- Check idea: Send a router in IGMPv2 mode repeated IGMPv3 queries. Confirm warnings appear and their rate is limited.

### RFC9776-COMPR-8

**In IGMPv2 mode, a router must fill Max Resp Code with the literal Max Response Time, not the IGMPv3 exponential encoding.**

> "They also MUST fill in the Max
> Response Time in the Max Resp Code field, i.e., the exponential
> algorithm described in Section 4.1.1 is not used." — §7.3.1, `rfc9776.txt:1897-1899`

- Strength: must. Class: wire.
- Check idea: Put a router into IGMPv2 mode with a chosen Max Response Time. Confirm the Max Resp Code field carries that value directly.

### RFC9776-COMPR-9

**A router not configured for IGMPv1 or IGMPv2 should log a warning on receiving an IGMPv1 Query or an IGMPv2 General Query.**

> "If a router is not explicitly configured to use IGMPv1 or IGMPv2
> and receives an IGMPv1 Query or IGMPv2 General Query, it SHOULD
> log a warning." — §7.3.1, `rfc9776.txt:1901-1903`

- Strength: should. Class: internal.
- Check idea: Send a router with no IGMPv1/IGMPv2 configuration an IGMPv1 Query. Confirm a warning is logged.

### RFC9776-COMPR-10

**These warnings about an unexpected older-version query must be rate-limited.**

> "These warnings MUST be rate-limited." — §7.3.1, `rfc9776.txt:1903`

- Strength: must. Class: internal.
- Check idea: Send a router repeated older-version queries it is not configured for. Confirm the resulting warnings are rate-limited.

### RFC9776-COMPR-11

**An implementation should offer a router configuration option to disable compatibility mode for SSM-only operation.**

> "It is RECOMMENDED that implementations provide a configuration
> option to disable use of compatibility mode to allow networks to
> operate only in SSM mode." — §7.3.1, `rfc9776.txt:1905-1907`

- Strength: should. Class: internal.
- Check idea: Inspect a router's configuration options. Confirm one exists to disable compatibility mode for SSM-only operation.

### RFC9776-COMPR-12

**This SSM-only configuration option should be disabled by default.**

> "This configuration option SHOULD be disabled by default." — §7.3.1, `rfc9776.txt:1907-1908`

- Strength: should. Class: internal.
- Check idea: Inspect a fresh router's default configuration. Confirm the SSM-only option is off by default.

### RFC9776-COMPR-13

**An IGMPv3 router must be able to operate in IGMPv1 and IGMPv2 compatibility modes for older hosts.**

> "IGMPv3 routers
> MUST operate in v1 and v2
> compatibility modes." — §7.3.2, `rfc9776.txt:1914-1915`

- Strength: must. Class: internal.
- Check idea: Put an IGMPv3 router on a network with an IGMPv1 host and, separately, an IGMPv2 host. Confirm the router operates in the matching compatibility mode for each.

### RFC9776-COMPR-14

**A router keeps a per-Group-Record Group Compatibility Mode, one of IGMPv1, IGMPv2, or IGMPv3, driven by received Membership Report versions and the Older-Version-Host-Present Timer.**

> "IGMPv3 routers keep a compatibility mode per Group Record.  A group's
> compatibility mode is determined from the Group Compatibility Mode
> variable, which can be in one of three states: IGMPv1, IGMPv2, or
> IGMPv3.  This variable is kept per Group Record and is dependent on
> the version of Membership Reports received for that group as well as
> the Older-Version-Host-Present Timer for the group." — §7.3.2, `rfc9776.txt:1915-1921`

- Strength: description. Class: internal.
- Check idea: Inspect a router's Group Compatibility Mode variable for one Group Record. Confirm its value is always one of IGMPv1, IGMPv2, or IGMPv3.

### RFC9776-COMPR-15

**A router keeps an IGMPv1-Host-Present Timer and an IGMPv2-Host-Present Timer per Group Record.**

> "routers keep
> an IGMPv1-Host-Present Timer and an IGMPv2-Host-Present Timer per
> Group Record." — §7.3.2, `rfc9776.txt:1923-1925`

- Strength: description. Class: internal.
- Check idea: Inspect a router's Group Record state. Confirm both an IGMPv1-Host-Present Timer and an IGMPv2-Host-Present Timer exist for it.

### RFC9776-COMPR-16

**Receiving an IGMPv1 Membership Report sets the group's IGMPv1-Host-Present Timer to [Older Version Host Present Interval].**

> "The IGMPv1-Host-Present Timer is set to [Older Version
> Host Present Interval] seconds whenever an IGMPv1 Membership Report
> is received." — §7.3.2, `rfc9776.txt:1925-1927`

- Strength: description. Class: internal.
- Check idea: Send a router an IGMPv1 Membership Report for a group. Confirm the group's IGMPv1-Host-Present Timer is set to [Older Version Host Present Interval].

### RFC9776-COMPR-17

**Receiving an IGMPv2 Membership Report sets the group's IGMPv2-Host-Present Timer to [Older Version Host Present Interval].**

> "The IGMPv2-Host-Present Timer is set to [Older Version Host Present
> Interval] seconds whenever an IGMPv2 Membership Report
> is received." — §7.3.2, `rfc9776.txt:1927-1929`

- Strength: description. Class: internal.
- Check idea: Send a router an IGMPv2 Membership Report for a group. Confirm the group's IGMPv2-Host-Present Timer is set to [Older Version Host Present Interval].

### RFC9776-COMPR-18

**A group's Compatibility Mode changes on an older-version report than its current mode, or on certain timer events.**

> "The Group Compatibility Mode of a Group Record changes whenever an
> older version report (than the current compatibility mode) is
> received or when certain timer conditions occur." — §7.3.2, `rfc9776.txt:1931-1933`

- Strength: description. Class: internal.
- Check idea: Put a Group Record in IGMPv3 Group Compatibility Mode, then send an IGMPv2 Membership Report for it. Confirm the Group Compatibility Mode changes.

### RFC9776-COMPR-19

**When the IGMPv1-Host-Present Timer expires, a router switches the group to IGMPv2 mode if the IGMPv2 timer is running, else to IGMPv3 mode.**

> "When the IGMPv1-
> Host-Present Timer expires, a router switches to Group Compatibility
> Mode of IGMPv2 if it has a running IGMPv2 Host Present timer.  If it
> does not have a running IGMPv2 Host Present timer, then it switches
> to Group Compatibility Mode of IGMPv3." — §7.3.2, `rfc9776.txt:1933-1937`

- Strength: description. Class: internal.
- Check idea: Let a Group Record's IGMPv1-Host-Present Timer expire, once with the IGMPv2 timer still running and once without. Confirm it switches to IGMPv2 mode in the first case and IGMPv3 mode in the second.

### RFC9776-COMPR-20

**When the IGMPv2-Host-Present Timer expires while the IGMPv1 timer is not running, a router switches the group to IGMPv3 mode.**

> "When the IGMPv2-Host-Present
> Timer expires and the IGMPv1-Host-Present Timer is not running, a
> router switches to Group Compatibility Mode of IGMPv3." — §7.3.2, `rfc9776.txt:1937-1939`

- Strength: description. Class: internal.
- Check idea: Let a Group Record's IGMPv2-Host-Present Timer expire while the IGMPv1-Host-Present Timer is not running. Confirm the Group Compatibility Mode becomes IGMPv3.

### RFC9776-COMPR-21

**After a group switches back to IGMPv3 mode, blocking of sources marked to be blocked is delayed until [Group Membership Interval] after regaining source-specific state.**

> "Source-specific
> information will be learned during the next General Query, but
> sources that should be blocked will not be blocked until [Group
> Membership Interval] after that." — §7.3.2, `rfc9776.txt:1941-1944`

- Strength: should (lower case). Class: end-to-end.
- Check idea: Let a group switch back to IGMPv3 mode and regain source-specific state at the next General Query. Confirm sources due to be blocked are not blocked until [Group Membership Interval] later.

### RFC9776-COMPR-22

**Group Compatibility Mode is IGMPv3, the default, when neither the IGMPv1 nor the IGMPv2 Host Present timer is running.**

> "| Group Compatibility Mode | Timer State                         |" — §7.3.2, `rfc9776.txt:1952`, and
> "| IGMPv3 (default)         | IGMPv2 Host Present not running and |
> |                          | IGMPv1 Host Present not running     |" — §7.3.2, `rfc9776.txt:1954-1955`

- Strength: description. Class: internal.
- Check idea: Let both the IGMPv1-Host-Present and IGMPv2-Host-Present timers of a Group Record expire. Confirm the Group Compatibility Mode is IGMPv3.

### RFC9776-COMPR-23

**Group Compatibility Mode is IGMPv2 when the IGMPv2 Host Present timer runs and the IGMPv1 one does not.**

> "| IGMPv2                   | IGMPv2 Host Present running and     |
> |                          | IGMPv1 Host Present not running     |" — §7.3.2, `rfc9776.txt:1957-1958`

- Strength: description. Class: internal.
- Check idea: Let a Group Record's IGMPv2-Host-Present timer run while the IGMPv1 one is not running. Confirm the Group Compatibility Mode is IGMPv2.

### RFC9776-COMPR-24

**Group Compatibility Mode is IGMPv1 whenever the IGMPv1 Host Present timer is running.**

> "| IGMPv1                   | IGMPv1 Host Present running         |" — §7.3.2, `rfc9776.txt:1960`

- Strength: description. Class: internal.
- Check idea: Let a Group Record's IGMPv1-Host-Present timer run. Confirm the Group Compatibility Mode is IGMPv1, whatever the IGMPv2 timer state.

### RFC9776-COMPR-25

**A router should switch a group's Compatibility Mode right away once a received report updates its Host Present timers.**

> "If a router receives a report that causes its older Host Present
> timers to be updated and correspondingly its compatibility mode, it
> SHOULD switch compatibility modes immediately." — §7.3.2, `rfc9776.txt:1965-1967`

- Strength: should. Class: internal.
- Check idea: Send a router a report that updates a Group Record's Host Present timer and changes its compatibility mode. Measure how soon the router's behavior reflects the new mode.

### RFC9776-COMPR-26

**In IGMPv3 Group Compatibility Mode, a router uses the IGMPv3 protocol for that group.**

> "When Group Compatibility Mode is IGMPv3, a router acts using the
> IGMPv3 protocol for that group." — §7.3.2, `rfc9776.txt:1969-1970`

- Strength: description. Class: internal.
- Check idea: Put a Group Record in IGMPv3 Group Compatibility Mode. Confirm the router's handling of reports for that group follows the IGMPv3 rules.

### RFC9776-COMPR-27

**In IGMPv2 Group Compatibility Mode, a router treats an IGMPv2 Report as IS_EX({}).**

> "| IGMPv2 Message | IGMPv3 Equivalent |" — §7.3.2, `rfc9776.txt:1977`, and
> "| Report         | IS_EX( {} )       |" — §7.3.2, `rfc9776.txt:1979`

- Strength: description. Class: internal.
- Check idea: Put a Group Record in IGMPv2 Group Compatibility Mode, then send an IGMPv2 Report for it. Confirm the router's state update matches an IS_EX({}) record.

### RFC9776-COMPR-28

**In IGMPv2 Group Compatibility Mode, a router treats an IGMPv2 Leave as TO_IN({}).**

> "| Leave          | TO_IN( {} )       |" — §7.3.2, `rfc9776.txt:1981`

- Strength: description. Class: internal.
- Check idea: Put a Group Record in IGMPv2 Group Compatibility Mode, then send an IGMPv2 Leave for it. Confirm the router's state update matches a TO_IN({}) record.

### RFC9776-COMPR-29

**In IGMPv2 Group Compatibility Mode, a router ignores IGMPv3 BLOCK messages.**

> "IGMPv3 BLOCK messages are ignored" — §7.3.2, `rfc9776.txt:1987`

- Strength: description. Class: end-to-end.
- Check idea: Put a Group Record in IGMPv2 Group Compatibility Mode, then send an IGMPv3 BLOCK message for it. Confirm the router takes no action on it.

### RFC9776-COMPR-30

**In IGMPv2 Group Compatibility Mode, a router ignores the source list of a TO_EX() message, treating it as TO_EX({}).**

> "as are source-lists in TO_EX()
> messages (i.e., any TO_EX() message is treated as TO_EX( {} ))." — §7.3.2, `rfc9776.txt:1987-1988`

- Strength: description. Class: end-to-end.
- Check idea: Put a Group Record in IGMPv2 Group Compatibility Mode, then send a TO_EX(source-list) message for it. Confirm the router acts as though it were TO_EX({}), ignoring the source list.

### RFC9776-COMPR-31

**In IGMPv1 Group Compatibility Mode, a router treats a v1 Report as IS_EX({}).**

> "| IGMPv2 Message | IGMPv3 Equivalent |" — §7.3.2, `rfc9776.txt:1995`, and
> "| v1 Report      | IS_EX( {} )       |" — §7.3.2, `rfc9776.txt:1997`

- Strength: description. Class: internal.
- Check idea: Put a Group Record in IGMPv1 Group Compatibility Mode, then send a v1 Report for it. Confirm the router's state update matches an IS_EX({}) record.

### RFC9776-COMPR-32

**In IGMPv1 Group Compatibility Mode, a router treats a v2 Report as IS_EX({}).**

> "| v2 Report      | IS_EX( {} )       |" — §7.3.2, `rfc9776.txt:1999`

- Strength: description. Class: internal.
- Check idea: Put a Group Record in IGMPv1 Group Compatibility Mode, then send a v2 Report for it. Confirm the router's state update matches an IS_EX({}) record.

### RFC9776-COMPR-33

**In IGMPv1 Group Compatibility Mode, a router also ignores IGMPv2 Leave messages and IGMPv3 TO_IN() messages, besides what it already ignores in IGMPv2 mode.**

> "In addition to ignoring IGMPv3 BLOCK messages and source-lists in
> TO_EX() messages as in IGMPv2 Group Compatibility Mode, IGMPv2 Leave
> messages and IGMPv3 TO_IN() messages are also ignored." — §7.3.2, `rfc9776.txt:2005-2007`

- Strength: description. Class: end-to-end.
- Check idea: Put a Group Record in IGMPv1 Group Compatibility Mode, then send an IGMPv2 Leave and separately an IGMPv3 TO_IN() message for it. Confirm the router takes no action on either.

## Timers, counters and default values

### RFC9776-TIMER-1

**If a link uses non-default timer or counter settings, every system on that link MUST use the
same settings.**

> "If non-default settings" and "are used, they MUST be consistent among all systems on a
> single link." — §8, `rfc9776.txt:2011`, `rfc9776.txt:2012`

- Strength: must. Class: internal.
- Check idea: configure two systems on the same link with matching non-default Query Interval
  values and check both operate consistently; a mismatch is a defect this rule forbids.

### RFC9776-TIMER-2

**The Robustness Variable tunes IGMP for expected packet loss; IGMP tolerates one less than the
Robustness Variable packet losses. Default: 2.**

> "The Robustness Variable allows tuning for the expected packet loss on" and "a network." and
> "IGMP is robust to (Robustness Variable -" and "1) packet losses." and "Default: 2." — §8.1,
> `rfc9776.txt:2018`, `rfc9776.txt:2019`, `rfc9776.txt:2020`, `rfc9776.txt:2021`,
> `rfc9776.txt:2022`

- Strength: description. Class: internal.
- Check idea: read a system's configured Robustness Variable when not otherwise set and check
  it equals 2.

### RFC9776-TIMER-3

**The Robustness Variable MUST NOT be zero and SHOULD NOT be one.**

> "The Robustness Variable MUST NOT be zero and" and "SHOULD NOT be one." — §8.1,
> `rfc9776.txt:2021`, `rfc9776.txt:2022`

- Strength: must not (zero), should not (one). Class: internal.
- Check idea: attempt to configure a system's Robustness Variable to zero and check the system
  rejects it; attempt to configure it to one and check the system flags it as discouraged.

### RFC9776-TIMER-4

**The Query Interval is the interval between General Queries sent by the Querier. Default: 125
seconds.**

> "The Query Interval is the interval between General Queries sent by" and "the Querier.
> Default: 125 seconds." — §8.2, `rfc9776.txt:2026`, `rfc9776.txt:2027`

- Strength: description. Class: internal.
- Check idea: read a querier's Query Interval when not otherwise configured and check it
  equals 125 seconds, and that the querier's General Queries repeat at that interval.

### RFC9776-TIMER-5

**The Query Response Interval uses the Max Response Time to calculate the Max Resp Code
inserted into periodic General Queries. Default: 100, that is 10 seconds.**

> "The Query Response Interval uses the Max Response Time to calculate" and "the Max Resp Code
> that is inserted into the periodic General Queries." and "Default: 100 (10 seconds)." —
> §8.3, `rfc9776.txt:2035`, `rfc9776.txt:2036`, `rfc9776.txt:2037`

- Strength: description. Class: internal.
- Check idea: read a querier's Query Response Interval when not otherwise configured and check
  it equals 100, giving a Max Resp Code for 10 seconds in its periodic General Queries.

### RFC9776-TIMER-6

**The number of seconds of the Query Response Interval must be less than the Query Interval.**

> "The number of seconds represented by the [Query" and "Response Interval] must be less than
> the [Query Interval]." — §8.3, `rfc9776.txt:2042`, `rfc9776.txt:2042-2043`

- Strength: must (lower case). Class: internal.
- Check idea: configure a querier's Query Response Interval and Query Interval and check the
  configured Query Response Interval, in seconds, is smaller than the Query Interval.

### RFC9776-TIMER-7

**The Group Membership Interval is the time a multicast router waits before it decides there
are no more members of a group or a source on a network.**

> "The Group Membership Interval is the amount of time that must pass" and "before a multicast
> router decides there are no more members of a" and "group or a particular source on a
> network." — §8.4, `rfc9776.txt:2047`, `rfc9776.txt:2048`, `rfc9776.txt:2049`

- Strength: must (lower case). Class: internal.
- Check idea: stop all reports for a group on a network and check the router waits the Group
  Membership Interval before it decides there are no more members.

### RFC9776-TIMER-8

**The Group Membership Interval MUST be the Robustness Variable times the Query Interval, plus
twice the Query Response Interval.**

> "This value MUST be ([Robustness Variable] times [Query Interval])" and "plus (2 * [Query
> Response Interval])." — §8.4, `rfc9776.txt:2051`, `rfc9776.txt:2052`

- Strength: must. Class: internal.
- Check idea: configure a router's Robustness Variable, Query Interval and Query Response
  Interval and check the resulting Group Membership Interval matches the formula.

### RFC9776-TIMER-9

**The Other Querier Present Interval is the time a multicast router waits before it decides
there is no longer another router that should be the querier.**

> "The Other Querier Present Interval is the length of time that must" and "pass before a
> multicast router decides that there is no longer" and "another multicast router that should
> be the querier." — §8.5, `rfc9776.txt:2056`, `rfc9776.txt:2057`, `rfc9776.txt:2058`

- Strength: must, should (lower case). Class: internal.
- Check idea: stop Queries from the current querier on a network and check a non-querier router
  waits the Other Querier Present Interval before it takes over.

### RFC9776-TIMER-10

**The Other Querier Present Interval MUST be the Robustness Variable times the Query Interval,
plus 0.5 times the Query Response Interval.**

> "This value MUST" and "be ([Robustness Variable] times [Query Interval]) plus (0.5 times" and
> "[Query Response Interval])." — §8.5, `rfc9776.txt:2058`, `rfc9776.txt:2059`,
> `rfc9776.txt:2060`

- Strength: must. Class: internal.
- Check idea: configure a router's Robustness Variable, Query Interval and Query Response
  Interval and check the resulting Other Querier Present Interval matches the formula.

### RFC9776-TIMER-11

**The Startup Query Interval is the interval between General Queries a Querier sends on
startup. Default: 1/4 times the Query Interval.**

> "The Startup Query Interval is the interval between General Queries" and "sent by a Querier
> on startup.  Default: 1/4 times [Query Interval]." — §8.6, `rfc9776.txt:2064`,
> `rfc9776.txt:2065`

- Strength: description. Class: internal.
- Check idea: start a querier and check the interval between its first General Queries equals
  one quarter of its configured Query Interval.

### RFC9776-TIMER-12

**The Startup Query Count is the number of Queries a Querier sends out on startup, separated
by the Startup Query Interval. Default: the Robustness Variable.**

> "The Startup Query Count is the number of Queries sent out on startup," and "separated by the
> Startup Query Interval.  Default: [Robustness" and "Variable]." — §8.7, `rfc9776.txt:2069`,
> `rfc9776.txt:2070`, `rfc9776.txt:2071`

- Strength: description. Class: internal.
- Check idea: start a querier with the default Robustness Variable and count the General
  Queries it sends on startup before it falls back to the Query Interval.

### RFC9776-TIMER-13

**The Last Member Query Interval is the Max Response Time used for the Max Resp Code of Group
Specific Queries sent after a Leave Group message, and of Group-and-Source Specific Queries.
Default: 10, that is 1 second.**

> "The Last Member Query Interval (LMQI) is the Max Response Time used" and "to calculate the
> Max Resp Code that is inserted into Group Specific" and "Queries sent in response to Leave
> Group messages." and "It is also the Max" and "Response Time used in calculating the Max
> Resp Code for Group-and-" and "Source Specific Query Messages." and "Default: 10 (1
> second)." — §8.8, `rfc9776.txt:2075`, `rfc9776.txt:2076`, `rfc9776.txt:2077`,
> `rfc9776.txt:2077`, `rfc9776.txt:2078`, `rfc9776.txt:2079`, `rfc9776.txt:2079`

- Strength: description. Class: internal.
- Check idea: trigger a router's response to a Leave Group message and check its Group Specific
  Query carries a Max Resp Code for 1 second.

### RFC9776-TIMER-14

**For a Last Member Query Interval greater than 12.8 seconds, only specific values are
representable, matching the sequential Max Resp Code values.**

> "Note that for values of LMQI greater than 12.8 seconds, a limited set" and "of values can be
> represented, corresponding to sequential values of" and "Max Resp Code." — §8.8,
> `rfc9776.txt:2081`, `rfc9776.txt:2082`, `rfc9776.txt:2083`

- Strength: description. Class: encoding.
- Check idea: configure a Last Member Query Interval above 12.8 seconds and check the resulting
  Max Resp Code matches one of the values the exponential encoding can represent.

### RFC9776-TIMER-15

**When a system converts a configured time to a Max Resp Code, it uses the exact value if
possible, or the next lower representable value otherwise.**

> "When converting a configured time to a Max Resp Code" and "value, it is recommended to use
> the exact value, if possible, or the" and "next lower value if the requested value is not
> exactly representable." — §8.8, `rfc9776.txt:2083`, `rfc9776.txt:2084`, `rfc9776.txt:2085`

- Strength: description. Class: encoding.
- Check idea: configure a Last Member Query Interval that is not exactly representable and
  check the resulting Max Resp Code decodes to the next lower representable value.

### RFC9776-TIMER-16

**The Last Member Query Count is the number of Group Specific Queries, and also the number of
Group-and-Source Specific Queries, a router sends before it assumes there are no more
listeners. Default: the Robustness Variable.**

> "The Last Member Query Count is the number of Group Specific Queries" and "sent before the
> router assumes there are no local members." and "The Last" and "Member Query Count is also
> the number of Group-and-Source Specific" and "Queries sent before the router assumes there
> are no listeners for a" and "particular source." and "Default: [Robustness Variable]." —
> §8.9, `rfc9776.txt:2093`, `rfc9776.txt:2094`, `rfc9776.txt:2094`, `rfc9776.txt:2095`,
> `rfc9776.txt:2096`, `rfc9776.txt:2097`, `rfc9776.txt:2097`

- Strength: description. Class: internal.
- Check idea: trigger a router's last-member query sequence for a group and count the Group
  Specific Queries it sends before it decides there are no more members; check the count equals
  the Robustness Variable.

### RFC9776-TIMER-17

**The Last Member Query Time is the Last Member Query Interval times the Last Member Query
Count; it is not itself a tunable value, only its components are.**

> "The Last Member Query Time is the time value represented by [Last" and "Member Query
> Interval] times [Last Member Query Count]." and "It is not a" and "tunable value, but it may
> be tuned by changing its components." — §8.10, `rfc9776.txt:2101`, `rfc9776.txt:2102`,
> `rfc9776.txt:2102`, `rfc9776.txt:2103`

- Strength: may (lower case). Class: internal.
- Check idea: configure a router's Last Member Query Interval and Last Member Query Count and
  check the total time of its last-member query sequence equals their product.

### RFC9776-TIMER-18

**The Unsolicited Report Interval is the time between repetitions of a host's initial report of
membership in a group. Default: 1 second.**

> "The Unsolicited Report Interval is the time between repetitions of a" and "host's initial
> report of membership in a group.  Default: 1 second." — §8.11, `rfc9776.txt:2107`,
> `rfc9776.txt:2108`

- Strength: description. Class: internal.
- Check idea: have a host join a group and check its first unsolicited Report repeats after 1
  second.

### RFC9776-TIMER-19

**The Older Version Querier Present Interval is the timeout for a host to transition back to
IGMPv3 mode after it receives an older version query.**

> "The Older Version Querier Present Interval is the timeout for" and "transitioning a host
> back to IGMPv3 mode once an older version query" and "is received." — §8.12,
> `rfc9776.txt:2112`, `rfc9776.txt:2113`, `rfc9776.txt:2114`

- Strength: description. Class: internal.
- Check idea: stop older-version queries reaching a host and check the host returns to IGMPv3
  mode once the Older Version Querier Present Interval has passed since the last one.

### RFC9776-TIMER-20

**When a host receives an older version query, it sets its Older-Version-Querier-Present Timer
to the Older Version Querier Present Interval.**

> "When an older version query is received, hosts set" and "their Older-Version-Querier-Present
> Timer to [Older Version Querier" and "Present Interval]." — §8.12, `rfc9776.txt:2114`,
> `rfc9776.txt:2115`, `rfc9776.txt:2116`

- Strength: description. Class: internal.
- Check idea: send a host an older version query and check its Older-Version-Querier-Present
  Timer is set to the Older Version Querier Present Interval.

### RFC9776-TIMER-21

**A host SHOULD use the default values to calculate the Older Version Querier Present
Interval, because it does not know the querying router's configured values.**

> "It is RECOMMENDED to use the default values for calculating the" and "interval value as
> hosts do not know the values configured on the" and "querying routers." — §8.12,
> `rfc9776.txt:2118`, `rfc9776.txt:2119`, `rfc9776.txt:2120`

- Strength: should. Class: internal.
- Check idea: give a host a non-default Robustness Variable and Query Interval, and check
  whether it uses the default values, rather than its own, to calculate the Older Version
  Querier Present Interval.

### RFC9776-TIMER-22

**The Older Version Querier Present Interval SHOULD be the Robustness Variable times the Query
Interval, plus 10 times the Max Response Time of the last received Query.**

> "This value SHOULD be [Robustness Variable] times" and "[Query Interval] plus (10 times the
> Max Response Time in the last" and "received Query Message)." — §8.12, `rfc9776.txt:2120`,
> `rfc9776.txt:2121`, `rfc9776.txt:2122`

- Strength: should. Class: internal.
- Check idea: send a host a Query with a known Max Response Time and check the host's resulting
  Older Version Querier Present Interval matches the formula.

### RFC9776-TIMER-23

**The Older Host Present Interval is the timeout for a group to transition back to IGMPv3 mode
after an older version report is sent for it.**

> "The Older Host Present Interval is the timeout for transitioning a" and "group back to
> IGMPv3 mode once an older version report is sent for" and "that group." — §8.13,
> `rfc9776.txt:2126`, `rfc9776.txt:2127`, `rfc9776.txt:2128`

- Strength: description. Class: internal.
- Check idea: stop older-version reports for a group reaching a router and check the router
  returns the group to IGMPv3 mode once the Older Host Present Interval has passed since the
  last one.

### RFC9776-TIMER-24

**When a router receives an older version report, it sets its Older-Host-Present Timer to the
Older Host Present Interval.**

> "When an older version report is received, routers set" and "their Older-Host-Present Timer
> to [Older Host Present Interval]." — §8.13, `rfc9776.txt:2128`, `rfc9776.txt:2129`

- Strength: description. Class: internal.
- Check idea: send a router an older version report for a group and check its
  Older-Host-Present Timer for that group is set to the Older Host Present Interval.

### RFC9776-TIMER-25

**The Older Host Present Interval MUST be the Robustness Variable times the Query Interval,
plus the Query Response Interval.**

> "This value MUST be ([Robustness Variable] times [Query Interval])" and "plus [Query Response
> Interval]." — §8.13, `rfc9776.txt:2131`, `rfc9776.txt:2132`

- Strength: must. Class: internal.
- Check idea: configure a router's Robustness Variable, Query Interval and Query Response
  Interval and check the resulting Older Host Present Interval matches the formula.

### RFC9776-TIMER-26

**The Query Interval MUST be equal to or longer than the Max Response Time inserted in General
Query messages.**

> "The Query Interval MUST be equal to" and "or longer than the Max Response Time inserted in
> General Query" and "Messages." — §8.14.2, `rfc9776.txt:2158`, `rfc9776.txt:2159`,
> `rfc9776.txt:2160`

- Strength: must. Class: internal.
- Check idea: configure a querier's Query Interval and the Max Response Time of its General
  Queries and check the Query Interval is never shorter than that Max Response Time.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§4 to §8, and every field of every message. What the catalog leaves out:

- **The sections outside the in-scope set.** §1 to §3 (introduction, the service interface,
  the reception state of a system), §9 to §11 (security, IANA, references) and the appendices.
  See [`standards.md`](../../protocol/igmp/standards.md#target-level) for the level at which
  each one enters.

Within the in-scope sections, these parts have no entry, with the reason:

In §4 and §8:

- `444:450-451` — "IGMP message types are registered per [BCP57]." IANA registry
  administration, not a testable node behavior.
- `444:483-484` — "Other message types may be used by newer versions or extensions of IGMP, by
  multicast routing protocols, or for other uses." Informative scope note about other
  protocols and future extensions, not a requirement on this protocol's implementation.
- `444:486-489` — "In this document, unless otherwise qualified, the capitalized words 'Query'
  and 'Report' refer to..." Document notation, not a checkable statement.
- `490:496-515` — Figure 1, the Query message diagram. Its fields are individually catalogued
  under RFC9776-QRY-2 through RFC9776-QRY-22 (§4.1.1-4.1.9); the figure itself is not
  separately catalogued to avoid duplicating those entries.
- `490:538-542` — Paragraph on leave-latency and burstiness tuning rationale for Max Response
  Time. Informative explanation, no independent checkable requirement.
- `490:616-621` — The Ethernet/1500-octet MTU example ("For example, on an Ethernet..."
  through "366 (1464/4)."). Illustrative arithmetic, not a general normative rule; the general
  "limited by the MTU" clause is RFC9776-QRY-21.
- `672:679-735` — Figures 4 and 5, the Report and Group Record diagrams. Fields individually
  catalogued under RFC9776-REP-2 through RFC9776-REP-12 (§4.2.1-4.2.10).
- `672:766-768` — §4.2.6 Record Type: "See Section 4.2.13, below." Pure cross-reference; the
  actual Record Type values are catalogued under §4.2.13 (RFC9776-REP-18 to RFC9776-REP-30).
- `672:799-801` — "The semantics and internal encoding of the Auxiliary Data field are to be
  defined by any future version or extension of IGMP..." Forward-looking note about future
  protocol extensions, not a requirement on this document.
- `672:815-816` — "There are a number of different types of Group Records that may be included
  in a Report message:" Introductory sentence with no independent checkable content.
- `672:890-891` — "We use the term 'State-Change Record' to refer to either a Filter-Mode-Change
  Record or a Source-List-Change Record." Pure terminology, not a checkable statement.
- `672:899-900` — "Note that the 0.0.0.0 source address may simultaneously be used by multiple
  systems on a LAN." Informative note explaining a consequence, not an independent requirement.
- `672:915-936` — §4.2.16 Notation for Group Records (IS_IN, IS_EX, TO_IN, TO_EX, ALLOW,
  BLOCK). Pure symbolic notation used elsewhere in the document (outside this range) to
  describe protocol state tables; not itself a field, message, or behavior statement.
- `672:952-954` — "though the choice of which sources to report is arbitrary, it is preferable
  to report the same set of sources in each subsequent report..." Explicitly non-mandatory
  ("arbitrary", "preferable") guidance, not a testable requirement.
- `2009:2011` — "Most timers and counters are configurable." General scene-setting, no specific
  timer named, not independently testable.
- `2009:2013-2014` — "Note that parentheses are used to group expressions to make the algebra
  clear." Typographic note about the document's own notation.
- `2009:2019-2020` — "If a network is expected to be lossy, the Robustness Variable may be
  increased." Administrator tuning motivation (lowercase "may"), informative, not an
  independent requirement.
- `2009:2029-2031` — "By varying the Query Interval, an administrator may tune the number of
  IGMP messages..." Informative rationale for administrators.
- `2009:2039-2041` — "By varying the [Query Response Interval], an administrator may tune the
  burstiness..." Informative rationale for administrators.
- `2009:2087-2089` — "This value may be tuned to modify the leave latency... A reduced value
  results in reduced time to detect the loss..." Informative tuning rationale (lowercase
  "may").
- `2009:2136-2139` — §8.14 intro: "This section is meant to provide advice to network
  administrators..." Explicitly framed as advice, not a requirement.
- `2009:2141-2152` — §8.14.1 Robustness Variable. Explanatory/advisory restatement of §8.1 with
  an example and lowercase administrator tuning guidance ("should be increased"); no
  independent checkable requirement beyond RFC9776-TIMER-2/3.
- `2009:2156-2158` — §8.14.2 first two sentences: "The overall level of periodic IGMP traffic is
  inversely proportional to the Query Interval. A longer Query Interval results in a lower
  overall level of IGMP traffic." Informative rationale preceding the MUST sentence, which is
  catalogued as RFC9776-TIMER-26.
- `2009:2162-2192` — §8.14.3 Max Response Time, including Table 15. Entirely advisory guidance
  for optional dynamic tuning; it ends by stating "A router is not required to calculate these
  populations or tune the Max Response Time dynamically; these are simply guidelines,"
  confirming nothing here is a testable requirement.

In §5 to §7:

- §5 opening paragraphs, `rfc9776.txt:956-965` — informative framing: IGMP is asymmetric, and the aside that a router that is also a group member performs both parts (restated, and pointed to, elsewhere).
- `rfc9776.txt:971,974-977` — informative framing on which interfaces this section covers (MulticastRouterVersion = 3) and a pointer to §7.
- `rfc9776.txt:986-993` — structural list introducing "two types of events"; the one substantive clause in the parenthetical is cataloged as HOST-5.
- `rfc9776.txt:998-1001` — pointer to §8 for timer and counter defaults (out of range).
- `rfc9776.txt:1005-1008` — background on IPMulticastListen invocations, referencing §3.2 (out of range).
- `rfc9776.txt:1111-1114` — informal "Note:" clarifying the very first State-Change Report's initial merge state; the edge case itself is already covered by HOST-7 and HOST-27.
- `rfc9776.txt:1121-1124` — informative aside that a system may receive a variety of Queries.
- `rfc9776.txt:1143-1146` — framing sentence introducing the five numbered scheduling rules (the rules themselves are HQRY-4..9).
- `rfc9776.txt:1179-1183` — framing sentence introducing the three numbered transmission rules (the rules themselves are HQRY-10, 13, 14).
- §6 opening paragraphs, `rfc9776.txt:1236-1249` — informative purpose statement for IGMP and a restatement of the router-as-member aside.
- `rfc9776.txt:1276-1278` — informative connective sentence restating host-side reporting behavior (covered under HOST/HQRY).
- `rfc9776.txt:1280-1286` — framing about Filter-Mode-Change/Source-List-Change Records signaling a state change, leading into the rule cataloged as RQ-7.
- `rfc9776.txt:1307-1309` — pointer to §4.1.11 (out of range).
- `rfc9776.txt:1333-1341` — terminology/rationale for "Router Filter Mode"; restates RST-1 without a new checkable claim.
- `rfc9776.txt:1362-1369` — foreshadowing of the EXCLUDE-to-INCLUDE transition, fully covered in detail by RSW-1 and RSW-2.
- `rfc9776.txt:1376-1378` — pointer sentence ("Group timers are updated according to the types of Group Records received"), elaborated by the RREP Table 8/9 entries.
- `rfc9776.txt:1380-1384` — a second foreshadowing of the transition, again covered by RSW-1.
- `rfc9776.txt:1386-1389` — pure pointer to Table 6 and to §6.4 (table rows cataloged as RST-8..10).
- `rfc9776.txt:1444` — pointer to §6.3 (elaborated by the FWD area).
- `rfc9776.txt:1459-1463` — framing sentence introducing Table 7 (rows cataloged as FWD-3..8).
- `rfc9776.txt:1509-1510` — pointer to Table 8 (rows cataloged as RREP-8..11).
- `rfc9776.txt:1514-1527` — notation definitions for the sets A and B used to read Tables 8 and 9; definitional, not itself a checkable behavior.
- `rfc9776.txt:1539-1543` — legend explaining the table shorthand ("A=J", "Delete A", "Group Timer=J"); needed to read Tables 8/9 but not itself a new rule.
- `rfc9776.txt:1570-1574` — generic framing that routers must act on Filter-Mode-Change/Source-List-Change Records; elaborated fully by Table 9 (RREP-20..27).
- `rfc9776.txt:1599-1609` — framing sentence for Table 9 plus the Q(G)/Q(G,A) notation legend.
- `rfc9776.txt:1617-1620` — pointer stating that host reports may affect pending queries, deferring to §6.6.3 (covered there as RQRY-12..15).
- `rfc9776.txt:1661-1662` — one-sentence summary of the EXCLUDE-to-INCLUDE switch, restated in full by RSW-1.
- `rfc9776.txt:1677-1680` — a worked example ("For example, if a router's state...").
- `rfc9776.txt:1688-1691` — framing sentence for Table 10 (rows cataloged as RQRY-2, RQRY-3).
- `rfc9776.txt:1749-1750` — framing sentence ("The contents of these queries are calculated as follows"), leading into RQRY-14.
- `rfc9776.txt:1760-1764` — informal "Note:" about optionally suppressing a Group-and-Source Specific Query when it coincides with a Group Specific Query for the same group.
- §7 opening paragraph, `rfc9776.txt:1766-1773` — informative statement that IGMPv3 interoperates with older versions.
- `rfc9776.txt:1789` and `1877` — bare section headers (§7.2, §7.3) with no body text of their own.
- `rfc9776.txt:1881-1884` — lead-in to the §7.3.1 bullet list ("the following requirements apply").
- `rfc9776.txt:1911-1913` — background clause that IGMPv3 routers may share a network with older hosts, leading into COMPR-13.
- `rfc9776.txt:1946-1950` — framing sentence introducing Table 12 (rows cataloged as COMPR-22..24).
- `rfc9776.txt:1972-1975` and `1990-1993` — framing sentences introducing Tables 13 and 14 (rows cataloged as COMPR-27, 28, 31, 32).
