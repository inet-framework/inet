# RFC 4302 (IP Authentication Header) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC4302-*` · **Stands on:** [standards.md](../../protocol/ipsec/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 4302, IP Authentication Header, of
December 2005, a document of the in-scope set of IPsec. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc4302.txt`](../../../../../../standards/RFC/rfc4302.txt) —
  IP Authentication Header, December 2005. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc4302.txt>.

The architecture is in [`rfc4301/catalog.md`](../rfc4301/catalog.md), and ESP, the other
traffic protocol, in [`rfc4303/catalog.md`](../rfc4303/catalog.md). The in-scope sections are
§2 to §3.4 and §5.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/ipsec/standards.md). The features these statements build are in
[`features.md`](../../protocol/ipsec/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipsec/coverage.md`](../../model/ipsec/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc4302.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC4302-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 4302 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them (`must` for
  REQUIRED), and `description` for a normative sentence with no keyword. Where the text writes a
  keyword in lower case, the entry records the word and says so, for example `must (lower
  case)`. An entry with two keywords names what each one covers. A step of a numbered procedure
  that states an action is an entry of its own.
- **Class** — how a test can observe the statement: `wire` (fields of a header on a link, or
  the presence or absence of a packet), `end-to-end` (what an implementation accepts, delivers
  or discards, seen in what it sends or delivers afterwards), `error-signal` (an ICMP error
  report), `internal` (state or a database inside an implementation, an audit log), `encoding`
  (the exact bit layout, sizes and alignment).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC4302-FMT-1](#rfc4302-fmt-1) | The IP protocol header immediately before the AH header carries the AH protocol number. |
| [RFC4302-FMT-2](#rfc4302-fmt-2) | The AH header holds, in order, Next Header, Payload Length, Reserved, Security Parameters Index, Sequence Number, and Integrity Check Value fields. |
| [RFC4302-FMT-3](#rfc4302-fmt-3) | Every field defined for the AH format is always present and is covered by the ICV computation. |
| [RFC4302-FMT-4](#rfc4302-fmt-4) | Because AH carries no version number, IPsec peers must resolve any AH compatibility concern with a signaling or configuration mechanism, not with a version field. |
| [RFC4302-NH-1](#rfc4302-nh-1) | The Next Header field is 8 bits wide. |
| [RFC4302-NH-2](#rfc4302-nh-2) | The Next Header field carries the IANA IP Protocol Number of the payload that follows the AH header. |
| [RFC4302-LEN-1](#rfc4302-len-1) | The Payload Length field states the AH header's length in 32-bit words, minus two. |
| [RFC4302-LEN-2](#rfc4302-len-2) | For IPv6, the AH header's total length must be a multiple of 8 octets. |
| [RFC4302-RSV-1](#rfc4302-rsv-1) | The Reserved field is 16 bits wide and is held for future use. |
| [RFC4302-RSV-2](#rfc4302-rsv-2) | The sender sets the Reserved field to zero. |
| [RFC4302-RSV-3](#rfc4302-rsv-3) | The recipient ignores the value of the Reserved field. |
| [RFC4302-RSV-4](#rfc4302-rsv-4) | The receiver's ICV computation covers the Reserved field's transmitted value. |
| [RFC4302-SPI-1](#rfc4302-spi-1) | The SPI is an arbitrary 32-bit value. |
| [RFC4302-SPI-2](#rfc4302-spi-2) | The receiver uses the SPI to identify the SA an incoming packet is bound to. |
| [RFC4302-SPI-3](#rfc4302-spi-3) | For a unicast SA, the SPI alone, or the SPI together with the IPsec protocol type, identifies the SA. |
| [RFC4302-SPI-4](#rfc4302-spi-4) | For unicast SAs, the receiver generates the SPI value, and whether the SPI alone identifies the SA, or the SPI with the protocol value, is a local matter. |
| [RFC4302-SPI-5](#rfc4302-spi-5) | Every AH implementation must support the SPI-based mechanism for mapping inbound traffic to unicast SAs. |
| [RFC4302-SPI-6](#rfc4302-spi-6) | An IPsec implementation that supports multicast must support multicast SAs with the SAD search algorithm below; a unicast-only implementation need not. |
| [RFC4302-SPI-7](#rfc4302-spi-7) | A multicast-capable IPsec implementation must correctly de-multiplex inbound traffic even when a group SA and a unicast SA share the same SPI. |
| [RFC4302-SPI-8](#rfc4302-spi-8) | Each SAD entry must indicate whether its SA lookup uses the destination address alone, or the destination and source addresses, in addition to the SPI. |
| [RFC4302-SPI-9](#rfc4302-spi-9) | For multicast SAs, the SAD lookup does not use the protocol field. |
| [RFC4302-SPI-10](#rfc4302-spi-10) | For each inbound IPsec-protected packet, an implementation must search the SAD for the entry with the longest matching SA identifier. |
| [RFC4302-SPI-11](#rfc4302-spi-11) | When two or more SAD entries match on the SPI, the entry that also matches on the destination address, or on the destination and source addresses, is the longest match. |
| [RFC4302-SPI-12](#rfc4302-spi-12) | The first step of the inbound SAD search matches on SPI, destination address, and source address. |
| [RFC4302-SPI-13](#rfc4302-spi-13) | The second step of the inbound SAD search matches on SPI and destination address. |
| [RFC4302-SPI-14](#rfc4302-spi-14) | The third step of the inbound SAD search matches on the SPI alone, or on the SPI and protocol, depending on whether the receiver keeps one SPI space for AH and ESP. |
| [RFC4302-SPI-15](#rfc4302-spi-15) | When no SAD entry matches by the end of the third step, the receiver discards the packet and logs an auditable event. |
| [RFC4302-SPI-16](#rfc4302-spi-16) | An implementation may accelerate the SAD search by any method, but its externally visible behavior must be equivalent to searching in the three-step order. |
| [RFC4302-SPI-17](#rfc4302-spi-17) | The choice of whether inbound lookup requires address matching must be set by manual SA configuration or by SA-management-protocol negotiation. |
| [RFC4302-SPI-18](#rfc4302-spi-18) | A Source-Specific Multicast group typically uses an SA identifier composed of the SPI, the destination multicast address, and the source address. |
| [RFC4302-SPI-19](#rfc4302-spi-19) | An Any-Source Multicast group SA needs only an SPI and a destination multicast address as its identifier. |
| [RFC4302-SPI-20](#rfc4302-spi-20) | The SPI values 1 through 255 are reserved by IANA for future use. |
| [RFC4302-SPI-21](#rfc4302-spi-21) | The SPI value of zero must not be sent on the wire. |
| [RFC4302-SEQ-1](#rfc4302-seq-1) | The Sequence Number field is an unsigned 32-bit counter that increases by one for each packet sent on the SA. |
| [RFC4302-SEQ-2](#rfc4302-seq-2) | For a unicast SA or a single-sender multicast SA, the sender must increment the Sequence Number field for every transmitted packet. |
| [RFC4302-SEQ-3](#rfc4302-seq-3) | Sharing one SA among multiple senders is permitted, though generally not recommended. |
| [RFC4302-SEQ-4](#rfc4302-seq-4) | AH gives multiple senders on a multi-sender SA no way to synchronize their packet counters, so anti-replay is not available on that SA. |
| [RFC4302-SEQ-5](#rfc4302-seq-5) | The Sequence Number field must always be present, even when the receiver has not enabled anti-replay for the SA. |
| [RFC4302-SEQ-6](#rfc4302-seq-6) | Every AH implementation must be capable of the sequence-number generation and verification processing, even though acting on it is at the receiver's discretion. |
| [RFC4302-SEQ-7](#rfc4302-seq-7) | The sender must always transmit the Sequence Number field, even though the receiver need not act on it. |
| [RFC4302-SEQ-8](#rfc4302-seq-8) | Both the sender's and the receiver's counters start at zero when an SA is established, so the first packet sent carries sequence number one. |
| [RFC4302-SEQ-9](#rfc4302-seq-9) | If anti-replay is enabled, the transmitted sequence number must never be allowed to cycle. |
| [RFC4302-SEQ-10](#rfc4302-seq-10) | The sender and receiver must reset their counters, by establishing a new SA and key, before the 2^32nd packet on an SA. |
| [RFC4302-SEQ-11](#rfc4302-seq-11) | A new sequence-number option should be offered as an extension to the 32-bit field, to support high-speed implementations. |
| [RFC4302-SEQ-12](#rfc4302-seq-12) | Use of the Extended Sequence Number must be negotiated by an SA management protocol. |
| [RFC4302-SEQ-13](#rfc4302-seq-13) | The Extended Sequence Number feature applies to multicast SAs as well as unicast SAs. |
| [RFC4302-SEQ-14](#rfc4302-seq-14) | With ESN, an SA uses a 64-bit sequence number, but only its low-order 32 bits are transmitted in each packet's AH header. |
| [RFC4302-SEQ-15](#rfc4302-seq-15) | The high-order 32 bits of the ESN counter are kept by both sender and receiver and covered by the ICV, but never transmitted. |
| [RFC4302-ICV-1](#rfc4302-icv-1) | The ICV field is a variable-length field that holds the packet's Integrity Check Value. |
| [RFC4302-ICV-2](#rfc4302-icv-2) | The ICV field's length must be an integral multiple of 32 bits, for both IPv4 and IPv6. |
| [RFC4302-ICV-3](#rfc4302-icv-3) | The ICV field may carry explicit padding, to make the AH header's length a multiple of 32 bits (IPv4) or 64 bits (IPv6). |
| [RFC4302-ICV-4](#rfc4302-icv-4) | Every implementation must support ICV padding, and must insert only as much padding as the IPv4/IPv6 alignment requires. |
| [RFC4302-ICV-5](#rfc4302-icv-5) | The integrity algorithm's specification must state the ICV length and the comparison rules and processing steps used to validate it. |
| [RFC4302-LOC-1](#rfc4302-loc-1) | AH may be employed in transport mode or in tunnel mode. |
| [RFC4302-LOC-2](#rfc4302-loc-2) | In transport mode, AH is inserted after the IP header and before the next-layer protocol header or any already-inserted IPsec header; for IPv4 specifically, AH goes after the IP header and its options but before the next-layer protocol. |
| [RFC4302-LOC-3](#rfc4302-loc-3) | For IPv4 transport mode, the packet after AH is applied is ordered: original IP header (with any options), AH, next-layer protocol header, data. |
| [RFC4302-LOC-4](#rfc4302-loc-4) | In IPv4 transport mode, mutable-field processing is confined to the original IP header, and the packet is authenticated except for the mutable fields. |
| [RFC4302-LOC-5](#rfc4302-loc-5) | In the IPv6 context, AH appears after the hop-by-hop, routing, and fragmentation extension headers. |
| [RFC4302-LOC-6](#rfc4302-loc-6) | A destination options extension header may appear before the AH header, after it, or both, depending on the semantics desired. |
| [RFC4302-LOC-7](#rfc4302-loc-7) | For IPv6 transport mode, the packet after AH is applied is ordered: original IP header, hop-by-hop/routing/fragmentation extension headers (if present), AH, destination options extension header (if present), next-layer protocol header, data. |
| [RFC4302-LOC-8](#rfc4302-loc-8) | A "bump-in-the-stack" or "bump-in-the-wire" implementation in transport mode may need to reassemble and re-fragment IP traffic around IPsec processing, and needs special care when it uses multiple interfaces. |
| [RFC4302-LOC-9](#rfc4302-loc-9) | In tunnel mode, the inner IP header carries the ultimate source and destination addresses, while the outer IP header carries the addresses of the IPsec peers. |
| [RFC4302-LOC-10](#rfc4302-loc-10) | In tunnel mode, the inner and outer IP versions may be mixed, IPv6 over IPv4 or IPv4 over IPv6. |
| [RFC4302-LOC-11](#rfc4302-loc-11) | In tunnel mode, AH protects the entire inner IP packet, including the entire inner IP header. |
| [RFC4302-LOC-12](#rfc4302-loc-12) | AH's position relative to the outer IP header in tunnel mode is the same as its position in transport mode. |
| [RFC4302-LOC-13](#rfc4302-loc-13) | For IPv4 tunnel mode, the packet after AH is applied is ordered: new IP header (with any options), AH, original IP header (with any options), next-layer protocol header, data. |
| [RFC4302-LOC-14](#rfc4302-loc-14) | In IPv4 tunnel mode, mutable-field processing is confined to the new (outer) IP header, and the packet is authenticated except for the mutable fields in that new header. |
| [RFC4302-LOC-15](#rfc4302-loc-15) | For IPv6 tunnel mode, the packet after AH is applied is ordered: new IP header, extension headers (if present), AH, original IP header, extension headers (if present), next-layer protocol header, data. |
| [RFC4302-ALG-1](#rfc4302-alg-1) | The SA specifies the integrity algorithm used for the ICV computation. |
| [RFC4302-ALG-2](#rfc4302-alg-2) | For point-to-point communication, the integrity algorithm may be a keyed MAC built from a symmetric encryption algorithm or from a one-way hash function. |
| [RFC4302-OSA-1](#rfc4302-osa-1) | In transport mode, the sender inserts the AH header after the IP header and before the next-layer protocol header. |
| [RFC4302-OSA-2](#rfc4302-osa-2) | In tunnel mode, the outer and inner IP header and extensions can be related to each other in a variety of ways. |
| [RFC4302-OSA-3](#rfc4302-osa-3) | AH is applied to an outbound packet only after the IPsec implementation determines the packet is associated with an SA that calls for AH processing. |
| [RFC4302-OSEQ-1](#rfc4302-oseq-1) | The sender's counter is initialized to 0 when an SA is established. |
| [RFC4302-OSEQ-2](#rfc4302-oseq-2) | The sender increments the sequence-number (or ESN) counter and inserts its low-order 32 bits into the Sequence Number field, so the first packet sent on an SA carries sequence number one. |
| [RFC4302-OSEQ-3](#rfc4302-oseq-3) | If anti-replay is enabled, the sender checks that the counter has not cycled before inserting the new value into the Sequence Number field. |
| [RFC4302-OSEQ-4](#rfc4302-oseq-4) | The sender must not send a packet on an SA if doing so would cause the sequence number to cycle. |
| [RFC4302-OSEQ-5](#rfc4302-oseq-5) | An attempt to transmit a packet that would overflow the sequence number is an auditable event. |
| [RFC4302-OSEQ-6](#rfc4302-oseq-6) | The audit log entry for a sequence-number-overflow attempt should include the SPI value, current date/time, source address, destination address, and, in IPv6, the cleartext Flow ID. |
| [RFC4302-OSEQ-7](#rfc4302-oseq-7) | The sender assumes anti-replay is enabled by default, unless the receiver notifies otherwise or the SA is manually keyed. |
| [RFC4302-OSEQ-8](#rfc4302-oseq-8) | Typical sender behavior is to establish a new SA when the sequence number cycles, or in anticipation of it cycling. |
| [RFC4302-OSEQ-9](#rfc4302-oseq-9) | If anti-replay is disabled, the sender need not monitor or reset the counter, but still increments it and lets it roll over to zero at the maximum value. |
| [RFC4302-OSEQ-10](#rfc4302-oseq-10) | This roll-over behavior is recommended for multi-sender multicast SAs, unless an anti-replay mechanism outside this standard's scope has been negotiated. |
| [RFC4302-OSEQ-11](#rfc4302-oseq-11) | If ESN is selected, only the low-order 32 bits are transmitted in the Sequence Number field, though sender and receiver both maintain full 64-bit counters, and the high-order 32 bits are included in the ICV calculation. |
| [RFC4302-OSEQ-12](#rfc4302-oseq-12) | If a receiver does not enable anti-replay for an SA, the receiver should not negotiate ESN in an SA management protocol. |
| [RFC4302-OICV-1](#rfc4302-oicv-1) | The ICV covers the IP and extension header fields before the AH header that are immutable in transit, or predictable in value when they arrive at the AH SA's endpoint. |
| [RFC4302-OICV-2](#rfc4302-oicv-2) | The ICV covers the AH header itself, with the ICV field set to zero and including any explicit padding bytes. |
| [RFC4302-OICV-3](#rfc4302-oicv-3) | The ICV covers everything after the AH header, which is assumed immutable in transit. |
| [RFC4302-OICV-4](#rfc4302-oicv-4) | The ICV covers the ESN high-order bits, when used, and any implicit padding the integrity algorithm requires. |
| [RFC4302-OICV-5](#rfc4302-oicv-5) | A field that may be modified in transit is set to zero for the ICV computation. |
| [RFC4302-OICV-6](#rfc4302-oicv-6) | A mutable field whose value at the receiver is predictable has that predicted value inserted for the ICV calculation. |
| [RFC4302-OICV-7](#rfc4302-oicv-7) | The ICV field is itself set to zero before the ICV is computed. |
| [RFC4302-OICV-8](#rfc4302-oicv-8) | If the IP implementation does not recognize an extension header, it discards the packet and sends an ICMP message, so IPsec never processes that packet. |
| [RFC4302-OICV-9](#rfc4302-oicv-9) | If the IPsec implementation encounters an unrecognized IPv4 option, it should zero the whole option, using the option's second byte as its length. |
| [RFC4302-OICV-10](#rfc4302-oicv-10) | An IPv6 option in a Destination or Hop-by-Hop Extension Header carries a mutability flag that determines how the option is handled for the ICV. |
| [RFC4302-OICV-11](#rfc4302-oicv-11) | For IPv4, the Version, Internet Header Length, Total Length, Identification, Protocol, Source Address, and the Destination Address without loose or strict source routing, are immutable. |
| [RFC4302-OICV-12](#rfc4302-oicv-12) | For IPv4, when loose or strict source routing is used, the Destination Address field is mutable but predictable. |
| [RFC4302-OICV-13](#rfc4302-oicv-13) | For IPv4, the DSCP, ECN, Flags, Fragment Offset, Time to Live, and Header Checksum fields are mutable and are zeroed before the ICV calculation. |
| [RFC4302-OICV-14](#rfc4302-oicv-14) | AH applies only to non-fragmented IP packets, so the IPv4 Fragment Offset field on an AH packet must always be zero. |
| [RFC4302-OICV-15](#rfc4302-oicv-15) | For IPv4, options carry no in-transit mutability tag, so Appendix A explicitly classifies each option as immutable, mutable but predictable, or mutable. |
| [RFC4302-OICV-16](#rfc4302-oicv-16) | For IPv4, an option classified as mutable is zeroed in its entirety for the ICV computation, even though its type and length fields are individually immutable. |
| [RFC4302-OICV-17](#rfc4302-oicv-17) | For IPv6, the Version, Payload Length, Next Header, Source Address, and the Destination Address without a Routing Extension Header, are immutable. |
| [RFC4302-OICV-18](#rfc4302-oicv-18) | For IPv6, when a Routing Extension Header is used, the Destination Address field is mutable but predictable. |
| [RFC4302-OICV-19](#rfc4302-oicv-19) | For IPv6, the DSCP, ECN, Flow Label, and Hop Limit fields are mutable and are zeroed before the ICV calculation. |
| [RFC4302-OICV-20](#rfc4302-oicv-20) | An IPv6 option in a Hop-by-Hop or Destination Extension Header carries a bit that indicates whether the option might change unpredictably during transit. |
| [RFC4302-OICV-21](#rfc4302-oicv-21) | For an IPv6 option whose mutability bit marks it as changeable en route, the whole Option Data field must be treated as zero-valued octets for the ICV. |
| [RFC4302-OICV-22](#rfc4302-oicv-22) | The Option Type and Opt Data Len fields of an IPv6 option are always included in the ICV calculation. |
| [RFC4302-OICV-23](#rfc4302-oicv-23) | An IPv6 option whose mutability bit marks it as immutable is included in the ICV calculation in full. |
| [RFC4302-OICV-24](#rfc4302-oicv-24) | The IPv6 extension headers that carry no options are explicitly classified in Appendix A as immutable, mutable but predictable, or mutable. |
| [RFC4302-OICV-25](#rfc4302-oicv-25) | The ICV padding's length depends on the ICV's own length and on the IP protocol version, IPv4 or IPv6. |
| [RFC4302-OICV-26](#rfc4302-oicv-26) | The sender picks the ICV padding's content arbitrarily; it need not be random. |
| [RFC4302-OICV-27](#rfc4302-oicv-27) | The ICV padding bytes are covered by the ICV calculation, counted in the Payload Length, and transmitted at the end of the ICV field. |
| [RFC4302-OICV-28](#rfc4302-oicv-28) | Including more ICV padding than the minimum the IPv4/IPv6 alignment requires is prohibited. |
| [RFC4302-OICV-29](#rfc4302-oicv-29) | If ESN is elected for an SA, the sequence number's high-order 32 bits must be included in the ICV computation. |
| [RFC4302-OICV-30](#rfc4302-oicv-30) | For the ICV computation, the ESN's high-order bits are implicitly appended right after the payload and before any implicit packet padding. |
| [RFC4302-OICV-31](#rfc4302-oicv-31) | For some integrity algorithms, the byte string the ICV is computed over must be a multiple of the algorithm's blocksize. |
| [RFC4302-OICV-32](#rfc4302-oicv-32) | If the packet length, including AH and any ESN high-order bits, does not match the algorithm's blocksize, implicit padding must be appended to the end of the packet before the ICV computation. |
| [RFC4302-OICV-33](#rfc4302-oicv-33) | The implicit padding octets must have a value of zero. |
| [RFC4302-OICV-34](#rfc4302-oicv-34) | The algorithm's specification states the blocksize and hence the padding length, and this padding is never transmitted with the packet. |
| [RFC4302-OICV-35](#rfc4302-oicv-35) | The document that defines an integrity algorithm must be consulted to determine whether implicit padding is required. |
| [RFC4302-OICV-36](#rfc4302-oicv-36) | If an algorithm's document does not answer the question, the default assumption is that implicit padding is required to match the packet length to the algorithm's blocksize. |
| [RFC4302-OICV-37](#rfc4302-oicv-37) | If padding bytes are needed but the algorithm does not specify their contents, the padding octets must have a value of zero. |
| [RFC4302-FRAG-1](#rfc4302-frag-1) | IP fragmentation, if required, occurs after AH processing, so transport-mode AH applies only to whole IP datagrams, never to IP fragments. |
| [RFC4302-FRAG-2](#rfc4302-frag-2) | An IPv4 packet carrying AH may itself be fragmented by routers en route; such fragments must be reassembled before AH processing at the receiver. |
| [RFC4302-FRAG-3](#rfc4302-frag-3) | Router-initiated fragmentation of an AH packet does not occur for IPv6, since IPv6 has no router-initiated fragmentation. |
| [RFC4302-FRAG-4](#rfc4302-frag-4) | In tunnel mode, AH is applied to an IP packet whose payload may itself be a fragmented IP packet. |
| [RFC4302-FRAG-5](#rfc4302-frag-5) | A security gateway, or a bump-in-the-stack or bump-in-the-wire implementation, may apply tunnel-mode AH to a fragmented packet. |
| [RFC4302-FRAG-6](#rfc4302-frag-6) | A bump-in-the-stack or bump-in-the-wire implementation in transport mode may have to reassemble a locally fragmented packet, apply IPsec, and then re-fragment the result. |
| [RFC4302-FRAG-7](#rfc4302-frag-7) | For IPv6, a bump-in-the-stack or bump-in-the-wire implementation examines all extension headers to detect a fragmentation header and decide whether reassembly is needed before IPsec processing. |
| [RFC4302-FRAG-8](#rfc4302-frag-8) | An AH implementation may choose not to support fragmentation. |
| [RFC4302-FRAG-9](#rfc4302-frag-9) | An AH implementation may mark transmitted packets with the DF bit, to facilitate Path MTU discovery. |
| [RFC4302-FRAG-10](#rfc4302-frag-10) | In any case, an AH implementation must support generating ICMP PMTU messages, or equivalent internal signaling, to minimize the likelihood of fragmentation. |
| [RFC4302-REAS-1](#rfc4302-reas-1) | When more than one IPsec header is present, processing a given header ignores any IPsec header applied after it. |
| [RFC4302-REAS-2](#rfc4302-reas-2) | Reassembly, if required, is performed before AH processing. |
| [RFC4302-REAS-3](#rfc4302-reas-3) | If a packet offered to AH appears to be a fragment, the receiver must discard it, and this is an auditable event. |
| [RFC4302-REAS-4](#rfc4302-reas-4) | The audit log entry for a fragment discarded before AH processing should include the SPI value, date/time, source address, destination address, and, in IPv6, the Flow ID. |
| [RFC4302-REAS-5](#rfc4302-reas-5) | Because IPv4 does not require zeroing the Offset field or clearing the More-Fragments flag on reassembly, the IP code must do both after reassembling a packet, so IPsec processes it instead of discarding it as an apparent fragment. |
| [RFC4302-ISA-1](#rfc4302-isa-1) | Upon receiving an AH packet, the receiver determines the appropriate unidirectional SA by a lookup in the SAD. |
| [RFC4302-ISA-2](#rfc4302-isa-2) | For a unicast SA, the SAD lookup is based on the SPI, or on the SPI plus the protocol field. |
| [RFC4302-ISA-3](#rfc4302-isa-3) | If an implementation supports multicast traffic, the destination address is also used in the SAD lookup, and the sender address may also be used. |
| [RFC4302-ISA-4](#rfc4302-isa-4) | The SAD entry for an SA indicates whether the Sequence Number field is checked and whether 32-bit or 64-bit sequence numbers are used. |
| [RFC4302-ISA-5](#rfc4302-isa-5) | The SAD entry for an SA specifies the algorithm used for the ICV computation and the key needed to validate the ICV. |
| [RFC4302-ISA-6](#rfc4302-isa-6) | If no valid SA exists for the packet, the receiver must discard it, and this is an auditable event. |
| [RFC4302-ISA-7](#rfc4302-isa-7) | The audit log entry for a no-SA discard should include the SPI value, date/time, source address, destination address, and, in IPv6, the Flow ID. |
| [RFC4302-ISA-8](#rfc4302-isa-8) | SA-management traffic, such as IKE packets, need not be processed based on the SPI; it can be de-multiplexed separately, for example by the Next Protocol and Port fields. |
| [RFC4302-ISEQ-1](#rfc4302-iseq-1) | Every AH implementation must support the anti-replay service. |
| [RFC4302-ISEQ-2](#rfc4302-iseq-2) | The receiver may enable or disable anti-replay on a per-SA basis. |
| [RFC4302-ISEQ-3](#rfc4302-iseq-3) | Anti-replay applies to both unicast and multicast SAs. |
| [RFC4302-ISEQ-4](#rfc4302-iseq-4) | This standard specifies no anti-replay mechanism for a multi-sender SA, unicast or multicast. |
| [RFC4302-ISEQ-5](#rfc4302-iseq-5) | Without a negotiated or manually configured anti-replay mechanism for a multi-sender SA, sequence-number checking by sender and receiver is recommended to be disabled. |
| [RFC4302-ISEQ-6](#rfc4302-iseq-6) | If the receiver does not enable anti-replay for an SA, no inbound checks are performed on the Sequence Number. |
| [RFC4302-ISEQ-7](#rfc4302-iseq-7) | The sender's default assumption is that anti-replay is enabled at the receiver. |
| [RFC4302-ISEQ-8](#rfc4302-iseq-8) | If an SA establishment protocol such as IKE is used, the receiver should notify the sender, during SA establishment, when it will not provide anti-replay protection. |
| [RFC4302-ISEQ-9](#rfc4302-iseq-9) | If the receiver has enabled anti-replay for an SA, the receive packet counter must be initialized to zero when the SA is established. |
| [RFC4302-ISEQ-10](#rfc4302-iseq-10) | For each received packet, the receiver must verify that its Sequence Number does not duplicate that of any other packet received during the SA's life. |
| [RFC4302-ISEQ-11](#rfc4302-iseq-11) | The duplicate-sequence-number check should be the first AH check applied to a packet after it is matched to an SA. |
| [RFC4302-ISEQ-12](#rfc4302-iseq-12) | Duplicate packets are rejected using a sliding receive window; the window's implementation is a local matter, but it must exhibit the functionality described below. |
| [RFC4302-ISEQ-13](#rfc4302-iseq-13) | The window's right edge is the highest validated Sequence Number received on the SA. |
| [RFC4302-ISEQ-14](#rfc4302-iseq-14) | A packet with a sequence number lower than the window's left edge is rejected. |
| [RFC4302-ISEQ-15](#rfc4302-iseq-15) | A packet whose sequence number falls within the window is checked against the list of packets already received in the window. |
| [RFC4302-ISEQ-16](#rfc4302-iseq-16) | If ESN is selected, only the low-order 32 bits are transmitted, but the receiver reconstructs the full sequence number from its local high-order-bits counter when checking the window. |
| [RFC4302-ISEQ-17](#rfc4302-iseq-17) | If the packet's low-order 32 bits are lower than the receiver's counter's low-order 32 bits, the receiver assumes the high-order bits have incremented, moving to a new sequence-number subspace. |
| [RFC4302-ISEQ-18](#rfc4302-iseq-18) | This reconstruction algorithm tolerates a reception gap on a single SA as large as 2^32-1 packets. |
| [RFC4302-ISEQ-19](#rfc4302-iseq-19) | If a larger gap occurs, the receiver may apply additional heuristic checks to re-synchronize its sequence number counter. |
| [RFC4302-ISEQ-20](#rfc4302-iseq-20) | A packet within the window and not a duplicate, or to the right of the window, proceeds to ICV verification. |
| [RFC4302-ISEQ-21](#rfc4302-iseq-21) | If ICV validation fails, the receiver must discard the received IP datagram as invalid, and this is an auditable event. |
| [RFC4302-ISEQ-22](#rfc4302-iseq-22) | The audit log entry for an ICV-verification-failure discard should include the SPI value, date/time, source address, destination address, the sequence number, and, in IPv6, the Flow ID. |
| [RFC4302-ISEQ-23](#rfc4302-iseq-23) | The receive window is updated only if the ICV verification succeeds. |
| [RFC4302-ISEQ-24](#rfc4302-iseq-24) | A minimum receive-window size of 32 packets must be supported. |
| [RFC4302-ISEQ-25](#rfc4302-iseq-25) | A window size of 64 is preferred and should be used as the default. |
| [RFC4302-ISEQ-26](#rfc4302-iseq-26) | The receiver may choose another window size, larger than the minimum. |
| [RFC4302-ISEQ-27](#rfc4302-iseq-27) | The receiver does not notify the sender of the window size. |
| [RFC4302-ISEQ-28](#rfc4302-iseq-28) | The receive window size should be increased for higher-speed environments, irrespective of assurance issues. |
| [RFC4302-ISEQ-29](#rfc4302-iseq-29) | This standard does not specify minimum or recommended receive-window sizes for very-high-speed devices. |
| [RFC4302-IICV-1](#rfc4302-iicv-1) | The receiver computes the ICV over the appropriate fields of the packet, using the specified integrity algorithm, and checks it against the ICV carried in the packet. |
| [RFC4302-IICV-2](#rfc4302-iicv-2) | If the computed and received ICVs match, the datagram is valid and is accepted. |
| [RFC4302-IICV-3](#rfc4302-iicv-3) | If the ICVs do not match, the receiver must discard the received IP datagram as invalid, and this is an auditable event. |
| [RFC4302-IICV-4](#rfc4302-iicv-4) | The audit log entry for an ICV-mismatch discard should include the SPI value, date/time received, source address, destination address, and, in IPv6, the Flow ID. |
| [RFC4302-IICV-5](#rfc4302-iicv-5) | An implementation can use any set of steps that produces the same result as the steps described below. |
| [RFC4302-IICV-6](#rfc4302-iicv-6) | The receiver's verification procedure saves the received ICV, replaces it, but not any ICV field padding, with zero, and zeroes every other field that may have changed in transit. |
| [RFC4302-IICV-7](#rfc4302-iicv-7) | If ESN is elected for the SA, the receiver appends the sequence number's high-order 32 bits after the end of the packet before the verification ICV computation. |
| [RFC4302-IICV-8](#rfc4302-iicv-8) | The receiver checks the packet's overall length and, if the integrity algorithm requires it, appends zero-filled implicit padding to the end of the packet, after the ESN if present. |
| [RFC4302-IICV-9](#rfc4302-iicv-9) | The receiver performs the ICV computation and compares the result with the saved value, using the algorithm specification's comparison rules. |
| [RFC4302-CONF-1](#rfc4302-conf-1) | An implementation that claims conformance must fully implement the AH syntax and processing described in this specification for unicast traffic. |
| [RFC4302-CONF-2](#rfc4302-conf-2) | A conformant implementation must also comply with all requirements of the Security Architecture document. |
| [RFC4302-CONF-3](#rfc4302-conf-3) | An implementation that claims to support multicast traffic must comply with the additional requirements specified for that support. |
| [RFC4302-CONF-4](#rfc4302-conf-4) | If the ICV key is manually distributed, correct anti-replay provision needs correct sender counter-state maintenance until the key is replaced, with likely no automated recovery if counter overflow is imminent. |
| [RFC4302-CONF-5](#rfc4302-conf-5) | A compliant implementation should not provide the anti-replay service on manually keyed SAs. |
| [RFC4302-CONF-6](#rfc4302-conf-6) | The mandatory-to-implement algorithms for AH are defined in a separate RFC, kept independent of this protocol specification. |
| [RFC4302-CONF-7](#rfc4302-conf-7) | An implementation may support additional algorithms beyond those mandated for AH. |

## The AH format

### RFC4302-FMT-1

**The IP protocol header immediately before the AH header carries the AH protocol number.**

> "The protocol header (IPv4, IPv6, or IPv6 Extension) immediately preceding the AH header SHALL
> contain the value 51 in its Protocol (IPv4) or Next Header (IPv6, Extension) fields" — §2,
> `rfc4302.txt:184-186`.

- Strength: must. Class: wire.
- Check idea: capture the header immediately before an AH header on the link and check that its
  Protocol (IPv4) or Next Header (IPv6, Extension) field carries the value 51.

### RFC4302-FMT-2

**The AH header holds, in order, Next Header, Payload Length, Reserved, Security Parameters Index, Sequence Number, and Integrity Check Value fields.**

> "| Next Header   |  Payload Len  |          RESERVED             |" and "|                 Security
> Parameters Index (SPI)               |" and "|                    Sequence Number Field
> |" and "+                Integrity Check Value-ICV (variable)           |" — §2,
> `rfc4302.txt:192`, `rfc4302.txt:194`, `rfc4302.txt:196`, `rfc4302.txt:199`.

- Strength: description. Class: encoding.
- Check idea: capture an AH header on the link and check that its fields appear in this order:
  Next Header, Payload Length, Reserved, SPI, Sequence Number, Integrity Check Value.

### RFC4302-FMT-3

**Every field defined for the AH format is always present and is covered by the ICV computation.**

> "All the fields described here are mandatory; i.e., they are always present in the AH format
> and are included in the Integrity Check Value (ICV) computation" — §2, `rfc4302.txt:240-242`.

- Strength: description. Class: encoding.
- Check idea: capture an AH header and check that every defined field is present, and check that
  the receiver's ICV computation covers each of them.

### RFC4302-FMT-4

**Because AH carries no version number, IPsec peers must resolve any AH compatibility concern with a signaling or configuration mechanism, not with a version field.**

> "AH does not contain a version number, therefore if there are concerns about backward
> compatibility, they MUST be addressed by using a signaling mechanism between the two IPsec
> peers to ensure compatible versions of AH, e.g., IKE [IKEv2] or an out-of-band configuration
> mechanism." — §2, `rfc4302.txt:249-253`.

- Strength: must. Class: internal.
- Check idea: configure two IPsec peers with an AH backward-compatibility concern and check that
  they resolve it through a signaling protocol or an out-of-band configuration, since the AH
  header itself carries no version field.

## Next Header

### RFC4302-NH-1

**The Next Header field is 8 bits wide.**

> "The Next Header is an 8-bit field" — §2.1, `rfc4302.txt:257`.

- Strength: description. Class: encoding.
- Check idea: capture an AH header and check that the Next Header field occupies exactly 8 bits.

### RFC4302-NH-2

**The Next Header field carries the IANA IP Protocol Number of the payload that follows the AH header.**

> "The Next Header is an 8-bit field that identifies the type of the next payload after the
> Authentication Header. The value of this field is chosen from the set of IP Protocol Numbers
> defined on the web page of Internet Assigned Numbers Authority (IANA)." — §2.1,
> `rfc4302.txt:257-260`.

- Strength: description. Class: wire.
- Check idea: capture an AH header followed by a known next-layer payload and check that the
  Next Header field carries that payload's IANA IP Protocol Number.

## Payload Length

### RFC4302-LEN-1

**The Payload Length field states the AH header's length in 32-bit words, minus two.**

> "This 8-bit field specifies the length of AH in 32-bit words (4-byte units), minus "2"." —
> §2.2, `rfc4302.txt:266-267`.

- Strength: description. Class: encoding.
- Check idea: capture an AH header and check that the Payload Length field equals the header's
  length in 32-bit words, minus two.

### RFC4302-LEN-2

**For IPv6, the AH header's total length must be a multiple of 8 octets.**

> "For IPv6, the total length of the header must be a multiple of 8-octet units." — §2.2,
> `rfc4302.txt:270-271`.

- Strength: must (lower case). Class: encoding.
- Check idea: capture an AH header carried over IPv6 and check that its total length in octets is
  a multiple of 8.

## Reserved

### RFC4302-RSV-1

**The Reserved field is 16 bits wide and is held for future use.**

> "This 16-bit field is reserved for future use." — §2.3, `rfc4302.txt:289`.

- Strength: description. Class: encoding.
- Check idea: capture an AH header and check that the Reserved field occupies exactly 16 bits.

### RFC4302-RSV-2

**The sender sets the Reserved field to zero.**

> "It MUST be set to "zero" by the sender" — §2.3, `rfc4302.txt:289-290`.

- Strength: must. Class: wire.
- Check idea: capture an AH header sent by a host or security gateway and check that the Reserved
  field carries all zero bits.

### RFC4302-RSV-3

**The recipient ignores the value of the Reserved field.**

> "and it SHOULD be ignored by the recipient." — §2.3, `rfc4302.txt:290`.

- Strength: should. Class: end-to-end.
- Check idea: send an AH packet with a non-zero Reserved field to a receiver and check that the
  receiver still accepts the packet, disregarding the field's content.

### RFC4302-RSV-4

**The receiver's ICV computation covers the Reserved field's transmitted value.**

> "Note that the value is included in the ICV calculation, but is otherwise ignored by the
> recipient." — §2.3, `rfc4302.txt:291-292`.

- Strength: description. Class: encoding.
- Check idea: capture an AH packet and check that the Reserved field's transmitted bits are
  covered by the ICV computation.

## Security Parameters Index

### RFC4302-SPI-1

**The SPI is an arbitrary 32-bit value.**

> "The SPI is an arbitrary 32-bit value" — §2.4, `rfc4302.txt:296`.

- Strength: description. Class: encoding.
- Check idea: capture an AH header and check that the SPI field occupies 32 bits.

### RFC4302-SPI-2

**The receiver uses the SPI to identify the SA an incoming packet is bound to.**

> "that is used by a receiver to identify the SA to which an incoming packet is bound." — §2.4,
> `rfc4302.txt:296-297`.

- Strength: description. Class: internal.
- Check idea: send a packet on a known SA and check that the receiver selects that SA by looking
  up the packet's SPI value.

### RFC4302-SPI-3

**For a unicast SA, the SPI alone, or the SPI together with the IPsec protocol type, identifies the SA.**

> "For a unicast SA, the SPI can be used by itself to specify an SA, or it may be used in
> conjunction with the IPsec protocol type (in this case AH)." — §2.4, `rfc4302.txt:297-299`.

- Strength: may (lower case). Class: internal.
- Check idea: configure a unicast SA and check that the receiver identifies it from the SPI
  alone, or from the SPI together with the protocol field, according to its configuration.

### RFC4302-SPI-4

**For unicast SAs, the receiver generates the SPI value, and whether the SPI alone identifies the SA, or the SPI with the protocol value, is a local matter.**

> "Because for unicast SAs the SPI value is generated by the receiver, whether the value is
> sufficient to identify an SA by itself or whether it must be used in conjunction with the
> IPsec protocol value is a local matter." — §2.4, `rfc4302.txt:300-303`.

- Strength: must (lower case). Class: internal.
- Check idea: check that the SPI value of a unicast SA originates from the receiver, and that the
  receiver's choice of SPI-alone or SPI-plus-protocol lookup is a matter of local configuration.

### RFC4302-SPI-5

**Every AH implementation must support the SPI-based mechanism for mapping inbound traffic to unicast SAs.**

> "The SPI field is mandatory, and this mechanism for mapping inbound traffic to unicast SAs
> described above MUST be supported by all AH implementations." — §2.4, `rfc4302.txt:303-305`.

- Strength: must. Class: internal.
- Check idea: check that an AH implementation maps an inbound unicast packet to its SA using the
  SPI, or the SPI plus protocol, lookup mechanism.

### RFC4302-SPI-6

**An IPsec implementation that supports multicast must support multicast SAs with the SAD search algorithm below; a unicast-only implementation need not.**

> "If an IPsec implementation supports multicast, then it MUST support multicast SAs using the
> algorithm below for mapping inbound IPsec datagrams to SAs." and "Implementations that support
> only unicast traffic need not implement this de-multiplexing algorithm." — §2.4,
> `rfc4302.txt:307-309`, `rfc4302.txt:309-310`.

- Strength: must (multicast-capable implementations); description (unicast-only implementations). Class: internal.
- Check idea: on a multicast-capable implementation, check that inbound multicast IPsec datagrams
  are mapped to SAs with the algorithm below; check that a unicast-only implementation is not
  required to implement it.

### RFC4302-SPI-7

**A multicast-capable IPsec implementation must correctly de-multiplex inbound traffic even when a group SA and a unicast SA share the same SPI.**

> "A multicast-capable IPsec implementation MUST correctly de-multiplex inbound traffic even in
> the context of SPI collisions." — §2.4, `rfc4302.txt:318-320`.

- Strength: must. Class: internal.
- Check idea: configure a group SA and a unicast SA that share the same SPI value and check that
  the receiver still de-multiplexes each inbound packet to the correct SA.

### RFC4302-SPI-8

**Each SAD entry must indicate whether its SA lookup uses the destination address alone, or the destination and source addresses, in addition to the SPI.**

> "Each entry in the Security Association Database (SAD) [Ken-Arch] must indicate whether the SA
> lookup makes use of the destination, or destination and source, IP addresses, in addition to
> the SPI." — §2.4, `rfc4302.txt:322-324`.

- Strength: must (lower case). Class: internal.
- Check idea: check that every SAD entry records whether its lookup key is {SPI, destination
  address} or {SPI, destination address, source address}.

### RFC4302-SPI-9

**For multicast SAs, the SAD lookup does not use the protocol field.**

> "For multicast SAs, the protocol field is not employed for SA lookups." — §2.4,
> `rfc4302.txt:324-325`.

- Strength: description. Class: internal.
- Check idea: check that a receiver's SAD lookup for a multicast SA never uses the protocol field
  as part of the match.

### RFC4302-SPI-10

**For each inbound IPsec-protected packet, an implementation must search the SAD for the entry with the longest matching SA identifier.**

> "For each inbound, IPsec-protected packet, an implementation must conduct its search of the
> SAD such that it finds the entry that matches the "longest" SA identifier." — §2.4,
> `rfc4302.txt:326-328`.

- Strength: must (lower case). Class: internal.
- Check idea: present several SAD entries that could match an inbound packet and check that the
  receiver selects the entry with the longest matching identifier.

### RFC4302-SPI-11

**When two or more SAD entries match on the SPI, the entry that also matches on the destination address, or on the destination and source addresses, is the longest match.**

> "In this context, if two or more SAD entries match based on the SPI value, then the entry that
> also matches based on destination, or destination and source, address comparison (as
> indicated in the SAD entry) is the "longest" match." — §2.4, `rfc4302.txt:328-331`.

- Strength: description. Class: internal.
- Check idea: configure two SAD entries that share an SPI value, where only one also matches the
  packet's destination and source addresses, and check that the receiver picks that entry.

### RFC4302-SPI-12

**The first step of the inbound SAD search matches on SPI, destination address, and source address.**

> "Search the SAD for a match on {SPI, destination address, source address}. If an SAD entry
> matches, then process the inbound AH packet with that matching SAD entry. Otherwise, proceed
> to step 2." — §2.4, `rfc4302.txt:343-346`.

- Strength: description. Class: internal.
- Check idea: present an inbound AH packet that matches an SAD entry on SPI, destination
  address, and source address, and check that the receiver processes the packet with that entry.

### RFC4302-SPI-13

**The second step of the inbound SAD search matches on SPI and destination address.**

> "Search the SAD for a match on {SPI, destination address}. If an SAD entry matches, then
> process the inbound AH packet with that matching SAD entry. Otherwise, proceed to step 3." —
> §2.4, `rfc4302.txt:348-351`.

- Strength: description. Class: internal.
- Check idea: present an inbound AH packet that matches an SAD entry only on SPI and destination
  address, with no entry also matching on source address, and check that the receiver processes
  the packet with the {SPI, destination address} entry.

### RFC4302-SPI-14

**The third step of the inbound SAD search matches on the SPI alone, or on the SPI and protocol, depending on whether the receiver keeps one SPI space for AH and ESP.**

> "Search the SAD for a match on only {SPI} if the receiver has chosen to maintain a single SPI
> space for AH and ESP, or on {SPI, protocol} otherwise. If an SAD entry matches, then process
> the inbound AH packet with that matching SAD entry." — §2.4, `rfc4302.txt:353-357`.

- Strength: description. Class: internal.
- Check idea: present an inbound AH packet that matches an SAD entry only on {SPI} or only on
  {SPI, protocol}, with no more specific entry present, and check that the receiver processes
  the packet with that entry.

### RFC4302-SPI-15

**When no SAD entry matches by the end of the third step, the receiver discards the packet and logs an auditable event.**

> "Otherwise, discard the packet and log an auditable event." — §2.4, `rfc4302.txt:357-358`.

- Strength: description. Class: internal.
- Check idea: present an inbound AH packet with no matching SAD entry at any of the three steps
  and check that the receiver discards it and creates an audit log entry.

### RFC4302-SPI-16

**An implementation may accelerate the SAD search by any method, but its externally visible behavior must be equivalent to searching in the three-step order.**

> "In practice, an implementation MAY choose any method to accelerate this search, although its
> externally visible behavior MUST be functionally equivalent to having searched the SAD in the
> above order." — §2.4, `rfc4302.txt:360-363`.

- Strength: may (the choice of method); must (the equivalent behavior). Class: internal.
- Check idea: check that whichever search method a receiver uses, the SAD entry it selects for an
  inbound packet always matches the one the three-step order would have selected.

### RFC4302-SPI-17

**The choice of whether inbound lookup requires address matching must be set by manual SA configuration or by SA-management-protocol negotiation.**

> "The indication of whether source and destination address matching is required to map inbound
> IPsec traffic to SAs MUST be set either as a side effect of manual SA configuration or via
> negotiation using an SA management protocol, e.g., IKE or Group Domain of Interpretation
> (GDOI) [RFC3547]." — §2.4, `rfc4302.txt:373-377`.

- Strength: must. Class: internal.
- Check idea: configure an SA manually, or negotiate it with an SA management protocol, and check
  that the SAD entry's address-matching indication is set consistently with that configuration or
  negotiation.

### RFC4302-SPI-18

**A Source-Specific Multicast group typically uses an SA identifier composed of the SPI, the destination multicast address, and the source address.**

> "Typically, Source-Specific Multicast (SSM) [HC03] groups use a 3-tuple SA identifier composed
> of an SPI, a destination multicast address, and source address." — §2.4,
> `rfc4302.txt:377-379`.

- Strength: description. Class: internal.
- Check idea: configure an SSM group SA and check that the SAD entry's identifier is the 3-tuple
  of SPI, destination multicast address, and source address.

### RFC4302-SPI-19

**An Any-Source Multicast group SA needs only an SPI and a destination multicast address as its identifier.**

> "An Any-Source Multicast group SA requires only an SPI and a destination multicast address as
> an identifier." — §2.4, `rfc4302.txt:379-381`.

- Strength: description. Class: internal.
- Check idea: configure an Any-Source Multicast group SA and check that the SAD entry's
  identifier is the pair of SPI and destination multicast address, with no source address.

### RFC4302-SPI-20

**The SPI values 1 through 255 are reserved by IANA for future use.**

> "The set of SPI values in the range 1 through 255 is reserved by the Internet Assigned Numbers
> Authority (IANA) for future use; a reserved SPI value will not normally be assigned by IANA
> unless the use of the assigned SPI value is specified in an RFC." — §2.4,
> `rfc4302.txt:383-386`.

- Strength: description. Class: wire.
- Check idea: capture AH packets on the link and check that no SPI value in the range 1-255
  appears, other than one an RFC has specified.

### RFC4302-SPI-21

**The SPI value of zero must not be sent on the wire.**

> "The SPI value of zero (0) is reserved for local, implementation-specific use and MUST NOT be
> sent on the wire." — §2.4, `rfc4302.txt:386-388`.

- Strength: must not. Class: wire.
- Check idea: capture AH packets on the link and check that none of them carries an SPI field
  value of zero.

## Sequence Number

### RFC4302-SEQ-1

**The Sequence Number field is an unsigned 32-bit counter that increases by one for each packet sent on the SA.**

> "This unsigned 32-bit field contains a counter value that increases by one for each packet
> sent, i.e., a per-SA packet sequence number." — §2.5, `rfc4302.txt:405-406`.

- Strength: description. Class: encoding.
- Check idea: capture consecutive AH packets sent on one SA and check that the Sequence Number
  field increases by one from each packet to the next.

### RFC4302-SEQ-2

**For a unicast SA or a single-sender multicast SA, the sender must increment the Sequence Number field for every transmitted packet.**

> "For a unicast SA or a single-sender multicast SA, the sender MUST increment this field for
> every transmitted packet." — §2.5, `rfc4302.txt:406-408`.

- Strength: must. Class: wire.
- Check idea: send a series of packets on a unicast or single-sender multicast SA and check that
  the Sequence Number field is incremented on every packet.

### RFC4302-SEQ-3

**Sharing one SA among multiple senders is permitted, though generally not recommended.**

> "Sharing an SA among multiple senders is permitted, though generally not recommended." — §2.5,
> `rfc4302.txt:408-410`.

- Strength: description. Class: internal.
- Check idea: configure an SA with multiple senders and check that the implementation allows it,
  while treating it as a discouraged configuration.

### RFC4302-SEQ-4

**AH gives multiple senders on a multi-sender SA no way to synchronize their packet counters, so anti-replay is not available on that SA.**

> "AH provides no means of synchronizing packet counters among multiple senders or meaningfully
> managing a receiver packet counter and window in the context of multiple senders. Thus, for a
> multi-sender SA, the anti-reply features of AH are not available" — §2.5,
> `rfc4302.txt:410-413`.

- Strength: description. Class: internal.
- Check idea: configure a multi-sender SA and check that the receiver does not provide the
  anti-replay service for it.

### RFC4302-SEQ-5

**The Sequence Number field must always be present, even when the receiver has not enabled anti-replay for the SA.**

> "The field is mandatory and MUST always be present even if the receiver does not elect to
> enable the anti-replay service for a specific SA." — §2.5, `rfc4302.txt:416-418`.

- Strength: must. Class: wire.
- Check idea: configure a receiver with anti-replay disabled for an SA and check that AH packets
  on that SA still carry the Sequence Number field.

### RFC4302-SEQ-6

**Every AH implementation must be capable of the sequence-number generation and verification processing, even though acting on it is at the receiver's discretion.**

> "Processing of the Sequence Number field is at the discretion of the receiver, but all AH
> implementations MUST be capable of performing the processing described in Section 3.3.2,
> "Sequence Number Generation", and Section 3.4.3, "Sequence Number Verification"." — §2.5,
> `rfc4302.txt:418-422`.

- Strength: must. Class: internal.
- Check idea: check that an AH implementation can perform sequence-number generation and
  verification processing, independent of whether the receiver chooses to act on the field.

### RFC4302-SEQ-7

**The sender must always transmit the Sequence Number field, even though the receiver need not act on it.**

> "Thus, the sender MUST always transmit this field, but the receiver need not act upon it." —
> §2.5, `rfc4302.txt:422-423`.

- Strength: must. Class: wire.
- Check idea: capture AH packets from a sender whose receiver does not act on the Sequence
  Number and check that the field is still present and populated.

### RFC4302-SEQ-8

**Both the sender's and the receiver's counters start at zero when an SA is established, so the first packet sent carries sequence number one.**

> "The sender's counter and the receiver's counter are initialized to 0 when an SA is
> established." and "The first packet sent using a given SA will have a sequence number of 1" —
> §2.5, `rfc4302.txt:425-426`, `rfc4302.txt:426-427`.

- Strength: description. Class: internal.
- Check idea: establish a new SA and check that the first AH packet sent on it carries sequence
  number one.

### RFC4302-SEQ-9

**If anti-replay is enabled, the transmitted sequence number must never be allowed to cycle.**

> "If anti-replay is enabled (the default), the transmitted sequence number must never be
> allowed to cycle." — §2.5, `rfc4302.txt:428-430`.

- Strength: must (lower case). Class: wire.
- Check idea: run an SA with anti-replay enabled up to its highest sequence number and check that
  the sender never wraps the counter back to a lower value on that SA.

### RFC4302-SEQ-10

**The sender and receiver must reset their counters, by establishing a new SA and key, before the 2^32nd packet on an SA.**

> "Thus, the sender's counter and the receiver's counter MUST be reset (by establishing a new SA
> and thus a new key) prior to the transmission of the 2^32nd packet on an SA." — §2.5,
> `rfc4302.txt:430-432`.

- Strength: must. Class: internal.
- Check idea: drive an SA's sequence counter close to its maximum value and check that a new SA
  and key are established before the 2^32nd packet would be sent.

### RFC4302-SEQ-11

**A new sequence-number option should be offered as an extension to the 32-bit field, to support high-speed implementations.**

> "To support high-speed IPsec implementations, a new option for sequence numbers SHOULD be
> offered, as an extension to the current, 32-bit sequence number field." — §2.5.1,
> `rfc4302.txt:436-438`.

- Strength: should. Class: internal.
- Check idea: check that an implementation offers the Extended Sequence Number option in
  addition to the 32-bit Sequence Number field.

### RFC4302-SEQ-12

**Use of the Extended Sequence Number must be negotiated by an SA management protocol.**

> "Use of an Extended Sequence Number (ESN) MUST be negotiated by an SA management protocol." —
> §2.5.1, `rfc4302.txt:438-439`.

- Strength: must. Class: internal.
- Check idea: establish an SA that uses ESN and check that the two peers negotiated the option
  through an SA management protocol beforehand.

### RFC4302-SEQ-13

**The Extended Sequence Number feature applies to multicast SAs as well as unicast SAs.**

> "(The ESN feature is applicable to multicast as well as unicast SAs.)" — §2.5.1,
> `rfc4302.txt:441-442`.

- Strength: description. Class: internal.
- Check idea: configure ESN on a multicast SA and check that the implementation accepts it, the
  same as for a unicast SA.

### RFC4302-SEQ-14

**With ESN, an SA uses a 64-bit sequence number, but only its low-order 32 bits are transmitted in each packet's AH header.**

> "The ESN facility allows use of a 64-bit sequence number for an SA." and "Only the low-order 32
> bits of the sequence number are transmitted in" and "the AH header of each packet, thus
> minimizing packet overhead." — §2.5.1, `rfc4302.txt:444`, `rfc4302.txt:446`,
> `rfc4302.txt:455`.

- Strength: description. Class: wire.
- Check idea: capture an AH packet from an SA using ESN and check that the Sequence Number field
  carries only the low-order 32 bits of the 64-bit counter.

### RFC4302-SEQ-15

**The high-order 32 bits of the ESN counter are kept by both sender and receiver and covered by the ICV, but never transmitted.**

> "The high-order 32 bits are maintained as part of the sequence number counter by both
> transmitter and receiver and are included in the computation of the ICV, but are not
> transmitted." — §2.5.1, `rfc4302.txt:455-458`.

- Strength: description. Class: internal.
- Check idea: check that neither the sender nor the receiver transmits the high-order 32 bits of
  the ESN counter, while both use those bits in their respective ICV computations.

## Integrity Check Value (ICV)

### RFC4302-ICV-1

**The ICV field is a variable-length field that holds the packet's Integrity Check Value.**

> "This is a variable-length field that contains the Integrity Check Value (ICV) for this
> packet." — §2.6, `rfc4302.txt:462-463`.

- Strength: description. Class: encoding.
- Check idea: capture an AH header and check that the ICV field is present and holds the
  packet's Integrity Check Value.

### RFC4302-ICV-2

**The ICV field's length must be an integral multiple of 32 bits, for both IPv4 and IPv6.**

> "The field must be an integral multiple of 32 bits (IPv4 or IPv6) in length." — §2.6,
> `rfc4302.txt:463-464`.

- Strength: must (lower case). Class: encoding.
- Check idea: capture an AH header and check that the ICV field's length in bits is a multiple
  of 32, over both IPv4 and IPv6.

### RFC4302-ICV-3

**The ICV field may carry explicit padding, to make the AH header's length a multiple of 32 bits (IPv4) or 64 bits (IPv6).**

> "This field may include explicit padding, if required to ensure that the length of the AH
> header is an integral multiple of 32 bits (IPv4) or 64 bits (IPv6)." — §2.6,
> `rfc4302.txt:466-469`.

- Strength: may (lower case). Class: encoding.
- Check idea: capture an AH header whose ICV length would otherwise misalign the header and check
  that explicit padding brings the header to a multiple of 32 bits (IPv4) or 64 bits (IPv6).

### RFC4302-ICV-4

**Every implementation must support ICV padding, and must insert only as much padding as the IPv4/IPv6 alignment requires.**

> "All implementations MUST support such padding and MUST insert only enough padding to satisfy
> the IPv4/IPv6 alignment requirements." — §2.6, `rfc4302.txt:469-471`.

- Strength: must. Class: encoding.
- Check idea: capture an AH header from an implementation and check that it supports ICV padding
  and never inserts more padding than the alignment requirement needs.

### RFC4302-ICV-5

**The integrity algorithm's specification must state the ICV length and the comparison rules and processing steps used to validate it.**

> "The integrity algorithm specification MUST specify the length of the ICV and the comparison
> rules and processing steps for validation." — §2.6, `rfc4302.txt:472-474`.

- Strength: must. Class: internal.
- Check idea: check that the integrity algorithm specification an SA uses states the ICV length
  and the comparison rules and processing steps for validating it.

## The location of AH

### RFC4302-LOC-1

**AH may be employed in transport mode or in tunnel mode.**

> "AH may be employed in two ways: transport mode or tunnel mode." — §3.1, `rfc4302.txt:480`.

- Strength: may (lower case). Class: wire.
- Check idea: configure an SA in transport mode and separately in tunnel mode and check that the
  AH header appears at the position each mode defines.

### RFC4302-LOC-2

**In transport mode, AH is inserted after the IP header and before the next-layer protocol header or any already-inserted IPsec header; for IPv4 specifically, AH goes after the IP header and its options but before the next-layer protocol.**

> "In transport mode, AH is inserted after the IP header and before a next layer protocol (e.g.,
> TCP, UDP, ICMP, etc.) or before any other IPsec headers that have already been inserted." and
> "In the context of IPv4, this calls for placing AH after the IP header (and any options that it
> contains), but before the next layer protocol." — §3.1.1, `rfc4302.txt:486-488`,
> `rfc4302.txt:488-490`.

- Strength: description. Class: wire.
- Check idea: capture an IPv4 packet in transport mode and check that AH sits after the IP header
  and its options, and before the next-layer protocol header or any IPsec header already
  inserted.

### RFC4302-LOC-3

**For IPv4 transport mode, the packet after AH is applied is ordered: original IP header (with any options), AH, next-layer protocol header, data.**

> "|original IP hdr (any options) | AH | TCP |    Data   |" — §3.1.1, `rfc4302.txt:519`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv4 transport-mode packet with AH applied and check that the fields
  appear in this order: original IP header (with options), AH, next-layer header, data.

### RFC4302-LOC-4

**In IPv4 transport mode, mutable-field processing is confined to the original IP header, and the packet is authenticated except for the mutable fields.**

> "|<- mutable field processing ->|<- immutable fields ->|" and "|<----- authenticated except for
> mutable fields ----->|" — §3.1.1, `rfc4302.txt:521`, `rfc4302.txt:522`.

- Strength: description. Class: end-to-end.
- Check idea: capture an IPv4 transport-mode AH packet and check that the ICV covers the whole
  packet except for the fields the IP header marks as mutable.

### RFC4302-LOC-5

**In the IPv6 context, AH appears after the hop-by-hop, routing, and fragmentation extension headers.**

> "In the IPv6 context, AH is viewed as an end-to-end payload, and thus should appear after
> hop-by-hop, routing, and fragmentation extension headers." — §3.1.1, `rfc4302.txt:524-526`.

- Strength: should (lower case). Class: wire.
- Check idea: capture an IPv6 transport-mode packet carrying hop-by-hop, routing, or
  fragmentation extension headers and check that AH appears after all of them.

### RFC4302-LOC-6

**A destination options extension header may appear before the AH header, after it, or both, depending on the semantics desired.**

> "The destination options extension header(s) could appear before or after or both before and
> after the AH header depending on the semantics desired." — §3.1.1, `rfc4302.txt:526-528`.

- Strength: description. Class: wire.
- Check idea: capture an IPv6 transport-mode packet and check that a destination options header,
  when present, appears before AH, after AH, or in both places.

### RFC4302-LOC-7

**For IPv6 transport mode, the packet after AH is applied is ordered: original IP header, hop-by-hop/routing/fragmentation extension headers (if present), AH, destination options extension header (if present), next-layer protocol header, data.**

> "IPv6  |             |hop-by-hop, dest*, |    | dest |     |      |" and "|orig IP hdr  |routing,
> fragment. | AH | opt* | TCP | Data |" — §3.1.1, `rfc4302.txt:539`, `rfc4302.txt:540`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv6 transport-mode packet with AH applied and check that the headers
  appear in this order.

### RFC4302-LOC-8

**A "bump-in-the-stack" or "bump-in-the-wire" implementation in transport mode may need to reassemble and re-fragment IP traffic around IPsec processing, and needs special care when it uses multiple interfaces.**

> "Note that in transport mode, for "bump-in-the-stack" or "bump-in-the-wire" implementations,
> as defined in the Security Architecture document, inbound and outbound IP fragments may
> require an IPsec implementation to perform extra IP reassembly/fragmentation in order to both
> conform to this specification and provide transparent IPsec support." and "Special care is
> required to perform such operations within these implementations when multiple interfaces are
> in use." — §3.1.1, `rfc4302.txt:551-556`, `rfc4302.txt:556-557`.

- Strength: may (lower case). Class: internal.
- Check idea: configure a bump-in-the-stack or bump-in-the-wire implementation with multiple
  interfaces and check that fragmented traffic is reassembled before AH processing and
  re-fragmented afterward.

### RFC4302-LOC-9

**In tunnel mode, the inner IP header carries the ultimate source and destination addresses, while the outer IP header carries the addresses of the IPsec peers.**

> "In tunnel mode, the "inner" IP header carries the ultimate (IP) source and destination
> addresses, while an "outer" IP header contains the addresses of the IPsec "peers," e.g.,
> addresses of security gateways." — §3.1.2, `rfc4302.txt:569-572`.

- Strength: description. Class: wire.
- Check idea: capture a tunnel-mode packet and check that the outer IP header carries the IPsec
  peers' addresses while the inner IP header carries the ultimate source and destination
  addresses.

### RFC4302-LOC-10

**In tunnel mode, the inner and outer IP versions may be mixed, IPv6 over IPv4 or IPv4 over IPv6.**

> "Mixed inner and outer IP versions are allowed, i.e., IPv6 over IPv4 and IPv4 over IPv6." —
> §3.1.2, `rfc4302.txt:572-573`.

- Strength: description. Class: wire.
- Check idea: configure a tunnel-mode SA with an inner IP version different from the outer IP
  version and check that the implementation allows the combination.

### RFC4302-LOC-11

**In tunnel mode, AH protects the entire inner IP packet, including the entire inner IP header.**

> "In tunnel mode, AH protects the entire inner IP packet, including the entire inner IP
> header." — §3.1.2, `rfc4302.txt:573-574`.

- Strength: description. Class: end-to-end.
- Check idea: capture a tunnel-mode AH packet and check that the ICV covers the whole inner
  packet, including the inner IP header.

### RFC4302-LOC-12

**AH's position relative to the outer IP header in tunnel mode is the same as its position in transport mode.**

> "The position of AH in tunnel mode, relative to the outer IP header, is the same as for AH in
> transport mode." — §3.1.2, `rfc4302.txt:574-576`.

- Strength: description. Class: wire.
- Check idea: capture a tunnel-mode packet and a transport-mode packet and check that AH sits at
  the same position relative to the (outer) IP header in both.

### RFC4302-LOC-13

**For IPv4 tunnel mode, the packet after AH is applied is ordered: new IP header (with any options), AH, original IP header (with any options), next-layer protocol header, data.**

> "|                              |    | orig IP hdr*  |   |      |" and "|new IP header * (any
> options) | AH | (any options) |TCP| Data |" — §3.1.2, `rfc4302.txt:580`, `rfc4302.txt:581`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv4 tunnel-mode packet with AH applied and check that the headers
  appear in this order.

### RFC4302-LOC-14

**In IPv4 tunnel mode, mutable-field processing is confined to the new (outer) IP header, and the packet is authenticated except for the mutable fields in that new header.**

> "|<- mutable field processing ->|<------ immutable fields ----->|" and "|<- authenticated
> except for mutable fields in the new IP hdr->|" — §3.1.2, `rfc4302.txt:583`,
> `rfc4302.txt:584`.

- Strength: description. Class: end-to-end.
- Check idea: capture an IPv4 tunnel-mode AH packet and check that the ICV covers the whole
  packet except for the fields the new (outer) IP header marks as mutable.

### RFC4302-LOC-15

**For IPv6 tunnel mode, the packet after AH is applied is ordered: new IP header, extension headers (if present), AH, original IP header, extension headers (if present), next-layer protocol header, data.**

> "IPv6 |           | ext hdrs*|    |            | ext hdrs*|   |    |" and "|new IP hdr*|if
> present| AH |orig IP hdr*|if present|TCP|Data|" — §3.1.2, `rfc4302.txt:587`,
> `rfc4302.txt:588`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv6 tunnel-mode packet with AH applied and check that the headers
  appear in this order.

## Integrity algorithms

### RFC4302-ALG-1

**The SA specifies the integrity algorithm used for the ICV computation.**

> "The integrity algorithm employed for the ICV computation is specified by the SA." — §3.2,
> `rfc4302.txt:600-601`.

- Strength: description. Class: internal.
- Check idea: configure an SA with a chosen integrity algorithm and check that the ICV
  computation for that SA uses the configured algorithm.

### RFC4302-ALG-2

**For point-to-point communication, the integrity algorithm may be a keyed MAC built from a symmetric encryption algorithm or from a one-way hash function.**

> "For point-to-point communication, suitable integrity algorithms include keyed Message
> Authentication Codes (MACs) based on symmetric encryption algorithms (e.g., AES [AES]) or on
> one-way hash functions (e.g., MD5, SHA-1, SHA-256, etc.)." — §3.2, `rfc4302.txt:601-604`.

- Strength: description. Class: internal.
- Check idea: configure a point-to-point SA with a keyed-MAC integrity algorithm, built from a
  symmetric cipher or a one-way hash function, and check that the implementation accepts it.

## Outbound processing and SA lookup

### RFC4302-OSA-1

**In transport mode, the sender inserts the AH header after the IP header and before the next-layer protocol header.**

> "In transport mode, the sender inserts the AH header after the IP header and before a next
> layer protocol header, as described above." — §3.3, `rfc4302.txt:610-611`.

- Strength: description. Class: wire.
- Check idea: capture an outbound transport-mode packet and check that the sender placed the AH
  header after the IP header and before the next-layer protocol header.

### RFC4302-OSA-2

**In tunnel mode, the outer and inner IP header and extensions can be related to each other in a variety of ways.**

> "In tunnel mode, the outer and inner IP header/extensions can be" and "interrelated in a
> variety of ways." — §3.3, `rfc4302.txt:612`, `rfc4302.txt:623`.

- Strength: description. Class: wire.
- Check idea: configure a tunnel-mode SA and check that the sender's construction of the outer
  and inner IP headers and extensions is one of the allowed relationships.

### RFC4302-OSA-3

**AH is applied to an outbound packet only after the IPsec implementation determines the packet is associated with an SA that calls for AH processing.**

> "AH is applied to an outbound packet only after an IPsec implementation determines that the
> packet is associated with an SA that calls for AH processing." — §3.3.1,
> `rfc4302.txt:629-631`.

- Strength: description. Class: internal.
- Check idea: send an outbound packet that matches no AH-requiring SA and check that the sender
  does not apply AH to it.

## Sequence number generation

### RFC4302-OSEQ-1

**The sender's counter is initialized to 0 when an SA is established.**

> "The sender's counter is initialized to 0 when an SA is established." — §3.3.2,
> `rfc4302.txt:637`.

- Strength: description. Class: internal.
- Check idea: establish a new SA and check that the sender's sequence-number counter starts at
  zero.

### RFC4302-OSEQ-2

**The sender increments the sequence-number (or ESN) counter and inserts its low-order 32 bits into the Sequence Number field, so the first packet sent on an SA carries sequence number one.**

> "The sender increments the sequence number (or ESN) counter for this SA and inserts the
> low-order 32 bits of the value into the Sequence Number field." and "Thus, the first packet
> sent using a given SA will contain a sequence number of 1." — §3.3.2, `rfc4302.txt:638-640`,
> `rfc4302.txt:640-641`.

- Strength: description. Class: wire.
- Check idea: send the first packet on a new SA and check that its Sequence Number field carries
  the value one.

### RFC4302-OSEQ-3

**If anti-replay is enabled, the sender checks that the counter has not cycled before inserting the new value into the Sequence Number field.**

> "If anti-replay is enabled (the default), the sender checks to ensure that the counter has not
> cycled before inserting the new value in the Sequence Number field." — §3.3.2,
> `rfc4302.txt:643-645`.

- Strength: description. Class: internal.
- Check idea: run an SA with anti-replay enabled and check that the sender verifies the counter
  has not cycled before it sends each packet's sequence number.

### RFC4302-OSEQ-4

**The sender must not send a packet on an SA if doing so would cause the sequence number to cycle.**

> "In other words, the sender MUST NOT send a packet on an SA if doing so would cause the
> sequence number to cycle." — §3.3.2, `rfc4302.txt:645-646`.

- Strength: must not. Class: wire.
- Check idea: drive an SA's sequence number to its maximum value and check that the sender does
  not send a further packet that would cycle the counter.

### RFC4302-OSEQ-5

**An attempt to transmit a packet that would overflow the sequence number is an auditable event.**

> "An attempt to transmit a packet that would result in sequence number overflow is an auditable
> event." — §3.3.2, `rfc4302.txt:647-648`.

- Strength: description. Class: internal.
- Check idea: drive an SA's sequence number to the point of overflow and check that the sender
  logs an audit event for the blocked transmission attempt.

### RFC4302-OSEQ-6

**The audit log entry for a sequence-number-overflow attempt should include the SPI value, current date/time, source address, destination address, and, in IPv6, the cleartext Flow ID.**

> "The audit log entry for this event SHOULD include the SPI value, current date/time, Source
> Address, Destination Address, and (in IPv6) the cleartext Flow ID." — §3.3.2,
> `rfc4302.txt:648-650`.

- Strength: should. Class: internal.
- Check idea: trigger a sequence-number-overflow attempt and check that the audit log entry
  carries the SPI value, current date/time, source and destination addresses, and, for IPv6, the
  cleartext Flow ID.

### RFC4302-OSEQ-7

**The sender assumes anti-replay is enabled by default, unless the receiver notifies otherwise or the SA is manually keyed.**

> "The sender assumes anti-replay is enabled as a default, unless otherwise notified by the
> receiver (see Section 3.4.3) or if the SA was configured using manual key management." —
> §3.3.2, `rfc4302.txt:652-654`.

- Strength: description. Class: internal.
- Check idea: establish an SA with no anti-replay notification from the receiver and check that
  the sender treats anti-replay as enabled by default; repeat with a manually keyed SA.

### RFC4302-OSEQ-8

**Typical sender behavior is to establish a new SA when the sequence number cycles, or in anticipation of it cycling.**

> "Thus, typical behavior of an AH implementation calls for the sender to establish a new SA when
> the Sequence Number (or ESN) cycles, or in anticipation of this value cycling." — §3.3.2,
> `rfc4302.txt:654-657`.

- Strength: description. Class: internal.
- Check idea: drive an SA's sequence number toward its cycle point and check that the sender
  establishes a new SA in anticipation of, or upon, the cycle.

### RFC4302-OSEQ-9

**If anti-replay is disabled, the sender need not monitor or reset the counter, but still increments it and lets it roll over to zero at the maximum value.**

> "If anti-replay is disabled (as noted above), the sender does not need to monitor or reset the
> counter, e.g., in the case of manual key management (see Section 5)." and "However, the sender
> still increments the counter and when it reaches the maximum value, the counter rolls over
> back to zero." — §3.3.2, `rfc4302.txt:659-661`, `rfc4302.txt:661-663`.

- Strength: description. Class: internal.
- Check idea: run a manually keyed SA with anti-replay disabled past its maximum sequence number
  and check that the sender's counter rolls over to zero without being reset by the receiver.

### RFC4302-OSEQ-10

**This roll-over behavior is recommended for multi-sender multicast SAs, unless an anti-replay mechanism outside this standard's scope has been negotiated.**

> "This behavior is recommended for multi-sender, multicast SAs, unless anti-replay mechanisms
> outside the scope of this standard are negotiated between the sender and receiver." — §3.3.2,
> `rfc4302.txt:663-665`.

- Strength: should (lower case). Class: internal.
- Check idea: configure a multi-sender multicast SA with no external anti-replay mechanism
  negotiated and check that its counter rolls over to zero at the maximum value instead of being
  reset.

### RFC4302-OSEQ-11

**If ESN is selected, only the low-order 32 bits are transmitted in the Sequence Number field, though sender and receiver both maintain full 64-bit counters, and the high-order 32 bits are included in the ICV calculation.**

> "If ESN (see Appendix B) is selected, only the low-order 32 bits of the sequence number are
> transmitted in the Sequence Number field, although both sender and receiver maintain full
> 64-bit ESN counters." and "However, the high-order 32 bits are included in the ICV
> calculation." — §3.3.2, `rfc4302.txt:667-669`, `rfc4302.txt:670`.

- Strength: description. Class: encoding.
- Check idea: capture a packet from an ESN-enabled SA and check that the Sequence Number field
  carries only the low-order 32 bits, while the high-order 32 bits still affect the ICV.

### RFC4302-OSEQ-12

**If a receiver does not enable anti-replay for an SA, the receiver should not negotiate ESN in an SA management protocol.**

> "If a receiver chooses not to enable anti-replay for an SA, then the receiver SHOULD NOT
> negotiate ESN in an SA management protocol." — §3.3.2, `rfc4302.txt:679-680`.

- Strength: should not. Class: internal.
- Check idea: configure a receiver with anti-replay disabled for an SA and check that it does not
  negotiate the ESN option for that SA through an SA management protocol.

## ICV calculation

### RFC4302-OICV-1

**The ICV covers the IP and extension header fields before the AH header that are immutable in transit, or predictable in value when they arrive at the AH SA's endpoint.**

> "IP or extension header fields before the AH header that are either immutable in transit or
> that are predictable in value upon arrival at the endpoint for the AH SA" — §3.3.3,
> `rfc4302.txt:690-692`.

- Strength: description. Class: encoding.
- Check idea: capture an AH packet and check that the ICV covers the header fields before AH
  that are immutable in transit or predictable at the receiving endpoint.

### RFC4302-OICV-2

**The ICV covers the AH header itself, with the ICV field set to zero and including any explicit padding bytes.**

> "the AH header (Next Header, Payload Len, Reserved, SPI, Sequence Number (low-order 32 bits),
> and the ICV (which is set to zero for this computation), and explicit padding bytes (if
> any))" — §3.3.3, `rfc4302.txt:693-696`.

- Strength: description. Class: encoding.
- Check idea: capture an AH packet and check that the ICV computation covers Next Header,
  Payload Len, Reserved, SPI, the low-order Sequence Number, a zeroed ICV field, and any
  explicit padding bytes.

### RFC4302-OICV-3

**The ICV covers everything after the AH header, which is assumed immutable in transit.**

> "everything after AH is assumed to be immutable in transit" — §3.3.3, `rfc4302.txt:697`.

- Strength: description. Class: end-to-end.
- Check idea: capture an AH packet and check that the ICV covers the entire payload that follows
  the AH header.

### RFC4302-OICV-4

**The ICV covers the ESN high-order bits, when used, and any implicit padding the integrity algorithm requires.**

> "the high-order bits of the ESN (if employed), and any implicit padding required by the
> integrity algorithm" — §3.3.3, `rfc4302.txt:698-699`.

- Strength: description. Class: encoding.
- Check idea: check that, on an ESN-enabled SA, the ICV computation covers the sequence number's
  high-order 32 bits and any implicit padding the integrity algorithm needs.

### RFC4302-OICV-5

**A field that may be modified in transit is set to zero for the ICV computation.**

> "If a field may be modified during transit, the value of the field is set to zero for
> purposes of the ICV computation." — §3.3.3.1, `rfc4302.txt:703-704`.

- Strength: may (lower case). Class: encoding.
- Check idea: check that a mutable header field is replaced with zero before the ICV
  computation.

### RFC4302-OICV-6

**A mutable field whose value at the receiver is predictable has that predicted value inserted for the ICV calculation.**

> "If a field is mutable, but its value at the (IPsec) receiver is predictable, then that value
> is inserted into the field for purposes of the ICV calculation." — §3.3.3.1,
> `rfc4302.txt:704-707`.

- Strength: description. Class: encoding.
- Check idea: check that a mutable-but-predictable field, such as a source-routed destination
  address, is set to its predicted arrival value before the ICV calculation.

### RFC4302-OICV-7

**The ICV field is itself set to zero before the ICV is computed.**

> "The Integrity Check Value field is also set to zero in preparation for this computation." —
> §3.3.3.1, `rfc4302.txt:707-708`.

- Strength: description. Class: encoding.
- Check idea: check that both sender and receiver zero the ICV field before performing the ICV
  computation.

### RFC4302-OICV-8

**If the IP implementation does not recognize an extension header, it discards the packet and sends an ICMP message, so IPsec never processes that packet.**

> "If the IP (v4 or v6) implementation encounters an extension header that it does not
> recognize, it will discard the packet and send an ICMP message." and "IPsec will never see the
> packet." — §3.3.3.1, `rfc4302.txt:718-720`, `rfc4302.txt:720-721`.

- Strength: description. Class: error-signal.
- Check idea: send a packet with an unrecognized extension header and check that the IP layer
  discards it and sends an ICMP error, so no AH processing occurs on it.

### RFC4302-OICV-9

**If the IPsec implementation encounters an unrecognized IPv4 option, it should zero the whole option, using the option's second byte as its length.**

> "If the IPsec implementation encounters an IPv4 option that it does not recognize, it should
> zero the whole option, using the second byte of the option as the length." — §3.3.3.1,
> `rfc4302.txt:721-723`.

- Strength: should (lower case). Class: encoding.
- Check idea: send an AH packet with an unrecognized IPv4 option and check that the receiver
  zeroes the whole option for the ICV computation, reading the option's length from its second
  byte.

### RFC4302-OICV-10

**An IPv6 option in a Destination or Hop-by-Hop Extension Header carries a mutability flag that determines how the option is handled for the ICV.**

> "IPv6 options (in Destination Extension Headers or the Hop-by-Hop Extension Header) contain a
> flag indicating mutability, which determines appropriate processing for such options." —
> §3.3.3.1, `rfc4302.txt:723-726`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv6 option and check that its mutability flag determines whether the
  option is zeroed or included as-is in the ICV computation.

### RFC4302-OICV-11

**For IPv4, the Version, Internet Header Length, Total Length, Identification, Protocol, Source Address, and the Destination Address without loose or strict source routing, are immutable.**

> "Immutable
> Version
> Internet Header Length
> Total Length
> Identification
> Protocol (This should be the value for AH.)
> Source Address
> Destination Address (without loose or strict source routing)" — §3.3.3.1.1.1,
> `rfc4302.txt:741-748`.

- Strength: should (lower case). Class: encoding.
- Check idea: capture an IPv4 AH packet and check that the ICV covers the Version, Internet
  Header Length, Total Length, Identification, Protocol, Source Address, and (without source
  routing) Destination Address fields at their transmitted values.

### RFC4302-OICV-12

**For IPv4, when loose or strict source routing is used, the Destination Address field is mutable but predictable.**

> "Mutable but predictable
> Destination Address (with loose or strict source routing)" — §3.3.3.1.1.1,
> `rfc4302.txt:750-751`.

- Strength: description. Class: encoding.
- Check idea: send a source-routed IPv4 AH packet and check that the ICV covers the Destination
  Address field at its predicted final value rather than its value at the sender.

### RFC4302-OICV-13

**For IPv4, the DSCP, ECN, Flags, Fragment Offset, Time to Live, and Header Checksum fields are mutable and are zeroed before the ICV calculation.**

> "Mutable (zeroed prior to ICV calculation)
> Differentiated Services Code Point (DSCP)
> (6 bits, see RFC 2474 [NBBB98])
> Explicit Congestion Notification (ECN)
> (2 bits, see RFC 3168 [RFB01])
> Flags
> Fragment Offset
> Time to Live (TTL)
> Header Checksum" — §3.3.3.1.1.1, `rfc4302.txt:753-761`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv4 AH packet and check that the ICV computation used a zero value for
  the DSCP, ECN, Flags, Fragment Offset, TTL, and Header Checksum fields, whatever their
  transmitted values.

### RFC4302-OICV-14

**AH applies only to non-fragmented IP packets, so the IPv4 Fragment Offset field on an AH packet must always be zero.**

> "Since AH is applied only to non-fragmented IP packets, the Offset Field must always be zero,
> and thus it is excluded (even though it is predictable)." — §3.3.3.1.1.1,
> `rfc4302.txt:774-776`.

- Strength: must (lower case). Class: wire.
- Check idea: capture an IPv4 packet carrying AH and check that its Fragment Offset field is
  always zero.

### RFC4302-OICV-15

**For IPv4, options carry no in-transit mutability tag, so Appendix A explicitly classifies each option as immutable, mutable but predictable, or mutable.**

> "For IPv4 (unlike IPv6), there is no mechanism for tagging options as mutable in transit.
> Hence the IPv4 options are explicitly listed in Appendix A and classified as immutable,
> mutable but predictable, or mutable." — §3.3.3.1.1.2, `rfc4302.txt:797-800`.

- Strength: description. Class: internal.
- Check idea: check that an IPv4 AH implementation classifies each IP option it handles
  according to Appendix A's immutable, mutable-but-predictable, or mutable listing.

### RFC4302-OICV-16

**For IPv4, an option classified as mutable is zeroed in its entirety for the ICV computation, even though its type and length fields are individually immutable.**

> "For IPv4, the entire option is viewed as a unit; so even though the type and length fields
> within most options are immutable in transit, if an option is classified as mutable, the
> entire option is zeroed for ICV computation purposes." — §3.3.3.1.1.2,
> `rfc4302.txt:800-803`.

- Strength: description. Class: encoding.
- Check idea: send an IPv4 AH packet carrying an option classified as mutable and check that the
  ICV computation used an all-zero value for the whole option, including its type and length
  bytes.

### RFC4302-OICV-17

**For IPv6, the Version, Payload Length, Next Header, Source Address, and the Destination Address without a Routing Extension Header, are immutable.**

> "Immutable
> Version
> Payload Length
> Next Header
> Source Address
> Destination Address (without Routing Extension Header)" — §3.3.3.1.2.1,
> `rfc4302.txt:811-816`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv6 AH packet and check that the ICV covers the Version, Payload
  Length, Next Header, Source Address, and (without a Routing header) Destination Address
  fields at their transmitted values.

### RFC4302-OICV-18

**For IPv6, when a Routing Extension Header is used, the Destination Address field is mutable but predictable.**

> "Mutable but predictable
> Destination Address (with Routing Extension Header)" — §3.3.3.1.2.1, `rfc4302.txt:818-819`.

- Strength: description. Class: encoding.
- Check idea: send an IPv6 AH packet with a Routing Extension Header and check that the ICV
  covers the Destination Address field at its predicted final value.

### RFC4302-OICV-19

**For IPv6, the DSCP, ECN, Flow Label, and Hop Limit fields are mutable and are zeroed before the ICV calculation.**

> "Mutable (zeroed prior to ICV calculation)
> DSCP (6 bits, see RFC2474 [NBBB98])
> ECN (2 bits, see RFC3168 [RFB01])
> Flow Label (*)
> Hop Limit" — §3.3.3.1.2.1, `rfc4302.txt:821-825`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv6 AH packet and check that the ICV computation used a zero value for
  the DSCP, ECN, Flow Label, and Hop Limit fields, whatever their transmitted values.

### RFC4302-OICV-20

**An IPv6 option in a Hop-by-Hop or Destination Extension Header carries a bit that indicates whether the option might change unpredictably during transit.**

> "IPv6 options in the Hop-by-Hop and Destination Extension Headers contain a bit that indicates
> whether the option might change (unpredictably) during transit." — §3.3.3.1.2.2,
> `rfc4302.txt:834-836`.

- Strength: description. Class: encoding.
- Check idea: capture an IPv6 Hop-by-Hop or Destination option and check that its mutability bit
  is present.

### RFC4302-OICV-21

**For an IPv6 option whose mutability bit marks it as changeable en route, the whole Option Data field must be treated as zero-valued octets for the ICV.**

> "For any option for which contents may change en-route, the entire "Option Data" field must
> be treated as zero-valued octets when computing or verifying the ICV." — §3.3.3.1.2.2,
> `rfc4302.txt:836-838`.

- Strength: must (lower case). Class: encoding.
- Check idea: send an IPv6 option whose mutability bit marks it as changeable and check that the
  ICV computation used all-zero octets for its Option Data field.

### RFC4302-OICV-22

**The Option Type and Opt Data Len fields of an IPv6 option are always included in the ICV calculation.**

> "Option Type and Opt Data Len are included in the ICV calculation." — §3.3.3.1.2.2,
> `rfc4302.txt:847`.

- Strength: description. Class: encoding.
- Check idea: capture any IPv6 option on an AH packet and check that its Option Type and Opt
  Data Len fields are covered by the ICV, whether or not the option's data is mutable.

### RFC4302-OICV-23

**An IPv6 option whose mutability bit marks it as immutable is included in the ICV calculation in full.**

> "All options for which the bit indicates immutability are included in the ICV calculation." —
> §3.3.3.1.2.2, `rfc4302.txt:848-849`.

- Strength: description. Class: encoding.
- Check idea: send an IPv6 option whose mutability bit marks it immutable and check that the ICV
  computation covers its full transmitted value.

### RFC4302-OICV-24

**The IPv6 extension headers that carry no options are explicitly classified in Appendix A as immutable, mutable but predictable, or mutable.**

> "The IPv6 extension headers that do not contain options are explicitly listed in Appendix A
> and classified as immutable, mutable but predictable, or mutable." — §3.3.3.1.2.3,
> `rfc4302.txt:854-856`.

- Strength: description. Class: internal.
- Check idea: check that an IPv6 AH implementation classifies each option-free extension header
  it handles according to Appendix A's immutable, mutable-but-predictable, or mutable listing.

### RFC4302-OICV-25

**The ICV padding's length depends on the ICV's own length and on the IP protocol version, IPv4 or IPv6.**

> "If padding is required, its length is" and "determined by two factors:" and "the length of
> the ICV" and "the IP protocol version (v4 or v6)" — §3.3.3.2.1, `rfc4302.txt:864-865`,
> `rfc4302.txt:865`, `rfc4302.txt:867`, `rfc4302.txt:868`.

- Strength: description. Class: encoding.
- Check idea: change the ICV length or the IP protocol version on an SA and check that the
  amount of explicit ICV padding changes accordingly.

### RFC4302-OICV-26

**The sender picks the ICV padding's content arbitrarily; it need not be random.**

> "The content of the padding field is arbitrarily selected by the sender." and "(The padding is
> arbitrary, but need not be random to achieve security.)" — §3.3.3.2.1, `rfc4302.txt:873-874`,
> `rfc4302.txt:875-876`.

- Strength: description. Class: wire.
- Check idea: capture the ICV padding bytes of several AH packets on the same SA and check that
  their content is not required to follow any particular pattern.

### RFC4302-OICV-27

**The ICV padding bytes are covered by the ICV calculation, counted in the Payload Length, and transmitted at the end of the ICV field.**

> "These padding bytes are included in the ICV calculation, counted as part of the Payload
> Length, and transmitted at the end of the ICV field to enable the receiver to perform the ICV
> calculation." — §3.3.3.2.1, `rfc4302.txt:876-878`.

- Strength: description. Class: wire.
- Check idea: capture an AH header with ICV padding and check that the Payload Length field
  counts the padding, that the padding bytes sit at the end of the ICV field, and that the ICV
  covers them.

### RFC4302-OICV-28

**Including more ICV padding than the minimum the IPv4/IPv6 alignment requires is prohibited.**

> "Inclusion of padding in excess of the minimum amount required to satisfy IPv4/IPv6 alignment
> requirements is prohibited." — §3.3.3.2.1, `rfc4302.txt:879-880`.

- Strength: must not (lower case). Class: encoding.
- Check idea: capture an AH header and check that its explicit ICV padding is no longer than the
  minimum needed to satisfy the IPv4 or IPv6 alignment requirement.

### RFC4302-OICV-29

**If ESN is elected for an SA, the sequence number's high-order 32 bits must be included in the ICV computation.**

> "If the ESN option is elected for an SA, then the high-order 32 bits of the ESN must be
> included in the ICV computation." — §3.3.3.2.2, `rfc4302.txt:884-885`.

- Strength: must (lower case). Class: encoding.
- Check idea: configure an SA with ESN and check that the ICV computation includes the sequence
  number's high-order 32 bits.

### RFC4302-OICV-30

**For the ICV computation, the ESN's high-order bits are implicitly appended right after the payload and before any implicit packet padding.**

> "For purposes of ICV computation, these bits are appended (implicitly) immediately after the
> end of the payload, and before any implicit packet padding." — §3.3.3.2.2,
> `rfc4302.txt:885-887`.

- Strength: description. Class: encoding.
- Check idea: check that, in the ICV computation of an ESN-enabled SA, the high-order sequence
  bits are placed right after the payload and before any implicit padding, though never
  transmitted in that position.

### RFC4302-OICV-31

**For some integrity algorithms, the byte string the ICV is computed over must be a multiple of the algorithm's blocksize.**

> "For some integrity algorithms, the byte string over which the ICV computation is performed
> must be a multiple of a blocksize specified by the algorithm." — §3.3.3.2.2,
> `rfc4302.txt:889-891`.

- Strength: must (lower case). Class: encoding.
- Check idea: use an integrity algorithm with a stated blocksize and check that the byte string
  fed to the ICV computation is a multiple of that blocksize.

### RFC4302-OICV-32

**If the packet length, including AH and any ESN high-order bits, does not match the algorithm's blocksize, implicit padding must be appended to the end of the packet before the ICV computation.**

> "If the IP packet length (including AH and the 32 high-order bits of the ESN, if enabled) does
> not match the blocksize requirements for the algorithm, implicit padding MUST be appended to
> the end of the packet, prior to ICV computation." — §3.3.3.2.2, `rfc4302.txt:891-894`.

- Strength: must. Class: encoding.
- Check idea: use an integrity algorithm whose blocksize the packet length does not match and
  check that implicit padding is appended to the end of the packet before the ICV computation.

### RFC4302-OICV-33

**The implicit padding octets must have a value of zero.**

> "The padding octets" and "MUST have a value of zero." — §3.3.3.2.2, `rfc4302.txt:894`,
> `rfc4302.txt:903`.

- Strength: must. Class: encoding.
- Check idea: check that every implicit padding octet appended before the ICV computation has
  the value zero.

### RFC4302-OICV-34

**The algorithm's specification states the blocksize and hence the padding length, and this padding is never transmitted with the packet.**

> "The blocksize (and hence the length of the padding) is specified by the algorithm
> specification." and "This padding is not transmitted with the packet." — §3.3.3.2.2,
> `rfc4302.txt:903-905`, `rfc4302.txt:905`.

- Strength: description. Class: internal.
- Check idea: check that the integrity algorithm's specification states the blocksize used to
  size the implicit padding, and that the padding never appears on the link.

### RFC4302-OICV-35

**The document that defines an integrity algorithm must be consulted to determine whether implicit padding is required.**

> "The document that defines an integrity algorithm MUST be consulted to determine if implicit
> padding is required as described above." — §3.3.3.2.2, `rfc4302.txt:905-907`.

- Strength: must. Class: internal.
- Check idea: check that an implementation determines whether implicit padding is required for
  an integrity algorithm by consulting that algorithm's defining document.

### RFC4302-OICV-36

**If an algorithm's document does not answer the question, the default assumption is that implicit padding is required to match the packet length to the algorithm's blocksize.**

> "If the document does not specify an answer to this, then the default is to assume that
> implicit padding is required (as needed to match the packet length to the algorithm's
> blocksize.)" — §3.3.3.2.2, `rfc4302.txt:907-910`.

- Strength: description. Class: internal.
- Check idea: use an integrity algorithm whose document is silent on implicit padding and check
  that the implementation defaults to applying implicit padding to match the blocksize.

### RFC4302-OICV-37

**If padding bytes are needed but the algorithm does not specify their contents, the padding octets must have a value of zero.**

> "If padding bytes are needed but the algorithm does not specify the padding contents, then the
> padding octets MUST have a value of zero." — §3.3.3.2.2, `rfc4302.txt:910-912`.

- Strength: must. Class: encoding.
- Check idea: use an integrity algorithm that requires padding but does not specify its content
  and check that the padding octets used are all zero.

## Fragmentation

### RFC4302-FRAG-1

**IP fragmentation, if required, occurs after AH processing, so transport-mode AH applies only to whole IP datagrams, never to IP fragments.**

> "If required, IP fragmentation occurs after AH processing within an IPsec implementation.
> Thus, transport mode AH is applied only to whole IP datagrams (not to IP fragments)." —
> §3.3.4, `rfc4302.txt:916-918`.

- Strength: description. Class: end-to-end.
- Check idea: send a large packet in transport mode and check that AH is applied to the whole
  datagram before fragmentation, never to an individual fragment.

### RFC4302-FRAG-2

**An IPv4 packet carrying AH may itself be fragmented by routers en route; such fragments must be reassembled before AH processing at the receiver.**

> "An IPv4 packet to which AH has been applied may itself be fragmented by routers en route, and
> such fragments must be reassembled prior to AH processing at a receiver." — §3.3.4,
> `rfc4302.txt:918-921`.

- Strength: must (lower case). Class: end-to-end.
- Check idea: let routers fragment an IPv4 AH packet en route and check that the receiver
  reassembles the fragments before it performs AH processing.

### RFC4302-FRAG-3

**Router-initiated fragmentation of an AH packet does not occur for IPv6, since IPv6 has no router-initiated fragmentation.**

> "(This does not apply to IPv6, where there is no router-" and "initiated fragmentation.)" —
> §3.3.4, `rfc4302.txt:921`, `rfc4302.txt:922`.

- Strength: description. Class: wire.
- Check idea: capture an IPv6 AH packet on the link and check that no router along the path
  fragments it.

### RFC4302-FRAG-4

**In tunnel mode, AH is applied to an IP packet whose payload may itself be a fragmented IP packet.**

> "In tunnel mode, AH is applied to an IP packet, the payload of which may be a fragmented IP
> packet." — §3.3.4, `rfc4302.txt:922-924`.

- Strength: may (lower case). Class: end-to-end.
- Check idea: apply tunnel-mode AH to an outer packet whose payload is an IP fragment and check
  that the implementation accepts it.

### RFC4302-FRAG-5

**A security gateway, or a bump-in-the-stack or bump-in-the-wire implementation, may apply tunnel-mode AH to a fragmented packet.**

> "a security gateway or a "bump-in-the-stack" or "bump-in-the-wire" IPsec implementation (see
> the Security Architecture document for details) may apply tunnel mode AH to such fragments." —
> §3.3.4, `rfc4302.txt:924-926`.

- Strength: may (lower case). Class: end-to-end.
- Check idea: configure a security gateway, or a bump-in-the-stack or bump-in-the-wire
  implementation, to apply tunnel-mode AH to an already-fragmented packet and check that it
  succeeds.

### RFC4302-FRAG-6

**A bump-in-the-stack or bump-in-the-wire implementation in transport mode may have to reassemble a locally fragmented packet, apply IPsec, and then re-fragment the result.**

> "NOTE: For transport mode -- As mentioned at the end of Section 3.1.1, bump-in-the-stack and
> bump-in-the-wire implementations may have to first reassemble a packet fragmented by the local
> IP layer, then apply IPsec, and then fragment the resulting packet." — §3.3.4,
> `rfc4302.txt:928-931`.

- Strength: may (lower case). Class: end-to-end.
- Check idea: send a locally fragmented packet through a bump-in-the-stack or bump-in-the-wire
  implementation in transport mode and check that it reassembles the packet, applies AH, and
  then re-fragments the result.

### RFC4302-FRAG-7

**For IPv6, a bump-in-the-stack or bump-in-the-wire implementation examines all extension headers to detect a fragmentation header and decide whether reassembly is needed before IPsec processing.**

> "NOTE: For IPv6 -- For bump-in-the-stack and bump-in-the-wire implementations, it will be
> necessary to examine all the extension headers to determine if there is a fragmentation header
> and hence that the packet needs reassembling prior to IPsec processing." — §3.3.4,
> `rfc4302.txt:933-936`.

- Strength: description. Class: internal.
- Check idea: send an IPv6 packet with a fragmentation extension header through a
  bump-in-the-stack or bump-in-the-wire implementation and check that it examines the extension
  headers and reassembles the packet before IPsec processing.

### RFC4302-FRAG-8

**An AH implementation may choose not to support fragmentation.**

> "Thus, an AH implementation MAY choose to not support fragmentation" — §3.3.4,
> `rfc4302.txt:942`.

- Strength: may. Class: internal.
- Check idea: configure an AH implementation that does not support fragmentation and check that
  it still conforms otherwise, simply rejecting or not producing fragments.

### RFC4302-FRAG-9

**An AH implementation may mark transmitted packets with the DF bit, to facilitate Path MTU discovery.**

> "and may mark transmitted packets with the DF bit, to facilitate Path MTU (PMTU) discovery." —
> §3.3.4, `rfc4302.txt:942-944`.

- Strength: may (lower case). Class: wire.
- Check idea: capture packets from an AH implementation and check whether it sets the DF bit to
  support Path MTU discovery.

### RFC4302-FRAG-10

**In any case, an AH implementation must support generating ICMP PMTU messages, or equivalent internal signaling, to minimize the likelihood of fragmentation.**

> "In any case, an AH implementation MUST support generation of ICMP PMTU messages (or
> equivalent internal signaling for native host implementations) to minimize the likelihood of
> fragmentation." — §3.3.4, `rfc4302.txt:944-947`.

- Strength: must. Class: error-signal.
- Check idea: send an oversized packet through an AH implementation and check that it generates
  an ICMP PMTU message, or the equivalent internal signal on a native host, instead of silently
  fragmenting.

## Inbound processing and reassembly

### RFC4302-REAS-1

**When more than one IPsec header is present, processing a given header ignores any IPsec header applied after it.**

> "If there is more than one IPsec header/extension present, the processing for each one ignores
> (does not zero, does not use) any IPsec headers applied subsequent to the header being
> processed." — §3.4, `rfc4302.txt:961-963`.

- Strength: description. Class: internal.
- Check idea: build a packet with AH followed by another IPsec header and check that AH
  processing neither zeroes nor uses the header applied after it.

### RFC4302-REAS-2

**Reassembly, if required, is performed before AH processing.**

> "If required, reassembly is performed prior to AH processing." — §3.4.1, `rfc4302.txt:967`.

- Strength: description. Class: end-to-end.
- Check idea: send a fragmented packet to a receiver that requires reassembly and check that
  reassembly completes before AH processing begins.

### RFC4302-REAS-3

**If a packet offered to AH appears to be a fragment, the receiver must discard it, and this is an auditable event.**

> "If a packet offered to AH for processing appears to be an IP fragment, i.e., the OFFSET field
> is nonzero or the MORE FRAGMENTS flag is set, the receiver MUST discard the packet; this is an
> auditable event." — §3.4.1, `rfc4302.txt:967-970`.

- Strength: must. Class: wire.
- Check idea: offer AH a packet with a nonzero Offset field or the More-Fragments flag set and
  check that the receiver discards it and logs an audit event.

### RFC4302-REAS-4

**The audit log entry for a fragment discarded before AH processing should include the SPI value, date/time, source address, destination address, and, in IPv6, the Flow ID.**

> "The audit log entry for this event SHOULD include the SPI value, date/time, Source Address,
> Destination Address, and (in IPv6) the Flow ID." — §3.4.1, `rfc4302.txt:971-973`.

- Strength: should. Class: internal.
- Check idea: trigger the fragment-discard event and check that the audit log entry carries the
  SPI value, date/time, source and destination addresses, and, for IPv6, the Flow ID.

### RFC4302-REAS-5

**Because IPv4 does not require zeroing the Offset field or clearing the More-Fragments flag on reassembly, the IP code must do both after reassembling a packet, so IPsec processes it instead of discarding it as an apparent fragment.**

> "the current IPv4 spec does NOT require either the zeroing of the OFFSET field or the clearing
> of the MORE FRAGMENTS flag. In order for a reassembled packet to be processed by IPsec (as
> opposed to discarded as an apparent fragment), the IP code must do these two things after it
> reassembles a packet." — §3.4.1, `rfc4302.txt:975-979`.

- Strength: must (lower case). Class: internal.
- Check idea: reassemble a fragmented packet and check that the IP code clears the Offset field
  and the More-Fragments flag before handing the packet to IPsec, so it is processed rather than
  discarded as an apparent fragment.

## Inbound SA lookup

### RFC4302-ISA-1

**Upon receiving an AH packet, the receiver determines the appropriate unidirectional SA by a lookup in the SAD.**

> "Upon receipt of a packet containing an IP Authentication Header, the receiver determines the
> appropriate (unidirectional) SA via lookup in the SAD." — §3.4.2, `rfc4302.txt:983-985`.

- Strength: description. Class: internal.
- Check idea: send an AH packet to a receiver and check that it selects the SA for that packet
  through a SAD lookup.

### RFC4302-ISA-2

**For a unicast SA, the SAD lookup is based on the SPI, or on the SPI plus the protocol field.**

> "For a unicast SA, this determination is based on the SPI or the SPI plus protocol field, as
> described in Section 2.4." — §3.4.2, `rfc4302.txt:985-986`.

- Strength: description. Class: internal.
- Check idea: send an AH packet on a unicast SA and check that the receiver's SAD lookup keys on
  the SPI, or on the SPI together with the protocol field.

### RFC4302-ISA-3

**If an implementation supports multicast traffic, the destination address is also used in the SAD lookup, and the sender address may also be used.**

> "If an implementation supports multicast traffic, the destination address is also employed in
> the lookup (in addition to the SPI), and the sender address also may be employed, as described
> in Section 2.4." — §3.4.2, `rfc4302.txt:986-989`.

- Strength: may (lower case). Class: internal.
- Check idea: send an AH packet on a multicast SA to an implementation that supports multicast
  and check that its SAD lookup uses the destination address, and, where configured, the sender
  address, in addition to the SPI.

### RFC4302-ISA-4

**The SAD entry for an SA indicates whether the Sequence Number field is checked and whether 32-bit or 64-bit sequence numbers are used.**

> "The SAD entry for the SA also indicates whether the Sequence Number field will be checked and
> whether 32- or 64-bit sequence numbers are employed for the SA." — §3.4.2,
> `rfc4302.txt:991-993`.

- Strength: description. Class: internal.
- Check idea: configure an SA's SAD entry with a sequence-number-checking setting and a 32-bit
  or 64-bit sequence-number choice and check that inbound processing follows that entry.

### RFC4302-ISA-5

**The SAD entry for an SA specifies the algorithm used for the ICV computation and the key needed to validate the ICV.**

> "The SAD entry for the SA also specifies the algorithm(s) employed for ICV computation, and
> indicates the key required to validate the ICV." — §3.4.2, `rfc4302.txt:993-995`.

- Strength: description. Class: internal.
- Check idea: configure an SA's SAD entry with an integrity algorithm and key and check that the
  receiver uses that algorithm and key to validate inbound ICVs on the SA.

### RFC4302-ISA-6

**If no valid SA exists for the packet, the receiver must discard it, and this is an auditable event.**

> "If no valid Security Association exists for this packet the receiver MUST discard the
> packet; this is an auditable event." — §3.4.2, `rfc4302.txt:997-998`.

- Strength: must. Class: end-to-end.
- Check idea: send an AH packet whose SPI matches no SAD entry and check that the receiver
  discards it and logs an audit event.

### RFC4302-ISA-7

**The audit log entry for a no-SA discard should include the SPI value, date/time, source address, destination address, and, in IPv6, the Flow ID.**

> "The audit log entry for this event SHOULD include the SPI value, date/time, Source Address,
> Destination Address, and (in IPv6) the Flow ID." — §3.4.2, `rfc4302.txt:998-1000`.

- Strength: should. Class: internal.
- Check idea: trigger the no-valid-SA discard event and check that the audit log entry carries
  the SPI value, date/time, source and destination addresses, and, for IPv6, the Flow ID.

### RFC4302-ISA-8

**SA-management traffic, such as IKE packets, need not be processed based on the SPI; it can be de-multiplexed separately, for example by the Next Protocol and Port fields.**

> "SA management traffic, such as IKE packets, does not need to be processed based on SPI, i.e.,
> one can de-multiplex this traffic separately based on Next Protocol and Port fields, for
> example." — §3.4.2, `rfc4302.txt:1002-1004`.

- Strength: description. Class: internal.
- Check idea: send SA-management traffic, such as IKE packets, to a receiver and check that it
  can de-multiplex that traffic by fields such as Next Protocol and Port, without an SPI-based
  SAD lookup.

## Sequence number verification

### RFC4302-ISEQ-1

**Every AH implementation must support the anti-replay service.**

> "All AH implementations MUST support the anti-replay service," — §3.4.3,
> `rfc4302.txt:1017`.

- Strength: must. Class: internal.
- Check idea: check that an AH implementation is able to run the anti-replay service, whether or
  not it is enabled for a given SA.

### RFC4302-ISEQ-2

**The receiver may enable or disable anti-replay on a per-SA basis.**

> "though its use may be enabled or disabled by the receiver on a per-SA basis." — §3.4.3,
> `rfc4302.txt:1017-1018`.

- Strength: may (lower case). Class: internal.
- Check idea: enable anti-replay on one SA and disable it on another at the same receiver and
  check that both configurations are accepted.

### RFC4302-ISEQ-3

**Anti-replay applies to both unicast and multicast SAs.**

> "Anti-replay is applicable to unicast as well as multicast SAs." — §3.4.3,
> `rfc4302.txt:1019`.

- Strength: description. Class: internal.
- Check idea: enable anti-replay on a unicast SA and separately on a multicast SA and check that
  the receiver runs the service on both.

### RFC4302-ISEQ-4

**This standard specifies no anti-replay mechanism for a multi-sender SA, unicast or multicast.**

> "However, this standard specifies no mechanisms for providing anti-" and "replay for a
> multi-sender SA (unicast or multicast)." — §3.4.3, `rfc4302.txt:1020`,
> `rfc4302.txt:1021`.

- Strength: description. Class: internal.
- Check idea: configure a multi-sender SA and check that no anti-replay mechanism from this
  standard is available for it.

### RFC4302-ISEQ-5

**Without a negotiated or manually configured anti-replay mechanism for a multi-sender SA, sequence-number checking by sender and receiver is recommended to be disabled.**

> "In the absence of negotiation (or manual configuration) of an anti-replay mechanism for such
> an SA, it is recommended that sender and receiver checking of the Sequence Number for the SA
> be disabled (via negotiation or manual configuration), as noted below." — §3.4.3,
> `rfc4302.txt:1021-1025`.

- Strength: should (lower case). Class: internal.
- Check idea: configure a multi-sender SA with no anti-replay mechanism negotiated and check
  that sender and receiver Sequence-Number checking is disabled for it.

### RFC4302-ISEQ-6

**If the receiver does not enable anti-replay for an SA, no inbound checks are performed on the Sequence Number.**

> "If the receiver does not enable anti-replay for an SA, no inbound checks are performed on the
> Sequence Number." — §3.4.3, `rfc4302.txt:1027-1028`.

- Strength: description. Class: end-to-end.
- Check idea: disable anti-replay on an SA and send packets with out-of-order or duplicate
  sequence numbers, and check that the receiver accepts them without a Sequence-Number check.

### RFC4302-ISEQ-7

**The sender's default assumption is that anti-replay is enabled at the receiver.**

> "However, from the perspective of the sender, the default is to assume that anti-replay is
> enabled at the receiver." — §3.4.3, `rfc4302.txt:1028-1030`.

- Strength: description. Class: internal.
- Check idea: establish an SA with no anti-replay notification from the receiver and check that
  the sender behaves as though anti-replay is enabled.

### RFC4302-ISEQ-8

**If an SA establishment protocol such as IKE is used, the receiver should notify the sender, during SA establishment, when it will not provide anti-replay protection.**

> "if an SA establishment protocol such as IKE is employed, the receiver SHOULD notify the
> sender, during SA establishment, if the receiver will not provide anti-replay protection." —
> §3.4.3, `rfc4302.txt:1032-1035`.

- Strength: should. Class: internal.
- Check idea: establish an SA through IKE with the receiver configured to skip anti-replay and
  check that it notifies the sender of this during SA establishment.

### RFC4302-ISEQ-9

**If the receiver has enabled anti-replay for an SA, the receive packet counter must be initialized to zero when the SA is established.**

> "If the receiver has enabled the anti-replay service for this SA, the receive packet counter
> for the SA MUST be initialized to zero when the SA is established." — §3.4.3,
> `rfc4302.txt:1037-1039`.

- Strength: must. Class: internal.
- Check idea: establish an SA with anti-replay enabled and check that the receiver's packet
  counter for that SA starts at zero.

### RFC4302-ISEQ-10

**For each received packet, the receiver must verify that its Sequence Number does not duplicate that of any other packet received during the SA's life.**

> "For each received packet, the receiver MUST verify that the packet contains a Sequence Number
> that does not duplicate the Sequence Number of any other packets received during the life of
> this SA." — §3.4.3, `rfc4302.txt:1039-1042`.

- Strength: must. Class: end-to-end.
- Check idea: replay a packet with a Sequence Number already received on an SA and check that
  the receiver detects and rejects the duplicate.

### RFC4302-ISEQ-11

**The duplicate-sequence-number check should be the first AH check applied to a packet after it is matched to an SA.**

> "This SHOULD be the first AH check applied to a packet after it has been matched to an SA, to
> speed rejection of duplicate packets." — §3.4.3, `rfc4302.txt:1042-1044`.

- Strength: should. Class: internal.
- Check idea: send a duplicate packet with an invalid ICV and check that the receiver rejects it
  for the sequence-number duplication before it performs ICV verification.

### RFC4302-ISEQ-12

**Duplicate packets are rejected using a sliding receive window; the window's implementation is a local matter, but it must exhibit the functionality described below.**

> "Duplicates are rejected through the use of a sliding receive window." and "How the window is
> implemented is a local matter, but the following text describes the functionality that the
> implementation must exhibit." — §3.4.3, `rfc4302.txt:1046`, `rfc4302.txt:1047-1049`.

- Strength: must (lower case). Class: internal.
- Check idea: check that a receiver rejects duplicate packets using a sliding receive window,
  whatever internal structure it uses to implement it.

### RFC4302-ISEQ-13

**The window's right edge is the highest validated Sequence Number received on the SA.**

> "The "right" edge of the window represents the highest, validated Sequence Number value
> received on this SA." — §3.4.3, `rfc4302.txt:1051-1052`.

- Strength: description. Class: internal.
- Check idea: send a run of validated packets on an SA and check that the window's right edge
  tracks the highest Sequence Number among them.

### RFC4302-ISEQ-14

**A packet with a sequence number lower than the window's left edge is rejected.**

> "Packets that contain sequence numbers lower than the "left" edge of the window are
> rejected." — §3.4.3, `rfc4302.txt:1052-1054`.

- Strength: description. Class: end-to-end.
- Check idea: send a packet whose Sequence Number is below the window's left edge and check that
  the receiver rejects it.

### RFC4302-ISEQ-15

**A packet whose sequence number falls within the window is checked against the list of packets already received in the window.**

> "Packets falling within the window are checked against a list of received packets within the
> window." — §3.4.3, `rfc4302.txt:1054-1055`.

- Strength: description. Class: internal.
- Check idea: send a packet whose Sequence Number falls inside the current window and check
  that the receiver compares it against the window's list of already-received packets.

### RFC4302-ISEQ-16

**If ESN is selected, only the low-order 32 bits are transmitted, but the receiver reconstructs the full sequence number from its local high-order-bits counter when checking the window.**

> "If the ESN option is selected for an SA, only the low-order 32 bits of the sequence number
> are explicitly transmitted, but the receiver employs the full sequence number computed using
> the high-order 32 bits for the indicated SA (from his local counter) when checking the
> received Sequence Number against the receive window." — §3.4.3, `rfc4302.txt:1057-1062`.

- Strength: description. Class: internal.
- Check idea: send a packet from an ESN-enabled SA and check that the receiver checks it against
  the window using the full 64-bit number it reconstructs from its own high-order-bits counter.

### RFC4302-ISEQ-17

**If the packet's low-order 32 bits are lower than the receiver's counter's low-order 32 bits, the receiver assumes the high-order bits have incremented, moving to a new sequence-number subspace.**

> "In constructing the full sequence number, if the low-order 32 bits carried in the" and
> "packet are lower in value than the low-order 32 bits of the receiver's sequence number
> counter, the receiver assumes that the high-order 32 bits have been incremented, moving to a
> new sequence number subspace." — §3.4.3, `rfc4302.txt:1061-1062`, `rfc4302.txt:1071-1074`.

- Strength: description. Class: internal.
- Check idea: send an ESN packet whose low-order 32 bits are numerically lower than the
  receiver's stored low-order 32 bits and check that the receiver reconstructs the full sequence
  number as though the high-order bits had incremented.

### RFC4302-ISEQ-18

**This reconstruction algorithm tolerates a reception gap on a single SA as large as 2^32-1 packets.**

> "This algorithm accommodates gaps in reception for a single SA as large as 2**32-1 packets." —
> §3.4.3, `rfc4302.txt:1074-1075`.

- Strength: description. Class: internal.
- Check idea: create a reception gap on an SA approaching 2^32-1 packets and check that the
  receiver still reconstructs the full sequence number correctly.

### RFC4302-ISEQ-19

**If a larger gap occurs, the receiver may apply additional heuristic checks to re-synchronize its sequence number counter.**

> "If a larger gap occurs, additional, heuristic checks for re-synchronization of the receiver's
> sequence number counter MAY be employed, as described in Appendix B.)" — §3.4.3,
> `rfc4302.txt:1075-1077`.

- Strength: may. Class: internal.
- Check idea: create a reception gap larger than 2^32-1 packets and check whether the receiver
  applies a heuristic re-synchronization check instead of rejecting all further packets.

### RFC4302-ISEQ-20

**A packet within the window and not a duplicate, or to the right of the window, proceeds to ICV verification.**

> "If the received packet falls within the window and is not a duplicate, or if the packet is to
> the right of the window, then the receiver proceeds to ICV verification." — §3.4.3,
> `rfc4302.txt:1079-1081`.

- Strength: description. Class: internal.
- Check idea: send a non-duplicate packet within the window, and separately a packet to the
  right of the window, and check that the receiver proceeds to ICV verification in both cases.

### RFC4302-ISEQ-21

**If ICV validation fails, the receiver must discard the received IP datagram as invalid, and this is an auditable event.**

> "If the ICV validation fails, the receiver MUST discard the received IP datagram as invalid.
> This is an auditable event." — §3.4.3, `rfc4302.txt:1081-1083`.

- Strength: must. Class: end-to-end.
- Check idea: send a packet with an invalid ICV that passes the sequence-number check and check
  that the receiver discards it and logs an audit event.

### RFC4302-ISEQ-22

**The audit log entry for an ICV-verification-failure discard should include the SPI value, date/time, source address, destination address, the sequence number, and, in IPv6, the Flow ID.**

> "The audit log entry for this event SHOULD include the SPI value, date/time, Source Address,
> Destination Address, the Sequence Number, and (in IPv6) the Flow ID." — §3.4.3,
> `rfc4302.txt:1083-1086`.

- Strength: should. Class: internal.
- Check idea: trigger an ICV-verification-failure discard and check that the audit log entry
  carries the SPI value, date/time, source and destination addresses, the sequence number, and,
  for IPv6, the Flow ID.

### RFC4302-ISEQ-23

**The receive window is updated only if the ICV verification succeeds.**

> "The receive window is updated only if the ICV verification succeeds." — §3.4.3,
> `rfc4302.txt:1085-1086`.

- Strength: description. Class: internal.
- Check idea: send a packet whose ICV verification fails and check that the receive window is
  left unchanged.

### RFC4302-ISEQ-24

**A minimum receive-window size of 32 packets must be supported.**

> "A MINIMUM window size of 32 packets MUST be supported," — §3.4.3, `rfc4302.txt:1088`.

- Strength: must. Class: internal.
- Check idea: check that a receiver supports a receive window of at least 32 packets.

### RFC4302-ISEQ-25

**A window size of 64 is preferred and should be used as the default.**

> "but a window size of 64 is preferred and SHOULD be employed as the default." — §3.4.3,
> `rfc4302.txt:1088-1089`.

- Strength: should. Class: internal.
- Check idea: check the default receive-window size of an unconfigured receiver and check that
  it is 64 packets.

### RFC4302-ISEQ-26

**The receiver may choose another window size, larger than the minimum.**

> "Another window size (larger than the MINIMUM) MAY be chosen by the receiver." — §3.4.3,
> `rfc4302.txt:1090-1091`.

- Strength: may. Class: internal.
- Check idea: configure a receiver with a window size larger than 32 packets and check that it
  is accepted.

### RFC4302-ISEQ-27

**The receiver does not notify the sender of the window size.**

> "(The receiver does NOT notify the sender of the window size.)" — §3.4.3,
> `rfc4302.txt:1091-1092`.

- Strength: description. Class: wire.
- Check idea: capture the traffic exchanged while establishing and running an SA and check that
  no message carries the receiver's window size to the sender.

### RFC4302-ISEQ-28

**The receive window size should be increased for higher-speed environments, irrespective of assurance issues.**

> "The receive window size should be increased for higher-speed environments, irrespective of
> assurance issues." — §3.4.3, `rfc4302.txt:1092-1093`.

- Strength: should (lower case). Class: internal.
- Check idea: deploy a receiver in a higher-speed environment and check that its configured
  receive-window size is larger than the default.

### RFC4302-ISEQ-29

**This standard does not specify minimum or recommended receive-window sizes for very-high-speed devices.**

> "Values for minimum and recommended receive window sizes for very high-speed (e.g.,
> multi-gigabit/second) devices are not specified by this standard." — §3.4.3,
> `rfc4302.txt:1093-1095`.

- Strength: description. Class: internal.
- Check idea: check that no minimum or recommended receive-window size for very-high-speed
  devices is asserted as a requirement of this standard.

## ICV verification

### RFC4302-IICV-1

**The receiver computes the ICV over the appropriate fields of the packet, using the specified integrity algorithm, and checks it against the ICV carried in the packet.**

> "The receiver computes the ICV over the appropriate fields of the packet, using the specified
> integrity algorithm, and verifies that it is the same as the ICV included in the ICV field of
> the packet." — §3.4.4, `rfc4302.txt:1099-1101`.

- Strength: description. Class: end-to-end.
- Check idea: send a valid AH packet to a receiver and check that it recomputes the ICV over the
  appropriate fields and compares it with the packet's ICV field.

### RFC4302-IICV-2

**If the computed and received ICVs match, the datagram is valid and is accepted.**

> "If the computed and received ICVs match, then the datagram is valid, and it is accepted." —
> §3.4.4, `rfc4302.txt:1104-1105`.

- Strength: description. Class: end-to-end.
- Check idea: send a valid AH packet and check that the receiver accepts and delivers it once
  the computed and received ICVs match.

### RFC4302-IICV-3

**If the ICVs do not match, the receiver must discard the received IP datagram as invalid, and this is an auditable event.**

> "If the test fails, then the receiver MUST discard the received IP datagram as invalid. This
> is an auditable event." — §3.4.4, `rfc4302.txt:1105-1107`.

- Strength: must. Class: end-to-end.
- Check idea: send an AH packet with a tampered ICV and check that the receiver discards it and
  logs an audit event.

### RFC4302-IICV-4

**The audit log entry for an ICV-mismatch discard should include the SPI value, date/time received, source address, destination address, and, in IPv6, the Flow ID.**

> "The audit log entry SHOULD include the SPI value, date/time received, Source Address,
> Destination Address, and (in IPv6) the Flow ID." — §3.4.4, `rfc4302.txt:1107-1109`.

- Strength: should. Class: internal.
- Check idea: trigger an ICV-mismatch discard and check that the audit log entry carries the SPI
  value, date/time received, source and destination addresses, and, for IPv6, the Flow ID.

### RFC4302-IICV-5

**An implementation can use any set of steps that produces the same result as the steps described below.**

> "Implementations can use any set of steps that results in the same result as the following set
> of steps." — §3.4.4, `rfc4302.txt:1113-1114`.

- Strength: description. Class: internal.
- Check idea: check that a receiver's ICV verification, however it is implemented internally,
  produces the same accept/discard outcome as the described step sequence.

### RFC4302-IICV-6

**The receiver's verification procedure saves the received ICV, replaces it, but not any ICV field padding, with zero, and zeroes every other field that may have changed in transit.**

> "Begin by saving the ICV value and replacing it (but not any ICV field padding) with zero." and
> "Zero all other fields that may have been modified during transit." — §3.4.4,
> `rfc4302.txt:1114-1115`, `rfc4302.txt:1116`.

- Strength: may (lower case). Class: encoding.
- Check idea: check that a receiver's ICV verification saves the packet's received ICV, zeroes
  the ICV field (leaving any ICV padding untouched), and zeroes every other field that may have
  changed in transit, before recomputing the ICV.

### RFC4302-IICV-7

**If ESN is elected for the SA, the receiver appends the sequence number's high-order 32 bits after the end of the packet before the verification ICV computation.**

> "If the ESN option is elected for this SA, append the high-order 32 bits of the ESN after the
> end of the packet." — §3.4.4, `rfc4302.txt:1127-1128`.

- Strength: description. Class: encoding.
- Check idea: verify a packet from an ESN-enabled SA and check that the receiver appends the
  high-order 32 bits of its sequence-number counter after the packet's end before computing the
  ICV.

### RFC4302-IICV-8

**The receiver checks the packet's overall length and, if the integrity algorithm requires it, appends zero-filled implicit padding to the end of the packet, after the ESN if present.**

> "Check the overall length of the packet (as described above), and if it requires implicit
> padding based on the requirements of the integrity algorithm, append zero-filled bytes to the
> end of the packet (after the ESN if present) as required." — §3.4.4,
> `rfc4302.txt:1128-1132`.

- Strength: description. Class: encoding.
- Check idea: verify a packet whose length does not match the integrity algorithm's blocksize
  and check that the receiver appends zero-filled implicit padding, after the ESN if present,
  before computing the ICV.

### RFC4302-IICV-9

**The receiver performs the ICV computation and compares the result with the saved value, using the algorithm specification's comparison rules.**

> "Perform the ICV computation and compare the result with the saved value, using the comparison
> rules defined by the algorithm specification." — §3.4.4, `rfc4302.txt:1132-1134`.

- Strength: description. Class: internal.
- Check idea: check that a receiver's final accept/discard decision follows the comparison rules
  the integrity algorithm's specification defines, applied to the recomputed and saved ICV
  values.

## Conformance requirements

### RFC4302-CONF-1

**An implementation that claims conformance must fully implement the AH syntax and processing described in this specification for unicast traffic.**

> "Implementations that claim conformance or compliance with this specification MUST fully
> implement the AH syntax and processing described here for unicast traffic," — §5,
> `rfc4302.txt:1157-1159`.

- Strength: must. Class: internal.
- Check idea: check that an implementation claiming conformance implements every AH syntax and
  processing rule of this specification for unicast traffic.

### RFC4302-CONF-2

**A conformant implementation must also comply with all requirements of the Security Architecture document.**

> "and MUST comply with all requirements of the Security Architecture document [Ken-Arch]." —
> §5, `rfc4302.txt:1159-1160`.

- Strength: must. Class: internal.
- Check idea: check that an implementation claiming conformance also meets the requirements of
  the Security Architecture document.

### RFC4302-CONF-3

**An implementation that claims to support multicast traffic must comply with the additional requirements specified for that support.**

> "Additionally, if an implementation claims to support multicast traffic, it MUST comply with
> the additional requirements specified for support of such traffic." — §5,
> `rfc4302.txt:1161-1163`.

- Strength: must. Class: internal.
- Check idea: check that an implementation claiming multicast support meets the additional
  multicast-specific requirements, beyond the unicast conformance requirements.

### RFC4302-CONF-4

**If the ICV key is manually distributed, correct anti-replay provision needs correct sender counter-state maintenance until the key is replaced, with likely no automated recovery if counter overflow is imminent.**

> "If the key used to compute an ICV is manually distributed, correct provision of the
> anti-replay service would require correct maintenance of the counter state at the sender,
> until the key is replaced, and there likely would be no automated recovery provision if
> counter overflow were imminent." — §5, `rfc4302.txt:1163-1167`.

- Strength: description. Class: internal.
- Check idea: configure a manually keyed SA and check that no automated recovery exists if the
  sender's sequence counter approaches overflow before the key is replaced.

### RFC4302-CONF-5

**A compliant implementation should not provide the anti-replay service on manually keyed SAs.**

> "Thus, a compliant implementation SHOULD NOT provide this service in conjunction with SAs that
> are manually keyed." — §5, `rfc4302.txt:1167-1169`.

- Strength: should not. Class: internal.
- Check idea: configure a manually keyed SA and check that the implementation does not offer the
  anti-replay service for it.

### RFC4302-CONF-6

**The mandatory-to-implement algorithms for AH are defined in a separate RFC, kept independent of this protocol specification.**

> "The mandatory-to-implement algorithms for use with AH are described in a separate RFC
> [Eas04], to facilitate updating the algorithm requirements independently from the protocol per
> se." — §5, `rfc4302.txt:1171-1173`.

- Strength: description. Class: internal.
- Check idea: check that the set of mandatory-to-implement AH integrity algorithms an
  implementation supports matches the separate algorithm-requirements RFC, not this
  specification.

### RFC4302-CONF-7

**An implementation may support additional algorithms beyond those mandated for AH.**

> "Additional algorithms, beyond those mandated for AH, MAY be supported." — §5,
> `rfc4302.txt:1173-1174`.

- Strength: may. Class: internal.
- Check idea: configure an SA with an integrity algorithm beyond the mandatory-to-implement set
  and check that the implementation accepts it.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§2 to §3.4 and §5, and every field of every header. What the catalog leaves out:

- **The sections outside the in-scope set**: §1 (introduction), §4 (auditing), §6 to §9
  (security, the changes from RFC 2402, references) and the appendices (the mutability of the
  IP options, the 64-bit sequence numbers).

Within the in-scope sections, these parts have no entry, with the reason:

In §2 to §3.4, and §5:

- Lines 186-187: pointer sentence introducing Figure 1 ("Figure 1 illustrates the format for
  AH."); no independent checkable content.
- Lines 205-238: the summary table of AH fields (size, mandatory flag, ICV coverage,
  transmission); an informative preview whose content is covered by RFC4302-FMT-3 and by the
  per-field areas (NH, LEN, RSV, SPI, SEQ, ICV) and by OICV/OSEQ.
- Lines 244-247: the byte-order note ("cryptographic algorithms...expect...network byte order");
  a generic convention shared by all IPsec and IP processing, not an AH-specific field mandate.
- Lines 267-269: the illustrative "for example" computation of the Payload Length for a 96-bit
  ICV; an example of the formula already captured by RFC4302-LEN-1.
- Lines 271-275: parenthetical explaining that AH's length unit differs from other IPv6
  extension headers, plus a cross-reference to §2.6 and §3.3.3.2.1; informative elaboration, no
  independent requirement.
- Lines 312-318: informative background on how a Group Controller/Key Server unilaterally
  assigns a multicast SPI; describes key-management-protocol behavior, not an AH requirement.
- Lines 363-371: illustrative examples of accelerated SAD search implementation (hash table,
  TCAM); examples of the general rule in RFC4302-SPI-16, not separate requirements.
- Lines 388-390, 399-401: illustrative example of a key-management implementation using the
  zero SPI value to mean "No Security Association Exists"; describes key-management behavior.
- Lines 439-441: the IKEv2-specific default for ESN negotiation; describes IKEv2's own
  negotiation semantics, not a requirement on AH traffic.
- Lines 471-472: cross-reference to §3.3.3.2, "Padding", for the padding-length computation.
- Lines 490-492: terminology clarification that "transport" mode is not restricted to TCP and
  UDP; no independent checkable requirement.
- Lines 547-549: cross-reference to the Security Architecture document for the AH/ESP
  combinations that must be supported; the substantive combination rules belong to that
  document.
- Lines 594-596: footnote cross-referencing the Security Architecture document for how the
  outer IP header/extensions are constructed and the inner ones modified in tunnel mode.
- Lines 605-607: informative note that multicast integrity strategies are an area of ongoing
  research; no checkable statement.
- Lines 623-625 (from "The construction..."): cross-reference to the Security Architecture
  document for outer IP header/extension construction during encapsulation.
- Lines 631-633 (from "The process of determining..."): cross-reference to the Security
  Architecture document for how outbound IPsec processing is determined.
- Lines 708-713 (from "Note that by replacing..."): rationale for the zero-fill technique
  (alignment preservation, fixed field length); explanation, not a separate requirement.
- Lines 715-718: a SHOULD directed at the authors of future extension-header/option RFCs (to
  document AH-ICV handling in their Security Considerations section), not a requirement on an
  IPsec implementation.
- Lines 763-772, 778-780, 791-793: rationale explaining why DSCP, ECN, Flags, TTL, and Header
  Checksum are classified as mutable; the classification itself is captured in RFC4302-OICV-13.
- Lines 827-830: historical/compatibility footnote on why the IPv6 Flow Label, mutable in AHv1,
  is excluded from the ICV in AHv2; explanation, not a separate requirement.
- Line 850: cross-reference to the IPv6 specification for more information on option handling.
- Lines 870-873: illustrative "for example" case where a 96-bit ICV needs no padding; example
  of the rule already captured in RFC4302-OICV-25.
- Lines 938-941: rationale that fragmentation reduces performance and that receiver-side
  reassembly creates denial-of-service vulnerabilities; motivates RFC4302-FRAG-8/FRAG-9 but adds
  no separate checkable statement.
- Lines 947-948: cross-reference to the Security Architecture document for MTU-management
  support details.
- Lines 989-991 (from "This process is described..."): cross-reference to the Security
  Architecture document for inbound SA-determination detail.
- Lines 1117-1118 (from "(See Section 3.3.3.1..."): cross-reference to §3.3.3.1 for which fields
  are zeroed; the classification itself is catalogued under OICV.
- Lines 1134-1136: illustrative parenthetical that a digital-signature/one-way-hash ICV makes the
  comparison process "more complex"; example, no separate checkable statement.
