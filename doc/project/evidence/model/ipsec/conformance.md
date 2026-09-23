# IPsec — model claims

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ipsec/standards.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, part 1 only. A level 1 pass records what the
model intends: which standards it claims to implement, mapped onto the standards map. Part 2,
the conformance matrix, needs the feature map and the verdicts of a level 2 pass.

Read record — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

## Part 1 — the claims

Every place in the model that names a standard of the IPsec family, found with
`grep -rn -E 'RFC ?[0-9]{3,4}|draft-' src/inet/networklayer/ipsec --include=*.ned --include=*.cc --include=*.h --include=*.msg`
(the generated `*_m.cc`/`*_m.h` files excluded; nothing under `src/inet/networklayer/ipv4/ipsec/`
exists to search, that directory holds no files):

| Claim | Where | What it says |
| --- | --- | --- |
| RFC 4301 | [`IPsec.ned:23-25`](../../../../../src/inet/networklayer/ipsec/IPsec.ned) | "Implements basic IPsec (RFC 4301) functionality. It supports Authentication Header (AH) and Encapsulating Security Payload (ESP), in transport mode, for IPv4 and IPv6 unicast traffic (UDP/TCP/ICMP/ICMPv6)." The documentation comment of the module, which the module reference publishes. |
| RFC 4302 | [`IPsecAuthenticationHeader.msg:28`](../../../../../src/inet/networklayer/ipsec/IPsecAuthenticationHeader.msg) | "IPsec AH header (RFC 4302)." |
| RFC 4303 | [`IPsecEncapsulatingSecurityPayload.msg:29,40`](../../../../../src/inet/networklayer/ipsec/IPsecEncapsulatingSecurityPayload.msg) | "IPsec ESP header (RFC 4303)." / "IPsec ESP trailer (RFC 4303)." |
| RFC 4303 | [`IPsec.cc:492`](../../../../../src/inet/networklayer/ipsec/IPsec.cc) | "Compute padding according to RFC 4303 Sections 2.4 and 2.5", above the ESP padding-length arithmetic. |
| RFC 4301 | [`IPsec.cc:506`](../../../../../src/inet/networklayer/ipsec/IPsec.cc) | "ASSERT(false); // forbidden by RFC 4301", on the case where an ESP Security Association carries neither confidentiality nor integrity. |
| RFC 4301 §4.4.1.2 | [`SecurityPolicy.h:32-34`](../../../../../src/inet/networklayer/ipsec/SecurityPolicy.h) | A stated non-claim: "several fields listed in RFC 4301 section 4.4.1.2 do not occur here due to simplifications in the model. For example, PFP flags are not needed because SAs are configured statically." |
| RFC 4301 §4.4.2.1 | [`SecurityAssociation.h:30-35`](../../../../../src/inet/networklayer/ipsec/SecurityAssociation.h) | A stated non-claim: "several fields listed in RFC 4301 section 4.4.2.1 do not occur here... the choice of AH/ESP cryptographic algorithm, their parameters, keys, etc. are omitted because the model does not perform actual cryptography. SA lifetime is omitted because all SAs are statically configured." |
| RFC 2405, RFC 2451, RFC 3602, RFC 4106, RFC 4309, RFC 7634 | [`IPsecRule.h:79-116`](../../../../../src/inet/networklayer/ipsec/IPsecRule.h), [`IPsec.cc:623-673`](../../../../../src/inet/networklayer/ipsec/IPsec.cc) | One citation per encryption algorithm of the `EncryptionAlg` enum, each above its name (DES, 3DES, AES-CBC, AES-GCM, AES-CCM, ChaCha20-Poly1305). |
| RFC 8221 | [`IPsecRule.h:83,91,127-134`](../../../../../src/inet/networklayer/ipsec/IPsecRule.h) | The MUST/SHOULD NOT/MAY policy per algorithm, quoted next to each enum value, for example "DES, // MUST NOT RFC 8221 5." and "HMAC_SHA2_256_128, // MUST RFC 8221 Section 6". |
| RFC 4543 | [`IPsec.cc:690,708`](../../../../../src/inet/networklayer/ipsec/IPsec.cc) | "case AuthenticationAlg::AES_256_GMAC: return 128; //RFC 4543 Section 4." (and the same for the 64-bit ICV length). |

The module documentation comment continues past the RFC 4301 claim with a limitations list
([`IPsec.ned:32-46`](../../../../../src/inet/networklayer/ipsec/IPsec.ned)) that states, in
its own words, five things the model does not do: it does not perform actual cryptography
("an explicit non-goal"); it does not implement key exchange protocols, so Security
Associations "are statically configured and remain in effect for the entire duration of the
simulation"; it supports "Transport mode only. Tunnel mode is not supported"; it does not
support multicast traffic; it does not implement the anti-replay mechanism; and it does not
implement DSCP-based SA selection. These five are stated refusals, not silent gaps: each
names the exact behavior it leaves out.

Mapped onto the standards map, [`standards.md`](../../protocol/ipsec/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 4301 | yes, `base` | **yes**, in the published documentation, and at clause level in two source comments | `IPsec.ned:23`, `SecurityPolicy.h:32`, `SecurityAssociation.h:30`. The claim names the current document; RFC 2401 never appears. |
| RFC 4302 | yes, `companion` | **yes**, in the published documentation and in the packet definition | `IPsec.ned:24`, `IPsecAuthenticationHeader.msg:28`. |
| RFC 4303 | yes, `companion` | **yes**, in the published documentation, the packet definition, and a clause-level comment | `IPsec.ned:24`, `IPsecEncapsulatingSecurityPayload.msg:29,40`, `IPsec.cc:492`. |
| RFC 8221 | no, level 5 | **yes**, at clause level, for every algorithm the model enumerates | `IPsecRule.h:83-134`. A claimed document outside the in-scope set; the model states the policy but performs none of the cryptography the policy governs. |
| RFC 2405, RFC 2451, RFC 3602, RFC 4106, RFC 4309, RFC 7634, RFC 4543 | no, level 5 | **yes**, one citation each | Named for the algorithm's byte lengths only (block size, IV length, ICV length); no citation claims the cipher transform itself, and the model performs none. |
| RFC 6040 | no, level 5 | no | Nothing names it. `IPsec.ned:40` declares tunnel mode unsupported in the model's own words, which is the area RFC 6040 amends. |
| RFC 7619 | no, level 5 | no | Nothing names it. `IPsec.ned:37-39` declares key exchange unimplemented in the model's own words, which is the protocol family RFC 7619 amends. |

**The finding of the survey.** The claim is accurate: the module names the current base document, RFC 4301, and the two companion
documents by their current numbers, RFC 4302 and RFC 4303; no obsolete document is claimed
anywhere in the tree. The claim is also unusually well scoped: the module documentation does
not stop at "implements RFC 4301", it lists five specific things the model leaves out, each
in a sentence a reader can check against the standard. Two of those five sentences name
exactly the areas this document's override table places at level 5 for a different reason
(tunnel mode against RFC 6040, key exchange against RFC 7619), so the model's own words and
the register's relations agree without needing to be reconciled.

For the level 2 pass, facts from the code (the guide permits this look to decide the claim
question, and the pass must check each one):

1. **ICV verification on ingress is an open TODO, not a stated refusal.**
   [`IPsec.cc:832`](../../../../../src/inet/networklayer/ipsec/IPsec.cc) reads
   "// TODO check icv, ..." on the AH ingress path, and
   [`IPsec.cc:885`](../../../../../src/inet/networklayer/ipsec/IPsec.cc) reads "// TODO
   calculate icv bytes length from SPI and use espHeader.icvBytes for verify it" on the ESP
   ingress path. Neither comment gives a reason the check is left out. By the guide's table,
   a bare "TODO implement X" is a claim, so a check that a corrupted Integrity Check Value is
   rejected is a **defect** if it fails, not an unimplemented feature — the ICV field itself
   is present, sized per algorithm, and carried on the wire; only the verification step is
   missing.
2. **The `EncryptedChunk` payload has no cipher behind it.** `IPsec.cc` computes only
   algorithm-dependent lengths (`getIntegrityCheckValueBitLength`,
   `getInitializationVectorBitLength`, `IPsec.cc:588-716`) and never calls a cryptographic
   transform; the protected payload becomes an `EncryptedChunk`
   (`src/inet/common/packet/chunk/EncryptedChunk.h`), a generic opaque-data marker used
   elsewhere in INET too. This matches the stated non-goal of `IPsec.ned:33-36` exactly, so a
   check of confidentiality or integrity as a cryptographic property is out of scope by the
   third principle, and only the framing, sizing and policy outcome (PROTECT/BYPASS/DROP,
   ACCEPT/DROP) are checkable claims.
3. **No serializer exists for AH or ESP** (`grep Register_Serializer` over
   `src/inet/networklayer/ipsec/` returns nothing); two dissectors do
   (`Protocol::ipsecAh`, `Protocol::ipsecEsp`, `IpSecProtocolDissector.cc:18-19`). A check
   that needs a byte-accurate field falls back to the chunk class name, per `AUTHORING.md`;
   a check that needs the wire bytes themselves cannot be built yet, which is an untestable-
   claim candidate for the AH/ESP header fields RFC 4302 §2 and RFC 4303 §2 define.
