# RFC 6775 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC6775-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 6775 in the selected 6LoWPAN family.
Source: [rfc6775.txt](rfc6775.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc6775.txt>. The publication version and family
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
| [RFC6775-ARO-1](#rfc6775-aro-1) | An ARO registration includes SLLAO and registers the NS source address. |
| [RFC6775-CTX-1](#rfc6775-ctx-1) | A 6CO context longer than 64 bits has option Length three. |
| [RFC6775-CTX-2](#rfc6775-ctx-2) | A context with C clear is unavailable for compression but should remain usable for decompression. |
| [RFC6775-CTX-3](#rfc6775-ctx-3) | A zero Valid Lifetime removes a context immediately. |
| [RFC6775-HOST-1](#rfc6775-host-1) | Hosts do not multicast Neighbor Solicitations. |
| [RFC6775-HOST-2](#rfc6775-host-2) | Router Solicitations include SLLAO and use a specified source address. |
| [RFC6775-HOST-3](#rfc6775-host-3) | Hosts ignore Prefix Information Options with the on-link flag set. |
| [RFC6775-HOST-4](#rfc6775-host-4) | Registration NS messages include SLLAO and cannot use the unspecified source address. |
| [RFC6775-HOST-5](#rfc6775-host-5) | A duplicate-address result stops use of the address and removes other registrations. |
| [RFC6775-ROUTER-1](#rfc6775-router-1) | Routers do not advertise the on-link flag in PIOs. |
| [RFC6775-ROUTER-2](#rfc6775-router-2) | An RS SLLAO cannot overwrite an existing neighbor entry. |
| [RFC6775-ROUTER-3](#rfc6775-router-3) | Router Advertisements include the router's SLLAO. |
| [RFC6775-ARO-2](#rfc6775-aro-2) | A base ARO request with invalid Length or nonzero Status is silently ignored. |
| [RFC6775-ARO-3](#rfc6775-aro-3) | An ARO without SLLAO or with an unspecified NS source is ignored. |
| [RFC6775-ARO-4](#rfc6775-aro-4) | Conflicting EUI-64 ownership returns duplicate status without modifying the cache. |
| [RFC6775-ARO-5](#rfc6775-aro-5) | Base registration errors use the EUI-64-derived link-local destination. |
| [RFC6775-ARO-6](#rfc6775-aro-6) | A full cache returns status two when registration requires a new entry. |
| [RFC6775-ARO-7](#rfc6775-aro-7) | Zero registration lifetime deletes the existing neighbor entry and elicits an NA. |
| [RFC6775-ARO-8](#rfc6775-aro-8) | Expired registration removes the neighbor entry. |
| [RFC6775-CTX-4](#rfc6775-ctx-4) | Multiple border routers use consistent context information and CIDs. |
| [RFC6775-ABRO-1](#rfc6775-abro-1) | Border routers using multihop distribution include ABRO and increment its version on information changes. |
| [RFC6775-ABRO-2](#rfc6775-abro-2) | Routers using RA-based multihop distribution silently ignore advertisements without ABRO. |

## Checkable statements

### RFC6775-ARO-1

**An ARO registration includes SLLAO and registers the NS source address.**

> When the ARO is used by hosts, an SLLAO (Source Link-Layer Address
> Option) [RFC4861] MUST be included, and the address that is to be
> registered MUST be the IPv6 source address of the NS message.

— §4.1, `rfc6775.txt:865-867`.

- Strength: must. Class: wire.
- Check idea: Inspect the source address and SLLAO in a base RFC 6775 registration request.
- Overridden by: [RFC8505-ADDR-2](../rfc8505/catalog.md#rfc8505-addr-2). For extended-capable peers; base-peer compatibility remains governed by RFC 8505 §6.

### RFC6775-CTX-1

**A 6CO context longer than 64 bits has option Length three.**

> Context Length:  8-bit unsigned integer.  The number of leading bits
> in the Context Prefix field that are valid.  The
> value ranges from 0 to 128.  If it is more than 64,
> then the Length MUST be 3.

— §4.2, `rfc6775.txt:966-969`.

- Strength: must. Class: encoding.
- Check idea: Encode 64-bit and 65-bit contexts and check the option length and prefix storage.

### RFC6775-CTX-2

**A context with C clear is unavailable for compression but should remain usable for decompression.**

> C:               1-bit context Compression flag.  This flag indicates
> if the context is valid for use in compression.  A
> context that is not valid MUST NOT be used for
> compression but SHOULD be used in decompression in
> case another compressor has not yet received the
> updated context information.  This flag is used to
> manage the context life cycle based on the
> recommendations in Section 7.2.

— §4.2, `rfc6775.txt:971-978`.

- Strength: must not; should. Class: encoding.
- Check idea: Advertise C zero, confirm no outgoing context compression, and decode a delayed packet using the context.

### RFC6775-CTX-3

**A zero Valid Lifetime removes a context immediately.**

> Valid Lifetime:  16-bit unsigned integer.  The length of time in
> units of 60 seconds (relative to the time the packet
> is received) that the context is valid for the
> purpose of header compression or decompression.  A
> value of all zero bits (0x0) indicates that this
> context entry MUST be removed immediately.

— §4.2, `rfc6775.txt:991-996`.

- Strength: must. Class: internal.
- Check idea: Install a context, advertise it with zero lifetime and check its removal.

### RFC6775-HOST-1

**Hosts do not multicast Neighbor Solicitations.**

> A host MUST NOT multicast an NS message.

— §5.1, `rfc6775.txt:1212-1212`.

- Strength: must not. Class: wire.
- Check idea: Exercise registration and neighbor reachability detection and verify all emitted NS destinations are unicast.

### RFC6775-HOST-2

**Router Solicitations include SLLAO and use a specified source address.**

> The RS is formatted as specified in [RFC4861] and sent to the IPv6
> all-routers multicast address (see [RFC4861] Section 6.3.7 for
> details).  An SLLAO MUST be included to enable unicast RAs in
> response.  An unspecified source address MUST NOT be used in RS
> messages.

— §5.3, `rfc6775.txt:1241-1245`.

- Strength: must; must not. Class: wire.
- Check idea: Initialize a host and inspect its Router Solicitation source and options.

### RFC6775-HOST-3

**Hosts ignore Prefix Information Options with the on-link flag set.**

> Should the host erroneously receive a PIO with the L (on-link) flag
> set, then that PIO MUST be ignored.

— §5.4, `rfc6775.txt:1274-1275`.

- Strength: must. Class: internal.
- Check idea: Inject such a PIO and verify it does not create the advertised on-link state.

### RFC6775-HOST-4

**Registration NS messages include SLLAO and cannot use the unspecified source address.**

> The host triggers sending NS messages containing an ARO when a new
> address is configured, when it discovers a new default router, or
> well before the Registration Lifetime expires.  Such an NS MUST
> include an SLLAO, since the router needs to record the link-layer
> address of the host.  An unspecified source address MUST NOT be used
> in NS messages.

— §5.5.1, `rfc6775.txt:1379-1384`.

- Strength: must; must not. Class: wire.
- Check idea: Trigger initial registration and renewal and inspect both messages.

### RFC6775-HOST-5

**A duplicate-address result stops use of the address and removes other registrations.**

> The address registration procedure may fail for two reasons: no
> response to NSs is received (NUD failure), or an ARO with a failure
> Status (Status > 0) is received.  In the case of NUD failure, the
> entry for that router will be removed; thus, address registration is
> no longer of importance.  When an ARO with a non-zero Status field is
> received, this indicates that registration for that address has
> failed.  A failure Status of one indicates that a duplicate address
> was detected, and the procedure described in [RFC4862] Section 5.4.5
> is followed.  The host MUST NOT use the address it tried to register.
> If the host has valid registrations with other routers, these MUST be
> removed by registering with each using a zero ARO lifetime.

— §5.5.3, `rfc6775.txt:1419-1429`.

- Strength: must not; must. Class: wire.
- Check idea: Return status one while another registration exists and check cessation and zero-lifetime deregistration.

### RFC6775-ROUTER-1

**Routers do not advertise the on-link flag in PIOs.**

> A router MUST NOT set the L (on-link) flag in the PIOs, since that
> might trigger hosts to send multicast NSs.

— §6.1, `rfc6775.txt:1582-1583`.

- Strength: must not. Class: wire.
- Check idea: Inspect PIOs emitted by both router roles.

### RFC6775-ROUTER-2

**An RS SLLAO cannot overwrite an existing neighbor entry.**

> An RS might be received from a host that has not yet registered its
> address with the router.  Thus, the router MUST NOT modify an
> existing NCE based on the SLLAO from the RS.  However, a router MAY
> create a Tentative NCE based on the SLLAO.  Such a Tentative NCE
> SHOULD be timed out in TENTATIVE_NCE_LIFETIME seconds, unless a
> registration converts it into a Registered NCE.

— §6.3, `rfc6775.txt:1616-1621`.

- Strength: must not; may. Class: internal.
- Check idea: Send an RS with a changed link-layer address for an existing neighbor and check the binding remains intact.

### RFC6775-ROUTER-3

**Router Advertisements include the router's SLLAO.**

> A 6LR or 6LBR MUST include an SLLAO in the RAs it sends; this is
> required so that the hosts will know the link-layer address of the
> router.  Unlike in [RFC4861], the maximum value of the RA Router
> Lifetime field MAY be up to 0xFFFF (approximately 18 hours).

— §6.3, `rfc6775.txt:1631-1634`.

- Strength: must. Class: wire.
- Check idea: Solicit advertisements from both router roles and inspect their options.

### RFC6775-ARO-2

**A base ARO request with invalid Length or nonzero Status is silently ignored.**

> In addition to the normal validation of an NS and its options, the
> ARO is verified as follows (if present).  If the Length field is not
> two, or if the Status field is not zero, then the NS is silently
> ignored.

— §6.5, `rfc6775.txt:1657-1660`.

- Strength: description. Class: wire.
- Check idea: Inject each malformed request and check that it produces neither a registration nor a response.
- Overridden by: [RFC8505-EARO-3](../rfc8505/catalog.md#rfc8505-earo-3). For extended option length handling; the base Length-two rule still applies to RFC 6775-only registration.

### RFC6775-ARO-3

**An ARO without SLLAO or with an unspecified NS source is ignored.**

> If the source address of the NS is the unspecified address, or if no
> SLLAO is included, then any included ARO is ignored, that is, the NS
> is processed as if it did not contain an ARO.

— §6.5, `rfc6775.txt:1662-1664`.

- Strength: description. Class: internal.
- Check idea: Supply each invalid context and verify that normal NS processing does not create a registration from the ARO.

### RFC6775-ARO-4

**Conflicting EUI-64 ownership returns duplicate status without modifying the cache.**

> If the NS contains a valid ARO, then the router inspects its Neighbor
> Cache on the arriving interface to see if it is a duplicate.  It
> isn't a duplicate if (1) there is no NCE for the IPv6 source address
> of the NS or (2) there is such an NCE and the EUI-64 is the same.
> Otherwise, it is a duplicate address.  Note that if multihop DAD
> (Section 8.2) is used, then the checks are slightly different, to
> take into account Tentative NCEs.  In the case where it is a
> duplicate address, then the router responds with a unicast NA message
> with the ARO Status field set to one (to indicate that the address is
> a duplicate) as described in Section 6.5.2.  In this case, there is
> no modification to the Neighbor Cache.

— §6.5.1, `rfc6775.txt:1668-1678`.

- Strength: description. Class: error-signal.
- Check idea: Register one address with two owners and verify status one plus preservation of the original entry; apply the multihop-DAD qualification in the source.
- Overridden by: [RFC8505-ROVR-1](../rfc8505/catalog.md#rfc8505-rovr-1). For the ownership identifier in extended registration; duplicate detection also requires the transaction rules of RFC 8505 §5.2.

### RFC6775-ARO-5

**Base registration errors use the EUI-64-derived link-local destination.**

> Address registration errors are not sent back to the source address
> of the NS due to a possible risk of L2 address collision.  Instead,
> the NA is sent to the link-local IPv6 address with the Interface ID
> part derived from the EUI-64 field of the ARO as per [RFC4944].  In
> particular, this means that the universal/local bit needs to be
> inverted.  The NA is formatted with a copy of the ARO from the NS,
> but with the Status field set to indicate the appropriate error.

— §6.5.2, `rfc6775.txt:1689-1695`.

- Strength: description. Class: wire.
- Check idea: Cause a short-address registration error and inspect the returned NA's IPv6 and link-layer destinations. This entry describes base RFC 6775 behavior; extended peers use RFC 8505 §§5.5–5.6 and its compatibility rules.

### RFC6775-ARO-6

**A full cache returns status two when registration requires a new entry.**

> If the ARO did not result in a duplicate address being detected as
> above, then if the Registration Lifetime is non-zero the router
> creates (if it didn't exist) or updates (otherwise) an NCE for the
> IPv6 source address of the NS.  If the Neighbor Cache is full and a
> new entry needs to be created, then the router responds with a
> unicast NA with the ARO Status field set to two (to indicate that the
> router's Neighbor Cache is full) as described in Section 6.5.2.

— §6.5.3, `rfc6775.txt:1704-1710`.

- Strength: description. Class: error-signal.
- Check idea: Fill the cache and attempt a new valid registration; verify the error without eviction of registered neighbors.

### RFC6775-ARO-7

**Zero registration lifetime deletes the existing neighbor entry and elicits an NA.**

> If the ARO contains a zero Registration Lifetime, then any existing
> NCE for the IPv6 source address of the NS MUST be deleted and an NA
> sent as above.

— §6.5.3, `rfc6775.txt:1719-1721`.

- Strength: must. Class: wire.
- Check idea: Deregister an owned address and check the response and loss of registered reachability.

### RFC6775-ARO-8

**Expired registration removes the neighbor entry.**

> Should the Registration Lifetime in an NCE expire, then the router
> MUST delete the cache entry.

— §6.5.3, `rfc6775.txt:1723-1724`.

- Strength: must. Class: internal.
- Check idea: Advance past registration expiry without renewal and inspect the cache.

### RFC6775-CTX-4

**Multiple border routers use consistent context information and CIDs.**

> If the LoWPAN uses header compression [RFC6282] with context, then
> the 6LBR must be configured with context information and related
> CIDs.  If the LoWPAN has multiple 6LBRs, then they MUST be configured
> with the same context information and CIDs.  As noted in [RFC6282],
> maintaining consistency of context information is crucial for
> ensuring that packets will be decompressed correctly.

— §7.2, `rfc6775.txt:1827-1832`.

- Strength: must. Class: encoding.
- Check idea: Use packets compressed under each border router's advertised context and verify identical decompression at shared receivers.

### RFC6775-ABRO-1

**Border routers using multihop distribution include ABRO and increment its version on information changes.**

> 6LBRs supporting multihop prefix and context distribution MUST
> include an ABRO in each of their RAs.  The ABRO Version Number field
> is used to keep prefix and context information consistent throughout
> the LoWPAN, along with the guidelines in Section 7.2.  Each time any
> information in the set of PIOs or 6COs changes, the ABRO version is
> increased by one.

— §8.1.1, `rfc6775.txt:1922-1927`.

- Strength: must; description. Class: wire.
- Check idea: Change a PIO or 6CO and compare the ABRO and version in successive advertisements.

### RFC6775-ABRO-2

**Routers using RA-based multihop distribution silently ignore advertisements without ABRO.**

> If a received RA does not contain an ABRO, then the RA MUST be
> silently ignored.

— §8.1.3, `rfc6775.txt:1951-1952`.

- Strength: must. Class: internal.
- Check idea: Enable that distribution mode, inject an RA without ABRO, and verify no prefix or context update.

## Areas outside this selection

This catalog selects host/router registration, context use, cache expiry and selected ABRO distribution rules. Full option bit layouts (§4), discovery backoff, sleeping and reachability timers (§5), context retirement (§7.2), ABRO version arithmetic and full DAR/DAC multihop DAD procedures (§8), constants (§9), security (§12) and deployment guidance (§14) remain outside this selection. The substitute-mechanism conditions in §§1.4 and 8 apply to the selected multihop-distribution entries.
