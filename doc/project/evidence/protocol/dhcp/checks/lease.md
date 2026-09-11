# DHCP — English check procedures: the lease and its two timers

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md), [rfc2132/catalog.md](../../../standard/rfc2132/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

Three checks walk one lease from its start to its end. The first one needs nothing but
patience. The second and the third need a fault, because a client renews at T1 unless the
renewal fails, and it rebinds at T2 unless the rebinding fails.

## Renewal at T1

Checks: **RFC2131-LEASE-5** (description), **RFC2131-REQ-6** (must and must not),
**RFC2131-LEASE-4** (description), **RFC2131-BCAST-3** (description), **RFC2131-SRVID-3**
(must), **RFC2132-T1-1** (description). Also covers: **RFC2131-LEASE-1** (description).

### Requirement

RFC 2131 §4.4.5: at time T1 the client moves to RENEWING and sends, by unicast, a
DHCPREQUEST to the server, with `ciaddr` set to its current address. §4.3.2: in RENEWING the
`server identifier` option must not be filled in, the `requested IP address` option must not
be filled in, and `ciaddr` must be filled in. §4.4.5: T1 defaults to half the lease. §4.1: a
client must use the address of the `server identifier` option for any unicast to the server,
and a server with `giaddr` zero and `ciaddr` non-zero unicasts its DHCPACK to `ciaddr`.

### Scenario constants

- The plain mockup. The lease is 300 seconds, so T1 falls at 150 seconds after the
  assignment. The server sends no `renewal (T1) time value` option, so the default applies.
- Observation stops 200 seconds after the start, which is past T1 and well before T2.

### Procedure

1. Build the plain mockup.
2. Let the client start and get its address. Record the address of `yiaddr` and the address
   of the `server identifier` option of the DHCPACK, and the instant the DHCPACK arrived.
3. Wait past T1 and observe the next message the client sends and the answer to it.

### Expected observations

1. The client gets a DHCPACK with a lease time of 300 and an address in `yiaddr`. This
   confirms the stimulus: without a lease there is no T1 (RFC2131-LEASE-1).
2. About 150 seconds after the assignment, a DHCPREQUEST leaves the client. Its instant lies
   within one second of half the lease (RFC2131-LEASE-4, RFC2131-LEASE-5).
3. Its IP destination address is the address of the `server identifier` option of the DHCPACK,
   and not the broadcast address (RFC2131-LEASE-5, RFC2131-SRVID-3).
4. Its `ciaddr` field holds the recorded `yiaddr` (RFC2131-REQ-6).
5. Its option area holds neither code 54, `server identifier`, nor code 50, `requested IP
   address` (RFC2131-REQ-6).
6. A DHCPACK answers it, and the IP destination address of that DHCPACK is the address in
   `ciaddr` of the renewal, a unicast and not a broadcast (RFC2131-BCAST-3).

### Notes

- Observation 2 states a tolerance of one second on a 150-second instant. That is not a
  statistical check: T1 in this scenario is a fixed fraction of a fixed lease, so the instant
  is a value and not a distribution. The tolerance covers the microseconds the first exchange
  itself needed. RFC2131-LEASE-10, the random fuzz on T1, is a distribution and is out of
  scope at level 3; a run whose T1 carried a fuzz of several seconds would fail observation 2,
  and the results would have to say which of the two statements the model followed.
- Observation 5 is the interesting one. A renewing client is fully configured, so it could
  name the server and the address, and the standard forbids both. The reason is that `ciaddr`
  already names the address, and the unicast destination already names the server; a client
  that also filled the two options would look to a server like a client in SELECTING.
- Observation 3 and observation 6 are the two directions of the same rule and are not the
  same statement. One is the client's duty to unicast to the identifier; the other is the
  server's duty to answer to `ciaddr`.
- The renewal also needs the hardware address of the server, which a unicast on an Ethernet
  link cannot have without address resolution. Where the mockup resolves addresses without
  traffic, that request does not appear on the link; where it does not, an address resolution
  request before observation 2 is expected and is not a violation.

## Rebinding at T2

Checks: **RFC2131-LEASE-6** (description), **RFC2131-REQ-7** (must), **RFC2132-T2-1**
(description). Also covers: **RFC2131-LEASE-3** (must).

### Requirement

RFC 2131 §4.4.5: if no DHCPACK arrives before time T2, the client moves to REBINDING and
sends, by broadcast, a DHCPREQUEST to extend its lease, with `ciaddr` set to its current
address. §4.3.2: in REBINDING the `server identifier` option must not be filled in, the
`requested IP address` option must not be filled in, `ciaddr` must be filled in, and the
message must be broadcast to 255.255.255.255. §4.4.5: T2 defaults to 0.875 of the lease, and
T1 is earlier than T2, which is earlier than the expiry.

### Scenario constants

- The mockup with a relay on the path. **The lease is 120 seconds**, so T1 falls at 60 seconds,
  T2 at 105 seconds and the expiry at 120 seconds.
- The relay lets the first exchange through and drops the DHCPACK that answers the renewal at
  T1. The renewal therefore stays unanswered from the client's point of view, and the client
  reaches T2.
- Observation stops 115 seconds after the start: past T2 and before the expiry.

The lease is shorter here than the 300 seconds of the other checks, and the reason comes from
the standard. RFC 2131 §4.4.3 names "a reasonable period of time (60 seconds or 4 tries if using
timeout suggested in section 4.1)" for a client that waits for a DHCPACK, and a client that
gives up after that period restarts from INIT. T2 must therefore fall **inside** that period
after T1, or the client abandons the lease before it can rebind. The gap T2 − T1 is 0.375 of
the lease, so a lease under 160 seconds keeps the gap under 60 seconds. A lease of 120 seconds
gives a gap of 45 seconds, comfortably inside it.

### Procedure

1. Build the mockup with a relay between the server and the client.
2. Let the client start and get its address; the relay passes the first DHCPACK through.
3. Arm the relay to drop the DHCPACK that answers the renewal.
4. Observe the renewal at T1, the DHCPACK that the server sends in answer, the drop, and the
   next message the client sends.

### Expected observations

1. The client gets its address, and a renewal leaves it at about 60 seconds, half the lease.
   This confirms the first half of the stimulus.
2. A DHCPACK leaves the server in answer to the renewal, and it does not reach the client.
   This confirms the second half of the stimulus: the client did not hear an answer because
   the answer was removed, and not because the server stayed silent.
3. About 105 seconds after the assignment, a DHCPREQUEST leaves the client. Its instant lies
   within one second of 0.875 of the lease (RFC2131-LEASE-6).
4. Its IP destination address is 255.255.255.255 (RFC2131-REQ-7).
5. Its `ciaddr` field holds the address the client holds, and its option area holds neither
   code 54 nor code 50 (RFC2131-REQ-7).
6. The instant of observation 3 is later than the instant of the renewal of observation 1 and
   earlier than 120 seconds after the assignment (RFC2131-LEASE-3).

### Notes

- The drop of observation 2 is what makes this a level 3 check. A client that never loses a
  DHCPACK never leaves BOUND, so T2 is unreachable by observation alone.
- The relay drops the DHCPACK on the path from the server to the client and not the renewal
  on the way out. The difference matters: a dropped renewal would leave the server with no
  record of the attempt, and the check could not show that the client stopped hearing rather
  than stopped asking.
- Observation 6 establishes RFC2131-LEASE-3 for the two instants a run can see, T1 and T2,
  and for the expiry only as an upper bound. The rule is about three configured values, and a
  check reads them as three events.
- RFC 2131 §4.4.5 also says a client should wait half the remaining time before it repeats an
  unanswered renewal, RFC2131-LEASE-7. With the lease of this scenario that wait is about 22
  seconds, so a repeat may fall inside the window between T1 and T2. Such a repeat is expected
  and is not a violation; it is a DHCPREQUEST with the same shape as the one of observation 1,
  addressed by unicast, and observation 3 asks for a broadcast.

## Lease expiry

Checks: **RFC2131-LEASE-8** (must), **RFC2131-LEASE-9** (must not). Also covers:
**RFC2131-LEASE-2** (description).

### Requirement

RFC 2131 §4.4.5: if the lease expires before the client receives a DHCPACK, the client moves
to INIT, must immediately stop any other network processing, and requests initialization
parameters as if it were uninitialized. §3.7: the client must immediately discontinue use of
the previous network address. §4.4.5: if the client is given a new address it must not
continue using the previous one. §4.4.1: the client records the expiry as the send time of
its DHCPREQUEST plus the lease of the DHCPACK.

### Scenario constants

- The mockup with a relay on the path. **The lease is 120 seconds**, for the reason the check
  above gives: T2 must fall within the period a client waits for a DHCPACK, or the client
  abandons the lease before the rebinding.
- The relay lets the first exchange through and then drops the DHCPACK that answers the
  renewal at T1 and the one that answers the rebinding at T2, so that neither succeeds and the
  lease runs out.
- Observation stops 140 seconds after the start, which is past the expiry at 120 seconds.

### Procedure

1. Build the mockup with a relay between the server and the client.
2. Let the client start and get its address. Record the instant the client sent its
   DHCPREQUEST and the lease time of the DHCPACK.
3. Arm the relay to drop the DHCPACK that answers the renewal and the one that answers the
   rebinding.
4. Observe the renewal, the rebinding, and what the client does at the expiry.

### Expected observations

1. The client gets its address, and the renewal and the rebinding both leave it. This confirms
   the stimulus.
2. Each DHCPACK the server sends after the first one leaves the server and does not reach the
   client. This confirms that the failure was on the path.
3. At the recorded send instant plus 120 seconds, within one second, a message of type 1,
   DHCPDISCOVER, leaves the client (RFC2131-LEASE-8, RFC2131-LEASE-2).
4. That DHCPDISCOVER carries IP source address 0.0.0.0 and `ciaddr` 0.0.0.0: the client is
   asking as an uninitialized client and not as one that holds an address (RFC2131-LEASE-8).
5. From the expiry onward, no frame leaves the client with the expired address as its IP
   source address (RFC2131-LEASE-9, RFC2131-LEASE-8).

### Notes

- Observation 3 anchors the expiry to the **send** instant of the DHCPREQUEST, which is
  RFC2131-LEASE-2. The arrival instant of the DHCPACK is later, by the time the link and the
  server needed. On a 100 Mbit link that difference is tens of microseconds, so the one-second
  tolerance of observation 3 cannot tell the two anchors apart. The observation therefore
  establishes RFC2131-LEASE-2 only weakly, and the ledger records it as `covered` and not as
  `selected`. A check that could separate them would need a slow link or a slow server, and
  that is a sharpening candidate for the next pass.
- Observation 5 is the `MUST NOT` and the reason the check matters. A client that kept its
  expired address would break the one promise DHCP makes: that no address serves two clients
  at once. The observation is an absence, and its window opens at the expiry.
- The observation cannot cover the second half of RFC2131-LEASE-9, which is about the case
  where the new exchange returns a *different* address. In this scenario the server still
  holds the same binding, so it returns the same address. A check of the other half needs a
  server whose pool has moved on, and it is a sharpening candidate.
- "Stop any other network processing" is wider than "stop using the address". A client that
  stopped sending from the address but kept a route through it would pass observation 5 and
  not follow the sentence. The wider half is a statement about state inside the node, and the
  ledger records that bound.
