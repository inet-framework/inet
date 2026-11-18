# DSDV source scope

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [catalog](../../standard/perkins1994/catalog.md)

The governing source is Perkins/Bhagwat, SIGCOMM 1994, §§3–4. The initial
profile covers IPv4 host destinations and hop counts on one bidirectional ad hoc
interface. Layer-2 operation, default routes, external networks, IPv6, asymmetric
links, security and interoperable external encodings are outside its claim.

The conflicting damping paragraph is resolved as recorded in the source catalog.
64-bit checked sequences, retained withdrawal floors, separate destination and
neighbor clocks, the smoothing law and the UDP encoding are engineering choices.

## Version 1 wire profile (engineering choices)

Use IPv4 UDP limited broadcast with TTL 1. All multibyte integers use network
byte order. The fixed header has version (8 bits, value 1), kind (8 bits,
1 for a full fragment or 2 for an incremental), entry count (16 bits), sender
IPv4 address (32 bits), and sender self sequence (64 bits, even). Its length
is 16 bytes. Each 16-byte entry has destination IPv4 address (32 bits),
destination sequence (64 bits), and hop metric (32 bits). The count implies
the exact payload length; trailing bytes and truncation are invalid.

Infinity is `0xffffffff` with an odd sequence. Finite metrics use even
sequences and may not exceed `0x7fffffff`; a receiver rejects that maximum
before adding its hop. Zero metric belongs only to the sender, whose entry,
when present, must agree with the header's self sequence. Sender discovery
does not imply refreshing destinations omitted from a packet.

Reject duplicate destinations, unknown kind or version, unspecified, 0/8,
loopback, multicast, class-E and limited broadcast identities. Subnet network
and directed-broadcast addresses are excluded by the fixture's static address
assignment contract; the wire carries no prefix with which to infer them.
Validate the entire datagram before changing clocks or route state. A model
receiver also checks ingress interface, UDP source port and sender/source-IP
agreement. These checks provide input consistency, not authentication.
