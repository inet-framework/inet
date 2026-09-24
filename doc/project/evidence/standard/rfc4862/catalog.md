# RFC 4862 (IPv6 Stateless Address Autoconfiguration) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC4862-*` · **Stands on:** [standards.md](../../protocol/nd/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 4862, IPv6 Stateless Address Autoconfiguration, of
September 2007, a document of the in-scope set of Neighbor Discovery. The catalog comes from the RFC
text only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc4862.txt`](../../../../../../standards/RFC/rfc4862.txt) —
  IPv6 Stateless Address Autoconfiguration, September 2007. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc4862.txt>.

Neighbor Discovery itself is in [`rfc4861/catalog.md`](../rfc4861/catalog.md); this document
uses its messages. The in-scope section is §5.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/nd/standards.md). The features these statements build are in
[`features.md`](../../protocol/nd/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`nd/coverage.md`](../../model/nd/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc4862.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC4862-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 4862 uses the keywords of RFC 2119 in capitals;
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
| [RFC4862-CONF-1](#rfc4862-conf-1) | A node runs autoconfiguration on each multicast-capable interface, and a multihomed host runs it on each interface independently. |
| [RFC4862-CONF-2](#rfc4862-conf-2) | A node lets system management set DupAddrDetectTransmits for each multicast-capable interface. |
| [RFC4862-CONF-3](#rfc4862-conf-3) | DupAddrDetectTransmits is the number of consecutive Neighbor Solicitations for a tentative address, and the value one means one transmission with no retransmission. |
| [RFC4862-CONF-4](#rfc4862-conf-4) | The default value of DupAddrDetectTransmits is 1, and the document for a link type can set a different value. |
| [RFC4862-CONF-5](#rfc4862-conf-5) | RetransTimer is the gap between the Neighbor Solicitations of Duplicate Address Detection, and the wait after the last one before the procedure ends. |
| [RFC4862-CONF-6](#rfc4862-conf-6) | A host keeps a list of its addresses with their lifetimes, and the list holds both autoconfigured and manually configured addresses. |
| [RFC4862-LL-1](#rfc4862-ll-1) | A router also forms a link-local address with the procedure of §5.3. |
| [RFC4862-LL-2](#rfc4862-ll-2) | A node forms a link-local address whenever an interface becomes enabled, for example at system startup. |
| [RFC4862-LL-3](#rfc4862-ll-3) | A node forms its link-local address again when an interface comes back after a failure, or after system management disabled it and enabled it again. |
| [RFC4862-LL-4](#rfc4862-ll-4) | A node forms a link-local address when an interface attaches to a link for the first time, also after a change of the wireless access point. |
| [RFC4862-LL-5](#rfc4862-ll-5) | A link-local address has the prefix FE80::0 in its left-most bits, zeros after the prefix, and the N-bit interface identifier in its right-most N bits. |
| [RFC4862-LL-6](#rfc4862-ll-6) | If the link-local prefix length plus the interface identifier length is more than 128, autoconfiguration fails and the node needs manual configuration. |
| [RFC4862-LL-7](#rfc4862-ll-7) | A link-local address has infinite preferred and valid lifetimes, and it never times out. |
| [RFC4862-DAD-1](#rfc4862-dad-1) | A router performs Duplicate Address Detection on all its addresses before it assigns them to an interface. |
| [RFC4862-DAD-2](#rfc4862-dad-2) | A node performs Duplicate Address Detection on every unicast address before it assigns the address to an interface, whatever the source of the address. |
| [RFC4862-DAD-3](#rfc4862-dad-3) | An interface with DupAddrDetectTransmits set to zero does not perform Duplicate Address Detection. |
| [RFC4862-DAD-4](#rfc4862-dad-4) | A node does not perform Duplicate Address Detection on an anycast address. |
| [RFC4862-DAD-5](#rfc4862-dad-5) | A node tests each unicast address for uniqueness, and a new implementation does not skip the test of a global address that has the interface identifier of the link-local address. |
| [RFC4862-DAD-6](#rfc4862-dad-6) | An address under Duplicate Address Detection is tentative until the procedure succeeds, and a tentative address does not count as assigned to the interface. |
| [RFC4862-DAD-7](#rfc4862-dad-7) | The interface accepts a Neighbor Solicitation or Neighbor Advertisement with the tentative address as Target Address, and it processes it in a different way from one for an assigned address. |
| [RFC4862-DAD-8](#rfc4862-dad-8) | A node silently discards all other packets to a tentative address, also a Neighbor Solicitation or Neighbor Advertisement with the tentative address as both IP destination and Target Address. |
| [RFC4862-DAD-9](#rfc4862-dad-9) | An address is unique if no test finds a duplicate within RetransTimer milliseconds after the node sent DupAddrDetectTransmits Neighbor Solicitations, and then the node may assign it. |
| [RFC4862-DAD-10](#rfc4862-dad-10) | A node silently discards a Neighbor Solicitation or Neighbor Advertisement that fails the validity checks of RFC 4861. |
| [RFC4862-DAD-11](#rfc4862-dad-11) | Before it sends a Neighbor Solicitation, the interface joins the all-nodes multicast address and the solicited-node multicast address of the tentative address. |
| [RFC4862-DAD-12](#rfc4862-dad-12) | To check an address, a node sends DupAddrDetectTransmits Neighbor Solicitations, RetransTimer milliseconds apart. |
| [RFC4862-DAD-13](#rfc4862-dad-13) | The Neighbor Solicitation has the tested address as Target Address, the unspecified address as IP source, and the solicited-node multicast address of the target as IP destination. |
| [RFC4862-DAD-14](#rfc4862-dad-14) | When the Neighbor Solicitation is the first message of an interface after (re)initialization, the node delays the join of the solicited-node multicast address by a random time from 0 to MAX_RTR_SOLICITATION_DELAY. |
| [RFC4862-DAD-15](#rfc4862-dad-15) | For an address from a Router Advertisement to a multicast address, the node also delays the join by a random time from 0 to MAX_RTR_SOLICITATION_DELAY. |
| [RFC4862-DAD-16](#rfc4862-dad-16) | The delay of the join is a delay of the MLD report, which Duplicate Address Detection needs for MLD-snooping switches. |
| [RFC4862-DAD-17](#rfc4862-dad-17) | During the delay, the interface receives and processes packets to the all-nodes multicast address and to the solicited-node multicast address of the tentative address. |
| [RFC4862-DAD-18](#rfc4862-dad-18) | A valid Neighbor Solicitation for an assigned target address is processed as RFC 4861 says. |
| [RFC4862-DAD-19](#rfc4862-dad-19) | A Neighbor Solicitation for a tentative address from a unicast source is address resolution, and the node silently ignores it. |
| [RFC4862-DAD-20](#rfc4862-dad-20) | A node never responds to a Neighbor Solicitation for a tentative address. |
| [RFC4862-DAD-21](#rfc4862-dad-21) | A Neighbor Solicitation from the unspecified address that comes from another node makes the tentative address a duplicate, and neither node uses the address. |
| [RFC4862-DAD-22](#rfc4862-dad-22) | A Neighbor Solicitation that the node loops back from itself does not show a duplicate. |
| [RFC4862-DAD-23](#rfc4862-dad-23) | A Neighbor Solicitation for the tentative address that arrives before the node sends its own makes the address a duplicate. |
| [RFC4862-DAD-24](#rfc4862-dad-24) | More received Neighbor Solicitations than the loopback behavior of the interface explains make the tentative address a duplicate. |
| [RFC4862-DAD-25](#rfc4862-dad-25) | A valid Neighbor Advertisement for a tentative address shows that the address is not unique. |
| [RFC4862-DAD-26](#rfc4862-dad-26) | Any other valid Neighbor Advertisement is processed as RFC 4861 says. |
| [RFC4862-DAD-27](#rfc4862-dad-27) | A node does not assign a tentative address that it found to be a duplicate. |
| [RFC4862-DAD-28](#rfc4862-dad-28) | The node logs a system management error for the duplicate address. |
| [RFC4862-DAD-29](#rfc4862-dad-29) | When the duplicate is a link-local address with an interface identifier from the hardware address, the node disables IP on the interface: it sends nothing, drops what it receives, and forwards nothing to it. |
| [RFC4862-DAD-30](#rfc4862-dad-30) | When the duplicate link-local address does not come from the hardware address, the node may keep IP in operation on the interface. |
| [RFC4862-GLOB-1](#rfc4862-glob-1) | A node forms a global address from the prefix of a Prefix Information option in a Router Advertisement and an interface identifier. |
| [RFC4862-GLOB-2](#rfc4862-glob-2) | The creation of global addresses is locally configurable, and it is on by default. |
| [RFC4862-GLOB-3](#rfc4862-glob-3) | Routers send Router Advertisements periodically to the all-nodes address, and a host sends Router Solicitations to get one quickly. |
| [RFC4862-GLOB-4](#rfc4862-glob-4) | For autoconfiguration, a link has no routers when a small number of Router Solicitations brings no Router Advertisement. |
| [RFC4862-GLOB-5](#rfc4862-glob-5) | A node silently ignores a Prefix Information option with the Autonomous flag clear. |
| [RFC4862-GLOB-6](#rfc4862-glob-6) | A node silently ignores a Prefix Information option for the link-local prefix. |
| [RFC4862-GLOB-7](#rfc4862-glob-7) | A node silently ignores a Prefix Information option with a Preferred Lifetime greater than its Valid Lifetime, and it may log an error. |
| [RFC4862-GLOB-8](#rfc4862-glob-8) | For a new prefix with a Valid Lifetime other than 0, a node forms an address from the prefix and an interface identifier, and adds the address to its list. |
| [RFC4862-GLOB-9](#rfc4862-glob-9) | A node ignores a Prefix Information option when the prefix length plus the interface identifier length is not 128, and it may log an error. |
| [RFC4862-GLOB-10](#rfc4862-glob-10) | A node does not assume one fixed interface identifier length, and it accepts any length. |
| [RFC4862-GLOB-11](#rfc4862-glob-11) | A node adds a new address to its list, with the preferred and valid lifetimes of the Prefix Information option. |
| [RFC4862-GLOB-12](#rfc4862-glob-12) | For a known prefix, a node sets the preferred lifetime of the address to the received Preferred Lifetime, whatever it does with the valid lifetime. |
| [RFC4862-GLOB-13](#rfc4862-glob-13) | When the received Valid Lifetime is more than 2 hours or more than the valid lifetime that is left (RemainingLifetime), a node sets the valid lifetime to the received value. |
| [RFC4862-GLOB-14](#rfc4862-glob-14) | When 2 hours or less of the valid lifetime are left, a node ignores the Valid Lifetime of an option from an unauthenticated Router Advertisement. |
| [RFC4862-GLOB-15](#rfc4862-glob-15) | In all other cases, a node sets the valid lifetime to 2 hours. |
| [RFC4862-GLOB-16](#rfc4862-glob-16) | A preferred address becomes deprecated when its preferred lifetime expires. |
| [RFC4862-GLOB-17](#rfc4862-glob-17) | A node keeps a deprecated address as the source address of communications that already exist. |
| [RFC4862-GLOB-18](#rfc4862-glob-18) | A node does not start new communications from a deprecated address when an alternative non-deprecated address of sufficient scope is easily available. |
| [RFC4862-GLOB-19](#rfc4862-glob-19) | When an application asks for a deprecated address as source address, the protocol stack accepts the request. |
| [RFC4862-GLOB-20](#rfc4862-glob-20) | IP and the layers above it accept and process datagrams to a deprecated address as normal, and TCP answers a SYN to that address with a SYN-ACK from that address. |
| [RFC4862-GLOB-21](#rfc4862-glob-21) | A node may block new communications from deprecated addresses, but system management can disable that facility, and it is disabled by default. |
| [RFC4862-GLOB-22](#rfc4862-glob-22) | An address becomes invalid when its valid lifetime expires. |
| [RFC4862-GLOB-23](#rfc4862-glob-23) | A node never uses an invalid address as the source address of a packet that it sends. |
| [RFC4862-GLOB-24](#rfc4862-glob-24) | A node does not accept an invalid address as a destination on the interface where a packet arrives. |
| [RFC4862-CONS-1](#rfc4862-cons-1) | A host accepts the union of the information from Neighbor Discovery and from DHCPv6, and inconsistent values are not a fatal error. |
| [RFC4862-CONS-2](#rfc4862-cons-2) | When no security difference exists, the most recent value takes precedence over a value that the node learned earlier. |
| [RFC4862-CONS-3](#rfc4862-cons-3) | A node that keeps autoconfigured addresses in stable storage also keeps the expiry times of their preferred and valid lifetimes. |

## Node configuration variables

### RFC4862-CONF-1

**A node runs autoconfiguration on each multicast-capable interface, and a multihomed host runs it on each interface independently.**

> "Autoconfiguration is performed on a per-interface basis on multicast-
> capable interfaces.  For multihomed hosts, autoconfiguration is
> performed independently on each interface." — §5, `rfc4862.txt:542-544`

- Strength: description. Class: end-to-end.
- Check idea: host A has two interfaces on two links, and each link has a router with its own
  prefix. Host A forms one address from each prefix on the interface of that link, and it
  tests each address only on its own link.

### RFC4862-CONF-2

**A node lets system management set DupAddrDetectTransmits for each multicast-capable interface.**

> "A node MUST allow the following autoconfiguration-related variable to
> be configured by system management for each multicast-capable
> interface:" — §5.1, `rfc4862.txt:552-554`, and "DupAddrDetectTransmits" — §5.1,
> `rfc4862.txt:567`

- Strength: must. Class: wire.
- Check idea: set DupAddrDetectTransmits to 3 on one interface of a host and to 1 on a second
  interface. Count the Duplicate Address Detection Neighbor Solicitations on each link: three
  and one for each tentative address.

### RFC4862-CONF-3

**DupAddrDetectTransmits is the number of consecutive Neighbor Solicitations for a tentative address, and the value one means one transmission with no retransmission.**

> "DupAddrDetectTransmits  The number of consecutive Neighbor
> Solicitation messages sent while performing Duplicate Address
> Detection on a tentative address." — §5.1, `rfc4862.txt:567-569`, and "A value of one indicates a single transmission with no
> follow-up retransmissions." — §5.1, `rfc4862.txt:571-572`

- Strength: description. Class: wire.
- Check idea: set the variable to 1 and then to 3 in two runs. Count the Neighbor
  Solicitations with the tentative address as Target Address: one, then three.

### RFC4862-CONF-4

**The default value of DupAddrDetectTransmits is 1, and the document for a link type can set a different value.**

> "Default: 1, but may be overridden by a link-type specific value in
> the document that covers issues related to the transmission of IP
> over a particular link type (e.g., [RFC2464])." — §5.1, `rfc4862.txt:574-576`

- Strength: may (lower case). Class: wire.
- Check idea: a host on an Ethernet link, with no setting of the variable, sends exactly one
  Neighbor Solicitation for each tentative address.

### RFC4862-CONF-5

**RetransTimer is the gap between the Neighbor Solicitations of Duplicate Address Detection, and the wait after the last one before the procedure ends.**

> "Autoconfiguration also assumes the presence of the variable
> RetransTimer as defined in [RFC4861].  For autoconfiguration
> purposes, RetransTimer specifies the delay between consecutive
> Neighbor Solicitation transmissions performed during Duplicate
> Address Detection (if DupAddrDetectTransmits is greater than 1),
> as well as the time a node waits after sending the last Neighbor
> Solicitation before ending the Duplicate Address Detection
> process." — §5.1, `rfc4862.txt:578-585`

- Strength: description. Class: wire.
- Check idea: set DupAddrDetectTransmits to 3 and RetransTimer to 1000 ms. The three
  Neighbor Solicitations are 1 s apart, and the first packet with the new address as source
  comes 1 s or more after the last solicitation.

### RFC4862-CONF-6

**A host keeps a list of its addresses with their lifetimes, and the list holds both autoconfigured and manually configured addresses.**

> "A host maintains a list of addresses together with their
> corresponding lifetimes.  The address list contains both
> autoconfigured addresses and those configured manually." — §5.2, `rfc4862.txt:593-595`

- Strength: description. Class: internal.
- Check idea: a host has one manually configured address and one address from a Router
  Advertisement. The host answers a Neighbor Solicitation for each address. After the valid
  lifetime of the autoconfigured address expires, it answers only for the manual address.

## Link-local addresses

### RFC4862-LL-1

**A router also forms a link-local address with the procedure of §5.3.**

> "Autoconfiguration applies
> primarily to hosts, with two exceptions.  Routers are expected to
> generate a link-local address using the procedure outlined below." — §5,
> `rfc4862.txt:544-546`

- Strength: description. Class: wire.
- Check idea: enable an interface of a router. The router sends a Neighbor Solicitation with
  Target Address FE80::<interface identifier>, and later sends its Router Advertisements from
  that address.

### RFC4862-LL-2

**A node forms a link-local address whenever an interface becomes enabled, for example at system startup.**

> "A node forms a link-local address whenever an interface becomes
> enabled.  An interface may become enabled after any of the following
> events:" — §5.3, `rfc4862.txt:599-601`, and "-  The interface is initialized at system startup time." — §5.3,
> `rfc4862.txt:603`

- Strength: may (lower case). Class: wire.
- Check idea: start a host. It sends a Duplicate Address Detection Neighbor Solicitation for
  its link-local address, and after the test it uses that address as a source.

### RFC4862-LL-3

**A node forms its link-local address again when an interface comes back after a failure, or after system management disabled it and enabled it again.**

> "-  The interface is reinitialized after a temporary interface failure
> or after being temporarily disabled by system management." — §5.3, `rfc4862.txt:605-606`;
> "-  The interface becomes enabled by system management after having
> been administratively disabled." — §5.3, `rfc4862.txt:623-624`

- Strength: description. Class: wire.
- Check idea: disable an interface of a host and then enable it; in a second run, cut the link
  of the host for a short time. After each event, the host sends a new Duplicate Address
  Detection Neighbor Solicitation for its link-local address.

### RFC4862-LL-4

**A node forms a link-local address when an interface attaches to a link for the first time, also after a change of the wireless access point.**

> "-  The interface attaches to a link for the first time.  This
> includes the case where the attached link is dynamically changed
> due to a change of the access point of wireless networks." — §5.3, `rfc4862.txt:608-610`

- Strength: description. Class: wire.
- Check idea: a wireless host moves from one access point to an access point of a different
  link. On the new link, it sends a Duplicate Address Detection Neighbor Solicitation for its
  link-local address.

### RFC4862-LL-5

**A link-local address has the prefix FE80::0 in its left-most bits, zeros after the prefix, and the N-bit interface identifier in its right-most N bits.**

> "A link-local address is formed by combining the well-known link-local
> prefix FE80::0 [RFC4291] (of appropriate length) with an interface
> identifier as follows:
>
> 1.  The left-most 'prefix length' bits of the address are those of
> the link-local prefix.
>
> 2.  The bits in the address to the right of the link-local prefix are
> set to all zeroes.
>
> 3.  If the length of the interface identifier is N bits, the right-
> most N bits of the address are replaced by the interface
> identifier." — §5.3, `rfc4862.txt:626-638`

- Strength: description. Class: encoding.
- Check idea: a host on Ethernet has a 64-bit interface identifier. The Target Address of its
  first Neighbor Solicitation has FE80:0:0:0 in the left 64 bits and the interface identifier
  in the right 64 bits.

### RFC4862-LL-6

**If the link-local prefix length plus the interface identifier length is more than 128, autoconfiguration fails and the node needs manual configuration.**

> "If the sum of the link-local prefix length and N is larger than 128,
> autoconfiguration fails and manual configuration is required." — §5.3, `rfc4862.txt:640-641`

- Strength: description. Class: end-to-end.
- Check idea: give a node an interface identifier that is too long for the link-local prefix.
  The node sends no Neighbor Solicitation for a link-local address and uses none, until an
  operator configures one.

### RFC4862-LL-7

**A link-local address has infinite preferred and valid lifetimes, and it never times out.**

> "A link-local address has an infinite preferred and valid lifetime; it
> is never timed out." — §5.3, `rfc4862.txt:648-649`

- Strength: description. Class: end-to-end.
- Check idea: a host stays for a long time on a link with no Router Advertisements. It still
  answers a Neighbor Solicitation for its link-local address, and it still sends packets from
  that address.

## Duplicate Address Detection

### RFC4862-DAD-1

**A router performs Duplicate Address Detection on all its addresses before it assigns them to an interface.**

> "In
> addition, routers perform Duplicate Address Detection on all
> addresses prior to assigning them to an interface." — §5, `rfc4862.txt:546-548`

- Strength: description. Class: wire.
- Check idea: a router interface has a link-local address and a manually configured global
  address. Before the router uses either address, it sends a Duplicate Address Detection
  Neighbor Solicitation for each of them.

### RFC4862-DAD-2

**A node performs Duplicate Address Detection on every unicast address before it assigns the address to an interface, whatever the source of the address.**

> "Duplicate Address Detection MUST be performed on all unicast
> addresses prior to assigning them to an interface, regardless of
> whether they are obtained through stateless autoconfiguration,
> DHCPv6, or manual configuration, with the following exceptions:" — §5.4,
> `rfc4862.txt:653-656`

- Strength: must. Class: wire.
- Check idea: host A gets one manually configured address and one address from a Prefix
  Information option. For each address, a Neighbor Solicitation with that Target Address and
  the unspecified source comes before the first packet with that address as source.

### RFC4862-DAD-3

**An interface with DupAddrDetectTransmits set to zero does not perform Duplicate Address Detection.**

> "-  An interface whose DupAddrDetectTransmits variable is set to zero
> does not perform Duplicate Address Detection." — §5.4, `rfc4862.txt:658-659`

- Strength: description. Class: wire.
- Check idea: set DupAddrDetectTransmits to zero on a host. The host sends no Neighbor
  Solicitation from the unspecified address, and it uses each new address at once.

### RFC4862-DAD-4

**A node does not perform Duplicate Address Detection on an anycast address.**

> "-  Duplicate Address Detection MUST NOT be performed on anycast
> addresses (note that anycast addresses cannot syntactically be
> distinguished from unicast addresses)." — §5.4, `rfc4862.txt:661-663`

- Strength: must not. Class: wire.
- Check idea: configure an anycast address on a router interface, for example the
  Subnet-Router anycast address of its prefix. The router sends no Neighbor Solicitation from
  the unspecified address with that anycast address as Target Address.

### RFC4862-DAD-5

**A node tests each unicast address for uniqueness, and a new implementation does not skip the test of a global address that has the interface identifier of the link-local address.**

> "-  Each individual unicast address SHOULD be tested for uniqueness." — §5.4,
> `rfc4862.txt:665`; "Whereas this
> document does not invalidate such implementations, this kind of
> "optimization" is NOT RECOMMENDED, and new implementations MUST
> NOT do that optimization." — §5.4, `rfc4862.txt:669-670`, `rfc4862.txt:679-680`

- Strength: should (each address), should not ("NOT RECOMMENDED", the shortcut), must not
  (the shortcut in a new implementation). Class: wire.
- Check idea: host A has link-local address FE80::X and forms global address P::X from a
  Prefix Information option. Host A sends a separate Duplicate Address Detection Neighbor
  Solicitation with Target Address P::X.

### RFC4862-DAD-6

**An address under Duplicate Address Detection is tentative until the procedure succeeds, and a tentative address does not count as assigned to the interface.**

> "An address on which the Duplicate Address Detection procedure is
> applied is said to be tentative until the procedure has completed
> successfully.  A tentative address is not considered "assigned to an
> interface" in the traditional sense." — §5.4, `rfc4862.txt:701-704`

- Strength: description. Class: internal.
- Check idea: while host A tests address X, it sends no packet with source X. It sends
  packets with source X only after the test succeeds.

### RFC4862-DAD-7

**The interface accepts a Neighbor Solicitation or Neighbor Advertisement with the tentative address as Target Address, and it processes it in a different way from one for an assigned address.**

> "That is, the interface must
> accept Neighbor Solicitation and Advertisement messages containing
> the tentative address in the Target Address field, but processes such
> packets differently from those whose Target Address matches an
> address assigned to the interface." — §5.4, `rfc4862.txt:704-708`

- Strength: must (lower case). Class: end-to-end.
- Check idea: while host A tests address X, host B sends a Neighbor Advertisement with Target
  Address X to the all-nodes address. Host A accepts it and treats X as a duplicate; it does
  not update a Neighbor Cache entry as for an assigned address.

### RFC4862-DAD-8

**A node silently discards all other packets to a tentative address, also a Neighbor Solicitation or Neighbor Advertisement with the tentative address as both IP destination and Target Address.**

> "Other packets addressed to the
> tentative address should be silently discarded.  Note that the "other
> packets" include Neighbor Solicitation and Advertisement messages
> that have the tentative (i.e., unicast) address as the IP destination
> address and contain the tentative address in the Target Address
> field." — §5.4, `rfc4862.txt:708-713`

- Strength: should (lower case). Class: end-to-end.
- Check idea: while host A tests address X, send it an ICMPv6 Echo Request to X, and a
  Neighbor Advertisement to X with Target Address X. Host A sends no Echo Reply, and it
  assigns X after the test.

### RFC4862-DAD-9

**An address is unique if no test finds a duplicate within RetransTimer milliseconds after the node sent DupAddrDetectTransmits Neighbor Solicitations, and then the node may assign it.**

> "An address is considered unique if
> none of the tests indicate the presence of a duplicate address within
> RetransTimer milliseconds after having sent DupAddrDetectTransmits
> Neighbor Solicitations.  Once an address is determined to be unique,
> it may be assigned to an interface." — §5.4, `rfc4862.txt:736-740`

- Strength: may (lower case). Class: end-to-end.
- Check idea: host A tests address X on a link where nobody answers. Host A starts to use X
  as a source RetransTimer after its last Neighbor Solicitation, not before. In a second run,
  a Neighbor Advertisement for X that arrives inside that time stops the assignment.

### RFC4862-DAD-10

**A node silently discards a Neighbor Solicitation or Neighbor Advertisement that fails the validity checks of RFC 4861.**

> "A node MUST silently discard any Neighbor Solicitation or
> Advertisement message that does not pass the validity checks
> specified in [RFC4861]." — §5.4.1, `rfc4862.txt:744-746`

- Strength: must. Class: end-to-end.
- Check idea: while host A tests address X, host B sends a Neighbor Advertisement for X with
  IP Hop Limit 254; in a second run, a Neighbor Solicitation for X from the unspecified
  address with ICMP Code 1. Host A ignores both and assigns X.

### RFC4862-DAD-11

**Before it sends a Neighbor Solicitation, the interface joins the all-nodes multicast address and the solicited-node multicast address of the tentative address.**

> "Before sending a Neighbor Solicitation, an interface MUST join the
> all-nodes multicast address and the solicited-node multicast address
> of the tentative address." — §5.4.2, `rfc4862.txt:752-754`

- Strength: must. Class: wire.
- Check idea: host A tests address X. An MLD report for the solicited-node multicast address
  of X comes before the first Neighbor Solicitation for X. A Neighbor Advertisement for X to
  the all-nodes address, after the join, makes host A treat X as a duplicate.

### RFC4862-DAD-12

**To check an address, a node sends DupAddrDetectTransmits Neighbor Solicitations, RetransTimer milliseconds apart.**

> "To check an address, a node sends DupAddrDetectTransmits Neighbor
> Solicitations, each separated by RetransTimer milliseconds." — §5.4.2, `rfc4862.txt:759-760`

- Strength: description. Class: wire.
- Check idea: set DupAddrDetectTransmits to 3 and RetransTimer to 1000 ms. The host sends
  three Neighbor Solicitations for the tentative address, 1 s apart.

### RFC4862-DAD-13

**The Neighbor Solicitation has the tested address as Target Address, the unspecified address as IP source, and the solicited-node multicast address of the target as IP destination.**

> "The
> solicitation's Target Address is set to the address being checked,
> the IP source is set to the unspecified address, and the IP
> destination is set to the solicited-node multicast address of the
> target address." — §5.4.2, `rfc4862.txt:760-764`

- Strength: description. Class: wire.
- Check idea: capture the Neighbor Solicitation for tentative address X. It has Target
  Address X, IP source ::, and IP destination FF02::1:FFxx:xxxx with the low 24 bits of X.

### RFC4862-DAD-14

**When the Neighbor Solicitation is the first message of an interface after (re)initialization, the node delays the join of the solicited-node multicast address by a random time from 0 to MAX_RTR_SOLICITATION_DELAY.**

> "If the Neighbor Solicitation is going to be the first message sent
> from an interface after interface (re)initialization, the node SHOULD
> delay joining the solicited-node multicast address by a random delay
> between 0 and MAX_RTR_SOLICITATION_DELAY as specified in [RFC4861]." — §5.4.2,
> `rfc4862.txt:766-769`

- Strength: should. Class: wire.
- Check idea: enable the interfaces of several hosts at the same time. The first MLD report
  of each host comes at a random time from 0 to 1 s (MAX_RTR_SOLICITATION_DELAY) after the
  enable, and the times differ between the hosts.

### RFC4862-DAD-15

**For an address from a Router Advertisement to a multicast address, the node also delays the join by a random time from 0 to MAX_RTR_SOLICITATION_DELAY.**

> "Even if the Neighbor Solicitation is not going to be the first
> message sent, the node SHOULD delay joining the solicited-node
> multicast address by a random delay between 0 and
> MAX_RTR_SOLICITATION_DELAY if the address being checked is configured
> by a router advertisement message sent to a multicast address." — §5.4.2,
> `rfc4862.txt:775-779`

- Strength: should. Class: wire.
- Check idea: several hosts on a link already have their link-local addresses. A router sends
  one multicast Router Advertisement with a new prefix. Each host sends its MLD report for
  the new solicited-node group at a random time from 0 to 1 s after the advertisement.

### RFC4862-DAD-16

**The delay of the join is a delay of the MLD report, which Duplicate Address Detection needs for MLD-snooping switches.**

> "In the case of Duplicate Address
> Detection, the MLD report message is required in order to inform MLD-
> snooping switches, rather than routers, to forward multicast packets.
> In the above description, the delay for joining the multicast address
> thus means delaying transmission of the corresponding MLD report
> message." — §5.4.2, `rfc4862.txt:793-798`

- Strength: description. Class: wire.
- Check idea: after an interface becomes enabled, the host sends an MLD report for the
  solicited-node group of each tentative address. The random delay lies between the enable
  and that MLD report, not between the report and the Neighbor Solicitation.

### RFC4862-DAD-17

**During the delay, the interface receives and processes packets to the all-nodes multicast address and to the solicited-node multicast address of the tentative address.**

> "In order to improve the robustness of the Duplicate Address Detection
> algorithm, an interface MUST receive and process datagrams sent to
> the all-nodes multicast address or solicited-node multicast address
> of the tentative address during the delay period." — §5.4.2, `rfc4862.txt:808-811`

- Strength: must. Class: end-to-end.
- Check idea: host A has tentative address X and waits in its random delay. Host B sends a
  Neighbor Advertisement for X to the all-nodes address; in a second run, a Neighbor
  Solicitation for X from :: to the solicited-node address. Host A does not assign X.

### RFC4862-DAD-18

**A valid Neighbor Solicitation for an assigned target address is processed as RFC 4861 says.**

> "On receipt of a valid Neighbor Solicitation message on an interface,
> node behavior depends on whether or not the target address is
> tentative.  If the target address is not tentative (i.e., it is
> assigned to the receiving interface), the solicitation is processed
> as described in [RFC4861]." — §5.4.3, `rfc4862.txt:821-825`

- Strength: description. Class: end-to-end.
- Check idea: host B sends a Neighbor Solicitation for an assigned address of host A. Host A
  answers with a Neighbor Advertisement as RFC 4861 specifies.

### RFC4862-DAD-19

**A Neighbor Solicitation for a tentative address from a unicast source is address resolution, and the node silently ignores it.**

> "If the target address is tentative, and
> the source address is a unicast address, the solicitation's sender is
> performing address resolution on the target; the solicitation should
> be silently ignored." — §5.4.3, `rfc4862.txt:825-828`

- Strength: should (lower case). Class: end-to-end.
- Check idea: while host A tests address X, host B sends a Neighbor Solicitation for X from
  its own unicast address. Host A sends no Neighbor Advertisement, does not treat X as a
  duplicate, and assigns X after the test.

### RFC4862-DAD-20

**A node never responds to a Neighbor Solicitation for a tentative address.**

> "In all cases, a node MUST NOT respond to a Neighbor
> Solicitation for a tentative address." — §5.4.3, `rfc4862.txt:829-830`

- Strength: must not. Class: wire.
- Check idea: while host A tests address X, send it a Neighbor Solicitation for X from a
  unicast source, and one from the unspecified source. Host A sends no Neighbor Advertisement
  with Target Address X.

### RFC4862-DAD-21

**A Neighbor Solicitation from the unspecified address that comes from another node makes the tentative address a duplicate, and neither node uses the address.**

> "If the source address of the Neighbor Solicitation is the unspecified
> address, the solicitation is from a node performing Duplicate Address
> Detection.  If the solicitation is from another node, the tentative
> address is a duplicate and should not be used (by either node)." — §5.4.3,
> `rfc4862.txt:832-835`

- Strength: should not (lower case). Class: end-to-end.
- Check idea: hosts A and B test the same address X at the same time, and each receives the
  Neighbor Solicitation of the other. Neither host sends a packet with source X.

### RFC4862-DAD-22

**A Neighbor Solicitation that the node loops back from itself does not show a duplicate.**

> "If
> the solicitation is from the node itself (because the node loops back
> multicast packets), the solicitation does not indicate the presence
> of a duplicate address." — §5.4.3, `rfc4862.txt:835-838`

- Strength: description. Class: end-to-end.
- Check idea: host A has an interface that loops back its multicast packets. Host A tests
  address X alone on the link, receives its own Neighbor Solicitation, and still assigns X.

### RFC4862-DAD-23

**A Neighbor Solicitation for the tentative address that arrives before the node sends its own makes the address a duplicate.**

> "The following tests identify conditions under which a tentative
> address is not unique:" — §5.4.3, `rfc4862.txt:853-854`, and "-  If a Neighbor Solicitation for a tentative address is received
> before one is sent, the tentative address is a duplicate." — §5.4.3, `rfc4862.txt:856-857`

- Strength: description. Class: end-to-end.
- Check idea: host A has tentative address X and has sent no Neighbor Solicitation yet. Host
  B sends a Neighbor Solicitation for X from the unspecified address. Host A never uses X as
  a source.

### RFC4862-DAD-24

**More received Neighbor Solicitations than the loopback behavior of the interface explains make the tentative address a duplicate.**

> "-  If the actual number of Neighbor Solicitations received exceeds
> the number expected based on the loopback semantics (e.g., the
> interface does not loop back the packet, yet one or more
> solicitations was received), the tentative address is a duplicate." — §5.4.3,
> `rfc4862.txt:864-867`

- Strength: description. Class: end-to-end.
- Check idea: host A has an interface without multicast loopback. Host A sends its Neighbor
  Solicitation for X, and then host B sends one for X. Host A does not use X.

### RFC4862-DAD-25

**A valid Neighbor Advertisement for a tentative address shows that the address is not unique.**

> "On receipt of a valid Neighbor Advertisement message on an interface,
> node behavior depends on whether the target address is tentative or
> matches a unicast or anycast address assigned to the interface:" — §5.4.4,
> `rfc4862.txt:874-876`, and "1.  If the target address is tentative, the tentative address is not
> unique." — §5.4.4, `rfc4862.txt:878-879`

- Strength: description. Class: end-to-end.
- Check idea: host B owns address X. Host A tests X, and host B answers the Neighbor
  Solicitation with a Neighbor Advertisement to the all-nodes address. Host A does not use X.

### RFC4862-DAD-26

**Any other valid Neighbor Advertisement is processed as RFC 4861 says.**

> "3.  Otherwise, the advertisement is processed as described in
> [RFC4861]." — §5.4.4, `rfc4862.txt:888-889`

- Strength: description. Class: end-to-end.
- Check idea: host A has tentative address X and a Neighbor Cache entry for address Y of host
  B. A Neighbor Advertisement for Y arrives. Host A updates the entry for Y as RFC 4861
  specifies, and the test of X continues.

### RFC4862-DAD-27

**A node does not assign a tentative address that it found to be a duplicate.**

> "A tentative address that is determined to be a duplicate as described
> above MUST NOT be assigned to an interface" — §5.4.5, `rfc4862.txt:905-906`

- Strength: must not. Class: end-to-end.
- Check idea: host B owns X. Host A tests X and receives a Neighbor Advertisement from host
  B. Host A never sends a packet with source X and never answers a Neighbor Solicitation
  for X.

### RFC4862-DAD-28

**The node logs a system management error for the duplicate address.**

> "and the node SHOULD log a
> system management error." — §5.4.5, `rfc4862.txt:906-907`

- Strength: should. Class: internal.
- Check idea: after host A finds that X is a duplicate, its management log holds an error for
  X on that interface.

### RFC4862-DAD-29

**When the duplicate is a link-local address with an interface identifier from the hardware address, the node disables IP on the interface: it sends nothing, drops what it receives, and forwards nothing to it.**

> "If the address is a link-local address formed from an interface
> identifier based on the hardware address, which is supposed to be
> uniquely assigned (e.g., EUI-64 for an Ethernet interface), IP
> operation on the interface SHOULD be disabled.  By disabling IP
> operation, the node will then:
>
> -  not send any IP packets from the interface,
>
> -  silently drop any IP packets received on the interface, and
>
> -  not forward any IP packets to the interface (when acting as a
> router or processing a packet with a Routing header)." — §5.4.5, `rfc4862.txt:909-920`

- Strength: should. Class: end-to-end.
- Check idea: hosts A and B have the same MAC address, and so the same EUI-64 link-local
  address. After host A detects the duplicate, it sends no IPv6 packet on that interface and
  answers no Echo Request that arrives there. A router in the same state forwards no packet
  to that interface.

### RFC4862-DAD-30

**When the duplicate link-local address does not come from the hardware address, the node may keep IP in operation on the interface.**

> "On the other hand, if the duplicate link-local address is not formed
> from an interface identifier based on the hardware address, which is
> supposed to be uniquely assigned, IP operation on the interface MAY
> be continued." — §5.4.5, `rfc4862.txt:930-933`

- Strength: may. Class: end-to-end.
- Check idea: give host A a manually configured link-local address that host B already owns.
  After the detection, host A can still send and receive IPv6 packets with its other
  addresses on that interface.

## Global addresses

### RFC4862-GLOB-1

**A node forms a global address from the prefix of a Prefix Information option in a Router Advertisement and an interface identifier.**

> "Global addresses are formed by appending an interface identifier to a
> prefix of appropriate length.  Prefixes are obtained from Prefix
> Information options contained in Router Advertisements." — §5.5, `rfc4862.txt:942-944`

- Strength: description. Class: wire.
- Check idea: a router advertises prefix 2001:db8:1::/64 with the Autonomous flag set. Host A
  sends a Neighbor Solicitation for 2001:db8:1::<interface identifier>, and later uses that
  address as a source.

### RFC4862-GLOB-2

**The creation of global addresses is locally configurable, and it is on by default.**

> "Creation of
> global addresses as described in this section SHOULD be locally
> configurable.  However, the processing described below MUST be
> enabled by default." — §5.5, `rfc4862.txt:944-947`

- Strength: should (the local switch), must (on by default). Class: end-to-end.
- Check idea: a host with no configuration forms a global address from an advertised prefix.
  The same host with the creation switched off forms no address from the same Router
  Advertisement.

### RFC4862-GLOB-3

**Routers send Router Advertisements periodically to the all-nodes address, and a host sends Router Solicitations to get one quickly.**

> "Router Advertisements are sent periodically to the all-nodes
> multicast address.  To obtain an advertisement quickly, a host sends
> out Router Solicitations as described in [RFC4861]." — §5.5.1, `rfc4862.txt:961-963`

- Strength: description. Class: wire.
- Check idea: a host interface becomes enabled on a link with a router that has a long
  advertisement interval. The host sends a Router Solicitation, and it forms its global
  address from the answer before the next periodic Router Advertisement.

### RFC4862-GLOB-4

**For autoconfiguration, a link has no routers when a small number of Router Solicitations brings no Router Advertisement.**

> "From
> the perspective of autoconfiguration, a link has no routers if no
> Router Advertisements are received after having sent a small number
> of Router Solicitations as described in [RFC4861]." — §5.5.2, `rfc4862.txt:968-971`

- Strength: description. Class: end-to-end.
- Check idea: a host on a link with no router sends its Router Solicitations
  (MAX_RTR_SOLICITATIONS of RFC 4861) and then no more. It has only its link-local address.

### RFC4862-GLOB-5

**A node silently ignores a Prefix Information option with the Autonomous flag clear.**

> "For each Prefix-Information option in the Router Advertisement:" — §5.5.3,
> `rfc4862.txt:982`, and "a)  If the Autonomous flag is not set, silently ignore the Prefix
> Information option." — §5.5.3, `rfc4862.txt:984-985`

- Strength: description. Class: end-to-end.
- Check idea: a Router Advertisement holds a Prefix Information option for P with A = 0. The
  host sends no Neighbor Solicitation for an address in P and forms no address from P.

### RFC4862-GLOB-6

**A node silently ignores a Prefix Information option for the link-local prefix.**

> "b)  If the prefix is the link-local prefix, silently ignore the
> Prefix Information option." — §5.5.3, `rfc4862.txt:987-988`

- Strength: description. Class: end-to-end.
- Check idea: a Router Advertisement holds a Prefix Information option for FE80::/64 with
  A = 1 and a short Valid Lifetime. The host forms no new address, and its link-local address
  stays valid after that lifetime.

### RFC4862-GLOB-7

**A node silently ignores a Prefix Information option with a Preferred Lifetime greater than its Valid Lifetime, and it may log an error.**

> "c)  If the preferred lifetime is greater than the valid lifetime,
> silently ignore the Prefix Information option.  A node MAY wish to
> log a system management error in this case." — §5.5.3, `rfc4862.txt:990-992`

- Strength: description (the ignore), may (the log). Class: end-to-end.
- Check idea: a Prefix Information option for P has A = 1, Preferred Lifetime 600 s and Valid
  Lifetime 300 s. The host forms no address from P.

### RFC4862-GLOB-8

**For a new prefix with a Valid Lifetime other than 0, a node forms an address from the prefix and an interface identifier, and adds the address to its list.**

> "d)  If the prefix advertised is not equal to the prefix of an
> address configured by stateless autoconfiguration already in the
> list of addresses associated with the interface (where "equal"
> means the two prefix lengths are the same and the first prefix-
> length bits of the prefixes are identical), and if the Valid
> Lifetime is not 0, form an address (and add it to the list) by
> combining the advertised prefix with an interface identifier of
> the link as follows:" — §5.5.3, `rfc4862.txt:994-1001`, and
> "|            128 - N bits               |       N bits           |
> +---------------------------------------+------------------------+
> |            link prefix                |  interface identifier  |" — §5.5.3,
> `rfc4862.txt:1003-1005`

- Strength: description. Class: wire.
- Check idea: a Router Advertisement brings a new /64 prefix P with A = 1 and Valid Lifetime
  3600 s. The host tests and then uses P::<interface identifier>. The same option with Valid
  Lifetime 0 gives no address.

### RFC4862-GLOB-9

**A node ignores a Prefix Information option when the prefix length plus the interface identifier length is not 128, and it may log an error.**

> "If the sum of the prefix length and interface identifier length
> does not equal 128 bits, the Prefix Information option MUST be
> ignored.  An implementation MAY wish to log a system management
> error in this case." — §5.5.3, `rfc4862.txt:1015-1018`

- Strength: must (the ignore), may (the log). Class: end-to-end.
- Check idea: on an Ethernet link, with 64-bit interface identifiers, a router advertises a
  /48 prefix and an /80 prefix, both with A = 1. The host forms no address from either prefix.

### RFC4862-GLOB-10

**A node does not assume one fixed interface identifier length, and it accepts any length.**

> "Thus, an implementation should not assume a
> particular constant.  Rather, it should expect any lengths of
> interface identifiers." — §5.5.3, `rfc4862.txt:1039-1041`

- Strength: should not, should (lower case). Class: internal.
- Check idea: on a link type with an interface identifier that is not 64 bits long, a prefix
  whose length plus that length is 128 gives an address, and a /64 prefix gives none.

### RFC4862-GLOB-11

**A node adds a new address to its list, with the preferred and valid lifetimes of the Prefix Information option.**

> "If an address is formed successfully and the address is not yet in
> the list, the host adds it to the list of addresses assigned to
> the interface, initializing its preferred and valid lifetime
> values from the Prefix Information option." — §5.5.3, `rfc4862.txt:1043-1046`

- Strength: description. Class: end-to-end.
- Check idea: a Router Advertisement gives prefix P with Preferred Lifetime 100 s and Valid
  Lifetime 200 s, and no more Router Advertisements follow. The address P::X becomes
  deprecated after 100 s and invalid after 200 s.

### RFC4862-GLOB-12

**For a known prefix, a node sets the preferred lifetime of the address to the received Preferred Lifetime, whatever it does with the valid lifetime.**

> "e)  If the advertised prefix is equal to the prefix of an address
> configured by stateless autoconfiguration in the list, the
> preferred lifetime of the address is reset to the Preferred
> Lifetime in the received advertisement." — §5.5.3, `rfc4862.txt:1053-1056`; "Note that the preferred lifetime of the corresponding address is
> always reset to the Preferred Lifetime in the received Prefix
> Information option, regardless of whether the valid lifetime is
> also reset or ignored." — §5.5.3, `rfc4862.txt:1097-1100`

- Strength: description. Class: end-to-end.
- Check idea: host A has address P::X with a long preferred lifetime. A Router Advertisement
  for P brings Preferred Lifetime 10 s and Valid Lifetime 60 s. After 10 s, P::X is
  deprecated, and it stays valid for more than 60 s.

### RFC4862-GLOB-13

**When the received Valid Lifetime is more than 2 hours or more than the valid lifetime that is left (RemainingLifetime), a node sets the valid lifetime to the received value.**

> "1.  If the received Valid Lifetime is greater than 2 hours or
> greater than RemainingLifetime, set the valid lifetime of the
> corresponding address to the advertised Valid Lifetime." — §5.5.3, `rfc4862.txt:1071-1073`

- Strength: description. Class: end-to-end.
- Check idea: host A has P::X with 60 minutes of valid lifetime left. A Router Advertisement
  for P brings Valid Lifetime 90 minutes: P::X becomes invalid 90 minutes later. In a second
  run, a Valid Lifetime of 3 hours keeps P::X valid for 3 hours.

### RFC4862-GLOB-14

**When 2 hours or less of the valid lifetime are left, a node ignores the Valid Lifetime of an option from an unauthenticated Router Advertisement.**

> "2.  If RemainingLifetime is less than or equal to 2 hours, ignore
> the Prefix Information option with regards to the valid
> lifetime, unless the Router Advertisement from which this
> option was obtained has been authenticated (e.g., via Secure
> Neighbor Discovery [RFC3971])." — §5.5.3, `rfc4862.txt:1075-1079`

- Strength: description. Class: end-to-end.
- Check idea: host A has P::X with 60 minutes of valid lifetime left. An unauthenticated
  Router Advertisement for P brings Valid Lifetime 10 minutes. P::X is still valid after 30
  minutes, and it becomes invalid after 60 minutes.

### RFC4862-GLOB-15

**In all other cases, a node sets the valid lifetime to 2 hours.**

> "3.  Otherwise, reset the valid lifetime of the corresponding
> address to 2 hours." — §5.5.3, `rfc4862.txt:1084-1085`

- Strength: description. Class: end-to-end.
- Check idea: host A has P::X with 5 hours of valid lifetime left. A Router Advertisement for
  P brings Valid Lifetime 10 minutes. P::X is still valid after 1 hour, and it becomes
  invalid 2 hours after the advertisement.

### RFC4862-GLOB-16

**A preferred address becomes deprecated when its preferred lifetime expires.**

> "A preferred address becomes deprecated when its preferred lifetime
> expires." — §5.5.4, `rfc4862.txt:1109-1110`

- Strength: description. Class: internal.
- Check idea: address P::X has Preferred Lifetime 30 s, and no Router Advertisement renews
  it. After 30 s, a new connection from host A takes another address of the same scope, if
  host A has one.

### RFC4862-GLOB-17

**A node keeps a deprecated address as the source address of communications that already exist.**

> "A deprecated address SHOULD continue to be used as a source
> address in existing communications," — §5.5.4, `rfc4862.txt:1110-1111`

- Strength: should. Class: wire.
- Check idea: host A opens a TCP connection from P::X. P::X becomes deprecated while the
  connection is open. Host A keeps P::X as the source of the segments of that connection.

### RFC4862-GLOB-18

**A node does not start new communications from a deprecated address when an alternative non-deprecated address of sufficient scope is easily available.**

> "but SHOULD NOT be used to
> initiate new communications if an alternate (non-deprecated) address
> of sufficient scope can easily be used instead." — §5.5.4, `rfc4862.txt:1111-1113`

- Strength: should not. Class: wire.
- Check idea: host A has deprecated address P::X and preferred address Q::Y of the same scope.
  A new connection from host A to a global destination has source Q::Y.

### RFC4862-GLOB-19

**When an application asks for a deprecated address as source address, the protocol stack accepts the request.**

> "For
> example, if an application explicitly specifies that the protocol
> stack use a deprecated address as a source address, the protocol
> stack must accept that;" — §5.5.4, `rfc4862.txt:1118`, `rfc4862.txt:1127-1129`

- Strength: must (lower case). Class: wire.
- Check idea: an application on host A binds to deprecated address P::X and sends a UDP
  datagram. The datagram leaves host A with source P::X.

### RFC4862-GLOB-20

**IP and the layers above it accept and process datagrams to a deprecated address as normal, and TCP answers a SYN to that address with a SYN-ACK from that address.**

> "IP and higher layers (e.g., TCP, UDP) MUST continue to accept and
> process datagrams destined to a deprecated address as normal since a
> deprecated address is still a valid address for the interface.  In
> the case of TCP, this means TCP SYN segments sent to a deprecated
> address are responded to using the deprecated address as a source
> address in the corresponding SYN-ACK (if the connection would
> otherwise be allowed)." — §5.5.4, `rfc4862.txt:1134-1140`

- Strength: must. Class: end-to-end.
- Check idea: host B sends a TCP SYN to deprecated address P::X of host A, on a port where
  host A listens. Host A answers with a SYN-ACK from P::X. An Echo Request to P::X gets an
  Echo Reply.

### RFC4862-GLOB-21

**A node may block new communications from deprecated addresses, but system management can disable that facility, and it is disabled by default.**

> "An implementation MAY prevent any new communication from using a
> deprecated address, but system management MUST have the ability to
> disable such a facility, and the facility MUST be disabled by
> default." — §5.5.4, `rfc4862.txt:1142-1145`

- Strength: may (the facility), must (the switch that disables it), must (disabled by default).
  Class: wire.
- Check idea: a host with default configuration has only a deprecated global address. It
  opens a new connection to a global destination, and the connection has the deprecated
  address as source.

### RFC4862-GLOB-22

**An address becomes invalid when its valid lifetime expires.**

> "An address (and its association with an interface) becomes invalid
> when its valid lifetime expires." — §5.5.4, `rfc4862.txt:1154-1155`

- Strength: description. Class: internal.
- Check idea: address P::X has Valid Lifetime 60 s, and no Router Advertisement renews it.
  After 60 s, host A no longer answers a Neighbor Solicitation for P::X.

### RFC4862-GLOB-23

**A node never uses an invalid address as the source address of a packet that it sends.**

> "An invalid address MUST NOT be used
> as a source address in outgoing communications" — §5.5.4, `rfc4862.txt:1155-1156`

- Strength: must not. Class: wire.
- Check idea: after P::X becomes invalid, host A sends no packet with source P::X, also not
  for a connection that was open before the expiry.

### RFC4862-GLOB-24

**A node does not accept an invalid address as a destination on the interface where a packet arrives.**

> "and MUST NOT be
> recognized as a destination on a receiving interface." — §5.5.4, `rfc4862.txt:1156-1157`

- Strength: must not. Class: end-to-end.
- Check idea: after P::X becomes invalid, host B, which still has the link-layer address of
  host A in its Neighbor Cache, sends an Echo Request and a TCP SYN to P::X. Host A answers
  neither.

## Configuration consistency and stability

### RFC4862-CONS-1

**A host accepts the union of the information from Neighbor Discovery and from DHCPv6, and inconsistent values are not a fatal error.**

> "However, it is not
> considered a fatal error if information received from multiple
> sources is inconsistent.  Hosts accept the union of all information
> received via Neighbor Discovery and DHCPv6." — §5.6, `rfc4862.txt:1167-1170`

- Strength: description. Class: end-to-end.
- Check idea: host A gets address P::X from a Router Advertisement and address Q::Y from
  DHCPv6. Host A answers Neighbor Solicitations and Echo Requests for both addresses.

### RFC4862-CONS-2

**When no security difference exists, the most recent value takes precedence over a value that the node learned earlier.**

> "In any case, if there is no security difference, the most recently
> obtained values SHOULD have precedence over information learned
> earlier." — §5.6, `rfc4862.txt:1190-1192`

- Strength: should. Class: end-to-end.
- Check idea: router R1 advertises an MTU option with 1400, and later router R2 on the same
  link advertises an MTU option with 1300. Host A then sends no packet larger than 1300
  octets on the link.

### RFC4862-CONS-3

**A node that keeps autoconfigured addresses in stable storage also keeps the expiry times of their preferred and valid lifetimes.**

> "An implementation that has stable storage may want to retain
> addresses in the storage when the addresses were acquired using
> stateless address autoconfiguration." — §5.7, `rfc4862.txt:1196-1198`, and "When this
> technique is used, it should also be noted that the expiration times
> of the preferred and valid lifetimes must be retained, in order to
> prevent the use of an address after it has become deprecated or
> invalid." — §5.7, `rfc4862.txt:1201-1205`

- Strength: may, must (lower case). Class: end-to-end.
- Check idea: a host with a stored address P::X reboots on a link with no router. P::X
  becomes deprecated at the stored expiry time of its preferred lifetime, and invalid at the
  stored expiry time of its valid lifetime.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of §5. What
the catalog leaves out:

- **The sections outside the in-scope set**: §1 to §4 (introduction, terminology, design
  goals, overview) and §6 to §8 (security, acknowledgements, references).
- **Optimistic Duplicate Address Detection**, RFC 4429: §5 does not mention it; it is level 5.

Within §5, these parts have no entry, with the reason:

- §5 heading, `rfc4862.txt:540`. No content.
- The zero value of DupAddrDetectTransmits, `rfc4862.txt:569-571`. §5.4 states the same rule
  as the first exception to Duplicate Address Detection; the entry is RFC4862-DAD-3.
- The scope statement of §5.2 for routers, `rfc4862.txt:589-591`: how routers configure their
  interfaces, beyond link-local addresses and Duplicate Address Detection, is out of scope.
- The length of the interface identifier, `rfc4862.txt:642-646` and `rfc4862.txt:1018-1021`.
  A link-type document defines it; the lower-case "should" binds that document, not a node.
- The explanation of the shortcut that skips the test of global addresses,
  `rfc4862.txt:666-669` and `rfc4862.txt:680-688`. Reasons only.
- The introduction of the procedure, `rfc4862.txt:690-691`, and the introduction of the
  subsections, `rfc4862.txt:735-736`.
- "If a duplicate address is discovered during the procedure, the address cannot be assigned
  to the interface", `rfc4862.txt:691-693`. §5.4.5 states the same rule with MUST NOT; the
  entry is RFC4862-DAD-27.
- The recovery after a duplicate, `rfc4862.txt:693-696`: a new interface identifier or manual
  configuration. It is an administrative action, and it demands no protocol behavior.
- The note that Duplicate Address Detection is not completely reliable, `rfc4862.txt:696-699`.
- The remark that a unicast Neighbor Solicitation or Advertisement to a tentative address does
  not happen in normal operation, `rfc4862.txt:713-715`. Explanation.
- "Duplicate Address Detection must be performed prior to assigning an address", with its
  reason, `rfc4862.txt:717-724`. It restates RFC4862-DAD-2 and explains it.
- The names "valid solicitation" and "valid advertisement", `rfc4862.txt:746-748`.
  Terminology; RFC4862-DAD-18 and RFC4862-DAD-25 use it.
- The reasons for the joins and for the random delays, `rfc4862.txt:754-757`,
  `rfc4862.txt:770-773` and `rfc4862.txt:779-782`.
- The MLD background, `rfc4862.txt:791-793` and `rfc4862.txt:798-806`: MLD reports in general,
  the congestion that a delay of only the Neighbor Solicitation causes, and the source address
  of the MLD report of RFC 3590. These belong to MLD, another protocol.
- The explanation of reception during the delay, `rfc4862.txt:811-817`.
- The Implementer's Note on multicast loopback, `rfc4862.txt:847-851`. It points to Appendix A.
- The examples of when the two duplicate conditions occur, `rfc4862.txt:857-862` and
  `rfc4862.txt:868-870`.
- A Neighbor Advertisement for an address that is already assigned, rule 2 of §5.4.4,
  `rfc4862.txt:881-886`. The document says that its handling is out of scope.
- The reasons to disable IP operation, `rfc4862.txt:922-928`, and the note that "IP" means
  IPv6, `rfc4862.txt:935-938`.
- DHCPv6 on a link with no routers, `rfc4862.txt:967-968`, and the manual configuration of a
  forwarding node on such a link, `rfc4862.txt:973-978`. DHCPv6 is another protocol, and the
  second statement is an administrative note.
- The duty of the system administrator, and the reasons to validate the prefix length,
  `rfc4862.txt:1023-1038`. Explanation; the rule is RFC4862-GLOB-9.
- The note that the prefix check cannot find every address conflict, `rfc4862.txt:1046-1051`.
- The introduction of the three valid-lifetime rules and of the name RemainingLifetime,
  `rfc4862.txt:1056-1061`. The rules are RFC4862-GLOB-13 to RFC4862-GLOB-15.
- The authenticated branch of rule 2, `rfc4862.txt:1079-1082`: the valid lifetime of a Router
  Advertisement authenticated with Secure Neighbor Discovery (RFC 3971). SEND is not in the
  in-scope set. The unauthenticated branch is RFC4862-GLOB-14.
- The reasons of the lifetime rules, `rfc4862.txt:1087-1095` and `rfc4862.txt:1100-1105`.
- The context of the application request, `rfc4862.txt:1115-1118` and
  `rfc4862.txt:1129-1132`.
- Source address selection, `rfc4862.txt:1147-1152`. RFC 3484 covers it, and the document
  says that it is out of scope.
- The context of §5.6, `rfc4862.txt:1161-1167`. The lower-case "should be consistent" binds
  the sources of the information, not the host.
- The precedence of secure information, `rfc4862.txt:1172-1188`: a lower-case "may want"
  that names Secure Neighbor Discovery and secured DHCPv6, both outside the in-scope set.
- The reason to retain addresses, `rfc4862.txt:1198-1201`, and the scope note,
  `rfc4862.txt:1207-1208`.
- Optimistic Duplicate Address Detection (RFC 4429): §5 does not mention it.
