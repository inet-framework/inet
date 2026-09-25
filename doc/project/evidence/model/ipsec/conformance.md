# IPsec — model claims and conformance

> **Kind:** report · **Status:** snapshot 2026-09-24 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/ipsec/standards.md), [features.md](../../protocol/ipsec/features.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow, in two parts. Part 1 records what the model
intends: which standards it claims to implement, mapped onto the standards map; the level 1 pass
wrote it, and the level 2 pass adds what its run found for the facts that part 1 named. Part 2,
the conformance matrix, combines the claims with the feature support of the level 2 run.

Read record of part 1 — no build, no run:

- Date: 2026-09-23
- INET: branch `topic/standards-tests-wave0`, commit `e360ca980e`, tree clean
- Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`

The src tree of the level 2 run, `5c4f41c600`, is a later one; the claims of part 1 are the same
in it (`IPsec.ned`, `SecurityPolicy.h`, `SecurityAssociation.h`).

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

### What the level 2 run found for these facts

1. **The ICV is never verified**, on AH and on ESP. No level 2 check reaches it: a failed ICV needs
   a crafted packet. The two passes of `Rfc4302AhAcrossRouter` therefore have no weight, and the
   level 3 checks will be defects, because the TODOs are claims
   ([results.md](results.md#other-findings)).
2. **The `EncryptedChunk` payload** made every ESP field observable to the tests, which read the
   plaintext "with the keys of the SA". AH wraps its payload in the same chunk, although AH does
   not encrypt; the dissector opens it.
3. **No serializer**: the tests read the chunks of a frame (`tests/protocol/ipsec/IpsecChecks.h`).
   The chunk order is the order of the wire, and it showed that the AH ICV is at the end of the
   packet ([gap 3](results.md#gap-3-defect--the-ah-icv-is-at-the-end-of-the-packet)).
4. **The five stated refusals hold**: tunnel mode, key exchange, multicast, anti-replay and DSCP
   selection are absent, and part 2 treats the features that they name as not claimed. The run
   found two more absences that no sentence of the model states: dummy packets and the path MTU
   ([gaps 11 and 12](results.md#gap-11-unimplemented-feature--no-dummy-packets)).

## Part 2 — the conformance matrix

Run record of the verdicts behind the support values:

- Date: 2026-09-29 18:33 +0200
- INET: branch `topic/standards-tests-ipsec-level2-fixes`, commit `3b03623b11`, tree clean
- Trees: src `3194a24929`, tests/protocol `1edf25a348`
- OMNeT++: 6.4.0, debug build from this commit, Ubuntu clang 23.0.0, Ubuntu 26.04.1 LTS
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/ipsec$'`

A feature is `claimed` when the claims of part 1 cover its governing source document. The model
claims RFC 4301, RFC 4302 and RFC 4303, so every feature is claimed by its document, except the
five that a stated refusal of part 1 names: tunnel mode, multicast, anti-replay, and the two
features that need key management, the creation of an SA and the named SPD entries. The choice
of an SA by DSCP and the SA lifetime were refusals too; the repairs of 2026-09-25 implemented both,
and the documentation no longer refuses them. The support comes from
[`coverage.md`](coverage.md#feature-support).

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [IPSEC-F-ARCHITECTURE](../../protocol/ipsec/features.md#ipsec-f-architecture) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-SECURITY-ASSOCIATION](../../protocol/ipsec/features.md#ipsec-f-security-association) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-SPD-PROCESSING](../../protocol/ipsec/features.md#ipsec-f-spd-processing) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-SPD-MANAGEMENT](../../protocol/ipsec/features.md#ipsec-f-spd-management) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-SELECTORS](../../protocol/ipsec/features.md#ipsec-f-selectors) | mandatory | yes | partial | `partial` — no core check fails; RFC4301-SEL-13 (OPAQUE) belongs to a gateway, and SEL-8 needs an extension header, owed |
| [IPSEC-F-NAMED-SPD-ENTRIES](../../protocol/ipsec/features.md#ipsec-f-named-spd-entries) | mandatory | no | untested | `out of claim` — a stated refusal: no key management (`IPsec.ned:37`), whose identities a name matches |
| [IPSEC-F-SA-CREATION](../../protocol/ipsec/features.md#ipsec-f-sa-creation) | mandatory | no | supported | `undocumented` — a stated refusal: "Key exchange protocols are not implemented" (`IPsec.ned:37`); [gap 6](results.md#gap-6-defect--a-protect-entry-without-an-sa-sends-the-packet-in-clear), [gap 9](results.md#gap-9-unimplemented-feature--no-sa-lifetime) |
| [IPSEC-F-SA-LOOKUP](../../protocol/ipsec/features.md#ipsec-f-sa-lookup) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-INBOUND-SELECTOR-CHECK](../../protocol/ipsec/features.md#ipsec-f-inbound-selector-check) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-TRANSPORT-MODE](../../protocol/ipsec/features.md#ipsec-f-transport-mode) | mandatory | yes | partial | `partial` — RFC4301-SA-50: a host also supports tunnel mode, [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode); every rule of transport mode itself passed |
| [IPSEC-F-TUNNEL-MODE](../../protocol/ipsec/features.md#ipsec-f-tunnel-mode) | mandatory | no | not supported | `out of claim` — a stated refusal: "Transport mode only" (`IPsec.ned:40`); [gap 8](results.md#gap-8-unimplemented-feature--no-tunnel-mode) |
| [IPSEC-F-PARALLEL-SAS](../../protocol/ipsec/features.md#ipsec-f-parallel-sas) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-SA-COMBINATION](../../protocol/ipsec/features.md#ipsec-f-sa-combination) | optional | yes | supported | `confirmed` |
| [IPSEC-F-FRAGMENTATION](../../protocol/ipsec/features.md#ipsec-f-fragmentation) | mandatory | yes | partial | `partial` — no core check fails; RFC4302-REAS-3 and RFC4303-REAS-2, a fragment offered to AH or ESP, are level 3 |
| [IPSEC-F-PATH-MTU](../../protocol/ipsec/features.md#ipsec-f-path-mtu) | mandatory | yes | not supported | `defect` — no code in IPsec or IPv6 keeps a path MTU, [gap 12](results.md#gap-12-unimplemented-feature--no-path-mtu); a plan of its own |
| [IPSEC-F-OUTBOUND-DISCARD-REPORT](../../protocol/ipsec/features.md#ipsec-f-outbound-discard-report) | optional | yes | untested | `unverified` — no check: the model has no ICMP report of a discard |
| [IPSEC-F-AUDIT](../../protocol/ipsec/features.md#ipsec-f-audit) | unstated | yes | untested | `unverified` — level 4: an audit log |
| [IPSEC-F-AH-FORMAT](../../protocol/ipsec/features.md#ipsec-f-ah-format) | optional | yes | supported | `confirmed` |
| [IPSEC-F-AH-INTEGRITY](../../protocol/ipsec/features.md#ipsec-f-ah-integrity) | optional | yes | partial | `partial` — no core check fails, but the passes have no weight: the receiver never checks the ICV; IICV-3 and ISEQ-21 are level 3 |
| [IPSEC-F-ESP-FORMAT](../../protocol/ipsec/features.md#ipsec-f-esp-format) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-ESP-PADDING](../../protocol/ipsec/features.md#ipsec-f-esp-padding) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-ESP-SERVICES](../../protocol/ipsec/features.md#ipsec-f-esp-services) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-ESP-PROCESSING](../../protocol/ipsec/features.md#ipsec-f-esp-processing) | mandatory | yes | partial | `partial` — no core check fails; the ICV that fails (IICV-3, ISEQ-23) and the order of the operations are level 3 and 5 |
| [IPSEC-F-SEQUENCE-NUMBERS](../../protocol/ipsec/features.md#ipsec-f-sequence-numbers) | mandatory | yes | partial | `partial` — no core check fails; the counter near 2^32 is level 4 |
| [IPSEC-F-ANTI-REPLAY](../../protocol/ipsec/features.md#ipsec-f-anti-replay) | mandatory | no | untested | `out of claim` — a stated refusal: "Anti-replay mechanism is not implemented" (`IPsec.ned:42`) |
| [IPSEC-F-EXTENDED-SEQUENCE-NUMBERS](../../protocol/ipsec/features.md#ipsec-f-extended-sequence-numbers) | optional | yes | untested | `unverified` — no check: the counter is 32 bits |
| [IPSEC-F-ESP-TFC-PADDING](../../protocol/ipsec/features.md#ipsec-f-esp-tfc-padding) | optional | yes | supported | `confirmed` |
| [IPSEC-F-ESP-DUMMY-PACKETS](../../protocol/ipsec/features.md#ipsec-f-esp-dummy-packets) | mandatory | yes | supported | `confirmed` |
| [IPSEC-F-MULTICAST](../../protocol/ipsec/features.md#ipsec-f-multicast) | optional | no | untested | `out of claim` — a stated refusal: "Multicast traffic is not supported" (`IPsec.ned:41`) |

### How to read the matrix

- **One mandatory feature is `defect` by the table, and it is a missing feature**: the path MTU.
  The model claims RFC 4301, which makes it a must, and has no code for it; a repair is new code
  in the IPv6 layer first, and it has a plan of its own.
- **Since the repairs of 2026-09-25 no feature has a failing core statement** except the path
  MTU and tunnel mode. The `partial` features lack checks of a later level, or, for transport
  mode, the tunnel mode that RFC 4301 also asks of a host.
- **Four `out of claim` features are stated refusals of part 1.** Tunnel mode, one of them, has
  a plan of its own; the model says in its own words that it does not do the others.
- **The creation of an SA is `undocumented`**: its core statements pass since the SA lifetime of
  2026-09-25, but the documentation still refuses key exchange, and the lifetime is the part of
  the feature that the model does. A documentation task, not a test task.
- **SA combination, dummy packets and the choice of an SA by DSCP are `confirmed`** since the
  repairs; in the level 2 run they were `declined`, `defect` and `out of claim`.
- **The three `unverified` features are not mandatory**: the ICMP report of a discard and
  extended sequence numbers are optional, the audit log is unstated.
