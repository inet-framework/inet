# IEEE 802.15.4 — bounded frame codec

> **Kind:** design · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [applicability.md](applicability.md), [integration.md](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/integration.md)

Steps 2/3 introduce `Ieee802154Frame` and its registered serializer independently of the old
`Ieee802154MacHeader`. One chunk represents the complete M1 MPDU. The native addresses retain
all 16/64 bits; field presence follows IEEE Std 802.15.4-2024, 7.2.2.6 (physical PDF p. 80,
`ieee802154-2024@367030:368561`). Legacy versions 0/1 use the DSN and optional destination
PAN/address then optional source PAN/address. Compression requires both addresses and equal
PAN IDs. Version 2 is recognized as unsupported without applying legacy offsets. Reserved
address mode/version and illegal legacy suppression/IE combinations are malformed. Reserved
FCF bit 7 is ignored on reception and retained for forensic reserialization (4.6). For secured
legacy data the common addressing is decoded while the protected remainder stays opaque; the
step-6 security-policy early return must precede any plaintext interpretation. Generating
secured frames, beacon and command formats remains unsupported until their respective packages.

The codec owns little-endian integers, lengths and the selected two-octet FCS. It never stores
an upper protocol discriminator in a PAN field. CRC follows 7.2.11 (physical PDF pp. 84–85,
`ieee802154-2024@380294:384027`), initial remainder zero, reflected polynomial 0x8408,
no final XOR. Its independent ACK example is bytes `02 00 6a` with FCS `e4 79`; these are
the source's time-ordered bit strings converted into LSB-first octets. FCS covers MHR and
payload once. A decoded FCS is preserved even when invalid; absent FCS is a distinct input
mode and is never fabricated. Oversized O-QPSK PSDUs are rejected. Decode results distinguish
truncation, malformed and unsupported inputs without assertions on untrusted bytes.

Pre-write contract: new generated frame declaration, codec and registered serializer live under
`linklayer/ieee802154`, outside the sealed packet subsystem. The decoder bounds-checks before
each read, retains raw bytes for unsupported/malformed input, and exposes parse status separately
from FCS validity. Transmit callers finalize length before inserting the chunk. The production
dissector recognizes the new frame, reports fields and keeps unknown payload opaque; a typed
legacy chunk retains the existing legacy dissection path. Printer fields include full identity,
version, DSN and FCS status. Existing default traffic remains on its old representation.

Proposed direct unit cases: `Ieee802154FrameWire_1.test` and `Ieee802154Fcs_1.test` cover
independent address/PAN byte vectors, all selected presence combinations, high EUI-64 bits,
254/255/0/1, immediate ACK, bad/truncated/reserved/unsupported input, CRC and invalid/absent FCS.
The registered serializer is exercised through `Packet` typed-to-bytes and bytes-to-typed paths.
Capture production tooling and external decoder evidence remain a separate step-3 gate.
