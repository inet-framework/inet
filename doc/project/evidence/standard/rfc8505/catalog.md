# RFC 8505 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8505-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 8505 in the selected 6LoWPAN family.
Source: [rfc8505.txt](rfc8505.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc8505.txt>. The publication version and family
relationships are pinned in the linked standards map.

The catalog is a scoped selection, not an exhaustive inventory or a claim of test
coverage. Its scope exclusions are listed at the end. No test-depth level is claimed.
Statements and check ideas are independent of any simulation implementation.

Quotes retain the wording and line breaks of the cached text; leading indentation
is omitted; excerpts may start or end within a line. Line references count physical
newline-delimited lines, including page
headers. Strength records the source keyword, including qualified permissions;
`description` denotes a format or procedure stated without a requirement keyword.
Each check idea is a proposed observation, not a test result. Conditional rules apply
only to the roles, modes and prerequisites named in their source paragraphs.

## Index

| ID | Statement |
| --- | --- |
| [RFC8505-EARO-1](#rfc8505-earo-1) | An EARO carries a TID with T set. |
| [RFC8505-EARO-2](#rfc8505-earo-2) | A host-only registering node sets R to request reachability services. |
| [RFC8505-TID-1](#rfc8505-tid-1) | Each re-registration increments the transaction identifier. |
| [RFC8505-TID-2](#rfc8505-tid-2) | Parallel registrations of the same transaction use the same TID. |
| [RFC8505-ROVR-1](#rfc8505-rovr-1) | ROVR is scoped to one IPv6 address and cannot correlate different addresses. |
| [RFC8505-ROVR-2](#rfc8505-rovr-2) | Recognized different ROVR namespaces remain distinct even when values match. |
| [RFC8505-ROVR-3](#rfc8505-rovr-3) | ROVR cannot be the registering-node key or registration index. |
| [RFC8505-ADDR-1](#rfc8505-addr-1) | An NS with EARO is a registration only when it also includes SLLAO. |
| [RFC8505-ADDR-2](#rfc8505-addr-2) | Extended registration identifies the registered address in the NS and NA Target Address. |
| [RFC8505-ADDR-3](#rfc8505-addr-3) | The registration source is a link-local address already registered or being registered. |
| [RFC8505-ADDR-4](#rfc8505-addr-4) | Without an existing registered address, the first registration uses a link-local address as source and target. |
| [RFC8505-STATE-1](#rfc8505-state-1) | Renewal is reported by the router to the border router. |
| [RFC8505-COMPAT-1](#rfc8505-compat-1) | A node uses base-compatible behavior until the router is known to support the extension. |
| [RFC8505-COMPAT-2](#rfc8505-compat-2) | An updated router answers both ARO and EARO requests with EARO. |
| [RFC8505-COMPAT-3](#rfc8505-compat-3) | Capability options accompany discovery unless already known, and an RS with 6CIO always elicits RA with 6CIO. |
| [RFC8505-COMPAT-4](#rfc8505-compat-4) | An updated router accepts a valid base-only host registration and uses base DAR/DAC messages. |
| [RFC8505-COMPAT-5](#rfc8505-compat-5) | Registration to a base-only router uses a 64-bit ROVR. |
| [RFC8505-CAP-1](#rfc8505-cap-1) | An updated router signals EARO support with the 6CIO E flag. |
| [RFC8505-EARO-3](#rfc8505-earo-3) | EARO is backward compatible with ARO only when option Length is two. |

## Checkable statements

### RFC8505-EARO-1

**An EARO carries a TID with T set.**

> o  A node that supports this specification MUST provide a TID field
> in the EARO and set the T flag to indicate the presence of the TID
> (see Section 5.2).

— §5.1, `rfc8505.txt:903-905`.

- Strength: must. Class: encoding.
- Check idea: Inspect registration options for the T flag and transaction identifier.

### RFC8505-EARO-2

**A host-only registering node sets R to request reachability services.**

> When registering, a 6LN that acts only as a host MUST set the R flag
> to indicate that it is not a router and that it will not handle its
> own reachability.  A 6LR that manages its reachability SHOULD NOT set
> the R flag; if it does, routes towards this router may be installed
> on its behalf and may interfere with those it advertises.

— §5.1, `rfc8505.txt:910-914`.

- Strength: must. Class: wire.
- Check idea: Register from a host-only node and verify the R flag.

### RFC8505-TID-1

**Each re-registration increments the transaction identifier.**

> The TID is a sequence number that is incremented by the 6LN with each
> re-registration to a 6LR.  The TID is used to determine the recency
> of the registration request.  The network uses the most recent TID to
> determine the most recent known location(s) of a moving 6LN.  When a
> Registered Node is registered with multiple 6LRs in parallel, the
> same TID MUST be used.  This enables the 6LBRs and/or Routing
> Registrars to determine whether the registrations are identical and
> to distinguish that situation from a movement (for example, see
> Section 5.7 and Appendix A).

— §5.2, `rfc8505.txt:918-926`.

- Strength: description. Class: wire.
- Check idea: Renew a registration and compare transaction identifiers.

### RFC8505-TID-2

**Parallel registrations of the same transaction use the same TID.**

> The TID is a sequence number that is incremented by the 6LN with each
> re-registration to a 6LR.  The TID is used to determine the recency
> of the registration request.  The network uses the most recent TID to
> determine the most recent known location(s) of a moving 6LN.  When a
> Registered Node is registered with multiple 6LRs in parallel, the
> same TID MUST be used.  This enables the 6LBRs and/or Routing
> Registrars to determine whether the registrations are identical and
> to distinguish that situation from a movement (for example, see
> Section 5.7 and Appendix A).

— §5.2, `rfc8505.txt:918-926`.

- Strength: must. Class: wire.
- Check idea: Register concurrently with two routers and compare the transaction IDs.

### RFC8505-ROVR-1

**ROVR is scoped to one IPv6 address and cannot correlate different addresses.**

> The ROVR field replaces the EUI-64 field of the ARO defined in
> [RFC6775].  It is associated in the 6LR and the 6LBR with the
> registration state.  The ROVR can be a unique ID of the Registering
> Node, such as the EUI-64 address of an interface.  This can also be a
> token obtained with cryptographic methods that can be used in
> additional protocol exchanges to associate a cryptographic identity
> (key) with this registration to ensure that only the owner can modify
> it later, if the proof of ownership of the ROVR can be obtained.  The
> scope of a ROVR is the registration of a particular IPv6 Address, and
> it MUST NOT be used to correlate registrations of different
> addresses.

— §5.3, `rfc8505.txt:1024-1034`.

- Strength: must not. Class: internal.
- Check idea: Register two addresses with the same ROVR and verify separate registration state.

### RFC8505-ROVR-2

**Recognized different ROVR namespaces remain distinct even when values match.**

> Note regarding ROVR collisions: Different techniques for forming the
> ROVR will operate in different namespaces.  [RFC6775] specifies the
> use of EUI-64 addresses.  [AP-ND] specifies the generation of
> cryptographic tokens.  While collisions are not expected in the
> EUI-64 namespace only, they may happen if [AP-ND] is implemented by
> at least one of the nodes.  An implementation that understands the
> namespace MUST consider that ROVRs from different namespaces are
> different even if they have the same value.  An RFC 6775-only 6LBR or
> 6LR will confuse the namespaces; this slightly increases the risk of
> a ROVR collision.  A ROVR collision has no effect if the two
> Registering Nodes register different addresses, since the ROVR is
> only significant within the context of one registration.  A ROVR is
> not expected to be unique to one registration, as this specification
> allows a node to use the same ROVR to register multiple IPv6
> Addresses.  This is why the ROVR MUST NOT be used as a key to
> identify the Registering Node or as an index to the registration.  It
> is only used as a match to ensure that the node that updates a
> registration for an IPv6 Address is the node that made the original

— §5.3, `rfc8505.txt:1045-1062`.

- Strength: must. Class: internal.
- Check idea: Submit equal ROVR values in two understood namespaces for one address and verify ownership is not conflated.

### RFC8505-ROVR-3

**ROVR cannot be the registering-node key or registration index.**

> Note regarding ROVR collisions: Different techniques for forming the
> ROVR will operate in different namespaces.  [RFC6775] specifies the
> use of EUI-64 addresses.  [AP-ND] specifies the generation of
> cryptographic tokens.  While collisions are not expected in the
> EUI-64 namespace only, they may happen if [AP-ND] is implemented by
> at least one of the nodes.  An implementation that understands the
> namespace MUST consider that ROVRs from different namespaces are
> different even if they have the same value.  An RFC 6775-only 6LBR or
> 6LR will confuse the namespaces; this slightly increases the risk of
> a ROVR collision.  A ROVR collision has no effect if the two
> Registering Nodes register different addresses, since the ROVR is
> only significant within the context of one registration.  A ROVR is
> not expected to be unique to one registration, as this specification
> allows a node to use the same ROVR to register multiple IPv6
> Addresses.  This is why the ROVR MUST NOT be used as a key to
> identify the Registering Node or as an index to the registration.  It
> is only used as a match to ensure that the node that updates a
> registration for an IPv6 Address is the node that made the original

— §5.3, `rfc8505.txt:1045-1062`.

- Strength: must not. Class: internal.
- Check idea: Use the same ROVR with multiple addresses and verify updates affect only the addressed registration.

### RFC8505-ADDR-1

**An NS with EARO is a registration only when it also includes SLLAO.**

> An NS message with an EARO is a registration if and only if it also
> carries an SLLA Option ("SLLAO") [RFC6775] ("SLLA" stands for "Source
> Link-Layer Address").  The EARO can also be used in NS and NA
> messages between Routing Registrars to determine the distributed
> registration state; in that case, it does not carry the SLLA Option
> and is not confused with a registration.

— §5.5, `rfc8505.txt:1099-1104`.

- Strength: description. Class: wire.
- Check idea: Compare EARO messages with and without SLLAO; distinguish registration from registrar state queries.

### RFC8505-ADDR-2

**Extended registration identifies the registered address in the NS and NA Target Address.**

> In order to enable the latter operation, this specification changes
> the behavior of the 6LN and the 6LR so that the Registered Address is
> found in the Target Address field of the NS and NA messages as
> opposed to the Source Address field.  With this convention, a TLLA
> Option (Target Link-Layer Address Option, or "TLLAO") indicates the
> link-layer address of the 6LN that owns the address.

— §5.5, `rfc8505.txt:1131-1136`.

- Strength: description. Class: wire.
- Check idea: Register a global target from a link-local source and check which address acquires the binding.

### RFC8505-ADDR-3

**The registration source is a link-local address already registered or being registered.**

> When sending an NS(EARO) to a 6LR, a 6LN MUST use a Link-Local
> Address as the Source Address of the registration, whatever the type
> of IPv6 Address that is being registered.  That Link-Local Address
> MUST be either an address that is already registered to the 6LR or
> the address that is being registered.

— §5.6, `rfc8505.txt:1183-1187`.

- Strength: must. Class: wire.
- Check idea: Inspect initial link-local registration and subsequent global-address registration sources.

### RFC8505-ADDR-4

**Without an existing registered address, the first registration uses a link-local address as source and target.**

> When a Registering Node does not have an already-registered address,
> it MUST register a Link-Local Address, using it as both the Source
> Address and the Target Address of an NS(EARO) message.  In that case,
> it is RECOMMENDED to use an address for which DAD is not required
> (see [RFC6775]), e.g., derived from a globally unique EUI-64 address;
> using the SLLA Option in the NS is consistent with existing ND
> specifications such as [RFC4429] ("Optimistic Duplicate Address
> Detection (DAD) for IPv6").  The 6LN MAY then use that address to
> register one or more other addresses.

— §5.6, `rfc8505.txt:1194-1202`.

- Strength: must. Class: wire.
- Check idea: Start an unregistered node and check its first NS(EARO) source and target.

### RFC8505-STATE-1

**Renewal is reported by the router to the border router.**

> A node renews an existing registration by sending a new NS(EARO)
> message for the Registered Address, and the 6LR MUST report the new
> registration to the 6LBR.

— §5.7, `rfc8505.txt:1257-1259`.

- Strength: must. Class: wire.
- Check idea: Renew a registration and observe the corresponding duplicate-address exchange toward the border router.

### RFC8505-COMPAT-1

**A node uses base-compatible behavior until the router is known to support the extension.**

> This specification changes the behavior of the peers in a
> registration flow.  To enable backward compatibility, a 6LN that
> registers to a 6LR that is not known to support this specification
> MUST behave in a manner that is backward compatible with [RFC6775].
> Conversely, if the 6LR is found to support this specification, then
> the 6LN MUST conform to this specification when communicating with
> that 6LR.

— §6, `rfc8505.txt:1319-1325`.

- Strength: must. Class: wire.
- Check idea: Compare exchanges with a base-only router and a router advertising extension support.

### RFC8505-COMPAT-2

**An updated router answers both ARO and EARO requests with EARO.**

> A 6LN that supports this specification MUST always use an EARO as a
> replacement for an ARO in its registration to a router.  This
> behavior is backward compatible, since the T flag and TID field
> occupy fields that are reserved in [RFC6775] and are thus ignored by
> an RFC 6775-only router.  A router that supports this specification
> MUST answer an NS(ARO) and an NS(EARO) with an NA(EARO).  A router
> that does not support this specification will consider the ROVR as an
> EUI-64 address and treat it the same; this scenario has no
> consequence if the Registered Addresses are different.

— §6, `rfc8505.txt:1327-1335`.

- Strength: must. Class: wire.
- Check idea: Send valid requests in both formats and inspect the response option.

### RFC8505-COMPAT-3

**Capability options accompany discovery unless already known, and an RS with 6CIO always elicits RA with 6CIO.**

> [RFC7400] specifies the 6CIO, which indicates a node's capabilities
> to the node's peers.  The 6CIO MUST be present in both RS and RA
> messages, unless the 6CIO information was already shared in recent
> exchanges or pre-configured in all nodes in a network.  In any case,
> a 6CIO MUST be placed in an RA message that is sent in response to an
> RS with a 6CIO.

— §6.1, `rfc8505.txt:1353-1358`.

- Strength: must. Class: wire.
- Check idea: Exercise unknown and preconfigured capabilities, then solicit with 6CIO and inspect the response.

### RFC8505-COMPAT-4

**An updated router accepts a valid base-only host registration and uses base DAR/DAC messages.**

> An RFC 6775-only 6LN will use the Registered Address as the Source
> Address of the NS message and will not use an EARO.  An updated 6LR
> MUST accept that registration if it is valid per [RFC6775], and it
> MUST manage the binding cache accordingly.  The updated 6LR MUST then
> use the RFC 6775-only DAR and DAC messages as specified in [RFC6775]
> to indicate to the 6LBR that the TID is not present in the messages.

— §6.2, `rfc8505.txt:1376-1381`.

- Strength: must. Class: wire.
- Check idea: Register from a base-only host and inspect both the cache binding and border-router exchange.

### RFC8505-COMPAT-5

**Registration to a base-only router uses a 64-bit ROVR.**

> An updated 6LN MUST use an EARO in the request, regardless of the
> type of 6LR -- RFC 6775-only or updated; this implies that the T flag
> is set.  It MUST use a ROVR of 64 bits if the 6LR is RFC 6775-only.

— §6.3, `rfc8505.txt:1394-1396`.

- Strength: must. Class: encoding.
- Check idea: Discover a base-only router and verify the EARO length and ROVR size.

### RFC8505-CAP-1

**An updated router signals EARO support with the 6CIO E flag.**

> The E flag indicates that the EARO can be used in a registration.  A
> 6LR that supports this specification MUST set the E flag.

— §4.3, `rfc8505.txt:705-706`.

- Strength: must. Class: encoding.
- Check idea: Inspect a capability option emitted by an updated router.

### RFC8505-EARO-3

**EARO is backward compatible with ARO only when option Length is two.**

> The EARO updates the ARO and is backward compatible with the ARO if
> and only if the Length value of the option is set to 2.  The format
> of the EARO is presented in Section 4.1.  More details on backward
> compatibility can be found in Section 6.

— §5.1, `rfc8505.txt:849-852`.

- Strength: description. Class: encoding.
- Check idea: Compare Length two with longer EAROs and retain Length two for base-only peers; check the additional compatibility conditions in section 6.

## Areas outside this selection

This catalog selects EARO use, transaction and ownership identity, target-address registration, renewal and compatibility. Full option/status encodings (§4), lollipop TID comparison including wrap (§5.2.1), all movement and cleanup transitions (§5.7), border-router compatibility (§6.4), security and privacy (§§7–8), and appendices are not exhaustively cataloged. A numeric greater-than comparison is not a substitute for the omitted TID algorithm.
