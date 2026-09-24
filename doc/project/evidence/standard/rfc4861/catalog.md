# RFC 4861 (Neighbor Discovery for IP version 6 (IPv6)) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC4861-*` · **Stands on:** [standards.md](../../protocol/nd/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 4861, Neighbor Discovery for IP version 6 (IPv6), of
September 2007, a document of the in-scope set of Neighbor Discovery. The catalog comes from the RFC
text only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc4861.txt`](../../../../../../standards/RFC/rfc4861.txt) —
  Neighbor Discovery for IP version 6 (IPv6), September 2007. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc4861.txt>.

The other documents of the in-scope set have catalogs of their own:
[`rfc4862/catalog.md`](../rfc4862/catalog.md) (stateless address autoconfiguration),
[`rfc5942/catalog.md`](../rfc5942/catalog.md) (the on-link determination, which replaces two
bullets of the definition in §2.1 of this document) and
[`rfc6980/catalog.md`](../rfc6980/catalog.md) (no fragmentation of Neighbor Discovery messages).

The in-scope sections are §4, §6, §7.2 and §8. Three flaws of the text itself show in the
entries, and the entries quote the text as it stands: §7.2.6 counts RetransTimer in seconds
where the rest of the document counts it in milliseconds (RFC4861-AR-58); the default of
MinRtrAdvInterval for a small MaxRtrAdvInterval breaks the upper bound that the same section
sets (RFC4861-RCFG-6); and §6.2.3 names the variable "CurHopLimit" that §6.2.1 calls
AdvCurHopLimit (RFC4861-ADV-7).

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/nd/standards.md). The features these statements build are in
[`features.md`](../../protocol/nd/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`nd/coverage.md`](../../model/nd/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc4861.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC4861-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 4861 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them, and
  `description` for a normative sentence with no keyword. Where the text writes a keyword in
  lower case, the entry records the word and says so, for example `must (lower case)`; a
  "might" stays `description`. An entry with two keywords names what each one covers.
- **Class** — how a test can observe the statement: `wire` (fields of messages on a link, or
  the presence or absence of a message), `end-to-end` (what a node accepts and acts on, seen
  in what it sends or forwards afterwards), `error-signal` (an ICMPv6 error report),
  `internal` (state inside a node), `encoding` (the exact bit layout).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC4861-RS-1](#rfc4861-rs-1) | A host sends Router Solicitations to make routers send Router Advertisements quickly. |
| [RFC4861-RS-2](#rfc4861-rs-2) | A Router Solicitation holds Type, Code, Checksum, a 32-bit Reserved field, and then options. |
| [RFC4861-RS-3](#rfc4861-rs-3) | The Source Address of a Router Solicitation is an address of the interface that sends it, or :: when that interface has no address. |
| [RFC4861-RS-4](#rfc4861-rs-4) | The Destination Address of a Router Solicitation is typically the all-routers multicast address. |
| [RFC4861-RS-5](#rfc4861-rs-5) | A Router Solicitation has IP Hop Limit 255. |
| [RFC4861-RS-6](#rfc4861-rs-6) | A Router Solicitation has ICMP Type 133. |
| [RFC4861-RS-7](#rfc4861-rs-7) | A Router Solicitation has ICMP Code 0. |
| [RFC4861-RS-8](#rfc4861-rs-8) | The Checksum field of a Router Solicitation holds the ICMPv6 checksum. |
| [RFC4861-RS-9](#rfc4861-rs-9) | The sender sets the Reserved field of a Router Solicitation to zero. |
| [RFC4861-RS-10](#rfc4861-rs-10) | A receiver ignores the value of the Reserved field of a Router Solicitation. |
| [RFC4861-RS-11](#rfc4861-rs-11) | A Router Solicitation from the unspecified address carries no Source Link-Layer Address option. |
| [RFC4861-RS-12](#rfc4861-rs-12) | On link layers with addresses, a Router Solicitation from an assigned address carries a Source Link-Layer Address option with the sender's link-layer address. |
| [RFC4861-RS-13](#rfc4861-rs-13) | A receiver of a Router Solicitation silently ignores an option that it does not recognize, and it processes the rest of the message. |
| [RFC4861-RA-1](#rfc4861-ra-1) | A router sends Router Advertisements at intervals, and also in answer to Router Solicitations. |
| [RFC4861-RA-2](#rfc4861-ra-2) | After the ICMP header, a Router Advertisement holds Cur Hop Limit, M, O, Reserved, Router Lifetime, Reachable Time, Retrans Timer, and then options. |
| [RFC4861-RA-3](#rfc4861-ra-3) | A router sends a Router Advertisement from the link-local address of the interface that sends it. |
| [RFC4861-RA-4](#rfc4861-ra-4) | The Destination Address of a Router Advertisement is typically the source of the Router Solicitation that caused it, or the all-nodes multicast address. |
| [RFC4861-RA-5](#rfc4861-ra-5) | A Router Advertisement has IP Hop Limit 255. |
| [RFC4861-RA-6](#rfc4861-ra-6) | A Router Advertisement has ICMP Type 134. |
| [RFC4861-RA-7](#rfc4861-ra-7) | A Router Advertisement has ICMP Code 0. |
| [RFC4861-RA-8](#rfc4861-ra-8) | The Checksum field of a Router Advertisement holds the ICMPv6 checksum. |
| [RFC4861-RA-9](#rfc4861-ra-9) | Cur Hop Limit is the default Hop Limit that a host puts in the IP header of the packets that it sends. |
| [RFC4861-RA-10](#rfc4861-ra-10) | A Cur Hop Limit of zero means that the router does not specify a value. |
| [RFC4861-RA-11](#rfc4861-ra-11) | When the M flag is set, it tells hosts that addresses are available through DHCPv6. |
| [RFC4861-RA-12](#rfc4861-ra-12) | When the O flag is set, it tells hosts that other configuration information is available through DHCPv6. |
| [RFC4861-RA-13](#rfc4861-ra-13) | The sender sets the 6-bit Reserved field of a Router Advertisement to zero. |
| [RFC4861-RA-14](#rfc4861-ra-14) | A receiver ignores the value of the Reserved field of a Router Advertisement. |
| [RFC4861-RA-15](#rfc4861-ra-15) | Router Lifetime is the time, in seconds, for which the sender is a default router. |
| [RFC4861-RA-16](#rfc4861-ra-16) | A receiver accepts any Router Lifetime up to 65535 seconds, although the rules for senders limit the value to 9000 seconds. |
| [RFC4861-RA-17](#rfc4861-ra-17) | A router that advertises Router Lifetime 0 is not a default router, and a host does not put it in its default router list. |
| [RFC4861-RA-18](#rfc4861-ra-18) | Router Lifetime applies only to the default router role of the sender; it does not limit the other fields or options of the message. |
| [RFC4861-RA-19](#rfc4861-ra-19) | Reachable Time is the time, in milliseconds, for which a node assumes that a neighbor is reachable after a reachability confirmation. |
| [RFC4861-RA-20](#rfc4861-ra-20) | A Reachable Time of zero means that the router does not specify a value. |
| [RFC4861-RA-21](#rfc4861-ra-21) | Retrans Timer is the time, in milliseconds, between two retransmitted Neighbor Solicitations. |
| [RFC4861-RA-22](#rfc4861-ra-22) | A Retrans Timer of zero means that the router does not specify a value. |
| [RFC4861-RA-23](#rfc4861-ra-23) | The Source Link-Layer Address option of a Router Advertisement holds the link-layer address of the interface that sends it. |
| [RFC4861-RA-24](#rfc4861-ra-24) | A router may leave out the Source Link-Layer Address option, to share inbound load across several link-layer addresses. |
| [RFC4861-RA-25](#rfc4861-ra-25) | A router sends the MTU option in its Router Advertisements on links that have a variable MTU. |
| [RFC4861-RA-26](#rfc4861-ra-26) | A router may send the MTU option on other links too. |
| [RFC4861-RA-27](#rfc4861-ra-27) | A router includes a Prefix Information option for each of its on-link prefixes, except the link-local prefix. |
| [RFC4861-RA-28](#rfc4861-ra-28) | A receiver of a Router Advertisement silently ignores an option that it does not recognize, and it processes the rest of the message. |
| [RFC4861-NS-1](#rfc4861-ns-1) | A node sends a Neighbor Solicitation to ask for the link-layer address of a target, and it gives its own link-layer address to the target. |
| [RFC4861-NS-2](#rfc4861-ns-2) | A node multicasts a Neighbor Solicitation to resolve an address, and unicasts it to verify that a neighbor is reachable. |
| [RFC4861-NS-3](#rfc4861-ns-3) | A Neighbor Solicitation holds Type, Code, Checksum, a 32-bit Reserved field, the 128-bit Target Address, and then options. |
| [RFC4861-NS-4](#rfc4861-ns-4) | The Source Address of a Neighbor Solicitation is an address of the interface that sends it, or the unspecified address during Duplicate Address Detection. |
| [RFC4861-NS-5](#rfc4861-ns-5) | The Destination Address of a Neighbor Solicitation is the solicited-node multicast address of the target, or the target address. |
| [RFC4861-NS-6](#rfc4861-ns-6) | A Neighbor Solicitation has IP Hop Limit 255. |
| [RFC4861-NS-7](#rfc4861-ns-7) | A Neighbor Solicitation has ICMP Type 135. |
| [RFC4861-NS-8](#rfc4861-ns-8) | A Neighbor Solicitation has ICMP Code 0. |
| [RFC4861-NS-9](#rfc4861-ns-9) | The Checksum field of a Neighbor Solicitation holds the ICMPv6 checksum. |
| [RFC4861-NS-10](#rfc4861-ns-10) | The sender sets the Reserved field of a Neighbor Solicitation to zero. |
| [RFC4861-NS-11](#rfc4861-ns-11) | A receiver ignores the value of the Reserved field of a Neighbor Solicitation. |
| [RFC4861-NS-12](#rfc4861-ns-12) | The Target Address of a Neighbor Solicitation is the IP address of the target of the solicitation. |
| [RFC4861-NS-13](#rfc4861-ns-13) | The Target Address of a Neighbor Solicitation is never a multicast address. |
| [RFC4861-NS-14](#rfc4861-ns-14) | A Neighbor Solicitation from the unspecified address carries no Source Link-Layer Address option. |
| [RFC4861-NS-15](#rfc4861-ns-15) | On link layers with addresses, a multicast Neighbor Solicitation from an assigned address carries a Source Link-Layer Address option with the sender's link-layer address. |
| [RFC4861-NS-16](#rfc4861-ns-16) | A unicast Neighbor Solicitation carries a Source Link-Layer Address option, on link layers that have addresses. |
| [RFC4861-NS-17](#rfc4861-ns-17) | A receiver of a Neighbor Solicitation silently ignores an option that it does not recognize, and it processes the rest of the message. |
| [RFC4861-NA-1](#rfc4861-na-1) | A node sends Neighbor Advertisements in answer to Neighbor Solicitations, and it sends unsolicited Neighbor Advertisements to spread new information quickly. |
| [RFC4861-NA-2](#rfc4861-na-2) | A Neighbor Advertisement holds Type, Code, Checksum, the R, S and O flags, a 29-bit Reserved field, the 128-bit Target Address, and then options. |
| [RFC4861-NA-3](#rfc4861-na-3) | The Source Address of a Neighbor Advertisement is an address of the interface that sends it. |
| [RFC4861-NA-4](#rfc4861-na-4) | A solicited Neighbor Advertisement goes to the source of the Neighbor Solicitation, or to the all-nodes multicast address when that source is the unspecified address. |
| [RFC4861-NA-5](#rfc4861-na-5) | An unsolicited Neighbor Advertisement typically goes to the all-nodes multicast address. |
| [RFC4861-NA-6](#rfc4861-na-6) | A Neighbor Advertisement has IP Hop Limit 255. |
| [RFC4861-NA-7](#rfc4861-na-7) | A Neighbor Advertisement has ICMP Type 136. |
| [RFC4861-NA-8](#rfc4861-na-8) | A Neighbor Advertisement has ICMP Code 0. |
| [RFC4861-NA-9](#rfc4861-na-9) | The Checksum field of a Neighbor Advertisement holds the ICMPv6 checksum. |
| [RFC4861-NA-10](#rfc4861-na-10) | The R flag is set when the sender of the Neighbor Advertisement is a router. |
| [RFC4861-NA-11](#rfc4861-na-11) | Neighbor Unreachability Detection uses the R flag to find a router that becomes a host. |
| [RFC4861-NA-12](#rfc4861-na-12) | The S flag is set when the Neighbor Advertisement answers a Neighbor Solicitation from the Destination Address. |
| [RFC4861-NA-13](#rfc4861-na-13) | Neighbor Unreachability Detection uses the S flag as a reachability confirmation. |
| [RFC4861-NA-14](#rfc4861-na-14) | The S flag is clear in multicast Neighbor Advertisements and in unsolicited unicast Neighbor Advertisements. |
| [RFC4861-NA-15](#rfc4861-na-15) | When the O flag is set, the Neighbor Advertisement overrides a cache entry and updates the cached link-layer address. |
| [RFC4861-NA-16](#rfc4861-na-16) | When the O flag is clear, the Neighbor Advertisement does not change a cached link-layer address. But it fills a cache entry that has no link-layer address. |
| [RFC4861-NA-17](#rfc4861-na-17) | The O flag is clear in a solicited Neighbor Advertisement for an anycast address and in a solicited proxy Neighbor Advertisement. |
| [RFC4861-NA-18](#rfc4861-na-18) | The O flag is set in all other solicited Neighbor Advertisements and in unsolicited Neighbor Advertisements. |
| [RFC4861-NA-19](#rfc4861-na-19) | The sender sets the 29-bit Reserved field of a Neighbor Advertisement to zero. |
| [RFC4861-NA-20](#rfc4861-na-20) | A receiver ignores the value of the Reserved field of a Neighbor Advertisement. |
| [RFC4861-NA-21](#rfc4861-na-21) | The Target Address of a solicited Neighbor Advertisement is the Target Address of the Neighbor Solicitation that caused it. |
| [RFC4861-NA-22](#rfc4861-na-22) | The Target Address of an unsolicited Neighbor Advertisement is the address whose link-layer address changed. |
| [RFC4861-NA-23](#rfc4861-na-23) | The Target Address of a Neighbor Advertisement is never a multicast address. |
| [RFC4861-NA-24](#rfc4861-na-24) | On link layers with addresses, a Neighbor Advertisement in answer to a multicast solicitation carries a Target Link-Layer Address option with the sender's link-layer address. |
| [RFC4861-NA-25](#rfc4861-na-25) | A Neighbor Advertisement in answer to a unicast Neighbor Solicitation carries a Target Link-Layer Address option. |
| [RFC4861-NA-26](#rfc4861-na-26) | A receiver of a Neighbor Advertisement silently ignores an option that it does not recognize, and it processes the rest of the message. |
| [RFC4861-RDM-1](#rfc4861-rdm-1) | A router sends a Redirect to tell a host about a better first hop for a destination: a router or the destination itself. |
| [RFC4861-RDM-2](#rfc4861-rdm-2) | A Redirect holds Type, Code, Checksum, a 32-bit Reserved field, the 128-bit Target Address, the 128-bit Destination Address, and then options. |
| [RFC4861-RDM-3](#rfc4861-rdm-3) | A router sends a Redirect from the link-local address of the interface that sends it. |
| [RFC4861-RDM-4](#rfc4861-rdm-4) | A Redirect goes to the Source Address of the packet that caused it. |
| [RFC4861-RDM-5](#rfc4861-rdm-5) | A Redirect has IP Hop Limit 255. |
| [RFC4861-RDM-6](#rfc4861-rdm-6) | A Redirect has ICMP Type 137. |
| [RFC4861-RDM-7](#rfc4861-rdm-7) | A Redirect has ICMP Code 0. |
| [RFC4861-RDM-8](#rfc4861-rdm-8) | The Checksum field of a Redirect holds the ICMPv6 checksum. |
| [RFC4861-RDM-9](#rfc4861-rdm-9) | The sender sets the Reserved field of a Redirect to zero. |
| [RFC4861-RDM-10](#rfc4861-rdm-10) | A receiver ignores the value of the Reserved field of a Redirect. |
| [RFC4861-RDM-11](#rfc4861-rdm-11) | The Target Address of a Redirect is a better first hop for the ICMP Destination Address. |
| [RFC4861-RDM-12](#rfc4861-rdm-12) | When the destination is a neighbor of the host, the Target Address of the Redirect is equal to the ICMP Destination Address. |
| [RFC4861-RDM-13](#rfc4861-rdm-13) | When the target is a better first-hop router, the Target Address of the Redirect is the link-local address of that router. |
| [RFC4861-RDM-14](#rfc4861-rdm-14) | The ICMP Destination Address of a Redirect is the destination that the Redirect sends to the target. |
| [RFC4861-RDM-15](#rfc4861-rdm-15) | A Redirect carries a Target Link-Layer Address option with the link-layer address of the target, when the router knows it. |
| [RFC4861-RDM-16](#rfc4861-rdm-16) | On an NBMA link where hosts learn link-layer addresses from Redirects, every Redirect carries the Target Link-Layer Address option. |
| [RFC4861-RDM-17](#rfc4861-rdm-17) | The Redirected Header option holds as much of the packet that caused the Redirect as fits in the minimum IPv6 MTU. |
| [RFC4861-OPT-1](#rfc4861-opt-1) | A Neighbor Discovery message holds zero or more options, and some options can occur more than once in one message. |
| [RFC4861-OPT-2](#rfc4861-opt-2) | Each option is padded, when necessary, so that it ends on a 64-bit boundary. |
| [RFC4861-OPT-3](#rfc4861-opt-3) | Every option starts with an 8-bit Type and an 8-bit Length, and the option data follows. |
| [RFC4861-OPT-4](#rfc4861-opt-4) | The option types are 1 (Source Link-Layer Address), 2 (Target Link-Layer Address), 3 (Prefix Information), 4 (Redirected Header) and 5 (MTU). |
| [RFC4861-OPT-5](#rfc4861-opt-5) | The Length of an option is its size in units of 8 octets, and the size includes the Type and Length fields. |
| [RFC4861-OPT-6](#rfc4861-opt-6) | A node silently discards a Neighbor Discovery packet that contains an option with Length zero. |
| [RFC4861-OPT-7](#rfc4861-opt-7) | A link-layer address option holds an 8-bit Type, an 8-bit Length, and the link-layer address. |
| [RFC4861-OPT-8](#rfc4861-opt-8) | The Type of the Source Link-Layer Address option is 1, and the Type of the Target Link-Layer Address option is 2. |
| [RFC4861-OPT-9](#rfc4861-opt-9) | The Length of a link-layer address option counts 8-octet units, and it is 1 for IEEE 802 addresses. |
| [RFC4861-OPT-10](#rfc4861-opt-10) | The Link-Layer Address field holds the link-layer address of variable length, in the format of the document for the link layer. |
| [RFC4861-OPT-11](#rfc4861-opt-11) | The Source Link-Layer Address option holds the link-layer address of the sender, and it occurs in Neighbor Solicitations, Router Solicitations and Router Advertisements. |
| [RFC4861-OPT-12](#rfc4861-opt-12) | The Target Link-Layer Address option holds the link-layer address of the target, and it occurs in Neighbor Advertisements and Redirects. |
| [RFC4861-OPT-13](#rfc4861-opt-13) | A node silently ignores a link-layer address option in a Neighbor Discovery message that does not use that option. |
| [RFC4861-OPT-14](#rfc4861-opt-14) | A Prefix Information option holds Type, Length, Prefix Length, the L and A flags, Reserved1, Valid Lifetime, Preferred Lifetime, Reserved2, and the 128-bit Prefix. |
| [RFC4861-OPT-15](#rfc4861-opt-15) | The Type of the Prefix Information option is 3. |
| [RFC4861-OPT-16](#rfc4861-opt-16) | The Length of the Prefix Information option is 4. |
| [RFC4861-OPT-17](#rfc4861-opt-17) | Prefix Length is the number of valid bits at the start of the Prefix, from 0 to 128. |
| [RFC4861-OPT-18](#rfc4861-opt-18) | A host uses the Prefix Length, together with the L flag, to decide which addresses are on-link. |
| [RFC4861-OPT-19](#rfc4861-opt-19) | When the L flag is set, a host can use the prefix for on-link determination. |
| [RFC4861-OPT-20](#rfc4861-opt-20) | When the L flag is clear, a host does not conclude that the prefix is off-link. It keeps an earlier on-link indication. |
| [RFC4861-OPT-21](#rfc4861-opt-21) | When the A flag is set, a host can use the prefix for stateless address autoconfiguration. |
| [RFC4861-OPT-22](#rfc4861-opt-22) | The sender sets the Reserved1 field of a Prefix Information option to zero. |
| [RFC4861-OPT-23](#rfc4861-opt-23) | A receiver ignores the value of the Reserved1 field of a Prefix Information option. |
| [RFC4861-OPT-24](#rfc4861-opt-24) | Valid Lifetime is the time, in seconds from the transmission of the packet, for which the prefix is valid for on-link determination. |
| [RFC4861-OPT-25](#rfc4861-opt-25) | A Valid Lifetime of all one bits (0xffffffff) means infinity. |
| [RFC4861-OPT-26](#rfc4861-opt-26) | Preferred Lifetime is the time, in seconds from the transmission of the packet, for which addresses formed from the prefix stay preferred. |
| [RFC4861-OPT-27](#rfc4861-opt-27) | A Preferred Lifetime of all one bits (0xffffffff) means infinity. |
| [RFC4861-OPT-28](#rfc4861-opt-28) | The Preferred Lifetime of a Prefix Information option is not greater than its Valid Lifetime. |
| [RFC4861-OPT-29](#rfc4861-opt-29) | The sender sets the Reserved2 field of a Prefix Information option to zero. |
| [RFC4861-OPT-30](#rfc4861-opt-30) | A receiver ignores the value of the Reserved2 field of a Prefix Information option. |
| [RFC4861-OPT-31](#rfc4861-opt-31) | The Prefix field holds an IP address or a prefix of one, and Prefix Length gives the number of its valid first bits. |
| [RFC4861-OPT-32](#rfc4861-opt-32) | The sender sets the bits of the Prefix after the prefix length to zero. |
| [RFC4861-OPT-33](#rfc4861-opt-33) | A receiver ignores the bits of the Prefix after the prefix length. |
| [RFC4861-OPT-34](#rfc4861-opt-34) | A router does not send a Prefix Information option for the link-local prefix. |
| [RFC4861-OPT-35](#rfc4861-opt-35) | A host ignores a Prefix Information option for the link-local prefix. |
| [RFC4861-OPT-36](#rfc4861-opt-36) | The Prefix Information option occurs in Router Advertisements, and a node silently ignores it in other messages. |
| [RFC4861-OPT-37](#rfc4861-opt-37) | A Redirected Header option holds Type, Length, 6 octets of Reserved, and then the IP header and data of the redirected packet. |
| [RFC4861-OPT-38](#rfc4861-opt-38) | The Type of the Redirected Header option is 4. |
| [RFC4861-OPT-39](#rfc4861-opt-39) | The Length of the Redirected Header option is its size in units of 8 octets. |
| [RFC4861-OPT-40](#rfc4861-opt-40) | The sender sets the Reserved fields of a Redirected Header option to zero. |
| [RFC4861-OPT-41](#rfc4861-opt-41) | A receiver ignores the value of the Reserved fields of a Redirected Header option. |
| [RFC4861-OPT-42](#rfc4861-opt-42) | The Redirected Header option holds the redirected packet, cut so that the Redirect is not larger than the minimum IPv6 MTU. |
| [RFC4861-OPT-43](#rfc4861-opt-43) | A node silently ignores a Redirected Header option in a Neighbor Discovery message other than a Redirect. |
| [RFC4861-OPT-44](#rfc4861-opt-44) | An MTU option holds an 8-bit Type, an 8-bit Length, a 16-bit Reserved field, and a 32-bit MTU. |
| [RFC4861-OPT-45](#rfc4861-opt-45) | The Type of the MTU option is 5. |
| [RFC4861-OPT-46](#rfc4861-opt-46) | The Length of the MTU option is 1. |
| [RFC4861-OPT-47](#rfc4861-opt-47) | The sender sets the Reserved field of an MTU option to zero. |
| [RFC4861-OPT-48](#rfc4861-opt-48) | A receiver ignores the value of the Reserved field of an MTU option. |
| [RFC4861-OPT-49](#rfc4861-opt-49) | The MTU field holds the recommended MTU of the link, so that all nodes on a link with an uncertain MTU use the same value. |
| [RFC4861-OPT-50](#rfc4861-opt-50) | A node silently ignores an MTU option in a Neighbor Discovery message other than a Router Advertisement. |
| [RFC4861-RVAL-1](#rfc4861-rval-1) | A host silently discards every Router Solicitation that it receives. |
| [RFC4861-RVAL-2](#rfc4861-rval-2) | A router silently discards a Router Solicitation whose IP Hop Limit is not 255. |
| [RFC4861-RVAL-3](#rfc4861-rval-3) | A router silently discards a Router Solicitation whose ICMP checksum is not valid. |
| [RFC4861-RVAL-4](#rfc4861-rval-4) | A router silently discards a Router Solicitation whose ICMP Code is not 0. |
| [RFC4861-RVAL-5](#rfc4861-rval-5) | A router silently discards a Router Solicitation whose ICMP length is less than 8 octets. |
| [RFC4861-RVAL-6](#rfc4861-rval-6) | A router silently discards a Router Solicitation that holds an option of length zero. |
| [RFC4861-RVAL-7](#rfc4861-rval-7) | A router silently discards a Router Solicitation from the unspecified address that holds a Source Link-Layer Address option. |
| [RFC4861-RVAL-8](#rfc4861-rval-8) | A router ignores the Reserved field and any unrecognized option of a Router Solicitation. |
| [RFC4861-RVAL-9](#rfc4861-rval-9) | A router ignores a defined option that does not belong in a Router Solicitation, and processes the message as normal. |
| [RFC4861-RVAL-10](#rfc4861-rval-10) | A node silently discards a Router Advertisement whose IP source address is not a link-local address. |
| [RFC4861-RVAL-11](#rfc4861-rval-11) | A router uses its link-local address as the source address of its Router Advertisements and Redirects. |
| [RFC4861-RVAL-12](#rfc4861-rval-12) | A node silently discards a Router Advertisement whose IP Hop Limit is not 255. |
| [RFC4861-RVAL-13](#rfc4861-rval-13) | A node silently discards a Router Advertisement whose ICMP checksum is not valid. |
| [RFC4861-RVAL-14](#rfc4861-rval-14) | A node silently discards a Router Advertisement whose ICMP Code is not 0. |
| [RFC4861-RVAL-15](#rfc4861-rval-15) | A node silently discards a Router Advertisement whose ICMP length is less than 16 octets. |
| [RFC4861-RVAL-16](#rfc4861-rval-16) | A node silently discards a Router Advertisement that holds an option of length zero. |
| [RFC4861-RVAL-17](#rfc4861-rval-17) | A node ignores the Reserved field and any unrecognized option of a Router Advertisement. |
| [RFC4861-RVAL-18](#rfc4861-rval-18) | A node ignores a defined option that does not belong in a Router Advertisement, and processes the message as normal. |
| [RFC4861-RCFG-1](#rfc4861-rcfg-1) | A router lets system management configure the router variables of each interface. |
| [RFC4861-RCFG-2](#rfc4861-rcfg-2) | A document for a link layer can override some default values, and the limits of the Router Lifetime. |
| [RFC4861-RCFG-3](#rfc4861-rcfg-3) | IsRouter tells whether an interface forwards packets, and it is FALSE by default. |
| [RFC4861-RCFG-4](#rfc4861-rcfg-4) | AdvSendAdvertisements controls whether a router sends Router Advertisements and answers Router Solicitations, and it is FALSE by default. |
| [RFC4861-RCFG-5](#rfc4861-rcfg-5) | MaxRtrAdvInterval is the longest time between two unsolicited multicast Router Advertisements; it is 4 to 1800 seconds, and 600 seconds by default. |
| [RFC4861-RCFG-6](#rfc4861-rcfg-6) | MinRtrAdvInterval is the shortest time between two unsolicited multicast Router Advertisements; it is 3 seconds to 0.75 × MaxRtrAdvInterval, with a default that depends on MaxRtrAdvInterval. |
| [RFC4861-RCFG-7](#rfc4861-rcfg-7) | AdvManagedFlag is the value of the M flag of the Router Advertisement, and it is FALSE by default. |
| [RFC4861-RCFG-8](#rfc4861-rcfg-8) | AdvOtherConfigFlag is the value of the O flag of the Router Advertisement, and it is FALSE by default. |
| [RFC4861-RCFG-9](#rfc4861-rcfg-9) | AdvLinkMTU is the value of the MTU option; zero, the default, means that no MTU option is sent. |
| [RFC4861-RCFG-10](#rfc4861-rcfg-10) | AdvReachableTime is the value of the Reachable Time field; it is at most 3,600,000 milliseconds, and zero, the default, means unspecified. |
| [RFC4861-RCFG-11](#rfc4861-rcfg-11) | AdvRetransTimer is the value of the Retrans Timer field; zero, the default, means unspecified. |
| [RFC4861-RCFG-12](#rfc4861-rcfg-12) | AdvCurHopLimit is the value of the Cur Hop Limit field; its default is the value of the Assigned Numbers registry, and zero means unspecified. |
| [RFC4861-RCFG-13](#rfc4861-rcfg-13) | AdvDefaultLifetime is the value of the Router Lifetime field; it is zero or from MaxRtrAdvInterval to 9000 seconds, and 3 × MaxRtrAdvInterval by default. |
| [RFC4861-RCFG-14](#rfc4861-rcfg-14) | AdvPrefixList holds the prefixes of the Prefix Information options; by default it holds every prefix that is on-link for the interface. |
| [RFC4861-RCFG-15](#rfc4861-rcfg-15) | A router does not advertise the link-local prefix. |
| [RFC4861-RCFG-16](#rfc4861-rcfg-16) | AdvValidLifetime is the value of the Valid Lifetime field; all ones means infinity, and the default is a fixed 2592000 seconds. |
| [RFC4861-RCFG-17](#rfc4861-rcfg-17) | An implementation may let AdvValidLifetime decrement in real time, or keep it fixed. |
| [RFC4861-RCFG-18](#rfc4861-rcfg-18) | AdvOnLinkFlag is the value of the L flag of the Prefix Information option, and it is TRUE by default. |
| [RFC4861-RCFG-19](#rfc4861-rcfg-19) | AdvPreferredLifetime is the value of the Preferred Lifetime field; all ones means infinity, and the default is a fixed 604800 seconds. |
| [RFC4861-RCFG-20](#rfc4861-rcfg-20) | An implementation may let AdvPreferredLifetime decrement in real time, or keep it fixed. |
| [RFC4861-RCFG-21](#rfc4861-rcfg-21) | The preferred lifetime of a prefix is not larger than its valid lifetime. |
| [RFC4861-RCFG-22](#rfc4861-rcfg-22) | AdvAutonomousFlag is the value of the A flag of the Prefix Information option, and it is TRUE by default. |
| [RFC4861-RCFG-23](#rfc4861-rcfg-23) | A router behaves like a host for CurHopLimit, RetransTimer and ReachableTime, with the random ReachableTime included. |
| [RFC4861-ADV-1](#rfc4861-adv-1) | A router sends Router Advertisements only on an advertising interface: one that works, is enabled, has a unicast address, and has AdvSendAdvertisements TRUE. |
| [RFC4861-ADV-2](#rfc4861-adv-2) | An interface also becomes an advertising interface after startup: when its flag becomes TRUE, when it is enabled, or when the node starts to forward. |
| [RFC4861-ADV-3](#rfc4861-adv-3) | A router joins the all-routers multicast address on each advertising interface. |
| [RFC4861-ADV-4](#rfc4861-adv-4) | A router sends periodic and solicited Router Advertisements on its advertising interfaces. |
| [RFC4861-ADV-5](#rfc4861-adv-5) | The Router Lifetime field of a Router Advertisement carries AdvDefaultLifetime. |
| [RFC4861-ADV-6](#rfc4861-adv-6) | The M and O flags of a Router Advertisement carry AdvManagedFlag and AdvOtherConfigFlag. |
| [RFC4861-ADV-7](#rfc4861-adv-7) | The Cur Hop Limit field of a Router Advertisement carries the configured hop limit of the interface. |
| [RFC4861-ADV-8](#rfc4861-adv-8) | The Reachable Time field of a Router Advertisement carries AdvReachableTime. |
| [RFC4861-ADV-9](#rfc4861-adv-9) | The Retrans Timer field of a Router Advertisement carries AdvRetransTimer. |
| [RFC4861-ADV-10](#rfc4861-adv-10) | The Source Link-Layer Address option of a Router Advertisement carries the link-layer address of the interface, and a router may leave the option out. |
| [RFC4861-ADV-11](#rfc4861-adv-11) | A Router Advertisement carries an MTU option with AdvLinkMTU when AdvLinkMTU is not zero, and no MTU option when it is zero. |
| [RFC4861-ADV-12](#rfc4861-adv-12) | A Router Advertisement carries one Prefix Information option for each prefix of AdvPrefixList. |
| [RFC4861-ADV-13](#rfc4861-adv-13) | The L flag of a Prefix Information option carries AdvOnLinkFlag of the prefix. |
| [RFC4861-ADV-14](#rfc4861-adv-14) | The Valid Lifetime field of a Prefix Information option carries AdvValidLifetime of the prefix. |
| [RFC4861-ADV-15](#rfc4861-adv-15) | The A flag of a Prefix Information option carries AdvAutonomousFlag of the prefix. |
| [RFC4861-ADV-16](#rfc4861-adv-16) | The Preferred Lifetime field of a Prefix Information option carries AdvPreferredLifetime of the prefix. |
| [RFC4861-ADV-17](#rfc4861-adv-17) | A router that advertises but does not want to be a default router sends Router Lifetime zero. |
| [RFC4861-ADV-18](#rfc4861-adv-18) | A router may leave out some or all options in unsolicited Router Advertisements. |
| [RFC4861-ADV-19](#rfc4861-adv-19) | A router includes all options in a solicited Router Advertisement and in the first few unsolicited ones. |
| [RFC4861-ADV-20](#rfc4861-adv-20) | When all options do not fit into the link MTU, a router can split them over several Router Advertisements. |
| [RFC4861-ADV-21](#rfc4861-adv-21) | A host never sends a Router Advertisement. |
| [RFC4861-ADV-22](#rfc4861-adv-22) | Each advertising interface has its own timer: after each multicast Router Advertisement, the next one comes after a uniform random time between MinRtrAdvInterval and MaxRtrAdvInterval. |
| [RFC4861-ADV-23](#rfc4861-adv-23) | For the first few Router Advertisements of a new advertising interface, the interval is at most MAX_INITIAL_RTR_ADVERT_INTERVAL. |
| [RFC4861-ADV-24](#rfc4861-adv-24) | When system management changes the advertised information, a router may send up to MAX_INITIAL_RTR_ADVERTISEMENTS advertisements at the initial rate. |
| [RFC4861-ADV-25](#rfc4861-adv-25) | When an interface stops to be an advertising interface, the router sends one to MAX_FINAL_RTR_ADVERTISEMENTS final multicast Router Advertisements with Router Lifetime zero. |
| [RFC4861-ADV-26](#rfc4861-adv-26) | A router that becomes a host leaves the all-routers multicast group on all its multicast interfaces. |
| [RFC4861-ADV-27](#rfc4861-adv-27) | A router that becomes a host sends its later Neighbor Advertisements with the Router flag zero. |
| [RFC4861-ADV-28](#rfc4861-adv-28) | When forwarding is off but the interface still advertises, the Router Advertisements carry Router Lifetime zero. |
| [RFC4861-ADV-29](#rfc4861-adv-29) | A router sends a Router Advertisement in reply to a valid Router Solicitation that arrives on an advertising interface. |
| [RFC4861-ADV-30](#rfc4861-adv-30) | A router may unicast its reply to the address of the host, but usually it multicasts the reply to all nodes. |
| [RFC4861-ADV-31](#rfc4861-adv-31) | A multicast reply restarts the interval timer with a new random value, as an unsolicited advertisement does. |
| [RFC4861-ADV-32](#rfc4861-adv-32) | A router delays each solicited Router Advertisement by a random time from 0 to MAX_RA_DELAY_TIME, counted from the first solicitation. |
| [RFC4861-ADV-33](#rfc4861-adv-33) | A router sends at most one Router Advertisement to the all-nodes address every MIN_DELAY_BETWEEN_RAS seconds. |
| [RFC4861-ADV-34](#rfc4861-adv-34) | When the random reply delay ends after the next scheduled multicast advertisement, the router sends the reply at the scheduled time. |
| [RFC4861-ADV-35](#rfc4861-adv-35) | When the router sent a multicast advertisement within MIN_DELAY_BETWEEN_RAS, it schedules the reply at MIN_DELAY_BETWEEN_RAS plus the random delay after that advertisement; else at the random delay. |
| [RFC4861-ADV-36](#rfc4861-adv-36) | Only replies to solicitations may come more often than MinRtrAdvInterval; unsolicited multicast advertisements never do. |
| [RFC4861-ADV-37](#rfc4861-adv-37) | A Router Solicitation from the unspecified address does not change the Neighbor Cache of the router. |
| [RFC4861-ADV-38](#rfc4861-adv-38) | A Router Solicitation with a new link-layer address updates the Neighbor Cache entry of the sender, and the entry becomes STALE. |
| [RFC4861-ADV-39](#rfc4861-adv-39) | A Router Solicitation from a sender with no Neighbor Cache entry creates a STALE entry with the link-layer address. |
| [RFC4861-ADV-40](#rfc4861-adv-40) | When the router has no entry and the solicitation has no Source Link-Layer Address option, the reply can be multicast or unicast. |
| [RFC4861-ADV-41](#rfc4861-adv-41) | A Router Solicitation sets the IsRouter flag of the Neighbor Cache entry of its sender to FALSE. |
| [RFC4861-ADV-42](#rfc4861-adv-42) | A router checks the Router Advertisements of other routers for consistency, and it logs an inconsistency. |
| [RFC4861-ADV-43](#rfc4861-adv-43) | The consistency check covers at least Cur Hop Limit, the M and O flags, Reachable Time, Retrans Timer, the MTU option and the prefix lifetimes; zero values are exempt. |
| [RFC4861-ADV-44](#rfc4861-adv-44) | For lifetimes that decrement in real time, the check compares the times of deprecation and invalidation, and it allows some clock skew. |
| [RFC4861-ADV-45](#rfc4861-adv-45) | A router does not log different prefix sets or zero values; it logs only conflicts that make hosts switch values with each advertisement. |
| [RFC4861-ADV-46](#rfc4861-adv-46) | The source address of the Router Advertisements of a router equals the Target Address of a Redirect to that router. |
| [RFC4861-ADV-47](#rfc4861-adv-47) | A router that changes a link-local address tells the hosts: a few advertisements from the old address with Router Lifetime zero, and a few from the new address. |
| [RFC4861-HOST-1](#rfc4861-host-1) | Router Advertisements override the defaults of the host variables; a host uses the defaults when no router is present or all routers leave a value unspecified. |
| [RFC4861-HOST-2](#rfc4861-host-2) | A document for a link layer can override the default values of the host variables. |
| [RFC4861-HOST-3](#rfc4861-host-3) | LinkMTU is the MTU of the link; its default comes from the document for the link layer. |
| [RFC4861-HOST-4](#rfc4861-host-4) | CurHopLimit is the default hop limit of the packets of a host; its default is the value of the Assigned Numbers registry. |
| [RFC4861-HOST-5](#rfc4861-host-5) | BaseReachableTime is the base of the random ReachableTime; its default is REACHABLE_TIME milliseconds. |
| [RFC4861-HOST-6](#rfc4861-host-6) | ReachableTime is a uniform random value between MIN_RANDOM_FACTOR and MAX_RANDOM_FACTOR times BaseReachableTime, recomputed when the base changes and at least every few hours. |
| [RFC4861-HOST-7](#rfc4861-host-7) | RetransTimer is the time between retransmitted Neighbor Solicitations; its default is RETRANS_TIMER milliseconds. |
| [RFC4861-HOST-8](#rfc4861-host-8) | A host joins the all-nodes multicast address on every multicast-capable interface. |
| [RFC4861-HOST-9](#rfc4861-host-9) | A host keeps the union of what it learns; a new Router Advertisement does not remove all earlier information. |
| [RFC4861-HOST-10](#rfc4861-host-10) | For a parameter with one value, the most recent received value wins. |
| [RFC4861-HOST-11](#rfc4861-host-11) | A host ignores an unspecified field and keeps its current value; it does not return to the default. |
| [RFC4861-HOST-12](#rfc4861-host-12) | An advertisement from a new source with a non-zero Router Lifetime adds a Default Router List entry with that lifetime. |
| [RFC4861-HOST-13](#rfc4861-host-13) | An advertisement from a router that is in the list restarts the invalidation timer with the new Router Lifetime. |
| [RFC4861-HOST-14](#rfc4861-host-14) | An advertisement with Router Lifetime zero from a router in the list removes the router at once. |
| [RFC4861-HOST-15](#rfc4861-host-15) | A host may limit the size of its Default Router List, but it keeps at least two routers, and more if it can. |
| [RFC4861-HOST-16](#rfc4861-host-16) | A non-zero Cur Hop Limit in a Router Advertisement sets the CurHopLimit of the host. |
| [RFC4861-HOST-17](#rfc4861-host-17) | A non-zero Reachable Time sets BaseReachableTime, and a changed value gives a new random ReachableTime. |
| [RFC4861-HOST-18](#rfc4861-host-18) | A host recomputes the random ReachableTime at least once every few hours, also when the base does not change. |
| [RFC4861-HOST-19](#rfc4861-host-19) | A non-zero Retrans Timer in a Router Advertisement sets the RetransTimer of the host. |
| [RFC4861-HOST-20](#rfc4861-host-20) | A Source Link-Layer Address option in a Router Advertisement goes into the Neighbor Cache entry of the router, and the entry gets IsRouter TRUE. |
| [RFC4861-HOST-21](#rfc4861-host-21) | A Router Advertisement without a Source Link-Layer Address option sets IsRouter TRUE in an entry that exists. |
| [RFC4861-HOST-22](#rfc4861-host-22) | A Neighbor Cache entry that a Router Advertisement creates is STALE. |
| [RFC4861-HOST-23](#rfc4861-host-23) | A Neighbor Cache entry that gets a different link-layer address from a Router Advertisement becomes STALE. |
| [RFC4861-HOST-24](#rfc4861-host-24) | A host takes the MTU option value as LinkMTU when it is between the minimum IPv6 link MTU and the maximum of the link type. |
| [RFC4861-HOST-25](#rfc4861-host-25) | A Prefix Information option with the L flag set marks its prefix as on-link. |
| [RFC4861-HOST-26](#rfc4861-host-26) | A Prefix Information option with the L flag zero says nothing about on-link, and it does not make the prefix off-link. |
| [RFC4861-HOST-27](#rfc4861-host-27) | Only a Prefix Information option with the L flag set and lifetime zero cancels an on-link indication. |
| [RFC4861-HOST-28](#rfc4861-host-28) | A packet to an address of unknown on-link status goes to a default router, and an L flag of zero does not change this. |
| [RFC4861-HOST-29](#rfc4861-host-29) | A host silently ignores a Prefix Information option for the link-local prefix. |
| [RFC4861-HOST-30](#rfc4861-host-30) | A new prefix with a non-zero Valid Lifetime goes into the Prefix List with that lifetime. |
| [RFC4861-HOST-31](#rfc4861-host-31) | A prefix that is in the Prefix List gets its invalidation timer reset to the new Valid Lifetime. |
| [RFC4861-HOST-32](#rfc4861-host-32) | A Valid Lifetime of zero for a prefix in the Prefix List removes the prefix at once. |
| [RFC4861-HOST-33](#rfc4861-host-33) | A host silently ignores a Prefix Information option with Valid Lifetime zero for a prefix that it does not know. |
| [RFC4861-HOST-34](#rfc4861-host-34) | For on-link determination, a host accepts any Valid Lifetime; Neighbor Discovery has no minimum check on prefix lifetimes. |
| [RFC4861-HOST-35](#rfc4861-host-35) | A prefix length that address autoconfiguration rejects is still valid for on-link determination. |
| [RFC4861-HOST-36](#rfc4861-host-36) | When the invalidation timer of a prefix expires, the host discards the prefix; Destination Cache entries need no update. |
| [RFC4861-HOST-37](#rfc4861-host-37) | When the lifetime of a default router expires, the host discards the router. |
| [RFC4861-HOST-38](#rfc4861-host-38) | When a router leaves the Default Router List, destinations that use it do next-hop determination again. |
| [RFC4861-HOST-39](#rfc4861-host-39) | A host selects a default router when a new off-link destination has no Destination Cache entry, or when its router seems to fail; later traffic keeps that router. |
| [RFC4861-HOST-40](#rfc4861-host-40) | A host prefers routers that are reachable or probably reachable over routers in INCOMPLETE state or without an entry. |
| [RFC4861-HOST-41](#rfc4861-host-41) | When no router is known to be reachable, a host selects routers in round-robin order. |
| [RFC4861-HOST-42](#rfc4861-host-42) | A host sends up to MAX_RTR_SOLICITATIONS Router Solicitations, at least RTR_SOLICITATION_INTERVAL apart. |
| [RFC4861-HOST-43](#rfc4861-host-43) | A host can send Router Solicitations after startup, after reinitialization of an interface, after it stops to be a router, and after it attaches or re-attaches to a link. |
| [RFC4861-HOST-44](#rfc4861-host-44) | A host sends Router Solicitations to the all-routers multicast address. |
| [RFC4861-HOST-45](#rfc4861-host-45) | The IP source address of a Router Solicitation is a unicast address of the interface or the unspecified address. |
| [RFC4861-HOST-46](#rfc4861-host-46) | A Router Solicitation from a unicast address carries a Source Link-Layer Address option with the link-layer address of the host. |
| [RFC4861-HOST-47](#rfc4861-host-47) | Before its first Router Solicitation, a host waits a random time from 0 to MAX_RTR_SOLICITATION_DELAY. |
| [RFC4861-HOST-48](#rfc4861-host-48) | A host that already had a random delay since the interface became enabled does not need to delay its first Router Solicitation again. |
| [RFC4861-HOST-49](#rfc4861-host-49) | A host may skip the random delay when necessary; a mobile node with a link-layer hint of movement may send a Router Solicitation at once. |
| [RFC4861-HOST-50](#rfc4861-host-50) | After a valid Router Advertisement with a non-zero Router Lifetime, a host stops its Router Solicitations on that interface until the next event. |
| [RFC4861-HOST-51](#rfc4861-host-51) | A host sends at least one Router Solicitation, also when an advertisement arrives before its first solicitation. |
| [RFC4861-HOST-52](#rfc4861-host-52) | After MAX_RTR_SOLICITATIONS solicitations without an answer, a host concludes that the link has no router, but it still processes later Router Advertisements. |
| [RFC4861-AR-1](#rfc4861-ar-1) | A node does address resolution only for an on-link address whose link-layer address it does not know. |
| [RFC4861-AR-2](#rfc4861-ar-2) | A node never does address resolution for a multicast address. |
| [RFC4861-AR-3](#rfc4861-ar-3) | A solicitation, a Router Advertisement or a Redirect without a link-layer address option does not create or update a Neighbor Cache entry, except for the IsRouter flag. |
| [RFC4861-AR-4](#rfc4861-ar-4) | When a multicast-capable interface becomes enabled, the node joins the all-nodes address and the solicited-node address of each address of the interface. |
| [RFC4861-AR-5](#rfc4861-ar-5) | When the set of addresses changes, the node joins the solicited-node address of each new address and leaves that of each removed address. |
| [RFC4861-AR-6](#rfc4861-ar-6) | A node joins a solicited-node multicast address with Multicast Listener Discovery (MLD or MLDv2). |
| [RFC4861-AR-7](#rfc4861-ar-7) | A node leaves a solicited-node group only after it removes all its addresses that map to that group. |
| [RFC4861-AR-8](#rfc4861-ar-8) | A node with a unicast packet for a neighbor with an unknown link-layer address creates an INCOMPLETE Neighbor Cache entry and sends an NS for the neighbor. |
| [RFC4861-AR-9](#rfc4861-ar-9) | The NS for address resolution goes to the solicited-node multicast address of the target. |
| [RFC4861-AR-10](#rfc4861-ar-10) | If the source of the packet that causes the NS is an address of the interface, the NS uses that address as its source. |
| [RFC4861-AR-11](#rfc4861-ar-11) | Otherwise, the NS uses any address of the interface as its source. |
| [RFC4861-AR-12](#rfc4861-ar-12) | An NS to a solicited-node multicast address includes the Source Link-Layer Address option, if the sender has a link-layer address. |
| [RFC4861-AR-13](#rfc4861-ar-13) | Another NS includes the Source Link-Layer Address option too, but a unicast NS can omit it. |
| [RFC4861-AR-14](#rfc4861-ar-14) | During address resolution, the sender keeps a small queue of packets for each neighbor. |
| [RFC4861-AR-15](#rfc4861-ar-15) | The queue holds at least one packet, and it can hold more. |
| [RFC4861-AR-16](#rfc4861-ar-16) | The number of queued packets for each neighbor is limited to a small value. |
| [RFC4861-AR-17](#rfc4861-ar-17) | When the queue overflows, the new packet replaces the oldest packet. |
| [RFC4861-AR-18](#rfc4861-ar-18) | When address resolution completes, the node sends the queued packets. |
| [RFC4861-AR-19](#rfc4861-ar-19) | While it waits for an answer, the sender retransmits the NS about every RetransTimer milliseconds, also without new traffic. |
| [RFC4861-AR-20](#rfc4861-ar-20) | The sender sends at most one NS for each neighbor every RetransTimer milliseconds. |
| [RFC4861-AR-21](#rfc4861-ar-21) | Address resolution fails when MAX_MULTICAST_SOLICIT solicitations get no Neighbor Advertisement. |
| [RFC4861-AR-22](#rfc4861-ar-22) | When address resolution fails, the sender returns an ICMP Destination Unreachable, code 3, for each queued packet. |
| [RFC4861-AR-23](#rfc4861-ar-23) | A node silently discards a valid NS whose Target Address is not its own valid or tentative address and not an address that it is a proxy for. |
| [RFC4861-AR-24](#rfc4861-ar-24) | When an NS from a specified source carries a Source Link-Layer Address option, the recipient creates or updates the entry for the source. |
| [RFC4861-AR-25](#rfc4861-ar-25) | If no entry exists, the recipient creates one in the STALE state. |
| [RFC4861-AR-26](#rfc4861-ar-26) | If the entry exists with another link-layer address, the recipient replaces the address and sets the entry to STALE. |
| [RFC4861-AR-27](#rfc4861-ar-27) | A Neighbor Cache entry that an NS creates has the IsRouter flag FALSE. |
| [RFC4861-AR-28](#rfc4861-ar-28) | An NS does not change the IsRouter flag of an entry that already exists. |
| [RFC4861-AR-29](#rfc4861-ar-29) | An NS from the unspecified address does not create or update a Neighbor Cache entry. |
| [RFC4861-AR-30](#rfc4861-ar-30) | After it updates the Neighbor Cache, the node sends a Neighbor Advertisement in response. |
| [RFC4861-AR-31](#rfc4861-ar-31) | A node sends a Neighbor Advertisement in response to a valid NS for one of its assigned addresses. |
| [RFC4861-AR-32](#rfc4861-ar-32) | The Target Address of the NA is a copy of the Target Address of the NS. |
| [RFC4861-AR-33](#rfc4861-ar-33) | An NA that answers a unicast NS can omit the Target Link-Layer Address option. |
| [RFC4861-AR-34](#rfc4861-ar-34) | An NA that answers a multicast NS includes the Target Link-Layer Address option. |
| [RFC4861-AR-35](#rfc4861-ar-35) | A router sets the Router flag of a solicited NA to one, and a host sets it to zero. |
| [RFC4861-AR-36](#rfc4861-ar-36) | The Override flag is zero when the target is an anycast or proxied address, or when the NA has no Target Link-Layer Address option. |
| [RFC4861-AR-37](#rfc4861-ar-37) | In all other NAs that answer an NS, the Override flag is one. |
| [RFC4861-AR-38](#rfc4861-ar-38) | An NA that answers an NS from the unspecified address has the Solicited flag zero and goes to the all-nodes address. |
| [RFC4861-AR-39](#rfc4861-ar-39) | Any other solicited NA has the Solicited flag one and goes by unicast to the Source Address of the NS. |
| [RFC4861-AR-40](#rfc4861-ar-40) | A node delays the NA for an anycast target by a random time between 0 and MAX_ANYCAST_DELAY_TIME seconds. |
| [RFC4861-AR-41](#rfc4861-ar-41) | A node that answers an NS but has no link-layer address for the source first resolves that address with a multicast NS. |
| [RFC4861-AR-42](#rfc4861-ar-42) | A node silently discards a valid NA for a target that has no Neighbor Cache entry. |
| [RFC4861-AR-43](#rfc4861-ar-43) | For an INCOMPLETE entry, a node silently discards an NA without a Target Link-Layer Address option on a link that has addresses. |
| [RFC4861-AR-44](#rfc4861-ar-44) | For an INCOMPLETE entry, a node records the link-layer address from the NA and sends the queued packets. |
| [RFC4861-AR-45](#rfc4861-ar-45) | For an INCOMPLETE entry, the entry becomes REACHABLE if the NA has the Solicited flag set, and STALE if not. |
| [RFC4861-AR-46](#rfc4861-ar-46) | For an INCOMPLETE entry, the IsRouter flag takes the value of the Router flag of the NA. |
| [RFC4861-AR-47](#rfc4861-ar-47) | A node ignores the Override flag of an NA for an INCOMPLETE entry. |
| [RFC4861-AR-48](#rfc4861-ar-48) | If the Override flag is clear and the link-layer address is different, a REACHABLE entry becomes STALE and does not change in any other way. |
| [RFC4861-AR-49](#rfc4861-ar-49) | If the Override flag is clear and the link-layer address is different, an entry that is not REACHABLE does not change. |
| [RFC4861-AR-50](#rfc4861-ar-50) | If the Override flag is set, or the address is the same, or no Target Link-Layer Address option is present, the NA updates the entry, and a different supplied address goes into the cache. |
| [RFC4861-AR-51](#rfc4861-ar-51) | In that update, an NA with the Solicited flag set makes the entry REACHABLE. |
| [RFC4861-AR-52](#rfc4861-ar-52) | In that update, an NA with the Solicited flag zero that changes the address makes the entry STALE; otherwise the state does not change. |
| [RFC4861-AR-53](#rfc4861-ar-53) | A node sets the Solicited flag only in an NA that answers an NS. |
| [RFC4861-AR-54](#rfc4861-ar-54) | When an unsolicited NA changes the address that a node uses, the node verifies the new path when it sends the next packet. |
| [RFC4861-AR-55](#rfc4861-ar-55) | In that update, the IsRouter flag of the entry takes the value of the Router flag of the NA. |
| [RFC4861-AR-56](#rfc4861-ar-56) | When IsRouter changes from TRUE to FALSE, the node removes that router from the Default Router List and updates the Destination Cache. |
| [RFC4861-AR-57](#rfc4861-ar-57) | A node with a new link-layer address can send up to MAX_NEIGHBOR_ADVERTISEMENT unsolicited NAs to the all-nodes address. |
| [RFC4861-AR-58](#rfc4861-ar-58) | The interval between these unsolicited NAs is at least RetransTimer. |
| [RFC4861-AR-59](#rfc4861-ar-59) | The unsolicited NA has an address of the interface as Target Address and the new link-layer address in the Target Link-Layer Address option. |
| [RFC4861-AR-60](#rfc4861-ar-60) | The unsolicited NA has the Solicited flag zero. |
| [RFC4861-AR-61](#rfc4861-ar-61) | A router sets the Router flag of an unsolicited NA to one, and a host sets it to zero. |
| [RFC4861-AR-62](#rfc4861-ar-62) | The Override flag of the unsolicited NA can be zero or one. |
| [RFC4861-AR-63](#rfc4861-ar-63) | A neighbor that receives an unsolicited NA with the Override flag set makes its entry STALE and installs the new link-layer address. |
| [RFC4861-AR-64](#rfc4861-ar-64) | A neighbor that receives an unsolicited NA with the Override flag clear makes its entry STALE, keeps the old address and probes it. |
| [RFC4861-AR-65](#rfc4861-ar-65) | A node with more than one address on an interface can multicast a separate NA for each address. |
| [RFC4861-AR-66](#rfc4861-ar-66) | The node puts a small delay between these NAs. |
| [RFC4861-AR-67](#rfc4861-ar-67) | A proxy can multicast NAs when its link-layer address changes or when it starts to act as proxy for an address. |
| [RFC4861-AR-68](#rfc4861-ar-68) | A node that belongs to an anycast address can multicast unsolicited NAs for that address when its link-layer address changes. |
| [RFC4861-ANY-1](#rfc4861-any-1) | Address resolution and Neighbor Unreachability Detection treat an anycast address like a unicast address. |
| [RFC4861-ANY-2](#rfc4861-any-2) | A node with an anycast address delays an NA that answers an NS for it by a random time between 0 and MAX_ANYCAST_DELAY_TIME. |
| [RFC4861-ANY-3](#rfc4861-any-3) | A node sets the Override flag to 0 in an NA for its anycast address. |
| [RFC4861-ANY-4](#rfc4861-any-4) | Neighbor Unreachability Detection finds quickly when the binding for an anycast address is no longer valid. |
| [RFC4861-ANY-5](#rfc4861-any-5) | A router can act as proxy for other nodes, and it announces this with Neighbor Advertisements. |
| [RFC4861-ANY-6](#rfc4861-any-6) | A proxy joins the solicited-node multicast address of each address that it is a proxy for. |
| [RFC4861-ANY-7](#rfc4861-any-7) | The proxy joins these groups with a multicast listener discovery protocol, such as MLD or MLDv2. |
| [RFC4861-ANY-8](#rfc4861-any-8) | Every solicited proxy NA has the Override flag set to zero. |
| [RFC4861-ANY-9](#rfc4861-any-9) | A proxy can send unsolicited NAs with the Override flag set to one. |
| [RFC4861-ANY-10](#rfc4861-any-10) | A proxy delays its NA that answers an NS by a random time between 0 and MAX_ANYCAST_DELAY_TIME seconds, unless it is the only proxy. |
| [RFC4861-RDVAL-1](#rfc4861-rdval-1) | A host silently discards a Redirect whose IP Source Address is not a link-local address. |
| [RFC4861-RDVAL-2](#rfc4861-rdval-2) | A router uses its link-local address as the source of its Router Advertisements and Redirects. |
| [RFC4861-RDVAL-3](#rfc4861-rdval-3) | A host silently discards a Redirect whose IP Hop Limit is not 255. |
| [RFC4861-RDVAL-4](#rfc4861-rdval-4) | A host silently discards a Redirect with an ICMP Checksum that is not valid. |
| [RFC4861-RDVAL-5](#rfc4861-rdval-5) | A host silently discards a Redirect with an ICMP Code other than 0. |
| [RFC4861-RDVAL-6](#rfc4861-rdval-6) | A host silently discards a Redirect with an ICMP length of less than 40 octets. |
| [RFC4861-RDVAL-7](#rfc4861-rdval-7) | A host silently discards a Redirect whose IP Source Address is not the current first-hop router for the destination. |
| [RFC4861-RDVAL-8](#rfc4861-rdval-8) | A host silently discards a Redirect whose ICMP Destination Address is a multicast address. |
| [RFC4861-RDVAL-9](#rfc4861-rdval-9) | A host silently discards a Redirect whose Target Address is neither link-local nor equal to the Destination Address. |
| [RFC4861-RDVAL-10](#rfc4861-rdval-10) | A host silently discards a Redirect that has an option with length zero. |
| [RFC4861-RDVAL-11](#rfc4861-rdval-11) | A host ignores the Reserved field and unknown options of a Redirect. |
| [RFC4861-RDVAL-12](#rfc4861-rdval-12) | A host ignores defined options that do not belong to a Redirect and processes the Redirect as usual. |
| [RFC4861-RDVAL-13](#rfc4861-rdval-13) | The Target Link-Layer Address option and the Redirected Header option are the only defined options of a Redirect. |
| [RFC4861-RDVAL-14](#rfc4861-rdval-14) | A host does not discard a Redirect only because its Target Address is outside the prefixes of the link. |
| [RFC4861-RDR-1](#rfc4861-rdr-1) | A router sends a Redirect to point a host to a better first-hop router, or to tell the host that the destination is on-link; for an on-link destination, the Target Address equals the Destination Address. |
| [RFC4861-RDR-2](#rfc4861-rdr-2) | A router knows the link-local address of each neighbor router, so that the Target Address of a Redirect is a link-local address. |
| [RFC4861-RDR-3](#rfc4861-rdr-3) | A router sends a Redirect, subject to a rate limit, when it forwards a packet from a neighbor to a unicast destination that has a better first hop on the same link. |
| [RFC4861-RDR-4](#rfc4861-rdr-4) | When the target is a router, the Target Address of the Redirect is the link-local address of that router. |
| [RFC4861-RDR-5](#rfc4861-rdr-5) | When the target is a host, the Target Address equals the Destination Address. |
| [RFC4861-RDR-6](#rfc4861-rdr-6) | The Destination Address field holds the destination address of the packet that caused the Redirect. |
| [RFC4861-RDR-7](#rfc4861-rdr-7) | The Redirect carries a Target Link-Layer Address option with the link-layer address of the target, if the router knows it. |
| [RFC4861-RDR-8](#rfc4861-rdr-8) | The Redirected Header option holds as much of the forwarded packet as fits, and the Redirect is not larger than the IPv6 minimum MTU. |
| [RFC4861-RDR-9](#rfc4861-rdr-9) | A router limits the rate at which it sends Redirect messages. |
| [RFC4861-RDR-10](#rfc4861-rdr-10) | A router does not update its routing table when it receives a Redirect. |
| [RFC4861-RDH-1](#rfc4861-rdh-1) | A host that receives a valid Redirect updates its Destination Cache, so that later traffic goes to the target. |
| [RFC4861-RDH-2](#rfc4861-rdh-2) | If no Destination Cache entry exists for the destination, the host creates one. |
| [RFC4861-RDH-3](#rfc4861-rdh-3) | If the Redirect carries a Target Link-Layer Address option, the host creates or updates the Neighbor Cache entry of the target with that address. |
| [RFC4861-RDH-4](#rfc4861-rdh-4) | A Neighbor Cache entry that a Redirect creates is in the STALE state. |
| [RFC4861-RDH-5](#rfc4861-rdh-5) | A Redirect that changes the link-layer address of an entry sets the entry to STALE; the same address leaves the state unchanged. |
| [RFC4861-RDH-6](#rfc4861-rdh-6) | If the Target Address equals the Destination Address, the host treats the target as on-link. |
| [RFC4861-RDH-7](#rfc4861-rdh-7) | If the Target Address differs from the Destination Address, the host sets IsRouter to TRUE for the target. |
| [RFC4861-RDH-8](#rfc4861-rdh-8) | If the Target Address equals the Destination Address, a new Neighbor Cache entry has IsRouter FALSE, and an entry that exists keeps its flag. |
| [RFC4861-RDH-9](#rfc4861-rdh-9) | A Redirect applies to all flows to the destination, whatever the Flow Label in the Redirected Header option. |
| [RFC4861-RDH-10](#rfc4861-rdh-10) | A host does not send Redirect messages. |

## Router Solicitation message

### RFC4861-RS-1

**A host sends Router Solicitations to make routers send Router Advertisements quickly.**

> "Hosts send Router Solicitations in order to prompt routers to
> generate Router Advertisements quickly." — §4.1, `rfc4861.txt:982-983`

- Strength: description. Class: wire.
- Check idea: a host attaches to a link with a router and sends a Router Solicitation. The
  router answers with a Router Advertisement before its next periodic one is due.

### RFC4861-RS-2

**A Router Solicitation holds Type, Code, Checksum, a 32-bit Reserved field, and then options.**

> "|     Type      |     Code      |          Checksum             |" — §4.1,
> `rfc4861.txt:988`; "|                            Reserved                           |" —
> `rfc4861.txt:990`; "|   Options ..." — `rfc4861.txt:992`

- Strength: description. Class: encoding.
- Check idea: decode a Router Solicitation of a host. The ICMP message has one octet of Type,
  one of Code, two of Checksum, four of Reserved, and then the options.

### RFC4861-RS-3

**The Source Address of a Router Solicitation is an address of the interface that sends it, or :: when that interface has no address.**

> "Source Address
> An IP address assigned to the sending interface, or
> the unspecified address if no address is assigned
> to the sending interface." — §4.1, `rfc4861.txt:997-1000`

- Strength: description. Class: wire.
- Check idea: a host with a link-local address sends a Router Solicitation from that address. A
  host with no address sends it from the unspecified address (::).

### RFC4861-RS-4

**The Destination Address of a Router Solicitation is typically the all-routers multicast address.**

> "Destination Address
> Typically the all-routers multicast address." — §4.1, `rfc4861.txt:1002-1003`

- Strength: description. Class: wire.
- Check idea: capture the Router Solicitations of a host. Their Destination Address is ff02::2,
  the all-routers multicast address.

### RFC4861-RS-5

**A Router Solicitation has IP Hop Limit 255.**

> "Hop Limit      255" — §4.1, `rfc4861.txt:1005`

- Strength: description. Class: wire.
- Check idea: capture the Router Solicitations of a host. Each one has Hop Limit 255 in the IP
  header.

### RFC4861-RS-6

**A Router Solicitation has ICMP Type 133.**

> "Type           133" — §4.1, `rfc4861.txt:1017`

- Strength: description. Class: wire.
- Check idea: capture the Router Solicitations of a host. Each one has ICMP Type 133.

### RFC4861-RS-7

**A Router Solicitation has ICMP Code 0.**

> "Code           0" — §4.1, `rfc4861.txt:1019`

- Strength: description. Class: wire.
- Check idea: capture the Router Solicitations of a host. Each one has ICMP Code 0.

### RFC4861-RS-8

**The Checksum field of a Router Solicitation holds the ICMPv6 checksum.**

> "Checksum       The ICMP checksum.  See [ICMPv6]." — §4.1, `rfc4861.txt:1021`

- Strength: description. Class: wire.
- Check idea: compute the ICMPv6 checksum, with the IPv6 pseudo-header, over a captured Router
  Solicitation. The result is equal to the Checksum field.

### RFC4861-RS-9

**The sender sets the Reserved field of a Router Solicitation to zero.**

> "Reserved       This field is unused.  It MUST be initialized to
> zero by the sender" — §4.1, `rfc4861.txt:1023-1024`

- Strength: must. Class: wire.
- Check idea: capture the Router Solicitations of a host. The Reserved field of each one is
  zero.

### RFC4861-RS-10

**A receiver ignores the value of the Reserved field of a Router Solicitation.**

> "Reserved       This field is unused." — §4.1, `rfc4861.txt:1023`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1024-1025`

- Strength: must. Class: end-to-end.
- Check idea: host A sends a Router Solicitation with a nonzero Reserved field to a router. The
  router answers with a Router Advertisement, as it does for a Reserved field of zero.

### RFC4861-RS-11

**A Router Solicitation from the unspecified address carries no Source Link-Layer Address option.**

> "MUST NOT be included if the Source Address
> is the unspecified address." — §4.1, `rfc4861.txt:1029-1030`

- Strength: must not. Class: wire.
- Check idea: a host with no address sends a Router Solicitation from ::. The message has no
  Source Link-Layer Address option.

### RFC4861-RS-12

**On link layers with addresses, a Router Solicitation from an assigned address carries a Source Link-Layer Address option with the sender's link-layer address.**

> "Source link-layer address The link-layer address of the sender, if
> known." — §4.1, `rfc4861.txt:1028-1029`; "Otherwise, it SHOULD
> be included on link layers that have addresses." — `rfc4861.txt:1030-1031`

- Strength: should. Class: wire.
- Check idea: on an Ethernet link, a host with a link-local address sends a Router
  Solicitation. The message carries a Source Link-Layer Address option with the MAC address of
  the host.

### RFC4861-RS-13

**A receiver of a Router Solicitation silently ignores an option that it does not recognize, and it processes the rest of the message.**

> "Future versions of this protocol may define new option types.
> Receivers MUST silently ignore any options they do not recognize
> and continue processing the message." — §4.1, `rfc4861.txt:1033-1035`

- Strength: must. Class: end-to-end.
- Check idea: host A sends a Router Solicitation with an option of an unknown type before its
  Source Link-Layer Address option. The router answers with a Router Advertisement and sends no
  ICMPv6 error.

## Router Advertisement message

### RFC4861-RA-1

**A router sends Router Advertisements at intervals, and also in answer to Router Solicitations.**

> "Routers send out Router Advertisement messages periodically, or in
> response to Router Solicitations." — §4.2, `rfc4861.txt:1039-1040`

- Strength: description. Class: wire.
- Check idea: a router on a link with no other traffic sends Router Advertisements at
  intervals. After a host sends a Router Solicitation, the router also sends a Router
  Advertisement.

### RFC4861-RA-2

**After the ICMP header, a Router Advertisement holds Cur Hop Limit, M, O, Reserved, Router Lifetime, Reachable Time, Retrans Timer, and then options.**

> "|     Type      |     Code      |          Checksum             |" — §4.2,
> `rfc4861.txt:1045`; "| Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |" —
> `rfc4861.txt:1047`; "|                         Reachable Time                        |" —
> `rfc4861.txt:1049`; "|                          Retrans Timer                        |" —
> `rfc4861.txt:1051`; "|   Options ..." — `rfc4861.txt:1053`

- Strength: description. Class: encoding.
- Check idea: decode a Router Advertisement. After Type, Code and Checksum come Cur Hop Limit (8
  bits), M and O (1 bit each) and Reserved (6 bits). Router Lifetime (16 bits), Reachable Time
  (32 bits), Retrans Timer (32 bits) and the options follow.

### RFC4861-RA-3

**A router sends a Router Advertisement from the link-local address of the interface that sends it.**

> "Source Address
> MUST be the link-local address assigned to the
> interface from which this message is sent." — §4.2, `rfc4861.txt:1058-1060`

- Strength: must. Class: wire.
- Check idea: a router with a global address on each of two interfaces sends Router
  Advertisements on both links. The Source Address on each link is the link-local address of
  that interface.

### RFC4861-RA-4

**The Destination Address of a Router Advertisement is typically the source of the Router Solicitation that caused it, or the all-nodes multicast address.**

> "Destination Address
> Typically the Source Address of an invoking Router
> Solicitation or the all-nodes multicast address." — §4.2, `rfc4861.txt:1071-1073`

- Strength: description. Class: wire.
- Check idea: a periodic Router Advertisement goes to ff02::1. A Router Advertisement in answer
  to a Router Solicitation from host A goes to ff02::1 or to the source address of host A.

### RFC4861-RA-5

**A Router Advertisement has IP Hop Limit 255.**

> "Hop Limit      255" — §4.2, `rfc4861.txt:1075`

- Strength: description. Class: wire.
- Check idea: capture the Router Advertisements of a router. Each one has Hop Limit 255 in the
  IP header.

### RFC4861-RA-6

**A Router Advertisement has ICMP Type 134.**

> "Type           134" — §4.2, `rfc4861.txt:1079`

- Strength: description. Class: wire.
- Check idea: capture the Router Advertisements of a router. Each one has ICMP Type 134.

### RFC4861-RA-7

**A Router Advertisement has ICMP Code 0.**

> "Code           0" — §4.2, `rfc4861.txt:1081`

- Strength: description. Class: wire.
- Check idea: capture the Router Advertisements of a router. Each one has ICMP Code 0.

### RFC4861-RA-8

**The Checksum field of a Router Advertisement holds the ICMPv6 checksum.**

> "Checksum       The ICMP checksum.  See [ICMPv6]." — §4.2, `rfc4861.txt:1083`

- Strength: description. Class: wire.
- Check idea: compute the ICMPv6 checksum, with the IPv6 pseudo-header, over a captured Router
  Advertisement. The result is equal to the Checksum field.

### RFC4861-RA-9

**Cur Hop Limit is the default Hop Limit that a host puts in the IP header of the packets that it sends.**

> "Cur Hop Limit  8-bit unsigned integer.  The default value that
> should be placed in the Hop Count field of the IP
> header for outgoing IP packets." — §4.2, `rfc4861.txt:1085-1087`

- Strength: should (lower case). Class: end-to-end.
- Check idea: a router advertises Cur Hop Limit 32. After it receives the Router Advertisement,
  a host sends its packets with Hop Limit 32.

### RFC4861-RA-10

**A Cur Hop Limit of zero means that the router does not specify a value.**

> "A value of zero
> means unspecified (by this router)." — §4.2, `rfc4861.txt:1087-1088`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises Cur Hop Limit 0. A host keeps the Hop Limit that it used
  before the Router Advertisement.

### RFC4861-RA-11

**When the M flag is set, it tells hosts that addresses are available through DHCPv6.**

> "When
> set, it indicates that addresses are available via
> Dynamic Host Configuration Protocol [DHCPv6]." — §4.2, `rfc4861.txt:1090-1092`

- Strength: description. Class: wire.
- Check idea: a router configured for managed address configuration sends Router
  Advertisements with M = 1. A router without that configuration sends M = 0.

### RFC4861-RA-12

**When the O flag is set, it tells hosts that other configuration information is available through DHCPv6.**

> "When set, it
> indicates that other configuration information is
> available via DHCPv6." — §4.2, `rfc4861.txt:1098-1100`

- Strength: description. Class: wire.
- Check idea: a router configured to announce other configuration sends Router Advertisements
  with O = 1. A router without that configuration sends O = 0.

### RFC4861-RA-13

**The sender sets the 6-bit Reserved field of a Router Advertisement to zero.**

> "Reserved       A 6-bit unused field.  It MUST be initialized to
> zero by the sender" — §4.2, `rfc4861.txt:1107-1108`

- Strength: must. Class: wire.
- Check idea: capture the Router Advertisements of a router. The 6 bits after the O flag are
  zero in each one.

### RFC4861-RA-14

**A receiver ignores the value of the Reserved field of a Router Advertisement.**

> "Reserved       A 6-bit unused field." — §4.2, `rfc4861.txt:1107`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1108-1109`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with nonzero Reserved bits and a Prefix
  Information option. A host still puts the router in its default router list and uses the
  prefix.

### RFC4861-RA-15

**Router Lifetime is the time, in seconds, for which the sender is a default router.**

> "Router Lifetime
> 16-bit unsigned integer.  The lifetime associated
> with the default router in units of seconds." — §4.2, `rfc4861.txt:1111-1113`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises Router Lifetime 30 and then sends no more Router
  Advertisements. A host sends off-link packets to the router for 30 s, and after that time it
  stops.

### RFC4861-RA-16

**A receiver accepts any Router Lifetime up to 65535 seconds, although the rules for senders limit the value to 9000 seconds.**

> "The
> field can contain values up to 65535 and receivers
> should handle any value, while the sending rules in
> Section 6 limit the lifetime to 9000 seconds." — §4.2, `rfc4861.txt:1113-1116`

- Strength: should (lower case). Class: end-to-end.
- Check idea: host A receives a Router Advertisement with Router Lifetime 65535. The host keeps
  the router as default router for 65535 s and does not cut the value to 9000 s.

### RFC4861-RA-17

**A router that advertises Router Lifetime 0 is not a default router, and a host does not put it in its default router list.**

> "A
> Lifetime of 0 indicates that the router is not a
> default router and SHOULD NOT appear on the default
> router list." — §4.2, `rfc4861.txt:1116-1118`, `rfc4861.txt:1127`

- Strength: should not. Class: end-to-end.
- Check idea: the only router on a link advertises Router Lifetime 0. A host sends no off-link
  packet to that router.

### RFC4861-RA-18

**Router Lifetime applies only to the default router role of the sender; it does not limit the other fields or options of the message.**

> "The Router Lifetime applies only to
> the router's usefulness as a default router; it
> does not apply to information contained in other
> message fields or options." — §4.2, `rfc4861.txt:1127-1130`; "Options that need time
> limits for their information include their own
> lifetime fields." — `rfc4861.txt:1130-1132`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises Router Lifetime 0 with a Prefix Information option, L = 1. A
  host still treats the prefix as on-link for the Valid Lifetime of the option.

### RFC4861-RA-19

**Reachable Time is the time, in milliseconds, for which a node assumes that a neighbor is reachable after a reachability confirmation.**

> "Reachable Time 32-bit unsigned integer.  The time, in
> milliseconds, that a node assumes a neighbor is
> reachable after having received a reachability
> confirmation.  Used by the Neighbor Unreachability
> Detection algorithm (see Section 7.3)." — §4.2, `rfc4861.txt:1134-1138`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises Reachable Time 10000. Host A confirms neighbor B, then sends
  to B again after 20 s. Host A starts Neighbor Unreachability Detection for B.

### RFC4861-RA-20

**A Reachable Time of zero means that the router does not specify a value.**

> "A value of
> zero means unspecified (by this router)." — §4.2, `rfc4861.txt:1138-1139`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises Reachable Time 10000 and later Reachable Time 0. A host keeps
  the value 10000 as the base for its reachable time.

### RFC4861-RA-21

**Retrans Timer is the time, in milliseconds, between two retransmitted Neighbor Solicitations.**

> "Retrans Timer  32-bit unsigned integer.  The time, in
> milliseconds, between retransmitted Neighbor
> Solicitation messages.  Used by address resolution
> and the Neighbor Unreachability Detection algorithm
> (see Sections 7.2 and 7.3)." — §4.2, `rfc4861.txt:1141-1145`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises Retrans Timer 2000. A host resolves an address that does not
  answer. Its Neighbor Solicitations are 2 s apart.

### RFC4861-RA-22

**A Retrans Timer of zero means that the router does not specify a value.**

> "A value of zero means
> unspecified (by this router)." — §4.2, `rfc4861.txt:1145-1146`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises Retrans Timer 2000 and later Retrans Timer 0. A host still
  sends its retransmitted Neighbor Solicitations 2 s apart.

### RFC4861-RA-23

**The Source Link-Layer Address option of a Router Advertisement holds the link-layer address of the interface that sends it.**

> "Source link-layer address
> The link-layer address of the interface from which
> the Router Advertisement is sent.  Only used on
> link layers that have addresses." — §4.2, `rfc4861.txt:1150-1153`

- Strength: description. Class: wire.
- Check idea: on an Ethernet link, capture a Router Advertisement with this option. The address
  in the option is the MAC address of the router interface on that link.

### RFC4861-RA-24

**A router may leave out the Source Link-Layer Address option, to share inbound load across several link-layer addresses.**

> "A router MAY omit
> this option in order to enable inbound load sharing
> across multiple link-layer addresses." — §4.2, `rfc4861.txt:1153-1155`

- Strength: may. Class: wire.
- Check idea: a router configured to omit the option sends Router Advertisements with no Source
  Link-Layer Address option. A host then resolves the router address with a Neighbor
  Solicitation.

### RFC4861-RA-25

**A router sends the MTU option in its Router Advertisements on links that have a variable MTU.**

> "MTU            SHOULD be sent on links that have a variable MTU
> (as specified in the document that describes how to
> run IP over the particular link type)." — §4.2, `rfc4861.txt:1157-1159`

- Strength: should. Class: wire.
- Check idea: on a link type with a variable MTU, capture the Router Advertisements of a router.
  Each one carries an MTU option.

### RFC4861-RA-26

**A router may send the MTU option on other links too.**

> "MAY be sent
> on other links." — §4.2, `rfc4861.txt:1159-1160`

- Strength: may. Class: wire.
- Check idea: on an Ethernet link, a router with a configured link MTU sends an MTU option. A
  host accepts a Router Advertisement with or without the option.

### RFC4861-RA-27

**A router includes a Prefix Information option for each of its on-link prefixes, except the link-local prefix.**

> "These options specify the prefixes that are on-link
> and/or are used for stateless address
> autoconfiguration." — §4.2, `rfc4861.txt:1163-1165`; "A router SHOULD include all its
> on-link prefixes (except the link-local prefix) so
> that multihomed hosts have complete prefix
> information about on-link destinations for the
> links to which they attach." — `rfc4861.txt:1165-1169`

- Strength: should. Class: wire.
- Check idea: a router has two on-link prefixes on one interface. Its Router Advertisements on
  that link carry a Prefix Information option for each prefix, and none for fe80::/64.

### RFC4861-RA-28

**A receiver of a Router Advertisement silently ignores an option that it does not recognize, and it processes the rest of the message.**

> "Future versions of this protocol may define new option types.
> Receivers MUST silently ignore any options they do not recognize
> and continue processing the message." — §4.2, `rfc4861.txt:1183-1185`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with an option of an unknown type before a
  Prefix Information option. A host still uses the router as default router and uses the
  prefix, and it sends no ICMPv6 error.

## Neighbor Solicitation message

### RFC4861-NS-1

**A node sends a Neighbor Solicitation to ask for the link-layer address of a target, and it gives its own link-layer address to the target.**

> "Nodes send Neighbor Solicitations to request the link-layer address
> of a target node while also providing their own link-layer address to
> the target." — §4.3, `rfc4861.txt:1189-1191`

- Strength: description. Class: wire.
- Check idea: host A sends a packet to host B and does not know the link-layer address of B.
  Host A sends a Neighbor Solicitation for B that carries the link-layer address of A.

### RFC4861-NS-2

**A node multicasts a Neighbor Solicitation to resolve an address, and unicasts it to verify that a neighbor is reachable.**

> "Neighbor Solicitations are multicast when the node needs
> to resolve an address and unicast when the node seeks to verify the
> reachability of a neighbor." — §4.3, `rfc4861.txt:1191-1193`

- Strength: description. Class: wire.
- Check idea: for address resolution, host A sends its Neighbor Solicitation to a multicast
  address. For a reachability probe of a known neighbor, host A sends it to the unicast address
  of the neighbor.

### RFC4861-NS-3

**A Neighbor Solicitation holds Type, Code, Checksum, a 32-bit Reserved field, the 128-bit Target Address, and then options.**

> "|     Type      |     Code      |          Checksum             |" — §4.3,
> `rfc4861.txt:1198`; "|                           Reserved                            |" —
> `rfc4861.txt:1200`; "+                       Target Address                          +" —
> `rfc4861.txt:1205`; "|   Options ..." — `rfc4861.txt:1210`

- Strength: description. Class: encoding.
- Check idea: decode a Neighbor Solicitation. After Type, Code and Checksum come 4 octets of
  Reserved, 16 octets of Target Address, and then the options.

### RFC4861-NS-4

**The Source Address of a Neighbor Solicitation is an address of the interface that sends it, or the unspecified address during Duplicate Address Detection.**

> "Source Address
> Either an address assigned to the interface from
> which this message is sent or (if Duplicate Address
> Detection is in progress [ADDRCONF]) the
> unspecified address." — §4.3, `rfc4861.txt:1215-1219`

- Strength: description. Class: wire.
- Check idea: a Neighbor Solicitation for address resolution has an address of the interface as
  source. A Neighbor Solicitation for Duplicate Address Detection has the source ::.

### RFC4861-NS-5

**The Destination Address of a Neighbor Solicitation is the solicited-node multicast address of the target, or the target address.**

> "Destination Address
> Either the solicited-node multicast address
> corresponding to the target address, or the target
> address." — §4.3, `rfc4861.txt:1220-1223`

- Strength: description. Class: wire.
- Check idea: a multicast Neighbor Solicitation goes to ff02::1:ffXX:XXXX, where XX:XXXX are the
  low 24 bits of the target. A unicast Neighbor Solicitation goes to the target address.

### RFC4861-NS-6

**A Neighbor Solicitation has IP Hop Limit 255.**

> "Hop Limit      255" — §4.3, `rfc4861.txt:1224`

- Strength: description. Class: wire.
- Check idea: capture the Neighbor Solicitations of a host and of a router. Each one has Hop
  Limit 255 in the IP header.

### RFC4861-NS-7

**A Neighbor Solicitation has ICMP Type 135.**

> "Type           135" — §4.3, `rfc4861.txt:1228`

- Strength: description. Class: wire.
- Check idea: capture the Neighbor Solicitations on a link. Each one has ICMP Type 135.

### RFC4861-NS-8

**A Neighbor Solicitation has ICMP Code 0.**

> "Code           0" — §4.3, `rfc4861.txt:1230`

- Strength: description. Class: wire.
- Check idea: capture the Neighbor Solicitations on a link. Each one has ICMP Code 0.

### RFC4861-NS-9

**The Checksum field of a Neighbor Solicitation holds the ICMPv6 checksum.**

> "Checksum       The ICMP checksum.  See [ICMPv6]." — §4.3, `rfc4861.txt:1239`

- Strength: description. Class: wire.
- Check idea: compute the ICMPv6 checksum, with the IPv6 pseudo-header, over a captured Neighbor
  Solicitation. The result is equal to the Checksum field.

### RFC4861-NS-10

**The sender sets the Reserved field of a Neighbor Solicitation to zero.**

> "Reserved       This field is unused.  It MUST be initialized to
> zero by the sender" — §4.3, `rfc4861.txt:1241-1242`

- Strength: must. Class: wire.
- Check idea: capture the Neighbor Solicitations of a host. The Reserved field of each one is
  zero.

### RFC4861-NS-11

**A receiver ignores the value of the Reserved field of a Neighbor Solicitation.**

> "Reserved       This field is unused." — §4.3, `rfc4861.txt:1241`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1242-1243`

- Strength: must. Class: end-to-end.
- Check idea: host A sends a Neighbor Solicitation for host B with a nonzero Reserved field.
  Host B answers with a Neighbor Advertisement.

### RFC4861-NS-12

**The Target Address of a Neighbor Solicitation is the IP address of the target of the solicitation.**

> "Target Address The IP address of the target of the solicitation." — §4.3, `rfc4861.txt:1245`

- Strength: description. Class: wire.
- Check idea: host A resolves the address of host B. The Target Address of the Neighbor
  Solicitation is the address of B.

### RFC4861-NS-13

**The Target Address of a Neighbor Solicitation is never a multicast address.**

> "It MUST NOT be a multicast address." — §4.3, `rfc4861.txt:1246`

- Strength: must not. Class: wire.
- Check idea: capture the Neighbor Solicitations of a host for address resolution, reachability
  probes and Duplicate Address Detection. No Target Address is a multicast address.

### RFC4861-NS-14

**A Neighbor Solicitation from the unspecified address carries no Source Link-Layer Address option.**

> "MUST NOT be
> included when the source IP address is the
> unspecified address." — §4.3, `rfc4861.txt:1251-1253`

- Strength: must not. Class: wire.
- Check idea: a host runs Duplicate Address Detection for a new address. Its Neighbor
  Solicitations from :: carry no Source Link-Layer Address option.

### RFC4861-NS-15

**On link layers with addresses, a multicast Neighbor Solicitation from an assigned address carries a Source Link-Layer Address option with the sender's link-layer address.**

> "The link-layer address for the sender." — §4.3, `rfc4861.txt:1251`; "Otherwise, on link layers
> that have addresses this option MUST be included in
> multicast solicitations" — `rfc4861.txt:1253-1255`

- Strength: must. Class: wire.
- Check idea: on an Ethernet link, host A resolves the address of host B. The multicast Neighbor
  Solicitation carries a Source Link-Layer Address option with the MAC address of A.

### RFC4861-NS-16

**A unicast Neighbor Solicitation carries a Source Link-Layer Address option, on link layers that have addresses.**

> "and SHOULD be included in
> unicast solicitations." — §4.3, `rfc4861.txt:1255-1256`

- Strength: should. Class: wire.
- Check idea: on an Ethernet link, host A probes the reachability of neighbor B. The unicast
  Neighbor Solicitation carries a Source Link-Layer Address option with the MAC address of A.

### RFC4861-NS-17

**A receiver of a Neighbor Solicitation silently ignores an option that it does not recognize, and it processes the rest of the message.**

> "Future versions of this protocol may define new option types.
> Receivers MUST silently ignore any options they do not recognize
> and continue processing the message." — §4.3, `rfc4861.txt:1258-1260`

- Strength: must. Class: end-to-end.
- Check idea: host A sends a Neighbor Solicitation for host B with an option of an unknown type
  before its Source Link-Layer Address option. Host B answers with a Neighbor Advertisement and
  sends no ICMPv6 error.

## Neighbor Advertisement message

### RFC4861-NA-1

**A node sends Neighbor Advertisements in answer to Neighbor Solicitations, and it sends unsolicited Neighbor Advertisements to spread new information quickly.**

> "A node sends Neighbor Advertisements in response to Neighbor
> Solicitations and sends unsolicited Neighbor Advertisements in order
> to (unreliably) propagate new information quickly." — §4.4, `rfc4861.txt:1264-1266`

- Strength: description. Class: wire.
- Check idea: host B answers a Neighbor Solicitation from host A with a Neighbor Advertisement.
  When the link-layer address of B changes, B sends a Neighbor Advertisement with no
  solicitation.

### RFC4861-NA-2

**A Neighbor Advertisement holds Type, Code, Checksum, the R, S and O flags, a 29-bit Reserved field, the 128-bit Target Address, and then options.**

> "|     Type      |     Code      |          Checksum             |" — §4.4,
> `rfc4861.txt:1271`; "|R|S|O|                     Reserved                            |" —
> `rfc4861.txt:1273`; "+                       Target Address                          +" —
> `rfc4861.txt:1278`; "|   Options ..." — `rfc4861.txt:1283`

- Strength: description. Class: encoding.
- Check idea: decode a Neighbor Advertisement. After Type, Code and Checksum come the R, S and O
  flags and 29 bits of Reserved. Then come 16 octets of Target Address and the options.

### RFC4861-NA-3

**The Source Address of a Neighbor Advertisement is an address of the interface that sends it.**

> "Source Address
> An address assigned to the interface from which the
> advertisement is sent." — §4.4, `rfc4861.txt:1297-1299`

- Strength: description. Class: wire.
- Check idea: host B answers a Neighbor Solicitation. The Source Address of the Neighbor
  Advertisement is an address of the interface of B on that link.

### RFC4861-NA-4

**A solicited Neighbor Advertisement goes to the source of the Neighbor Solicitation, or to the all-nodes multicast address when that source is the unspecified address.**

> "Destination Address
> For solicited advertisements, the Source Address of
> an invoking Neighbor Solicitation or, if the
> solicitation's Source Address is the unspecified
> address, the all-nodes multicast address." — §4.4, `rfc4861.txt:1300-1304`

- Strength: description. Class: wire.
- Check idea: host B answers a Neighbor Solicitation from host A to the address of A. Host B
  answers a Duplicate Address Detection solicitation from :: to ff02::1.

### RFC4861-NA-5

**An unsolicited Neighbor Advertisement typically goes to the all-nodes multicast address.**

> "For unsolicited advertisements typically the all-
> nodes multicast address." — §4.4, `rfc4861.txt:1306-1307`

- Strength: description. Class: wire.
- Check idea: the link-layer address of host B changes. The unsolicited Neighbor Advertisements
  of B go to ff02::1.

### RFC4861-NA-6

**A Neighbor Advertisement has IP Hop Limit 255.**

> "Hop Limit      255" — §4.4, `rfc4861.txt:1309`

- Strength: description. Class: wire.
- Check idea: capture the solicited and unsolicited Neighbor Advertisements of a node. Each one
  has Hop Limit 255 in the IP header.

### RFC4861-NA-7

**A Neighbor Advertisement has ICMP Type 136.**

> "Type           136" — §4.4, `rfc4861.txt:1313`

- Strength: description. Class: wire.
- Check idea: capture the Neighbor Advertisements on a link. Each one has ICMP Type 136.

### RFC4861-NA-8

**A Neighbor Advertisement has ICMP Code 0.**

> "Code           0" — §4.4, `rfc4861.txt:1315`

- Strength: description. Class: wire.
- Check idea: capture the Neighbor Advertisements on a link. Each one has ICMP Code 0.

### RFC4861-NA-9

**The Checksum field of a Neighbor Advertisement holds the ICMPv6 checksum.**

> "Checksum       The ICMP checksum.  See [ICMPv6]." — §4.4, `rfc4861.txt:1317`

- Strength: description. Class: wire.
- Check idea: compute the ICMPv6 checksum, with the IPv6 pseudo-header, over a captured Neighbor
  Advertisement. The result is equal to the Checksum field.

### RFC4861-NA-10

**The R flag is set when the sender of the Neighbor Advertisement is a router.**

> "R              Router flag.  When set, the R-bit indicates that
> the sender is a router." — §4.4, `rfc4861.txt:1319-1320`

- Strength: description. Class: wire.
- Check idea: a router and a host each answer a Neighbor Solicitation. The Neighbor
  Advertisement of the router has R = 1, and that of the host has R = 0.

### RFC4861-NA-11

**Neighbor Unreachability Detection uses the R flag to find a router that becomes a host.**

> "The R-bit is used by
> Neighbor Unreachability Detection to detect a
> router that changes to a host." — §4.4, `rfc4861.txt:1320-1322`

- Strength: description. Class: end-to-end.
- Check idea: host A uses router R as its default router. Router R then sends a Neighbor
  Advertisement with R = 0, and host A no longer sends off-link packets to R.

### RFC4861-NA-12

**The S flag is set when the Neighbor Advertisement answers a Neighbor Solicitation from the Destination Address.**

> "S              Solicited flag.  When set, the S-bit indicates that
> the advertisement was sent in response to a
> Neighbor Solicitation from the Destination address." — §4.4, `rfc4861.txt:1324-1326`

- Strength: description. Class: wire.
- Check idea: host B answers a Neighbor Solicitation from host A. The Neighbor Advertisement goes
  to A and has S = 1.

### RFC4861-NA-13

**Neighbor Unreachability Detection uses the S flag as a reachability confirmation.**

> "The S-bit is used as a reachability confirmation
> for Neighbor Unreachability Detection." — §4.4, `rfc4861.txt:1327-1328`

- Strength: description. Class: end-to-end.
- Check idea: host A probes neighbor B with a unicast Neighbor Solicitation, and B answers with S
  = 1. Host A sends no more probes to B.

### RFC4861-NA-14

**The S flag is clear in multicast Neighbor Advertisements and in unsolicited unicast Neighbor Advertisements.**

> "It MUST NOT
> be set in multicast advertisements or in
> unsolicited unicast advertisements." — §4.4, `rfc4861.txt:1328-1330`

- Strength: must not. Class: wire.
- Check idea: capture the answer of host B to a Duplicate Address Detection solicitation, which
  goes to ff02::1, and the unsolicited Neighbor Advertisements of B. Each one has S = 0.

### RFC4861-NA-15

**When the O flag is set, the Neighbor Advertisement overrides a cache entry and updates the cached link-layer address.**

> "O              Override flag.  When set, the O-bit indicates that
> the advertisement should override an existing cache
> entry and update the cached link-layer address." — §4.4, `rfc4861.txt:1332-1334`

- Strength: should (lower case). Class: end-to-end.
- Check idea: host A has the link-layer address L1 of host B in its cache. Host A receives a
  Neighbor Advertisement for B with O = 1 and address L2, and then sends to B at L2.

### RFC4861-NA-16

**When the O flag is clear, the Neighbor Advertisement does not change a cached link-layer address. But it fills a cache entry that has no link-layer address.**

> "When it is not set the advertisement will not
> update a cached link-layer address though it will
> update an existing Neighbor Cache entry for which
> no link-layer address is known." — §4.4, `rfc4861.txt:1335-1338`

- Strength: description. Class: end-to-end.
- Check idea: host A has address L1 of host B in its cache. It receives an advertisement for B
  with O = 0 and address L2, and still sends to L1. Host A resolves host C, receives an answer
  with O = 0, and sends to the address in that answer.

### RFC4861-NA-17

**The O flag is clear in a solicited Neighbor Advertisement for an anycast address and in a solicited proxy Neighbor Advertisement.**

> "It SHOULD NOT be
> set in solicited advertisements for anycast
> addresses and in solicited proxy advertisements." — §4.4, `rfc4861.txt:1338-1340`

- Strength: should not. Class: wire.
- Check idea: a node answers a Neighbor Solicitation for an anycast address that it holds, and a
  proxy answers for an address that it serves. Each Neighbor Advertisement has O = 0.

### RFC4861-NA-18

**The O flag is set in all other solicited Neighbor Advertisements and in unsolicited Neighbor Advertisements.**

> "It SHOULD be set in other solicited advertisements
> and in unsolicited advertisements." — §4.4, `rfc4861.txt:1341-1342`

- Strength: should. Class: wire.
- Check idea: host B answers a Neighbor Solicitation for its own unicast address, and later sends
  unsolicited Neighbor Advertisements. Each Neighbor Advertisement has O = 1.

### RFC4861-NA-19

**The sender sets the 29-bit Reserved field of a Neighbor Advertisement to zero.**

> "Reserved       29-bit unused field.  It MUST be initialized to
> zero by the sender" — §4.4, `rfc4861.txt:1351-1352`

- Strength: must. Class: wire.
- Check idea: capture the Neighbor Advertisements of a node. The 29 bits after the O flag are
  zero in each one.

### RFC4861-NA-20

**A receiver ignores the value of the Reserved field of a Neighbor Advertisement.**

> "Reserved       29-bit unused field." — §4.4, `rfc4861.txt:1351`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1352-1353`

- Strength: must. Class: end-to-end.
- Check idea: host B answers the Neighbor Solicitation of host A with a nonzero Reserved field.
  Host A completes address resolution and sends its queued packet to B.

### RFC4861-NA-21

**The Target Address of a solicited Neighbor Advertisement is the Target Address of the Neighbor Solicitation that caused it.**

> "Target Address
> For solicited advertisements, the Target Address
> field in the Neighbor Solicitation message that
> prompted this advertisement." — §4.4, `rfc4861.txt:1355-1358`

- Strength: description. Class: wire.
- Check idea: host A sends a Neighbor Solicitation with Target Address X to host B. The Neighbor
  Advertisement of B has Target Address X.

### RFC4861-NA-22

**The Target Address of an unsolicited Neighbor Advertisement is the address whose link-layer address changed.**

> "For an unsolicited
> advertisement, the address whose link-layer address
> has changed." — §4.4, `rfc4861.txt:1358-1360`

- Strength: description. Class: wire.
- Check idea: the link-layer address of host B changes. The unsolicited Neighbor Advertisements
  of B carry the IP addresses of B as Target Address.

### RFC4861-NA-23

**The Target Address of a Neighbor Advertisement is never a multicast address.**

> "The Target Address MUST NOT be a
> multicast address." — §4.4, `rfc4861.txt:1360-1361`

- Strength: must not. Class: wire.
- Check idea: capture the solicited and unsolicited Neighbor Advertisements of a node. No Target
  Address is a multicast address.

### RFC4861-NA-24

**On link layers with addresses, a Neighbor Advertisement in answer to a multicast solicitation carries a Target Link-Layer Address option with the sender's link-layer address.**

> "The link-layer address for the target, i.e., the
> sender of the advertisement." — §4.4, `rfc4861.txt:1366-1367`; "This option MUST be
> included on link layers that have addresses when
> responding to multicast solicitations." — `rfc4861.txt:1367-1369`; "The option MUST be included for multicast
> solicitations" — `rfc4861.txt:1373-1374`

- Strength: must. Class: wire.
- Check idea: on an Ethernet link, host B answers a multicast Neighbor Solicitation. The Neighbor
  Advertisement carries a Target Link-Layer Address option with the MAC address of B.

### RFC4861-NA-25

**A Neighbor Advertisement in answer to a unicast Neighbor Solicitation carries a Target Link-Layer Address option.**

> "When
> responding to a unicast Neighbor Solicitation this
> option SHOULD be included." — §4.4, `rfc4861.txt:1369-1371`

- Strength: should. Class: wire.
- Check idea: on an Ethernet link, host B answers a unicast reachability probe from host A. The
  Neighbor Advertisement carries a Target Link-Layer Address option with the MAC address of B.

### RFC4861-NA-26

**A receiver of a Neighbor Advertisement silently ignores an option that it does not recognize, and it processes the rest of the message.**

> "Future versions of this protocol may define new option types.
> Receivers MUST silently ignore any options they do not recognize
> and continue processing the message." — §4.4, `rfc4861.txt:1388-1390`

- Strength: must. Class: end-to-end.
- Check idea: host B answers a Neighbor Solicitation of host A with an option of an unknown type
  before the Target Link-Layer Address option. Host A completes address resolution and sends no
  ICMPv6 error.

## Redirect message

### RFC4861-RDM-1

**A router sends a Redirect to tell a host about a better first hop for a destination: a router or the destination itself.**

> "Routers send Redirect packets to inform a host of a better first-hop
> node on the path to a destination.  Hosts can be redirected to a
> better first-hop router but can also be informed by a redirect that
> the destination is in fact a neighbor." — §4.5, `rfc4861.txt:1409-1412`

- Strength: description. Class: wire.
- Check idea: host A sends a packet for destination D to router R1, and router R2 on the same
  link is the better first hop. Router R1 forwards the packet and sends a Redirect to A.

### RFC4861-RDM-2

**A Redirect holds Type, Code, Checksum, a 32-bit Reserved field, the 128-bit Target Address, the 128-bit Destination Address, and then options.**

> "|     Type      |     Code      |          Checksum             |" — §4.5,
> `rfc4861.txt:1419`; "|                           Reserved                            |" —
> `rfc4861.txt:1421`; "+                       Target Address                          +" —
> `rfc4861.txt:1426`; "+                     Destination Address                       +" —
> `rfc4861.txt:1434`; "|   Options ..." — `rfc4861.txt:1439`

- Strength: description. Class: encoding.
- Check idea: decode a Redirect. After Type, Code and Checksum come 4 octets of Reserved, 16
  octets of Target Address, 16 octets of Destination Address, and then the options.

### RFC4861-RDM-3

**A router sends a Redirect from the link-local address of the interface that sends it.**

> "Source Address
> MUST be the link-local address assigned to the
> interface from which this message is sent." — §4.5, `rfc4861.txt:1444-1446`

- Strength: must. Class: wire.
- Check idea: a router with a global address on the link of host A sends a Redirect to A. The
  Source Address is the link-local address of the router interface.

### RFC4861-RDM-4

**A Redirect goes to the Source Address of the packet that caused it.**

> "Destination Address
> The Source Address of the packet that triggered the
> redirect." — §4.5, `rfc4861.txt:1448-1450`

- Strength: description. Class: wire.
- Check idea: host A sends a packet from address S that causes a Redirect. The IP Destination
  Address of the Redirect is S.

### RFC4861-RDM-5

**A Redirect has IP Hop Limit 255.**

> "Hop Limit      255" — §4.5, `rfc4861.txt:1452`

- Strength: description. Class: wire.
- Check idea: capture the Redirects of a router. Each one has Hop Limit 255 in the IP header.

### RFC4861-RDM-6

**A Redirect has ICMP Type 137.**

> "Type           137" — §4.5, `rfc4861.txt:1465`

- Strength: description. Class: wire.
- Check idea: capture the Redirects of a router. Each one has ICMP Type 137.

### RFC4861-RDM-7

**A Redirect has ICMP Code 0.**

> "Code           0" — §4.5, `rfc4861.txt:1467`

- Strength: description. Class: wire.
- Check idea: capture the Redirects of a router. Each one has ICMP Code 0.

### RFC4861-RDM-8

**The Checksum field of a Redirect holds the ICMPv6 checksum.**

> "Checksum       The ICMP checksum.  See [ICMPv6]." — §4.5, `rfc4861.txt:1469`

- Strength: description. Class: wire.
- Check idea: compute the ICMPv6 checksum, with the IPv6 pseudo-header, over a captured Redirect.
  The result is equal to the Checksum field.

### RFC4861-RDM-9

**The sender sets the Reserved field of a Redirect to zero.**

> "Reserved       This field is unused.  It MUST be initialized to
> zero by the sender" — §4.5, `rfc4861.txt:1471-1472`

- Strength: must. Class: wire.
- Check idea: capture the Redirects of a router. The Reserved field of each one is zero.

### RFC4861-RDM-10

**A receiver ignores the value of the Reserved field of a Redirect.**

> "Reserved       This field is unused." — §4.5, `rfc4861.txt:1471`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1472-1473`

- Strength: must. Class: end-to-end.
- Check idea: router R1 sends host A a Redirect to router R2 with a nonzero Reserved field. Host
  A sends its next packet for that destination to R2.

### RFC4861-RDM-11

**The Target Address of a Redirect is a better first hop for the ICMP Destination Address.**

> "Target Address
> An IP address that is a better first hop to use for
> the ICMP Destination Address." — §4.5, `rfc4861.txt:1475-1477`

- Strength: description. Class: wire.
- Check idea: host A sends a packet for destination D to router R1, and router R2 is the better
  first hop. The Redirect of R1 has Destination Address D and names R2 as Target Address.

### RFC4861-RDM-12

**When the destination is a neighbor of the host, the Target Address of the Redirect is equal to the ICMP Destination Address.**

> "The latter is accomplished by
> setting the ICMP Target Address equal to the ICMP Destination
> Address." — §4.5, `rfc4861.txt:1412-1414`; "When the target is
> the actual endpoint of communication, i.e., the
> destination is a neighbor, the Target Address field
> MUST contain the same value as the ICMP Destination
> Address field." — `rfc4861.txt:1477-1481`

- Strength: must. Class: wire.
- Check idea: host A sends a packet through router R to host D, which is on the same link as A.
  The Redirect of R has Target Address D and Destination Address D.

### RFC4861-RDM-13

**When the target is a better first-hop router, the Target Address of the Redirect is the link-local address of that router.**

> "Otherwise, the target is a better
> first-hop router and the Target Address MUST be the
> router's link-local address so that hosts can
> uniquely identify routers." — §4.5, `rfc4861.txt:1481-1484`

- Strength: must. Class: wire.
- Check idea: router R2 has a global address and a link-local address on the link of host A.
  The Redirect of router R1 to R2 has the link-local address of R2 as Target Address.

### RFC4861-RDM-14

**The ICMP Destination Address of a Redirect is the destination that the Redirect sends to the target.**

> "Destination Address
> The IP address of the destination that is
> redirected to the target." — §4.5, `rfc4861.txt:1486-1488`

- Strength: description. Class: wire.
- Check idea: host A sends a packet for destination D that causes a Redirect. The ICMP
  Destination Address field of the Redirect is D, the IP Destination Address of that packet.

### RFC4861-RDM-15

**A Redirect carries a Target Link-Layer Address option with the link-layer address of the target, when the router knows it.**

> "Target link-layer address
> The link-layer address for the target.  It SHOULD
> be included (if known)." — §4.5, `rfc4861.txt:1492-1494`

- Strength: should. Class: wire.
- Check idea: router R1 has the link-layer address of router R2 in its cache and redirects host A
  to R2. The Redirect carries a Target Link-Layer Address option with the MAC address of R2.

### RFC4861-RDM-16

**On an NBMA link where hosts learn link-layer addresses from Redirects, every Redirect carries the Target Link-Layer Address option.**

> "Note that on NBMA links,
> hosts may rely on the presence of the Target Link-
> Layer Address option in Redirect messages as the
> means for determining the link-layer addresses of
> neighbors.  In such cases, the option MUST be
> included in Redirect messages." — §4.5, `rfc4861.txt:1494-1499`

- Strength: must. Class: wire.
- Check idea: on an NBMA link where hosts have no other way to learn link-layer addresses, a
  router redirects a host. Each Redirect carries a Target Link-Layer Address option.

### RFC4861-RDM-17

**The Redirected Header option holds as much of the packet that caused the Redirect as fits in the minimum IPv6 MTU.**

> "Redirected Header
> As much as possible of the IP packet that triggered
> the sending of the Redirect without making the
> redirect packet exceed the minimum MTU specified in
> [IPv6]." — §4.5, `rfc4861.txt:1501-1505`

- Strength: description. Class: wire.
- Check idea: host A sends a 1400-octet packet that causes a Redirect. The Redirect is 1280
  octets or less, and its Redirected Header option holds the first part of that packet.

## Options

### RFC4861-OPT-1

**A Neighbor Discovery message holds zero or more options, and some options can occur more than once in one message.**

> "Neighbor Discovery messages include zero or more options, some of
> which may appear multiple times in the same message." — §4.6, `rfc4861.txt:1521-1522`

- Strength: may (lower case). Class: encoding.
- Check idea: a router sends a Router Advertisement with two Prefix Information options. A host
  uses both prefixes.

### RFC4861-OPT-2

**Each option is padded, when necessary, so that it ends on a 64-bit boundary.**

> "Options should
> be padded when necessary to ensure that they end on their natural
> 64-bit boundaries." — §4.6, `rfc4861.txt:1522-1524`

- Strength: should (lower case). Class: encoding.
- Check idea: decode the options of captured Neighbor Discovery messages. The size of each option
  is a multiple of 8 octets.

### RFC4861-OPT-3

**Every option starts with an 8-bit Type and an 8-bit Length, and the option data follows.**

> "All options are of the form:" — §4.6, `rfc4861.txt:1524`; "|     Type      |    Length     |              ...              |" — `rfc4861.txt:1529`

- Strength: description. Class: encoding.
- Check idea: decode the options of captured Neighbor Discovery messages. Each option starts with
  one octet of Type and one octet of Length.

### RFC4861-OPT-4

**The option types are 1 (Source Link-Layer Address), 2 (Target Link-Layer Address), 3 (Prefix Information), 4 (Redirected Header) and 5 (MTU).**

> "Type           8-bit identifier of the type of option.  The
> options defined in this document are:" — §4.6, `rfc4861.txt:1536-1537`; "Source Link-Layer Address                    1" — `rfc4861.txt:1541`; "Target Link-Layer Address                    2" — `rfc4861.txt:1542`; "Prefix Information                           3" — `rfc4861.txt:1543`; "Redirected Header                            4" — `rfc4861.txt:1544`; "MTU                                          5" — `rfc4861.txt:1545`

- Strength: description. Class: wire.
- Check idea: capture a Router Solicitation, a Router Advertisement, a Neighbor Advertisement and
  a Redirect. Each option has the Type value of its kind in this table.

### RFC4861-OPT-5

**The Length of an option is its size in units of 8 octets, and the size includes the Type and Length fields.**

> "Length         8-bit unsigned integer.  The length of the option
> (including the type and length fields) in units of
> 8 octets." — §4.6, `rfc4861.txt:1547-1549`

- Strength: description. Class: encoding.
- Check idea: decode captured options. A Source Link-Layer Address option of 8 octets has Length
  1, and a Prefix Information option of 32 octets has Length 4.

### RFC4861-OPT-6

**A node silently discards a Neighbor Discovery packet that contains an option with Length zero.**

> "The value 0 is invalid.  Nodes MUST
> silently discard an ND packet that contains an
> option with length zero." — §4.6, `rfc4861.txt:1549-1551`

- Strength: must. Class: end-to-end.
- Check idea: host A sends host B a Neighbor Solicitation with an option of Length 0. Host B
  sends no Neighbor Advertisement and no ICMPv6 error.

### RFC4861-OPT-7

**A link-layer address option holds an 8-bit Type, an 8-bit Length, and the link-layer address.**

> "|     Type      |    Length     |    Link-Layer Address ..." — §4.6.1, `rfc4861.txt:1558`

- Strength: description. Class: encoding.
- Check idea: decode a Source Link-Layer Address option. One octet of Type and one octet of
  Length come before the link-layer address.

### RFC4861-OPT-8

**The Type of the Source Link-Layer Address option is 1, and the Type of the Target Link-Layer Address option is 2.**

> "Type
> 1 for Source Link-layer Address
> 2 for Target Link-layer Address" — §4.6.1, `rfc4861.txt:1563-1565`

- Strength: description. Class: wire.
- Check idea: capture Neighbor Solicitations, Router Solicitations and Router Advertisements, and
  their link-layer address options have Type 1. In Neighbor Advertisements and Redirects, the
  Type is 2.

### RFC4861-OPT-9

**The Length of a link-layer address option counts 8-octet units, and it is 1 for IEEE 802 addresses.**

> "Length         The length of the option (including the type and
> length fields) in units of 8 octets.  For example,
> the length for IEEE 802 addresses is 1
> [IPv6-ETHER]." — §4.6.1, `rfc4861.txt:1575-1578`

- Strength: description. Class: wire.
- Check idea: on an Ethernet link, capture link-layer address options. Each one has Length 1.

### RFC4861-OPT-10

**The Link-Layer Address field holds the link-layer address of variable length, in the format of the document for the link layer.**

> "Link-Layer Address
> The variable length link-layer address." — §4.6.1, `rfc4861.txt:1580-1581`; "The content and format of this field (including
> byte and bit ordering) is expected to be specified
> in specific documents that describe how IPv6
> operates over different link layers." — `rfc4861.txt:1583-1586`

- Strength: description. Class: encoding.
- Check idea: on an Ethernet link, decode a link-layer address option. The field holds the 48-bit
  MAC address of the node in the order that RFC 2464 specifies.

### RFC4861-OPT-11

**The Source Link-Layer Address option holds the link-layer address of the sender, and it occurs in Neighbor Solicitations, Router Solicitations and Router Advertisements.**

> "The Source Link-Layer Address option contains the
> link-layer address of the sender of the packet.  It
> is used in the Neighbor Solicitation, Router
> Solicitation, and Router Advertisement packets." — §4.6.1, `rfc4861.txt:1590-1593`

- Strength: description. Class: wire.
- Check idea: capture Neighbor Solicitations, Router Solicitations and Router Advertisements with
  this option. The address in the option is the MAC address of the sender.

### RFC4861-OPT-12

**The Target Link-Layer Address option holds the link-layer address of the target, and it occurs in Neighbor Advertisements and Redirects.**

> "The Target Link-Layer Address option contains the
> link-layer address of the target.  It is used in
> Neighbor Advertisement and Redirect packets." — §4.6.1, `rfc4861.txt:1595-1597`

- Strength: description. Class: wire.
- Check idea: capture Neighbor Advertisements and Redirects with this option. The address in the
  option is the MAC address of the target of the message.

### RFC4861-OPT-13

**A node silently ignores a link-layer address option in a Neighbor Discovery message that does not use that option.**

> "These options MUST be silently ignored for other
> Neighbor Discovery messages." — §4.6.1, `rfc4861.txt:1599-1600`

- Strength: must. Class: end-to-end.
- Check idea: host B sends host A a Neighbor Advertisement that also carries a Source Link-Layer
  Address option with a false address. Host A does not use that false address.

### RFC4861-OPT-14

**A Prefix Information option holds Type, Length, Prefix Length, the L and A flags, Reserved1, Valid Lifetime, Preferred Lifetime, Reserved2, and the 128-bit Prefix.**

> "|     Type      |    Length     | Prefix Length |L|A| Reserved1 |" — §4.6.2,
> `rfc4861.txt:1607`; "|                         Valid Lifetime                        |" —
> `rfc4861.txt:1609`; "|                       Preferred Lifetime                      |" —
> `rfc4861.txt:1611`; "|                           Reserved2                           |" —
> `rfc4861.txt:1613`; "+                            Prefix                             +" —
> `rfc4861.txt:1618`

- Strength: description. Class: encoding.
- Check idea: decode a Prefix Information option. The fields are Type, Length, Prefix Length (8
  bits each), L, A (1 bit each), Reserved1 (6 bits), three 32-bit fields, and the 16-octet
  Prefix.

### RFC4861-OPT-15

**The Type of the Prefix Information option is 3.**

> "Type           3" — §4.6.2, `rfc4861.txt:1633`

- Strength: description. Class: wire.
- Check idea: capture the Prefix Information options of a router. Each one has Type 3.

### RFC4861-OPT-16

**The Length of the Prefix Information option is 4.**

> "Length         4" — §4.6.2, `rfc4861.txt:1635`

- Strength: description. Class: wire.
- Check idea: capture the Prefix Information options of a router. Each one has Length 4, which
  is 32 octets.

### RFC4861-OPT-17

**Prefix Length is the number of valid bits at the start of the Prefix, from 0 to 128.**

> "Prefix Length  8-bit unsigned integer.  The number of leading bits
> in the Prefix that are valid.  The value ranges
> from 0 to 128." — §4.6.2, `rfc4861.txt:1637-1639`

- Strength: description. Class: encoding.
- Check idea: a router advertises the prefix 2001:db8:1::/64. Its Prefix Information option has
  Prefix Length 64.

### RFC4861-OPT-18

**A host uses the Prefix Length, together with the L flag, to decide which addresses are on-link.**

> "The prefix length field provides
> necessary information for on-link determination
> (when combined with the L flag in the prefix
> information option)." — §4.6.2, `rfc4861.txt:1639-1642`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises 2001:db8::/48 with L = 1, and a host resolves 2001:db8:0:5::1
  directly. With 2001:db8::/64, the host sends that packet to the router.

### RFC4861-OPT-19

**When the L flag is set, a host can use the prefix for on-link determination.**

> "L              1-bit on-link flag.  When set, indicates that this
> prefix can be used for on-link determination." — §4.6.2, `rfc4861.txt:1647-1648`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises the prefix P with L = 1. A host sends a packet to an address in
  P directly, after it resolves that address with a Neighbor Solicitation.

### RFC4861-OPT-20

**When the L flag is clear, a host does not conclude that the prefix is off-link. It keeps an earlier on-link indication.**

> "When
> not set the advertisement makes no statement about
> on-link or off-link properties of the prefix.  In
> other words, if the L flag is not set a host MUST
> NOT conclude that an address derived from the
> prefix is off-link.  That is, it MUST NOT update a
> previous indication that the address is on-link." — §4.6.2, `rfc4861.txt:1648-1654`

- Strength: must not. Class: end-to-end.
- Check idea: a router advertises the prefix P with L = 1, and later with L = 0. The host still
  sends packets for addresses in P directly to the destinations.

### RFC4861-OPT-21

**When the A flag is set, a host can use the prefix for stateless address autoconfiguration.**

> "A              1-bit autonomous address-configuration flag.  When
> set indicates that this prefix can be used for
> stateless address configuration as specified in
> [ADDRCONF]." — §4.6.2, `rfc4861.txt:1656-1659`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises a /64 prefix with A = 1, and a host forms an address in it and
  runs Duplicate Address Detection. With A = 0, the host forms no address in the prefix.

### RFC4861-OPT-22

**The sender sets the Reserved1 field of a Prefix Information option to zero.**

> "Reserved1      6-bit unused field.  It MUST be initialized to zero
> by the sender" — §4.6.2, `rfc4861.txt:1661-1662`

- Strength: must. Class: wire.
- Check idea: capture the Prefix Information options of a router. The 6 bits after the A flag
  are zero in each one.

### RFC4861-OPT-23

**A receiver ignores the value of the Reserved1 field of a Prefix Information option.**

> "Reserved1      6-bit unused field." — §4.6.2, `rfc4861.txt:1661`; "and MUST be ignored by the receiver." — `rfc4861.txt:1662`

- Strength: must. Class: end-to-end.
- Check idea: a router advertises a prefix with L = 1, A = 1 and nonzero Reserved1 bits. A host
  treats the prefix as on-link and forms an address in it.

### RFC4861-OPT-24

**Valid Lifetime is the time, in seconds from the transmission of the packet, for which the prefix is valid for on-link determination.**

> "Valid Lifetime
> 32-bit unsigned integer.  The length of time in
> seconds (relative to the time the packet is sent)
> that the prefix is valid for the purpose of on-link
> determination." — §4.6.2, `rfc4861.txt:1664-1668`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises the prefix P with L = 1 and Valid Lifetime 30, and later
  Router Advertisements omit P. A host treats P as on-link for 30 s, and after that it sends
  packets for P to the router.

### RFC4861-OPT-25

**A Valid Lifetime of all one bits (0xffffffff) means infinity.**

> "A value of all one bits
> (0xffffffff) represents infinity." — §4.6.2, `rfc4861.txt:1668-1669`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises the prefix P with L = 1 and Valid Lifetime 0xffffffff, and
  later Router Advertisements omit P. A host treats P as on-link with no time limit.

### RFC4861-OPT-26

**Preferred Lifetime is the time, in seconds from the transmission of the packet, for which addresses formed from the prefix stay preferred.**

> "Preferred Lifetime
> 32-bit unsigned integer.  The length of time in
> seconds (relative to the time the packet is sent)
> that addresses generated from the prefix via
> stateless address autoconfiguration remain
> preferred [ADDRCONF]." — §4.6.2, `rfc4861.txt:1672-1677`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises a prefix with A = 1, Preferred Lifetime 20 and Valid Lifetime
  60. After 20 s, a host no longer uses the address from that prefix as source of new
  communication.

### RFC4861-OPT-27

**A Preferred Lifetime of all one bits (0xffffffff) means infinity.**

> "A value of all one bits
> (0xffffffff) represents infinity." — §4.6.2, `rfc4861.txt:1677-1678`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises a prefix with A = 1 and both lifetimes 0xffffffff, and later
  Router Advertisements omit it. The address of a host from that prefix stays preferred with no
  time limit.

### RFC4861-OPT-28

**The Preferred Lifetime of a Prefix Information option is not greater than its Valid Lifetime.**

> "Note that the value of this field MUST NOT exceed
> the Valid Lifetime field to avoid preferring
> addresses that are no longer valid." — §4.6.2, `rfc4861.txt:1687-1689`

- Strength: must not. Class: wire.
- Check idea: capture the Prefix Information options of a router with several configured
  lifetimes. In each option, Preferred Lifetime is less than or equal to Valid Lifetime.

### RFC4861-OPT-29

**The sender sets the Reserved2 field of a Prefix Information option to zero.**

> "Reserved2      This field is unused.  It MUST be initialized to
> zero by the sender" — §4.6.2, `rfc4861.txt:1691-1692`

- Strength: must. Class: wire.
- Check idea: capture the Prefix Information options of a router. The Reserved2 field of each
  one is zero.

### RFC4861-OPT-30

**A receiver ignores the value of the Reserved2 field of a Prefix Information option.**

> "Reserved2      This field is unused." — §4.6.2, `rfc4861.txt:1691`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1692-1693`

- Strength: must. Class: end-to-end.
- Check idea: a router advertises a prefix with L = 1, A = 1 and a nonzero Reserved2 field. A
  host treats the prefix as on-link and forms an address in it.

### RFC4861-OPT-31

**The Prefix field holds an IP address or a prefix of one, and Prefix Length gives the number of its valid first bits.**

> "Prefix         An IP address or a prefix of an IP address.  The
> Prefix Length field contains the number of valid
> leading bits in the prefix." — §4.6.2, `rfc4861.txt:1695-1697`

- Strength: description. Class: encoding.
- Check idea: a router advertises 2001:db8:1::/64. The option holds 2001:db8:1:: in the 16
  octets of Prefix, with Prefix Length 64.

### RFC4861-OPT-32

**The sender sets the bits of the Prefix after the prefix length to zero.**

> "The bits in the prefix
> after the prefix length are reserved and MUST be
> initialized to zero by the sender" — §4.6.2, `rfc4861.txt:1697-1699`

- Strength: must. Class: wire.
- Check idea: a router advertises a /64 prefix. The last 64 bits of the Prefix field are zero.

### RFC4861-OPT-33

**A receiver ignores the bits of the Prefix after the prefix length.**

> "The bits in the prefix
> after the prefix length are reserved and MUST be
> initialized to zero by the sender and ignored by
> the receiver." — §4.6.2, `rfc4861.txt:1697-1700`

- Strength: must. Class: end-to-end.
- Check idea: a router advertises the Prefix 2001:db8:1::abcd with Prefix Length 64 and L = 1. A
  host treats 2001:db8:1::/64 as on-link, as for a Prefix with zero low bits.

### RFC4861-OPT-34

**A router does not send a Prefix Information option for the link-local prefix.**

> "A router SHOULD NOT send a prefix
> option for the link-local prefix" — §4.6.2, `rfc4861.txt:1700-1701`

- Strength: should not. Class: wire.
- Check idea: capture the Router Advertisements of a router. No Prefix Information option holds
  fe80::/64.

### RFC4861-OPT-35

**A host ignores a Prefix Information option for the link-local prefix.**

> "and a host SHOULD
> ignore such a prefix option." — §4.6.2, `rfc4861.txt:1701-1702`

- Strength: should. Class: end-to-end.
- Check idea: a router advertises fe80::/64 with L = 1, A = 1 and Valid Lifetime 10. A host forms
  no new address, and after 10 s it still resolves link-local addresses directly.

### RFC4861-OPT-36

**The Prefix Information option occurs in Router Advertisements, and a node silently ignores it in other messages.**

> "The Prefix Information option
> appears in Router Advertisement packets and MUST be
> silently ignored for other messages." — §4.6.2, `rfc4861.txt:1707-1709`

- Strength: must. Class: end-to-end.
- Check idea: host A receives a Neighbor Advertisement that also carries a Prefix Information
  option with L = 1. Host A processes the advertisement and does not treat the prefix as
  on-link.

### RFC4861-OPT-37

**A Redirected Header option holds Type, Length, 6 octets of Reserved, and then the IP header and data of the redirected packet.**

> "|     Type      |    Length     |            Reserved           |" — §4.6.3,
> `rfc4861.txt:1716`; "|                           Reserved                            |" —
> `rfc4861.txt:1718`; "~                       IP header + data                        ~" —
> `rfc4861.txt:1721`

- Strength: description. Class: encoding.
- Check idea: decode a Redirected Header option. One octet of Type, one octet of Length and 6
  octets of Reserved come before the IP header of the redirected packet.

### RFC4861-OPT-38

**The Type of the Redirected Header option is 4.**

> "Type           4" — §4.6.3, `rfc4861.txt:1727`

- Strength: description. Class: wire.
- Check idea: capture the Redirects of a router. The Redirected Header option in each one has
  Type 4.

### RFC4861-OPT-39

**The Length of the Redirected Header option is its size in units of 8 octets.**

> "Length         The length of the option in units of 8 octets." — §4.6.3, `rfc4861.txt:1729`

- Strength: description. Class: wire.
- Check idea: decode the Redirected Header option of a captured Redirect. Length multiplied by 8
  is equal to the size of the option in octets.

### RFC4861-OPT-40

**The sender sets the Reserved fields of a Redirected Header option to zero.**

> "Reserved       These fields are unused.  They MUST be initialized
> to zero by the sender" — §4.6.3, `rfc4861.txt:1731-1732`

- Strength: must. Class: wire.
- Check idea: capture the Redirects of a router. The 6 Reserved octets of each Redirected Header
  option are zero.

### RFC4861-OPT-41

**A receiver ignores the value of the Reserved fields of a Redirected Header option.**

> "Reserved       These fields are unused." — §4.6.3, `rfc4861.txt:1731`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1732-1733`

- Strength: must. Class: end-to-end.
- Check idea: router R1 sends host A a Redirect to router R2 whose Redirected Header option has
  nonzero Reserved octets. Host A sends its next packet for that destination to R2.

### RFC4861-OPT-42

**The Redirected Header option holds the redirected packet, cut so that the Redirect is not larger than the minimum IPv6 MTU.**

> "IP header + data
> The original packet truncated to ensure that the
> size of the redirect message does not exceed the
> minimum MTU required to support IPv6 as specified
> in [IPv6]." — §4.6.3, `rfc4861.txt:1743-1747`; "The Redirected Header option is used in Redirect
> messages and contains all or part of the packet
> that is being redirected." — `rfc4861.txt:1750-1752`

- Strength: description. Class: wire.
- Check idea: host A sends a 100-octet packet and a 1400-octet packet that each cause a Redirect.
  The first option holds the whole packet, and the second Redirect is 1280 octets or less.

### RFC4861-OPT-43

**A node silently ignores a Redirected Header option in a Neighbor Discovery message other than a Redirect.**

> "This option MUST be silently ignored for other
> Neighbor Discovery messages." — §4.6.3, `rfc4861.txt:1754-1755`

- Strength: must. Class: end-to-end.
- Check idea: host B answers a Neighbor Solicitation of host A with a Neighbor Advertisement that
  also carries a Redirected Header option. Host A completes address resolution as usual.

### RFC4861-OPT-44

**An MTU option holds an 8-bit Type, an 8-bit Length, a 16-bit Reserved field, and a 32-bit MTU.**

> "|     Type      |    Length     |           Reserved            |" — §4.6.4,
> `rfc4861.txt:1762`; "|                              MTU                              |" —
> `rfc4861.txt:1764`

- Strength: description. Class: encoding.
- Check idea: decode an MTU option. It is 8 octets long: Type, Length, 2 octets of Reserved, and
  4 octets of MTU.

### RFC4861-OPT-45

**The Type of the MTU option is 5.**

> "Type           5" — §4.6.4, `rfc4861.txt:1769`

- Strength: description. Class: wire.
- Check idea: capture the MTU options of a router. Each one has Type 5.

### RFC4861-OPT-46

**The Length of the MTU option is 1.**

> "Length         1" — §4.6.4, `rfc4861.txt:1771`

- Strength: description. Class: wire.
- Check idea: capture the MTU options of a router. Each one has Length 1, which is 8 octets.

### RFC4861-OPT-47

**The sender sets the Reserved field of an MTU option to zero.**

> "Reserved       This field is unused.  It MUST be initialized to
> zero by the sender" — §4.6.4, `rfc4861.txt:1773-1774`

- Strength: must. Class: wire.
- Check idea: capture the MTU options of a router. The Reserved field of each one is zero.

### RFC4861-OPT-48

**A receiver ignores the value of the Reserved field of an MTU option.**

> "Reserved       This field is unused." — §4.6.4, `rfc4861.txt:1773`; "and MUST be ignored by the
> receiver." — `rfc4861.txt:1774-1775`

- Strength: must. Class: end-to-end.
- Check idea: a router advertises an MTU option with MTU 1400 and a nonzero Reserved field. A host
  sends no packet larger than 1400 octets on the link.

### RFC4861-OPT-49

**The MTU field holds the recommended MTU of the link, so that all nodes on a link with an uncertain MTU use the same value.**

> "MTU            32-bit unsigned integer.  The recommended MTU for
> the link." — §4.6.4, `rfc4861.txt:1777-1778`; "The MTU option is used in Router Advertisement
> messages to ensure that all nodes on a link use the
> same MTU value in those cases where the link MTU is
> not well known." — `rfc4861.txt:1781-1784`

- Strength: description. Class: end-to-end.
- Check idea: on an Ethernet link, a router advertises an MTU option with MTU 1400. A host sends
  no packet larger than 1400 octets on the link.

### RFC4861-OPT-50

**A node silently ignores an MTU option in a Neighbor Discovery message other than a Router Advertisement.**

> "This option MUST be silently ignored for other
> Neighbor Discovery messages." — §4.6.4, `rfc4861.txt:1786-1787`

- Strength: must. Class: end-to-end.
- Check idea: on an Ethernet link, host A receives a Neighbor Advertisement that also carries an
  MTU option with MTU 1300. Host A keeps its link MTU of 1500.
## Validation of router discovery messages

### RFC4861-RVAL-1

**A host silently discards every Router Solicitation that it receives.**

> "Hosts MUST silently discard any received Router Solicitation
> Messages." — §6.1.1, `rfc4861.txt:2139-2140` and
> "A host MUST silently discard any received Router Solicitation
> messages." — §6.2.6, `rfc4861.txt:2660-2661`

- Strength: must. Class: end-to-end.
- Check idea: host B sends a valid Router Solicitation with a Source Link-Layer Address
  option to host A. Host A sends no Router Advertisement, and a later packet from A to B
  still starts with a Neighbor Solicitation.

### RFC4861-RVAL-2

**A router silently discards a Router Solicitation whose IP Hop Limit is not 255.**

> "A router MUST silently discard any received Router Solicitation
> messages that do not satisfy all of the following validity checks:" — §6.1.1, `rfc4861.txt:2142-2143` and
> "The IP Hop Limit field has a value of 255, i.e., the packet
> could not possibly have been forwarded by a router." — §6.1.1, `rfc4861.txt:2145-2146`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation with Hop Limit 254 to a router. The router
  sends no Router Advertisement in reply. The same message with Hop Limit 255 gets a reply.

### RFC4861-RVAL-3

**A router silently discards a Router Solicitation whose ICMP checksum is not valid.**

> "A router MUST silently discard any received Router Solicitation
> messages that do not satisfy all of the following validity checks:" — §6.1.1, `rfc4861.txt:2142-2143` and
> "ICMP Checksum is valid." — §6.1.1, `rfc4861.txt:2148`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation with a wrong ICMP checksum. The router sends
  no Router Advertisement in reply.

### RFC4861-RVAL-4

**A router silently discards a Router Solicitation whose ICMP Code is not 0.**

> "A router MUST silently discard any received Router Solicitation
> messages that do not satisfy all of the following validity checks:" — §6.1.1, `rfc4861.txt:2142-2143` and
> "ICMP Code is 0." — §6.1.1, `rfc4861.txt:2150`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation with Code 1. The router sends no Router
  Advertisement in reply.

### RFC4861-RVAL-5

**A router silently discards a Router Solicitation whose ICMP length is less than 8 octets.**

> "A router MUST silently discard any received Router Solicitation
> messages that do not satisfy all of the following validity checks:" — §6.1.1, `rfc4861.txt:2142-2143` and
> "ICMP length (derived from the IP length) is 8 or more octets." — §6.1.1, `rfc4861.txt:2152`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation with an ICMP length of 4 octets. The router
  sends no Router Advertisement in reply.

### RFC4861-RVAL-6

**A router silently discards a Router Solicitation that holds an option of length zero.**

> "A router MUST silently discard any received Router Solicitation
> messages that do not satisfy all of the following validity checks:" — §6.1.1, `rfc4861.txt:2142-2143` and
> "All included options have a length that is greater than zero." — §6.1.1, `rfc4861.txt:2154`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation with an option whose Length field is 0. The
  router sends no Router Advertisement in reply.

### RFC4861-RVAL-7

**A router silently discards a Router Solicitation from the unspecified address that holds a Source Link-Layer Address option.**

> "A router MUST silently discard any received Router Solicitation
> messages that do not satisfy all of the following validity checks:" — §6.1.1, `rfc4861.txt:2142-2143` and
> "If the IP source address is the unspecified address, there is no
> source link-layer address option in the message." — §6.1.1, `rfc4861.txt:2156-2157`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation from the unspecified address with a Source
  Link-Layer Address option. The router sends no Router Advertisement in reply.

### RFC4861-RVAL-8

**A router ignores the Reserved field and any unrecognized option of a Router Solicitation.**

> "The contents of the Reserved field, and of any unrecognized options,
> MUST be ignored." — §6.1.1, `rfc4861.txt:2159-2160`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation with a non-zero Reserved field and an option
  of an unknown type. The router replies with a Router Advertisement, as for a normal
  solicitation.

### RFC4861-RVAL-9

**A router ignores a defined option that does not belong in a Router Solicitation, and processes the message as normal.**

> "The contents of any defined options that are not specified to be used
> with Router Solicitation messages MUST be ignored and the packet
> processed as normal.  The only defined option that may appear is the
> Source Link-Layer Address option." — §6.1.1, `rfc4861.txt:2164-2167`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation with a Prefix Information option and an MTU
  option. The router replies as for a normal solicitation, and it takes no information from
  the two options.

### RFC4861-RVAL-10

**A node silently discards a Router Advertisement whose IP source address is not a link-local address.**

> "A node MUST silently discard any received Router Advertisement
> messages that do not satisfy all of the following validity checks:" — §6.1.2, `rfc4861.txt:2174-2175` and
> "IP Source Address is a link-local address." — §6.1.2, `rfc4861.txt:2177`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with a non-zero Router Lifetime and a
  Prefix Information option from a global address. The host does not use that router for
  off-link packets, and it does not treat the prefix as on-link.

### RFC4861-RVAL-11

**A router uses its link-local address as the source address of its Router Advertisements and Redirects.**

> "Routers must use
> their link-local address as the source for Router Advertisement
> and Redirect messages so that hosts can uniquely identify
> routers." — §6.1.2, `rfc4861.txt:2177-2180`

- Strength: must (lower case). Class: wire.
- Check idea: every Router Advertisement and every Redirect that a router sends on a link has
  the link-local address of the router on that link as its IP source address.

### RFC4861-RVAL-12

**A node silently discards a Router Advertisement whose IP Hop Limit is not 255.**

> "A node MUST silently discard any received Router Advertisement
> messages that do not satisfy all of the following validity checks:" — §6.1.2, `rfc4861.txt:2174-2175` and
> "The IP Hop Limit field has a value of 255, i.e., the packet
> could not possibly have been forwarded by a router." — §6.1.2, `rfc4861.txt:2191-2192`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with Hop Limit 254, a non-zero Router
  Lifetime and a Prefix Information option. The host does not use that router, and it does
  not treat the prefix as on-link.

### RFC4861-RVAL-13

**A node silently discards a Router Advertisement whose ICMP checksum is not valid.**

> "A node MUST silently discard any received Router Advertisement
> messages that do not satisfy all of the following validity checks:" — §6.1.2, `rfc4861.txt:2174-2175` and
> "ICMP Checksum is valid." — §6.1.2, `rfc4861.txt:2194`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with a wrong ICMP checksum. The host does
  not use that router and takes no prefix from the message.

### RFC4861-RVAL-14

**A node silently discards a Router Advertisement whose ICMP Code is not 0.**

> "A node MUST silently discard any received Router Advertisement
> messages that do not satisfy all of the following validity checks:" — §6.1.2, `rfc4861.txt:2174-2175` and
> "ICMP Code is 0." — §6.1.2, `rfc4861.txt:2196`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with Code 1. The host does not use that
  router and takes no prefix from the message.

### RFC4861-RVAL-15

**A node silently discards a Router Advertisement whose ICMP length is less than 16 octets.**

> "A node MUST silently discard any received Router Advertisement
> messages that do not satisfy all of the following validity checks:" — §6.1.2, `rfc4861.txt:2174-2175` and
> "ICMP length (derived from the IP length) is 16 or more octets." — §6.1.2, `rfc4861.txt:2198`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with an ICMP length of 12 octets. The host
  does not use that router.

### RFC4861-RVAL-16

**A node silently discards a Router Advertisement that holds an option of length zero.**

> "A node MUST silently discard any received Router Advertisement
> messages that do not satisfy all of the following validity checks:" — §6.1.2, `rfc4861.txt:2174-2175` and
> "All included options have a length that is greater than zero." — §6.1.2, `rfc4861.txt:2200`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with an option whose Length field is 0.
  The host does not use that router and takes no prefix from the message.

### RFC4861-RVAL-17

**A node ignores the Reserved field and any unrecognized option of a Router Advertisement.**

> "The contents of the Reserved field, and of any unrecognized options,
> MUST be ignored." — §6.1.2, `rfc4861.txt:2202-2203`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with a non-zero Reserved field and an
  option of an unknown type. The host uses the router and the prefixes of the message as
  for a normal advertisement.

### RFC4861-RVAL-18

**A node ignores a defined option that does not belong in a Router Advertisement, and processes the message as normal.**

> "The contents of any defined options that are not specified to be used
> with Router Advertisement messages MUST be ignored and the packet
> processed as normal.  The only defined options that may appear are
> the Source Link-Layer Address, Prefix Information and MTU options." — §6.1.2, `rfc4861.txt:2207-2210`

- Strength: must. Class: end-to-end.
- Check idea: a router sends a Router Advertisement with a Target Link-Layer Address option
  and a Redirected Header option. The host uses the router and the prefixes of the message
  as for a normal advertisement.

## Router configuration variables

### RFC4861-RCFG-1

**A router lets system management configure the router variables of each interface.**

> "A router MUST allow for the following conceptual variables to be
> configured by system management." — §6.2.1, `rfc4861.txt:2219-2220`

- Strength: must. Class: internal.
- Check idea: set each variable of the list to a value other than its default on one
  interface. The Router Advertisements of that interface carry the configured values.

### RFC4861-RCFG-2

**A document for a link layer can override some default values, and the limits of the Router Lifetime.**

> "The default values for some of the variables listed below may be
> overridden by specific documents that describe how IPv6 operates over
> different link layers." — §6.2.1, `rfc4861.txt:2226-2228` and
> "These limits may be overridden
> by specific documents that describe how IPv6
> operates over different link layers." — §6.2.1, `rfc4861.txt:2365-2367`

- Strength: may (lower case). Class: internal.
- Check idea: on a link type whose own document gives other defaults, a router without
  explicit configuration sends Router Advertisements with those defaults.

### RFC4861-RCFG-3

**IsRouter tells whether an interface forwards packets, and it is FALSE by default.**

> "IsRouter       A flag indicating whether routing is enabled on
> this interface.  Enabling routing on the interface
> would imply that a router can forward packets to or
> from the interface." — §6.2.1, `rfc4861.txt:2249-2252` and
> "Default: FALSE" — §6.2.1, `rfc4861.txt:2254`

- Strength: description. Class: internal.
- Check idea: a node without configuration forwards no packet from one interface to another,
  and its Neighbor Advertisements have the Router flag zero.

### RFC4861-RCFG-4

**AdvSendAdvertisements controls whether a router sends Router Advertisements and answers Router Solicitations, and it is FALSE by default.**

> "AdvSendAdvertisements
> A flag indicating whether or not the router sends
> periodic Router Advertisements and responds to
> Router Solicitations." — §6.2.1, `rfc4861.txt:2256-2259`;
> "Default: FALSE" — §6.2.1, `rfc4861.txt:2261` and
> "Note that AdvSendAdvertisements MUST be FALSE by
> default so that a node will not accidentally start
> acting as a router unless it is explicitly
> configured by system management to send Router
> Advertisements." — §6.2.1, `rfc4861.txt:2263-2267`

- Strength: must. Class: wire.
- Check idea: a node that forwards packets but has no configured value of AdvSendAdvertisements sends
  no Router Advertisement, periodic or in reply to a Router Solicitation.

### RFC4861-RCFG-5

**MaxRtrAdvInterval is the longest time between two unsolicited multicast Router Advertisements; it is 4 to 1800 seconds, and 600 seconds by default.**

> "MaxRtrAdvInterval
> The maximum time allowed between sending
> unsolicited multicast Router Advertisements from
> the interface, in seconds.  MUST be no less than 4
> seconds and no greater than 1800 seconds." — §6.2.1, `rfc4861.txt:2269-2273` and
> "Default: 600 seconds" — §6.2.1, `rfc4861.txt:2275`

- Strength: must. Class: wire.
- Check idea: a router with the default value never leaves more than 600 seconds between two
  unsolicited Router Advertisements. A router refuses a value of 3 or of 1801 seconds.

### RFC4861-RCFG-6

**MinRtrAdvInterval is the shortest time between two unsolicited multicast Router Advertisements; it is 3 seconds to 0.75 × MaxRtrAdvInterval, with a default that depends on MaxRtrAdvInterval.**

> "MinRtrAdvInterval
> The minimum time allowed between sending
> unsolicited multicast Router Advertisements from
> the interface, in seconds.  MUST be no less than 3
> seconds and no greater than .75 *
> MaxRtrAdvInterval." — §6.2.1, `rfc4861.txt:2277-2282` and
> "Default: 0.33 * MaxRtrAdvInterval If
> MaxRtrAdvInterval >= 9 seconds; otherwise, the
> Default is MaxRtrAdvInterval." — §6.2.1, `rfc4861.txt:2284-2286`

- Strength: must. Class: wire.
- Check idea: with MaxRtrAdvInterval 600 and no other configured value, no two unsolicited Router
  Advertisements are closer than 198 seconds. A router refuses a value of 2 seconds, or a
  value above 0.75 × MaxRtrAdvInterval.

### RFC4861-RCFG-7

**AdvManagedFlag is the value of the M flag of the Router Advertisement, and it is FALSE by default.**

> "AdvManagedFlag
> The TRUE/FALSE value to be placed in the "Managed
> address configuration" flag field in the Router
> Advertisement." — §6.2.1, `rfc4861.txt:2288-2291` and
> "Default: FALSE" — §6.2.1, `rfc4861.txt:2293`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends Router Advertisements with the M flag
  zero. With AdvManagedFlag TRUE, the M flag is one.

### RFC4861-RCFG-8

**AdvOtherConfigFlag is the value of the O flag of the Router Advertisement, and it is FALSE by default.**

> "AdvOtherConfigFlag
> The TRUE/FALSE value to be placed in the "Other
> configuration" flag field in the Router
> Advertisement." — §6.2.1, `rfc4861.txt:2303-2306` and
> "Default: FALSE" — §6.2.1, `rfc4861.txt:2308`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends Router Advertisements with the O flag
  zero. With AdvOtherConfigFlag TRUE, the O flag is one.

### RFC4861-RCFG-9

**AdvLinkMTU is the value of the MTU option; zero, the default, means that no MTU option is sent.**

> "AdvLinkMTU     The value to be placed in MTU options sent by the
> router.  A value of zero indicates that no MTU
> options are sent." — §6.2.1, `rfc4861.txt:2310-2312` and
> "Default: 0" — §6.2.1, `rfc4861.txt:2314`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends Router Advertisements without an MTU
  option. With AdvLinkMTU 1400, each advertisement carries an MTU option of 1400.

### RFC4861-RCFG-10

**AdvReachableTime is the value of the Reachable Time field; it is at most 3,600,000 milliseconds, and zero, the default, means unspecified.**

> "AdvReachableTime
> The value to be placed in the Reachable Time field
> in the Router Advertisement messages sent by the
> router.  The value zero means unspecified (by this
> router).  MUST be no greater than 3,600,000
> milliseconds (1 hour)." — §6.2.1, `rfc4861.txt:2316-2321` and
> "Default: 0" — §6.2.1, `rfc4861.txt:2323`

- Strength: must. Class: wire.
- Check idea: a router with the default value sends Router Advertisements with Reachable Time
  0. A router refuses a value of 3,600,001 milliseconds.

### RFC4861-RCFG-11

**AdvRetransTimer is the value of the Retrans Timer field; zero, the default, means unspecified.**

> "AdvRetransTimer The value to be placed in the Retrans Timer field
> in the Router Advertisement messages sent by the
> router.  The value zero means unspecified (by this
> router)." — §6.2.1, `rfc4861.txt:2325-2328` and
> "Default: 0" — §6.2.1, `rfc4861.txt:2330`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends Router Advertisements with Retrans Timer
  0. With AdvRetransTimer 2000, the field is 2000.

### RFC4861-RCFG-12

**AdvCurHopLimit is the value of the Cur Hop Limit field; its default is the value of the Assigned Numbers registry, and zero means unspecified.**

> "AdvCurHopLimit
> The default value to be placed in the Cur Hop Limit
> field in the Router Advertisement messages sent by
> the router.  The value should be set to the current
> diameter of the Internet.  The value zero means
> unspecified (by this router)." — §6.2.1, `rfc4861.txt:2332-2337` and
> "Default:  The value specified in the "Assigned
> Numbers" [ASSIGNED] that was in effect at the time
> of implementation." — §6.2.1, `rfc4861.txt:2339-2341`

- Strength: should (lower case). Class: wire.
- Check idea: a router with the default value sends Router Advertisements whose Cur Hop Limit
  is the default hop limit of the Assigned Numbers registry. With AdvCurHopLimit 100, the
  field is 100.

### RFC4861-RCFG-13

**AdvDefaultLifetime is the value of the Router Lifetime field; it is zero or from MaxRtrAdvInterval to 9000 seconds, and 3 × MaxRtrAdvInterval by default.**

> "AdvDefaultLifetime
> The value to be placed in the Router Lifetime field
> of Router Advertisements sent from the interface,
> in seconds.  MUST be either zero or between
> MaxRtrAdvInterval and 9000 seconds.  A value of
> zero indicates that the router is not to be used as
> a default router." — §6.2.1, `rfc4861.txt:2359-2365` and
> "Default: 3 * MaxRtrAdvInterval" — §6.2.1, `rfc4861.txt:2373`

- Strength: must. Class: wire.
- Check idea: a router with default values sends Router Advertisements with Router Lifetime
  1800. A router refuses a value of 9001 seconds, or a non-zero value below
  MaxRtrAdvInterval.

### RFC4861-RCFG-14

**AdvPrefixList holds the prefixes of the Prefix Information options; by default it holds every prefix that is on-link for the interface.**

> "AdvPrefixList
> A list of prefixes to be placed in Prefix
> Information options in Router Advertisement
> messages sent from the interface." — §6.2.1, `rfc4861.txt:2375-2378` and
> "Default: all prefixes that the router advertises
> via routing protocols as being on-link for the
> interface from which the advertisement is sent." — §6.2.1, `rfc4861.txt:2380-2382`

- Strength: description. Class: wire.
- Check idea: a router with two global prefixes on an interface and no prefix configuration
  sends Router Advertisements with one Prefix Information option for each of the two
  prefixes.

### RFC4861-RCFG-15

**A router does not advertise the link-local prefix.**

> "The link-local prefix SHOULD NOT be included in the
> list of advertised prefixes." — §6.2.1, `rfc4861.txt:2383-2384`

- Strength: should not. Class: wire.
- Check idea: no Router Advertisement of the router carries a Prefix Information option for
  the prefix fe80::/64.

### RFC4861-RCFG-16

**AdvValidLifetime is the value of the Valid Lifetime field; all ones means infinity, and the default is a fixed 2592000 seconds.**

> "AdvValidLifetime
> The value to be placed in the Valid
> Lifetime in the Prefix Information option,
> in seconds.  The designated value of all
> 1's (0xffffffff) represents infinity." — §6.2.1, `rfc4861.txt:2388-2392` and
> "Default: 2592000 seconds (30 days), fixed
> (i.e., stays the same in consecutive
> advertisements)." — §6.2.1, `rfc4861.txt:2404-2406`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends the Valid Lifetime 2592000 in every
  Router Advertisement. A configured infinite lifetime appears as 0xffffffff.

### RFC4861-RCFG-17

**An implementation may let AdvValidLifetime decrement in real time, or keep it fixed.**

> "Implementations MAY allow AdvValidLifetime
> to be specified in two ways:" — §6.2.1, `rfc4861.txt:2393-2394`;
> "a time that decrements in real time,
> that is, one that will result in a
> Lifetime of zero at the specified time
> in the future, or" — §6.2.1, `rfc4861.txt:2396-2399` and
> "a fixed time that stays the same in
> consecutive advertisements." — §6.2.1, `rfc4861.txt:2401-2402`

- Strength: may. Class: wire.
- Check idea: with a lifetime that decrements in real time, the Valid Lifetime of consecutive Router
  Advertisements drops by the time between them and reaches zero at the configured time.

### RFC4861-RCFG-18

**AdvOnLinkFlag is the value of the L flag of the Prefix Information option, and it is TRUE by default.**

> "AdvOnLinkFlag
> The value to be placed in the on-link flag
> ("L-bit") field in the Prefix Information
> option." — §6.2.1, `rfc4861.txt:2415-2418` and
> "Default: TRUE" — §6.2.1, `rfc4861.txt:2420`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends Prefix Information options with the L
  flag set.

### RFC4861-RCFG-19

**AdvPreferredLifetime is the value of the Preferred Lifetime field; all ones means infinity, and the default is a fixed 604800 seconds.**

> "AdvPreferredLifetime
> The value to be placed in the Preferred
> Lifetime in the Prefix Information option,
> in seconds.  The designated value of all
> 1's (0xffffffff) represents infinity." — §6.2.1, `rfc4861.txt:2426-2430` and
> "Default: 604800 seconds (7 days), fixed
> (i.e., stays the same in consecutive
> advertisements)." — §6.2.1, `rfc4861.txt:2444-2446`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends the Preferred Lifetime 604800 in every
  Router Advertisement. A configured infinite lifetime appears as 0xffffffff.

### RFC4861-RCFG-20

**An implementation may let AdvPreferredLifetime decrement in real time, or keep it fixed.**

> "Implementations MAY allow
> AdvPreferredLifetime to be specified in two
> ways:" — §6.2.1, `rfc4861.txt:2432-2434`;
> "a time that decrements in real time,
> that is, one that will result in a
> Lifetime of zero at a specified time in
> the future, or" — §6.2.1, `rfc4861.txt:2436-2439` and
> "a fixed time that stays the same in
> consecutive advertisements." — §6.2.1, `rfc4861.txt:2441-2442`

- Strength: may. Class: wire.
- Check idea: with a lifetime that decrements in real time, the Preferred Lifetime of consecutive Router
  Advertisements drops by the time between them and reaches zero at the configured time.

### RFC4861-RCFG-21

**The preferred lifetime of a prefix is not larger than its valid lifetime.**

> "This value MUST NOT be
> larger than AdvValidLifetime." — §6.2.1, `rfc4861.txt:2446-2447`

- Strength: must not. Class: wire.
- Check idea: no Prefix Information option of a router has a Preferred Lifetime above its
  Valid Lifetime. A router refuses a configuration with a larger preferred lifetime.

### RFC4861-RCFG-22

**AdvAutonomousFlag is the value of the A flag of the Prefix Information option, and it is TRUE by default.**

> "AdvAutonomousFlag
> The value to be placed in the Autonomous
> Flag field in the Prefix Information
> option." — §6.2.1, `rfc4861.txt:2449-2452` and
> "Default: TRUE" — §6.2.1, `rfc4861.txt:2454`

- Strength: description. Class: wire.
- Check idea: a router with the default value sends Prefix Information options with the A
  flag set.

### RFC4861-RCFG-23

**A router behaves like a host for CurHopLimit, RetransTimer and ReachableTime, with the random ReachableTime included.**

> "Some of these host variables (e.g.,
> CurHopLimit, RetransTimer, and ReachableTime) apply to all nodes
> including routers." — §6.2.1, `rfc4861.txt:2459-2461` and
> "However, external router behavior MUST be
> the same as host behavior with respect to these variables.  In
> particular, this includes the occasional randomization of the
> ReachableTime value as described in Section 6.3.2." — §6.2.1, `rfc4861.txt:2471-2474`

- Strength: must. Class: end-to-end.
- Check idea: the packets that a router originates carry its CurHopLimit. Its Neighbor
  Solicitation retransmissions are RetransTimer apart. The time that its neighbor entries
  stay reachable varies between MIN_RANDOM_FACTOR and MAX_RANDOM_FACTOR times the base value.

## Router behavior

### RFC4861-ADV-1

**A router sends Router Advertisements only on an advertising interface: one that works, is enabled, has a unicast address, and has AdvSendAdvertisements TRUE.**

> "The term "advertising interface" refers to any functioning and
> enabled interface that has at least one unicast IP address assigned
> to it and whose corresponding AdvSendAdvertisements flag is TRUE.  A
> router MUST NOT send Router Advertisements out any interface that is
> not an advertising interface." — §6.2.2, `rfc4861.txt:2480-2484`

- Strength: must not. Class: wire.
- Check idea: a router has two interfaces; one has AdvSendAdvertisements FALSE. No Router
  Advertisement appears on the link of that interface, periodic or in reply to a Router
  Solicitation.

### RFC4861-ADV-2

**An interface also becomes an advertising interface after startup: when its flag becomes TRUE, when it is enabled, or when the node starts to forward.**

> "An interface may become an advertising interface at times other than
> system startup.  For example:" — §6.2.2, `rfc4861.txt:2486-2487`;
> "changing the AdvSendAdvertisements flag on an enabled interface
> from FALSE to TRUE, or" — §6.2.2, `rfc4861.txt:2489-2490`;
> "administratively enabling the interface, if it had been
> administratively disabled, and its AdvSendAdvertisements flag is
> TRUE, or" — §6.2.2, `rfc4861.txt:2492-2494` and
> "enabling IP forwarding capability (i.e., changing the system
> from being a host to being a router), when the interface's
> AdvSendAdvertisements flag is TRUE." — §6.2.2, `rfc4861.txt:2496-2498`

- Strength: may (lower case). Class: wire.
- Check idea: change the AdvSendAdvertisements flag of an enabled interface from FALSE to
  TRUE at run time. The router starts to send Router Advertisements on that link, with the
  initial short intervals.

### RFC4861-ADV-3

**A router joins the all-routers multicast address on each advertising interface.**

> "A router MUST join the all-routers multicast address on an
> advertising interface." — §6.2.2, `rfc4861.txt:2500-2501`

- Strength: must. Class: end-to-end.
- Check idea: a host sends a Router Solicitation to the all-routers multicast address. The
  router receives it and replies with a Router Advertisement.

### RFC4861-ADV-4

**A router sends periodic and solicited Router Advertisements on its advertising interfaces.**

> "A router sends periodic as well as solicited Router Advertisements
> out its advertising interfaces." — §6.2.3, `rfc4861.txt:2507-2508`

- Strength: description. Class: wire.
- Check idea: a router with one advertising interface sends Router Advertisements at
  intervals, and one more in reply to a Router Solicitation.

### RFC4861-ADV-5

**The Router Lifetime field of a Router Advertisement carries AdvDefaultLifetime.**

> "Outgoing Router Advertisements are
> filled with the following values consistent with the message format
> given in Section 4.2:" — §6.2.3, `rfc4861.txt:2508-2510` and
> "In the Router Lifetime field: the interface's configured
> AdvDefaultLifetime." — §6.2.3, `rfc4861.txt:2512-2513`

- Strength: description. Class: wire.
- Check idea: with AdvDefaultLifetime 1000, every Router Advertisement of the interface has
  Router Lifetime 1000.

### RFC4861-ADV-6

**The M and O flags of a Router Advertisement carry AdvManagedFlag and AdvOtherConfigFlag.**

> "Outgoing Router Advertisements are
> filled with the following values consistent with the message format
> given in Section 4.2:" — §6.2.3, `rfc4861.txt:2508-2510` and
> "In the M and O flags: the interface's configured AdvManagedFlag
> and AdvOtherConfigFlag, respectively." — §6.2.3, `rfc4861.txt:2515-2516`

- Strength: description. Class: wire.
- Check idea: with AdvManagedFlag TRUE and AdvOtherConfigFlag FALSE, every Router
  Advertisement has the M flag set and the O flag clear; then swap the two values.

### RFC4861-ADV-7

**The Cur Hop Limit field of a Router Advertisement carries the configured hop limit of the interface.**

> "Outgoing Router Advertisements are
> filled with the following values consistent with the message format
> given in Section 4.2:" — §6.2.3, `rfc4861.txt:2508-2510` and
> "In the Cur Hop Limit field: the interface's configured
> CurHopLimit." — §6.2.3, `rfc4861.txt:2527-2528`

- Strength: description. Class: wire.
- Check idea: with a configured hop limit of 100, every Router Advertisement has Cur Hop
  Limit 100.

### RFC4861-ADV-8

**The Reachable Time field of a Router Advertisement carries AdvReachableTime.**

> "Outgoing Router Advertisements are
> filled with the following values consistent with the message format
> given in Section 4.2:" — §6.2.3, `rfc4861.txt:2508-2510` and
> "In the Reachable Time field: the interface's configured
> AdvReachableTime." — §6.2.3, `rfc4861.txt:2530-2531`

- Strength: description. Class: wire.
- Check idea: with AdvReachableTime 20000, every Router Advertisement has Reachable Time
  20000.

### RFC4861-ADV-9

**The Retrans Timer field of a Router Advertisement carries AdvRetransTimer.**

> "Outgoing Router Advertisements are
> filled with the following values consistent with the message format
> given in Section 4.2:" — §6.2.3, `rfc4861.txt:2508-2510` and
> "In the Retrans Timer field: the interface's configured
> AdvRetransTimer." — §6.2.3, `rfc4861.txt:2533-2534`

- Strength: description. Class: wire.
- Check idea: with AdvRetransTimer 2000, every Router Advertisement has Retrans Timer 2000.

### RFC4861-ADV-10

**The Source Link-Layer Address option of a Router Advertisement carries the link-layer address of the interface, and a router may leave the option out.**

> "Source Link-Layer Address option: link-layer address of the
> sending interface.  This option MAY be omitted to
> facilitate in-bound load balancing over replicated
> interfaces." — §6.2.3, `rfc4861.txt:2538-2541`

- Strength: description (the option content), may (the omission). Class: wire.
- Check idea: when a Router Advertisement carries a Source Link-Layer Address option, the
  option holds the link-layer address of the interface that sends it.

### RFC4861-ADV-11

**A Router Advertisement carries an MTU option with AdvLinkMTU when AdvLinkMTU is not zero, and no MTU option when it is zero.**

> "MTU option: the interface's configured AdvLinkMTU value if
> the value is non-zero.  If AdvLinkMTU is zero, the MTU
> option is not sent." — §6.2.3, `rfc4861.txt:2543-2545`

- Strength: description. Class: wire.
- Check idea: with AdvLinkMTU 1400, every Router Advertisement carries an MTU option of 1400.
  With AdvLinkMTU 0, no Router Advertisement carries an MTU option.

### RFC4861-ADV-12

**A Router Advertisement carries one Prefix Information option for each prefix of AdvPrefixList.**

> "Prefix Information options: one Prefix Information option
> for each prefix listed in AdvPrefixList with the option
> fields set from the information in the AdvPrefixList entry
> as follows:" — §6.2.3, `rfc4861.txt:2547-2550`

- Strength: description. Class: wire.
- Check idea: with three prefixes in AdvPrefixList, a Router Advertisement carries exactly
  three Prefix Information options, one for each prefix.

### RFC4861-ADV-13

**The L flag of a Prefix Information option carries AdvOnLinkFlag of the prefix.**

> "Prefix Information options: one Prefix Information option
> for each prefix listed in AdvPrefixList with the option
> fields set from the information in the AdvPrefixList entry
> as follows:" — §6.2.3, `rfc4861.txt:2547-2550` and
> "In the "on-link" flag: the entry's AdvOnLinkFlag." — §6.2.3, `rfc4861.txt:2552`

- Strength: description. Class: wire.
- Check idea: one prefix has AdvOnLinkFlag TRUE and one has FALSE. The L flags of the two
  Prefix Information options are set and clear.

### RFC4861-ADV-14

**The Valid Lifetime field of a Prefix Information option carries AdvValidLifetime of the prefix.**

> "Prefix Information options: one Prefix Information option
> for each prefix listed in AdvPrefixList with the option
> fields set from the information in the AdvPrefixList entry
> as follows:" — §6.2.3, `rfc4861.txt:2547-2550` and
> "In the Valid Lifetime field: the entry's
> AdvValidLifetime." — §6.2.3, `rfc4861.txt:2554-2555`

- Strength: description. Class: wire.
- Check idea: with a fixed AdvValidLifetime of 7200 for a prefix, its Prefix Information
  option has Valid Lifetime 7200.

### RFC4861-ADV-15

**The A flag of a Prefix Information option carries AdvAutonomousFlag of the prefix.**

> "Prefix Information options: one Prefix Information option
> for each prefix listed in AdvPrefixList with the option
> fields set from the information in the AdvPrefixList entry
> as follows:" — §6.2.3, `rfc4861.txt:2547-2550` and
> "In the "Autonomous address configuration" flag: the
> entry's AdvAutonomousFlag." — §6.2.3, `rfc4861.txt:2557-2558`

- Strength: description. Class: wire.
- Check idea: one prefix has AdvAutonomousFlag TRUE and one has FALSE. The A flags of the
  two Prefix Information options are set and clear.

### RFC4861-ADV-16

**The Preferred Lifetime field of a Prefix Information option carries AdvPreferredLifetime of the prefix.**

> "Prefix Information options: one Prefix Information option
> for each prefix listed in AdvPrefixList with the option
> fields set from the information in the AdvPrefixList entry
> as follows:" — §6.2.3, `rfc4861.txt:2547-2550` and
> "In the Preferred Lifetime field: the entry's
> AdvPreferredLifetime." — §6.2.3, `rfc4861.txt:2560-2561`

- Strength: description. Class: wire.
- Check idea: with a fixed AdvPreferredLifetime of 3600 for a prefix, its Prefix Information
  option has Preferred Lifetime 3600.

### RFC4861-ADV-17

**A router that advertises but does not want to be a default router sends Router Lifetime zero.**

> "A router might want to send Router Advertisements without advertising
> itself as a default router.  For instance, a router might advertise
> prefixes for stateless address autoconfiguration while not wishing to
> forward packets.  Such a router sets the Router Lifetime field in
> outgoing advertisements to zero." — §6.2.3, `rfc4861.txt:2563-2567`

- Strength: description. Class: wire.
- Check idea: a router with AdvDefaultLifetime 0 and one prefix sends Router Advertisements
  with Router Lifetime 0 and the Prefix Information option.

### RFC4861-ADV-18

**A router may leave out some or all options in unsolicited Router Advertisements.**

> "A router MAY choose not to include some or all options when sending
> unsolicited Router Advertisements." — §6.2.3, `rfc4861.txt:2569-2570`

- Strength: may. Class: wire.
- Check idea: a host accepts unsolicited Router Advertisements that omit the prefix options,
  and it keeps the prefixes that it learned before.

### RFC4861-ADV-19

**A router includes all options in a solicited Router Advertisement and in the first few unsolicited ones.**

> "However, when responding to a
> Router Solicitation or while sending the first few initial
> unsolicited advertisements, a router SHOULD include all options so
> that all information (e.g., prefixes) is propagated quickly during
> system initialization." — §6.2.3, `rfc4861.txt:2572-2574`, `rfc4861.txt:2583-2585`

- Strength: should. Class: wire.
- Check idea: the reply to a Router Solicitation and the first initial unsolicited Router
  Advertisements carry every configured option: the Source Link-Layer Address, the MTU and
  all prefixes.

### RFC4861-ADV-20

**When all options do not fit into the link MTU, a router can split them over several Router Advertisements.**

> "If including all options causes the size of an advertisement to
> exceed the link MTU, multiple advertisements can be sent, each
> containing a subset of the options." — §6.2.3, `rfc4861.txt:2587-2589`

- Strength: description. Class: wire.
- Check idea: a router has more prefixes than one advertisement can hold within the link MTU.
  No Router Advertisement exceeds the link MTU, and together the advertisements carry all
  prefixes.

### RFC4861-ADV-21

**A host never sends a Router Advertisement.**

> "A host MUST NOT send Router Advertisement messages at any time." — §6.2.4, `rfc4861.txt:2593`

- Strength: must not. Class: wire.
- Check idea: a host runs on a link for a long time and receives Router Solicitations. It
  sends no Router Advertisement.

### RFC4861-ADV-22

**Each advertising interface has its own timer: after each multicast Router Advertisement, the next one comes after a uniform random time between MinRtrAdvInterval and MaxRtrAdvInterval.**

> "Unsolicited Router Advertisements are not strictly periodic: the
> interval between subsequent transmissions is randomized to reduce the
> probability of synchronization with the advertisements from other
> routers on the same link [SYNC].  Each advertising interface has its
> own timer.  Whenever a multicast advertisement is sent from an
> interface, the timer is reset to a uniformly distributed random value
> between the interface's configured MinRtrAdvInterval and
> MaxRtrAdvInterval; expiration of the timer causes the next
> advertisement to be sent and a new random value to be chosen." — §6.2.4, `rfc4861.txt:2595-2603`

- Strength: description. Class: wire.
- Check idea: record many unsolicited Router Advertisements after the initial ones. Each
  interval lies between MinRtrAdvInterval and MaxRtrAdvInterval, and the intervals vary.
  Two interfaces of one router have independent intervals.

### RFC4861-ADV-23

**For the first few Router Advertisements of a new advertising interface, the interval is at most MAX_INITIAL_RTR_ADVERT_INTERVAL.**

> "For the first few advertisements (up to
> MAX_INITIAL_RTR_ADVERTISEMENTS) sent from an interface when it
> becomes an advertising interface, if the randomly chosen interval is
> greater than MAX_INITIAL_RTR_ADVERT_INTERVAL, the timer SHOULD be set
> to MAX_INITIAL_RTR_ADVERT_INTERVAL instead." — §6.2.4, `rfc4861.txt:2605-2609`

- Strength: should. Class: wire.
- Check idea: with the default intervals, an interface becomes an advertising interface. The
  first MAX_INITIAL_RTR_ADVERTISEMENTS advertisements come at most
  MAX_INITIAL_RTR_ADVERT_INTERVAL apart; the later ones follow the normal intervals.

### RFC4861-ADV-24

**When system management changes the advertised information, a router may send up to MAX_INITIAL_RTR_ADVERTISEMENTS advertisements at the initial rate.**

> "The information contained in Router Advertisements may change through
> actions of system management.  For instance, the lifetime of
> advertised prefixes may change, new prefixes could be added, a router
> could cease to be a router (i.e., switch from being a router to being
> a host), etc.  In such cases, the router MAY transmit up to
> MAX_INITIAL_RTR_ADVERTISEMENTS unsolicited advertisements, using the
> same rules as when an interface becomes an advertising interface." — §6.2.4, `rfc4861.txt:2614-2620`

- Strength: may. Class: wire.
- Check idea: add a prefix to a router at run time. The router sends at most
  MAX_INITIAL_RTR_ADVERTISEMENTS advertisements at intervals of at most
  MAX_INITIAL_RTR_ADVERT_INTERVAL, then returns to the normal intervals.

### RFC4861-ADV-25

**When an interface stops to be an advertising interface, the router sends one to MAX_FINAL_RTR_ADVERTISEMENTS final multicast Router Advertisements with Router Lifetime zero.**

> "An interface may cease to be an advertising interface, through
> actions of system management such as:" — §6.2.5, `rfc4861.txt:2624-2625`;
> "changing the AdvSendAdvertisements flag of an enabled interface
> from TRUE to FALSE, or" — §6.2.5, `rfc4861.txt:2627-2628`;
> "administratively disabling the interface, or" — §6.2.5, `rfc4861.txt:2630`;
> "shutting down the system." — §6.2.5, `rfc4861.txt:2639` and
> "In such cases, the router SHOULD transmit one or more (but not more
> than MAX_FINAL_RTR_ADVERTISEMENTS) final multicast Router
> Advertisements on the interface with a Router Lifetime field of zero." — §6.2.5, `rfc4861.txt:2641-2643`

- Strength: should. Class: wire.
- Check idea: change AdvSendAdvertisements of an advertising interface to FALSE. The router
  sends one to MAX_FINAL_RTR_ADVERTISEMENTS multicast Router Advertisements with Router
  Lifetime 0, then no more.

### RFC4861-ADV-26

**A router that becomes a host leaves the all-routers multicast group on all its multicast interfaces.**

> "In the case of a router becoming a host, the system SHOULD also
> depart from the all-routers IP multicast group on all interfaces on
> which the router supports IP multicast (whether or not they had been
> advertising interfaces)." — §6.2.5, `rfc4861.txt:2644-2647`

- Strength: should. Class: wire.
- Check idea: turn off IP forwarding on a router with two interfaces; only one of them is an advertising interface.
  The node reports on both links that it leaves the all-routers group.

### RFC4861-ADV-27

**A router that becomes a host sends its later Neighbor Advertisements with the Router flag zero.**

> "In addition, the host MUST ensure that
> subsequent Neighbor Advertisement messages sent from the interface
> have the Router flag set to zero." — §6.2.5, `rfc4861.txt:2647-2649`

- Strength: must. Class: wire.
- Check idea: turn off IP forwarding on a router. Each Neighbor Advertisement that the node
  sends afterwards has the Router flag zero.

### RFC4861-ADV-28

**When forwarding is off but the interface still advertises, the Router Advertisements carry Router Lifetime zero.**

> "Note that system management may disable a router's IP forwarding
> capability (i.e., changing the system from being a router to being a
> host), a step that does not necessarily imply that the router's
> interfaces stop being advertising interfaces.  In such cases,
> subsequent Router Advertisements MUST set the Router Lifetime field
> to zero." — §6.2.5, `rfc4861.txt:2651-2656`

- Strength: must. Class: wire.
- Check idea: turn off IP forwarding, and keep AdvSendAdvertisements TRUE. Every later Router
  Advertisement of the node has Router Lifetime 0.

### RFC4861-ADV-29

**A router sends a Router Advertisement in reply to a valid Router Solicitation that arrives on an advertising interface.**

> "In addition to sending periodic, unsolicited advertisements, a router
> sends advertisements in response to valid solicitations received on
> an advertising interface." — §6.2.6, `rfc4861.txt:2663-2665`

- Strength: description. Class: wire.
- Check idea: a host sends a valid Router Solicitation. The router sends a Router
  Advertisement within MAX_RA_DELAY_TIME, before its next periodic one is due.

### RFC4861-ADV-30

**A router may unicast its reply to the address of the host, but usually it multicasts the reply to all nodes.**

> "A router MAY choose to unicast the
> response directly to the soliciting host's address (if the
> solicitation's source address is not the unspecified address), but
> the usual case is to multicast the response to the all-nodes group." — §6.2.6, `rfc4861.txt:2665-2668`

- Strength: may. Class: wire.
- Check idea: the reply to a Router Solicitation from a unicast address goes to that address
  or to the all-nodes address. The reply to a solicitation from the unspecified address goes
  to the all-nodes address.

### RFC4861-ADV-31

**A multicast reply restarts the interval timer with a new random value, as an unsolicited advertisement does.**

> "In the latter case, the interface's interval timer is reset to a new
> random value, as if an unsolicited advertisement had just been sent
> (see Section 6.2.4)." — §6.2.6, `rfc4861.txt:2669-2671`

- Strength: description. Class: wire.
- Check idea: after a multicast reply to a Router Solicitation, the next unsolicited Router
  Advertisement comes between MinRtrAdvInterval and MaxRtrAdvInterval after the reply.

### RFC4861-ADV-32

**A router delays each solicited Router Advertisement by a random time from 0 to MAX_RA_DELAY_TIME, counted from the first solicitation.**

> "In all cases, Router Advertisements sent in response to a Router
> Solicitation MUST be delayed by a random time between 0 and
> MAX_RA_DELAY_TIME seconds. (If a single advertisement is sent in
> response to multiple solicitations, the delay is relative to the
> first solicitation.)" — §6.2.6, `rfc4861.txt:2673-2677`

- Strength: must. Class: wire.
- Check idea: send many Router Solicitations, far apart. Each reply comes between 0 and
  MAX_RA_DELAY_TIME after its solicitation, and the delays vary.

### RFC4861-ADV-33

**A router sends at most one Router Advertisement to the all-nodes address every MIN_DELAY_BETWEEN_RAS seconds.**

> "In addition, consecutive Router Advertisements
> sent to the all-nodes multicast address MUST be rate limited to no
> more than one advertisement every MIN_DELAY_BETWEEN_RAS seconds." — §6.2.6, `rfc4861.txt:2677-2679`

- Strength: must. Class: wire.
- Check idea: several hosts send Router Solicitations in quick succession. No two multicast
  Router Advertisements of the router are closer than MIN_DELAY_BETWEEN_RAS.

### RFC4861-ADV-34

**When the random reply delay ends after the next scheduled multicast advertisement, the router sends the reply at the scheduled time.**

> "A router might process Router Solicitations as follows:" — §6.2.6, `rfc4861.txt:2695` and
> "Upon receipt of a Router Solicitation, compute a random delay
> within the range 0 through MAX_RA_DELAY_TIME.  If the computed
> value corresponds to a time later than the time the next multicast
> Router Advertisement is scheduled to be sent, ignore the random
> delay and send the advertisement at the already-scheduled time." — §6.2.6, `rfc4861.txt:2697-2701`

- Strength: description (the text says "might"). Class: wire.
- Check idea: a host sends a Router Solicitation shortly before a periodic advertisement is
  due. The router sends one advertisement at the scheduled time, not a second one.

### RFC4861-ADV-35

**When the router sent a multicast advertisement within MIN_DELAY_BETWEEN_RAS, it schedules the reply at MIN_DELAY_BETWEEN_RAS plus the random delay after that advertisement; else at the random delay.**

> "A router might process Router Solicitations as follows:" — §6.2.6, `rfc4861.txt:2695` and
> "If the router sent a multicast Router Advertisement (solicited or
> unsolicited) within the last MIN_DELAY_BETWEEN_RAS seconds,
> schedule the advertisement to be sent at a time corresponding to
> MIN_DELAY_BETWEEN_RAS plus the random value after the previous
> advertisement was sent.  This ensures that the multicast Router
> Advertisements are rate limited." — §6.2.6, `rfc4861.txt:2703-2708` and
> "Otherwise, schedule the sending of a Router Advertisement at the
> time given by the random value." — §6.2.6, `rfc4861.txt:2710-2711`

- Strength: description (the text says "might"). Class: wire.
- Check idea: a host sends a Router Solicitation 1 second after a multicast advertisement.
  The reply comes between MIN_DELAY_BETWEEN_RAS and MIN_DELAY_BETWEEN_RAS plus
  MAX_RA_DELAY_TIME after that advertisement.

### RFC4861-ADV-36

**Only replies to solicitations may come more often than MinRtrAdvInterval; unsolicited multicast advertisements never do.**

> "Note that a router is permitted to send multicast Router
> Advertisements more frequently than indicated by the
> MinRtrAdvInterval configuration variable so long as the more frequent
> advertisements are responses to Router Solicitations.  In all cases,
> however, unsolicited multicast advertisements MUST NOT be sent more
> frequently than indicated by MinRtrAdvInterval." — §6.2.6, `rfc4861.txt:2713-2718`

- Strength: must not. Class: wire.
- Check idea: with no solicitations after the initial phase, no two unsolicited multicast
  Router Advertisements are closer than MinRtrAdvInterval. With solicitations, replies may
  come closer.

### RFC4861-ADV-37

**A Router Solicitation from the unspecified address does not change the Neighbor Cache of the router.**

> "Router Solicitations in which the Source Address is the unspecified
> address MUST NOT update the router's Neighbor Cache" — §6.2.6, `rfc4861.txt:2720-2721`

- Strength: must not. Class: internal.
- Check idea: a host sends a Router Solicitation from the unspecified address. The router
  makes no Neighbor Cache entry; a later unicast packet from the router to the host starts
  with a Neighbor Solicitation.

### RFC4861-ADV-38

**A Router Solicitation with a new link-layer address updates the Neighbor Cache entry of the sender, and the entry becomes STALE.**

> "solicitations
> with a proper source address update the Neighbor Cache as follows.
> If the router already has a Neighbor Cache entry for the
> solicitation's sender, the solicitation contains a Source Link-Layer
> Address option, and the received link-layer address differs from that
> already in the cache, then the link-layer address SHOULD be updated
> in the appropriate Neighbor Cache entry, and its reachability state
> MUST also be set to STALE." — §6.2.6, `rfc4861.txt:2721-2728`

- Strength: should (the address), must (the state). Class: internal.
- Check idea: the router has a REACHABLE entry for host A. Host A sends a Router Solicitation
  with a new link-layer address. The next unicast frame from the router to A goes to the new
  address, and the router later probes A as for a STALE entry.

### RFC4861-ADV-39

**A Router Solicitation from a sender with no Neighbor Cache entry creates a STALE entry with the link-layer address.**

> "If there is no existing Neighbor Cache
> entry for the solicitation's sender, the router creates one, installs
> the link- layer address and sets its reachability state to STALE as
> specified in Section 7.3.3." — §6.2.6, `rfc4861.txt:2728-2731`

- Strength: description. Class: internal.
- Check idea: host A, unknown to the router, sends a Router Solicitation with a Source
  Link-Layer Address option. The router then sends a unicast packet to A without a Neighbor
  Solicitation before it.

### RFC4861-ADV-40

**When the router has no entry and the solicitation has no Source Link-Layer Address option, the reply can be multicast or unicast.**

> "If there is no existing Neighbor Cache
> entry and no Source Link-Layer Address option was present in the
> solicitation, the router may respond with either a multicast or a
> unicast router advertisement." — §6.2.6, `rfc4861.txt:2731-2734`

- Strength: may (lower case). Class: wire.
- Check idea: an unknown host sends a Router Solicitation from a unicast address without a
  Source Link-Layer Address option. The reply goes to the all-nodes address, or to the host
  after address resolution.

### RFC4861-ADV-41

**A Router Solicitation sets the IsRouter flag of the Neighbor Cache entry of its sender to FALSE.**

> "Whether or not a Source Link-Layer
> Address option is provided, if a Neighbor Cache entry for the
> solicitation's sender exists (or is created) the entry's IsRouter
> flag MUST be set to FALSE." — §6.2.6, `rfc4861.txt:2734-2737`

- Strength: must. Class: internal.
- Check idea: node A sends Neighbor Advertisements with the Router flag set, then a Router
  Solicitation. After that, the router treats A as a host, not as a router.

### RFC4861-ADV-42

**A router checks the Router Advertisements of other routers for consistency, and it logs an inconsistency.**

> "Routers SHOULD inspect valid Router Advertisements sent by other
> routers and verify that the routers are advertising consistent
> information on a link.  Detected inconsistencies indicate that one or
> more routers might be misconfigured and SHOULD be logged to system or
> network management." — §6.2.7, `rfc4861.txt:2753-2757`

- Strength: should. Class: internal.
- Check idea: router B advertises an MTU option that differs from the one of router A on the
  same link. Router A records an inconsistency in its log.

### RFC4861-ADV-43

**The consistency check covers at least Cur Hop Limit, the M and O flags, Reachable Time, Retrans Timer, the MTU option and the prefix lifetimes; zero values are exempt.**

> "The minimum set of information to check
> includes:" — §6.2.7, `rfc4861.txt:2757-2758`;
> "Cur Hop Limit values (except for the unspecified value of zero
> other inconsistencies SHOULD be logged to system network
> management)." — §6.2.7, `rfc4861.txt:2760-2762`;
> "Values of the M or O flags." — §6.2.7, `rfc4861.txt:2764`;
> "Reachable Time values (except for the unspecified value of zero)." — §6.2.7, `rfc4861.txt:2766`;
> "Retrans Timer values (except for the unspecified value of zero)." — §6.2.7, `rfc4861.txt:2768`;
> "Values in the MTU options." — §6.2.7, `rfc4861.txt:2770` and
> "Preferred and Valid Lifetimes for the same prefix." — §6.2.7, `rfc4861.txt:2772`

- Strength: should. Class: internal.
- Check idea: router B differs from router A in one field at a time: Cur Hop Limit, M, O,
  Reachable Time, Retrans Timer, MTU, and the lifetimes of one prefix. Router A logs each
  difference. A value of zero in B gives no log entry.

### RFC4861-ADV-44

**For lifetimes that decrement in real time, the check compares the times of deprecation and invalidation, and it allows some clock skew.**

> "If
> AdvPreferredLifetime and/or AdvValidLifetime decrement in real
> time as specified in Section 6.2.1 then the comparison of the
> lifetimes cannot compare the content of the fields in the Router
> Advertisement, but must instead compare the time at which the
> prefix will become deprecated and invalidated, respectively.  Due
> to link propagation delays and potentially poorly synchronized
> clocks between the routers such comparison SHOULD allow some time
> skew." — §6.2.7, `rfc4861.txt:2772-2780`

- Strength: must (lower case), should (the skew). Class:
  internal.
- Check idea: routers A and B advertise one prefix with lifetimes that decrement to the same
  end time, sent at different moments. Router A logs no inconsistency. A different end time
  gives a log entry.

### RFC4861-ADV-45

**A router does not log different prefix sets or zero values; it logs only conflicts that make hosts switch values with each advertisement.**

> "Note that it is not an error for different routers to advertise
> different sets of prefixes.  Also, some routers might leave some
> fields as unspecified, i.e., with the value zero, while other routers
> specify values.  The logging of errors SHOULD be restricted to
> conflicting information that causes hosts to switch from one value to
> another with each received advertisement." — §6.2.7, `rfc4861.txt:2782-2787`

- Strength: should. Class: internal.
- Check idea: router B advertises other prefixes than router A, and Reachable Time 0 where A
  advertises 30000. Router A logs nothing.

### RFC4861-ADV-46

**The source address of the Router Advertisements of a router equals the Target Address of a Redirect to that router.**

> "Thus, the
> source address used in Router Advertisements sent by a particular
> router must be identical to the target address in a Redirect message
> when redirecting to that router." — §6.2.8, `rfc4861.txt:2808-2811`

- Strength: must (lower case). Class: wire.
- Check idea: router A redirects a host to router B on the same link. The Target Address of
  the Redirect equals the source address of the Router Advertisements of router B.

### RFC4861-ADV-47

**A router that changes a link-local address tells the hosts: a few advertisements from the old address with Router Lifetime zero, and a few from the new address.**

> "If a router changes the link-local address for one of its interfaces,
> it SHOULD inform hosts of this change.  The router SHOULD multicast a
> few Router Advertisements from the old link-local address with the
> Router Lifetime field set to zero and also multicast a few Router
> Advertisements from the new link-local address.  The overall effect
> should be the same as if one interface ceases being an advertising
> interface, and a different one starts being an advertising
> interface." — §6.2.8, `rfc4861.txt:2817-2823`

- Strength: should. Class: wire.
- Check idea: change the link-local address of an advertising interface. The router sends
  multicast Router Advertisements from the old address with Router Lifetime 0, and then
  multicast Router Advertisements from the new address. The hosts use the new address as
  default router.

## Host behavior

### RFC4861-HOST-1

**Router Advertisements override the defaults of the host variables; a host uses the defaults when no router is present or all routers leave a value unspecified.**

> "These variables have default values that are overridden by
> information received in Router Advertisement messages.  The default
> values are used when there is no router on the link or when all
> received Router Advertisements have left a particular value
> unspecified." — §6.3.2, `rfc4861.txt:2839-2843`

- Strength: description. Class: end-to-end.
- Check idea: on a link without a router, a host sends packets with the default hop limit.
  After a Router Advertisement with Cur Hop Limit 100, it uses 100. On a link where all
  routers send Cur Hop Limit 0, it keeps the default.

### RFC4861-HOST-2

**A document for a link layer can override the default values of the host variables.**

> "The default values in this specification may be overridden by
> specific documents that describe how IP operates over different link
> layers." — §6.3.2, `rfc4861.txt:2845-2847`

- Strength: may (lower case). Class: internal.
- Check idea: on a link type whose own document gives other defaults, a host without Router
  Advertisements uses those defaults.

### RFC4861-HOST-3

**LinkMTU is the MTU of the link; its default comes from the document for the link layer.**

> "LinkMTU        The MTU of the link.
> Default: The valued defined in the specific
> document that describes how IPv6 operates over
> the particular link layer (e.g., [IPv6-ETHER])." — §6.3.2, `rfc4861.txt:2865-2868`

- Strength: description. Class: end-to-end.
- Check idea: on an Ethernet link without an MTU option, a host sends packets of up to 1500
  octets, and it fragments larger ones.

### RFC4861-HOST-4

**CurHopLimit is the default hop limit of the packets of a host; its default is the value of the Assigned Numbers registry.**

> "CurHopLimit    The default hop limit to be used when sending IP
> packets." — §6.3.2, `rfc4861.txt:2870-2871` and
> "Default: The value specified in the "Assigned
> Numbers" [ASSIGNED] that was in effect at the
> time of implementation." — §6.3.2, `rfc4861.txt:2873-2875`

- Strength: description. Class: wire.
- Check idea: a host that has received no Router Advertisement sends packets whose Hop Limit
  is the default hop limit of the Assigned Numbers registry.

### RFC4861-HOST-5

**BaseReachableTime is the base of the random ReachableTime; its default is REACHABLE_TIME milliseconds.**

> "BaseReachableTime
> A base value used for computing the random
> ReachableTime value." — §6.3.2, `rfc4861.txt:2877-2879` and
> "Default: REACHABLE_TIME milliseconds." — §6.3.2, `rfc4861.txt:2881`

- Strength: description. Class: internal.
- Check idea: without Router Advertisements, the time that a neighbor entry of a host stays
  REACHABLE after a confirmation lies between MIN_RANDOM_FACTOR and MAX_RANDOM_FACTOR times
  REACHABLE_TIME.

### RFC4861-HOST-6

**ReachableTime is a uniform random value between MIN_RANDOM_FACTOR and MAX_RANDOM_FACTOR times BaseReachableTime, recomputed when the base changes and at least every few hours.**

> "ReachableTime  The time a neighbor is considered reachable after
> receiving a reachability confirmation." — §6.3.2, `rfc4861.txt:2883-2884` and
> "This value should be a uniformly distributed
> random value between MIN_RANDOM_FACTOR and
> MAX_RANDOM_FACTOR times BaseReachableTime
> milliseconds.  A new random value should be
> calculated when BaseReachableTime changes (due to
> Router Advertisements) or at least every few
> hours even if no Router Advertisements are
> received." — §6.3.2, `rfc4861.txt:2886-2893`

- Strength: should (lower case). Class: internal.
- Check idea: measure how long neighbor entries stay REACHABLE on many hosts, or on one host
  over many hours. The times lie between the two factors times the base, and they differ.

### RFC4861-HOST-7

**RetransTimer is the time between retransmitted Neighbor Solicitations; its default is RETRANS_TIMER milliseconds.**

> "RetransTimer   The time between retransmissions of Neighbor
> Solicitation messages to a neighbor when
> resolving the address or when probing the
> reachability of a neighbor." — §6.3.2, `rfc4861.txt:2895-2898` and
> "Default: RETRANS_TIMER milliseconds" — §6.3.2, `rfc4861.txt:2900`

- Strength: description. Class: wire.
- Check idea: without Router Advertisements, a host resolves an address that nobody owns. Its
  Neighbor Solicitations are RETRANS_TIMER apart.

### RFC4861-HOST-8

**A host joins the all-nodes multicast address on every multicast-capable interface.**

> "The host joins the all-nodes multicast address on all multicast-
> capable interfaces." — §6.3.3, `rfc4861.txt:2904-2905`

- Strength: description. Class: end-to-end.
- Check idea: a host with two interfaces receives a Router Advertisement to the all-nodes
  address on each link, and it uses each router on its link.

### RFC4861-HOST-9

**A host keeps the union of what it learns; a new Router Advertisement does not remove all earlier information.**

> "Hosts
> accept the union of all received information; the receipt of a Router
> Advertisement MUST NOT invalidate all information received in a
> previous advertisement or from another source." — §6.3.4, `rfc4861.txt:2924-2927`

- Strength: must not. Class: end-to-end.
- Check idea: router A advertises prefix P1, and router B advertises prefix P2. After both
  advertisements, the host treats P1 and P2 as on-link and uses both routers.

### RFC4861-HOST-10

**For a parameter with one value, the most recent received value wins.**

> "However, when
> received information for a specific parameter (e.g., Link MTU) or
> option (e.g., Lifetime on a specific Prefix) differs from information
> received earlier, and the parameter/option can only have one value,
> the most recently received information is considered authoritative." — §6.3.4, `rfc4861.txt:2927-2931`

- Strength: description. Class: end-to-end.
- Check idea: router A advertises MTU 1400, then router B advertises MTU 1300. The host then
  sends packets of at most 1300 octets.

### RFC4861-HOST-11

**A host ignores an unspecified field and keeps its current value; it does not return to the default.**

> "A Router Advertisement field (e.g., Cur Hop Limit, Reachable Time,
> and Retrans Timer) may contain a value denoting that it is
> unspecified.  In such cases, the parameter should be ignored and the
> host should continue using whatever value it is already using.  In
> particular, a host MUST NOT interpret the unspecified value as
> meaning change back to the default value that was in use before the
> first Router Advertisement was received." — §6.3.4, `rfc4861.txt:2933-2939`

- Strength: must not. Class: end-to-end.
- Check idea: router A advertises Cur Hop Limit 100, then router B advertises Cur Hop Limit
  0. The host keeps Hop Limit 100 in its packets.

### RFC4861-HOST-12

**An advertisement from a new source with a non-zero Router Lifetime adds a Default Router List entry with that lifetime.**

> "On receipt of a valid Router Advertisement, a host extracts the
> source address of the packet and does the following:" — §6.3.4, `rfc4861.txt:2944-2945` and
> "If the address is not already present in the host's Default
> Router List, and the advertisement's Router Lifetime is non-
> zero, create a new entry in the list, and initialize its
> invalidation timer value from the advertisement's Router
> Lifetime field." — §6.3.4, `rfc4861.txt:2947-2951`

- Strength: description. Class: end-to-end.
- Check idea: a router sends one Router Advertisement with Router Lifetime 30. The host sends
  off-link packets through the router for 30 seconds, and no longer. An advertisement with
  Router Lifetime 0 from a new router adds no default router.

### RFC4861-HOST-13

**An advertisement from a router that is in the list restarts the invalidation timer with the new Router Lifetime.**

> "On receipt of a valid Router Advertisement, a host extracts the
> source address of the packet and does the following:" — §6.3.4, `rfc4861.txt:2944-2945` and
> "If the address is already present in the host's Default Router
> List as a result of a previously received advertisement, reset
> its invalidation timer to the Router Lifetime value in the newly
> received advertisement." — §6.3.4, `rfc4861.txt:2953-2956`

- Strength: description. Class: end-to-end.
- Check idea: a router sends Router Lifetime 30, and 20 seconds later again Router Lifetime
  30. The host uses the router until 50 seconds after the first advertisement.

### RFC4861-HOST-14

**An advertisement with Router Lifetime zero from a router in the list removes the router at once.**

> "On receipt of a valid Router Advertisement, a host extracts the
> source address of the packet and does the following:" — §6.3.4, `rfc4861.txt:2944-2945` and
> "If the address is already present in the host's Default Router
> List and the received Router Lifetime value is zero, immediately
> time-out the entry as specified in Section 6.3.5." — §6.3.4, `rfc4861.txt:2958-2960`

- Strength: description. Class: end-to-end.
- Check idea: a host uses router A and router B. Router A sends Router Lifetime 0. The next
  off-link packets of the host go through router B.

### RFC4861-HOST-15

**A host may limit the size of its Default Router List, but it keeps at least two routers, and more if it can.**

> "To limit the storage needed for the Default Router List, a host MAY
> choose not to store all of the router addresses discovered via
> advertisements.  However, a host MUST retain at least two router
> addresses and SHOULD retain more." — §6.3.4, `rfc4861.txt:2962-2965`

- Strength: may (not all), must (two), should (more). Class: end-to-end.
- Check idea: three routers advertise on a link. The router in use fails. The host sends its
  next off-link packets through another advertised router, without a new advertisement.

### RFC4861-HOST-16

**A non-zero Cur Hop Limit in a Router Advertisement sets the CurHopLimit of the host.**

> "If the received Cur Hop Limit value is non-zero, the host SHOULD set
> its CurHopLimit variable to the received value." — §6.3.4, `rfc4861.txt:2979-2980`

- Strength: should. Class: wire.
- Check idea: a router advertises Cur Hop Limit 100. The packets that the host sends
  afterwards have Hop Limit 100.

### RFC4861-HOST-17

**A non-zero Reachable Time sets BaseReachableTime, and a changed value gives a new random ReachableTime.**

> "If the received Reachable Time value is non-zero, the host SHOULD set
> its BaseReachableTime variable to the received value.  If the new
> value differs from the previous value, the host SHOULD re-compute a
> new random ReachableTime value.  ReachableTime is computed as a
> uniformly distributed random value between MIN_RANDOM_FACTOR and
> MAX_RANDOM_FACTOR times the BaseReachableTime." — §6.3.4, `rfc4861.txt:2982-2987`

- Strength: should. Class: internal.
- Check idea: a router advertises Reachable Time 10000. The time that a neighbor entry of the
  host stays REACHABLE after a confirmation then lies between MIN_RANDOM_FACTOR and
  MAX_RANDOM_FACTOR times 10000 milliseconds.

### RFC4861-HOST-18

**A host recomputes the random ReachableTime at least once every few hours, also when the base does not change.**

> "In most cases, the advertised Reachable Time value will be the same
> in consecutive Router Advertisements, and a host's BaseReachableTime
> rarely changes.  In such cases, an implementation SHOULD ensure that
> a new random value gets re-computed at least once every few hours." — §6.3.4, `rfc4861.txt:2991-2994`

- Strength: should. Class: internal.
- Check idea: a router advertises the same Reachable Time for a day. The time that neighbor
  entries of the host stay REACHABLE changes at least once every few hours.

### RFC4861-HOST-19

**A non-zero Retrans Timer in a Router Advertisement sets the RetransTimer of the host.**

> "The RetransTimer variable SHOULD be copied from the Retrans Timer
> field, if the received value is non-zero." — §6.3.4, `rfc4861.txt:2996-2997`

- Strength: should. Class: wire.
- Check idea: a router advertises Retrans Timer 2000. The host resolves an address that
  nobody owns, and its Neighbor Solicitations are 2 seconds apart.

### RFC4861-HOST-20

**A Source Link-Layer Address option in a Router Advertisement goes into the Neighbor Cache entry of the router, and the entry gets IsRouter TRUE.**

> "After extracting information from the fixed part of the Router
> Advertisement message, the advertisement is scanned for valid
> options.  If the advertisement contains a Source Link-Layer Address
> option, the link-layer address SHOULD be recorded in the Neighbor
> Cache entry for the router (creating an entry if necessary) and the
> IsRouter flag in the Neighbor Cache entry MUST be set to TRUE." — §6.3.4, `rfc4861.txt:2999-3004`

- Strength: should (the address), must (the IsRouter flag). Class: internal.
- Check idea: a router sends a Router Advertisement with a Source Link-Layer Address option.
  The host sends its first off-link packet to that link-layer address without a Neighbor
  Solicitation before it.

### RFC4861-HOST-21

**A Router Advertisement without a Source Link-Layer Address option sets IsRouter TRUE in an entry that exists.**

> "If no
> Source Link-Layer Address is included, but a corresponding Neighbor
> Cache entry exists, its IsRouter flag MUST be set to TRUE." — §6.3.4, `rfc4861.txt:3004-3006`

- Strength: must. Class: internal.
- Check idea: the host has an entry for node R from a Neighbor Advertisement with the Router
  flag zero. R sends a Router Advertisement without the option. The host then treats R as a
  router.

### RFC4861-HOST-22

**A Neighbor Cache entry that a Router Advertisement creates is STALE.**

> "If a Neighbor Cache entry is created
> for the router, its reachability state MUST be set to STALE as
> specified in Section 7.3.3." — §6.3.4, `rfc4861.txt:3009-3011`

- Strength: must. Class: internal.
- Check idea: after the first Router Advertisement with the option, the host sends a packet
  through the router. After DELAY_FIRST_PROBE_TIME, the host probes the router with a
  unicast Neighbor Solicitation.

### RFC4861-HOST-23

**A Neighbor Cache entry that gets a different link-layer address from a Router Advertisement becomes STALE.**

> "If a cache entry already exists and is
> updated with a different link-layer address, the reachability state
> MUST also be set to STALE." — §6.3.4, `rfc4861.txt:3011-3013`

- Strength: must. Class: internal.
- Check idea: the host has a REACHABLE entry for the router. The router sends a Router
  Advertisement with a new link-layer address. The host sends to the new address, and it
  probes the router as for a STALE entry.

### RFC4861-HOST-24

**A host takes the MTU option value as LinkMTU when it is between the minimum IPv6 link MTU and the maximum of the link type.**

> "If the MTU option is present, hosts SHOULD copy the option's value
> into LinkMTU so long as the value is greater than or equal to the
> minimum link MTU [IPv6] and does not exceed the maximum LinkMTU value
> specified in the link-type-specific document (e.g., [IPv6-ETHER])." — §6.3.4, `rfc4861.txt:3015-3018`

- Strength: should. Class: end-to-end.
- Check idea: on Ethernet, an MTU option of 1400 makes the host send packets of at most 1400
  octets. An MTU option of 1000 or of 9000 changes nothing.

### RFC4861-HOST-25

**A Prefix Information option with the L flag set marks its prefix as on-link.**

> "Prefix Information options that have the "on-link" (L) flag set
> indicate a prefix identifying a range of addresses that should be
> considered on-link." — §6.3.4, `rfc4861.txt:3020-3022`

- Strength: should (lower case). Class: end-to-end.
- Check idea: a router advertises prefix P with the L flag set. The host sends a packet to an
  address in P directly: it sends a Neighbor Solicitation for that address, not a packet to
  the router.

### RFC4861-HOST-26

**A Prefix Information option with the L flag zero says nothing about on-link, and it does not make the prefix off-link.**

> "Note, however, that a Prefix Information option
> with the on-link flag set to zero conveys no information concerning
> on-link determination and MUST NOT be interpreted to mean that
> addresses covered by the prefix are off-link." — §6.3.4, `rfc4861.txt:3022-3023`, `rfc4861.txt:3031-3033`

- Strength: must not. Class: end-to-end.
- Check idea: router A advertises prefix P with L set. Router B then advertises P with L
  zero. The host still sends packets to addresses in P directly.

### RFC4861-HOST-27

**Only a Prefix Information option with the L flag set and lifetime zero cancels an on-link indication.**

> "The only way to cancel
> a previous on-link indication is to advertise that prefix with the
> L-bit set and the Lifetime set to zero." — §6.3.4, `rfc4861.txt:3033-3035`

- Strength: description. Class: end-to-end.
- Check idea: router A advertises P with L set, then with L set and Valid Lifetime 0. The
  host then sends packets to addresses in P through a default router.

### RFC4861-HOST-28

**A packet to an address of unknown on-link status goes to a default router, and an L flag of zero does not change this.**

> "The default behavior (see
> Section 5.2) when sending a packet to an address for which no
> information is known about the on-link status of the address is to
> forward the packet to a default router; the reception of a Prefix
> Information option with the "on-link" (L) flag set to zero does not
> change this behavior." — §6.3.4, `rfc4861.txt:3035-3040`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises prefix P only with L zero. The host sends packets to
  addresses in P through the default router.

### RFC4861-HOST-29

**A host silently ignores a Prefix Information option for the link-local prefix.**

> "For each Prefix Information option with the on-link flag set, a host
> does the following:" — §6.3.4, `rfc4861.txt:3045-3046` and
> "If the prefix is the link-local prefix, silently ignore the
> Prefix Information option." — §6.3.4, `rfc4861.txt:3048-3049`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises fe80::/64 with L set and Valid Lifetime 0. The host still
  reaches link-local neighbors directly.

### RFC4861-HOST-30

**A new prefix with a non-zero Valid Lifetime goes into the Prefix List with that lifetime.**

> "For each Prefix Information option with the on-link flag set, a host
> does the following:" — §6.3.4, `rfc4861.txt:3045-3046` and
> "If the prefix is not already present in the Prefix List, and the
> Prefix Information option's Valid Lifetime field is non-zero,
> create a new entry for the prefix and initialize its
> invalidation timer to the Valid Lifetime value in the Prefix
> Information option." — §6.3.4, `rfc4861.txt:3051-3055`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises prefix P once with Valid Lifetime 30. The host sends to
  addresses in P directly for 30 seconds, then through the default router.

### RFC4861-HOST-31

**A prefix that is in the Prefix List gets its invalidation timer reset to the new Valid Lifetime.**

> "For each Prefix Information option with the on-link flag set, a host
> does the following:" — §6.3.4, `rfc4861.txt:3045-3046` and
> "If the prefix is already present in the host's Prefix List as
> the result of a previously received advertisement, reset its
> invalidation timer to the Valid Lifetime value in the Prefix
> Information option." — §6.3.4, `rfc4861.txt:3057-3060`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises P with Valid Lifetime 30, and 20 seconds later with Valid
  Lifetime 30 again. The host treats P as on-link until 50 seconds after the first
  advertisement.

### RFC4861-HOST-32

**A Valid Lifetime of zero for a prefix in the Prefix List removes the prefix at once.**

> "For each Prefix Information option with the on-link flag set, a host
> does the following:" — §6.3.4, `rfc4861.txt:3045-3046` and
> "If the new Lifetime value is zero, time-out
> the prefix immediately (see Section 6.3.5)." — §6.3.4, `rfc4861.txt:3060-3061`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises P with a long lifetime, then with Valid Lifetime 0. The
  host at once sends packets to addresses in P through the default router.

### RFC4861-HOST-33

**A host silently ignores a Prefix Information option with Valid Lifetime zero for a prefix that it does not know.**

> "For each Prefix Information option with the on-link flag set, a host
> does the following:" — §6.3.4, `rfc4861.txt:3045-3046` and
> "If the Prefix Information option's Valid Lifetime field is zero,
> and the prefix is not present in the host's Prefix List,
> silently ignore the option." — §6.3.4, `rfc4861.txt:3063-3065`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises an unknown prefix P with Valid Lifetime 0. The host sends
  packets to addresses in P through the default router.

### RFC4861-HOST-34

**For on-link determination, a host accepts any Valid Lifetime; Neighbor Discovery has no minimum check on prefix lifetimes.**

> "However, since the effect of the same denial of service targeted at
> the on-link prefix list is not catastrophic (hosts would send packets
> to a default router and receive a redirect rather than sending
> packets directly to a neighbor), the Neighbor Discovery protocol does
> not impose such a check on the prefix lifetime values." — §6.3.4, `rfc4861.txt:3070-3074`

- Strength: description. Class: end-to-end.
- Check idea: a router advertises P with Valid Lifetime 3600, then with Valid Lifetime 10.
  The host stops to treat P as on-link after 10 seconds.

### RFC4861-HOST-35

**A prefix length that address autoconfiguration rejects is still valid for on-link determination.**

> "Similarly,
> [ADDRCONF] may impose certain restrictions on the prefix length for
> address configuration purposes.  Therefore, the prefix might be
> rejected by [ADDRCONF] implementation in the host.  However, the
> prefix length is still valid for on-link determination when combined
> with other flags in the prefix option." — §6.3.4, `rfc4861.txt:3074-3077`, `rfc4861.txt:3087-3088`

- Strength: may (lower case). Class: end-to-end.
- Check idea: a router advertises a /48 prefix with the L and A flags set. The host sends
  packets to any address in the /48 directly.

### RFC4861-HOST-36

**When the invalidation timer of a prefix expires, the host discards the prefix; Destination Cache entries need no update.**

> "Whenever the invalidation timer expires for a Prefix List entry, that
> entry is discarded.  No existing Destination Cache entries need be
> updated, however." — §6.3.5, `rfc4861.txt:3099-3101`

- Strength: description. Class: end-to-end.
- Check idea: prefix P expires. The host sends packets to a new address in P through the
  default router.

### RFC4861-HOST-37

**When the lifetime of a default router expires, the host discards the router.**

> "Whenever the Lifetime of an entry in the Default Router List expires,
> that entry is discarded." — §6.3.5, `rfc4861.txt:3105-3106`

- Strength: description. Class: end-to-end.
- Check idea: a router sends Router Lifetime 30 and then no more advertisements. After 30
  seconds, the host sends no off-link packet through that router.

### RFC4861-HOST-38

**When a router leaves the Default Router List, destinations that use it do next-hop determination again.**

> "When removing a router from the Default
> Router list, the node MUST update the Destination Cache in such a way
> that all entries using the router perform next-hop determination
> again rather than continue sending traffic to the (deleted) router." — §6.3.5, `rfc4861.txt:3106-3109`

- Strength: must. Class: end-to-end.
- Check idea: a host sends to off-link destination D through router A. Router A sends Router
  Lifetime 0. The next packets to D go through router B.

### RFC4861-HOST-39

**A host selects a default router when a new off-link destination has no Destination Cache entry, or when its router seems to fail; later traffic keeps that router.**

> "The algorithm for selecting a default router is invoked
> during next-hop determination when no Destination Cache entry exists
> for an off-link destination or when communication through an existing
> router appears to be failing.  Under normal conditions, a router
> would be selected the first time traffic is sent to a destination,
> with subsequent traffic for that destination using the same router as
> indicated in the Destination Cache modulo any changes to the
> Destination Cache caused by Redirect messages." — §6.3.6, `rfc4861.txt:3116-3123`

- Strength: description. Class: end-to-end.
- Check idea: a host with two reachable default routers sends many packets to destination D.
  All packets to D go through the same router.

### RFC4861-HOST-40

**A host prefers routers that are reachable or probably reachable over routers in INCOMPLETE state or without an entry.**

> "Routers that are reachable or probably reachable (i.e., in any
> state other than INCOMPLETE) SHOULD be preferred over routers
> whose reachability is unknown or suspect (i.e., in the
> INCOMPLETE state, or for which no Neighbor Cache entry exists)." — §6.3.6, `rfc4861.txt:3128-3131`

- Strength: should. Class: end-to-end.
- Check idea: router A does not answer Neighbor Solicitations, and router B is REACHABLE. The
  host sends packets to new off-link destinations through router B.

### RFC4861-HOST-41

**When no router is known to be reachable, a host selects routers in round-robin order.**

> "When no routers on the list are known to be reachable or
> probably reachable, routers SHOULD be selected in a round-robin
> fashion, so that subsequent requests for a default router do not
> return the same router until all other routers have been
> selected." — §6.3.6, `rfc4861.txt:3143-3147`

- Strength: should. Class: end-to-end.
- Check idea: three routers advertise and then stop to answer. The host sends to new
  off-link destinations, and it probes the three routers in turn before it repeats one.

### RFC4861-HOST-42

**A host sends up to MAX_RTR_SOLICITATIONS Router Solicitations, at least RTR_SOLICITATION_INTERVAL apart.**

> "To obtain Router Advertisements quickly,
> a host SHOULD transmit up to MAX_RTR_SOLICITATIONS Router
> Solicitation messages, each separated by at least
> RTR_SOLICITATION_INTERVAL seconds." — §6.3.7, `rfc4861.txt:3160-3163`

- Strength: should. Class: wire.
- Check idea: a host starts on a link without a router. It sends at most
  MAX_RTR_SOLICITATIONS Router Solicitations, and no two are closer than
  RTR_SOLICITATION_INTERVAL.

### RFC4861-HOST-43

**A host can send Router Solicitations after startup, after reinitialization of an interface, after it stops to be a router, and after it attaches or re-attaches to a link.**

> "Router Solicitations may be sent
> after any of the following events:" — §6.3.7, `rfc4861.txt:3163-3164`;
> "The interface is initialized at system startup time." — §6.3.7, `rfc4861.txt:3166`;
> "The interface is reinitialized after a temporary interface
> failure or after being temporarily disabled by system
> management." — §6.3.7, `rfc4861.txt:3168-3170`;
> "The system changes from being a router to being a host, by
> having its IP forwarding capability turned off by system
> management." — §6.3.7, `rfc4861.txt:3172-3174`;
> "The host attaches to a link for the first time." — §6.3.7, `rfc4861.txt:3176` and
> "The host re-attaches to a link after being detached for some
> time." — §6.3.7, `rfc4861.txt:3178-3179`

- Strength: may (lower case). Class: wire.
- Check idea: disable and enable the interface of a host, or move the host to another link.
  The host sends Router Solicitations again.

### RFC4861-HOST-44

**A host sends Router Solicitations to the all-routers multicast address.**

> "A host sends Router Solicitations to the all-routers multicast
> address." — §6.3.7, `rfc4861.txt:3181-3182`

- Strength: description. Class: wire.
- Check idea: every Router Solicitation of a host has the destination address ff02::2.

### RFC4861-HOST-45

**The IP source address of a Router Solicitation is a unicast address of the interface or the unspecified address.**

> "The IP source address is set to either one of the
> interface's unicast addresses or the unspecified address." — §6.3.7, `rfc4861.txt:3182-3183`

- Strength: description. Class: wire.
- Check idea: the source address of every Router Solicitation of a host is a unicast address
  of the interface that sends it, or the unspecified address.

### RFC4861-HOST-46

**A Router Solicitation from a unicast address carries a Source Link-Layer Address option with the link-layer address of the host.**

> "The Source
> Link-Layer Address option SHOULD be set to the host's link-layer
> address, if the IP source address is not the unspecified address." — §6.3.7, `rfc4861.txt:3183-3185`

- Strength: should. Class: wire.
- Check idea: each Router Solicitation with a unicast source address carries a Source
  Link-Layer Address option that holds the link-layer address of the interface.

### RFC4861-HOST-47

**Before its first Router Solicitation, a host waits a random time from 0 to MAX_RTR_SOLICITATION_DELAY.**

> "Before a host sends an initial solicitation, it SHOULD delay the
> transmission for a random amount of time between 0 and
> MAX_RTR_SOLICITATION_DELAY." — §6.3.7, `rfc4861.txt:3187-3189`

- Strength: should. Class: wire.
- Check idea: many hosts start on one link at the same time. The first Router Solicitation
  of each comes between 0 and MAX_RTR_SOLICITATION_DELAY after the start, and the delays
  vary.

### RFC4861-HOST-48

**A host that already had a random delay since the interface became enabled does not need to delay its first Router Solicitation again.**

> "If a host has already performed
> a random delay since the interface became (re)enabled (e.g., as part
> of Duplicate Address Detection [ADDRCONF]), there is no need to delay
> again before sending the first Router Solicitation message." — §6.3.7, `rfc4861.txt:3199-3202`

- Strength: description. Class: wire.
- Check idea: a host delays its Duplicate Address Detection by a random time after start.
  Its first Router Solicitation can follow without a second random delay.

### RFC4861-HOST-49

**A host may skip the random delay when necessary; a mobile node with a link-layer hint of movement may send a Router Solicitation at once.**

> "In some cases, the random delay MAY be omitted if necessary." — §6.3.7, `rfc4861.txt:3204` and
> "Hence, if a mobile node received link-layer
> information indicating that movement might have taken place, it MAY
> send a Router Solicitation immediately, without random delays." — §6.3.7, `rfc4861.txt:3210-3212`

- Strength: may. Class: wire.
- Check idea: a host attaches to a new link and gets a link-layer indication of the move. Its
  first Router Solicitation may come without delay; a test accepts any delay from 0 to
  MAX_RTR_SOLICITATION_DELAY.

### RFC4861-HOST-50

**After a valid Router Advertisement with a non-zero Router Lifetime, a host stops its Router Solicitations on that interface until the next event.**

> "Once the host sends a Router Solicitation, and receives a valid
> Router Advertisement with a non-zero Router Lifetime, the host MUST
> desist from sending additional solicitations on that interface, until
> the next time one of the above events occurs." — §6.3.7, `rfc4861.txt:3222-3225`

- Strength: must. Class: wire.
- Check idea: a host sends its first Router Solicitation, and a router replies with Router
  Lifetime 1800. The host sends no more Router Solicitations on that interface.

### RFC4861-HOST-51

**A host sends at least one Router Solicitation, also when an advertisement arrives before its first solicitation.**

> "Moreover, a host
> SHOULD send at least one solicitation in the case where an
> advertisement is received prior to having sent a solicitation." — §6.3.7, `rfc4861.txt:3225-3227`

- Strength: should. Class: wire.
- Check idea: a router sends an unsolicited Router Advertisement during the initial delay of
  a host. The host still sends one Router Solicitation.

### RFC4861-HOST-52

**After MAX_RTR_SOLICITATIONS solicitations without an answer, a host concludes that the link has no router, but it still processes later Router Advertisements.**

> "If a host sends MAX_RTR_SOLICITATIONS solicitations, and receives no
> Router Advertisements after having waited MAX_RTR_SOLICITATION_DELAY
> seconds after sending the last solicitation, the host concludes that
> there are no routers on the link for the purpose of [ADDRCONF].
> However, the host continues to receive and process Router
> Advertisements messages in the event that routers appear on the link." — §6.3.7, `rfc4861.txt:3231-3236`

- Strength: description. Class: end-to-end.
- Check idea: a host sends MAX_RTR_SOLICITATIONS solicitations without an answer. Later a
  router starts and sends an unsolicited Router Advertisement. The host then uses that
  router.
## Address resolution

### RFC4861-AR-1

**A node does address resolution only for an on-link address whose link-layer address it does not know.**

> "Address
> resolution is performed only on addresses that are determined to be
> on-link and for which the sender does not know the corresponding
> link-layer address (see Section 5.2)." — §7.2, `rfc4861.txt:3356-3358`,
> `rfc4861.txt:3367`

- Strength: description. Class: wire.
- Check idea: host A has a Neighbor Cache entry for on-link host B. A sends a packet to B and a
  packet to an off-link destination. No NS for B or for the off-link destination appears.

### RFC4861-AR-2

**A node never does address resolution for a multicast address.**

> "Address resolution is never
> performed on multicast addresses." — §7.2, `rfc4861.txt:3367-3368`

- Strength: description. Class: wire.
- Check idea: host A sends a packet to a multicast group on the link. No NS appears, and the
  frame goes to the link-layer multicast address that maps the group.

### RFC4861-AR-3

**A solicitation, a Router Advertisement or a Redirect without a link-layer address option does not create or update a Neighbor Cache entry, except for the IsRouter flag.**

> "It is possible that a host may receive a solicitation, a router
> advertisement, or a Redirect message without a link-layer address
> option included. These messages MUST NOT create or update neighbor
> cache entries, except with respect to the IsRouter flag as specified
> in Sections 6.3.4 and 7.2.5. If a Neighbor Cache entry does not
> exist for the source of such a message, Address Resolution will be
> required before unicast communications with that address can begin." — §7.2,
> `rfc4861.txt:3370-3376`

- Strength: must not. Class: end-to-end.
- Check idea: node B has no entry at host A. B sends A a unicast NS, a Router Advertisement or a
  Redirect, each without a link-layer address option. Before A sends a unicast packet to B, A
  sends a multicast NS for B.

### RFC4861-AR-4

**When a multicast-capable interface becomes enabled, the node joins the all-nodes address and the solicited-node address of each address of the interface.**

> "When a multicast-capable interface becomes enabled, the node MUST
> join the all-nodes multicast address on that interface, as well as
> the solicited-node multicast address corresponding to each of the IP
> addresses assigned to the interface." — §7.2.1, `rfc4861.txt:3383-3386`

- Strength: must. Class: end-to-end.
- Check idea: enable the interface of host A with two addresses. A answers an NS sent to the
  solicited-node address of each address, and A answers an Echo Request sent to the all-nodes
  address.

### RFC4861-AR-5

**When the set of addresses changes, the node joins the solicited-node address of each new address and leaves that of each removed address.**

> "The set of addresses assigned to an interface may change over time.
> New addresses might be added and old addresses might be removed
> [ADDRCONF]. In such cases the node MUST join and leave the
> solicited-node multicast address corresponding to the new and old
> addresses, respectively." — §7.2.1, `rfc4861.txt:3388-3392`

- Strength: must. Class: wire.
- Check idea: add an address to host A, then remove it. The MLD messages of A join the
  solicited-node group of the new address, and later leave it. Between the two, A answers an
  NS for the new address sent to that group.

### RFC4861-AR-6

**A node joins a solicited-node multicast address with Multicast Listener Discovery (MLD or MLDv2).**

> "Joining the solicited-node multicast
> address is done using a Multicast Listener Discovery such as [MLD] or
> [MLDv2] protocols." — §7.2.1, `rfc4861.txt:3392-3394`

- Strength: description. Class: wire.
- Check idea: enable the interface of host A. An MLD or MLDv2 report for each solicited-node
  address of A appears on the link.

### RFC4861-AR-7

**A node leaves a solicited-node group only after it removes all its addresses that map to that group.**

> "Note that multiple unicast addresses may map into
> the same solicited-node multicast address; a node MUST NOT leave the
> solicited-node multicast group until all assigned addresses
> corresponding to that multicast address have been removed." — §7.2.1,
> `rfc4861.txt:3394-3397`

- Strength: must not. Class: wire.
- Check idea: host A has two addresses with the same low-order 24 bits. Remove one of them. A
  sends no MLD message that leaves the group, and A still answers an NS for the other address
  sent to that group.

### RFC4861-AR-8

**A node with a unicast packet for a neighbor with an unknown link-layer address creates an INCOMPLETE Neighbor Cache entry and sends an NS for the neighbor.**

> "When a node has a unicast packet to send to a neighbor, but does not
> know the neighbor's link-layer address, it performs address
> resolution. For multicast-capable interfaces, this entails creating
> a Neighbor Cache entry in the INCOMPLETE state and transmitting a
> Neighbor Solicitation message targeted at the neighbor." — §7.2.2,
> `rfc4861.txt:3401-3405`

- Strength: description. Class: wire.
- Check idea: host A sends a first packet to on-link host B. An NS with Target Address B leaves
  A before the packet. The entry of A for B stays INCOMPLETE until an NA arrives.

### RFC4861-AR-9

**The NS for address resolution goes to the solicited-node multicast address of the target.**

> "The
> solicitation is sent to the solicited-node multicast address
> corresponding to the target address." — §7.2.2, `rfc4861.txt:3405-3407`

- Strength: description. Class: wire.
- Check idea: host A resolves host B. The IP Destination Address of the NS is the
  solicited-node multicast address of B.

### RFC4861-AR-10

**If the source of the packet that causes the NS is an address of the interface, the NS uses that address as its source.**

> "If the source address of the packet prompting the solicitation is the
> same as one of the addresses assigned to the outgoing interface, that
> address SHOULD be placed in the IP Source Address of the outgoing
> solicitation." — §7.2.2, `rfc4861.txt:3409-3412`

- Strength: should. Class: wire.
- Check idea: host A has two addresses on its interface and sends a packet to host B from the
  second address. The IP Source Address of the NS is the second address.

### RFC4861-AR-11

**Otherwise, the NS uses any address of the interface as its source.**

> "Otherwise, any one of the addresses assigned to the
> interface should be used." — §7.2.2, `rfc4861.txt:3412-3413`

- Strength: should (lower case). Class: wire.
- Check idea: router R1 forwards a packet from another link to host B, which it has not
  resolved. The IP Source Address of the NS is an address of the interface of R1 on the link
  of B.

### RFC4861-AR-12

**An NS to a solicited-node multicast address includes the Source Link-Layer Address option, if the sender has a link-layer address.**

> "If the solicitation is being sent to a solicited-node multicast
> address, the sender MUST include its link-layer address (if it has
> one) as a Source Link-Layer Address option." — §7.2.2, `rfc4861.txt:3427-3429`

- Strength: must. Class: wire.
- Check idea: host A resolves host B. The multicast NS carries a Source Link-Layer Address
  option with the link-layer address of A.

### RFC4861-AR-13

**Another NS includes the Source Link-Layer Address option too, but a unicast NS can omit it.**

> "Otherwise, the sender
> SHOULD include its link-layer address (if it has one) as a Source
> Link-Layer Address option." — §7.2.2, `rfc4861.txt:3429-3431`; "On unicast
> solicitations, an implementation MAY omit the Source Link-Layer
> Address option." — `rfc4861.txt:3433-3435`

- Strength: should (include), may (omit on a unicast NS). Class: wire.
- Check idea: host A sends a unicast NS to host B, for example a reachability probe. Record if
  the NS carries the Source Link-Layer Address option. When it is present, it holds the
  link-layer address of A.

### RFC4861-AR-14

**During address resolution, the sender keeps a small queue of packets for each neighbor.**

> "While waiting for address resolution to complete, the sender MUST,
> for each neighbor, retain a small queue of packets waiting for
> address resolution to complete." — §7.2.2, `rfc4861.txt:3440-3442`

- Strength: must. Class: end-to-end.
- Check idea: host A sends packets to hosts B and C, which it has not resolved, at the same
  time. After the NA of each neighbor, the queued packets for that neighbor arrive at it.

### RFC4861-AR-15

**The queue holds at least one packet, and it can hold more.**

> "The queue MUST hold at least one
> packet, and MAY contain more." — §7.2.2, `rfc4861.txt:3442-3443`

- Strength: must (one packet), may (more). Class: end-to-end.
- Check idea: host A sends one packet to host B, which answers the NS only after a
  retransmission. The packet arrives at B.

### RFC4861-AR-16

**The number of queued packets for each neighbor is limited to a small value.**

> "However, the number of queued packets
> per neighbor SHOULD be limited to some small value." — §7.2.2, `rfc4861.txt:3443-3444`

- Strength: should. Class: end-to-end.
- Check idea: host A sends many packets to host B before B answers the NS. After the NA, B
  receives only a small number of them.

### RFC4861-AR-17

**When the queue overflows, the new packet replaces the oldest packet.**

> "When a queue
> overflows, the new arrival SHOULD replace the oldest entry." — §7.2.2,
> `rfc4861.txt:3444-3445`

- Strength: should. Class: end-to-end.
- Check idea: host A sends numbered packets to host B before B answers the NS, more than the
  queue holds. After the NA, B receives the most recent packets, and the oldest ones are lost.

### RFC4861-AR-18

**When address resolution completes, the node sends the queued packets.**

> "Once
> address resolution completes, the node transmits any queued packets." — §7.2.2,
> `rfc4861.txt:3445-3446`

- Strength: description. Class: wire.
- Check idea: host B answers the NS of host A. The queued packets leave A to the link-layer
  address of B.

### RFC4861-AR-19

**While it waits for an answer, the sender retransmits the NS about every RetransTimer milliseconds, also without new traffic.**

> "While awaiting a response, the sender SHOULD retransmit Neighbor
> Solicitation messages approximately every RetransTimer milliseconds,
> even in the absence of additional traffic to the neighbor." — §7.2.2,
> `rfc4861.txt:3448-3450`

- Strength: should. Class: wire.
- Check idea: host A sends one packet to host B, which does not answer. A sends more NS
  messages for B at intervals of about RetransTimer.

### RFC4861-AR-20

**The sender sends at most one NS for each neighbor every RetransTimer milliseconds.**

> "Retransmissions MUST be rate-limited to at most one solicitation per
> neighbor every RetransTimer milliseconds." — §7.2.2, `rfc4861.txt:3451-3452`

- Strength: must. Class: wire.
- Check idea: host A sends many packets in a short time to host B, which does not answer. The
  interval between two NS messages for B is never less than RetransTimer.

### RFC4861-AR-21

**Address resolution fails when MAX_MULTICAST_SOLICIT solicitations get no Neighbor Advertisement.**

> "If no Neighbor Advertisement is received after MAX_MULTICAST_SOLICIT
> solicitations, address resolution has failed." — §7.2.2, `rfc4861.txt:3454-3455`

- Strength: description. Class: wire.
- Check idea: host A resolves host B, which does not answer. A sends MAX_MULTICAST_SOLICIT NS
  messages for B (3 by default), and then no more for that resolution.

### RFC4861-AR-22

**When address resolution fails, the sender returns an ICMP Destination Unreachable, code 3, for each queued packet.**

> "The sender MUST return
> ICMP destination unreachable indications with code 3 (Address
> Unreachable) for each packet queued awaiting address resolution." — §7.2.2,
> `rfc4861.txt:3455-3457`

- Strength: must. Class: error-signal.
- Check idea: router R1 forwards packets from host S to host B, which does not answer. After
  the last NS, S receives one Destination Unreachable, code 3 (Address Unreachable), for each
  queued packet.

### RFC4861-AR-23

**A node silently discards a valid NS whose Target Address is not its own valid or tentative address and not an address that it is a proxy for.**

> "A valid Neighbor Solicitation that does not meet any of the following
> requirements MUST be silently discarded:
>
> - The Target Address is a "valid" unicast or anycast address
> assigned to the receiving interface [ADDRCONF],
>
> - The Target Address is a unicast or anycast address for which the
> node is offering proxy service, or
>
> - The Target Address is a "tentative" address on which Duplicate
> Address Detection is being performed [ADDRCONF]." — §7.2.3,
> `rfc4861.txt:3461-3468`, `rfc4861.txt:3479-3480`

- Strength: must. Class: end-to-end.
- Check idea: host A receives a valid NS for an address that A does not have and is not a
  proxy for. A sends no NA, and its Neighbor Cache does not change.

### RFC4861-AR-24

**When an NS from a specified source carries a Source Link-Layer Address option, the recipient creates or updates the entry for the source.**

> "If the Source Address is not the unspecified
> address and, on link layers that have addresses, the solicitation
> includes a Source Link-Layer Address option, then the recipient
> SHOULD create or update the Neighbor Cache entry for the IP Source
> Address of the solicitation." — §7.2.3, `rfc4861.txt:3484-3488`

- Strength: should. Class: end-to-end.
- Check idea: host A has no entry for host B. B sends a multicast NS for A with a Source
  Link-Layer Address option. A sends its unicast NA to that link-layer address with no NS of
  its own.

### RFC4861-AR-25

**If no entry exists, the recipient creates one in the STALE state.**

> "If an entry does not already exist, the
> node SHOULD create a new one and set its reachability state to STALE
> as specified in Section 7.3.3." — §7.2.3, `rfc4861.txt:3488-3490`

- Strength: should. Class: internal.
- Check idea: after that NS, the entry of A for B is STALE. A sends a packet to B and gets no
  reachability confirmation. After DELAY_FIRST_PROBE_TIME, A sends a unicast NS to B.

### RFC4861-AR-26

**If the entry exists with another link-layer address, the recipient replaces the address and sets the entry to STALE.**

> "If an entry already exists, and the
> cached link-layer address differs from the one in the received Source
> Link-Layer option, the cached address should be replaced by the
> received address, and the entry's reachability state MUST be set to
> STALE." — §7.2.3, `rfc4861.txt:3490-3494`

- Strength: description (replace the address), must (set STALE). Class: internal.
- Check idea: host A has a REACHABLE entry for host B with link-layer address X. B sends an NS
  with Source Link-Layer Address Y. A sends its next frames for B to Y, and the entry is STALE.

### RFC4861-AR-27

**A Neighbor Cache entry that an NS creates has the IsRouter flag FALSE.**

> "If a Neighbor Cache entry is created, the IsRouter flag SHOULD be set
> to FALSE." — §7.2.3, `rfc4861.txt:3496-3497`

- Strength: should. Class: internal.
- Check idea: router R1 has no entry at host A. R1 sends A an NS with a Source Link-Layer
  Address option. The new entry of A for R1 has IsRouter FALSE.

### RFC4861-AR-28

**An NS does not change the IsRouter flag of an entry that already exists.**

> "If a Neighbor Cache entry already exists, its
> IsRouter flag MUST NOT be modified." — §7.2.3, `rfc4861.txt:3502-3503`

- Strength: must not. Class: end-to-end.
- Check idea: host A uses router R1 as its default router, and the entry for R1 has IsRouter
  TRUE. R1 sends A an NS. A keeps R1 in its Default Router List and continues to send
  off-link traffic through R1.

### RFC4861-AR-29

**An NS from the unspecified address does not create or update a Neighbor Cache entry.**

> "If the Source Address is the unspecified address, the node MUST NOT
> create or update the Neighbor Cache entry." — §7.2.3, `rfc4861.txt:3505-3506`

- Strength: must not. Class: internal.
- Check idea: host A receives an NS from the unspecified address for one of its addresses.
  The Neighbor Cache of A does not change.

### RFC4861-AR-30

**After it updates the Neighbor Cache, the node sends a Neighbor Advertisement in response.**

> "After any updates to the Neighbor Cache, the node sends a Neighbor
> Advertisement response as described in the next section." — §7.2.3,
> `rfc4861.txt:3508-3509`

- Strength: description. Class: wire.
- Check idea: host A receives a valid NS for its address with a Source Link-Layer Address
  option. A creates the entry for the source, and then an NA leaves A.

### RFC4861-AR-31

**A node sends a Neighbor Advertisement in response to a valid NS for one of its assigned addresses.**

> "A node sends a Neighbor Advertisement in response to a valid Neighbor
> Solicitation targeting one of the node's assigned addresses." — §7.2.4,
> `rfc4861.txt:3513-3514`

- Strength: description. Class: wire.
- Check idea: host B sends a valid NS for an address of host A. A sends one NA.

### RFC4861-AR-32

**The Target Address of the NA is a copy of the Target Address of the NS.**

> "The Target Address of the advertisement is copied from the Target Address
> of the solicitation." — §7.2.4, `rfc4861.txt:3514-3516`

- Strength: description. Class: wire.
- Check idea: host B sends an NS for an address of host A. The Target Address of the NA of A
  is the same as the Target Address of the NS.

### RFC4861-AR-33

**An NA that answers a unicast NS can omit the Target Link-Layer Address option.**

> "If the solicitation's IP Destination Address is
> not a multicast address, the Target Link-Layer Address option MAY be
> omitted; the neighboring node's cached value must already be current
> in order for the solicitation to have been received." — §7.2.4,
> `rfc4861.txt:3516-3519`

- Strength: may. Class: wire.
- Check idea: host B sends a unicast NS to host A. Record if the NA of A carries a Target
  Link-Layer Address option. B accepts the NA in both cases.

### RFC4861-AR-34

**An NA that answers a multicast NS includes the Target Link-Layer Address option.**

> "If the
> solicitation's IP Destination Address is a multicast address, the
> Target Link-Layer option MUST be included in the advertisement." — §7.2.4,
> `rfc4861.txt:3519-3521`

- Strength: must. Class: wire.
- Check idea: host B sends a multicast NS for an address of host A. The NA of A carries a
  Target Link-Layer Address option with the link-layer address of A.

### RFC4861-AR-35

**A router sets the Router flag of a solicited NA to one, and a host sets it to zero.**

> "Furthermore, if the node is a router, it MUST set the Router flag to
> one; otherwise, it MUST set the flag to zero." — §7.2.4, `rfc4861.txt:3522-3523`

- Strength: must. Class: wire.
- Check idea: a router and a host each answer an NS. The NA of the router has Router flag 1,
  and the NA of the host has Router flag 0.

### RFC4861-AR-36

**The Override flag is zero when the target is an anycast or proxied address, or when the NA has no Target Link-Layer Address option.**

> "If the Target Address is either an anycast address or a unicast
> address for which the node is providing proxy service, or the Target
> Link-Layer Address option is not included, the Override flag SHOULD
> be set to zero." — §7.2.4, `rfc4861.txt:3535-3538`

- Strength: should. Class: wire.
- Check idea: a node answers an NS for its anycast address. A proxy answers an NS for a proxied
  address. A host answers a unicast NS with an NA without a Target Link-Layer Address option.
  Each NA has Override flag 0.

### RFC4861-AR-37

**In all other NAs that answer an NS, the Override flag is one.**

> "Otherwise, the Override flag SHOULD be set to one." — §7.2.4, `rfc4861.txt:3538`

- Strength: should. Class: wire.
- Check idea: host A answers a multicast NS for its own unicast address. The NA has Override
  flag 1.

### RFC4861-AR-38

**An NA that answers an NS from the unspecified address has the Solicited flag zero and goes to the all-nodes address.**

> "If the source of the solicitation is the unspecified address, the
> node MUST set the Solicited flag to zero and multicast the
> advertisement to the all-nodes address." — §7.2.4, `rfc4861.txt:3544-3546`

- Strength: must. Class: wire.
- Check idea: host B sends an NS from the unspecified address for an address of host A. A
  sends an NA with Solicited flag 0 to the all-nodes multicast address.

### RFC4861-AR-39

**Any other solicited NA has the Solicited flag one and goes by unicast to the Source Address of the NS.**

> "Otherwise, the node MUST set
> the Solicited flag to one and unicast the advertisement to the Source
> Address of the solicitation." — §7.2.4, `rfc4861.txt:3546-3548`

- Strength: must. Class: wire.
- Check idea: host B sends an NS from its own address for an address of host A. A sends an NA
  with Solicited flag 1, and its IP Destination Address is the address of B.

### RFC4861-AR-40

**A node delays the NA for an anycast target by a random time between 0 and MAX_ANYCAST_DELAY_TIME seconds.**

> "If the Target Address is an anycast address, the sender SHOULD delay
> sending a response for a random time between 0 and
> MAX_ANYCAST_DELAY_TIME seconds." — §7.2.4, `rfc4861.txt:3550-3552`

- Strength: should. Class: wire.
- Check idea: host B sends several NS messages for an anycast address of host A. The delay from
  each NS to its NA is random, and it is not more than MAX_ANYCAST_DELAY_TIME (1 second).

### RFC4861-AR-41

**A node that answers an NS but has no link-layer address for the source first resolves that address with a multicast NS.**

> "Because unicast Neighbor Solicitations are not required to include a
> Source Link-Layer Address, it is possible that a node sending a
> solicited Neighbor Advertisement does not have a corresponding link-
> layer address for its neighbor in its Neighbor Cache. In such
> situations, a node will first have to use Neighbor Discovery to
> determine the link-layer address of its neighbor (i.e., send out a
> multicast Neighbor Solicitation)." — §7.2.4, `rfc4861.txt:3554-3560`

- Strength: description. Class: wire.
- Check idea: host B has no entry at host A. B sends A a unicast NS without a Source Link-Layer
  Address option. A sends a multicast NS for B before its NA to B.

### RFC4861-AR-42

**A node silently discards a valid NA for a target that has no Neighbor Cache entry.**

> "When a valid Neighbor Advertisement is received (either solicited or
> unsolicited), the Neighbor Cache is searched for the target's entry.
> If no entry exists, the advertisement SHOULD be silently discarded." — §7.2.5,
> `rfc4861.txt:3564-3566`

- Strength: should. Class: end-to-end.
- Check idea: host B sends an unsolicited NA to host A, which has no entry for B. A creates no
  entry: its first packet to B later comes after a multicast NS for B.

### RFC4861-AR-43

**For an INCOMPLETE entry, a node silently discards an NA without a Target Link-Layer Address option on a link that has addresses.**

> "If the target's Neighbor Cache entry is in the INCOMPLETE state when
> the advertisement is received, one of two things happens. If the
> link layer has addresses and no Target Link-Layer Address option is
> included, the receiving node SHOULD silently discard the received
> advertisement." — §7.2.5, `rfc4861.txt:3576-3580`

- Strength: should. Class: end-to-end.
- Check idea: host A resolves host B, and an NA for B without a Target Link-Layer Address
  option arrives. A keeps the queued packet and continues to send NS messages for B.

### RFC4861-AR-44

**For an INCOMPLETE entry, a node records the link-layer address from the NA and sends the queued packets.**

> "Otherwise, the receiving node performs the following
> steps:" — §7.2.5, `rfc4861.txt:3580-3581`; "- It records the link-layer address in the
> Neighbor Cache entry." — `rfc4861.txt:3591`; "- It sends any packets queued for the
> neighbor awaiting address
> resolution." — `rfc4861.txt:3599-3600`

- Strength: description. Class: wire.
- Check idea: host A resolves host B, and an NA with a Target Link-Layer Address option
  arrives. The queued packets leave A to that link-layer address.

### RFC4861-AR-45

**For an INCOMPLETE entry, the entry becomes REACHABLE if the NA has the Solicited flag set, and STALE if not.**

> "- If the advertisement's Solicited flag is set, the state of the
> entry is set to REACHABLE; otherwise, it is set to STALE." — §7.2.5,
> `rfc4861.txt:3593-3594`

- Strength: description. Class: internal.
- Check idea: host A resolves host B. A solicited NA makes the entry REACHABLE. An unsolicited
  NA makes it STALE, and A probes B with a unicast NS after its next packet to B.

### RFC4861-AR-46

**For an INCOMPLETE entry, the IsRouter flag takes the value of the Router flag of the NA.**

> "- It sets the IsRouter flag in the cache entry based on the Router
> flag in the received advertisement." — §7.2.5, `rfc4861.txt:3596-3597`

- Strength: description. Class: internal.
- Check idea: host A resolves router R1, and the NA of R1 has Router flag 1. The entry of A
  for R1 has IsRouter TRUE.

### RFC4861-AR-47

**A node ignores the Override flag of an NA for an INCOMPLETE entry.**

> "Note that the Override flag is ignored if the entry is in the
> INCOMPLETE state." — §7.2.5, `rfc4861.txt:3602-3603`

- Strength: description. Class: end-to-end.
- Check idea: host A resolves host B, and B answers with Override flag 0 and a Target
  Link-Layer Address option. A accepts the address and sends the queued packets.

### RFC4861-AR-48

**If the Override flag is clear and the link-layer address is different, a REACHABLE entry becomes STALE and does not change in any other way.**

> "If the target's Neighbor Cache entry is in any state other than
> INCOMPLETE when the advertisement is received, the following actions
> take place:
>
> I. If the Override flag is clear and the supplied link-layer address
> differs from that in the cache, then one of two actions takes
> place:
> a. If the state of the entry is REACHABLE, set it to STALE, but
> do not update the entry in any other way." — §7.2.5, `rfc4861.txt:3605-3613`

- Strength: description. Class: end-to-end.
- Check idea: host A has a REACHABLE entry for host B with address X. B sends an NA with
  Override flag 0 and Target Link-Layer Address Y. A keeps X, and the entry becomes STALE: A
  later probes X with a unicast NS.

### RFC4861-AR-49

**If the Override flag is clear and the link-layer address is different, an entry that is not REACHABLE does not change.**

> "I. If the Override flag is clear and the supplied link-layer address
> differs from that in the cache, then one of two actions takes
> place:" — §7.2.5, `rfc4861.txt:3609-3611`; "b. Otherwise, the received advertisement
> should be ignored and
> MUST NOT update the cache." — `rfc4861.txt:3614-3615`

- Strength: must not. Class: end-to-end.
- Check idea: host A has a STALE entry for host B with address X. B sends an NA with Override
  flag 0 and address Y. A continues to send frames for B to X, and the entry stays STALE.

### RFC4861-AR-50

**If the Override flag is set, or the address is the same, or no Target Link-Layer Address option is present, the NA updates the entry, and a different supplied address goes into the cache.**

> "II. If the Override flag is set, or the supplied link-layer address
> is the same as that in the cache, or no Target Link-Layer Address
> option was supplied, the received advertisement MUST update the
> Neighbor Cache entry as follows:
>
> - The link-layer address in the Target Link-Layer Address option
> MUST be inserted in the cache (if one is supplied and differs
> from the already recorded address)." — §7.2.5, `rfc4861.txt:3617-3624`

- Strength: must. Class: end-to-end.
- Check idea: host A has an entry for host B with address X. B sends an NA with Override flag 1
  and address Y. A sends its next frames for B to Y.

### RFC4861-AR-51

**In that update, an NA with the Solicited flag set makes the entry REACHABLE.**

> "- If the Solicited flag is set, the state of the entry MUST be
> set to REACHABLE." — §7.2.5, `rfc4861.txt:3626-3627`

- Strength: must. Class: internal.
- Check idea: host A has a STALE entry for host B and probes B with a unicast NS. B answers
  with a solicited NA. The entry becomes REACHABLE, and A sends no more probes to B for the
  reachable time.

### RFC4861-AR-52

**In that update, an NA with the Solicited flag zero that changes the address makes the entry STALE; otherwise the state does not change.**

> "If the Solicited flag is zero and the link-
> layer address was updated with a different address, the state
> MUST be set to STALE. Otherwise, the entry's state remains
> unchanged." — §7.2.5, `rfc4861.txt:3627-3630`

- Strength: must. Class: internal.
- Check idea: host A has a REACHABLE entry for host B with address X. An unsolicited NA from B
  with Override flag 1 and address Y makes the entry STALE. The same NA with address X leaves
  the entry REACHABLE.

### RFC4861-AR-53

**A node sets the Solicited flag only in an NA that answers an NS.**

> "An advertisement's Solicited flag should only be set if the
> advertisement is a response to a Neighbor Solicitation." — §7.2.5,
> `rfc4861.txt:3632-3633`

- Strength: should (lower case). Class: wire.
- Check idea: record all NAs of host A. Only the NAs that answer an NS have Solicited flag 1.

### RFC4861-AR-54

**When an unsolicited NA changes the address that a node uses, the node verifies the new path when it sends the next packet.**

> "If the urgent information
> indicates a change from what a node is currently using, the
> node should verify the reachability of the (new) path when it
> sends the next packet." — §7.2.5, `rfc4861.txt:3647-3650`

- Strength: should (lower case). Class: end-to-end.
- Check idea: host B sends host A an unsolicited NA with Override flag 1 and a new address Y.
  After A sends its next packet to B, A probes Y with a unicast NS, unless it gets a
  reachability confirmation first.

### RFC4861-AR-55

**In that update, the IsRouter flag of the entry takes the value of the Router flag of the NA.**

> "- The IsRouter flag in the cache entry MUST be set based on the
> Router flag in the received advertisement." — §7.2.5, `rfc4861.txt:3654-3655`

- Strength: must. Class: internal.
- Check idea: host A has an entry for node B with IsRouter FALSE. B sends an NA with Router
  flag 1 and Override flag 1. The entry has IsRouter TRUE.

### RFC4861-AR-56

**When IsRouter changes from TRUE to FALSE, the node removes that router from the Default Router List and updates the Destination Cache.**

> "In those cases
> where the IsRouter flag changes from TRUE to FALSE as a result
> of this update, the node MUST remove that router from the
> Default Router List and update the Destination Cache entries
> for all destinations using that neighbor as a router as
> specified in Section 7.3.3." — §7.2.5, `rfc4861.txt:3655-3660`

- Strength: must. Class: end-to-end.
- Check idea: host A uses router R1 as its default router, and router R2 is also in its list.
  R1 sends an NA with Router flag 0. A no longer sends off-link traffic through R1; it uses R2.

### RFC4861-AR-57

**A node with a new link-layer address can send up to MAX_NEIGHBOR_ADVERTISEMENT unsolicited NAs to the all-nodes address.**

> "In some cases, a node may be able to determine that its link-layer
> address has changed (e.g., hot-swap of an interface card) and may
> wish to inform its neighbors of the new link-layer address quickly.
> In such cases, a node MAY send up to MAX_NEIGHBOR_ADVERTISEMENT
> unsolicited Neighbor Advertisement messages to the all-nodes
> multicast address." — §7.2.6, `rfc4861.txt:3674-3679`

- Strength: may. Class: wire.
- Check idea: change the link-layer address of host A. A sends at most
  MAX_NEIGHBOR_ADVERTISEMENT (3) unsolicited NAs to the all-nodes multicast address.

### RFC4861-AR-58

**The interval between these unsolicited NAs is at least RetransTimer.**

> "These advertisements MUST be separated by at
> least RetransTimer seconds." — §7.2.6, `rfc4861.txt:3679-3680`

- Strength: must. Class: wire.
- Check idea: change the link-layer address of host A. The interval between two unsolicited
  NAs of A is RetransTimer or more.

### RFC4861-AR-59

**The unsolicited NA has an address of the interface as Target Address and the new link-layer address in the Target Link-Layer Address option.**

> "The Target Address field in the unsolicited advertisement is set to
> an IP address of the interface, and the Target Link-Layer Address
> option is filled with the new link-layer address." — §7.2.6, `rfc4861.txt:3682-3684`

- Strength: description. Class: wire.
- Check idea: change the link-layer address of host A. Each unsolicited NA of A has an address
  of that interface as Target Address and the new link-layer address in the option.

### RFC4861-AR-60

**The unsolicited NA has the Solicited flag zero.**

> "The Solicited flag
> MUST be set to zero, in order to avoid confusing the Neighbor
> Unreachability Detection algorithm." — §7.2.6, `rfc4861.txt:3684-3686`

- Strength: must. Class: wire.
- Check idea: change the link-layer address of host A. Each unsolicited NA of A has Solicited
  flag 0.

### RFC4861-AR-61

**A router sets the Router flag of an unsolicited NA to one, and a host sets it to zero.**

> "If the node is a router, it MUST
> set the Router flag to one; otherwise, it MUST set it to zero." — §7.2.6,
> `rfc4861.txt:3686-3687`

- Strength: must. Class: wire.
- Check idea: change the link-layer address of a router and of a host. The unsolicited NAs of
  the router have Router flag 1, and those of the host have Router flag 0.

### RFC4861-AR-62

**The Override flag of the unsolicited NA can be zero or one.**

> "The
> Override flag MAY be set to either zero or one." — §7.2.6, `rfc4861.txt:3687-3688`

- Strength: may. Class: wire.
- Check idea: change the link-layer address of host A and record the Override flag of its
  unsolicited NAs. The values 0 and 1 are both correct.

### RFC4861-AR-63

**A neighbor that receives an unsolicited NA with the Override flag set makes its entry STALE and installs the new link-layer address.**

> "In either case,
> neighboring nodes will immediately change the state of their Neighbor
> Cache entries for the Target Address to STALE, prompting them to
> verify the path for reachability. If the Override flag is set to
> one, neighboring nodes will install the new link-layer address in
> their caches." — §7.2.6, `rfc4861.txt:3688-3693`

- Strength: description. Class: end-to-end.
- Check idea: host B has a REACHABLE entry for host A. A sends an unsolicited NA with Override
  flag 1 and a new address. B sends its next frames for A to the new address, and B probes it.

### RFC4861-AR-64

**A neighbor that receives an unsolicited NA with the Override flag clear makes its entry STALE, keeps the old address and probes it.**

> "In either case,
> neighboring nodes will immediately change the state of their Neighbor
> Cache entries for the Target Address to STALE" — §7.2.6, `rfc4861.txt:3688-3690`;
> "Otherwise, they will ignore the new link-layer
> address, choosing instead to probe the cached address." — `rfc4861.txt:3693-3694`

- Strength: description. Class: end-to-end.
- Check idea: host B has a REACHABLE entry for host A. A sends an unsolicited NA with Override
  flag 0 and a new address. B keeps the old address, and after its next packet to A it probes
  the old address with a unicast NS.

### RFC4861-AR-65

**A node with more than one address on an interface can multicast a separate NA for each address.**

> "A node that has multiple IP addresses assigned to an interface MAY
> multicast a separate Neighbor Advertisement for each address." — §7.2.6,
> `rfc4861.txt:3703-3704`

- Strength: may. Class: wire.
- Check idea: host A has three addresses and changes its link-layer address. Record the
  unsolicited NAs of A; there can be one for each address.

### RFC4861-AR-66

**The node puts a small delay between these NAs.**

> "In
> such a case, the node SHOULD introduce a small delay between the
> sending of each advertisement to reduce the probability of the
> advertisements being lost due to congestion." — §7.2.6, `rfc4861.txt:3704-3707`

- Strength: should. Class: wire.
- Check idea: host A has three addresses and changes its link-layer address. A small delay
  separates the unsolicited NAs for the different addresses.

### RFC4861-AR-67

**A proxy can multicast NAs when its link-layer address changes or when it starts to act as proxy for an address.**

> "A proxy MAY multicast Neighbor Advertisements when its link-layer
> address changes or when it is configured (by system management or
> other mechanisms) to proxy for an address." — §7.2.6, `rfc4861.txt:3709-3711`

- Strength: may. Class: wire.
- Check idea: configure router R1 as proxy for address P. Record if R1 multicasts an
  unsolicited NA for P with its own link-layer address.

### RFC4861-AR-68

**A node that belongs to an anycast address can multicast unsolicited NAs for that address when its link-layer address changes.**

> "Also, a node belonging to an anycast address MAY multicast
> unsolicited Neighbor Advertisements for the anycast address when the
> node's link-layer address changes." — §7.2.6, `rfc4861.txt:3720-3722`

- Strength: may. Class: wire.
- Check idea: host A holds an anycast address and changes its link-layer address. Record if A
  multicasts an unsolicited NA for the anycast address with the new link-layer address.

## Anycast and proxy advertisements

### RFC4861-ANY-1

**Address resolution and Neighbor Unreachability Detection treat an anycast address like a unicast address.**

> "From the perspective of Neighbor Discovery, anycast addresses are
> treated just like unicast addresses in most cases. Because an
> anycast address is syntactically the same as a unicast address, nodes
> performing address resolution or Neighbor Unreachability Detection on
> an anycast address treat it as if it were a unicast address. No
> special processing takes place." — §7.2.7, `rfc4861.txt:3734-3739`

- Strength: description. Class: end-to-end.
- Check idea: host A sends a packet to an anycast address that host B holds. A sends a
  multicast NS to the solicited-node address of the anycast address, as for a unicast address,
  and then sends the packet to B.

### RFC4861-ANY-2

**A node with an anycast address delays an NA that answers an NS for it by a random time between 0 and MAX_ANYCAST_DELAY_TIME.**

> "Nodes that have an anycast address assigned to an interface treat
> them exactly the same as if they were unicast addresses with two
> exceptions. First, Neighbor Advertisements sent in response to a
> Neighbor Solicitation SHOULD be delayed by a random time between 0
> and MAX_ANYCAST_DELAY_TIME to reduce the probability of network
> congestion." — §7.2.7, `rfc4861.txt:3741-3746`

- Strength: should. Class: wire.
- Check idea: host A sends several NS messages for an anycast address of host B. The delay from
  each NS to the NA of B is random, and it is not more than MAX_ANYCAST_DELAY_TIME.

### RFC4861-ANY-3

**A node sets the Override flag to 0 in an NA for its anycast address.**

> "Second, the Override flag in Neighbor Advertisements
> SHOULD be set to 0, so that when multiple advertisements are
> received, the first received advertisement is used rather than the
> most recently received advertisement." — §7.2.7, `rfc4861.txt:3746-3749`

- Strength: should. Class: wire.
- Check idea: hosts B and C hold the same anycast address, and host A resolves it. Both NAs
  have Override flag 0, and A sends its packets to the node whose NA arrives first.

### RFC4861-ANY-4

**Neighbor Unreachability Detection finds quickly when the binding for an anycast address is no longer valid.**

> "As with unicast addresses, Neighbor Unreachability Detection ensures
> that a node quickly detects when the current binding for an anycast
> address becomes invalid." — §7.2.7, `rfc4861.txt:3759-3761`

- Strength: description. Class: end-to-end.
- Check idea: host A sends to an anycast address that hosts B and C hold, and it uses B. B
  leaves the link. The probes of A fail, A resolves the address again, and C gets the packets.

### RFC4861-ANY-5

**A router can act as proxy for other nodes, and it announces this with Neighbor Advertisements.**

> "Under limited circumstances, a router MAY proxy for one or more other
> nodes, that is, through Neighbor Advertisements indicate that it is
> willing to accept packets not explicitly addressed to itself." — §7.2.8,
> `rfc4861.txt:3765-3767`

- Strength: may. Class: wire.
- Check idea: configure router R1 as proxy for off-link node M. Host A resolves M. R1 answers
  with an NA that holds the link-layer address of R1, and the packets of A for M go to R1.

### RFC4861-ANY-6

**A proxy joins the solicited-node multicast address of each address that it is a proxy for.**

> "A proxy MUST join the solicited-node multicast address(es) that
> correspond to the IP address(es) assigned to the node for which it is
> proxying." — §7.2.8, `rfc4861.txt:3772-3774`

- Strength: must. Class: end-to-end.
- Check idea: router R1 is proxy for address P. Host A sends an NS for P to the solicited-node
  address of P. R1 answers it.

### RFC4861-ANY-7

**The proxy joins these groups with a multicast listener discovery protocol, such as MLD or MLDv2.**

> "This SHOULD be done using a multicast listener discovery
> protocol such as [MLD] or [MLDv2]." — §7.2.8, `rfc4861.txt:3774-3775`

- Strength: should. Class: wire.
- Check idea: configure router R1 as proxy for address P. An MLD or MLDv2 report of R1 for the
  solicited-node address of P appears on the link.

### RFC4861-ANY-8

**Every solicited proxy NA has the Override flag set to zero.**

> "All solicited proxy Neighbor Advertisement messages MUST have the
> Override flag set to zero." — §7.2.8, `rfc4861.txt:3777-3778`

- Strength: must. Class: wire.
- Check idea: router R1 is proxy for host M, and M is also on the link. Host A resolves M. The
  NA of R1 has Override flag 0, and A sends its packets for M to the link-layer address in the
  NA of M.

### RFC4861-ANY-9

**A proxy can send unsolicited NAs with the Override flag set to one.**

> "A proxy MAY send unsolicited advertisements with the
> Override flag set to one as specified in Section 7.2.6, but doing so
> may cause the proxy advertisement to override a valid entry created
> by the node itself." — §7.2.8, `rfc4861.txt:3781-3784`

- Strength: may. Class: wire.
- Check idea: configure router R1 as proxy for address P. Record the Override flag of the
  unsolicited NAs of R1 for P; the value 1 is correct.

### RFC4861-ANY-10

**A proxy delays its NA that answers an NS by a random time between 0 and MAX_ANYCAST_DELAY_TIME seconds, unless it is the only proxy.**

> "Finally, when sending a proxy advertisement in response to a Neighbor
> Solicitation, the sender should delay its response by a random time
> between 0 and MAX_ANYCAST_DELAY_TIME seconds to avoid collisions due
> to multiple responses sent by several proxies. However, in some
> cases (e.g., Mobile IPv6) where only one proxy is present, such delay
> is not necessary." — §7.2.8, `rfc4861.txt:3786-3791`

- Strength: should (lower case). Class: wire.
- Check idea: routers R1 and R2 are both proxy for address P, and host A sends several NS
  messages for P. The delay from each NS to each proxy NA is random, and it is not more than
  MAX_ANYCAST_DELAY_TIME.

## Validation of Redirect messages

### RFC4861-RDVAL-1

**A host silently discards a Redirect whose IP Source Address is not a link-local address.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- IP Source Address is a link-local address." — `rfc4861.txt:4100`

- Strength: must. Class: end-to-end.
- Check idea: host A sends packets for destination D through router R1. R1 sends A a Redirect
  for D to router R2 from a global address of R1. A continues to send packets for D to R1.

### RFC4861-RDVAL-2

**A router uses its link-local address as the source of its Router Advertisements and Redirects.**

> "Routers must use
> their link-local address as the source for Router Advertisement
> and Redirect messages so that hosts can uniquely identify
> routers." — §8.1, `rfc4861.txt:4100-4103`

- Strength: must (lower case). Class: wire.
- Check idea: router R1 has global addresses on the link and sends Redirects. The IP Source
  Address of each Redirect is the link-local address of R1.

### RFC4861-RDVAL-3

**A host silently discards a Redirect whose IP Hop Limit is not 255.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- The IP Hop Limit field has a value of 255, i.e., the packet
> could not possibly have been forwarded by a router." — `rfc4861.txt:4105-4106`

- Strength: must. Class: end-to-end.
- Check idea: router R1, the first hop of host A for D, sends a Redirect for D with Hop Limit
  254 and all other fields valid. A continues to send packets for D to R1.

### RFC4861-RDVAL-4

**A host silently discards a Redirect with an ICMP Checksum that is not valid.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- ICMP Checksum is valid." — `rfc4861.txt:4108`

- Strength: must. Class: end-to-end.
- Check idea: router R1, the first hop of host A for D, sends a Redirect for D with a wrong
  ICMP Checksum. A continues to send packets for D to R1.

### RFC4861-RDVAL-5

**A host silently discards a Redirect with an ICMP Code other than 0.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- ICMP Code is 0." — `rfc4861.txt:4110`

- Strength: must. Class: end-to-end.
- Check idea: router R1, the first hop of host A for D, sends a Redirect for D with Code 1. A
  continues to send packets for D to R1.

### RFC4861-RDVAL-6

**A host silently discards a Redirect with an ICMP length of less than 40 octets.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- ICMP length (derived from the IP length) is 40 or more octets." — `rfc4861.txt:4112`

- Strength: must. Class: end-to-end.
- Check idea: router R1, the first hop of host A for D, sends a Redirect for D that is cut to
  32 octets of ICMP length. A continues to send packets for D to R1.

### RFC4861-RDVAL-7

**A host silently discards a Redirect whose IP Source Address is not the current first-hop router for the destination.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- The IP source address of the Redirect is the same as the current
> first-hop router for the specified ICMP Destination Address." — `rfc4861.txt:4114-4115`

- Strength: must. Class: end-to-end.
- Check idea: host A sends packets for D through router R1. Router R2 sends A a Redirect for D
  to R2. A continues to send packets for D to R1.

### RFC4861-RDVAL-8

**A host silently discards a Redirect whose ICMP Destination Address is a multicast address.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- The ICMP Destination Address field in the redirect message does
> not contain a multicast address." — `rfc4861.txt:4117-4118`

- Strength: must. Class: end-to-end.
- Check idea: host A sends packets to a multicast group G through router R1. R1 sends A a
  Redirect for G to router R2. A continues to send packets for G as before.

### RFC4861-RDVAL-9

**A host silently discards a Redirect whose Target Address is neither link-local nor equal to the Destination Address.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- The ICMP Target Address is either a link-local address (when
> redirected to a router) or the same as the ICMP Destination
> Address (when redirected to the on-link destination)." — `rfc4861.txt:4120-4122`

- Strength: must. Class: end-to-end.
- Check idea: router R1, the first hop of host A for D, sends a Redirect for D whose Target
  Address is a global address of router R2. A continues to send packets for D to R1.

### RFC4861-RDVAL-10

**A host silently discards a Redirect that has an option with length zero.**

> "A host MUST silently discard any received Redirect message that does
> not satisfy all of the following validity checks:" — §8.1, `rfc4861.txt:4097-4098`;
> "- All included options have a length that is greater than zero." — `rfc4861.txt:4124`

- Strength: must. Class: end-to-end.
- Check idea: router R1, the first hop of host A for D, sends a Redirect for D with an option of
  length 0. A continues to send packets for D to R1.

### RFC4861-RDVAL-11

**A host ignores the Reserved field and unknown options of a Redirect.**

> "The contents of the Reserved field, and of any unrecognized options,
> MUST be ignored." — §8.1, `rfc4861.txt:4126-4127`

- Strength: must. Class: end-to-end.
- Check idea: router R1, the first hop of host A for D, sends a valid Redirect for D to router
  R2 with a Reserved field that is not zero and an option of unknown type. A sends its next
  packets for D to R2.

### RFC4861-RDVAL-12

**A host ignores defined options that do not belong to a Redirect and processes the Redirect as usual.**

> "The contents of any defined options that are not specified to be used
> with Redirect messages MUST be ignored and the packet processed as
> normal." — §8.1, `rfc4861.txt:4131-4133`

- Strength: must. Class: end-to-end.
- Check idea: router R1 sends host A a valid Redirect for D to router R2 that also carries a
  Prefix Information option and an MTU option. A sends its next packets for D to R2, adds no
  prefix and keeps its link MTU.

### RFC4861-RDVAL-13

**The Target Link-Layer Address option and the Redirected Header option are the only defined options of a Redirect.**

> "The only defined options that may appear are the Target
> Link-Layer Address option and the Redirected Header option." — §8.1,
> `rfc4861.txt:4133-4134`

- Strength: may (lower case). Class: wire.
- Check idea: record the Redirects of router R1. Each option in them is a Target Link-Layer
  Address option or a Redirected Header option.

### RFC4861-RDVAL-14

**A host does not discard a Redirect only because its Target Address is outside the prefixes of the link.**

> "A host MUST NOT consider a redirect invalid just because the Target
> Address of the redirect is not covered under one of the link's
> prefixes. Part of the semantics of the Redirect message is that the
> Target Address is on-link." — §8.1, `rfc4861.txt:4136-4139`

- Strength: must not. Class: end-to-end.
- Check idea: host B is on the link of host A, but its address is outside every on-link prefix.
  Router R1 sends A a valid Redirect with Target Address and Destination Address both B. A
  sends its next packets for B directly to B.

## Router sending Redirects

### RFC4861-RDR-1

**A router sends a Redirect to point a host to a better first-hop router, or to tell the host that the destination is on-link; for an on-link destination, the Target Address equals the Destination Address.**

> "Redirect messages are sent by routers to redirect a host to a better
> first-hop router for a specific destination or to inform hosts that a
> destination is in fact a neighbor (i.e., on-link). The latter is
> accomplished by having the ICMP Target Address be equal to the ICMP
> Destination Address." — §8, `rfc4861.txt:4066-4070`

- Strength: description. Class: wire.
- Check idea: host A sends packets for on-link host B through router R1. R1 sends A a Redirect
  whose Target Address and Destination Address are both B.

### RFC4861-RDR-2

**A router knows the link-local address of each neighbor router, so that the Target Address of a Redirect is a link-local address.**

> "A router MUST be able to determine the link-local address for each of
> its neighboring routers in order to ensure that the target address in
> a Redirect message identifies the neighbor router by its link-local
> address." — §8, `rfc4861.txt:4072-4075`

- Strength: must. Class: wire.
- Check idea: router R1 has a route to destination D with a global address of router R2 as next
  hop. A Redirect from R1 for D has the link-local address of R2 as Target Address.

### RFC4861-RDR-3

**A router sends a Redirect, subject to a rate limit, when it forwards a packet from a neighbor to a unicast destination that has a better first hop on the same link.**

> "A router SHOULD send a redirect message, subject to rate limiting,
> whenever it forwards a packet that is not explicitly addressed to
> itself (i.e., a packet that is not source routed through the router)
> in which:
>
> - the Source Address field of the packet identifies a neighbor,
> and
>
> - the router determines (by means outside the scope of this
> specification) that a better first-hop node resides on the same
> link as the sending node for the Destination Address of the
> packet being forwarded, and
>
> - the Destination Address of the packet is not a multicast
> address." — §8.2, `rfc4861.txt:4153-4167`

- Strength: should. Class: wire.
- Check idea: host A sends packets for destination D through router R1, and router R2 on the
  same link is a better first hop for D. R1 forwards the packet and sends A a Redirect for D.

### RFC4861-RDR-4

**When the target is a router, the Target Address of the Redirect is the link-local address of that router.**

> "The transmitted redirect packet contains, consistent with the message
> format given in Section 4.5:" — §8.2, `rfc4861.txt:4169-4170`; "- In the Target Address
> field: the address to which subsequent
> packets for the destination should be sent. If the target is a
> router, that router's link-local address MUST be used." — `rfc4861.txt:4172-4174`

- Strength: must. Class: wire.
- Check idea: router R1 redirects host A for destination D to router R2. The Target Address of
  the Redirect is the link-local address of R2.

### RFC4861-RDR-5

**When the target is a host, the Target Address equals the Destination Address.**

> "If the
> target is a host, the target address field MUST be set to the
> same value as the Destination Address field." — §8.2, `rfc4861.txt:4174-4176`

- Strength: must. Class: wire.
- Check idea: router R1 redirects host A for on-link host B. The Target Address and the
  Destination Address of the Redirect are both the address of B.

### RFC4861-RDR-6

**The Destination Address field holds the destination address of the packet that caused the Redirect.**

> "- In the Destination Address field: the destination address of the
> invoking IP packet." — §8.2, `rfc4861.txt:4178-4179`

- Strength: description. Class: wire.
- Check idea: host A sends a packet for destination D through router R1, and R1 sends a
  Redirect. The Destination Address field of the Redirect is D.

### RFC4861-RDR-7

**The Redirect carries a Target Link-Layer Address option with the link-layer address of the target, if the router knows it.**

> "- In the options:
>
> o Target Link-Layer Address option: link-layer address of the
> target, if known." — §8.2, `rfc4861.txt:4181-4184`

- Strength: description. Class: wire.
- Check idea: router R1 has a Neighbor Cache entry for router R2 and redirects host A to R2.
  The Redirect carries a Target Link-Layer Address option with the link-layer address of R2.

### RFC4861-RDR-8

**The Redirected Header option holds as much of the forwarded packet as fits, and the Redirect is not larger than the IPv6 minimum MTU.**

> "o Redirected Header: as much of the forwarded packet as can
> fit without the redirect packet exceeding the minimum MTU
> required to support IPv6 as specified in [IPv6]." — §8.2, `rfc4861.txt:4186-4188`

- Strength: description. Class: wire.
- Check idea: host A sends a 1400-octet packet that causes a Redirect. The Redirect is not
  larger than 1280 octets, and its Redirected Header option fills it up to that limit, in
  units of 8 octets.

### RFC4861-RDR-9

**A router limits the rate at which it sends Redirect messages.**

> "A router MUST limit the rate at which Redirect messages are sent, in
> order to limit the bandwidth and processing costs incurred by the
> Redirect messages when the source does not correctly respond to the
> Redirects, or the source chooses to ignore unauthenticated Redirect
> messages." — §8.2, `rfc4861.txt:4190-4194`

- Strength: must. Class: wire.
- Check idea: host A ignores Redirects and sends a fast packet stream for D through router R1.
  R1 sends Redirects at a limited rate, not one Redirect for each packet.

### RFC4861-RDR-10

**A router does not update its routing table when it receives a Redirect.**

> "A router MUST NOT update its routing tables upon receipt of a
> Redirect." — §8.2, `rfc4861.txt:4197-4198`

- Strength: must not. Class: end-to-end.
- Check idea: router R1 receives a Redirect for destination D to router R2. R1 continues to
  forward packets for D to the same next hop as before.

## Host receiving Redirects

### RFC4861-RDH-1

**A host that receives a valid Redirect updates its Destination Cache, so that later traffic goes to the target.**

> "A host receiving a valid redirect SHOULD update its Destination Cache
> accordingly so that subsequent traffic goes to the specified target." — §8.3,
> `rfc4861.txt:4209-4210`

- Strength: should. Class: wire.
- Check idea: router R1 redirects host A for destination D to router R2. The next packets of A
  for D go to R2.

### RFC4861-RDH-2

**If no Destination Cache entry exists for the destination, the host creates one.**

> "If no Destination Cache entry exists for the destination, an
> implementation SHOULD create such an entry." — §8.3, `rfc4861.txt:4211-4212`

- Strength: should. Class: end-to-end.
- Check idea: host A has not sent to destination D yet, and R1 is its default router. R1 sends
  A a valid Redirect for D to router R2. The first packet of A for D goes to R2.

### RFC4861-RDH-3

**If the Redirect carries a Target Link-Layer Address option, the host creates or updates the Neighbor Cache entry of the target with that address.**

> "If the redirect contains a Target Link-Layer Address option, the host
> either creates or updates the Neighbor Cache entry for the target.
> In both cases, the cached link-layer address is copied from the
> Target Link-Layer Address option." — §8.3, `rfc4861.txt:4214-4217`

- Strength: description. Class: end-to-end.
- Check idea: host A has no entry for router R2. A Redirect for D to R2 carries the link-layer
  address of R2. A sends its next packet for D to that link-layer address with no NS.

### RFC4861-RDH-4

**A Neighbor Cache entry that a Redirect creates is in the STALE state.**

> "If a Neighbor Cache entry is
> created for the target, its reachability state MUST be set to STALE
> as specified in Section 7.3.3." — §8.3, `rfc4861.txt:4217-4219`

- Strength: must. Class: internal.
- Check idea: after that Redirect, the new entry of A for R2 is STALE. A sends a packet for D
  and gets no reachability confirmation. After DELAY_FIRST_PROBE_TIME, A sends a unicast NS to
  R2.

### RFC4861-RDH-5

**A Redirect that changes the link-layer address of an entry sets the entry to STALE; the same address leaves the state unchanged.**

> "If a cache entry already existed and
> it is updated with a different link-layer address, its reachability
> state MUST also be set to STALE. If the link-layer address is the
> same as that already in the cache, the cache entry's state remains
> unchanged." — §8.3, `rfc4861.txt:4219-4223`

- Strength: must. Class: internal.
- Check idea: host A has a REACHABLE entry for router R2 with address X. A Redirect to R2 with
  address Y makes the entry STALE with Y. A Redirect to R2 with address X leaves it REACHABLE.

### RFC4861-RDH-6

**If the Target Address equals the Destination Address, the host treats the target as on-link.**

> "If the Target and Destination Addresses are the same, the host MUST
> treat the Target as on-link." — §8.3, `rfc4861.txt:4225-4226`

- Strength: must. Class: end-to-end.
- Check idea: host A sends packets for host B through router R1. R1 sends a Redirect with
  Target Address and Destination Address both B. A resolves B with an NS and sends its next
  packets directly to B.

### RFC4861-RDH-7

**If the Target Address differs from the Destination Address, the host sets IsRouter to TRUE for the target.**

> "If the Target Address is not the same
> as the Destination Address, the host MUST set IsRouter to TRUE for
> the target." — §8.3, `rfc4861.txt:4226-4228`

- Strength: must. Class: internal.
- Check idea: router R1 redirects host A for destination D to router R2. After the Redirect,
  the entry of A for R2 has IsRouter TRUE.

### RFC4861-RDH-8

**If the Target Address equals the Destination Address, a new Neighbor Cache entry has IsRouter FALSE, and an entry that exists keeps its flag.**

> "If the Target and Destination Addresses are the same,
> however, one cannot reliably determine whether the Target Address is
> a router. Consequently, newly created Neighbor Cache entries should
> set the IsRouter flag to FALSE, while existing cache entries should
> leave the flag unchanged." — §8.3, `rfc4861.txt:4228-4232`

- Strength: should (lower case). Class: internal.
- Check idea: router R1 sends host A a Redirect with Target Address and Destination Address
  both B. A new entry for B has IsRouter FALSE. An entry for B with IsRouter TRUE keeps TRUE.

### RFC4861-RDH-9

**A Redirect applies to all flows to the destination, whatever the Flow Label in the Redirected Header option.**

> "Redirect messages apply to all flows that are being sent to a given
> destination. That is, upon receipt of a Redirect for a Destination
> Address, all Destination Cache entries to that address should be
> updated to use the specified next-hop, regardless of the contents of
> the Flow Label field that appears in the Redirected Header option." — §8.3,
> `rfc4861.txt:4236-4240`

- Strength: should (lower case). Class: end-to-end.
- Check idea: host A sends two flows with different Flow Labels to destination D through router
  R1. R1 sends a Redirect to router R2 whose Redirected Header holds a packet of one flow. The
  packets of both flows then go to R2.

### RFC4861-RDH-10

**A host does not send Redirect messages.**

> "A host MUST NOT send Redirect messages." — §8.3, `rfc4861.txt:4242`

- Strength: must not. Class: wire.
- Check idea: record all ICMPv6 messages that hosts send in a scenario where a host receives
  packets for a destination that a neighbor serves better. No host sends a Redirect.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of §4, §6,
§7.2 and §8, and every field of every message and option. What the catalog leaves out:

- **The sections outside the in-scope set.** §1 to §3 (introduction, terminology, overview),
  §5 (the conceptual model of a host), §7.1 (the validation of Neighbor Solicitations and
  Advertisements, level 3), §7.3 (Neighbor Unreachability Detection, level 4), and §9 to §13
  (option processing, protocol constants, security, renumbering, IANA). See
  [`standards.md`](../../protocol/nd/standards.md#target-level) for the level at which each one
  enters.
- **The flags of other protocols.** §4 defines no flag of Mobile IPv6 or SEND; the H flag of a
  Router Advertisement and the R flag of a Prefix Information option come from RFC 6275. The
  Reserved fields are cataloged whole.

Within the in-scope sections, these parts have no entry, with the reason:

In §4:

- `rfc4861.txt:977-978` — introduction of §4; pure explanation.
- `rfc4861.txt:1094-1096` — "the O flag is redundant and can be ignored" when M is set: the
  use of the flags by a DHCPv6 client belongs to DHCPv6, another protocol.
- `rfc4861.txt:1100-1102` — examples of other configuration information (DNS); examples.
- `rfc4861.txt:1104-1105` — note that neither M nor O means no information through DHCPv6;
  informative note about another protocol.
- `rfc4861.txt:1169-1173` — reason for the SHOULD of RFC4861-RA-27 (multihomed hosts); pure
  explanation.
- `rfc4861.txt:1375-1386` — reason for the Target Link-Layer Address rules of RFC4861-NA-24 and
  RFC4861-NA-25 ("recursion", race condition); pure explanation. The keyword line 1373 is in
  RFC4861-NA-24.
- `rfc4861.txt:1587` — "For instance, [IPv6-ETHER]."; a reference only.
- `rfc4861.txt:1642-1645` — the Prefix Length also serves address autoconfiguration, with more
  limits there; that use belongs to RFC 4862 (ADDRCONF), another protocol.
- `rfc4861.txt:1669-1670` — "The Valid Lifetime is also used by [ADDRCONF]."; the use belongs to
  RFC 4862, another protocol.
- `rfc4861.txt:1678` — "See [ADDRCONF]."; a reference only.
- `rfc4861.txt:1705-1707` — first sentence of the Prefix Information description; a summary of
  the fields that RFC4861-OPT-19 and RFC4861-OPT-21 catalog.
- `rfc4861.txt:1799-1808` — bridged links of different technologies and the use of the MTU
  option there; pure explanation of a configuration choice.
- `rfc4861.txt:985-986`, `1042-1043`, `1195-1196`, `1268-1269`, `1416-1417`, `1526-1527`,
  `1555-1556`, `1604-1605`, `1713-1714`, `1759-1760` — bit rulers and frame lines of the format
  diagrams; the field rows of each diagram are in the layout entries.
- `rfc4861.txt:1047`, `1107-1109`, `1607`, `1661-1662` — a flag reserved for another protocol:
  none is in this range. RFC 4861 defines only M and O plus 6 Reserved bits in the Router
  Advertisement, and only L and A plus 6 Reserved1 bits in the Prefix Information option. The
  Home Agent (H) flag and the Router Address (R) flag of Mobile IPv6 are not in the text; the
  entries catalog the Reserved and Reserved1 fields whole (RFC4861-RA-13, RFC4861-RA-14,
  RFC4861-OPT-22, RFC4861-OPT-23).

In §6:

- `rfc4861.txt:2099-2120` — §6 introduction: an overview of Router Discovery and Prefix
  Discovery. The behaviors come again as statements in §6.2 and §6.3. The lower-case "must"
  of line 2112 concerns Stateless Address Autoconfiguration [ADDRCONF], another protocol.
- `rfc4861.txt:2160-2162` and `rfc4861.txt:2203-2205` — a remark on future changes of the
  protocol (lower-case "may"); no behavior of a node today.
- `rfc4861.txt:2169-2170` and `rfc4861.txt:2212-2213` — terminology: the definitions of
  "valid solicitation" and "valid advertisement".
- `rfc4861.txt:2220-2224` and `rfc4861.txt:2833-2837` — the variable names are for
  demonstration only, and the defaults are for convenience; no behavior.
- `rfc4861.txt:2228-2230` and `rfc4861.txt:2847-2848` — the reason for the link-layer
  override rule (RFC4861-RCFG-2, RFC4861-HOST-2).
- `rfc4861.txt:2367-2371` — an example for the override of the Router Lifetime limits
  (point-to-point links).
- `rfc4861.txt:2386` — "Each prefix has an associated:", a heading of the list.
- `rfc4861.txt:2422-2424` and `rfc4861.txt:2430-2432` — context and a pointer: the two
  variables AdvPreferredLifetime and AdvAutonomousFlag come from [ADDRCONF]. The defaults and
  the rules that this text states are in RFC4861-RCFG-19 to RFC4861-RCFG-22.
- `rfc4861.txt:2456-2459`, `rfc4861.txt:2461-2463` — summary and explanation: hosts take the
  router variables from Router Advertisements (the host side is in §6.3); routers may lack
  the host variables.
- `rfc4861.txt:2476` — a pointer to §10 (protocol constants).
- `rfc4861.txt:2501-2503` — a summary of §6.2.6 and §6.2.7, which are cataloged there.
- `rfc4861.txt:2570-2572` — an example for RFC4861-ADV-18 (lower-case "may").
- `rfc4861.txt:2609-2612` — the reason for the short initial intervals.
- `rfc4861.txt:2789-2790` — other actions on a received Router Advertisement are out of
  scope.
- `rfc4861.txt:2794-2808` (up to "first-hop router.") — explanation: the link-local address
  of a router should rarely change (lower-case "should", advice with no checkable
  behavior); nodes identify a router by its source address; the example about Redirect
  messages restates the Redirect validation of §8.1.
- `rfc4861.txt:2813-2815` — the benefit of link-local addresses on renumbering (lower-case
  "should").
- `rfc4861.txt:2829` — §6.3.1 "None.": a host has no configuration variables; nothing to
  check.
- `rfc4861.txt:2921-2924` — explanation: several routers and DHCPv6 can supply information.
- `rfc4861.txt:2940-2942` — the reason for RFC4861-HOST-11.
- `rfc4861.txt:2966-2977` — the reason for RFC4861-HOST-15.
- `rfc4861.txt:2987-2989` — the reason for the random ReachableTime.
- `rfc4861.txt:3006-3009` — explanation: NUD uses the IsRouter flag (§7.3).
- `rfc4861.txt:3040-3043` — a pointer to the definition of "on-link" in §2.1, and a remark
  on prefixes with L zero (lower-case "would").
- `rfc4861.txt:3067-3070` — what [ADDRCONF] does with prefix lifetimes: another protocol.
  The statement of this text is RFC4861-HOST-34.
- `rfc4861.txt:3090-3095` — an implementation note (separate "on-link" and "addrconf"
  functions).
- `rfc4861.txt:3101-3103` — explanation: NUD recovers from reachability problems (§7.3).
- `rfc4861.txt:3113-3116` — introduction and a pointer to §7.3.
- `rfc4861.txt:3131-3134` — a pointer to implementation hints in [LD-SHRE].
- `rfc4861.txt:3149-3154` — the reason for the round-robin order.
- `rfc4861.txt:3158-3160` — the motivation for Router Solicitations.
- `rfc4861.txt:3189-3191`, `rfc4861.txt:3199` (first sentence part) — the reason for the
  initial random delay.
- `rfc4861.txt:3205-3210`, `rfc4861.txt:3212-3220` — Mobile IPv6 context around
  RFC4861-HOST-49: movement detection of mobile nodes, the risk of solicitation storms,
  and a note that the strength of link-layer hints is out of scope.
- `rfc4861.txt:3228-3229` — the reason for RFC4861-HOST-51.
- Flags reserved for Mobile IPv6 and SEND: this range mentions none.

In §7.2 and §8:

- `rfc4861.txt:3353-3356` — §7.2 heading and the definition of address resolution;
  explanation.
- `rfc4861.txt:3376-3379` — "This is particularly relevant for unicast responses to
  solicitations..."; explanation of RFC4861-AR-3.
- `rfc4861.txt:3413-3425` — why the NS uses the source of the prompting packet; rationale.
- `rfc4861.txt:3431-3433` — "Including the source link-layer address in a multicast
  solicitation is required..."; rationale of RFC4861-AR-12, no new behavior.
- `rfc4861.txt:3435-3438` — why a unicast NS can omit the option; rationale.
- `rfc4861.txt:3482-3484` — a tentative Target Address is processed "as described in
  [ADDRCONF]"; the behavior belongs to Duplicate Address Detection of RFC 4862, not to this
  document.
- `rfc4861.txt:3497-3502` — why a new entry from an NS has IsRouter FALSE; rationale.
- `rfc4861.txt:3539-3542` — why the Override flag is set this way; rationale.
- `rfc4861.txt:3567-3574` — why no entry is created for an NA, and the introduction to the
  state-dependent rules; explanation.
- `rfc4861.txt:3633-3647` — why a solicited NA confirms the forward path and an unsolicited NA
  can carry urgent information; explanation (the normative sentence after it is
  RFC4861-AR-54).
- `rfc4861.txt:3650-3652` — "There is no need to update the state for unsolicited
  advertisements that do not change the contents of the cache."; restates the "remains
  unchanged" rule of RFC4861-AR-52.
- `rfc4861.txt:3660-3662` — why IsRouter changes are tracked; rationale.
- `rfc4861.txt:3664-3670` — summary of the NA receipt rules; explanation.
- `rfc4861.txt:3711-3718` — a mechanism to stop several proxies from multicasting NAs for one
  address; the text states that "This is a requirement on other protocols", and the Home Agent
  of [MIPv6] is an example. Belongs to other protocols.
- `rfc4861.txt:3724-3730` — unsolicited NAs are only a performance optimization; informative
  note.
- `rfc4861.txt:3767-3770` — example of a mobile node, and "the mechanisms used by proxy are
  essentially the same as the mechanisms used with anycast addresses"; explanation.
- `rfc4861.txt:3778-3781` — why a proxy NA has Override flag 0; rationale (the effect is in the
  check idea of RFC4861-ANY-8).
- `rfc4861.txt:4063-4064` — introduction of §8; explanation.
- `rfc4861.txt:4075-4079` — static routes should name the next-hop router by its link-local
  address, and "all IPv6 routing protocols must somehow exchange the link-local addresses";
  configuration guidance and a requirement on routing protocols, not on Neighbor Discovery.
- `rfc4861.txt:4127-4129` — future backward-compatible changes; informative.
- `rfc4861.txt:4141-4142` — definition of "valid redirect"; terminology.
- `rfc4861.txt:4194-4195` — pointer to [ICMPv6] for details of ICMP rate limiting;
  informative.
- `rfc4861.txt:4232-4234` — later NAs and RAs correct the IsRouter flag; explanation.
