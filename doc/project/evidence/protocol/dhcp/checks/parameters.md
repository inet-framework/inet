# DHCP — English check procedures: the configuration parameters

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Two checks share one scenario. The first asks what the server returns for a parameter it has
a value for; the second asks what it returns for a parameter it does not. They are separate
checks because the second one is the `MUST NOT` of the rule, and a failure of the prohibition
must not hide the verdict on the four requirements.

## Requested parameters returned

Checks: **RFC2131-SEL-4** (must), **RFC2132-PRL-1** (must), **RFC2132-MASK-1** (must),
**RFC2132-ROUTER-1** (must). Also covers: **RFC2131-SEL-5** (the no-duplicate half).

### Requirement

RFC 2131 §4.3.1: the server must return the client's network address, the expiration time of
the lease, and the parameters the client requested; if the server has a configured value for a
requested parameter it must include that value. The server must include each requested
parameter only once. RFC 2132 §9.8: the `parameter request list` option lists the wanted option
codes. §3.3: if both the `subnet mask` and the `router` option are in a reply, the `subnet
mask` option must be first. §3.5: the length of the `router` option must always be a multiple
of 4.

### Scenario constants

- The plain mockup. The lease is 300 seconds. The subnet is 192.168.1.0/24, so the subnet mask
  the server should return is 255.255.255.0.
- The server has a value for the `router` option, code 3.
- The client asks for the `subnet mask`, code 1, and the `router`, code 3, among others.
- Observation stops one second after the start.

### Procedure

1. Build the plain mockup, and configure the server with a router address.
2. Let the client start, and record the option codes of the `parameter request list` option of
   its DHCPDISCOVER, in order.
3. Read every option of the DHCPOFFER and of the DHCPACK, in the order the octets carry them.

### Expected observations

1. The DHCPDISCOVER of the client carries code 55, `parameter request list`, and its value
   lists at least the codes 1 and 3. This confirms the stimulus (RFC2132-PRL-1).
2. The DHCPACK carries code 51, `IP address lease time`, with the value 300, and its `yiaddr` is
   an address of the subnet: the two parameters that §4.3.1 names before the requested ones
   (RFC2131-SEL-4).
3. The DHCPACK carries code 1, `subnet mask`, with the value 255.255.255.0 (RFC2131-SEL-4).
4. The DHCPACK carries code 3, `router`, and the length of its value is a multiple of 4
   (RFC2131-SEL-4, RFC2132-ROUTER-1).
5. In the DHCPACK, code 1 appears at a lower octet offset than code 3 (RFC2132-MASK-1).
6. In the DHCPACK, no option code appears twice (RFC2131-SEL-5, the no-duplicate half).
7. The DHCPOFFER carries observations 3, 4, 5 and 6 as well.

### Notes

- Observation 5 is a rule about the order of octets and not about the presence of two options.
  It exists because a client may act on the options as it reads them, and a router address is
  meaningless before the mask that says which addresses are local.
- The order half of RFC2132-PRL-1 — that the server must *try* to insert the options in the
  order the client asked for — is not observable as written. A server that tried and could not
  is indistinguishable on a link from a server that did not try. Observation 5 checks the one
  ordering rule that RFC 2132 states absolutely, in §3.3.
- RFC2131-SEL-4 has a third branch, the Host Requirements default for a parameter the server
  recognizes but was not configured with. Reaching it needs a parameter whose default RFC 1122
  or RFC 1123 states and whose value the server was not given, and the check would then have to
  assert that default. That is a sharpening candidate for a later pass, and it would bring
  RFC 1122 into the in-scope set.

## A parameter the server has no value for

Checks: **RFC2131-SEL-5** (must not).

### Requirement

RFC 2131 §4.3.1, on a parameter the client asked for: "IF the server has been explicitly
configured with a default value for the parameter, the server MUST include that value ... ELSE
IF the server recognizes the parameter as a parameter defined in the Host Requirements
Document, the server MUST include the default value for that parameter ... ELSE The server MUST
NOT return a value for that parameter". And: "The server MUST supply as many of the requested
parameters as possible and MUST omit any parameters it cannot provide."

### Scenario constants

- The plain mockup, the same one as the check above.
- The server has **no** value for the `domain name server` option, code 6, and **no** value for
  the `network time protocol servers` option, code 42. Neither has a default in the Host
  Requirements documents that a server could fall back on: both are lists of addresses that
  only a local administrator can know.
- The client asks for both, together with the two parameters the server does have.
- Observation stops one second after the start.

### Procedure

1. Build the plain mockup, and give the server no value for code 6 and none for code 42.
2. Let the client start, and confirm that its `parameter request list` names both codes.
3. Read every option of the DHCPOFFER and of the DHCPACK.

### Expected observations

1. The DHCPDISCOVER of the client carries code 55 and its value lists code 6 and code 42. This
   confirms the stimulus: the server was asked for two parameters it has no value for.
2. The DHCPACK carries no option with code 42 (RFC2131-SEL-5).
3. The DHCPACK carries no option with code 6 (RFC2131-SEL-5).
4. The DHCPOFFER carries neither code 42 nor code 6 (RFC2131-SEL-5).

### Notes

- The `MUST NOT` is worth the trouble because of what the alternative does to a client. A
  server that returned code 6 with an empty value, or with the address 0.0.0.0, would tell the
  client that the subnet has a name server at 0.0.0.0. Omitting the option leaves the client
  with no name server configured, which is what is true, and the client then keeps whatever
  default its own configuration gives it.
- Two codes and not one, because a server can fail this rule in two ways. It can return an
  option it has no value for, and it can return no option at all for a parameter it does have.
  Observation 2 and observation 3 ask about two parameters in the same state, so a server that
  omits one and returns the other fails on exactly the one it returns.
- An absent option is an observable thing here in the same sense as everywhere else in this
  pass: the check reads the option area of the message and states that the code is not in it.
