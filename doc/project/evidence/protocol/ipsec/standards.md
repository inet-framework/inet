# IPsec — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards
around IP security, records which document governs each contested clause, and pins the set
that the next pass tests against. IPsec has one architecture document and two traffic
security protocols that the architecture document governs: the Authentication Header (AH)
and the Encapsulating Security Payload (ESP). A third area, cryptographic algorithms and key
management, is a family of its own that the architecture document deliberately keeps
separate (`rfc4301.txt:293-296`: "the use of cryptographic key management procedures and
protocols"; `rfc4301.txt:305-308`: "IPsec security protocols (AH and ESP, and to a lesser
extent, IKE) are designed to be cryptographic algorithm independent").

The relationships come from the RFC-editor metadata
(`https://www.rfc-editor.org/rfc/rfcNNNN.json`), retrieved 2026-09-23.

## Target level

**Level 1 — Survey** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)).
This pass maps the family, pins the in-scope set that a level 2 pass needs, and records the
claims of the model. It writes no catalog, no feature map and no test.

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 4301 §3.1 to §3.2, §4.4.1, §4.4.2, §5.1, §5.2; RFC 4302 §2, §3.3, §3.4; RFC 4303 §2, §3.3, §3.4 | the normal path: what a Security Policy and a Security Association are, how the SPD and the SAD select one, how AH and ESP each build their header and trailer on egress, and how each is verified and removed on ingress |
| level 3, Edge | nothing new | RFC 4301 §5.1.1 (a packet that must be discarded), RFC 4302 §3.4.3 to §3.4.4 and RFC 4303 §3.4.3 to §3.4.4 (a sequence number or an Integrity Check Value that fails verification) are already inside the base documents above |
| level 4, Dynamics | nothing new | RFC 4302 §3.4.3 and Appendix B, and RFC 4303 §3.4.3, hold the anti-replay window, the one control loop of the family |
| level 5, Complete | RFC 8221, RFC 2405, RFC 2451, RFC 3602, RFC 4106, RFC 4309, RFC 7634, RFC 4543, RFC 6040, RFC 7619 | the cryptographic algorithms themselves, tunnel-mode ECN-field construction (a single field of RFC 4301 §5.1.2), and the IKEv2 authentication method that establishes a Security Association |

What a pass actually reached is not recorded here. It is in
[`model/ipsec/coverage.md`](../../model/ipsec/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Downloaded |
| --- | --- | --- | --- | --- | --- |
| RFC 4301 | Security Architecture for the Internet Protocol | December 2005 | Proposed Standard | `base` | [`standards/RFC/rfc4301.txt`](../../../../../../standards/RFC/rfc4301.txt), 2026-09-23 |
| RFC 4302 | IP Authentication Header | December 2005 | Proposed Standard | `companion`, AH | [`standards/RFC/rfc4302.txt`](../../../../../../standards/RFC/rfc4302.txt), 2026-09-23 |
| RFC 4303 | IP Encapsulating Security Payload (ESP) | December 2005 | Proposed Standard | `companion`, ESP | [`standards/RFC/rfc4303.txt`](../../../../../../standards/RFC/rfc4303.txt), 2026-09-23 |
| RFC 2401 | Security Architecture for the Internet Protocol | November 1998 | Proposed Standard | `obsoleted by` RFC 4301 | no |
| RFC 6040 | Tunnelling of Explicit Congestion Notification | November 2010 | Proposed Standard | `updates` RFC 4301 (and RFC 3168, RFC 4774) | no |
| RFC 7619 | The NULL Authentication Method in IKEv2 | August 2015 | Proposed Standard | `updates` RFC 4301 | no |
| RFC 8221 | Cryptographic Algorithm Implementation Requirements and Usage Guidance for ESP and AH | October 2017 | Proposed Standard | `companion`; obsoletes RFC 7321 | no |
| RFC 2405 | The ESP DES-CBC Cipher Algorithm With Explicit IV | November 1998 | Proposed Standard | `companion`, cipher | no |
| RFC 2451 | The ESP CBC-Mode Cipher Algorithms | November 1998 | Proposed Standard | `companion`, cipher | no |
| RFC 3602 | The AES-CBC Cipher Algorithm and Its Use with IPsec | September 2003 | Proposed Standard | `companion`, cipher | no |
| RFC 4106 | The Use of Galois/Counter Mode (GCM) in IPsec ESP | June 2005 | Proposed Standard | `companion`, cipher | no |
| RFC 4309 | Using AES CCM Mode with IPsec ESP | December 2005 | Proposed Standard | `companion`, cipher | no |
| RFC 7634 | ChaCha20, Poly1305, and Their Use in IKE and IPsec | August 2015 | Proposed Standard | `companion`, cipher | no |
| RFC 4543 | The Use of GMAC in IPsec ESP and AH | May 2006 | Proposed Standard | `companion`, cipher | no |

Source of the texts:

- `rfc4301.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc4301.txt) —
  <https://www.rfc-editor.org/rfc/rfc4301.txt>, downloaded 2026-09-23.
- `rfc4302.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc4302.txt) —
  <https://www.rfc-editor.org/rfc/rfc4302.txt>, downloaded 2026-09-23.
- `rfc4303.txt` in [`standards/RFC/`](../../../../../../standards/RFC/rfc4303.txt) —
  <https://www.rfc-editor.org/rfc/rfc4303.txt>, downloaded 2026-09-23.

RFC 4301 obsoletes RFC 2401 as a whole (`December 2005`, superseding the `November 1998`
edition); RFC 4302 obsoletes RFC 2402, and RFC 4303 obsoletes RFC 2406, in the same
December 2005 round. No predecessor of the three current documents is downloaded.

The three documents use the keywords of RFC 2119 throughout: RFC 4301 has "MUST" on 93
lines, "SHOULD" on 42 and "MAY" on 36; RFC 4302 has 37, 18 and 8; RFC 4303 has 59, 29 and
16 (`grep -c '\bMUST\b'` and so on). Unlike RIP or ARP, this family is recent and keyword-
dense; few statements are expected to carry the `description` strength.

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| The ECN field of the outer header in tunnel mode | RFC 4301 §5.1.2, `rfc4301.txt:3063,3169`: the tunnel-mode header-construction table states the outer ECN field is "copied from inner hdr" | RFC 6040: replaces this rule with a corrected copy-and-propagate algorithm for the outer and the inner ECN field together | RFC 6040, for a tunnel-mode Security Association | no; level 5. It is a single-field correction inside tunnel-mode header construction; it changes neither the AH or ESP integrity and confidentiality processing of §3 of RFC 4302 and RFC 4303, nor the SPD/SAD model of RFC 4301 §4.4, which anchor the core level |
| The authentication method that establishes a Security Association | RFC 4301 §2.1, `rfc4301.txt:293-296`: Security Associations are created "through the use of cryptographic key management procedures and protocols", a component the architecture keeps separate from AH and ESP | RFC 7619: defines one authentication method of IKEv2, the key-management protocol RFC 4301 assumes but does not itself specify | RFC 7619, for a peer that uses IKEv2 with NULL authentication | no; level 5. AH and ESP processing does not depend on which method established the Security Association; key management is out of scope of the base document by its own design |

## In-scope set

The set that a level 2 pass tests against:

| Document | Version | Catalog file |
| --- | --- | --- |
| RFC 4301 | December 2005, Proposed Standard; §3.1 to §3.2, §4.4.1, §4.4.2, §5.1, §5.2 | none yet; level 2 writes `standard/rfc4301/catalog.md` |
| RFC 4302 | December 2005, Proposed Standard; §2, §3.3, §3.4 | none yet; level 2 writes `standard/rfc4302/catalog.md` |
| RFC 4303 | December 2005, Proposed Standard; §2, §3.3, §3.4 | none yet; level 2 writes `standard/rfc4303/catalog.md` |

Out of scope, with the reason:

| Document | Reason |
| --- | --- |
| RFC 2401 | Obsoleted by RFC 4301. |
| RFC 6040 | Level 5. A single-field correction to tunnel-mode header construction; see the override table. |
| RFC 7619 | Level 5. An IKEv2 authentication method; key management is outside the base document's own scope. |
| RFC 8221 | Level 5. Algorithm mandatory-to-implement policy; RFC 4301 states AH and ESP are algorithm-independent, so no cipher policy belongs to the architecture, AH, or ESP processing rules. |
| RFC 2405, RFC 2451, RFC 3602, RFC 4106, RFC 4309, RFC 7634, RFC 4543 | Level 5. Cipher and integrity-transform specifications; each defines one algorithm's use inside ESP or AH, not the architecture, the header format, or the processing rules those algorithms plug into. |
