# IPv6 checks — packet size

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV6-F-PACKET-SIZE. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## Link MTU packet

Checks: **RFC8200-MTU-2** (must).

### Requirement

RFC 8200 §5: from each link a node is attached to, the node must be able to accept packets
as large as that link's MTU.

### Scenario constants

- Both links keep the Ethernet MTU of 1500 octets.
- Application data: 1452 octets, to port 5000: a packet of exactly 1500 octets, the link
  MTU. Nothing fragments it.

### Procedure

1. Build the mockup network.
2. Let host A send the datagram.
3. Observe link 1, link 2 and host B's UDP.

### Expected observations

1. On link 1, the packet appears whole: next header 17, payload length 1460. This confirms
   the stimulus.
2. On link 2, the packet appears whole with the same payload length: the router accepted
   and forwarded a packet of the link MTU.
3. Host B's UDP receives the datagram of 1460 octets (RFC8200-MTU-2): host B accepted a
   packet of the link MTU.

The check passes if all three observations occur in this order within the time limit.

### Notes

- Observation 2 is the router's half of the same rule; RFC 8200 states it for every node.

