# DHCP — English check procedures: the configuration parameters

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Requested parameters returned

Checks: **RFC2131-SEL-4** (must), **RFC2131-SEL-5** (must and must not), **RFC2132-PRL-1**
(must), **RFC2132-MASK-1** (must), **RFC2132-ROUTER-1** (must).

### Requirement

RFC 2131 §4.3.1: the server must return the client's network address, the expiration time of the
lease, and the parameters the client requested; if the server has a configured value for a
requested parameter it must include that value, else if it recognizes the parameter as one of the
Host Requirements Document it must include that default, else it must not return a value for
that parameter. The server must supply as many of the requested parameters as possible, must omit
any it cannot provide, and must include each requested parameter only once. RFC 2132 §9.8: the
`parameter request list` option lists the wanted option codes, and the server must try to insert
the requested options in the order the client asked for. §3.3: if both the `subnet mask` and the
`router` option are in a reply, the `subnet mask` option must be first. §3.5: the length of the
`router` option must always be a multiple of 4.

### Scenario constants

- The plain mockup. The lease is 300 seconds. The subnet is 192.168.1.0/24, so the subnet mask
  the server should return is 255.255.255.0.
- The server has a configured value for the `router` option and none for the `network time
  protocol servers` option, code 42. The client asks for both, plus the `subnet mask`, code 1,
  and the `domain name server`, code 6.
- So the reply must carry the two parameters the server has values for, and must carry no option
  for the one it has no value for.
- Observation stops one second after the start.

### Procedure

1. Build the plain mockup, and configure the server with a router address and with no network
   time protocol server.
2. Let the client start, and record the option codes of the `parameter request list` option of
   its DHCPDISCOVER, in order.
3. Read every option of the DHCPOFFER and of the DHCPACK, in the order the octets carry them.

### Expected observations

1. The DHCPDISCOVER of the client carries code 55, `parameter request list`, and its value lists
   the codes 1, 3, 6 and 42. This confirms the stimulus (RFC2132-PRL-1).
2. The DHCPACK carries code 51, `IP address lease time`, with the value 300, and its `yiaddr` is
   an address of the subnet: the two parameters that §4.3.1 names before the requested ones
   (RFC2131-SEL-4).
3. The DHCPACK carries code 1, `subnet mask`, with the value 255.255.255.0 (RFC2131-SEL-4).
4. The DHCPACK carries code 3, `router`, and the length of its value is a multiple of 4
   (RFC2131-SEL-4, RFC2132-ROUTER-1).
5. In the DHCPACK, code 1 appears at a lower octet offset than code 3 (RFC2132-MASK-1).
6. The DHCPACK carries no code 42: the server had no value for it and returned none rather than
   an empty or invented one (RFC2131-SEL-5).
7. In the DHCPACK, no option code appears twice (RFC2131-SEL-5).
8. The DHCPOFFER carries the same four observations 3 to 6 as the DHCPACK.

### Notes

- Observation 6 is the `MUST NOT` of the check and the one worth the trouble. A server that
  returned code 42 with an empty value, or with a zero address, would tell the client that the
  subnet has a time server at 0.0.0.0. Omitting the option leaves the client with the default of
  the Host Requirements documents, which is what §4.3.1 intends.
- Observation 5 is a rule about the order of octets and not about the presence of two options. It
  exists because a client may act on the options as it reads them, and a router address is
  meaningless before the mask that says which addresses are local.
- The check says nothing about code 6, `domain name server`, and that is deliberate. Whether the
  server has a value for it depends on the configuration, and the scenario does not fix it. A
  reply that carries it and a reply that does not are both conforming, so no observation can be
  written.
- The order half of RFC2132-PRL-1 — that the server must *try* to insert the options in the order
  the client asked for — is not observable as written. A server that tried and could not is
  indistinguishable on a link from a server that did not try. Observation 5 checks the one
  ordering rule that RFC 2132 states absolutely, in §3.3.
- RFC2131-SEL-4 has a third branch, the Host Requirements default for a parameter the server
  recognizes but was not configured with. Reaching it needs a parameter whose default RFC 1122 or
  RFC 1123 states and whose value the server was not given, and the check would then have to
  assert that default. That is a sharpening candidate for a later pass, and it would bring
  RFC 1122 into the in-scope set.
