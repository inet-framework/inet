# RFC 4303 (IP Encapsulating Security Payload (ESP)) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC4303-*` · **Stands on:** [standards.md](../../protocol/ipsec/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 4303, IP Encapsulating Security Payload (ESP), of
December 2005, a document of the in-scope set of IPsec. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc4303.txt`](../../../../../../standards/RFC/rfc4303.txt) —
  IP Encapsulating Security Payload (ESP), December 2005. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc4303.txt>.

The architecture is in [`rfc4301/catalog.md`](../rfc4301/catalog.md), and AH, the other
traffic protocol, in [`rfc4302/catalog.md`](../rfc4302/catalog.md). The in-scope sections are
§2 to §3.4 and §5.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/ipsec/standards.md). The features these statements build are in
[`features.md`](../../protocol/ipsec/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`ipsec/coverage.md`](../../model/ipsec/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc4303.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC4303-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 4303 uses the keywords of RFC 2119 in capitals;
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
| [RFC4303-FMT-1](#rfc4303-fmt-1) | The header that immediately precedes ESP carries protocol number 50. |
| [RFC4303-FMT-2](#rfc4303-fmt-2) | The ESP packet places its fields in the order SPI, Sequence Number, Payload Data, Padding, Pad Length, Next Header, and, if present, ICV. |
| [RFC4303-FMT-3](#rfc4303-fmt-3) | The transmitted ESP trailer is the Padding, Pad Length, and Next Header fields. |
| [RFC4303-FMT-4](#rfc4303-fmt-4) | Implicit ESP trailer data that is not transmitted is included in the integrity computation. |
| [RFC4303-FMT-5](#rfc4303-fmt-5) | When the integrity service is selected, the integrity computation covers the SPI, the Sequence Number, the Payload Data, and the ESP trailer. |
| [RFC4303-FMT-6](#rfc4303-fmt-6) | When the confidentiality service is selected, the ciphertext is the Payload Data, apart from any cryptographic synchronization data, and the explicit ESP trailer. |
| [RFC4303-FMT-7](#rfc4303-fmt-7) | The SPI and Sequence Number fields are not encrypted, but are given integrity whenever the integrity service is selected, regardless of the algorithm mode. |
| [RFC4303-FMT-8](#rfc4303-fmt-8) | For a combined mode algorithm, the explicit ICV field that would otherwise close the packet may be left out. |
| [RFC4303-FMT-9](#rfc4303-fmt-9) | When the ICV is left out under a combined mode algorithm, that algorithm encodes an ICV-equivalent means of verifying integrity within the Payload Data. |
| [RFC4303-FMT-10](#rfc4303-fmt-10) | When a combined mode algorithm gives integrity only to the data it encrypts, the SPI and Sequence Number are replicated inside the Payload Data. |
| [RFC4303-FMT-11](#rfc4303-fmt-11) | Traffic flow confidentiality padding, when used, is inserted after the Payload Data and before the ESP trailer. |
| [RFC4303-FMT-12](#rfc4303-fmt-12) | In tunnel mode, an IPsec implementation may add TFC padding after the Payload Data and before the Padding field. |
| [RFC4303-FMT-13](#rfc4303-fmt-13) | The detailed format of ESP packets, including the Payload Data substructure, is fixed for all traffic on a given SA. |
| [RFC4303-FMT-14](#rfc4303-fmt-14) | An optional field is present in neither the transmitted packet nor the data formatted for the ICV computation when its option is not selected. |
| [RFC4303-FMT-15](#rfc4303-fmt-15) | The format of ESP packets for a given SA is fixed for the duration of that SA. |
| [RFC4303-FMT-16](#rfc4303-fmt-16) | Mandatory ESP fields are always present in the packet format, for every SA. |
| [RFC4303-FMT-17](#rfc4303-fmt-17) | Cryptographic algorithms used in IPsec take input, and produce output, in network byte order, the same order in which IP packets are transmitted. |
| [RFC4303-FMT-18](#rfc4303-fmt-18) | Because ESP has no version number, a signaling or configuration mechanism between the two IPsec peers must resolve any backward-compatibility concern. |
| [RFC4303-SPI-1](#rfc4303-spi-1) | The SPI is an arbitrary 32-bit value a receiver uses to identify the SA of an incoming packet. |
| [RFC4303-SPI-2](#rfc4303-spi-2) | The SPI field is mandatory. |
| [RFC4303-SPI-3](#rfc4303-spi-3) | For a unicast SA, the receiver generates the SPI value. |
| [RFC4303-SPI-4](#rfc4303-spi-4) | An ESP implementation must support mapping inbound unicast traffic to an SA using the SPI, alone or together with the IPsec protocol type. |
| [RFC4303-SPI-5](#rfc4303-spi-5) | An IPsec implementation that supports multicast must support multicast SAs, using the inbound de-multiplexing algorithm given for the SPI. |
| [RFC4303-SPI-6](#rfc4303-spi-6) | An implementation that supports only unicast traffic need not implement the multicast SPI de-multiplexing algorithm. |
| [RFC4303-SPI-7](#rfc4303-spi-7) | A multicast-capable IPsec implementation must correctly de-multiplex inbound traffic even when a group SA and a unicast SA use the same SPI. |
| [RFC4303-SPI-8](#rfc4303-spi-8) | Each SAD entry indicates whether the SA lookup for it uses the destination address alone, or the destination and source addresses, together with the SPI. |
| [RFC4303-SPI-9](#rfc4303-spi-9) | For multicast SAs, the protocol field is not used for SA lookups. |
| [RFC4303-SPI-10](#rfc4303-spi-10) | For every inbound IPsec-protected packet, an implementation searches the SAD for the entry that matches the longest SA identifier: the entry that, among those matching the SPI, also matches on destination, or destination and source, address. |
| [RFC4303-SPI-11](#rfc4303-spi-11) | The receiver searches the SAD first for a match on SPI, destination, and source address; otherwise it processes the packet with that entry. |
| [RFC4303-SPI-12](#rfc4303-spi-12) | Failing a match on SPI, destination, and source, the receiver searches the SAD for a match on SPI and destination address only; otherwise it processes the packet with that entry. |
| [RFC4303-SPI-13](#rfc4303-spi-13) | Failing the first two searches, the receiver searches the SAD for a match on SPI alone, or on SPI and protocol, depending on whether it keeps one SPI space for AH and ESP; if no entry matches, it discards the packet and logs an auditable event. |
| [RFC4303-SPI-14](#rfc4303-spi-14) | An implementation may use any method to search the SAD, but its externally visible behavior must be equivalent to searching the SAD in the longest-match order given. |
| [RFC4303-SPI-15](#rfc4303-spi-15) | Whether inbound traffic mapping to SAs requires source-and-destination address matching must be set by manual SA configuration or by negotiation through an SA management protocol. |
| [RFC4303-SPI-16](#rfc4303-spi-16) | A Source-Specific Multicast group typically uses a 3-tuple SA identifier of SPI, destination multicast address, and source address. |
| [RFC4303-SPI-17](#rfc4303-spi-17) | An Any-Source Multicast group SA needs only an SPI and a destination multicast address as its identifier. |
| [RFC4303-SPI-18](#rfc4303-spi-18) | The SPI value zero is reserved for local, implementation-specific use and must not be sent on the wire. |
| [RFC4303-SEQ-1](#rfc4303-seq-1) | The Sequence Number field is an unsigned 32-bit per-SA counter that increases by one for each packet sent. |
| [RFC4303-SEQ-2](#rfc4303-seq-2) | For a unicast SA or a single-sender multicast SA, the sender must increment the Sequence Number for every transmitted packet. |
| [RFC4303-SEQ-3](#rfc4303-seq-3) | Sharing an SA among multiple senders is permitted, though not generally recommended. |
| [RFC4303-SEQ-4](#rfc4303-seq-4) | ESP's anti-replay features are not available for a multi-sender SA, because ESP has no way to synchronize packet counters, or meaningfully manage a receiver window, among multiple senders. |
| [RFC4303-SEQ-5](#rfc4303-seq-5) | The Sequence Number field must always be present and transmitted, even when the receiver has not enabled anti-replay for the SA. |
| [RFC4303-SEQ-6](#rfc4303-seq-6) | All ESP implementations must be capable of the sequence-number generation and verification processing, even though the receiver decides whether to act on it for a given SA. |
| [RFC4303-SEQ-7](#rfc4303-seq-7) | The sender's and receiver's sequence counters are initialized to zero when an SA is established, so the first packet sent on the SA carries sequence number 1. |
| [RFC4303-SEQ-8](#rfc4303-seq-8) | Anti-replay is enabled by default. |
| [RFC4303-SEQ-9](#rfc4303-seq-9) | When anti-replay is enabled, the sender's and receiver's counters must be reset, by establishing a new SA and a new key, before the sequence number would cycle at the 2^32nd packet. |
| [RFC4303-SEQ-10](#rfc4303-seq-10) | An IPsec implementation should implement Extended Sequence Numbers as an extension to the 32-bit Sequence Number field. |
| [RFC4303-SEQ-11](#rfc4303-seq-11) | Use of an Extended Sequence Number must be negotiated by an SA management protocol. |
| [RFC4303-SEQ-12](#rfc4303-seq-12) | The Extended Sequence Number feature applies to multicast SAs as well as unicast SAs. |
| [RFC4303-SEQ-13](#rfc4303-seq-13) | Only the low-order 32 bits of an extended sequence number are transmitted, in the plaintext ESP header of each packet. |
| [RFC4303-SEQ-14](#rfc4303-seq-14) | The high-order 32 bits of an extended sequence number are kept by both sender and receiver and included in the ICV computation when the integrity service is selected. |
| [RFC4303-SEQ-15](#rfc4303-seq-15) | With a separate integrity algorithm, the extended sequence number's high-order bits are part of the implicit ESP trailer and are not transmitted. |
| [RFC4303-SEQ-16](#rfc4303-seq-16) | With a combined mode algorithm, whether the extended sequence number's high-order bits are transmitted or included only implicitly in the computation depends on the algorithm's choice. |
| [RFC4303-PAY-1](#rfc4303-pay-1) | The Payload Data field is a variable-length field carrying the data identified by the Next Header field. |
| [RFC4303-PAY-2](#rfc4303-pay-2) | The Payload Data field is mandatory and an integral number of bytes long. |
| [RFC4303-PAY-3](#rfc4303-pay-3) | When an encryption algorithm needs explicit per-packet synchronization data, such as an IV, that data is carried inside the Payload Data field rather than as a separate ESP field, and typically immediately precedes the ciphertext. |
| [RFC4303-PAY-4](#rfc4303-pay-4) | For IPv4, the next layer protocol header must begin at an offset that is a multiple of 4 bytes from the start of the ESP header. |
| [RFC4303-PAY-5](#rfc4303-pay-5) | For IPv6, the next layer protocol header must begin at an offset that is a multiple of 8 bytes from the start of the ESP header. |
| [RFC4303-PAD-1](#rfc4303-pad-1) | When an encryption algorithm requires the plaintext to be a multiple of a block size, the Padding field fills the plaintext, consisting of the Payload Data, Padding, Pad Length, and Next Header fields, to the size the algorithm needs. |
| [RFC4303-PAD-2](#rfc4303-pad-2) | The Pad Length and Next Header fields are right-aligned within a 4-byte word, so the ICV field, when present, is aligned on a 4-byte boundary. |
| [RFC4303-PAD-3](#rfc4303-pad-3) | The Padding field is too limited to be effective for traffic flow confidentiality and should not be used for that purpose; the mechanism of Traffic Flow Confidentiality padding should be used when TFC is required. |
| [RFC4303-PAD-4](#rfc4303-pad-4) | The sender may add 0 to 255 bytes of Padding. |
| [RFC4303-PAD-5](#rfc4303-pad-5) | All implementations must support generating and consuming the Padding field, even though including it is optional. |
| [RFC4303-PAD-6](#rfc4303-pad-6) | For the block-size padding purpose, the padding computation covers the Payload Data excluding any IV, but including the ESP trailer fields. |
| [RFC4303-PAD-7](#rfc4303-pad-7) | When a combined mode algorithm replicates the SPI, Sequence Number, or the high-order Extended Sequence Number bits inside the Payload Data for integrity, those replicated items are included in the pad-length computation. |
| [RFC4303-PAD-8](#rfc4303-pad-8) | For the ICV-alignment padding purpose, the padding computation covers the Payload Data including the IV, the Pad Length, and Next Header fields, plus any combined-mode replicated or ICV-equivalent data. |
| [RFC4303-PAD-9](#rfc4303-pad-9) | When the encryption algorithm does not specify the padding contents, the sender must use the default processing: Padding bytes form a monotonically increasing integer sequence starting at 1. |
| [RFC4303-PAD-10](#rfc4303-pad-10) | When the default padding scheme is used, the receiver should inspect the Padding field. |
| [RFC4303-PADL-1](#rfc4303-padl-1) | The Pad Length field states the number of pad bytes immediately preceding it, a value from 0 to 255, where zero means no Padding bytes are present. |
| [RFC4303-PADL-2](#rfc4303-padl-2) | The Pad Length count does not include any TFC padding bytes. |
| [RFC4303-PADL-3](#rfc4303-padl-3) | The Pad Length field is mandatory. |
| [RFC4303-NH-1](#rfc4303-nh-1) | The Next Header field is a mandatory 8-bit field that identifies the type of data in the Payload Data field, using the IANA IP Protocol Numbers. |
| [RFC4303-NH-2](#rfc4303-nh-2) | Protocol value 59, "no next header," must be used to mark a dummy packet. |
| [RFC4303-NH-3](#rfc4303-nh-3) | A transmitter must be capable of generating dummy packets marked with Next Header value 59. |
| [RFC4303-NH-4](#rfc4303-nh-4) | A receiver must be prepared to discard a dummy packet without indicating an error. |
| [RFC4303-NH-5](#rfc4303-nh-5) | All other ESP header and trailer fields must be present in a dummy packet, even though the plaintext payload other than the Next Header field need not be well-formed. |
| [RFC4303-NH-6](#rfc4303-nh-6) | Implementations should provide local management controls, including parametric controls, to enable dummy-packet generation on a per-SA basis. |
| [RFC4303-TFC-1](#rfc4303-tfc-1) | An optional field within the Payload Data addresses the traffic flow confidentiality requirement, because the 255-byte Padding field is not adequate for it. |
| [RFC4303-TFC-2](#rfc4303-tfc-2) | An IPsec implementation should be capable of adding TFC padding bytes after the end of the Payload Data and before the Padding field. |
| [RFC4303-TFC-3](#rfc4303-tfc-3) | TFC padding can be added only if the Payload Data field carries a specification of the IP datagram's length, which always holds in tunnel mode and may hold in transport mode depending on the next layer protocol. |
| [RFC4303-TFC-4](#rfc4303-tfc-4) | The receiver locates the ESP trailer fields by counting back from the end of the ESP packet, and uses the IP datagram's known length to discard TFC padding. |
| [RFC4303-TFC-5](#rfc4303-tfc-5) | When TFC padding is added, the field carrying the IP datagram's length must not be modified to reflect the padding. |
| [RFC4303-TFC-6](#rfc4303-tfc-6) | No requirement is established for the value of the TFC padding bytes. |
| [RFC4303-TFC-7](#rfc4303-tfc-7) | An SA management protocol must negotiate use of TFC padding before a transmitter employs it, to keep backward compatibility. |
| [RFC4303-TFC-8](#rfc4303-tfc-8) | Implementations should provide local management controls, including parametric controls, to enable TFC padding on a per-SA basis. |
| [RFC4303-ICV-1](#rfc4303-icv-1) | The ICV is a variable-length field computed over the ESP header, the Payload, and the ESP trailer fields. |
| [RFC4303-ICV-2](#rfc4303-icv-2) | Implicit ESP trailer fields, integrity-algorithm padding and high-order Extended Sequence Number bits when applicable, are included in the ICV computation. |
| [RFC4303-ICV-3](#rfc4303-icv-3) | The ICV field is optional: present only when the integrity service is selected, and provided by a separate integrity algorithm or by a combined mode algorithm that uses an ICV. |
| [RFC4303-ICV-4](#rfc4303-icv-4) | The length of the ICV field is set by the integrity algorithm associated with the SA. |
| [RFC4303-LOC-1](#rfc4303-loc-1) | ESP operates in one of two modes: transport mode or tunnel mode. |
| [RFC4303-LOC-2](#rfc4303-loc-2) | In transport mode, ESP is inserted after the IP header and before the next layer protocol header. |
| [RFC4303-LOC-3](#rfc4303-loc-3) | In IPv4 transport mode, ESP is placed after the IP header, including any options it contains, and before the next layer protocol. |
| [RFC4303-LOC-4](#rfc4303-loc-4) | When AH is also applied to a transport-mode packet, AH covers the ESP header, Payload, ESP trailer, and the ICV if present. |
| [RFC4303-LOC-5](#rfc4303-loc-5) | In transport mode, encryption covers the next layer protocol header and data plus the ESP trailer; integrity covers the ESP header, the next layer protocol header and data, and the ESP trailer, but not the ICV itself. |
| [RFC4303-LOC-6](#rfc4303-loc-6) | In IPv6, ESP is placed after the hop-by-hop, routing, and fragmentation extension headers. |
| [RFC4303-LOC-7](#rfc4303-loc-7) | A destination options header may appear before the ESP header, after it, or both, but placing it after ESP is generally desirable, since ESP protects only what follows the ESP header. |
| [RFC4303-LOC-8](#rfc4303-loc-8) | In transport mode, a bump-in-the-stack or bump-in-the-wire implementation may need extra IP reassembly and fragmentation of inbound and outbound fragments, to conform to this specification while staying transparent. |
| [RFC4303-LOC-9](#rfc4303-loc-9) | In tunnel mode, the inner IP header carries the ultimate source and destination addresses, and the outer IP header carries the addresses of the IPsec peers. |
| [RFC4303-LOC-10](#rfc4303-loc-10) | Tunnel mode allows mixed inner and outer IP versions: IPv6 over IPv4, and IPv4 over IPv6. |
| [RFC4303-LOC-11](#rfc4303-loc-11) | In tunnel mode, ESP protects the entire inner IP packet, including the entire inner IP header. |
| [RFC4303-LOC-12](#rfc4303-loc-12) | The position of ESP in tunnel mode, relative to the outer IP header, is the same as in transport mode. |
| [RFC4303-LOC-13](#rfc4303-loc-13) | In tunnel mode, encryption covers the entire inner IP packet plus the ESP trailer; integrity covers the ESP header, the entire inner IP packet, and the ESP trailer, but not the ICV itself. |
| [RFC4303-ALG-1](#rfc4303-alg-1) | An ESP implementation may support algorithms beyond those mandated for ESP. |
| [RFC4303-ALG-2](#rfc4303-alg-2) | An SA must select at least one of confidentiality or integrity, so its encryption and integrity algorithms are never both NULL at the same time. |
| [RFC4303-ALG-3](#rfc4303-alg-3) | The SA that carries a packet specifies the encryption algorithm that protects that packet. |
| [RFC4303-ALG-4](#rfc4303-alg-4) | Each ESP packet carries, or lets the receiver derive, the data the receiver needs for cryptographic synchronization, because packets can be lost or arrive out of order. |
| [RFC4303-ALG-5](#rfc4303-alg-5) | The encryption algorithm for an SA may be NULL, because confidentiality is an optional ESP service. |
| [RFC4303-ALG-6](#rfc4303-alg-6) | The integrity algorithm for an SA may be NULL, because the integrity service may be optional. |
| [RFC4303-ALG-7](#rfc4303-alg-7) | The SA that carries a packet specifies the integrity algorithm that computes the ICV. |
| [RFC4303-ALG-8](#rfc4303-alg-8) | An integrity algorithm used with ESP allows the receiver to process packets that arrive out of order or are lost. |
| [RFC4303-ALG-9](#rfc4303-alg-9) | A combined mode algorithm, when an SA uses one, provides both confidentiality and integrity. |
| [RFC4303-ALG-10](#rfc4303-alg-10) | A combined mode algorithm used with ESP makes provisions for per-packet cryptographic synchronization, so decryption works despite out-of-order arrival or packet loss. |
| [RFC4303-ALG-11](#rfc4303-alg-11) | This specification defines no payload substructure for a combined mode algorithm, so its invocation stays uniform and algorithm-independent. |
| [RFC4303-ALG-12](#rfc4303-alg-12) | No internal detail of a combined mode algorithm's payload layout should be observable externally. |
| [RFC4303-OSA-1](#rfc4303-osa-1) | In transport mode, the sender places the original next layer protocol information between the ESP header and the ESP trailer, and keeps the specified IP header and any IPv6 extension headers unchanged. |
| [RFC4303-OSA-2](#rfc4303-osa-2) | An IPsec implementation applies ESP to an outbound packet only after it determines that the packet is associated with an SA that calls for ESP processing. |
| [RFC4303-OENC-1](#rfc4303-oenc-1) | The sender encapsulates just the original next layer protocol information into the ESP Payload field, in transport mode, whether it uses separate encryption and integrity algorithms or a combined mode algorithm. |
| [RFC4303-OENC-2](#rfc4303-oenc-2) | The sender encapsulates the entire original IP datagram into the ESP Payload field, in tunnel mode, whether it uses separate encryption and integrity algorithms or a combined mode algorithm. |
| [RFC4303-OENC-3](#rfc4303-oenc-3) | The sender adds any necessary padding after encapsulating the payload — optional TFC padding and encryption Padding — whether it uses separate encryption and integrity algorithms or a combined mode algorithm. |
| [RFC4303-OENC-4](#rfc4303-oenc-4) | The sender encrypts the encapsulated result using the key, encryption algorithm, and algorithm mode of the SA, together with any required cryptographic synchronization data. |
| [RFC4303-OENC-5](#rfc4303-oenc-5) | When explicit cryptographic synchronization data such as an IV is used, the sender inputs it to the encryption algorithm and places it in the Payload field. |
| [RFC4303-OENC-6](#rfc4303-oenc-6) | When implicit cryptographic synchronization data is used, the sender constructs it and inputs it to the encryption algorithm, whether it uses separate encryption and integrity algorithms or a combined mode algorithm. |
| [RFC4303-OENC-7](#rfc4303-oenc-7) | When integrity is selected, the sender encrypts before it applies the integrity algorithm, and the encryption never covers the ICV field. |
| [RFC4303-OENC-8](#rfc4303-oenc-8) | The sender uses a keyed integrity algorithm to compute the ICV, because encryption does not protect the ICV itself. |
| [RFC4303-OENC-9](#rfc4303-oenc-9) | The sender computes the ICV over the whole ESP packet except the ICV field, covering the SPI, Sequence Number, Payload Data, Padding, Pad Length, and Next Header in ciphertext form. |
| [RFC4303-OENC-10](#rfc4303-oenc-10) | When the ESN option is enabled, the sender includes the high-order 32 bits of the sequence number in the ICV computation, appended after the Next Header field, but does not transmit them. |
| [RFC4303-OENC-11](#rfc4303-oenc-11) | For some integrity algorithms, the byte string the ICV is computed over must be a multiple of the algorithm's block size. |
| [RFC4303-OENC-12](#rfc4303-oenc-12) | When the ESP packet length does not match the integrity algorithm's block size, the sender must append implicit padding, after the Next Header field or the ESN high-order bits, that it does not transmit. |
| [RFC4303-OENC-13](#rfc4303-oenc-13) | When the integrity algorithm's defining document does not say whether implicit padding is needed, the sender and receiver assume that it is needed to match the algorithm's block size. |
| [RFC4303-OENC-14](#rfc4303-oenc-14) | When padding octets are needed and the algorithm does not specify their contents, the sender must set them to zero. |
| [RFC4303-OENC-15](#rfc4303-oenc-15) | The sender encrypts and integrity-protects the encapsulated result together, using the key and combined mode algorithm of the SA and any required cryptographic synchronization data. |
| [RFC4303-OENC-16](#rfc4303-oenc-16) | When explicit cryptographic synchronization data such as an IV is used, the sender inputs it to the combined mode algorithm and places it in the Payload field. |
| [RFC4303-OENC-17](#rfc4303-oenc-17) | The Sequence Number, or Extended Sequence Number, and the SPI are inputs to the combined mode algorithm, because the integrity check must cover them. |
| [RFC4303-OENC-18](#rfc4303-oenc-18) | The ESP packet format may include an explicit ICV field when a combined mode algorithm is used. |
| [RFC4303-OSEQ-1](#rfc4303-oseq-1) | The sender's per-SA counter starts at 0 and increments for each packet, so the first packet sent on a new SA carries sequence number 1. |
| [RFC4303-OSEQ-2](#rfc4303-oseq-2) | When anti-replay is enabled, the sender checks that its counter has not cycled before it inserts a new value into the Sequence Number field. |
| [RFC4303-OSEQ-3](#rfc4303-oseq-3) | The sender must not send a packet on an SA if doing so would cycle the sequence number. |
| [RFC4303-OSEQ-4](#rfc4303-oseq-4) | An attempt to send a packet that would overflow the sequence number is an auditable event. |
| [RFC4303-OSEQ-5](#rfc4303-oseq-5) | The audit log entry for a sequence number overflow attempt should include the SPI, the current date and time, the source and destination addresses, and, for IPv6, the cleartext Flow ID. |
| [RFC4303-OSEQ-6](#rfc4303-oseq-6) | The sender assumes anti-replay is enabled by default, unless the receiver has notified it otherwise. |
| [RFC4303-OSEQ-7](#rfc4303-oseq-7) | A sender typically establishes a new SA when its Sequence Number or ESN cycles, or in anticipation of that. |
| [RFC4303-OSEQ-8](#rfc4303-oseq-8) | When the key used to compute an ICV is manually distributed, a compliant implementation should not provide anti-replay service. |
| [RFC4303-OSEQ-9](#rfc4303-oseq-9) | When a user chooses anti-replay for a manually keyed SA, the sequence number counter at the sender must stay correct across local reboots, until the key is replaced. |
| [RFC4303-OSEQ-10](#rfc4303-oseq-10) | When anti-replay is disabled, the sender does not need to monitor or reset its counter, but it keeps incrementing it and lets it roll over to zero at the maximum value. |
| [RFC4303-OSEQ-11](#rfc4303-oseq-11) | Letting the counter roll over is the recommended behavior for a multi-sender multicast SA, unless the sender and receiver have negotiated an anti-replay mechanism of their own. |
| [RFC4303-OSEQ-12](#rfc4303-oseq-12) | When ESN is selected, only the low-order 32 bits of the sequence number go on the wire, while sender and receiver each keep a full 64-bit ESN counter. |
| [RFC4303-OSEQ-13](#rfc4303-oseq-13) | The high-order 32 bits of the ESN enter the integrity check in a way that depends on the algorithm or mode, for example appended after the Next Header field when a separate integrity algorithm is used. |
| [RFC4303-FRAG-1](#rfc4303-frag-1) | An IPsec implementation performs fragmentation, when necessary, after ESP processing. |
| [RFC4303-FRAG-2](#rfc4303-frag-2) | In transport mode, ESP applies only to whole IP datagrams, never to IP fragments. |
| [RFC4303-FRAG-3](#rfc4303-frag-3) | A packet already protected by ESP may be fragmented by routers en route, and the receiver reassembles such fragments before ESP processing. |
| [RFC4303-FRAG-4](#rfc4303-frag-4) | In tunnel mode, ESP may apply to an IP packet that is itself a fragment of an IP datagram, for example at a security gateway or a bump-in-the-stack or bump-in-the-wire implementation. |
| [RFC4303-FRAG-5](#rfc4303-frag-5) | For transport mode, a bump-in-the-stack or bump-in-the-wire implementation may have to reassemble a locally fragmented packet, apply IPsec, and then fragment the result. |
| [RFC4303-FRAG-6](#rfc4303-frag-6) | For IPv6, a bump-in-the-stack or bump-in-the-wire implementation examines all extension headers to find a fragmentation header, and so learns whether the packet needs reassembly ahead of IPsec processing. |
| [RFC4303-FRAG-7](#rfc4303-frag-7) | An ESP implementation may choose to not support fragmentation. |
| [RFC4303-FRAG-8](#rfc4303-frag-8) | An ESP implementation may mark its transmitted packets with the DF bit, to help Path MTU discovery. |
| [RFC4303-FRAG-9](#rfc4303-frag-9) | An ESP implementation must support generating ICMP PMTU messages, or equivalent internal signaling on a native host, to reduce the likelihood of fragmentation. |
| [RFC4303-REAS-1](#rfc4303-reas-1) | An IPsec implementation performs reassembly, when required, before ESP processing. |
| [RFC4303-REAS-2](#rfc4303-reas-2) | The receiver must discard a packet offered to ESP that still looks like an IP fragment, when the OFFSET field is non-zero or the MORE FRAGMENTS flag is set. |
| [RFC4303-REAS-3](#rfc4303-reas-3) | Discarding an apparent IP fragment offered to ESP processing is an auditable event. |
| [RFC4303-REAS-4](#rfc4303-reas-4) | The audit log entry for a discarded apparent fragment should include the SPI, date and time received, source and destination addresses, Sequence Number, and, for IPv6, the Flow ID. |
| [RFC4303-REAS-5](#rfc4303-reas-5) | After it reassembles a packet, the IP code zeroes the OFFSET field and clears the MORE FRAGMENTS flag, so ESP does not mistake the reassembled packet for a fragment. |
| [RFC4303-ISA-1](#rfc4303-isa-1) | On receiving a packet with an ESP Header, the receiver finds the appropriate unidirectional SA by looking it up in the SAD. |
| [RFC4303-ISA-2](#rfc4303-isa-2) | For a unicast SA, the receiver bases the SAD lookup on the SPI, or on the SPI plus the protocol field. |
| [RFC4303-ISA-3](#rfc4303-isa-3) | When an implementation supports multicast traffic, it also uses the destination address, and may use the sender address, in the SAD lookup. |
| [RFC4303-ISA-4](#rfc4303-isa-4) | The SAD entry for an SA states whether the Sequence Number field is checked, whether sequence numbers are 32 or 64 bits, and whether an explicit ICV field is present and how large it is. |
| [RFC4303-ISA-5](#rfc4303-isa-5) | The SAD entry for an SA also states the algorithms and keys used for decryption and, where applicable, ICV computation. |
| [RFC4303-ISA-6](#rfc4303-isa-6) | The receiver must discard a packet for which no valid Security Association exists. |
| [RFC4303-ISA-7](#rfc4303-isa-7) | Discarding a packet for which no valid SA exists is an auditable event. |
| [RFC4303-ISA-8](#rfc4303-isa-8) | The audit log entry for a discard due to no valid SA should include the SPI, date and time received, source and destination addresses, Sequence Number, and, for IPv6, the cleartext Flow ID. |
| [RFC4303-ISEQ-1](#rfc4303-iseq-1) | Every ESP implementation must support the anti-replay service, though the receiver may enable or disable it for each SA. |
| [RFC4303-ISEQ-2](#rfc4303-iseq-2) | The receiver must not enable anti-replay on an SA unless the ESP integrity service is also enabled for it, because otherwise the Sequence Number field is not integrity protected. |
| [RFC4303-ISEQ-3](#rfc4303-iseq-3) | Anti-replay applies to unicast SAs as well as multicast SAs. |
| [RFC4303-ISEQ-4](#rfc4303-iseq-4) | When no anti-replay mechanism is negotiated or configured for a multi-sender SA, the sender and receiver disable sequence number checking for it. |
| [RFC4303-ISEQ-5](#rfc4303-iseq-5) | When the receiver has not enabled anti-replay for an SA, it performs no inbound checks on the Sequence Number. |
| [RFC4303-ISEQ-6](#rfc4303-iseq-6) | By default, the sender assumes the receiver has anti-replay enabled. |
| [RFC4303-ISEQ-7](#rfc4303-iseq-7) | When an SA establishment protocol is used, a receiver that will not provide anti-replay protection should notify the sender of this during SA establishment. |
| [RFC4303-ISEQ-8](#rfc4303-iseq-8) | When the receiver has enabled anti-replay for an SA, the receive packet counter for the SA must be initialized to zero when the SA is established. |
| [RFC4303-ISEQ-9](#rfc4303-iseq-9) | For each received packet, the receiver must verify that its Sequence Number does not duplicate that of any other packet received during the SA's life. |
| [RFC4303-ISEQ-10](#rfc4303-iseq-10) | The duplicate Sequence Number check should be the first ESP check the receiver applies to a packet once it is matched to an SA, to reject duplicates quickly. |
| [RFC4303-ISEQ-11](#rfc4303-iseq-11) | ESP allows a two-stage verification of packet sequence numbers, and an implementation capable of line-rate operation need not perform the preliminary stage. |
| [RFC4303-ISEQ-12](#rfc4303-iseq-12) | The preliminary Sequence Number check uses the Sequence Number value from the ESP Header and runs before integrity checking and decryption. |
| [RFC4303-ISEQ-13](#rfc4303-iseq-13) | When the preliminary Sequence Number check fails, the receiver discards the packet before it performs any cryptographic operation. |
| [RFC4303-ISEQ-14](#rfc4303-iseq-14) | When the preliminary check succeeds, the receiver does not yet update its local counter, because the Sequence Number's integrity has not been verified yet. |
| [RFC4303-ISEQ-15](#rfc4303-iseq-15) | The receiver rejects duplicate packets using a sliding receive window. |
| [RFC4303-ISEQ-16](#rfc4303-iseq-16) | The right edge of the receive window is the highest validated Sequence Number received on the SA, and the receiver rejects a packet whose sequence number is lower than the window's left edge. |
| [RFC4303-ISEQ-17](#rfc4303-iseq-17) | The receiver checks a packet that falls within the window against the list of Sequence Numbers already received in that window. |
| [RFC4303-ISEQ-18](#rfc4303-iseq-18) | With ESN, the receiver checks a received Sequence Number against the window using the full sequence number, reconstructed from the transmitted low-order 32 bits and its own local high-order 32 bits. |
| [RFC4303-ISEQ-19](#rfc4303-iseq-19) | When the packet's low-order 32 bits are lower than the receiver's own low-order bits, the receiver assumes the high-order 32 bits have advanced to a new subspace. |
| [RFC4303-ISEQ-20](#rfc4303-iseq-20) | The receiver may employ additional, heuristic checks to re-synchronize its sequence number counter when a gap larger than 2\*\*32-1 packets occurs. |
| [RFC4303-ISEQ-21](#rfc4303-iseq-21) | The receiver proceeds to integrity verification for a packet that is not a duplicate within the window, or that falls to the right of the window, when a separate integrity algorithm is used. |
| [RFC4303-ISEQ-22](#rfc4303-iseq-22) | When a combined mode algorithm is used, the receiver performs the integrity check together with decryption. |
| [RFC4303-ISEQ-23](#rfc4303-iseq-23) | When the integrity check fails, the receiver must discard the received IP datagram as invalid. |
| [RFC4303-ISEQ-24](#rfc4303-iseq-24) | Discarding a packet that fails the integrity check is an auditable event. |
| [RFC4303-ISEQ-25](#rfc4303-iseq-25) | The audit log entry for a failed integrity check should include the SPI, date and time received, source and destination addresses, the Sequence Number, and, for IPv6, the Flow ID. |
| [RFC4303-ISEQ-26](#rfc4303-iseq-26) | The receiver updates the receive window only when the integrity verification succeeds. |
| [RFC4303-ISEQ-27](#rfc4303-iseq-27) | With a combined mode algorithm, the integrity-protected Sequence Number also matches the Sequence Number used for anti-replay protection. |
| [RFC4303-ISEQ-28](#rfc4303-iseq-28) | The receiver must support a minimum receive window of 32 packets when using 32-bit sequence numbers. |
| [RFC4303-ISEQ-29](#rfc4303-iseq-29) | A window size of 64 is preferred, and should be the default receive window size. |
| [RFC4303-ISEQ-30](#rfc4303-iseq-30) | The receiver may choose another window size larger than the minimum, without notifying the sender of its choice. |
| [RFC4303-ISEQ-31](#rfc4303-iseq-31) | The receive window size should be increased for higher-speed environments, independent of assurance considerations. |
| [RFC4303-IICV-1](#rfc4303-iicv-1) | When integrity has been selected, the receiver computes the ICV over the ESP packet minus the ICV field, using the SA's integrity algorithm, and checks it against the carried ICV. |
| [RFC4303-IICV-2](#rfc4303-iicv-2) | The receiver accepts the datagram as valid when the computed and received ICVs match. |
| [RFC4303-IICV-3](#rfc4303-iicv-3) | When the ICV test fails, the receiver must discard the received IP datagram as invalid. |
| [RFC4303-IICV-4](#rfc4303-iicv-4) | Discarding a datagram for a failed ICV test is an auditable event. |
| [RFC4303-IICV-5](#rfc4303-iicv-5) | The log data for a failed ICV test should include the SPI, date and time received, source and destination addresses, the Sequence Number, and, for IPv6, the cleartext Flow ID. |
| [RFC4303-IICV-6](#rfc4303-iicv-6) | An implementation may follow any set of steps that gives the same result as the reference steps for ICV verification. |
| [RFC4303-IICV-7](#rfc4303-iicv-7) | When implicit padding is required, the receiver appends zero-filled bytes after the Next Header field, or after the ESN high-order bits, and computes and compares the ICV using the algorithm's own comparison rule. |
| [RFC4303-IICV-8](#rfc4303-iicv-8) | The receiver decrypts the ESP Payload Data, Padding, Pad Length, and Next Header using the SA's key, encryption algorithm, algorithm mode, and cryptographic synchronization data. |
| [RFC4303-IICV-9](#rfc4303-iicv-9) | When explicit cryptographic synchronization data such as an IV is used, the receiver takes it from the Payload field and inputs it to the decryption algorithm. |
| [RFC4303-IICV-10](#rfc4303-iicv-10) | When implicit cryptographic synchronization data is used, the receiver constructs a local version of it and inputs it to the decryption algorithm. |
| [RFC4303-IICV-11](#rfc4303-iicv-11) | The receiver processes any Padding as the encryption algorithm's specification directs. |
| [RFC4303-IICV-12](#rfc4303-iicv-12) | When the default padding scheme has been used, the receiver should inspect the Padding field before it removes the padding and passes the decrypted data to the next layer. |
| [RFC4303-IICV-13](#rfc4303-iicv-13) | The receiver checks the Next Header field, and discards the packet without further processing when the value is 59, meaning no next header. |
| [RFC4303-IICV-14](#rfc4303-iicv-14) | In transport mode, the receiver reconstructs the original IP datagram from the outer IP header plus the original next layer protocol information in the ESP Payload field. |
| [RFC4303-IICV-15](#rfc4303-iicv-15) | In tunnel mode, the receiver reconstructs the original IP datagram from the entire IP datagram carried in the ESP Payload field. |
| [RFC4303-IICV-16](#rfc4303-iicv-16) | In an IPv6 context, the receiver should at least ensure that the decrypted data is 8-byte aligned, so the protocol named in the Next Header field can process it. |
| [RFC4303-IICV-17](#rfc4303-iicv-17) | Reconstructing the original datagram discards any optional TFC padding, which sits after the IP datagram or transport-layer frame and before the Padding field. |
| [RFC4303-IICV-18](#rfc4303-iicv-18) | When integrity checking and encryption run in parallel, the receiver must complete integrity checking before it passes the decrypted packet on for further processing. |
| [RFC4303-IICV-19](#rfc4303-iicv-19) | A receiver that decrypts in parallel with integrity checking takes care to avoid race conditions in accessing and extracting the decrypted packet. |
| [RFC4303-IICV-20](#rfc4303-iicv-20) | The receiver decrypts and integrity checks the ESP Payload Data, Padding, Pad Length, and Next Header together, using the SA's key, algorithm, algorithm mode, and cryptographic synchronization data. |
| [RFC4303-IICV-21](#rfc4303-iicv-21) | The SPI from the ESP header and the receiver's adjusted packet counter value are inputs to the combined mode algorithm, because they are required for the integrity check. |
| [RFC4303-IICV-22](#rfc4303-iicv-22) | When the combined mode algorithm's integrity check fails, the receiver must discard the received IP datagram as invalid. |
| [RFC4303-IICV-23](#rfc4303-iicv-23) | Discarding a datagram for a failed combined-mode integrity check is an auditable event. |
| [RFC4303-IICV-24](#rfc4303-iicv-24) | The log data for a failed combined-mode integrity check should include the SPI, date and time received, source and destination addresses, the Sequence Number, and, for IPv6, the cleartext Flow ID. |
| [RFC4303-IICV-25](#rfc4303-iicv-25) | The receiver processes any Padding as the encryption algorithm specifies, unless the combined mode algorithm has already done so. |
| [RFC4303-IICV-26](#rfc4303-iicv-26) | The receiver extracts the original IP datagram, in tunnel mode, or the transport-layer frame, in transport mode, from the ESP Payload Data field. |
| [RFC4303-IICV-27](#rfc4303-iicv-27) | Extracting the original datagram or frame implicitly discards any optional TFC padding, which sits after the IP payload and before the Padding field. |
| [RFC4303-CONF-1](#rfc4303-conf-1) | A conformant implementation must implement the ESP syntax and processing this specification describes, for unicast traffic. |
| [RFC4303-CONF-2](#rfc4303-conf-2) | A conformant implementation must also comply with all additional packet processing requirements the Security Architecture document imposes. |
| [RFC4303-CONF-3](#rfc4303-conf-3) | An implementation that claims to support multicast traffic must also comply with the additional requirements specified for multicast support. |
| [RFC4303-CONF-4](#rfc4303-conf-4) | When the key used to compute an ICV is manually distributed, correct anti-replay service depends on the sender correctly keeping the counter state across events such as local reboots, until the key is replaced. |
| [RFC4303-CONF-5](#rfc4303-conf-5) | A compliant implementation should not provide anti-replay service for SAs that are manually keyed. |
| [RFC4303-CONF-6](#rfc4303-conf-6) | The mandatory-to-implement algorithms for ESP are defined in a separate document, so algorithm requirements can be updated independently of the protocol. |
| [RFC4303-CONF-7](#rfc4303-conf-7) | An implementation may support additional algorithms beyond those mandated for ESP. |
| [RFC4303-CONF-8](#rfc4303-conf-8) | A conformant implementation supports the NULL encryption algorithm, to stay consistent with how ESP services are negotiated, because encryption in ESP is optional. |
| [RFC4303-CONF-9](#rfc4303-conf-9) | Support for the confidentiality-only version of the ESP service is optional. |
| [RFC4303-CONF-10](#rfc4303-conf-10) | An implementation that offers the confidentiality-only service must also support negotiating the NULL integrity algorithm. |
| [RFC4303-CONF-11](#rfc4303-conf-11) | An SA must not set both the integrity and encryption algorithms to NULL at the same time, even though each one may individually be NULL. |

## The ESP packet format

### RFC4303-FMT-1

**The header that immediately precedes ESP carries protocol number 50.**

> "The (outer) protocol header (IPv4, IPv6, or Extension) that immediately precedes the ESP
> header SHALL contain the value 50 in its Protocol (IPv4) or Next Header (IPv6, Extension)
> field" — §2, `rfc4303.txt:236-238`

- Strength: must. Class: wire.
- Check idea: capture a packet carrying ESP and read the Protocol field (IPv4) or Next Header
  field (IPv6, or an extension header) of the header immediately before the ESP header; confirm
  it equals 50.

### RFC4303-FMT-2

**The ESP packet places its fields in the order SPI, Sequence Number, Payload Data, Padding, Pad Length, Next Header, and, if present, ICV.**

> "The packet begins with two 4-byte fields (Security Parameters Index (SPI) and Sequence
> Number)." and "Following these fields is the Payload Data" and "Following the Payload Data
> are Padding and Pad Length fields, and the Next Header field." and "The optional Integrity
> Check Value (ICV) field completes the packet." — §2, `rfc4303.txt:240-242`, `rfc4303.txt:245-247`

- Strength: description. Class: encoding.
- Check idea: capture an ESP packet and read the fields in order from the start of the packet;
  confirm the sequence SPI, Sequence Number, Payload Data, Padding, Pad Length, Next Header,
  and, when the SA uses an ICV, the ICV.

### RFC4303-FMT-3

**The transmitted ESP trailer is the Padding, Pad Length, and Next Header fields.**

> "The (transmitted) ESP trailer consists of the Padding, Pad Length, and Next Header fields."
> — §2, `rfc4303.txt:287-288`

- Strength: description. Class: encoding.
- Check idea: capture an ESP packet and confirm the three fields following the Payload Data,
  and preceding the ICV if present, are Padding, Pad Length, and Next Header.

### RFC4303-FMT-4

**Implicit ESP trailer data that is not transmitted is included in the integrity computation.**

> "Additional, implicit ESP trailer data (which is not transmitted) is included in the
> integrity computation, as described below." — §2, `rfc4303.txt:288-290`

- Strength: description. Class: internal.
- Check idea: under an SA using extended sequence numbers or integrity-algorithm padding,
  vary the high-order sequence bits or padding bits that are not sent on the wire and confirm
  the receiver's integrity check result changes accordingly.

### RFC4303-FMT-5

**When the integrity service is selected, the integrity computation covers the SPI, the Sequence Number, the Payload Data, and the ESP trailer.**

> "If the integrity service is selected, the integrity computation encompasses the SPI,
> Sequence Number, Payload Data, and the ESP trailer (explicit and implicit)." — §2,
> `rfc4303.txt:292-294`

- Strength: description. Class: encoding.
- Check idea: under an SA with integrity selected, corrupt one byte of the SPI, the Sequence
  Number, the Payload Data, or the ESP trailer in turn and confirm the receiver's integrity
  check fails each time.

### RFC4303-FMT-6

**When the confidentiality service is selected, the ciphertext is the Payload Data, apart from any cryptographic synchronization data, and the explicit ESP trailer.**

> "If the confidentiality service is selected, the ciphertext consists of the Payload Data
> (except for any cryptographic synchronization data that may be included) and the (explicit)
> ESP trailer." — §2, `rfc4303.txt:296-298`

- Strength: may (lower case). Class: encoding.
- Check idea: under an SA with confidentiality selected, confirm that only the Payload Data
  (excluding any synchronization data) and the explicit ESP trailer decrypt to meaningful
  content, while the SPI and Sequence Number stay in the clear.

### RFC4303-FMT-7

**The SPI and Sequence Number fields are not encrypted, but are given integrity whenever the integrity service is selected, regardless of the algorithm mode.**

> "Because the SPI and Sequence Number fields require integrity as part of the integrity
> service, and they are not encrypted, it is necessary to ensure that they are afforded
> integrity whenever the service is selected, regardless of the style of combined algorithm
> mode employed." — §2, `rfc4303.txt:312-316`

- Strength: description. Class: encoding.
- Check idea: under an SA with integrity selected, including one using a combined mode
  algorithm, confirm the SPI and Sequence Number fields remain in the clear on the wire and
  that corrupting either one makes the receiver's integrity check fail.

### RFC4303-FMT-8

**For a combined mode algorithm, the explicit ICV field that would otherwise close the packet may be left out.**

> "For combined mode algorithms, the ICV that would normally appear at the end of the ESP
> packet (when integrity is selected) may be omitted." and "If a combined algorithm mode is
> employed, the explicit ICV shown in Figures 1 and 2 may be omitted" — §2, `rfc4303.txt:320-322`,
> `rfc4303.txt:378-379`

- Strength: may (lower case). Class: wire.
- Check idea: under an SA using a combined mode algorithm, capture a packet and confirm no
  explicit ICV field is present at the end of the packet.

### RFC4303-FMT-9

**When the ICV is left out under a combined mode algorithm, that algorithm encodes an ICV-equivalent means of verifying integrity within the Payload Data.**

> "When the ICV is omitted and integrity is selected, it is the responsibility of the combined
> mode algorithm to encode within the Payload Data an ICV-equivalent means of verifying the
> integrity of the packet." — §2, `rfc4303.txt:322-325`

- Strength: description. Class: encoding.
- Check idea: under an SA using a combined mode algorithm with no explicit ICV field, confirm
  the Payload Data itself carries the data the receiver needs to verify integrity.

### RFC4303-FMT-10

**When a combined mode algorithm gives integrity only to the data it encrypts, the SPI and Sequence Number are replicated inside the Payload Data.**

> "If a combined mode algorithm offers integrity only to data that is encrypted, it will be
> necessary to replicate the SPI and Sequence Number as part of the Payload Data." — §2,
> `rfc4303.txt:327-329`

- Strength: description. Class: encoding.
- Check idea: under an SA using such a combined mode algorithm, decrypt a captured packet's
  Payload Data and confirm it contains a copy of the SPI and Sequence Number.

### RFC4303-FMT-11

**Traffic flow confidentiality padding, when used, is inserted after the Payload Data and before the ESP trailer.**

> "Finally, a new provision is made to insert padding for traffic flow confidentiality after
> the Payload Data and before the ESP trailer." — §2, `rfc4303.txt:331-332`

- Strength: description. Class: encoding.
- Check idea: capture a packet from an SA using TFC padding and confirm the padding bytes lie
  between the end of the Payload Data and the start of the ESP trailer fields.

### RFC4303-FMT-12

**In tunnel mode, an IPsec implementation may add TFC padding after the Payload Data and before the Padding field.**

> "If tunnel mode is being used, then the IPsec implementation can add Traffic Flow
> Confidentiality (TFC) padding (see Section 2.4) after the Payload Data and before the
> Padding (0-255 bytes) field." — Figure 2 note, `rfc4303.txt:373-376`

- Strength: description. Class: encoding.
- Check idea: capture a tunnel-mode packet from an SA using TFC padding and confirm the TFC
  padding bytes lie between the Payload Data and the Padding field.

### RFC4303-FMT-13

**The detailed format of ESP packets, including the Payload Data substructure, is fixed for all traffic on a given SA.**

> "Because algorithms and modes are fixed when an SA is established, the detailed format of
> ESP packets for a given SA (including the Payload Data substructure) is fixed, for all
> traffic on the SA." — §2, `rfc4303.txt:379-382`

- Strength: description. Class: wire.
- Check idea: capture several packets sent on the same SA and confirm they all share the same
  field layout and Payload Data substructure.

### RFC4303-FMT-14

**An optional field is present in neither the transmitted packet nor the data formatted for the ICV computation when its option is not selected.**

> ""Optional" means that the field is omitted if the option is not selected, i.e., it is
> present in neither the packet as transmitted nor as formatted for computation of an ICV"
> — §2, `rfc4303.txt:487-489`

- Strength: description. Class: encoding.
- Check idea: for an SA that does not select a given optional field, capture a packet and
  confirm the field is entirely absent, not merely zeroed, and is also absent from the data
  the ICV is computed over.

### RFC4303-FMT-15

**The format of ESP packets for a given SA is fixed for the duration of that SA.**

> "Whether or not an option is selected is determined as part of Security Association (SA)
> establishment. Thus, the format of ESP packets for a given SA is fixed, for the duration of
> the SA." — §2, `rfc4303.txt:490-492`

- Strength: description. Class: wire.
- Check idea: capture packets from the same SA at different times and confirm the set of
  fields present does not change over the SA's lifetime.

### RFC4303-FMT-16

**Mandatory ESP fields are always present in the packet format, for every SA.**

> "In contrast, "mandatory" fields are always present in the ESP packet format, for all SAs."
> — §2, `rfc4303.txt:492-494`

- Strength: description. Class: wire.
- Check idea: capture packets from different SAs with different option choices and confirm
  the mandatory fields are present in every one of them.

### RFC4303-FMT-17

**Cryptographic algorithms used in IPsec take input, and produce output, in network byte order, the same order in which IP packets are transmitted.**

> "All of the cryptographic algorithms used in IPsec expect their input in canonical network
> byte order (see Appendix of RFC 791 [Pos81]) and generate their output in canonical network
> byte order. IP packets are also transmitted in network byte order." — §2, `rfc4303.txt:496-499`

- Strength: description. Class: encoding.
- Check idea: confirm that multi-byte fields covered by a cryptographic computation appear on
  the wire in network (big-endian) byte order.

### RFC4303-FMT-18

**Because ESP has no version number, a signaling or configuration mechanism between the two IPsec peers must resolve any backward-compatibility concern.**

> "ESP does not contain a version number, therefore if there are concerns about backward
> compatibility, they MUST be addressed by using a signaling mechanism between the two IPsec
> peers to ensure compatible versions of ESP (e.g., Internet Key Exchange (IKEv2) [Kau05]) or
> an out-of-band configuration mechanism." — §2, `rfc4303.txt:511-515`

- Strength: must. Class: internal.
- Check idea: inspect the SA management configuration or negotiation between two peers and
  confirm a signaling or out-of-band mechanism establishes a compatible ESP version, since no
  version field exists on the wire to do so.

## Security Parameters Index (SPI)

### RFC4303-SPI-1

**The SPI is an arbitrary 32-bit value a receiver uses to identify the SA of an incoming packet.**

> "The SPI is an arbitrary 32-bit value that is used by a receiver to identify the SA to which
> an incoming packet is bound." — §2.1, `rfc4303.txt:519-521`

- Strength: description. Class: encoding.
- Check idea: capture an inbound ESP packet and confirm the receiver selects its SA using the
  32-bit SPI value carried in the packet.

### RFC4303-SPI-2

**The SPI field is mandatory.**

> "The SPI field is mandatory." — §2.1, `rfc4303.txt:520-521`

- Strength: description. Class: wire.
- Check idea: capture ESP packets from any SA and confirm the SPI field is always present.

### RFC4303-SPI-3

**For a unicast SA, the receiver generates the SPI value.**

> "Because the SPI value is generated by the receiver for a unicast SA" — §2.1,
> `rfc4303.txt:525-526`

- Strength: description. Class: internal.
- Check idea: observe SA establishment for a unicast SA and confirm the SPI value assigned to
  it originates from the receiving peer.

### RFC4303-SPI-4

**An ESP implementation must support mapping inbound unicast traffic to an SA using the SPI, alone or together with the IPsec protocol type.**

> "This mechanism for mapping inbound traffic to unicast SAs MUST be supported by all ESP
> implementations." — §2.1, `rfc4303.txt:528-529`

- Strength: must. Class: internal.
- Check idea: send an inbound unicast ESP packet and confirm the receiver maps it to the
  correct SA using the SPI, by itself or together with the ESP protocol value.

### RFC4303-SPI-5

**An IPsec implementation that supports multicast must support multicast SAs, using the inbound de-multiplexing algorithm given for the SPI.**

> "If an IPsec implementation supports multicast, then it MUST support multicast SAs using the
> algorithm below for mapping inbound IPsec datagrams to SAs." — §2.1, `rfc4303.txt:531-533`

- Strength: must. Class: internal.
- Check idea: on an implementation that supports multicast, send an inbound multicast ESP
  packet and confirm the receiver maps it to the correct SA by the SPI-based lookup steps.

### RFC4303-SPI-6

**An implementation that supports only unicast traffic need not implement the multicast SPI de-multiplexing algorithm.**

> "Implementations that support only unicast traffic need not implement this de-multiplexing
> algorithm." — §2.1, `rfc4303.txt:533-534`

- Strength: description. Class: internal.
- Check idea: on a unicast-only implementation, confirm the absence of the multicast SAD
  search algorithm does not affect correct handling of unicast SPI lookups.

### RFC4303-SPI-7

**A multicast-capable IPsec implementation must correctly de-multiplex inbound traffic even when a group SA and a unicast SA use the same SPI.**

> "A multicast-capable IPsec implementation MUST correctly de-multiplex inbound traffic even
> in the context of SPI collisions." — §2.1, `rfc4303.txt:542-544`

- Strength: must. Class: end-to-end.
- Check idea: configure a group SA and a unicast SA that share one SPI value, send a packet
  matching each, and confirm the receiver processes each packet with its correct SA.

### RFC4303-SPI-8

**Each SAD entry indicates whether the SA lookup for it uses the destination address alone, or the destination and source addresses, together with the SPI.**

> "Each entry in the Security Association Database (SAD) [Ken-Arch] must indicate whether the
> SA lookup makes use of the destination, or destination and source, IP addresses, in addition
> to the SPI." — §2.1, `rfc4303.txt:546-548`

- Strength: must (lower case). Class: internal.
- Check idea: inspect an SAD entry and confirm it records whether its lookup key is
  {SPI, destination} or {SPI, destination, source}.

### RFC4303-SPI-9

**For multicast SAs, the protocol field is not used for SA lookups.**

> "For multicast SAs, the protocol field is not employed for SA lookups." — §2.1,
> `rfc4303.txt:548-549`

- Strength: description. Class: internal.
- Check idea: for a multicast SA, confirm the receiver's SAD lookup ignores the protocol field
  and matches on the SPI and address fields alone.

### RFC4303-SPI-10

**For every inbound IPsec-protected packet, an implementation searches the SAD for the entry that matches the longest SA identifier: the entry that, among those matching the SPI, also matches on destination, or destination and source, address.**

> "For each inbound, IPsec-protected packet, an implementation must conduct its search of the
> SAD such that it finds the entry that matches the "longest" SA identifier." and "if two or
> more SAD entries match based on the SPI value, then the entry that also matches based on
> destination, or destination and source, address comparison (as indicated in the SAD entry)
> is the "longest" match." — §2.1, `rfc4303.txt:550-552`, `rfc4303.txt:552-555`

- Strength: must (lower case). Class: internal.
- Check idea: install two SAD entries that match the same SPI but differ in whether they also
  require an address match, send a matching packet, and confirm the receiver picks the entry
  with the longer identifier.

### RFC4303-SPI-11

**The receiver searches the SAD first for a match on SPI, destination, and source address; otherwise it processes the packet with that entry.**

> "1. Search the SAD for a match on {SPI, destination address, source address}. If an SAD
> entry matches, then process the inbound ESP packet with that matching SAD entry. Otherwise,
> proceed to step 2." — §2.1, `rfc4303.txt:567-570`

- Strength: description. Class: end-to-end.
- Check idea: send a packet matching an SAD entry keyed on {SPI, destination, source} and
  confirm the receiver processes it with that entry without going on to the next search step.

### RFC4303-SPI-12

**Failing a match on SPI, destination, and source, the receiver searches the SAD for a match on SPI and destination address only; otherwise it processes the packet with that entry.**

> "2. Search the SAD for a match on {SPI, destination address}. If the SAD entry matches, then
> process the inbound ESP packet with that matching SAD entry. Otherwise, proceed to step 3."
> — §2.1, `rfc4303.txt:572-575`

- Strength: description. Class: end-to-end.
- Check idea: send a packet that matches an SAD entry only on {SPI, destination} and confirm
  the receiver processes it with that entry after the first search step fails.

### RFC4303-SPI-13

**Failing the first two searches, the receiver searches the SAD for a match on SPI alone, or on SPI and protocol, depending on whether it keeps one SPI space for AH and ESP; if no entry matches, it discards the packet and logs an auditable event.**

> "3. Search the SAD for a match on only {SPI} if the receiver has chosen to maintain a single
> SPI space for AH and ESP, or on {SPI, protocol} otherwise. If an SAD entry matches, then
> process the inbound ESP packet with that matching SAD entry. Otherwise, discard the packet
> and log an auditable event." — §2.1, `rfc4303.txt:577-581`

- Strength: description. Class: end-to-end.
- Check idea: send a packet that matches an SAD entry only on {SPI} or {SPI, protocol} and
  confirm the receiver processes it with that entry; send one matching no entry at all and
  confirm the receiver discards it and records an audit event.

### RFC4303-SPI-14

**An implementation may use any method to search the SAD, but its externally visible behavior must be equivalent to searching the SAD in the longest-match order given.**

> "In practice, an implementation MAY choose any method to accelerate this search, although
> its externally visible behavior MUST be functionally equivalent to having searched the SAD
> in the above order." — §2.1, `rfc4303.txt:583-586`

- Strength: may; must. Class: end-to-end.
- Check idea: install SAD entries whose identifiers would produce different search outcomes
  under a naive ordering, send packets that exercise the collisions, and confirm the receiver
  always resolves each packet to the entry the longest-match order specifies.

### RFC4303-SPI-15

**Whether inbound traffic mapping to SAs requires source-and-destination address matching must be set by manual SA configuration or by negotiation through an SA management protocol.**

> "The indication of whether source and destination address matching is required to map
> inbound IPsec traffic to SAs MUST be set either as a side effect of manual SA configuration
> or via negotiation using an SA management protocol, e.g., IKE or Group Domain of
> Interpretation (GDOI) [RFC3547]." — §2.1, `rfc4303.txt:596-600`

- Strength: must. Class: internal.
- Check idea: inspect an SA and confirm the indication of whether address matching is required
  for its inbound lookup was set either by manual configuration or by SA-management-protocol
  negotiation.

### RFC4303-SPI-16

**A Source-Specific Multicast group typically uses a 3-tuple SA identifier of SPI, destination multicast address, and source address.**

> "Typically, Source-Specific Multicast (SSM) [HC03] groups use a 3-tuple SA identifier
> composed of an SPI, a destination multicast address, and source address." — §2.1,
> `rfc4303.txt:600-602`

- Strength: description. Class: internal.
- Check idea: inspect the SA identifier of an SSM group SA and confirm it is the 3-tuple SPI,
  destination multicast address, source address.

### RFC4303-SPI-17

**An Any-Source Multicast group SA needs only an SPI and a destination multicast address as its identifier.**

> "An Any-Source Multicast group SA requires only an SPI and a destination multicast address
> as an identifier." — §2.1, `rfc4303.txt:602-604`

- Strength: description. Class: internal.
- Check idea: inspect the SA identifier of an Any-Source Multicast group SA and confirm it is
  the pair SPI, destination multicast address, with no source address.

### RFC4303-SPI-18

**The SPI value zero is reserved for local, implementation-specific use and must not be sent on the wire.**

> "The SPI value of zero (0) is reserved for local, implementation-specific use and MUST NOT
> be sent on the wire." — §2.1, `rfc4303.txt:609-611`

- Strength: must not. Class: wire.
- Check idea: capture ESP packets leaving an implementation and confirm none carries an SPI
  field of value zero.

## Sequence Number

### RFC4303-SEQ-1

**The Sequence Number field is an unsigned 32-bit per-SA counter that increases by one for each packet sent.**

> "This unsigned 32-bit field contains a counter value that increases by one for each packet
> sent, i.e., a per-SA packet sequence number." — §2.2, `rfc4303.txt:629-630`

- Strength: description. Class: encoding.
- Check idea: capture successive packets sent on one SA and confirm the Sequence Number field
  increases by exactly one from each packet to the next.

### RFC4303-SEQ-2

**For a unicast SA or a single-sender multicast SA, the sender must increment the Sequence Number for every transmitted packet.**

> "For a unicast SA or a single-sender multicast SA, the sender MUST increment this field for
> every transmitted packet." — §2.2, `rfc4303.txt:630-632`

- Strength: must. Class: wire.
- Check idea: capture successive packets sent on a unicast or single-sender multicast SA and
  confirm the sender increments the Sequence Number field on every one.

### RFC4303-SEQ-3

**Sharing an SA among multiple senders is permitted, though not generally recommended.**

> "Sharing an SA among multiple senders is permitted, though generally not recommended." — §2.2,
> `rfc4303.txt:632-634`

- Strength: description. Class: internal.
- Check idea: configure an SA with more than one sender and confirm the implementation allows
  it to be established.

### RFC4303-SEQ-4

**ESP's anti-replay features are not available for a multi-sender SA, because ESP has no way to synchronize packet counters, or meaningfully manage a receiver window, among multiple senders.**

> "ESP provides no means of synchronizing packet counters among multiple senders or
> meaningfully managing a receiver packet counter and window in the context of multiple
> senders. Thus, for a multi-sender SA, the anti-replay features of ESP are not available"
> — §2.2, `rfc4303.txt:634-637`

- Strength: description. Class: internal.
- Check idea: configure a multi-sender SA and confirm the receiver does not apply an
  anti-replay window to it.

### RFC4303-SEQ-5

**The Sequence Number field must always be present and transmitted, even when the receiver has not enabled anti-replay for the SA.**

> "The field is mandatory and MUST always be present even if the receiver does not elect to
> enable the anti-replay service for a specific SA." and "the sender MUST always transmit this
> field" — §2.2, `rfc4303.txt:640-642`, `rfc4303.txt:645`

- Strength: must. Class: wire.
- Check idea: capture packets from an SA with anti-replay disabled and confirm the Sequence
  Number field is still present and populated.

### RFC4303-SEQ-6

**All ESP implementations must be capable of the sequence-number generation and verification processing, even though the receiver decides whether to act on it for a given SA.**

> "all ESP implementations MUST be capable of performing the processing described in Sections
> 3.3.3 and
> 3.4.3" and "receiver need not act upon it" — §2.2, `rfc4303.txt:643-645`,
> `rfc4303.txt:646`

- Strength: must (generation and verification capability); description (the receiver's discretion not to act on it). Class: internal.
- Check idea: confirm an implementation can generate and verify sequence numbers as described,
  then configure a receiver to disable anti-replay and confirm it still accepts packets without
  acting on the counter.

### RFC4303-SEQ-7

**The sender's and receiver's sequence counters are initialized to zero when an SA is established, so the first packet sent on the SA carries sequence number 1.**

> "The sender's counter and the receiver's counter are initialized to 0 when an SA is
> established." and "The first packet sent using a given SA will have a sequence number of 1"
> — §2.2, `rfc4303.txt:650-651`, `rfc4303.txt:651-652`

- Strength: description. Class: wire.
- Check idea: establish a new SA, capture the first packet sent on it, and confirm its
  Sequence Number field is 1.

### RFC4303-SEQ-8

**Anti-replay is enabled by default.**

> "If anti-replay is enabled
> (the default)" — §2.2, `rfc4303.txt:653-654`

- Strength: description. Class: internal.
- Check idea: establish an SA without explicitly negotiating anti-replay and confirm the
  receiver enables it.

### RFC4303-SEQ-9

**When anti-replay is enabled, the sender's and receiver's counters must be reset, by establishing a new SA and a new key, before the sequence number would cycle at the 2^32nd packet.**

> "the transmitted sequence number must never be allowed to cycle. Thus, the sender's counter
> and the receiver's counter MUST be reset (by establishing a new SA and thus a new key) prior
> to the transmission of the 2^32nd packet on an SA." — §2.2, `rfc4303.txt:654-657`

- Strength: must. Class: internal.
- Check idea: run an SA with anti-replay enabled to near its 2^32nd packet and confirm the
  implementation establishes a new SA and key before the counter would cycle.

### RFC4303-SEQ-10

**An IPsec implementation should implement Extended Sequence Numbers as an extension to the 32-bit Sequence Number field.**

> "To support high-speed IPsec implementations, Extended Sequence Numbers (ESNs) SHOULD be
> implemented, as an extension to the current, 32-bit sequence number field." — §2.2.1,
> `rfc4303.txt:661-663`

- Strength: should. Class: internal.
- Check idea: inspect an implementation's capabilities and confirm it offers an Extended
  Sequence Number option alongside the 32-bit sequence number.

### RFC4303-SEQ-11

**Use of an Extended Sequence Number must be negotiated by an SA management protocol.**

> "Use of an ESN MUST be negotiated by an SA management protocol." — §2.2.1, `rfc4303.txt:663-664`

- Strength: must. Class: internal.
- Check idea: establish an SA and confirm whether it uses extended sequence numbers is a
  negotiated outcome of SA management, not a silent local choice.

### RFC4303-SEQ-12

**The Extended Sequence Number feature applies to multicast SAs as well as unicast SAs.**

> "The ESN feature is applicable to multicast as well as unicast SAs." — §2.2.1,
> `rfc4303.txt:666-667`

- Strength: description. Class: internal.
- Check idea: establish a multicast SA with Extended Sequence Numbers negotiated and confirm
  the feature operates as it does for a unicast SA.

### RFC4303-SEQ-13

**Only the low-order 32 bits of an extended sequence number are transmitted, in the plaintext ESP header of each packet.**

> "Only the low-order 32 bits of the sequence number are transmitted in the plaintext ESP
> header of each packet, thus minimizing packet overhead." — §2.2.1, `rfc4303.txt:681-683`

- Strength: description. Class: wire.
- Check idea: capture a packet from an SA using Extended Sequence Numbers and confirm the
  Sequence Number field on the wire is 32 bits and unencrypted.

### RFC4303-SEQ-14

**The high-order 32 bits of an extended sequence number are kept by both sender and receiver and included in the ICV computation when the integrity service is selected.**

> "The high-order 32 bits are maintained as part of the sequence number counter by both
> transmitter and receiver and are included in the computation of the ICV (if the integrity
> service is selected)." — §2.2.1, `rfc4303.txt:683-686`

- Strength: description. Class: internal.
- Check idea: under an SA using Extended Sequence Numbers with integrity selected, vary the
  high-order bits the sender holds and confirm the receiver's ICV verification depends on
  them matching.

### RFC4303-SEQ-15

**With a separate integrity algorithm, the extended sequence number's high-order bits are part of the implicit ESP trailer and are not transmitted.**

> "If a separate integrity algorithm is employed, the high order bits are included in the
> implicit ESP trailer, but are not transmitted, analogous to integrity algorithm padding
> bits." — §2.2.1, `rfc4303.txt:686-688`

- Strength: description. Class: encoding.
- Check idea: under an SA using a separate integrity algorithm and Extended Sequence Numbers,
  capture a packet and confirm no high-order sequence bits appear on the wire.

### RFC4303-SEQ-16

**With a combined mode algorithm, whether the extended sequence number's high-order bits are transmitted or included only implicitly in the computation depends on the algorithm's choice.**

> "If a combined mode algorithm is employed, the algorithm choice determines whether the
> high-order ESN bits are transmitted or are included implicitly in the computation." — §2.2.1,
> `rfc4303.txt:688-691`

- Strength: description. Class: encoding.
- Check idea: under an SA using a combined mode algorithm with Extended Sequence Numbers,
  capture a packet and confirm the presence or absence of the high-order bits on the wire
  matches what the algorithm specifies.

## Payload Data

### RFC4303-PAY-1

**The Payload Data field is a variable-length field carrying the data identified by the Next Header field.**

> "Payload Data is a variable-length field containing data (from the original IP packet)
> described by the Next Header field." — §2.3, `rfc4303.txt:696-697`

- Strength: description. Class: encoding.
- Check idea: capture an ESP packet, decrypt or read the Payload Data, and confirm its content
  matches the type named by the Next Header field.

### RFC4303-PAY-2

**The Payload Data field is mandatory and an integral number of bytes long.**

> "The Payload Data field is mandatory and is an integral number of bytes in length." — §2.3,
> `rfc4303.txt:697-698`

- Strength: description. Class: wire.
- Check idea: capture ESP packets from any SA and confirm the Payload Data field is present
  and its length in bytes is a whole number.

### RFC4303-PAY-3

**When an encryption algorithm needs explicit per-packet synchronization data, such as an IV, that data is carried inside the Payload Data field rather than as a separate ESP field, and typically immediately precedes the ciphertext.**

> "If the algorithm used to encrypt the payload requires cryptographic synchronization data,
> e.g., an Initialization Vector (IV), then this data is carried explicitly in the Payload
> field, but it is not called out as a separate field in ESP, i.e., the transmission of an
> explicit IV is invisible to ESP." and "Typically, the IV immediately precedes the
> ciphertext." — §2.3, `rfc4303.txt:699-703`, `rfc4303.txt:707`

- Strength: description. Class: encoding.
- Check idea: under an SA whose encryption algorithm needs an explicit IV, capture a packet
  and confirm the IV appears within the Payload Data field, immediately before the ciphertext,
  with no separate ESP field for it.

### RFC4303-PAY-4

**For IPv4, the next layer protocol header must begin at an offset that is a multiple of 4 bytes from the start of the ESP header.**

> "Note that the beginning of the next layer protocol header MUST be aligned relative to the
> beginning of the ESP header as follows." and "For IPv4, this alignment is a multiple of 4
> bytes." — §2.3, `rfc4303.txt:715-717`

- Strength: must. Class: encoding.
- Check idea: capture an IPv4 ESP packet, decrypt it, and confirm the next layer protocol
  header starts at an offset from the ESP header that is a multiple of 4 bytes.

### RFC4303-PAY-5

**For IPv6, the next layer protocol header must begin at an offset that is a multiple of 8 bytes from the start of the ESP header.**

> "Note that the beginning of the next layer protocol header MUST be aligned relative to the
> beginning of the ESP header as follows." and "For IPv6, the alignment is a multiple of 8
> bytes." — §2.3, `rfc4303.txt:715-716`, `rfc4303.txt:717-718`

- Strength: must. Class: encoding.
- Check idea: capture an IPv6 ESP packet, decrypt it, and confirm the next layer protocol
  header starts at an offset from the ESP header that is a multiple of 8 bytes.

## Padding

### RFC4303-PAD-1

**When an encryption algorithm requires the plaintext to be a multiple of a block size, the Padding field fills the plaintext, consisting of the Payload Data, Padding, Pad Length, and Next Header fields, to the size the algorithm needs.**

> "If an encryption algorithm is employed that requires the plaintext to be a multiple of some
> number of bytes, e.g., the block size of a block cipher, the Padding field is used to fill
> the plaintext (consisting of the Payload Data, Padding, Pad Length, and Next Header fields)
> to the size required by the algorithm." — §2.4, `rfc4303.txt:744-749`

- Strength: description. Class: encoding.
- Check idea: under an SA using a block-size-constrained encryption algorithm, capture a
  packet and confirm the total plaintext length, including Padding, Pad Length, and Next
  Header, is a multiple of the algorithm's block size.

### RFC4303-PAD-2

**The Pad Length and Next Header fields are right-aligned within a 4-byte word, so the ICV field, when present, is aligned on a 4-byte boundary.**

> "the Pad Length and Next Header fields must be right aligned within a 4-byte word, as
> illustrated in the ESP packet format figures above, to ensure that the ICV field (if
> present) is aligned on a 4-byte boundary." — §2.4, `rfc4303.txt:753-757`

- Strength: must (lower case). Class: encoding.
- Check idea: capture an ESP packet and confirm the Pad Length and Next Header fields end on a
  4-byte boundary, so any following ICV field starts on a 4-byte boundary.

### RFC4303-PAD-3

**The Padding field is too limited to be effective for traffic flow confidentiality and should not be used for that purpose; the mechanism of Traffic Flow Confidentiality padding should be used when TFC is required.**

> "the Padding field described is too limited to be effective for TFC and thus should not be
> used for that purpose. Instead, the separate mechanism described below (see Section 2.7)
> should be used when TFC is required." — §2.4, `rfc4303.txt:761-764`

- Strength: should not, should (lower case). Class: internal.
- Check idea: when an implementation needs traffic flow confidentiality, confirm it uses the
  TFC padding mechanism rather than enlarging the Padding field beyond what encryption or
  alignment requires.

### RFC4303-PAD-4

**The sender may add 0 to 255 bytes of Padding.**

> "The sender MAY add 0 to 255 bytes of padding." — §2.4, `rfc4303.txt:766`

- Strength: may. Class: wire.
- Check idea: capture packets from a sender and confirm the Padding field length observed
  falls within 0 to 255 bytes.

### RFC4303-PAD-5

**All implementations must support generating and consuming the Padding field, even though including it is optional.**

> "Inclusion of the
> Padding field in an ESP packet is optional, subject to the requirements
> noted above, but all implementations MUST support generation and consumption of padding."
> — §2.4, `rfc4303.txt:766-769`

- Strength: must. Class: end-to-end.
- Check idea: configure a sender to add Padding and confirm the receiver correctly strips it;
  confirm the same implementation, acting as sender, can produce padded packets.

### RFC4303-PAD-6

**For the block-size padding purpose, the padding computation covers the Payload Data excluding any IV, but including the ESP trailer fields.**

> "For the purpose of ensuring that the bits to be encrypted are a multiple of the algorithm's
> block size (first bullet above), the padding computation applies to the Payload Data
> exclusive of any IV, but including the ESP trailer
> fields." — §2.4, `rfc4303.txt:771-775`

- Strength: description. Class: encoding.
- Check idea: under a block-size-constrained algorithm, confirm the amount of Padding added
  accounts for the Payload Data excluding the IV plus the ESP trailer fields.

### RFC4303-PAD-7

**When a combined mode algorithm replicates the SPI, Sequence Number, or the high-order Extended Sequence Number bits inside the Payload Data for integrity, those replicated items are included in the pad-length computation.**

> "If a combined algorithm mode requires transmission of the SPI and Sequence Number to effect
> integrity, e.g., replication of the SPI and Sequence Number in the Payload Data, then the
> replicated versions of these data items, and any associated, ICV-equivalent data, are
> included in the computation of the pad length." and "If the ESN option is selected, the
> high-order 32 bits of the ESN also would enter into the computation, if the combined mode
> algorithm requires their transmission for integrity." — §2.4, `rfc4303.txt:775-780`,
> `rfc4303.txt:790-793`

- Strength: description. Class: encoding.
- Check idea: under a combined mode algorithm that replicates the SPI and Sequence Number
  within the Payload Data, confirm the Pad Length reflects those replicated bytes.

### RFC4303-PAD-8

**For the ICV-alignment padding purpose, the padding computation covers the Payload Data including the IV, the Pad Length, and Next Header fields, plus any combined-mode replicated or ICV-equivalent data.**

> "For the purposes of ensuring that the ICV is aligned on a 4-byte boundary (second bullet
> above), the padding computation applies to the Payload Data inclusive of the IV, the Pad
> Length, and Next Header fields. If a combined mode algorithm is used, any replicated data
> and ICV-equivalent data are included in the Payload Data covered by the padding
> computation." — §2.4, `rfc4303.txt:795-801`

- Strength: description. Class: encoding.
- Check idea: confirm the amount of Padding added for 4-byte ICV alignment is computed over
  the Payload Data including the IV, Pad Length, and Next Header fields.

### RFC4303-PAD-9

**When the encryption algorithm does not specify the padding contents, the sender must use the default processing: Padding bytes form a monotonically increasing integer sequence starting at 1.**

> "If Padding bytes are needed but the encryption algorithm does not specify the padding
> contents, then the following default processing MUST be used. The Padding bytes are
> initialized with a series of (unsigned, 1-byte) integer values. The first padding byte
> appended to the plaintext is numbered 1, with subsequent padding bytes making up a
> monotonically increasing sequence: 1, 2, 3, ...." — §2.4, `rfc4303.txt:803-808`

- Strength: must. Class: wire.
- Check idea: under an algorithm that does not define its own padding contents, capture a
  padded packet and confirm the Padding bytes, once decrypted, form the sequence 1, 2, 3, ...

### RFC4303-PAD-10

**When the default padding scheme is used, the receiver should inspect the Padding field.**

> "When this padding scheme is employed, the receiver SHOULD inspect the Padding field."
> — §2.4, `rfc4303.txt:808-810`

- Strength: should. Class: end-to-end.
- Check idea: send a packet with the default padding scheme but corrupted Padding bytes and
  confirm the receiver detects the mismatch when it decrypts the packet.

## Pad Length

### RFC4303-PADL-1

**The Pad Length field states the number of pad bytes immediately preceding it, a value from 0 to 255, where zero means no Padding bytes are present.**

> "The Pad Length field indicates the number of pad bytes immediately preceding it in the
> Padding field. The range of valid values is 0 to 255, where a value of zero indicates that
> no Padding bytes are
> present." — §2.5, `rfc4303.txt:824-827`

- Strength: description. Class: encoding.
- Check idea: capture an ESP packet, decrypt it, and confirm the number of Padding bytes
  immediately before the Pad Length field equals the value the Pad Length field carries.

### RFC4303-PADL-2

**The Pad Length count does not include any TFC padding bytes.**

> "As noted above, this does not include any TFC padding
> bytes." — §2.5, `rfc4303.txt:827-828`

- Strength: description. Class: encoding.
- Check idea: under an SA using TFC padding as well as ordinary Padding, confirm the Pad
  Length field counts only the Padding field's bytes, not the separate TFC padding.

### RFC4303-PADL-3

**The Pad Length field is mandatory.**

> "The Pad Length field is mandatory." — §2.5, `rfc4303.txt:828`

- Strength: description. Class: wire.
- Check idea: capture ESP packets from any SA and confirm the Pad Length field is always
  present.

## Next Header

### RFC4303-NH-1

**The Next Header field is a mandatory 8-bit field that identifies the type of data in the Payload Data field, using the IANA IP Protocol Numbers.**

> "The Next Header is a mandatory, 8-bit field that identifies the type of data contained in
> the Payload Data field, e.g., an IPv4 or IPv6 packet, or a next layer header and data. The
> value of this field is chosen from the set of IP Protocol Numbers defined on the web page of
> the IANA" — §2.6, `rfc4303.txt:849-853`

- Strength: description. Class: encoding.
- Check idea: capture an ESP packet, decrypt it, and confirm the Next Header field carries an
  IANA IP Protocol Number matching the type of data the Payload Data actually holds.

### RFC4303-NH-2

**Protocol value 59, "no next header," must be used to mark a dummy packet.**

> "the protocol value 59 (which means "no next header") MUST be used to designate a "dummy"
> packet." — §2.6, `rfc4303.txt:858-859`

- Strength: must. Class: wire.
- Check idea: capture a dummy packet generated for traffic flow confidentiality and confirm
  its Next Header field carries value 59.

### RFC4303-NH-3

**A transmitter must be capable of generating dummy packets marked with Next Header value 59.**

> "A transmitter MUST be capable of generating dummy packets marked with this value in the
> next protocol
> field" — §2.6, `rfc4303.txt:859-861`

- Strength: must. Class: end-to-end.
- Check idea: request generation of TFC dummy traffic from a sender and confirm it can produce
  packets whose Next Header field is 59.

### RFC4303-NH-4

**A receiver must be prepared to discard a dummy packet without indicating an error.**

> "a receiver MUST be prepared to discard such packets, without indicating an error." — §2.6,
> `rfc4303.txt:861-862`

- Strength: must. Class: end-to-end.
- Check idea: send a dummy packet, Next Header value 59, to a receiver and confirm it discards
  the packet and raises no error indication.

### RFC4303-NH-5

**All other ESP header and trailer fields must be present in a dummy packet, even though the plaintext payload other than the Next Header field need not be well-formed.**

> "All other ESP header and trailer fields (SPI, Sequence Number, Padding, Pad Length, Next
> Header, and ICV) MUST be present in dummy packets, but the plaintext portion of the payload,
> other than this Next Header field, need not be well-formed, e.g., the rest of the Payload
> Data may consist of only random bytes." — §2.6, `rfc4303.txt:862-866`

- Strength: must. Class: wire.
- Check idea: capture a dummy packet and confirm the SPI, Sequence Number, Padding, Pad
  Length, Next Header, and ICV fields are all present, regardless of whether the rest of the
  Payload Data is well-formed.

### RFC4303-NH-6

**Implementations should provide local management controls, including parametric controls, to enable dummy-packet generation on a per-SA basis.**

> "Implementations SHOULD provide local management controls to enable the use of this
> capability on a per-SA basis." and "Implementations SHOULD provide controls to enable local
> administrators to manage the generation of dummy packets for TFC purposes." — §2.6,
> `rfc4303.txt:869-870`, `rfc4303.txt:889-891`

- Strength: should. Class: internal.
- Check idea: inspect an implementation's management interface and confirm it offers a per-SA
  control to enable or disable, and parametrize, dummy-packet generation.

## Traffic Flow Confidentiality (TFC) padding

### RFC4303-TFC-1

**An optional field within the Payload Data addresses the traffic flow confidentiality requirement, because the 255-byte Padding field is not adequate for it.**

> "As noted above, the Padding field is limited to 255 bytes in length. This generally will
> not be adequate to hide traffic characteristics relative to traffic flow confidentiality
> requirements. An optional field, within the payload data, is provided specifically to
> address the TFC requirement." — §2.7, `rfc4303.txt:905-909`

- Strength: description. Class: encoding.
- Check idea: under an SA that needs to hide traffic characteristics beyond what 255 bytes of
  Padding can conceal, confirm the implementation offers a separate, larger padding field
  within the Payload Data for that purpose.

### RFC4303-TFC-2

**An IPsec implementation should be capable of adding TFC padding bytes after the end of the Payload Data and before the Padding field.**

> "An IPsec implementation SHOULD be capable of padding traffic by adding bytes after the end
> of the Payload Data, prior to the beginning of the Padding field." — §2.7,
> `rfc4303.txt:911-913`

- Strength: should. Class: wire.
- Check idea: configure TFC padding on an SA and capture a packet; confirm the added padding
  bytes lie between the end of the true Payload Data and the start of the Padding field.

### RFC4303-TFC-3

**TFC padding can be added only if the Payload Data field carries a specification of the IP datagram's length, which always holds in tunnel mode and may hold in transport mode depending on the next layer protocol.**

> "this padding (hereafter referred to as TFC padding) can be added only if the Payload Data
> field contains a specification of the length of the IP datagram. This is always true in
> tunnel mode, and may be true in transport mode depending on whether the next layer protocol
> (e.g., IP, UDP, ICMP) contains explicit length information." — §2.7, `rfc4303.txt:913-918`

- Strength: may (lower case). Class: internal.
- Check idea: in transport mode with a next layer protocol that carries no explicit length
  field, confirm the implementation does not add TFC padding; in tunnel mode, confirm it can.

### RFC4303-TFC-4

**The receiver locates the ESP trailer fields by counting back from the end of the ESP packet, and uses the IP datagram's known length to discard TFC padding.**

> "This length information will enable the receiver to discard the TFC padding, because the
> true length of the Payload Data will be known. (ESP trailer fields are located by counting
> back from the end of the ESP packet.)" — §2.7, `rfc4303.txt:918-921`

- Strength: description. Class: end-to-end.
- Check idea: send a packet with TFC padding whose IP datagram carries an explicit length
  field, and confirm the receiver strips exactly the padding bytes and recovers the true
  payload.

### RFC4303-TFC-5

**When TFC padding is added, the field carrying the IP datagram's length must not be modified to reflect the padding.**

> "Accordingly, if TFC padding is added, the field containing the specification of the length
> of the IP datagram MUST NOT be modified to reflect this padding." — §2.7,
> `rfc4303.txt:922-924`

- Strength: must not. Class: wire.
- Check idea: capture a packet with TFC padding added and confirm the IP datagram's own length
  field still states the true, unpadded length.

### RFC4303-TFC-6

**No requirement is established for the value of the TFC padding bytes.**

> "No requirements for the value of this padding are established by this standard." — §2.7,
> `rfc4303.txt:924-925`

- Strength: description. Class: wire.
- Check idea: capture TFC padding bytes from an implementation and confirm the standard places
  no constraint on their content, so any byte values are acceptable.

### RFC4303-TFC-7

**An SA management protocol must negotiate use of TFC padding before a transmitter employs it, to keep backward compatibility.**

> "the SA management protocol MUST negotiate this service prior to a transmitter employing it,
> to ensure backward compatibility." — §2.7, `rfc4303.txt:930-931`

- Strength: must. Class: internal.
- Check idea: observe SA establishment and confirm TFC padding use is a negotiated outcome,
  not silently begun by the transmitter without agreement from the receiver.

### RFC4303-TFC-8

**Implementations should provide local management controls, including parametric controls, to enable TFC padding on a per-SA basis.**

> "Implementations SHOULD provide local management controls to enable the use of this
> capability on a per-SA basis. The controls should allow the user to specify if this feature
> is to be used and also provide parametric controls for the feature." — §2.7,
> `rfc4303.txt:937-940`

- Strength: should. Class: internal.
- Check idea: inspect an implementation's management interface and confirm it offers a per-SA
  control to enable or disable, and parametrize, TFC padding.

## Integrity Check Value (ICV)

### RFC4303-ICV-1

**The ICV is a variable-length field computed over the ESP header, the Payload, and the ESP trailer fields.**

> "The Integrity Check Value is a variable-length field computed over the ESP header, Payload,
> and ESP trailer fields." — §2.8, `rfc4303.txt:944-945`

- Strength: description. Class: encoding.
- Check idea: under an SA with integrity selected, corrupt a byte in the ESP header, the
  Payload, or the ESP trailer in turn and confirm the ICV verification fails each time.

### RFC4303-ICV-2

**Implicit ESP trailer fields, integrity-algorithm padding and high-order Extended Sequence Number bits when applicable, are included in the ICV computation.**

> "Implicit ESP trailer fields (integrity padding and high-order ESN bits, if applicable) are
> included in the ICV computation." — §2.8, `rfc4303.txt:945-947`

- Strength: description. Class: internal.
- Check idea: under an SA using Extended Sequence Numbers, vary the high-order sequence bits
  without changing anything transmitted and confirm the ICV verification result depends on
  them.

### RFC4303-ICV-3

**The ICV field is optional: present only when the integrity service is selected, and provided by a separate integrity algorithm or by a combined mode algorithm that uses an ICV.**

> "The ICV field is optional. It is present only if the integrity service is selected and is
> provided by either a separate integrity algorithm or a combined mode algorithm that uses an
> ICV." — §2.8, `rfc4303.txt:947-950`

- Strength: description. Class: wire.
- Check idea: capture packets from an SA with no integrity service selected and confirm no ICV
  field is present; capture packets from an SA with integrity selected and confirm it is.

### RFC4303-ICV-4

**The length of the ICV field is set by the integrity algorithm associated with the SA.**

> "The length of the field is" and "specified by the integrity algorithm selected and associated
> with the
> SA." — §2.8, `rfc4303.txt:950`, `rfc4303.txt:959-960`

- Strength: description. Class: encoding.
- Check idea: capture ICV fields from SAs using different integrity algorithms and confirm
  each field's length matches the length its algorithm specifies.

## The location of ESP

### RFC4303-LOC-1

**ESP operates in one of two modes: transport mode or tunnel mode.**

> "ESP may be employed in two ways: transport mode or tunnel mode." — §3.1, `rfc4303.txt:967`

- Strength: may (lower case). Class: internal.
- Check idea: configure an SA and confirm it operates in exactly one of transport mode or
  tunnel mode.

### RFC4303-LOC-2

**In transport mode, ESP is inserted after the IP header and before the next layer protocol header.**

> "In transport mode, ESP is inserted after the IP header and before a next layer protocol,
> e.g., TCP, UDP, ICMP, etc." — §3.1.1, `rfc4303.txt:971-972`

- Strength: description. Class: wire.
- Check idea: capture a transport-mode ESP packet and confirm the ESP header sits between the
  IP header and the next layer protocol header.

### RFC4303-LOC-3

**In IPv4 transport mode, ESP is placed after the IP header, including any options it contains, and before the next layer protocol.**

> "In the context of IPv4, this translates to placing ESP after the IP header (and any options
> that it contains), but before the next layer protocol." — §3.1.1, `rfc4303.txt:972-974`

- Strength: description. Class: wire.
- Check idea: capture an IPv4 transport-mode ESP packet whose IP header carries options and
  confirm the ESP header follows those options and precedes the next layer protocol.

### RFC4303-LOC-4

**When AH is also applied to a transport-mode packet, AH covers the ESP header, Payload, ESP trailer, and the ICV if present.**

> "If AH is also applied to a packet, it is applied to the ESP header, Payload, ESP trailer,
> and ICV, if present." — §3.1.1, `rfc4303.txt:974-976`

- Strength: description. Class: wire.
- Check idea: capture a packet combining AH and ESP and confirm AH's coverage spans the ESP
  header, Payload, ESP trailer, and ICV when the ICV is present.

### RFC4303-LOC-5

**In transport mode, encryption covers the next layer protocol header and data plus the ESP trailer; integrity covers the ESP header, the next layer protocol header and data, and the ESP trailer, but not the ICV itself.**

> "|<---- encryption ---->|" and "|<-------- integrity ------->|" — §3.1.1,
> `rfc4303.txt:995-996`

- Strength: description. Class: encoding.
- Check idea: under transport mode, confirm that decrypting a captured packet recovers the
  next layer protocol header, data, and ESP trailer, and that the ICV verification covers the
  ESP header through the ESP trailer but excludes the ICV field itself.

### RFC4303-LOC-6

**In IPv6, ESP is placed after the hop-by-hop, routing, and fragmentation extension headers.**

> "In the IPv6 context, ESP is viewed as an end-to-end payload, and thus should appear after
> hop-by-hop, routing, and fragmentation extension headers." — §3.1.1, `rfc4303.txt:998-1000`

- Strength: should (lower case). Class: wire.
- Check idea: capture an IPv6 transport-mode ESP packet carrying hop-by-hop, routing, and
  fragmentation extension headers and confirm the ESP header follows all of them.

### RFC4303-LOC-7

**A destination options header may appear before the ESP header, after it, or both, but placing it after ESP is generally desirable, since ESP protects only what follows the ESP header.**

> "Destination options extension header(s) could appear before, after, or both before and
> after the ESP header depending on the semantics desired. However, because ESP protects only
> fields after the ESP header, it generally will be desirable to place the destination options
> header(s) after the ESP header." — §3.1.1, `rfc4303.txt:1000-1004`

- Strength: description. Class: wire.
- Check idea: capture IPv6 transport-mode ESP packets with a destination options header placed
  after the ESP header and confirm ESP's protection covers it.

### RFC4303-LOC-8

**In transport mode, a bump-in-the-stack or bump-in-the-wire implementation may need extra IP reassembly and fragmentation of inbound and outbound fragments, to conform to this specification while staying transparent.**

> "Note that in transport mode, for "bump-in-the-stack" or "bump-in- the-wire" implementations,
> as defined in the Security Architecture document, inbound and outbound IP fragments may
> require an IPsec implementation to perform extra IP reassembly/fragmentation in order to both
> conform to this specification and provide transparent IPsec
> support." — §3.1.1,
> `rfc4303.txt:1031-1036`

- Strength: may (lower case). Class: internal.
- Check idea: send fragmented IP traffic through a bump-in-the-stack or bump-in-the-wire
  implementation and confirm it reassembles inbound fragments and re-fragments outbound
  traffic as needed around ESP processing.

### RFC4303-LOC-9

**In tunnel mode, the inner IP header carries the ultimate source and destination addresses, and the outer IP header carries the addresses of the IPsec peers.**

> "In tunnel mode, the "inner" IP header carries the ultimate (IP) source and destination
> addresses, while an "outer" IP header contains the addresses of the IPsec "peers", e.g.,
> addresses of security
> gateways." — §3.1.2, `rfc4303.txt:1041-1044`

- Strength: description. Class: wire.
- Check idea: capture a tunnel-mode ESP packet and confirm the outer IP header carries the
  IPsec peer addresses while the inner IP header, once decrypted, carries the ultimate source
  and destination addresses.

### RFC4303-LOC-10

**Tunnel mode allows mixed inner and outer IP versions: IPv6 over IPv4, and IPv4 over IPv6.**

> "Mixed inner and outer IP versions are allowed, i.e., IPv6 over IPv4 and IPv4 over IPv6."
> — §3.1.2, `rfc4303.txt:1044-1045`

- Strength: description. Class: wire.
- Check idea: establish a tunnel-mode SA with an inner IP version different from the outer,
  send traffic, and confirm the packet is accepted and processed correctly.

### RFC4303-LOC-11

**In tunnel mode, ESP protects the entire inner IP packet, including the entire inner IP header.**

> "In tunnel mode, ESP protects the entire inner IP packet, including the entire inner IP
> header." — §3.1.2, `rfc4303.txt:1045-1046`

- Strength: description. Class: encoding.
- Check idea: capture a tunnel-mode ESP packet and confirm the encryption or integrity
  coverage spans the whole inner IP packet, header included.

### RFC4303-LOC-12

**The position of ESP in tunnel mode, relative to the outer IP header, is the same as in transport mode.**

> "The position of ESP in tunnel mode, relative to the outer IP header, is the same as for ESP
> in transport mode." — §3.1.2, `rfc4303.txt:1046-1048`

- Strength: description. Class: wire.
- Check idea: capture tunnel-mode and transport-mode ESP packets and confirm ESP sits in the
  same position relative to the (outer) IP header in both.

### RFC4303-LOC-13

**In tunnel mode, encryption covers the entire inner IP packet plus the ESP trailer; integrity covers the ESP header, the entire inner IP packet, and the ESP trailer, but not the ICV itself.**

> "|<--------- encryption --------->|" and "|<------------- integrity ------------>|"
> — §3.1.2, `rfc4303.txt:1083-1084`

- Strength: description. Class: encoding.
- Check idea: under tunnel mode, confirm that decrypting a captured packet recovers the entire
  inner IP packet and the ESP trailer, and that ICV verification covers the ESP header through
  the ESP trailer but excludes the ICV field itself.

## Algorithms

### RFC4303-ALG-1

**An ESP implementation may support algorithms beyond those mandated for ESP.**

> "Additional algorithms,
>    beyond those mandated for ESP, MAY be supported." — §3.2, `rfc4303.txt:1110-1111`

- Strength: may. Class: internal.
- Check idea: configure an SA with an encryption or integrity algorithm outside the mandatory-to-implement set and check that the implementation processes packets on it.

### RFC4303-ALG-2

**An SA must select at least one of confidentiality or integrity, so its encryption and integrity algorithms are never both NULL at the same time.**

> "at least one of
>    these services MUST be selected, hence both algorithms MUST NOT be
>    simultaneously NULL." — §3.2, `rfc4303.txt:1112-1114`

- Strength: must (selecting at least one service); must not (setting both algorithms to NULL at the same time). Class: internal.
- Check idea: inspect the algorithms configured for an SA and check that they are never both NULL at once.

### RFC4303-ALG-3

**The SA that carries a packet specifies the encryption algorithm that protects that packet.**

> "The encryption algorithm employed to protect an ESP packet is
>    specified by the SA via which the packet is transmitted/received." — §3.2.1, `rfc4303.txt:1129-1130`

- Strength: description. Class: internal.
- Check idea: inspect the SA used for a packet and check that it names the encryption algorithm applied to that packet.

### RFC4303-ALG-4

**Each ESP packet carries, or lets the receiver derive, the data the receiver needs for cryptographic synchronization, because packets can be lost or arrive out of order.**

> "each packet must carry any data required to
>    allow the receiver to establish cryptographic synchronization for
>    decryption.  This data may be carried explicitly in the payload
>    field, e.g., as an IV (as described above), or the data may be
>    derived from the plaintext portions of the (outer IP or ESP) packet
>    header." — §3.2.1, `rfc4303.txt:1132-1137`

- Strength: must, may (lower case). Class: wire.
- Check idea: capture ESP packets sent out of order or with gaps and check that each one carries, or lets the receiver derive, the data needed for cryptographic synchronization.

### RFC4303-ALG-5

**The encryption algorithm for an SA may be NULL, because confidentiality is an optional ESP service.**

> "because encryption (confidentiality) MAY
>    be an optional service (e.g., integrity-only ESP), this algorithm MAY
>    be "NULL" [Ken-Arch]." — §3.2.1, `rfc4303.txt:1147-1149`

- Strength: may. Class: wire.
- Check idea: configure an SA with the NULL encryption algorithm and check that the ESP payload leaves the sender unencrypted.

### RFC4303-ALG-6

**The integrity algorithm for an SA may be NULL, because the integrity service may be optional.**

> "the integrity service MAY be optional, this algorithm may be "NULL"." — §3.2.2, `rfc4303.txt:1165`

- Strength: may (the integrity service being optional); may (lower case, the algorithm being NULL). Class: wire.
- Check idea: configure an SA with the NULL integrity algorithm and check that the receiver accepts packets on that SA without an ICV check.

### RFC4303-ALG-7

**The SA that carries a packet specifies the integrity algorithm that computes the ICV.**

> "The integrity algorithm employed for the ICV computation is specified
>    by the SA via which the packet is transmitted/received." — §3.2.2, `rfc4303.txt:1158-1159`

- Strength: description. Class: internal.
- Check idea: inspect the SA used for a packet and check that it names the integrity algorithm that computes the ICV.

### RFC4303-ALG-8

**An integrity algorithm used with ESP allows the receiver to process packets that arrive out of order or are lost.**

> "any integrity algorithm employed with
>    ESP must make provisions to permit processing of packets that arrive
>    out of order and to accommodate packet loss." — §3.2.2, `rfc4303.txt:1160-1162`

- Strength: must (lower case). Class: internal.
- Check idea: send packets on an SA out of order or with some missing, and check that the receiver's integrity algorithm still verifies the packets that arrive.

### RFC4303-ALG-9

**A combined mode algorithm, when an SA uses one, provides both confidentiality and integrity.**

> "If a combined mode algorithm is employed, both confidentiality and
>    integrity services are provided." — §3.2.3, `rfc4303.txt:1185-1186`

- Strength: description. Class: internal.
- Check idea: configure an SA with a combined mode algorithm and check that both encryption and integrity protection apply to its packets.

### RFC4303-ALG-10

**A combined mode algorithm used with ESP makes provisions for per-packet cryptographic synchronization, so decryption works despite out-of-order arrival or packet loss.**

> "a combined mode algorithm must make provisions for per-
>    packet cryptographic synchronization, to permit decryption of packets
>    that arrive out of order and to accommodate packet loss." — §3.2.3, `rfc4303.txt:1187-1189`

- Strength: must (lower case). Class: internal.
- Check idea: send packets on a combined-mode SA out of order or with some missing, and check that the receiver still decrypts the packets that arrive.

### RFC4303-ALG-11

**This specification defines no payload substructure for a combined mode algorithm, so its invocation stays uniform and algorithm-independent.**

> "In order to provide a uniform,
>    algorithm-independent approach to invocation of combined mode
>    algorithms, no payload substructure is defined." — §3.2.3, `rfc4303.txt:1192-1194`

- Strength: description. Class: encoding.
- Check idea: capture the ESP payload of a packet protected by a combined mode algorithm and check that this specification imposes no particular internal layout on it.

### RFC4303-ALG-12

**No internal detail of a combined mode algorithm's payload layout should be observable externally.**

> "None of
>    these details should be observable externally." — §3.2.3, `rfc4303.txt:1196-1197`

- Strength: should (lower case). Class: wire.
- Check idea: capture packets from SAs that use different combined mode algorithms or internal layouts and check that no such internal detail is distinguishable on the link.

## Outbound processing and SA lookup

### RFC4303-OSA-1

**In transport mode, the sender places the original next layer protocol information between the ESP header and the ESP trailer, and keeps the specified IP header and any IPv6 extension headers unchanged.**

> "In transport mode, the sender encapsulates the next layer protocol
>    information between the ESP header and the ESP trailer fields, and
>    retains the specified IP header (and any IP extension headers in the
>    IPv6 context)." — §3.3, `rfc4303.txt:1206-1209`

- Strength: description. Class: wire.
- Check idea: capture an outbound transport-mode ESP packet and check that the original next layer protocol data sits between the ESP header and trailer, with the IP header, and any IPv6 extension headers, unchanged.

### RFC4303-OSA-2

**An IPsec implementation applies ESP to an outbound packet only after it determines that the packet is associated with an SA that calls for ESP processing.**

> "ESP is applied to an outbound packet only after an IPsec
>    implementation determines that the packet is associated with an SA
>    that calls for ESP processing." — §3.3.1, `rfc4303.txt:1217-1219`

- Strength: description. Class: internal.
- Check idea: send an outbound packet that matches no ESP-requiring SA and check that the implementation does not apply ESP to it.

## Encryption and ICV calculation

### RFC4303-OENC-1

**The sender encapsulates just the original next layer protocol information into the ESP Payload field, in transport mode, whether it uses separate encryption and integrity algorithms or a combined mode algorithm.**

> "for transport mode -- just the original next layer
>                   protocol information." — §3.3.2.1, `rfc4303.txt:1245-1246` and "for transport mode -- just the original next layer
>                   protocol information." — §3.3.2.2, `rfc4303.txt:1319-1320`

- Strength: description. Class: wire.
- Check idea: capture an outbound transport-mode ESP packet and, after decryption, check that the ESP Payload field holds only the original next layer protocol data.

### RFC4303-OENC-2

**The sender encapsulates the entire original IP datagram into the ESP Payload field, in tunnel mode, whether it uses separate encryption and integrity algorithms or a combined mode algorithm.**

> "for tunnel mode -- the entire original IP datagram." — §3.3.2.1, `rfc4303.txt:1247` and "for tunnel mode -- the entire original IP datagram." — §3.3.2.2, `rfc4303.txt:1321`

- Strength: description. Class: wire.
- Check idea: capture an outbound tunnel-mode ESP packet and, after decryption, check that the ESP Payload field holds the entire original IP datagram.

### RFC4303-OENC-3

**The sender adds any necessary padding after encapsulating the payload — optional TFC padding and encryption Padding — whether it uses separate encryption and integrity algorithms or a combined mode algorithm.**

> "Add any necessary padding -- Optional TFC padding and
>             (encryption) Padding" — §3.3.2.1, `rfc4303.txt:1249-1250` and "Add any necessary padding -- includes optional TFC padding
>             and (encryption) Padding." — §3.3.2.2, `rfc4303.txt:1323-1324`

- Strength: description. Class: wire.
- Check idea: capture an outbound ESP packet configured for traffic flow confidentiality and check that, after decryption, it carries TFC padding and encryption Padding as needed.

### RFC4303-OENC-4

**The sender encrypts the encapsulated result using the key, encryption algorithm, and algorithm mode of the SA, together with any required cryptographic synchronization data.**

> "Encrypt the result using the key, encryption algorithm,
>             and algorithm mode specified for the SA and using any
>             required cryptographic synchronization data." — §3.3.2.1, `rfc4303.txt:1252-1254`

- Strength: description. Class: end-to-end.
- Check idea: configure an SA with a given key, encryption algorithm, and mode, send a packet, and check that the receiver only decrypts it correctly with the matching SA parameters.

### RFC4303-OENC-5

**When explicit cryptographic synchronization data such as an IV is used, the sender inputs it to the encryption algorithm and places it in the Payload field.**

> "If explicit cryptographic synchronization data,
>                   e.g., an IV, is indicated, it is input to the
>                   encryption algorithm per the algorithm specification
>                   and placed in the Payload field." — §3.3.2.1, `rfc4303.txt:1255-1258`

- Strength: description. Class: wire.
- Check idea: capture an outbound ESP packet from an SA that uses explicit synchronization data and check that the Payload field carries it, for example an IV.

### RFC4303-OENC-6

**When implicit cryptographic synchronization data is used, the sender constructs it and inputs it to the encryption algorithm, whether it uses separate encryption and integrity algorithms or a combined mode algorithm.**

> "If implicit cryptographic synchronization data is
>                   employed, it is constructed and input to the
>                   encryption algorithm as per the algorithm
>                   specification." — §3.3.2.1, `rfc4303.txt:1259-1262` and "If implicit cryptographic synchronization data is
>                   employed, it is constructed and input to the
>                   encryption algorithm as per the algorithm
>                   specification." — §3.3.2.2, `rfc4303.txt:1333-1336`

- Strength: description. Class: internal.
- Check idea: configure an SA that uses implicit synchronization data and check that the receiver can reconstruct the value the sender used, without it appearing as a separate field.

### RFC4303-OENC-7

**When integrity is selected, the sender encrypts before it applies the integrity algorithm, and the encryption never covers the ICV field.**

> "If integrity is selected, encryption is performed
>                   first, before the integrity algorithm is applied, and
>                   the encryption does not encompass the ICV field." — §3.3.2.1, `rfc4303.txt:1263-1266`

- Strength: description. Class: internal.
- Check idea: capture an outbound ESP packet from an SA with both services selected and check that the ICV field itself is not encrypted.

### RFC4303-OENC-8

**The sender uses a keyed integrity algorithm to compute the ICV, because encryption does not protect the ICV itself.**

> "Note that because the ICV is not protected
>                   by encryption, a keyed integrity algorithm must be
>                   employed to compute the ICV." — §3.3.2.1, `rfc4303.txt:1273-1275`

- Strength: must (lower case). Class: internal.
- Check idea: inspect the integrity algorithm configured for an SA and check that it is a keyed algorithm.

### RFC4303-OENC-9

**The sender computes the ICV over the whole ESP packet except the ICV field, covering the SPI, Sequence Number, Payload Data, Padding, Pad Length, and Next Header in ciphertext form.**

> "Compute the ICV over the ESP packet minus the ICV field.
>             Thus, the ICV computation encompasses the SPI, Sequence
>             Number, Payload Data, Padding (if present), Pad Length, and
>             Next Header.  (Note that the last 4 fields will be in
>             ciphertext form, because encryption is performed first.)" — §3.3.2.1, `rfc4303.txt:1277-1281`

- Strength: description. Class: internal.
- Check idea: recompute the ICV over a captured ESP packet, using the fields this specification names, and check that it matches the transmitted ICV.

### RFC4303-OENC-10

**When the ESN option is enabled, the sender includes the high-order 32 bits of the sequence number in the ICV computation, appended after the Next Header field, but does not transmit them.**

> "If
>             the ESN option is enabled for the SA, the high-order 32
>             bits of the sequence number are appended after the Next
>             Header field for purposes of this computation, but are not
>             transmitted." — §3.3.2.1, `rfc4303.txt:1281-1285`

- Strength: description. Class: internal.
- Check idea: configure an SA with ESN enabled, capture a packet, and check that its Sequence Number field carries only the low-order 32 bits while the ICV still verifies against the full 64-bit value.

### RFC4303-OENC-11

**For some integrity algorithms, the byte string the ICV is computed over must be a multiple of the algorithm's block size.**

> "For some integrity algorithms, the byte string over which the ICV
>    computation is performed must be a multiple of a block size specified
>    by the algorithm." — §3.3.2.1, `rfc4303.txt:1295-1297`

- Strength: must (lower case). Class: encoding.
- Check idea: configure an SA with a block-oriented integrity algorithm and check that the byte string the ICV is computed over is a multiple of the algorithm's block size.

### RFC4303-OENC-12

**When the ESP packet length does not match the integrity algorithm's block size, the sender must append implicit padding, after the Next Header field or the ESN high-order bits, that it does not transmit.**

> "implicit padding MUST be appended to the end of the ESP packet.
>    (This padding is added after the Next Header field, or after the
>    high-order 32 bits of the sequence number, if ESN is selected.)  The
>    block size (and hence the length of the padding) is specified by the
>    integrity algorithm specification.  This padding is not transmitted
>    with the packet." — §3.3.2.1, `rfc4303.txt:1299-1304`

- Strength: must. Class: internal.
- Check idea: configure an SA whose ESP packet length does not match the integrity algorithm's block size and check that the transmitted packet is not lengthened, while the ICV still verifies as if padding had been included.

### RFC4303-OENC-13

**When the integrity algorithm's defining document does not say whether implicit padding is needed, the sender and receiver assume that it is needed to match the algorithm's block size.**

> "If the document does not specify an answer to this
>    question, then the default is to assume that implicit padding is
>    required (as needed to match the packet length to the algorithm's
>    block size.)" — §3.3.2.1, `rfc4303.txt:1306-1309`

- Strength: description. Class: internal.
- Check idea: configure an SA with an integrity algorithm whose defining document is silent on implicit padding and check that both ends still compute a matching ICV using implicit padding.

### RFC4303-OENC-14

**When padding octets are needed and the algorithm does not specify their contents, the sender must set them to zero.**

> "If padding bytes are needed but the algorithm does not
>    specify the padding contents, then the padding octets MUST have a
>    value of zero." — §3.3.2.1, `rfc4303.txt:1309-1311`

- Strength: must. Class: internal.
- Check idea: configure an SA that needs implicit padding octets with an algorithm that does not define their contents, and check that the padding used in the ICV computation is all zero.

### RFC4303-OENC-15

**The sender encrypts and integrity-protects the encapsulated result together, using the key and combined mode algorithm of the SA and any required cryptographic synchronization data.**

> "Encrypt and integrity protect the result using the key
>             and combined mode algorithm specified for the SA and using
>             any required cryptographic synchronization data." — §3.3.2.2, `rfc4303.txt:1326-1328`

- Strength: description. Class: end-to-end.
- Check idea: configure an SA with a combined mode algorithm, send a packet, and check that the receiver only accepts it when both decryption and the integrity check succeed together.

### RFC4303-OENC-16

**When explicit cryptographic synchronization data such as an IV is used, the sender inputs it to the combined mode algorithm and places it in the Payload field.**

> "If explicit cryptographic synchronization data,
>                   e.g., an IV, is indicated, it is input to the
>                   combined mode algorithm per the algorithm
>                   specification and placed in the Payload field." — §3.3.2.2, `rfc4303.txt:1329-1332`

- Strength: description. Class: wire.
- Check idea: capture an outbound ESP packet from a combined-mode SA that uses explicit synchronization data and check that the Payload field carries it.

### RFC4303-OENC-17

**The Sequence Number, or Extended Sequence Number, and the SPI are inputs to the combined mode algorithm, because the integrity check must cover them.**

> "The Sequence Number (or Extended Sequence Number, as
>                   appropriate) and the SPI are inputs to the
>                   algorithm, as they must be included in the integrity
>                   check computation." — §3.3.2.2, `rfc4303.txt:1337-1340`

- Strength: must (lower case). Class: internal.
- Check idea: alter the SPI or Sequence Number of a captured combined-mode ESP packet and check that the receiver's integrity check then fails.

### RFC4303-OENC-18

**The ESP packet format may include an explicit ICV field when a combined mode algorithm is used.**

> "The (explicit) ICV field MAY be a part of the ESP
>                   packet format when a combined mode algorithm is
>                   employed." — §3.3.2.2, `rfc4303.txt:1353-1355`

- Strength: may. Class: encoding.
- Check idea: capture an ESP packet from a combined-mode SA and check whether an explicit ICV field is present at the end of the packet.

## Sequence number generation

### RFC4303-OSEQ-1

**The sender's per-SA counter starts at 0 and increments for each packet, so the first packet sent on a new SA carries sequence number 1.**

> "The sender's counter is initialized to 0 when an SA is established.
>    The sender increments the sequence number (or ESN) counter for this
>    SA and inserts the low-order 32 bits of the value into the Sequence
>    Number field.  Thus, the first packet sent using a given SA will
>    contain a sequence number of 1." — §3.3.3, `rfc4303.txt:1365-1369`

- Strength: description. Class: wire.
- Check idea: establish a new SA, send packets on it, and check that the first one carries Sequence Number 1 and each later one increments by one.

### RFC4303-OSEQ-2

**When anti-replay is enabled, the sender checks that its counter has not cycled before it inserts a new value into the Sequence Number field.**

> "If anti-replay is enabled (the default), the sender checks to ensure
>    that the counter has not cycled before inserting the new value in the
>    Sequence Number field." — §3.3.3, `rfc4303.txt:1371-1373`

- Strength: description. Class: internal.
- Check idea: drive an SA's sequence counter close to its maximum value and check that the sender verifies against cycling before it sends each further packet.

### RFC4303-OSEQ-3

**The sender must not send a packet on an SA if doing so would cycle the sequence number.**

> "the sender MUST NOT send a
>    packet on an SA if doing so would cause the sequence number to cycle." — §3.3.3, `rfc4303.txt:1373-1374`

- Strength: must not. Class: wire.
- Check idea: drive an SA's sequence counter to its maximum value and check that the sender sends no further packet on that SA once sending one would cycle the counter.

### RFC4303-OSEQ-4

**An attempt to send a packet that would overflow the sequence number is an auditable event.**

> "An attempt to transmit a packet that would result in sequence number
>    overflow is an auditable event." — §3.3.3, `rfc4303.txt:1375-1376`

- Strength: description. Class: internal.
- Check idea: drive an SA's counter to overflow and check that the sender logs the attempted transmission as an auditable event.

### RFC4303-OSEQ-5

**The audit log entry for a sequence number overflow attempt should include the SPI, the current date and time, the source and destination addresses, and, for IPv6, the cleartext Flow ID.**

> "The audit log entry for this event
>    SHOULD include the SPI value, current date/time, Source Address,
>    Destination Address, and (in IPv6) the cleartext Flow ID." — §3.3.3, `rfc4303.txt:1376-1378`

- Strength: should. Class: internal.
- Check idea: force a sequence number overflow attempt and check that the resulting audit log entry carries the SPI, date/time, source and destination addresses, and, for IPv6, the Flow ID.

### RFC4303-OSEQ-6

**The sender assumes anti-replay is enabled by default, unless the receiver has notified it otherwise.**

> "The sender assumes anti-replay is enabled as a default, unless
>    otherwise notified by the receiver (see Section 3.4.3)." — §3.3.3, `rfc4303.txt:1380-1381`

- Strength: description. Class: internal.
- Check idea: establish a new SA without any receiver notification and check that the sender behaves as though anti-replay is enabled.

### RFC4303-OSEQ-7

**A sender typically establishes a new SA when its Sequence Number or ESN cycles, or in anticipation of that.**

> "typical behavior of an ESP implementation calls for the sender to
>    establish a new SA when the Sequence Number (or ESN) cycles, or in
>    anticipation of this value cycling." — §3.3.3, `rfc4303.txt:1382-1384`

- Strength: description. Class: internal.
- Check idea: drive an SA's sequence counter toward its maximum value and check that the sender establishes a replacement SA at or before cycling.

### RFC4303-OSEQ-8

**When the key used to compute an ICV is manually distributed, a compliant implementation should not provide anti-replay service.**

> "If the key used to compute an ICV is manually distributed, a
>    compliant implementation SHOULD NOT provide anti-replay service." — §3.3.3, `rfc4303.txt:1386-1387`

- Strength: should not. Class: internal.
- Check idea: configure an SA with a manually distributed key and check that the implementation does not offer anti-replay service for it.

### RFC4303-OSEQ-9

**When a user chooses anti-replay for a manually keyed SA, the sequence number counter at the sender must stay correct across local reboots, until the key is replaced.**

> "the sequence number counter at the sender MUST be
>    correctly maintained across local reboots, etc., until the key is
>    replaced." — §3.3.3, `rfc4303.txt:1389-1391`

- Strength: must. Class: internal.
- Check idea: reboot a sender that uses anti-replay with a manually keyed SA and check that its sequence number counter resumes from where it left off, until the key changes.

### RFC4303-OSEQ-10

**When anti-replay is disabled, the sender does not need to monitor or reset its counter, but it keeps incrementing it and lets it roll over to zero at the maximum value.**

> "the sender does not need
>    to monitor or reset the counter.  However, the sender still
>    increments the counter and when it reaches the maximum value, the
>    counter rolls over back to zero." — §3.3.3, `rfc4303.txt:1393-1396`

- Strength: description. Class: wire.
- Check idea: disable anti-replay on an SA, drive its counter to the maximum value, and check that the Sequence Number field wraps to zero rather than the sender establishing a new SA.

### RFC4303-OSEQ-11

**Letting the counter roll over is the recommended behavior for a multi-sender multicast SA, unless the sender and receiver have negotiated an anti-replay mechanism of their own.**

> "This behavior is recommended for
>    multi-sender, multicast SAs, unless anti-replay mechanisms outside
>    the scope of this standard are negotiated between the sender and
>    receiver.)" — §3.3.3, `rfc4303.txt:1396-1397`, `rfc4303.txt:1407-1408`

- Strength: description. Class: internal.
- Check idea: run a multi-sender multicast SA with anti-replay disabled and check that a member's counter rolls over to zero rather than the SA being replaced.

### RFC4303-OSEQ-12

**When ESN is selected, only the low-order 32 bits of the sequence number go on the wire, while sender and receiver each keep a full 64-bit ESN counter.**

> "If ESN (see Appendix) is selected, only the low-order 32 bits of the
>    sequence number are transmitted in the Sequence Number field,
>    although both sender and receiver maintain full 64-bit ESN counters." — §3.3.3, `rfc4303.txt:1410-1412`

- Strength: description. Class: wire.
- Check idea: configure an SA with ESN and check that a captured packet's Sequence Number field carries only 32 bits, while sender and receiver track a 64-bit value internally.

### RFC4303-OSEQ-13

**The high-order 32 bits of the ESN enter the integrity check in a way that depends on the algorithm or mode, for example appended after the Next Header field when a separate integrity algorithm is used.**

> "The high order 32 bits are included in the integrity check in an
>    algorithm/mode-specific fashion, e.g., the high-order 32 bits may be
>    appended after the Next Header field when a separate integrity
>    algorithm is employed." — §3.3.3, `rfc4303.txt:1413-1416`

- Strength: may (lower case). Class: internal.
- Check idea: configure an SA with ESN and a separate integrity algorithm, and check that the ICV verifies only when the correct high-order 32 bits are included after the Next Header field.

## Fragmentation

### RFC4303-FRAG-1

**An IPsec implementation performs fragmentation, when necessary, after ESP processing.**

> "If necessary, fragmentation is performed after ESP processing within
>    an IPsec implementation." — §3.3.4, `rfc4303.txt:1427-1428`

- Strength: description. Class: wire.
- Check idea: capture the fragments of an outbound packet that needed fragmentation and check that ESP protection was applied before the packet was split.

### RFC4303-FRAG-2

**In transport mode, ESP applies only to whole IP datagrams, never to IP fragments.**

> "transport mode ESP is applied only to
>    whole IP datagrams (not to IP fragments)." — §3.3.4, `rfc4303.txt:1428-1429`

- Strength: description. Class: end-to-end.
- Check idea: offer a transport-mode IPsec implementation an IP fragment and check that it does not apply ESP to the fragment itself.

### RFC4303-FRAG-3

**A packet already protected by ESP may be fragmented by routers en route, and the receiver reassembles such fragments before ESP processing.**

> "An IP packet to which ESP
>    has been applied may itself be fragmented by routers en route, and
>    such fragments must be reassembled prior to ESP processing at a
>    receiver." — §3.3.4, `rfc4303.txt:1429-1432`

- Strength: may, must (lower case). Class: end-to-end.
- Check idea: fragment an ESP-protected packet in the network and check that the receiver reassembles the fragments before it runs ESP processing.

### RFC4303-FRAG-4

**In tunnel mode, ESP may apply to an IP packet that is itself a fragment of an IP datagram, for example at a security gateway or a bump-in-the-stack or bump-in-the-wire implementation.**

> "In tunnel mode, ESP is applied to an IP packet, which may
>    be a fragment of an IP datagram.  For example, a security gateway or
>    a "bump-in-the-stack" or "bump-in-the-wire" IPsec implementation (as
>    defined in the Security Architecture document) may apply tunnel mode
>    ESP to such fragments." — §3.3.4, `rfc4303.txt:1432-1436`

- Strength: may (lower case). Class: end-to-end.
- Check idea: offer a tunnel-mode security gateway an IP fragment and check that it applies ESP to the fragment as a tunnel-mode packet.

### RFC4303-FRAG-5

**For transport mode, a bump-in-the-stack or bump-in-the-wire implementation may have to reassemble a locally fragmented packet, apply IPsec, and then fragment the result.**

> "bump-in-the-stack and bump-in-the-wire implementations may have to
>    first reassemble a packet fragmented by the local IP layer, then
>    apply IPsec, and then fragment the resulting packet." — §3.3.4, `rfc4303.txt:1439-1441`

- Strength: may (lower case). Class: end-to-end.
- Check idea: send a locally fragmented packet through a bump-in-the-stack or bump-in-the-wire implementation and check that it reassembles the packet, applies ESP, and only then fragments the result.

### RFC4303-FRAG-6

**For IPv6, a bump-in-the-stack or bump-in-the-wire implementation examines all extension headers to find a fragmentation header, and so learns whether the packet needs reassembly ahead of IPsec processing.**

> "For bump-in-the-stack and bump-in-the-wire
>    implementations, it will be necessary to examine all the extension
>    headers to determine if there is a fragmentation header and hence
>    that the packet needs reassembling prior to IPsec processing." — §3.3.4, `rfc4303.txt:1443-1446`

- Strength: description. Class: internal.
- Check idea: send an IPv6 packet whose fragmentation header is not the first extension header to a bump-in-the-stack or bump-in-the-wire implementation and check that it still detects the fragment and reassembles it before IPsec processing.

### RFC4303-FRAG-7

**An ESP implementation may choose to not support fragmentation.**

> "an ESP implementation MAY choose to not support fragmentation" — §3.3.4, `rfc4303.txt:1452`

- Strength: may. Class: internal.
- Check idea: configure an implementation that does not support fragmentation and check that it never reassembles or accepts a fragmented ESP packet.

### RFC4303-FRAG-8

**An ESP implementation may mark its transmitted packets with the DF bit, to help Path MTU discovery.**

> "may mark transmitted packets with
>    the DF bit, to facilitate Path
>    MTU (PMTU) discovery." — §3.3.4, `rfc4303.txt:1453-1454`

- Strength: may (lower case). Class: wire.
- Check idea: capture packets sent by an ESP implementation that supports Path MTU discovery and check whether the IP header's DF bit is set.

### RFC4303-FRAG-9

**An ESP implementation must support generating ICMP PMTU messages, or equivalent internal signaling on a native host, to reduce the likelihood of fragmentation.**

> "In any case, an ESP implementation MUST
>    support generation of ICMP PMTU messages (or equivalent internal
>    signaling for native host implementations) to minimize the likelihood
>    of fragmentation." — §3.3.4, `rfc4303.txt:1454`, `rfc4303.txt:1463-1465`

- Strength: must. Class: error-signal.
- Check idea: send an oversized packet through an ESP implementation and check that it generates an ICMP PMTU message, or delivers equivalent internal signaling on a native host.

## Inbound processing and reassembly

### RFC4303-REAS-1

**An IPsec implementation performs reassembly, when required, before ESP processing.**

> "If required, reassembly is performed prior to ESP processing." — §3.4.1, `rfc4303.txt:1472`

- Strength: description. Class: end-to-end.
- Check idea: send fragments of a packet destined for ESP processing and check that the receiver reassembles them before it runs ESP processing.

### RFC4303-REAS-2

**The receiver must discard a packet offered to ESP that still looks like an IP fragment, when the OFFSET field is non-zero or the MORE FRAGMENTS flag is set.**

> "If a
>    packet offered to ESP for processing appears to be an IP fragment,
>    i.e., the OFFSET field is non-zero or the MORE FRAGMENTS flag is set,
>    the receiver MUST discard the packet" — §3.4.1, `rfc4303.txt:1472-1475`

- Strength: must. Class: end-to-end.
- Check idea: offer ESP processing a packet whose OFFSET field is non-zero or whose MORE FRAGMENTS flag is set, and check that the receiver discards it.

### RFC4303-REAS-3

**Discarding an apparent IP fragment offered to ESP processing is an auditable event.**

> "this is an auditable event." — §3.4.1, `rfc4303.txt:1475`

- Strength: description. Class: internal.
- Check idea: trigger the discard of an apparent IP fragment offered to ESP and check that the implementation logs it as an auditable event.

### RFC4303-REAS-4

**The audit log entry for a discarded apparent fragment should include the SPI, date and time received, source and destination addresses, Sequence Number, and, for IPv6, the Flow ID.**

> "The audit log entry for this event SHOULD include the SPI value,
>    date/time received, Source Address, Destination Address, Sequence
>    Number, and (in IPv6) the Flow ID." — §3.4.1, `rfc4303.txt:1476-1478`

- Strength: should. Class: internal.
- Check idea: trigger the discard of an apparent IP fragment offered to ESP and check that the audit log entry carries the SPI, date/time, addresses, Sequence Number, and, for IPv6, the Flow ID.

### RFC4303-REAS-5

**After it reassembles a packet, the IP code zeroes the OFFSET field and clears the MORE FRAGMENTS flag, so ESP does not mistake the reassembled packet for a fragment.**

> "the current IPv4 spec does NOT require
>    either the zeroing of the OFFSET field or the clearing of the MORE
>    FRAGMENTS flag.  In order for a reassembled packet to be processed by
>    IPsec (as opposed to discarded as an apparent fragment), the IP code
>    must do these two things after it reassembles a packet." — §3.4.1, `rfc4303.txt:1480-1484`

- Strength: must (lower case). Class: internal.
- Check idea: reassemble a fragmented packet ahead of ESP processing and check that its OFFSET field is zero and its MORE FRAGMENTS flag is clear before it reaches ESP.

## Inbound SA lookup

### RFC4303-ISA-1

**On receiving a packet with an ESP Header, the receiver finds the appropriate unidirectional SA by looking it up in the SAD.**

> "Upon receipt of a packet containing an ESP Header, the receiver
>    determines the appropriate (unidirectional) SA via lookup in the SAD." — §3.4.2, `rfc4303.txt:1488-1489`

- Strength: description. Class: internal.
- Check idea: send a packet with an ESP Header and check that the receiver looks up its SA in the SAD before further processing.

### RFC4303-ISA-2

**For a unicast SA, the receiver bases the SAD lookup on the SPI, or on the SPI plus the protocol field.**

> "For a unicast SA, this determination is based on the SPI or the SPI
>    plus protocol field, as described in Section 2.1." — §3.4.2, `rfc4303.txt:1490-1491`

- Strength: description. Class: internal.
- Check idea: send unicast ESP packets that share an SPI but differ in protocol field and check that the receiver's SAD lookup distinguishes them when needed.

### RFC4303-ISA-3

**When an implementation supports multicast traffic, it also uses the destination address, and may use the sender address, in the SAD lookup.**

> "If an
>    implementation supports multicast traffic, the destination address is
>    also employed in the lookup (in addition to the SPI), and the sender
>    address also may be employed, as described in Section 2.1." — §3.4.2, `rfc4303.txt:1491-1494`

- Strength: description; may (using the sender address). Class: internal.
- Check idea: send multicast ESP packets that share an SPI but differ in destination or sender address and check that the receiver's SAD lookup distinguishes them.

### RFC4303-ISA-4

**The SAD entry for an SA states whether the Sequence Number field is checked, whether sequence numbers are 32 or 64 bits, and whether an explicit ICV field is present and how large it is.**

> "The SAD entry for the SA also indicates whether the
>    Sequence Number field will be checked, whether 32- or 64-bit sequence
>    numbers are employed for the SA, and whether the (explicit) ICV field
>    should be present (and if so, its size)." — §3.4.2, `rfc4303.txt:1496-1499`

- Strength: should (lower case). Class: internal.
- Check idea: inspect the SAD entry for an SA and check that it records whether the Sequence Number is checked, the sequence number size, and whether an explicit ICV field is present and its size.

### RFC4303-ISA-5

**The SAD entry for an SA also states the algorithms and keys used for decryption and, where applicable, ICV computation.**

> "Also, the SAD entry will
>    specify the algorithms and keys to be employed for decryption and ICV
>    computation (if applicable)." — §3.4.2, `rfc4303.txt:1499-1501`

- Strength: description. Class: internal.
- Check idea: inspect the SAD entry for an SA and check that it records the decryption algorithm, the ICV-computation algorithm where applicable, and the associated keys.

### RFC4303-ISA-6

**The receiver must discard a packet for which no valid Security Association exists.**

> "If no valid Security Association exists for this packet, the receiver
>    MUST discard the packet" — §3.4.2, `rfc4303.txt:1503-1504`

- Strength: must. Class: end-to-end.
- Check idea: send an ESP packet whose SPI matches no SA at the receiver and check that the receiver discards the packet.

### RFC4303-ISA-7

**Discarding a packet for which no valid SA exists is an auditable event.**

> "this is an auditable event." — §3.4.2, `rfc4303.txt:1504`

- Strength: description. Class: internal.
- Check idea: send an ESP packet with no matching SA and check that the receiver logs the discard as an auditable event.

### RFC4303-ISA-8

**The audit log entry for a discard due to no valid SA should include the SPI, date and time received, source and destination addresses, Sequence Number, and, for IPv6, the cleartext Flow ID.**

> "The audit log
>    entry for this event SHOULD include the SPI value, date/time
>    received, Source Address, Destination Address, Sequence Number, and
>    (in IPv6) the cleartext Flow ID." — §3.4.2, `rfc4303.txt:1504-1507`

- Strength: should. Class: internal.
- Check idea: send an ESP packet with no matching SA and check that the resulting audit log entry carries the SPI, date/time, addresses, Sequence Number, and, for IPv6, the Flow ID.

## Sequence number verification

### RFC4303-ISEQ-1

**Every ESP implementation must support the anti-replay service, though the receiver may enable or disable it for each SA.**

> "All ESP implementations MUST support the anti-replay service, though
>    its use may be enabled or disabled by the receiver on a per-SA basis." — §3.4.3, `rfc4303.txt:1525-1526`

- Strength: must (supporting the service); may (lower case, enabling or disabling it per SA). Class: internal.
- Check idea: check that every ESP implementation offers the anti-replay service, and that the receiver can turn it on or off separately for each SA.

### RFC4303-ISEQ-2

**The receiver must not enable anti-replay on an SA unless the ESP integrity service is also enabled for it, because otherwise the Sequence Number field is not integrity protected.**

> "This service MUST NOT be enabled unless the ESP integrity service
>    also is enabled for the SA, because otherwise the Sequence Number
>    field has not been integrity protected." — §3.4.3, `rfc4303.txt:1527-1529`

- Strength: must not. Class: internal.
- Check idea: configure an SA with integrity disabled and check that the receiver does not also enable anti-replay for it.

### RFC4303-ISEQ-3

**Anti-replay applies to unicast SAs as well as multicast SAs.**

> "Anti-replay is applicable to
>    unicast as well as multicast SAs." — §3.4.3, `rfc4303.txt:1529-1530`

- Strength: description. Class: internal.
- Check idea: enable anti-replay on both a unicast and a multicast SA and check that the receiver applies sequence number checking to each.

### RFC4303-ISEQ-4

**When no anti-replay mechanism is negotiated or configured for a multi-sender SA, the sender and receiver disable sequence number checking for it.**

> "In the absence of negotiation (or manual
>    configuration) of an anti-replay mechanism for such an SA, it is
>    recommended that sender and receiver checking of the sequence number
>    for the SA be disabled (via negotiation or manual configuration), as
>    noted below." — §3.4.3, `rfc4303.txt:1532-1536`

- Strength: description. Class: internal.
- Check idea: set up a multi-sender SA with no negotiated anti-replay mechanism and check that sequence number checking is disabled at both ends.

### RFC4303-ISEQ-5

**When the receiver has not enabled anti-replay for an SA, it performs no inbound checks on the Sequence Number.**

> "If the receiver does not enable anti-replay for an SA, no inbound
>    checks are performed on the Sequence Number." — §3.4.3, `rfc4303.txt:1538-1539`

- Strength: description. Class: end-to-end.
- Check idea: disable anti-replay on an SA, send packets with repeated or out-of-order sequence numbers, and check that the receiver accepts them without a sequence number check.

### RFC4303-ISEQ-6

**By default, the sender assumes the receiver has anti-replay enabled.**

> "from the
>    perspective of the sender, the default is to assume that anti-replay
>    is enabled at the receiver." — §3.4.3, `rfc4303.txt:1539-1541`

- Strength: description. Class: internal.
- Check idea: establish an SA without any anti-replay notification and check that the sender behaves as though the receiver has anti-replay enabled.

### RFC4303-ISEQ-7

**When an SA establishment protocol is used, a receiver that will not provide anti-replay protection should notify the sender of this during SA establishment.**

> "if an SA establishment protocol is employed, the receiver
>    SHOULD notify the sender, during SA establishment, if the receiver
>    will not provide anti-replay protection." — §3.4.3, `rfc4303.txt:1543-1545`

- Strength: should. Class: internal.
- Check idea: establish an SA whose receiver will not provide anti-replay protection and check that the establishment protocol carries a notification of that to the sender.

### RFC4303-ISEQ-8

**When the receiver has enabled anti-replay for an SA, the receive packet counter for the SA must be initialized to zero when the SA is established.**

> "If the receiver has enabled the anti-replay service for this SA, the
>    receive packet counter for the SA MUST be initialized to zero when
>    the SA is established." — §3.4.3, `rfc4303.txt:1547-1549`

- Strength: must. Class: internal.
- Check idea: establish an SA with anti-replay enabled and check that the receiver's packet counter for that SA starts at zero.

### RFC4303-ISEQ-9

**For each received packet, the receiver must verify that its Sequence Number does not duplicate that of any other packet received during the SA's life.**

> "For each received packet, the receiver MUST
>    verify that the packet contains a Sequence Number that does not
>    duplicate the Sequence Number of any other packets received during
>    the life of this SA." — §3.4.3, `rfc4303.txt:1549-1552`

- Strength: must. Class: end-to-end.
- Check idea: replay a previously received packet on an anti-replay-enabled SA and check that the receiver rejects it as a duplicate.

### RFC4303-ISEQ-10

**The duplicate Sequence Number check should be the first ESP check the receiver applies to a packet once it is matched to an SA, to reject duplicates quickly.**

> "This SHOULD be the first ESP check applied to a
>    packet after it has been matched to an SA, to speed rejection of
>    duplicate packets." — §3.4.3, `rfc4303.txt:1552-1554`

- Strength: should. Class: internal.
- Check idea: send a duplicate packet on an anti-replay-enabled SA and check that the receiver rejects it before it performs decryption or the full integrity check.

### RFC4303-ISEQ-11

**ESP allows a two-stage verification of packet sequence numbers, and an implementation capable of line-rate operation need not perform the preliminary stage.**

> "ESP permits two-stage verification of packet sequence numbers." — §3.4.3, `rfc4303.txt:1556` and "If the implementation is
>    capable of such "line rate" operation, then it is not necessary to
>    perform the preliminary verification stage described below." — §3.4.3, `rfc4303.txt:1560-1562`

- Strength: description. Class: internal.
- Check idea: check whether an implementation that keeps up with its network interface's rate still performs a separate preliminary Sequence Number check before decryption.

### RFC4303-ISEQ-12

**The preliminary Sequence Number check uses the Sequence Number value from the ESP Header and runs before integrity checking and decryption.**

> "The preliminary Sequence Number check is effected utilizing the
>    Sequence Number value in the ESP Header and is performed prior to
>    integrity checking and decryption." — §3.4.3, `rfc4303.txt:1564-1566`

- Strength: description. Class: internal.
- Check idea: examine the processing order at a receiver performing the preliminary check and check that it reads the Sequence Number field before it starts decryption or integrity checking.

### RFC4303-ISEQ-13

**When the preliminary Sequence Number check fails, the receiver discards the packet before it performs any cryptographic operation.**

> "If this preliminary check fails,
>    the packet is discarded, thus avoiding the need for any cryptographic
>    operations by the receiver." — §3.4.3, `rfc4303.txt:1566`, `rfc4303.txt:1575-1576`

- Strength: description. Class: end-to-end.
- Check idea: send a packet that fails the preliminary Sequence Number check and check that the receiver discards it without performing decryption or the integrity check.

### RFC4303-ISEQ-14

**When the preliminary check succeeds, the receiver does not yet update its local counter, because the Sequence Number's integrity has not been verified yet.**

> "If the preliminary check is successful,
>    the receiver cannot yet modify its local counter, because the
>    integrity of the Sequence Number has not been verified at this point." — §3.4.3, `rfc4303.txt:1576-1578`

- Strength: description. Class: internal.
- Check idea: pass a packet through the preliminary Sequence Number check successfully and check that the receiver's window counter stays unchanged until the integrity check also succeeds.

### RFC4303-ISEQ-15

**The receiver rejects duplicate packets using a sliding receive window.**

> "Duplicates are rejected through the use of a sliding receive window." — §3.4.3, `rfc4303.txt:1580`

- Strength: description. Class: internal.
- Check idea: send a duplicate packet within the receive window and check that the receiver rejects it.

### RFC4303-ISEQ-16

**The right edge of the receive window is the highest validated Sequence Number received on the SA, and the receiver rejects a packet whose sequence number is lower than the window's left edge.**

> "The "right" edge of the window represents the highest, validated
>    Sequence Number value received on this SA.  Packets that contain
>    sequence numbers lower than the "left" edge of the window are
>    rejected." — §3.4.3, `rfc4303.txt:1585-1588`

- Strength: description. Class: internal.
- Check idea: send a packet whose sequence number is below the window's left edge and check that the receiver rejects it.

### RFC4303-ISEQ-17

**The receiver checks a packet that falls within the window against the list of Sequence Numbers already received in that window.**

> "Packets falling within the window are checked against a
>    list of received packets within the window." — §3.4.3, `rfc4303.txt:1588-1589`

- Strength: description. Class: internal.
- Check idea: send a packet whose sequence number falls inside the current window and check that the receiver compares it against the window's list of already-received numbers.

### RFC4303-ISEQ-18

**With ESN, the receiver checks a received Sequence Number against the window using the full sequence number, reconstructed from the transmitted low-order 32 bits and its own local high-order 32 bits.**

> "If the ESN option is
>    selected for an SA, only the low-order 32 bits of the sequence number
>    are explicitly transmitted, but the receiver employs the full
>    sequence number computed using the high-order 32 bits for the
>    indicated SA (from his local counter) when checking the received
>    Sequence Number against the receive window." — §3.4.3, `rfc4303.txt:1589-1594`

- Strength: description. Class: internal.
- Check idea: configure an SA with ESN, send a packet carrying only the low-order 32 bits, and check that the receiver checks it against the window using its own reconstructed high-order bits.

### RFC4303-ISEQ-19

**When the packet's low-order 32 bits are lower than the receiver's own low-order bits, the receiver assumes the high-order 32 bits have advanced to a new subspace.**

> "In constructing the full
>    sequence number, if the low-order 32 bits carried in the packet are
>    lower in value than the low-order 32 bits of the receiver's sequence
>    number, the receiver assumes that the high-order 32 bits have been
>    incremented, moving to a new sequence number subspace." — §3.4.3, `rfc4303.txt:1594-1598`

- Strength: description. Class: internal.
- Check idea: send an ESN packet whose transmitted low-order bits wrap below the receiver's current low-order value and check that the receiver reconstructs the sequence number using an incremented high-order part.

### RFC4303-ISEQ-20

**The receiver may employ additional, heuristic checks to re-synchronize its sequence number counter when a gap larger than 2\*\*32-1 packets occurs.**

> "If a larger gap occurs, additional, heuristic
>    checks for re-synchronization of the receiver sequence number counter
>    MAY be employed, as described in the Appendix." — §3.4.3, `rfc4303.txt:1600-1602`

- Strength: may. Class: internal.
- Check idea: create a reception gap on an ESN SA larger than 2\*\*32-1 packets and check whether the receiver applies a heuristic re-synchronization check.

### RFC4303-ISEQ-21

**The receiver proceeds to integrity verification for a packet that is not a duplicate within the window, or that falls to the right of the window, when a separate integrity algorithm is used.**

> "If the received packet falls within the window and is not a
>    duplicate, or if the packet is to the right of the window, and if a
>    separate integrity algorithm is employed, then the receiver proceeds
>    to integrity verification." — §3.4.3, `rfc4303.txt:1604-1607`

- Strength: description. Class: internal.
- Check idea: send a non-duplicate packet within or to the right of the window on an SA with a separate integrity algorithm and check that the receiver performs integrity verification on it.

### RFC4303-ISEQ-22

**When a combined mode algorithm is used, the receiver performs the integrity check together with decryption.**

> "If a combined mode algorithm is employed,
>    the integrity check is performed along with decryption." — §3.4.3, `rfc4303.txt:1607-1608`

- Strength: description. Class: internal.
- Check idea: send a packet on a combined-mode SA and check that the receiver's integrity check and decryption complete together, not as separate stages.

### RFC4303-ISEQ-23

**When the integrity check fails, the receiver must discard the received IP datagram as invalid.**

> "if the integrity check fails, the receiver MUST discard the
>    received IP datagram as invalid" — §3.4.3, `rfc4303.txt:1609-1610`

- Strength: must. Class: end-to-end.
- Check idea: send a packet with a corrupted ICV and check that the receiver discards the resulting IP datagram.

### RFC4303-ISEQ-24

**Discarding a packet that fails the integrity check is an auditable event.**

> "this is an auditable event." — §3.4.3, `rfc4303.txt:1610`

- Strength: description. Class: internal.
- Check idea: send a packet with a corrupted ICV and check that the receiver logs the discard as an auditable event.

### RFC4303-ISEQ-25

**The audit log entry for a failed integrity check should include the SPI, date and time received, source and destination addresses, the Sequence Number, and, for IPv6, the Flow ID.**

> "The
>    audit log entry for this event SHOULD include the SPI value,
>    date/time received, Source Address, Destination Address, the Sequence
>    Number, and (in IPv6) the Flow ID." — §3.4.3, `rfc4303.txt:1610-1613`

- Strength: should. Class: internal.
- Check idea: send a packet with a corrupted ICV and check that the resulting audit log entry carries the SPI, date/time, addresses, Sequence Number, and, for IPv6, the Flow ID.

### RFC4303-ISEQ-26

**The receiver updates the receive window only when the integrity verification succeeds.**

> "The receive window is updated
>    only if the integrity verification succeeds." — §3.4.3, `rfc4303.txt:1613-1614`

- Strength: description. Class: internal.
- Check idea: send a packet that passes the preliminary check but fails the integrity check and check that the receive window does not advance.

### RFC4303-ISEQ-27

**With a combined mode algorithm, the integrity-protected Sequence Number also matches the Sequence Number used for anti-replay protection.**

> "the integrity protected Sequence Number
>    must also match the Sequence Number used for anti-replay protection." — §3.4.3, `rfc4303.txt:1615-1616`

- Strength: must (lower case). Class: internal.
- Check idea: send a combined-mode packet whose integrity-protected Sequence Number differs from the one used in the anti-replay window and check that the receiver rejects it.

### RFC4303-ISEQ-28

**The receiver must support a minimum receive window of 32 packets when using 32-bit sequence numbers.**

> "A minimum window size of 32 packets MUST be supported when 32-bit
>    sequence numbers are employed" — §3.4.3, `rfc4303.txt:1618-1619`

- Strength: must. Class: internal.
- Check idea: configure a receiver with 32-bit sequence numbers and check that it accepts non-duplicate packets across a window of at least 32 packets.

### RFC4303-ISEQ-29

**A window size of 64 is preferred, and should be the default receive window size.**

> "a window size of 64 is preferred and
>    SHOULD be employed as the default." — §3.4.3, `rfc4303.txt:1619-1620`

- Strength: should. Class: internal.
- Check idea: check the default receive window size of an unconfigured SA and check that it is 64 packets.

### RFC4303-ISEQ-30

**The receiver may choose another window size larger than the minimum, without notifying the sender of its choice.**

> "Another window size (larger than
>    the minimum) MAY be chosen by the receiver.  (The receiver does NOT
>    notify the sender of the window size.)" — §3.4.3, `rfc4303.txt:1620-1622`

- Strength: may. Class: internal.
- Check idea: configure a receiver with an enlarged receive window and check that no notification of the window size reaches the sender.

### RFC4303-ISEQ-31

**The receive window size should be increased for higher-speed environments, independent of assurance considerations.**

> "The receive window size" — §3.4.3, `rfc4303.txt:1622` and "should be increased for higher-speed environments, irrespective of
>    assurance issues." — §3.4.3, `rfc4303.txt:1631-1632`

- Strength: should (lower case). Class: internal.
- Check idea: compare the receive window size configured for a high-speed interface against that of a low-speed one and check that the high-speed one is larger.

## ICV verification

### RFC4303-IICV-1

**When integrity has been selected, the receiver computes the ICV over the ESP packet minus the ICV field, using the SA's integrity algorithm, and checks it against the carried ICV.**

> "If integrity has been selected, the receiver computes the
>             ICV over the ESP packet minus the ICV, using the specified
>             integrity algorithm and verifies that it is the same as the
>             ICV carried in the packet." — §3.4.4.1, `rfc4303.txt:1646-1649`

- Strength: description. Class: internal.
- Check idea: send a packet with a valid ICV and check that the receiver recomputes the ICV over the packet minus the ICV field and compares it to the carried value.

### RFC4303-IICV-2

**The receiver accepts the datagram as valid when the computed and received ICVs match.**

> "If the computed and received ICVs match, then the datagram
>             is valid, and it is accepted." — §3.4.4.1, `rfc4303.txt:1652-1653`

- Strength: description. Class: end-to-end.
- Check idea: send a packet with a correct ICV and check that the receiver accepts the resulting datagram.

### RFC4303-IICV-3

**When the ICV test fails, the receiver must discard the received IP datagram as invalid.**

> "If the test fails, then the
>             receiver MUST discard the received IP datagram as invalid" — §3.4.4.1, `rfc4303.txt:1653-1654`

- Strength: must. Class: end-to-end.
- Check idea: send a packet with a corrupted ICV, using separate encryption and integrity algorithms, and check that the receiver discards the resulting datagram.

### RFC4303-IICV-4

**Discarding a datagram for a failed ICV test is an auditable event.**

> "this is an auditable event." — §3.4.4.1, `rfc4303.txt:1654-1655`

- Strength: description. Class: internal.
- Check idea: send a packet with a corrupted ICV and check that the receiver logs the discard as an auditable event.

### RFC4303-IICV-5

**The log data for a failed ICV test should include the SPI, date and time received, source and destination addresses, the Sequence Number, and, for IPv6, the cleartext Flow ID.**

> "The log data SHOULD include the
>             SPI value, date/time received, Source Address, Destination
>             Address, the Sequence Number, and (for IPv6) the cleartext
>             Flow ID." — §3.4.4.1, `rfc4303.txt:1655-1658`

- Strength: should. Class: internal.
- Check idea: send a packet with a corrupted ICV and check that the audit log entry carries the SPI, date/time, addresses, Sequence Number, and, for IPv6, the Flow ID.

### RFC4303-IICV-6

**An implementation may follow any set of steps that gives the same result as the reference steps for ICV verification.**

> "Implementations can use any set of steps that results in the
>             same result as the following set of steps." — §3.4.4.1, `rfc4303.txt:1662-1663`

- Strength: description. Class: internal.
- Check idea: check that an implementation using a different internal procedure for ICV verification still reaches the same accept-or-discard outcome as the reference steps.

### RFC4303-IICV-7

**When implicit padding is required, the receiver appends zero-filled bytes after the Next Header field, or after the ESN high-order bits, and computes and compares the ICV using the algorithm's own comparison rule.**

> "If implicit
>             padding is required, based on the block size of the
>             integrity algorithm, append zero-filled bytes to the end of
>             the ESP packet directly after the Next Header field, or
>             after the high-order 32 bits of the sequence number if ESN
>             is selected.  Perform the ICV computation and compare the
>             result with the saved value, using the comparison rules
>             defined by the algorithm specification." — §3.4.4.1, `rfc4303.txt:1665-1672`

- Strength: description. Class: internal.
- Check idea: configure an SA needing implicit padding for its integrity algorithm and check that the receiver's ICV verification still succeeds for a genuine packet.

### RFC4303-IICV-8

**The receiver decrypts the ESP Payload Data, Padding, Pad Length, and Next Header using the SA's key, encryption algorithm, algorithm mode, and cryptographic synchronization data.**

> "The receiver decrypts the ESP Payload Data, Padding, Pad
>             Length, and Next Header using the key, encryption algorithm,
>             algorithm mode, and cryptographic synchronization data (if
>             any), indicated by the SA." — §3.4.4.1, `rfc4303.txt:1674-1677`

- Strength: description. Class: end-to-end.
- Check idea: send a packet on an SA with a known key and algorithm and check that the receiver only recovers the original data by decrypting with that same key, algorithm, and mode.

### RFC4303-IICV-9

**When explicit cryptographic synchronization data such as an IV is used, the receiver takes it from the Payload field and inputs it to the decryption algorithm.**

> "If explicit cryptographic synchronization data, e.g.,
>                    an IV, is indicated, it is taken from the Payload
>                    field and input to the decryption algorithm as per
>                    the algorithm specification." — §3.4.4.1, `rfc4303.txt:1691-1694` and "If explicit cryptographic synchronization data, e.g.,
>                    an IV, is indicated, it is taken from the Payload
>                    field and input to the decryption algorithm as per
>                    the algorithm specification." — §3.4.4.2, `rfc4303.txt:1764-1767`

- Strength: description. Class: wire.
- Check idea: capture an inbound ESP packet from an SA that uses explicit synchronization data and check that the receiver reads it from the Payload field before it decrypts.

### RFC4303-IICV-10

**When implicit cryptographic synchronization data is used, the receiver constructs a local version of it and inputs it to the decryption algorithm.**

> "If implicit cryptographic synchronization data is
>                    indicated, a local version of the IV is constructed
>                    and input to the decryption algorithm as per the
>                    algorithm specification." — §3.4.4.1, `rfc4303.txt:1696-1699` and "If implicit cryptographic synchronization data, e.g.,
>                    an IV, is indicated, a local version of the IV is
>                    constructed and input to the decryption algorithm as
>                    per the algorithm specification." — §3.4.4.2, `rfc4303.txt:1769-1772`

- Strength: description. Class: internal.
- Check idea: configure an SA that uses implicit synchronization data and check that the receiver reconstructs it locally and still decrypts correctly, without reading it from a packet field.

### RFC4303-IICV-11

**The receiver processes any Padding as the encryption algorithm's specification directs.**

> "The receiver processes any Padding as specified in the
>             encryption algorithm specification." — §3.4.4.1, `rfc4303.txt:1701-1702`

- Strength: description. Class: internal.
- Check idea: send a packet with padding built for a particular encryption algorithm and check that the receiver processes it as that algorithm's specification directs.

### RFC4303-IICV-12

**When the default padding scheme has been used, the receiver should inspect the Padding field before it removes the padding and passes the decrypted data to the next layer.**

> "If the default padding
>             scheme (see Section 2.4) has been employed, the receiver
>             SHOULD inspect the Padding field before removing the padding
>             prior to passing the decrypted data to the next layer." — §3.4.4.1, `rfc4303.txt:1702-1705`

- Strength: should. Class: internal.
- Check idea: send a packet using the default padding scheme with incorrect padding bytes and check that the receiver's inspection can detect it before it delivers the data to the next layer.

### RFC4303-IICV-13

**The receiver checks the Next Header field, and discards the packet without further processing when the value is 59, meaning no next header.**

> "The receiver checks the Next Header field.  If the value is
>             "59" (no next header), the (dummy) packet is discarded
>             without further processing." — §3.4.4.1, `rfc4303.txt:1707-1709` and "The receiver checks the Next Header field.  If the value is
>             "59" (no next header), the (dummy) packet is discarded
>             without further processing." — §3.4.4.2, `rfc4303.txt:1784-1786`

- Strength: description. Class: end-to-end.
- Check idea: send a dummy ESP packet whose Next Header field is 59 and check that the receiver discards it without passing it to any next layer.

### RFC4303-IICV-14

**In transport mode, the receiver reconstructs the original IP datagram from the outer IP header plus the original next layer protocol information in the ESP Payload field.**

> "for transport mode -- outer IP header plus the
>                    original next layer protocol information in the ESP
>                    Payload field" — §3.4.4.1, `rfc4303.txt:1713-1715`

- Strength: description. Class: end-to-end.
- Check idea: send a transport-mode ESP packet and check that the receiver rebuilds the original datagram from the outer IP header and the decrypted next layer protocol data.

### RFC4303-IICV-15

**In tunnel mode, the receiver reconstructs the original IP datagram from the entire IP datagram carried in the ESP Payload field.**

> "for tunnel mode -- the entire IP datagram in the ESP
>                    Payload field." — §3.4.4.1, `rfc4303.txt:1716-1717`

- Strength: description. Class: end-to-end.
- Check idea: send a tunnel-mode ESP packet and check that the receiver recovers the entire original IP datagram from the decrypted Payload field.

### RFC4303-IICV-16

**In an IPv6 context, the receiver should at least ensure that the decrypted data is 8-byte aligned, so the protocol named in the Next Header field can process it.**

> "At a minimum, in an
>             IPv6 context, the receiver SHOULD ensure that the decrypted
>             data is 8-byte aligned, to facilitate processing by the
>             protocol identified in the Next Header field." — §3.4.4.1, `rfc4303.txt:1721-1724`

- Strength: should. Class: encoding.
- Check idea: send an IPv6 ESP packet whose decrypted data would not naturally be 8-byte aligned and check that the receiver still delivers 8-byte aligned data to the next protocol.

### RFC4303-IICV-17

**Reconstructing the original datagram discards any optional TFC padding, which sits after the IP datagram or transport-layer frame and before the Padding field.**

> "This
>             processing "discards" any (optional) TFC padding that has
>             been added for traffic flow confidentiality.  (If present,
>             this will have been inserted after the IP datagram (or
>             transport-layer frame) and before the Padding field (see
>             Section 2.4).)" — §3.4.4.1, `rfc4303.txt:1724-1729`

- Strength: description. Class: internal.
- Check idea: send a packet carrying TFC padding after the original datagram and check that the receiver delivers only the original datagram, without the padding, to the next layer.

### RFC4303-IICV-18

**When integrity checking and encryption run in parallel, the receiver must complete integrity checking before it passes the decrypted packet on for further processing.**

> "If integrity checking and encryption are performed in parallel,
>    integrity checking MUST be completed before the decrypted packet is
>    passed on for further processing." — §3.4.4.1, `rfc4303.txt:1731-1733`

- Strength: must. Class: end-to-end.
- Check idea: configure a receiver that decrypts and integrity-checks in parallel and check that it never delivers a packet before the integrity check has completed.

### RFC4303-IICV-19

**A receiver that decrypts in parallel with integrity checking takes care to avoid race conditions in accessing and extracting the decrypted packet.**

> "If the receiver performs decryption in parallel with integrity
>    checking, care must be taken to avoid possible race conditions with
>    regard to packet access and extraction of the decrypted packet." — §3.4.4.1, `rfc4303.txt:1746-1748`

- Strength: must (lower case). Class: internal.
- Check idea: check a receiver that decrypts in parallel with integrity checking for consistent, race-free access to the decrypted packet under concurrent load.

### RFC4303-IICV-20

**The receiver decrypts and integrity checks the ESP Payload Data, Padding, Pad Length, and Next Header together, using the SA's key, algorithm, algorithm mode, and cryptographic synchronization data.**

> "Decrypts and integrity checks the ESP Payload Data, Padding,
>             Pad Length, and Next Header, using the key, algorithm,
>             algorithm mode, and cryptographic synchronization data (if
>             any), indicated by the SA." — §3.4.4.2, `rfc4303.txt:1755-1758`

- Strength: description. Class: end-to-end.
- Check idea: send a packet on a combined-mode SA and check that the receiver's decryption and integrity result come from a single combined operation.

### RFC4303-IICV-21

**The SPI from the ESP header and the receiver's adjusted packet counter value are inputs to the combined mode algorithm, because they are required for the integrity check.**

> "The SPI from the ESP header, and
>             the (receiver) packet counter value (adjusted as required
>             from the processing described in Section 3.4.3) are inputs
>             to this algorithm, as they are required for the integrity
>             check." — §3.4.4.2, `rfc4303.txt:1758-1762`

- Strength: description. Class: internal.
- Check idea: alter the SPI or the expected sequence number of a captured combined-mode packet and check that the receiver's integrity check then fails.

### RFC4303-IICV-22

**When the combined mode algorithm's integrity check fails, the receiver must discard the received IP datagram as invalid.**

> "If the integrity check performed by the combined mode
>             algorithm fails, the receiver MUST discard the received IP
>             datagram as invalid" — §3.4.4.2, `rfc4303.txt:1774-1776`

- Strength: must. Class: end-to-end.
- Check idea: send a packet with a corrupted combined-mode integrity check and check that the receiver discards the resulting datagram.

### RFC4303-IICV-23

**Discarding a datagram for a failed combined-mode integrity check is an auditable event.**

> "this is an auditable event." — §3.4.4.2, `rfc4303.txt:1776`

- Strength: description. Class: internal.
- Check idea: send a packet with a corrupted combined-mode integrity check and check that the receiver logs the discard as an auditable event.

### RFC4303-IICV-24

**The log data for a failed combined-mode integrity check should include the SPI, date and time received, source and destination addresses, the Sequence Number, and, for IPv6, the cleartext Flow ID.**

> "The log
>             data SHOULD include the SPI value, date/time received,
>             Source Address, Destination Address, the Sequence Number,
>             and (in IPv6) the cleartext Flow ID." — §3.4.4.2, `rfc4303.txt:1776-1779`

- Strength: should. Class: internal.
- Check idea: send a packet with a corrupted combined-mode integrity check and check that the resulting audit log entry carries the SPI, date/time, addresses, Sequence Number, and, for IPv6, the Flow ID.

### RFC4303-IICV-25

**The receiver processes any Padding as the encryption algorithm specifies, unless the combined mode algorithm has already done so.**

> "Process any Padding as specified in the encryption algorithm
>             specification, if the algorithm has not already done so." — §3.4.4.2, `rfc4303.txt:1781-1782`

- Strength: description. Class: internal.
- Check idea: send a combined-mode packet with padding and check that the receiver ends up with correctly processed padding, whether the combined mode algorithm handled it or the receiver did so afterward.

### RFC4303-IICV-26

**The receiver extracts the original IP datagram, in tunnel mode, or the transport-layer frame, in transport mode, from the ESP Payload Data field.**

> "Extract the original IP datagram (tunnel mode) or
>             transport-layer frame (transport mode) from the ESP Payload
>             Data field." — §3.4.4.2, `rfc4303.txt:1788-1790`

- Strength: description. Class: end-to-end.
- Check idea: send a combined-mode ESP packet in each mode and check that the receiver extracts the original IP datagram, in tunnel mode, or the transport-layer frame, in transport mode, from the Payload Data field.

### RFC4303-IICV-27

**Extracting the original datagram or frame implicitly discards any optional TFC padding, which sits after the IP payload and before the Padding field.**

> "This implicitly discards any (optional) padding
>             that has been added for traffic flow confidentiality.  (If
>             present, the TFC padding will have been inserted after the
>             IP payload and before the Padding field (see Section 2.4).)" — §3.4.4.2, `rfc4303.txt:1790`, `rfc4303.txt:1799-1801`

- Strength: description. Class: internal.
- Check idea: send a combined-mode packet carrying TFC padding and check that the receiver delivers only the original datagram or frame, without the padding.

## Conformance requirements

### RFC4303-CONF-1

**A conformant implementation must implement the ESP syntax and processing this specification describes, for unicast traffic.**

> "Implementations that claim conformance or compliance with this
>    specification MUST implement the ESP syntax and processing described
>    here for unicast traffic" — §5, `rfc4303.txt:1860-1862`

- Strength: must. Class: internal.
- Check idea: check a claimed conformant implementation against the unicast ESP syntax and processing rules this specification defines.

### RFC4303-CONF-2

**A conformant implementation must also comply with all additional packet processing requirements the Security Architecture document imposes.**

> "MUST comply with all additional packet
>    processing requirements levied by the Security Architecture document
>    [Ken-Arch]." — §5, `rfc4303.txt:1862-1864`

- Strength: must. Class: internal.
- Check idea: check a claimed conformant implementation against the additional packet processing requirements of the Security Architecture document, RFC 4301.

### RFC4303-CONF-3

**An implementation that claims to support multicast traffic must also comply with the additional requirements specified for multicast support.**

> "if an implementation claims to support
>    multicast traffic, it MUST comply with the additional requirements
>    specified for support of such traffic." — §5, `rfc4303.txt:1864-1866`

- Strength: must. Class: internal.
- Check idea: check an implementation that claims multicast support against the additional requirements this specification lists for multicast traffic.

### RFC4303-CONF-4

**When the key used to compute an ICV is manually distributed, correct anti-replay service depends on the sender correctly keeping the counter state across events such as local reboots, until the key is replaced.**

> "If the key used to compute an
>    ICV is manually distributed, correct provision of the anti-replay
>    service requires correct maintenance of the counter state at the
>    sender (across local reboots, etc.), until the key is replaced, and
>    there likely would be no automated recovery provision if counter
>    overflow were imminent." — §5, `rfc4303.txt:1866-1871`

- Strength: description. Class: internal.
- Check idea: reboot a sender using a manually keyed SA with anti-replay enabled and check whether its sequence number counter state survives correctly.

### RFC4303-CONF-5

**A compliant implementation should not provide anti-replay service for SAs that are manually keyed.**

> "a compliant implementation SHOULD NOT
>    provide anti-replay service in conjunction with SAs that are manually
>    keyed." — §5, `rfc4303.txt:1871-1873`

- Strength: should not. Class: internal.
- Check idea: configure a manually keyed SA and check that a compliant implementation does not offer anti-replay service for it.

### RFC4303-CONF-6

**The mandatory-to-implement algorithms for ESP are defined in a separate document, so algorithm requirements can be updated independently of the protocol.**

> "The mandatory-to-implement algorithms for use with ESP are described
>    in a separate document [Eas04], to facilitate updating the algorithm
>    requirements independently from the protocol per se." — §5, `rfc4303.txt:1875-1877`

- Strength: description. Class: internal.
- Check idea: check that the mandatory-to-implement algorithms an implementation must support come from the separate algorithm-requirements document rather than from this specification.

### RFC4303-CONF-7

**An implementation may support additional algorithms beyond those mandated for ESP.**

> "Additional
>    algorithms, beyond those mandated for ESP, MAY be supported." — §5, `rfc4303.txt:1877-1878`

- Strength: may. Class: internal.
- Check idea: configure an SA with an algorithm outside the mandatory-to-implement set and check that a conformant implementation processes packets on it.

### RFC4303-CONF-8

**A conformant implementation supports the NULL encryption algorithm, to stay consistent with how ESP services are negotiated, because encryption in ESP is optional.**

> "Because use of encryption in ESP is optional, support for the "NULL"
>    encryption algorithm also is required to maintain consistency with
>    the way ESP services are negotiated." — §5, `rfc4303.txt:1880-1882`

- Strength: description. Class: internal.
- Check idea: configure an SA with the NULL encryption algorithm and check that a conformant implementation accepts and processes it.

### RFC4303-CONF-9

**Support for the confidentiality-only version of the ESP service is optional.**

> "Support for the
>    confidentiality-only service version of ESP is optional." — §5, `rfc4303.txt:1882-1883`

- Strength: description. Class: internal.
- Check idea: check whether an implementation that offers only the confidentiality-only ESP service still claims conformance without offering the full service.

### RFC4303-CONF-10

**An implementation that offers the confidentiality-only service must also support negotiating the NULL integrity algorithm.**

> "If an
>    implementation offers this service, it MUST also support the
>    negotiation of the "NULL" integrity algorithm." — §5, `rfc4303.txt:1883-1885`

- Strength: must. Class: internal.
- Check idea: check an implementation offering the confidentiality-only ESP service for support of negotiating the NULL integrity algorithm.

### RFC4303-CONF-11

**An SA must not set both the integrity and encryption algorithms to NULL at the same time, even though each one may individually be NULL.**

> "NOTE that although
>    integrity and encryption may each be "NULL" under the circumstances
>    noted above, they MUST NOT both be "NULL"." — §5, `rfc4303.txt:1885-1887`

- Strength: must not (both algorithms being NULL at the same time); may (lower case, either one being NULL on its own). Class: internal.
- Check idea: configure an SA and check that its encryption and integrity algorithms are never both NULL at once.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§2 to §3.4 and §5, and every field of every header. What the catalog leaves out:

- **The sections outside the in-scope set**: §1 (introduction), §4 (auditing), §6 to §10
  (security, the changes from RFC 2406, backward compatibility, references) and the
  appendices.

Within the in-scope sections, these parts have no entry, with the reason:

In §2 to §3.1:

- `rfc4303.txt:271-274` and `rfc4303.txt:710-713`: the Figure 1 and Payload Data footnotes on
  whether an explicit IV "usually is not encrypted per se" and is "sometimes" called part of
  the ciphertext are hedged, informative remarks about typical algorithm behavior, not a
  requirement or field value; the operative facts about IV placement and ciphertext scope are
  cataloged in RFC4303-FMT-6, RFC4303-FMT-7, and RFC4303-PAY-3.
- `rfc4303.txt:300-309`: the definition of "combined mode algorithms" and the note that
  "accommodation of combined mode algorithms requires that the algorithm explicitly describe
  the payload substructure" is a requirement on the algorithm-defining RFC, not on an IPsec
  implementation; it only sets terminology used elsewhere.
- `rfc4303.txt:318-320`: "the algorithm itself is expected to return both decrypted plaintext
  and a pass/fail indication" describes internal algorithm behavior assumed by the
  specification, not a separately checkable requirement on the ESP implementation beyond what
  RFC4303-FMT-8/9 already state.
- `rfc4303.txt:384-425` and `rfc4303.txt:455-485` (Tables 1 and 2): the per-field
  mandatory/optional/dummy column and the byte-count column restate facts already stated in
  prose and cataloged under their own fields (SPI/SEQ mandatory-ness in §2.1/§2.2, Payload Data
  in §2.3, Padding/Pad Length/Next Header/ICV in §2.4-2.8); the novel facts the tables add
  (SPI/Sequence Number never encrypted, always integrity-protected; ICV optional under combined
  mode) are cataloged in RFC4303-FMT-7 and RFC4303-FMT-8. The bracketed footnotes ([1]-[6] under
  each table) are explanatory keys to the table, not separate statements.
- `rfc4303.txt:483-484` (Table 2 note [6], "The algorithm spec determines whether this field is
  present") and `rfc4303.txt:480-482` (note [5], "the result is invisible to ESP"): restate, as
  table footnotes, facts already cataloged from the prose (RFC4303-SEQ-16, RFC4303-FMT-8).
- `rfc4303.txt:536-540`: the description of a Group Controller/Key Server unilaterally
  assigning a multicast SPI, and that this is not coordinated with IKE, describes the key
  management protocol's own operation, not a requirement on the IPsec implementation beyond
  the collision-handling MUST already cataloged as RFC4303-SPI-7.
- `rfc4303.txt:586-594`: the software hash-table and hardware TCAM examples of how an
  implementation might accelerate the SAD search are illustrative examples, not requirements.
- `rfc4303.txt:606-609`: the statement that SPI values 1-255 are reserved by IANA, and "will
  not normally be assigned" outside an RFC, is IANA registry policy, not a requirement on an
  IPsec implementation.
- `rfc4303.txt:611-613`, `rfc4303.txt:623-625`: the parenthetical example of a key management
  implementation using SPI zero locally to mean "No Security Association Exists" is an
  illustrative example of the already-cataloged RFC4303-SPI-18, not a separate statement.
- `rfc4303.txt:664-666`: "in IKEv2, this negotiation is implicit; the default is ESN unless
  32-bit sequence numbers are explicitly negotiated" describes IKEv2's own negotiation
  behavior, a key management protocol, not a requirement on the ESP data-plane implementation
  beyond RFC4303-SEQ-11.
- `rfc4303.txt:704-709`: "any encryption algorithm that requires such explicit, per-packet
  synchronization data MUST indicate the length... as part of an RFC" and "the algorithm for
  deriving the data MUST be part of the algorithm definition RFC" are requirements on the
  RFC defining the encryption algorithm, not on an IPsec implementation.
- `rfc4303.txt:723-727`: the description of some IV-based modes where "alignment of the start
  of the (real) ciphertext is not an issue at the receiver" is informative background
  contrasting with the next case, with no independent requirement of its own.
- `rfc4303.txt:735-738`: "the algorithm specification MUST address how alignment of the (real)
  ciphertext is to be achieved" is a requirement on the algorithm specification document, not
  on an IPsec implementation.
- `rfc4303.txt:759-761`: "Padding beyond that required for the algorithm or alignment reasons
  cited above could be used to conceal the actual length of the payload, in support of TFC" is
  an informative lead-in to RFC4303-PAD-3's recommendation, adding no separate checkable claim.
- `rfc4303.txt:786-793` (parenthetical on ESN in the padding computation) is folded into
  RFC4303-PAD-7's quote rather than left out, but the surrounding rationale text has no
  separate claim.
- `rfc4303.txt:810-814`: the parenthetical rationale for why the default padding scheme "was
  selected" (simplicity, hardware ease, cut-and-paste resistance) is design rationale, not a
  checkable requirement.
- `rfc4303.txt:816-820`: "If an encryption or combined mode algorithm imposes constraints on
  the values of the bytes used for padding, they MUST be specified by the RFC defining how the
  algorithm is employed with ESP. If the algorithm requires checking of the values of the
  bytes used for padding, this too MUST be specified in that RFC" are requirements on the
  algorithm-defining RFC, not on an IPsec implementation.
- `rfc4303.txt:876-888`: the "DISCUSSION" paragraph on strategies for masking traffic with
  dummy packets (constant bit rate shaping, bandwidth trade-offs across multiple SAs) is
  explicitly informative discussion, not a requirement; its closing SHOULD is folded into
  RFC4303-NH-6.
- `rfc4303.txt:927-929`: "In principle, existing IPsec implementations could have made use of
  this capability previously, in a transparent fashion" is historical/informative framing for
  the negotiation requirement already cataloged as RFC4303-TFC-7.
- `rfc4303.txt:932-935`: the summary sentence that combining protocol ID 59 dummy packets with
  TFC padding lets an implementation generate traffic with "much greater length variability"
  restates, informally, facts already cataloged as RFC4303-NH-2 and RFC4303-TFC-1/2, with no
  new claim.
- `rfc4303.txt:960-962`: "The integrity algorithm specification MUST specify the length of the
  ICV and the comparison rules and processing steps for validation" is a requirement on the
  integrity algorithm's specification document, not on an IPsec implementation.
- `rfc4303.txt:977-978`, `rfc4303.txt:1029`, `rfc4303.txt:1037`: parenthetical asides ("Note
  that the term 'transport' mode should not be misconstrued...", the "* = if present..."
  diagram footnotes, "Special care is required... when multiple interfaces are in use") are
  editorial or cross-reference notes, not independently checkable statements.
- `rfc4303.txt:980-982`: "This and subsequent diagrams in this section show the ICV field, the
  presence of which is a function of the security services and the algorithm/mode selected"
  restates the already-cataloged optionality of the ICV (RFC4303-ICV-3).
- `rfc4303.txt:984-994`, `rfc4303.txt:1015-1029`, `rfc4303.txt:1071-1101`: the before/after
  packet-layout diagrams for IPv4 and IPv6, transport and tunnel mode, illustrate the field
  order and mode positioning already cataloged in RFC4303-FMT-2, RFC4303-LOC-2/3/5/6/9/12/13;
  the diagrams are quoted directly only where they add a fact the prose does not state (the
  encryption/integrity coverage spans, RFC4303-LOC-5 and RFC4303-LOC-13).
- `rfc4303.txt:1102-1105`: the diagram footnote pointing to the Security Architecture document
  for how outer/inner IP header construction and modification works is a cross-reference, not
  a checkable statement of this document.

In §3.2 to §3.4, and §5:

- `rfc4303.txt:1108-1110`: background statement that the mandatory-to-implement algorithms are described in a separate RFC; states no behavior of an ESP implementation.
- `rfc4303.txt:1137-1144`: parenthetical example about deriving an IV from the Sequence Number and its FIPS 140-2 evaluation impact; informative example and rationale only.
- `rfc4303.txt:1145-1147`: "encryption algorithms employed with ESP may exhibit either block or stream mode characteristics" describes algorithm characteristics in general, not a requirement of an ESP implementation.
- `rfc4303.txt:1151-1154`: the padding-modulus requirement is addressed to "the RFC for each encryption algorithm used with ESP", not to an ESP implementation.
- `rfc4303.txt:1162-1164`: "The same admonition noted above applies to..." is a cross-reference to the earlier IV admonition, with no new content.
- `rfc4303.txt:1167-1169`: the padding-modulus requirement is addressed to "the RFC for each algorithm used with ESP", not to an ESP implementation.
- `rfc4303.txt:1190-1192`: "The means by which a combined mode algorithm provides integrity...may vary for different algorithm choices" notes variability without stating a requirement.
- `rfc4303.txt:1194-1196`: "For example, the SPI and Sequence Number fields might be replicated..." is an example only, not a rule.
- `rfc4303.txt:1199-1202`: the MTU-formula requirement is addressed to "the RFC for each [combined mode] algorithm used with ESP", not to an ESP implementation.
- `rfc4303.txt:1209-1213`: "In tunnel mode, the outer and inner IP header/extensions can be interrelated in a variety of ways. The construction..." defers the actual tunnel-mode header construction rules to the Security Architecture document (RFC 4301); states no rule of its own.
- `rfc4303.txt:1219-1221`: "The process of determining what, if any, IPsec processing is applied to outbound traffic is described in the Security Architecture document" is a cross-reference to RFC 4301.
- `rfc4303.txt:1225-1229`: "In this section, we speak in terms of encryption always being applied..." is explanatory framing for how the text is written, not a requirement.
- `rfc4303.txt:1266-1272`: "This order of processing facilitates rapid detection and rejection of replayed or bogus packets..." is rationale for the ordering rule already catalogued (RFC4303-OENC-7).
- `rfc4303.txt:1305-1306`: "The document that defines an integrity algorithm MUST be consulted..." directs the implementer to consult the algorithm's defining document; not itself a behavior a traffic test observes.
- `rfc4303.txt:1340-1352`: "The means by which these values are included in this computation are a function of the combined mode algorithm employed and thus not specified in this standard" states that this specification leaves the mechanism to the algorithm's own document; no independent rule here.
- `rfc4303.txt:1355-1356`: "If one is not used, an analogous field usually will be a part of the ciphertext payload" is an informal ("usually") description of an alternative outcome, not a specific requirement.
- `rfc4303.txt:1356-1361`: "The location of any integrity fields...MUST be defined in an RFC that defines the use of the combined mode algorithm with ESP" is a requirement addressed to that other RFC, not to an ESP implementation.
- `rfc4303.txt:1418-1423`: "Note: If a receiver chooses to not enable anti-replay for an SA, then the receiver SHOULD NOT negotiate ESN..." constrains SA negotiation through a key-management protocol (IKE); the "Use of ESN creates a need..." sentence is rationale for it. Neither is a behavior a test on ESP traffic itself can observe.
- `rfc4303.txt:1448-1451`: "Fragmentation...significantly reduces performance. Moreover, the requirement for an ESP receiver to accept fragments...creates denial of service vulnerabilities" is rationale for the rules that follow, not itself a requirement.
- `rfc4303.txt:1465-1466`: "Details of the support required for MTU management are contained in the Security Architecture document" is a cross-reference to RFC 4301.
- `rfc4303.txt:1530-1532`: "this standard specifies no mechanisms for providing anti-replay for a multi-sender SA" states a gap in the standard's own scope, not a behavior of an implementation.
- `rfc4303.txt:1556-1560`: "This capability is important whenever an ESP implementation...is not capable of performing decryption and/or integrity checking at the same rate..." is rationale for why two-stage verification exists.
- `rfc4303.txt:1581-1583`: "How the window is implemented is a local matter, but the following text describes the functionality that the implementation must exhibit" is a meta-statement introducing the rules that follow; no independent content of its own.
- `rfc4303.txt:1598-1600`: "(This algorithm accommodates gaps in reception for a single SA as large as 2\*\*32-1 packets" is an informative characterization of the algorithm's capacity, context for RFC4303-ISEQ-20, not a separate requirement.
- `rfc4303.txt:1632-1634`: "Values for minimum and recommended receive window sizes for very high-speed...devices are not specified by this standard" states that the standard leaves this unspecified; nothing to check.
- `rfc4303.txt:1638-1639`: "As with outbound processing, there are several options for inbound processing..." is scaffolding introducing the subsections that follow.
- `rfc4303.txt:1643-1644`: "If separate confidentiality and integrity algorithms are employed processing proceeds as follows:" is scaffolding.
- `rfc4303.txt:1649-1650`: "Details of the computation are provided below" is a forward reference, no independent content.
- `rfc4303.txt:1677-1689`: "As in Section 3.3.2, we speak here in terms of encryption always being applied..." is explanatory framing repeated from §3.3.2.
- `rfc4303.txt:1719-1721`: "The exact steps for reconstructing the original datagram depend on the mode...are described in the Security Architecture document" is a cross-reference to RFC 4301.
- `rfc4303.txt:1733-1735`, `rfc4303.txt:1743-1744`: "This order of processing facilitates rapid detection and rejection of replayed or bogus packets..." is rationale for the ordering rule already catalogued (RFC4303-IICV-18).
- `rfc4303.txt:1752-1753`: "If a combined confidentiality and integrity algorithm is employed, then the receiver proceeds as follows:" is scaffolding.
