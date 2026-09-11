# ARP — English check procedures: the packet queue

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc826/catalog.md](../../../standard/rfc826/catalog.md), [rfc1122/catalog.md](../../../standard/rfc1122/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## The waiting datagram

Checks: **RFC1122-AQUEUE-1** (should). Covers **RFC826-REQ-3** (overridden).

### Requirement

RFC 1122 §2.3.2.2: the link layer should save, rather than discard, at least one — the
latest — packet of each set of packets destined to the same unresolved IP address, and
transmit the saved packet when the address has been resolved. RFC 826 says the opposite: the
module "probably informs the caller that it is throwing the packet away (on the assumption
the packet will be retransmitted by a higher network layer)". RFC 1122 governs, and its
DISCUSSION gives the reason: without the queue, the first packet of every exchange is lost.

### Size and value arithmetic

The two documents predict two different outcomes for one datagram, so one observation
separates them:

| Document | The first datagram | The second datagram |
| --- | --- | --- |
| RFC 826 | never arrives | arrives |
| RFC 1122 | arrives, after the reply | arrives |

A host that sends one datagram therefore tells the two apart by itself. The check sends one,
so that nothing later can be mistaken for the first.

### Scenario constants

- The mockup is the pair.
- Host A sends exactly one datagram of 100 octets to port 5000 of host B, at 0.1 s. Nothing
  sends it again: the sending program transmits once and stops.
- A program on host B listens on port 5000, so that the datagram has somewhere to arrive.
- Host A holds no table entry for host B when it sends.

### Procedure

1. Build the mockup with two hosts on one Ethernet link.
2. Let host A send its one datagram.
3. Watch link 1 for the request and the reply, and watch host B for the datagram.

### Expected observations

1. The request from host A near 0.1 s: the datagram could not go out, and the resolution
   started.
2. The reply from host B.
3. The datagram reaches the program on host B, after the reply and not before it.

### Notes

- Observation 3 is the check, and the order matters as much as the arrival. A datagram that
  arrived before the reply would mean that the host did not need the resolution at all, and
  the scenario would be broken.
- The requirement asks for one packet, the latest one, per unresolved address. A host that
  keeps more than one conforms: the requirement is a floor. The check sends one datagram, so
  it cannot tell one saved packet from many, and it does not need to.
- The datagram must not be sent again by anything. A program that retransmits would hide
  the difference between the two documents, because the second copy would arrive whether the
  first was saved or thrown away. This is why the check names the sending program's
  behaviour among its constants.
