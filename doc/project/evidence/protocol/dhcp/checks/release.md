# DHCP — English check procedures: giving the address back

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Release on shutdown

Checks: **RFC2131-REL-1** (may), **RFC2131-REL-3** (description), **RFC2131-REL-5** (must and
must not). Also covers: **RFC2131-REL-2** (must).

### Requirement

RFC 2131 §3.1: the client may choose to relinquish its lease on an address by sending a
DHCPRELEASE message to the server, and it identifies the lease with its `client identifier`, or
with `chaddr` and the address. §4.4.6: if the client no longer requires use of its assigned
address, for example because it is gracefully shut down, the client sends a DHCPRELEASE.
§4.4.4: the client unicasts DHCPRELEASE messages to the server. Table 5, DHCPRELEASE column:
`ciaddr` holds the client's address, the `server identifier` option is a `MUST`, and the
`requested IP address` and `IP address lease time` options are a `MUST NOT`. §3.1: a client that
used a `client identifier` to obtain the lease must use the same one in the DHCPRELEASE.

### Scenario constants

- The plain mockup. The lease is 300 seconds.
- The client is shut down gracefully 60 seconds after it gets its address: well inside the
  lease, so the address is still its own, and well before T1 at 150 seconds, so no renewal can
  be confused with the release.
- Observation stops 90 seconds after the start.

### Procedure

1. Build the plain mockup.
2. Let the client start and get its address. Record `yiaddr`, the `server identifier` option and
   the `client identifier` option of the exchange.
3. Shut the client down gracefully at 60 seconds.
4. Observe every message the client sends from the shutdown to the end of the window.

### Expected observations

1. The client holds an address: a DHCPACK with a `yiaddr` reached it, and no DHCPNAK did. This
   confirms the first half of the stimulus.
2. The client's shutdown begins at 60 seconds. This confirms the second half of the stimulus.
3. A message of type 7, DHCPRELEASE, leaves the client (RFC2131-REL-1).
4. Its IP destination address is the address of the recorded `server identifier`, a unicast and
   not 255.255.255.255 (RFC2131-REL-3).
5. Its `ciaddr` field holds the recorded `yiaddr` (RFC2131-REL-5).
6. Its option area holds code 54, `server identifier`, and holds neither code 50, `requested IP
   address`, nor code 51, `IP address lease time` (RFC2131-REL-5).
7. Its option area holds code 61, `client identifier`, with the recorded octets
   (RFC2131-REL-2).

### Notes

- This check targets a `MAY`, and §4.4.6 says in one sentence why: "the correct operation of
  DHCP does not depend on the transmission of DHCPRELEASE messages". A client that sends none
  loses nothing but the server's chance to free the address early; the lease runs out instead.
  So a failure at observation 3 is a `declined` in the conformance matrix and not a defect, and
  the results must say so. This is the clearest case in the whole pass of a check that must not
  be read as a verdict on correctness.
- Observations 4 to 7 all depend on observation 3. When no DHCPRELEASE leaves the client there is
  nothing to read, and the four field rules stay untested rather than failed. The ledger records
  that distinction.
- Observation 5 and observation 6 together are the shape of the message, and the pair is
  unusual: DHCPRELEASE is the one client message that names the address in `ciaddr` and is
  forbidden to name it in the `requested IP address` option. A client that filled option 50 out
  of habit would look to the server like a client asking for the address, not giving it up.
- The server's half, RFC2131-REL-4, is that it marks the address as not allocated. Seeing it
  needs a second client to ask afterwards and to get that same address. That is a sharpening
  candidate; the ledger records the statement without a check.
