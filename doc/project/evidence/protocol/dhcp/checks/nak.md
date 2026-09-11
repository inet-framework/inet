# DHCP — English check procedures: the negative acknowledgement

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Both checks of this file put a crafted DHCPREQUEST onto the link. A client of the mockup
reaches INIT-REBOOT only after a restart that keeps a remembered address, and neither check
can wait for that: one needs an address of a foreign subnet and the other an address the
server never gave out. A crafted message states both cases exactly, and it stands for a
message from a client that has moved.

## Negative acknowledgement for a wrong subnet

Checks: **RFC2131-NAK-2** (should), **RFC2131-NAK-3** (must), **RFC2131-NAK-5** (must not,
six times), **RFC2131-NAK-6** (must), **RFC2131-REQ-5** (must and must not).

### Requirement

RFC 2131 §4.3.2: if the server detects that the client is on the wrong net — the result of
applying the subnet mask to the `requested IP address` option does not match reality — then
the server should send a DHCPNAK. §4.3.2 and §4.1: if `giaddr` is zero, the server must
broadcast the DHCPNAK to 255.255.255.255, because the client may hold a wrong address or a
wrong mask and may answer no address resolution request. Table 3, DHCPNAK column: `server
identifier` is a `MUST`; `requested IP address`, `IP address lease time`, the overloaded
fields, `parameter request list`, `maximum message size` and all other options are a `MUST
NOT`; `ciaddr`, `yiaddr` and `siaddr` are 0. §4.3.2: a DHCPREQUEST from INIT-REBOOT names no
server, carries the remembered address in the `requested IP address` option, and has `ciaddr`
zero.

### Scenario constants

- The mockup with an injector. The server holds 192.168.1.1 and serves 192.168.1.0/24.
- The crafted message is a DHCPREQUEST of a client in INIT-REBOOT: `op` BOOTREQUEST, a
  transaction identifier the check records, `ciaddr` 0.0.0.0, `giaddr` 0.0.0.0, a hardware
  address of its own in `chaddr`, the `requested IP address` option set to **10.0.0.7**, and
  no `server identifier` option. 10.0.0.7 is not an address of the server's subnet, so the
  subnet mask test of §4.3.2 fails for it.
- The message is a broadcast to 255.255.255.255 and UDP port 67, from IP source address
  0.0.0.0, as §4.4.2 and §4.1 require.
- Observation stops one second after the injection.

### Procedure

1. Build the mockup with an injector, with the server running and a client that already holds
   an address, so that the server has a binding and a pool in a known state.
2. Put the crafted DHCPREQUEST onto the link.
3. Observe every message the server sends afterwards.

### Expected observations

1. The crafted DHCPREQUEST reaches the server: a message of type 3, DHCPREQUEST, arrives at
   the server with the `requested IP address` option holding 10.0.0.7 and no `server
   identifier` option. This confirms the stimulus, and it also confirms that the crafted
   message has the shape RFC2131-REQ-5 demands of a DHCPREQUEST from INIT-REBOOT.
2. A message of type 6, DHCPNAK, leaves the server, with the recorded transaction identifier
   (RFC2131-NAK-2).
3. Its IP destination address is 255.255.255.255 and its link-layer destination address is the
   broadcast address (RFC2131-NAK-3).
4. Its option area holds code 54, `server identifier` (RFC2131-NAK-6).
5. Its option area holds no code 50, no code 51, no code 52, no code 55 and no code 57, and no
   configuration option such as code 1, `subnet mask`, or code 3, `router` (RFC2131-NAK-5).
6. Its `ciaddr`, its `yiaddr` and its `siaddr` are 0.0.0.0 (RFC2131-NAK-5).
7. No message of type 5, DHCPACK, leaves the server in answer to the crafted message.

### Notes

- Observation 3 is the strongest statement of the check and the easiest for a server to get
  wrong. The obvious thing to do with a reply is to unicast it to the address the client asked
  about; RFC 2131 forbids exactly that, because a client that gets a DHCPNAK is by definition
  a client whose idea of its own address is wrong.
- Observation 5 covers the `All others MUST NOT` row of table 3, which is the widest
  prohibition in the document. A check cannot enumerate every option that must not be there,
  so it names the ones the server sends in a DHCPOFFER and a DHCPACK: those are the ones a
  server would carry over by mistake.
- Observation 7 is the negative half. A server that answered a request for a foreign address
  with both a DHCPNAK and a DHCPACK would pass observations 2 to 6 and would have given away
  an address it was refusing.
- The check targets a `SHOULD`, so silence from the server is a `declined` and not a defect.
  Observations 3 to 6 then have nothing to read, and the results record which of the two
  happened.

## Silence for an unknown client

Checks: **RFC2131-NAK-7** (must).

### Requirement

RFC 2131 §4.3.2: if a client in INIT-REBOOT asks about an address on the correct network and
the DHCP server has no record of this client, then the server must remain silent, and may
output a warning to the network administrator. "This behavior is necessary for peaceful
coexistence of non-communicating DHCP servers on the same wire."

### Scenario constants

- The mockup with an injector. The server holds 192.168.1.1 and serves 192.168.1.0/24, and
  gives out addresses from 192.168.1.100.
- The crafted message is a DHCPREQUEST of a client in INIT-REBOOT, of the same shape as in the
  check above, with the `requested IP address` option set to **192.168.1.200** and a hardware
  address in `chaddr` that belongs to no client of the mockup. The address is inside the
  server's subnet, so the subnet test passes; the server has no binding for that hardware
  address, so it knows nothing about the client.
- Observation stops one second after the injection.

### Procedure

1. Build the mockup with an injector, with the server running and one client that already
   holds an address.
2. Put the crafted DHCPREQUEST onto the link.
3. Watch every message the server sends for the whole window.

### Expected observations

1. The crafted DHCPREQUEST reaches the server, with the `requested IP address` option holding
   192.168.1.200, no `server identifier` option, and a `chaddr` that no client of the mockup
   uses. This confirms the stimulus.
2. From that instant to the end of the window, the server sends no message at all: no DHCPNAK
   and no DHCPACK (RFC2131-NAK-7).

### Notes

- This check and the one above ask the same question with one value changed, and they expect
  opposite answers. The pair is the point: a server that answers both with a DHCPNAK, and a
  server that answers neither, each pass one check and fail the other. Neither check alone
  establishes the rule.
- Observation 2 is an absence over a whole node, and its window opens the instant the crafted
  message arrives. That anchor is what observation 1 provides; without it the absence could be
  the absence of a stimulus.
- The address 192.168.1.200 is inside the subnet and above the pool the server gives out, so
  no ordinary exchange of the mockup can have bound it. A value inside the pool would make the
  outcome depend on how many clients had asked before.
- The reason RFC 2131 gives for the silence is worth keeping in view while reading a failure:
  two servers that do not talk to each other may serve one wire, and an address this server
  never gave out may be the other server's. A DHCPNAK would then take an address away from a
  client that holds it correctly.
