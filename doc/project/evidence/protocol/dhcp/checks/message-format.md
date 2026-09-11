# DHCP — English check procedures: message framing

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Message framing on the wire

Checks: **RFC2131-MSG-2** (description), **RFC2131-MSG-4** (must), **RFC2131-MSG-7** (must),
**RFC2131-MSG-8** (description), **RFC2131-MSG-11** (description), **RFC2132-FMT-1**
(description), **RFC2132-FMT-3** (description), **RFC2132-FMT-4** (description),
**RFC2132-END-1** (description).

### Requirement

RFC 2131 §3: the first four octets of the `options` field hold the decimal values 99, 130,
83 and 99. §4.1: the last option is always the `end` option. §2: the reserved bits of `flags`
must be zero from a client. Table 1: the client sets `hops` to zero. §4.1: an option appears
only once. RFC 2132 §2: an option is a tag octet, a length octet that counts only the value,
and the value; only the codes 0 and 255 carry no length octet; every multi-octet value is in
network byte order. §3.2: the `end` option is code 255 and one octet long.

### Scenario constants

- The plain mockup. The lease is 300 seconds, which makes the lease time option a
  multi-octet value whose octets the check can read: 300 is 0x0000012C, so the four octets
  are 00 00 01 2C in network byte order.

### Procedure

1. Build the plain mockup.
2. Let the client start, so that all four messages of the exchange pass over the link.
3. For each of the four messages, read the octets of the message as they travel, and walk the
   option area from its first octet: the four cookie octets, then one option at a time by its
   tag and its length, to the `end` option.

### Expected observations

1. All four messages of the exchange pass over the link. This confirms the stimulus.
2. In each of the four, the four octets that follow the fixed part are 99, 130, 83, 99
   (RFC2131-MSG-2, RFC2132-FMT-4).
3. In each of the four, the option area parses from the first octet to its end by the rule
   "tag, length, value", with no octet left over and no length that runs past the end of the
   message (RFC2132-FMT-1).
4. In each of the four, the last octet of the option area is 255, the `end` option, and it
   carries no length octet (RFC2131-MSG-4, RFC2132-END-1).
5. In each of the four, no option code appears twice (RFC2131-MSG-11).
6. In the DHCPACK, the value of option 51, `IP address lease time`, is the four octets
   00 00 01 2C (RFC2132-FMT-3).
7. In the two messages the client sends, the 15 bits of `flags` below the top bit are zero
   (RFC2131-MSG-7), and `hops` is 0 (RFC2131-MSG-8).

### Notes

- This is the one check of the pass that reads a message as octets and not as fields. That is
  deliberate: every other check asks what a field holds, and these statements ask how the
  fields are laid out. A message whose fields are all right and whose option area does not
  parse is a message no other node can read.
- Observation 3 is the strongest of the seven and the hardest to state simply. Walking the
  area to its end, with nothing left over, establishes the whole of RFC 2132 §2 at once: a
  wrong length octet, a missing length octet or a value that overruns the area all stop the
  walk.
- Observation 6 uses the lease time because it is the one multi-octet number in the exchange
  whose value the scenario fixes. A check on an address would not test byte order well: an
  address is four octets that are already written in order, and a reversed one would be a
  different address, which observation 2 of another check would catch first.
- Observation 5 covers the first half of RFC2131-MSG-11 only. The second half, that a client
  concatenates the values of several instances of one option, cannot be observed here,
  because no conforming sender in the mockup sends a repeated option. RFC 3396 governs that
  half and is out of the in-scope set.
