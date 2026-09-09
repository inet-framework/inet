# IPv6 checks — hop limit

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV6-F-HOP-LIMIT. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## Hop limit expiry

Checks: **RFC8200-HL-2** (description), **RFC4443-TE-1** (must), **RFC4443-ERR-2**
(description).

### Requirement

RFC 8200 §3: when forwarding, a node discards a packet whose hop limit is decremented to
zero. RFC 4443 §3.3: a router that decrements a hop limit to zero must discard the packet
and originate a Time Exceeded message, type 3 code 0, to the source of the packet.

### Scenario constants

- Application data: 100 octets, to port 5000.
- The sender hop limit on host A is 1: the packet survives exactly one hop. The router
  decrements it to zero and must discard it.

### Procedure

1. Build the mockup network.
2. Set the sender hop limit on host A to 1.
3. Let host A send the datagram.
4. Observe link 1, the router's own origination of messages, link 2, and the messages that
   return to host A.

### Expected observations

1. On link 1, the packet appears with hop limit 1 and destination port 5000. This confirms
   the stimulus.
2. The router originates a Time Exceeded message, type 3 code 0: the message is recorded
   at the router at the moment of the discard (RFC8200-HL-2, RFC4443-TE-1). This is the
   observation that is in time — see the notes.
3. Host A receives that Time Exceeded message, type 3 code 0, addressed to host A
   (RFC4443-ERR-2).
4. From then on, the packet never appears on link 2, and host B never receives it
   (RFC8200-HL-2, the wire form of observation 2).

The check passes if observations 1 to 3 occur in this order and observation 4 records no
traffic of the packet on link 2 within the time limit.

### Notes

- Observation 2 is recorded at the router rather than on the wire. A watch on link 2 that
  starts only after the report arrives at host A starts too late: a forbidden forward would
  leave the router at the moment of the decision, before the report crosses link 1. The
  router's own origination of the report is available at that moment; the wire absence of
  observation 4 then guards the rest of the window.
- Unlike the IPv4 check, the report is a must, and its absence is a violation of RFC 4443.


## Hop limit 1 at the destination

Checks: **RFC8200-HL-3** (should). Also covers: **RFC8200-HL-1** (description).

### Requirement

RFC 8200 §3: the discard for a hop limit of zero is a forwarding rule; a node that is the
destination of a packet should not discard it for its hop limit and should process it
normally.

### Scenario constants

- Application data: 100 octets, to port 5000.
- The sender hop limit on host A is 2: exactly the hop count of the path. The router
  decrements it to 1, and host B receives the packet with hop limit 1.

### Procedure

1. Build the mockup network.
2. Set the sender hop limit on host A to 2.
3. Let host A send the datagram.
4. Observe link 1, link 2, host B's UDP, and the messages that return to host A.

### Expected observations

1. On link 1, the packet appears with hop limit 2. This confirms the stimulus.
2. On link 2, the packet appears with hop limit 1 (RFC8200-HL-1).
3. Host B's UDP receives the datagram, 108 octets (RFC8200-HL-3): the destination
   delivered a packet that arrived with the smallest hop limit a forwarded packet can have.
4. Host A receives no Time Exceeded message: host B did not treat the packet as expired.

The check passes if observations 1 to 3 occur in this order and observation 4 records no
message within the time limit.

### Notes

- A packet that arrives with hop limit 0 is the exact case of the should. It needs a
  sender that emits hop limit 0, which no ordinary sender does; that variant is a crafted
  input for a later pass. Hop limit 1 is the boundary this pass can reach.


## Hop limit 0 at the destination

Checks: **RFC8200-HL-3** (should), the exact case.

### Requirement

RFC 8200 §3: a node that is the destination of a packet should not discard a packet with
hop limit equal to zero; it should process the packet normally. The discard is a forwarding
rule.

### Scenario constants

- Mockup with a relay. Application data: 100 octets, to port 5000, sender hop limit 64.
- The relay rewrites the hop limit of the packet to 0 on link 2, after the router. Host B
  receives a packet with hop limit 0 addressed to itself.

### Procedure

1. Build the mockup with the relay.
2. Tell the relay to rewrite the hop limit to 0.
3. Let host A send the datagram.
4. Observe host B's interface, host B's UDP, and the messages that return to host A.

### Expected observations

1. At host B's interface, the packet arrives with hop limit 0 and destination port 5000.
   This confirms the stimulus.
2. Host B's UDP receives the datagram, 108 octets (RFC8200-HL-3): the destination
   processed a packet with hop limit 0.
3. Host A receives no Time Exceeded message.

The check passes if observations 1 and 2 occur in this order and observation 3 records no
message within the time limit.

### Notes

- A destination that discards here misreads the forwarding rule; the standard says
  "should", so a discard is a declined should rather than a violation, and the ledger says
  so.
