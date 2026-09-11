# DHCP — English check procedures: parameters without a lease

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Inform answered without a lease

Checks: **RFC2131-INF-1** (may and should not), **RFC2131-INF-2** (description and should),
**RFC2131-INF-3** (must not and should not), **RFC2131-INF-5** (must not). Also covers:
**RFC2131-ACK-3** (the must-not half), **RFC2131-INF-6** (must).

### Requirement

RFC 2131 §3.4: a client that has obtained an address by other means may use a DHCPINFORM to
obtain other local configuration parameters; the server constructs a DHCPACK with the local
parameters without allocating an address, without checking for an existing binding, without
filling in `yiaddr` and without lease time parameters, and should unicast it to the address in
the `ciaddr` field of the DHCPINFORM. §4.3.5: the server must not send a lease expiration time
and should not fill in `yiaddr`. §4.4.3: the client puts its own address in `ciaddr`, should not
request lease time parameters, and must direct the message to the DHCP server port. Table 5,
DHCPINFORM column: `requested IP address` and `IP address lease time` are a `MUST NOT`.
Table 3, DHCPACK column: `IP address lease time` is a `MUST NOT` in the answer to a DHCPINFORM.

### Scenario constants

- The mockup with an injector. The server holds 192.168.1.1 and serves 192.168.1.0/24.
- A host in the mockup holds **192.168.1.50** by static configuration. That address is not in
  the server's pool, so the server has no binding for it and could not have given it out.
- The crafted message is a DHCPINFORM: `op` BOOTREQUEST, a transaction identifier the check
  records, `ciaddr` 192.168.1.50, `yiaddr` 0.0.0.0, `giaddr` 0.0.0.0, the hardware address of
  the statically configured host in `chaddr`, a `parameter request list` option asking for code
  1, `subnet mask`, and code 3, `router`, and no `requested IP address` option and no `IP
  address lease time` option.
- The message is a unicast to the server, from IP source address 192.168.1.50, to UDP port 67.
- Observation stops one second after the injection.

### Procedure

1. Build the mockup with an injector, and give one host the static address 192.168.1.50.
2. Put the crafted DHCPINFORM onto the link, addressed to the server.
3. Observe the answer.

### Expected observations

1. The crafted DHCPINFORM reaches the server: a message of type 8, DHCPINFORM, arrives with
   `ciaddr` 192.168.1.50, with the recorded transaction identifier, and with UDP destination
   port 67. Its option area holds code 55 and holds neither code 50 nor code 51. This confirms
   the stimulus, and it establishes RFC2131-INF-1, RFC2131-INF-5 and RFC2131-INF-6 for the
   crafted message as a well-formed DHCPINFORM.
2. A message of type 5, DHCPACK, leaves the server, with the recorded transaction identifier
   (RFC2131-INF-2).
3. Its IP destination address is 192.168.1.50, a unicast to the `ciaddr` of the DHCPINFORM, and
   not 255.255.255.255 (RFC2131-INF-2).
4. Its option area holds no code 51, `IP address lease time` (RFC2131-INF-3, RFC2131-ACK-3).
5. Its option area holds no code 58 and no code 59, the two lease timers (RFC2131-INF-3).
6. Its `yiaddr` field is 0.0.0.0 (RFC2131-INF-3).
7. Its option area holds code 1, `subnet mask`, and code 3, `router`: the parameters the
   DHCPINFORM asked for (RFC2131-INF-2).
8. No message of type 2, DHCPOFFER, and no message of type 6, DHCPNAK, leaves the server in
   answer to the crafted message.

### Notes

- Observation 1 does double duty, and that is worth stating plainly. The statements about the
  shape of a DHCPINFORM — `ciaddr` is the client's address, no `requested IP address`, no lease
  time, port 67 — are checked on the **crafted** message and not on a message a node built. So
  they establish that the stimulus is a legal DHCPINFORM; they say nothing about whether a
  client of the mockup would build one correctly. The ledger records that bound: those three
  entries are `covered` by the stimulus and not `selected`.
- Observations 4, 5 and 6 are the heart of the check, and they are all absences. A DHCPINFORM
  asks for the second service of DHCP without the first, and the answer must not smuggle the
  first one back in. A lease time in the answer would tell the client it holds an address on
  loan, which it does not: it owns 192.168.1.50 by configuration, and the server was told not
  even to look for a binding.
- Observation 8 is the other negative half. A server that answered a DHCPINFORM with a
  DHCPOFFER would have allocated an address that nobody asked for.
- RFC2131-INF-4, that the server must not look for an existing lease, has no check here. Its
  consequence is invisible from the link: a server that looked and found nothing behaves
  exactly like a server that did not look. Seeing the difference needs a DHCPINFORM for an
  address that **is** bound to another client, and a check that the answer is the same. That is
  a sharpening candidate.
- The address 192.168.1.50 is below the pool on purpose. An address inside the pool would let a
  server pass observation 3 by treating the message as a renewal.
