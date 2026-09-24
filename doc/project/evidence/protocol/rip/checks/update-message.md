# RIP — English check procedures: the update message

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Update transport and addressing (RIP version 2)

Checks: **RFC2453-MSG-1**, **MSG-2**, **MSG-3**, **MCAST-1**, **OUT-2** (all description).

### Requirement

RFC 2453 §3.6: a router sends and receives on UDP port 520; everything for the RIP process of
another router goes to port 520; every update leaves port 520, and an unsolicited update has
520 as both ports. §4.5: the periodic update goes to the IP multicast address 224.0.0.9.
§3.10: a regular update is prepared once for each directly-connected network and multicast
on it.

### Scenario constants

- The pair. The marker of R1 is netA.
- Both routers come up in the first 30 seconds. A router that comes up asks for the table of
  its neighbors, and the answer to that request is a response too, sent to a unicast address.
  Observation therefore starts at 40 seconds, when the requests of the start are over.
- Observation lasts 70 seconds, from 40 to 110 seconds, which holds at least one periodic
  update of R1 on each of its two networks.

### Procedure

1. Build the pair, and let both routers start.
2. From 40 seconds on, observe the updates that R1 sends on L1 and on netA.

### Expected observations

1. On L1, from R1, a periodic update: a response to the multicast group that holds netA.
   This confirms the stimulus.
2. That update has UDP source port 520 and UDP destination port 520 (RFC2453-MSG-1, MSG-2,
   MSG-3).
3. That update has IP destination address 224.0.0.9 (RFC2453-MCAST-1).
4. On netA, from R1, a periodic update too, with the same ports and the same destination
   address (RFC2453-OUT-2: one response for each directly-connected network).
5. No RIP message that R1 sends on L1 in the window has a UDP source port other than 520.

### Notes

- Observation 1 finds the update by its content and its time, not by its address or its
  ports, so that observations 2 and 3 can fail. A search that selected the message by port
  520 would pass over a message on the wrong port and time out instead of reporting it. The
  answer to a request also holds netA; the start at 40 seconds keeps it out.
- Host A on netA runs no RIP. The standard sends the update onto every network that supports
  broadcasting, and does not ask whether a router listens there; an administrator may switch
  it off, which the mockup does not do.

## Update message fields (RIP version 2)

Checks: **RFC2453-MSG-9**, **MSG-10**, **MSG-12**, **GEN-2**, **MET-3**, **MET-4**,
**MASK-1** (all description), **NH-2** (must).

### Requirement

RFC 2453 §3.6 and §3.10.2: a response carries command 2 and zero in the two octets that must
be zero. §4: a message that carries information in the version 2 fields has version 2, and
an IPv4 entry has address family identifier 2. §3.5 and §3.6: the metric of a
directly-connected network is its cost, and a metric is 1 to 15, or 16 for unreachable. §4.3:
the subnet mask field holds the mask of the entry. §4.4: a next hop that an entry names is
directly reachable on the subnet where the entry is advertised, and 0.0.0.0 names the sender.

### Scenario constants

- The pair. The marker of R1 is netA, a /24 network.
- Observation starts 60 seconds after the start, when each router has heard the other, and
  lasts 40 seconds.

### Procedure

1. Build the pair, and let both routers start.
2. From 60 seconds on, observe the next periodic update that R1 sends on L1.

### Expected observations

1. On L1, from R1, a periodic update. This confirms the stimulus.
2. Its header has command 2, version 2, and zero in the two octets after the version
   (RFC2453-MSG-9, MSG-12, GEN-2).
3. Every entry has address family identifier 2 and a metric from 1 to 16 (RFC2453-MSG-10,
   MET-4).
4. The entry of netA has metric 1 and subnet mask 255.255.255.0 (RFC2453-MET-3, MASK-1).
5. The next hop of every entry is 0.0.0.0 or an address in 10.0.12.0/24, the subnet of L1
   (RFC2453-NH-2).

### Notes

- Observation 2 checks version 2 through RFC2453-MSG-12: the entries carry a subnet mask, a
  field of version 2, so the version must be 2.
- The octets that must be zero are read from the header. Whether the bytes on the wire hold
  those zeros in the right place is the bit layout of RFC2453-MSG-6, a serializer test.

## Update transport and addressing (RIPng)

Checks: **RFC2080-MSG-1**, **MSG-2**, **MSG-3**, **OUT-3** (description), **GEN-1**,
**RESP-5** (must).

### Requirement

RFC 2080 §2.1: a router sends and receives on UDP port 521, everything for the RIPng process
of another router goes to port 521, and an unsolicited update has 521 as both ports. §2.5: a
regular update is multicast to FF02::9 on every directly-connected network. §2.5.2: the IPv6
source address of an update is a link-local address of the interface it leaves. §2.4.2:
periodic advertisements carry hop count 255.

### Scenario constants

- The pair, on IPv6. The marker of R1 is netA.
- Observation lasts 70 seconds, from 40 to 110 seconds, after the requests of the start, as
  in the IPv4 twin.

### Procedure

1. Build the pair on IPv6, and let both routers start.
2. From 40 seconds on, observe the updates that R1 sends on L1 and on netA.

### Expected observations

1. On L1, from R1, a periodic update: a response to the multicast group that holds netA.
   This confirms the stimulus.
2. That update has UDP source port 521 and UDP destination port 521 (RFC2080-MSG-1, MSG-2,
   MSG-3).
3. That update has IPv6 destination address FF02::9 (RFC2080-OUT-3).
4. That update leaves from the link-local address of the L1 interface of R1 (RFC2080-GEN-1:
   a link-local address of the interface that the message leaves).
5. That update has hop limit 255 (RFC2080-RESP-5).
6. On netA, from R1, a periodic update too, to FF02::9 (RFC2080-OUT-3: one response for
   each directly-connected network).
7. No RIPng message that R1 sends on L1 in the window has a UDP source port other than 521,
   and no update leaves from an address other than the link-local address of the L1
   interface of R1.

### Notes

- Observations 4 and 5 are the two rules that RFC 2080 adds to RFC 2453. The receiver
  depends on both: it takes the source address as the next hop, and it takes hop limit 255 as
  proof that the message did not cross a router.

## Update message fields (RIPng)

Checks: **RFC2080-MSG-8**, **GEN-3**, **MET-3**, **MET-4**, **MSG-10** (description),
**GEN-5** (must not).

### Requirement

RFC 2080 §2.1 and §2.5.2: a response carries command 2, version 1, and zero in the two
octets that must be zero. §2: the metric of a directly-connected network is its cost, and a
metric is 1 to 15, or 16 for unreachable. §2.1: the prefix length is the number of
significant bits of the prefix. §2.5.2: a route to a link-local address is never in an entry.

### Scenario constants

- The pair, on IPv6. The marker of R1 is netA, a /64 prefix.
- Observation starts 60 seconds after the start and lasts 40 seconds.

### Procedure

1. Build the pair on IPv6, and let both routers start.
2. From 60 seconds on, observe the next periodic update that R1 sends on L1.

### Expected observations

1. On L1, from R1, a periodic update. This confirms the stimulus.
2. Its header has command 2 and zero in the two octets after the version (RFC2080-MSG-8,
   GEN-3).
3. Every route entry has a metric from 1 to 16 (RFC2080-MET-4).
4. The entry of netA has metric 1 and prefix length 64 (RFC2080-MET-3, MSG-10).
5. No entry has a prefix in fe80::/10 (RFC2080-GEN-5).
6. Its header has version 1 (RFC2080-GEN-3).

### Notes

- A next hop entry carries 0xFF in its metric field; observation 3 reads route entries only.
- The version has an observation of its own, the last one, so that a wrong version cannot keep
  the other fields of the header and the entries from a verdict.
