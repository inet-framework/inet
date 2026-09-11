# DHCP — English check procedures: repeating an unanswered message

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2131/catalog.md](../../../standard/rfc2131/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Discover repeated without a server

Checks: **RFC2131-RETX-1** (must), **RFC2131-RETX-3** (description).

### Requirement

RFC 2131 §3.1: the client times out and retransmits the DHCPDISCOVER message if it receives no
DHCPOFFER messages. §4.1: DHCP clients are responsible for all message retransmission, and the
client must adopt a retransmission strategy that incorporates a randomized exponential backoff
algorithm to determine the delay between retransmissions.

### Scenario constants

- The mockup with no server. The client is alone on the link with a host that runs no DHCP, so
  nothing can answer a DHCPDISCOVER.
- Observation stops 120 seconds after the start, which leaves room for at least three
  retransmissions under the figures RFC 2131 §4.1 suggests: about 4 seconds, then about 8,
  then about 16.

### Procedure

1. Build the mockup with no server: the client and a host that runs no DHCP application.
2. Let the client start.
3. Observe every message the client sends for the whole window, and record the instant of each
   one.

### Expected observations

1. A message of type 1, DHCPDISCOVER, leaves the client. Record its instant. This confirms the
   stimulus.
2. No message of type 2, DHCPOFFER, reaches the client at any time in the window. This confirms
   the other half of the stimulus: nothing answered.
3. A second DHCPDISCOVER leaves the client (RFC2131-RETX-3).
4. A third DHCPDISCOVER leaves the client.
5. The interval between the second and the third is longer than the interval between the first
   and the second (RFC2131-RETX-1, the exponential half).

### Notes

- Observation 5 is the whole of what a single run can say about the backoff. A growing interval
  is the observable consequence of an exponential rule; the rule itself, and the random part of
  it, are a distribution. RFC2131-RETX-2 states the figures — 4 seconds with a fuzz of one, then
  8, doubling to at most 64 — and a check of those figures needs many runs and a stated
  tolerance. That is level 4 work, and the ledger records RFC2131-RETX-2 as `later`.
- The check does not state a bound on the instants of observations 3 and 4, and that is
  deliberate. Any bound would be a bound on the distribution, and a run that drew an unlucky
  value would then fail for the wrong reason.
- A client that repeats the DHCPDISCOVER at a fixed interval passes observations 1 to 4 and fails
  observation 5. That is the case the check exists to separate, because a fixed interval is the
  obvious implementation and the standard forbids it with a `MUST`.
- Observation 2 needs the window of the whole run, and it overlaps the positive observations. An
  absence over the same window as a presence is exactly what this mockup can offer: the absence
  is of a message type that no node in the mockup can send.

## Request repeated when the reply is lost

Checks: **RFC2131-RETX-4** (description).

### Requirement

RFC 2131 §3.1: the client times out and retransmits the DHCPREQUEST message if it receives
neither a DHCPACK nor a DHCPNAK message; it retransmits according to the algorithm of §4.1; and
if it receives neither after employing the retransmission algorithm, the client reverts to INIT
state and restarts the initialization process.

### Scenario constants

- The mockup with a relay on the path. The lease is 300 seconds.
- The relay drops every DHCPACK the server sends, from the first one on. The client therefore
  never leaves REQUESTING by an answer.
- Observation stops 120 seconds after the start.

### Procedure

1. Build the mockup with a relay between the server and the client.
2. Arm the relay to drop every DHCPACK.
3. Let the client start.
4. Observe the first DHCPREQUEST, the DHCPACK that the server sends and the relay drops, and what
   the client sends afterwards.

### Expected observations

1. The client sends a DHCPDISCOVER, gets a DHCPOFFER, and sends a DHCPREQUEST. Record the instant
   of the DHCPREQUEST. This confirms the first half of the stimulus.
2. A DHCPACK leaves the server in answer, and it does not reach the client. This confirms the
   second half: the client heard nothing because the answer was removed, and not because the
   server stayed silent.
3. No message of type 6, DHCPNAK, reaches the client at any time in the window: neither of the
   two answers §3.1 names arrived.
4. A second DHCPREQUEST leaves the client (RFC2131-RETX-4).
5. The second DHCPREQUEST carries the same `requested IP address` option as the first one: the
   client is still asking for the address it selected, and has not started a new transaction.

### Notes

- Observation 2 is the level 3 part. A DHCPACK always arrives on a working link, so the state
  that RFC2131-RETX-4 describes cannot be reached by observation.
- Observation 5 separates a retransmission from a restart. Both produce a message from the
  client, and only the retransmission keeps the address of the earlier offer. Without it a client
  that gave up at once and began a new DHCPDISCOVER-DHCPOFFER-DHCPREQUEST cycle would pass
  observation 4.
- The second half of the statement — that the client reverts to INIT after enough attempts — has
  no observation here. How many attempts is "enough" is an implementation decision that §3.1
  leaves open, with four as an example, and the total delay of four attempts under the figures
  of §4.1 is about 60 seconds. A window long enough to see the revert with certainty would have
  to outlast an implementation that chose more attempts. The ledger records that half as covered
  only in part, and a check of the revert is a sharpening candidate for a later pass.
