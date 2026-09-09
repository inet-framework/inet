# IPv6 checks — packet too big

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV6-F-PACKET-TOO-BIG. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## Packet too big

Checks: **RFC8200-FRAG-1** (description), **RFC4443-PTB-1** (must), **RFC4443-ERR-2**
(description).

### Requirement

RFC 8200 §4.5 and §5: fragmentation is performed by source nodes only; a router does not
fragment a packet that does not fit the next link. RFC 4443 §3.2: a router must send Packet
Too Big, type 2, to the source of a packet that it cannot forward because the packet is
larger than the MTU of the outgoing link.

### Scenario constants

- Link 2 has an MTU of 1280 octets. Link 1 keeps 1500 octets.
- Application data: 1452 octets, to port 5000: a 1500-octet packet. It crosses link 1
  whole; the router cannot forward it over link 2.

### Procedure

1. Build the mockup with MTU 1280 on link 2.
2. Let host A send the datagram.
3. Observe link 1, the router's own origination of messages, link 2, and the messages that
   return to host A.

### Expected observations

1. On link 1, the packet appears whole: next header 17, payload length 1460. This confirms
   the stimulus.
2. The router originates a Packet Too Big message, type 2: recorded at the router at the
   moment of the discard (RFC4443-PTB-1).
3. Host A receives that message, type 2, addressed to host A (RFC4443-ERR-2).
4. From then on, no part of the packet appears on link 2: neither a fragment (next header
   44) nor the packet itself (next header 17), and host B receives nothing of it
   (RFC8200-FRAG-1).

The check passes if observations 1 to 3 occur in this order and observation 4 records no
traffic of the packet on link 2 within the time limit.

### Notes

- The content of the message, the MTU field, is a check of its own
  ([packet too big, the MTU field](#packet-too-big-the-mtu-field)), so that a wrong field
  value does not hide the result of the two rules here.
- The 1280-octet MTU on link 2 is the smallest value RFC 8200 allows on an IPv6 link
  (RFC8200-MTU-1).


## Packet too big, the MTU field

Checks: **RFC4443-PTB-2** (description; the field definition of a mandatory message).

### Requirement

RFC 4443 §3.2: the MTU field of a Packet Too Big message is the maximum transmission unit
of the next-hop link, and the code is 0. Path MTU discovery (RFC 8201) reads this field.

### Scenario constants

- The same as [packet too big](#packet-too-big): MTU 1280 on link 2, a 1500-octet packet.

### Procedure

1. Build the mockup with MTU 1280 on link 2.
2. Let host A send the datagram.
3. Observe the Packet Too Big message that returns to host A.

### Expected observations

1. On link 1, the packet appears whole with payload length 1460. This confirms the
   stimulus.
2. Host A receives a Packet Too Big message, type 2, code 0, whose MTU field is 1280
   (RFC4443-PTB-2).

The check passes if both observations occur in this order within the time limit.

### Notes

- A message with an MTU field of zero, or of any value other than the next-hop MTU, fails
  this check while [packet too big](#packet-too-big) passes: the router reported, but the
  report carries nothing a source could act on.

