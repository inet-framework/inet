# IPv6 — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/ipv6/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/ipv6/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan: 2026-09-09, worktree at commit `95f9805952`. A grep for `RFC 8200`,
  `RFC8200`, `RFC 2460`, `RFC2460`, `RFC 4443`, `RFC4443`, `RFC 2463`, `RFC 8504`,
  `RFC 5722`, `RFC 6946` and `RFC 8021` over `src/inet/networklayer/ipv6/`,
  `src/inet/networklayer/icmpv6/` and `src/inet/networklayer/contract/ipv6/`, plus a read of
  the module documentation comments. The pass 2 additions (RFC 8504 and the four documents
  it restates) return no line.
  The claim scan did not run again on 2026-09-10. Every source file that part 1 cites is
  identical to the file at the commit above, so the claims still hold.
- Support values: [`coverage.md`](coverage.md#feature-support), from the run of 2026-09-10
  on `master`, commit `0868c36c88`. Every verdict of that run repeats the verdict of the
  earlier pass.

This is the one document of the workflow whose first part reads the model documentation on
purpose. The claims must not travel back into the catalogs, the feature map, or the check
descriptions.

## Part 1 — what the model claims

### The claims of the active modules

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [Ipv6.ned:13](../../../../../src/inet/networklayer/ipv6/Ipv6.ned#L13) | "Implements the IPv6 protocol." | protocol claim, no document |
| [Icmpv6.ned:13-16](../../../../../src/inet/networklayer/icmpv6/Icmpv6.ned#L13-L16) | "Implements ICMPv6 (RFC 4443): it generates and processes ICMPv6 error messages (destination unreachable, packet too big, time exceeded, parameter problem) on behalf of ~Ipv6, and answers Echo Requests (ping) with Echo Replies." | **explicit claim on RFC 4443** |

### Document references elsewhere in the IPv6 tree

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [Ipv6Header.msg:24](../../../../../src/inet/networklayer/ipv6/Ipv6Header.msg#L24) | "Ipv6 datagram. RFC 2460 Section 3." | citation of the obsoleted base document, on the header definition |
| [Ipv6ExtensionHeaders.msg:78](../../../../../src/inet/networklayer/ipv6/Ipv6ExtensionHeaders.msg#L78) | "Fragment Header / RFC 2460 Section 4.5" | citation of the obsoleted base document, on the fragment header (and §4.3, §4.4, §4.6 for the other extension headers) |
| [Ipv6ExtensionHeaders.cc:23](../../../../../src/inet/networklayer/ipv6/Ipv6ExtensionHeaders.cc#L23), [Ipv6.cc:974](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L974) | "the order of extension headers according to RFC 2460 4.1" | citation of the obsoleted base document |
| [Ipv6.cc:1266](../../../../../src/inet/networklayer/ipv6/Ipv6.cc#L1266) | "RFC 8200 Section 4.2: the two highest-order bits of the option type encode ..." | citation of the current base document, on option processing |
| [Icmpv6.h:34-43](../../../../../src/inet/networklayer/icmpv6/Icmpv6.h#L34-L43) | "RFC 2463, Section 3: ICMPv6 Error Messages" | citation of the obsoleted ICMPv6 document, in a header comment |
| [Ipv6FragBuf.cc:76-95](../../../../../src/inet/networklayer/ipv6/Ipv6FragBuf.cc#L76-L95) | "RFC 2460 4.5: If the length of a fragment ..." | quotes of the obsoleted base document in the reassembly code |

### How this pass reads the claim

- **ICMPv6 claims RFC 4443 by name.** The matrix reads that claim as explicit for every
  feature whose governing statement is an RFC 4443 statement.
- **IPv6 claims the protocol and cites RFC 2460 for its formats.** RFC 8200 replaced
  RFC 2460 in July 2017 and folded nine updates into it. "Implements the IPv6 protocol"
  is read, as in the IPv4 pass, as an implicit claim on the Internet Standard, which is
  RFC 8200; the RFC 2460 citations are recorded as the version the authors had in view.
  At level 3 the reading matters: RFC 8200 §4.5 added the rules on overlapping fragments
  (RFC 5722) and on atomic fragments (RFC 6946), the reassembly code quotes RFC 2460 for
  exactly that section, and exactly those two rules fail.
- **RFC 8504 is not claimed**, and no line names the documents it restates. Its
  behavior-level statements govern the reassembly and the next-header rules in the matrix;
  the reading keeps the claim on the RFC 8200 base statement and records the RFC 8504
  restatement as unclaimed. The verdicts below are the same either way, because every
  RFC 8504 rule that fails also fails as an RFC 8200 rule.

## Part 2 — conformance matrix

The verdict combines the claim on the governing source document, the support value of the
ledger, and the level of the feature, by the table of step 8.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| IPV6-F-HEADER | mandatory | yes (implicit, RFC 8200; RFC 2460 cited) | partial | **partial** — RFC8200-HDR-2: the payload length of a fragment packet is the original's |
| IPV6-F-DELIVERY | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-HOP-LIMIT | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-SOURCE-FRAGMENTATION | optional | yes (RFC 2460 §4.5 cited on the fragment header) | partial | **partial** — RFC8200-FRAG-4, FRAG-5: the payload length of every fragment is the original's |
| IPV6-F-REASSEMBLY | mandatory | yes (RFC 2460 §4.5 cited in the reassembly code) | partial | **partial** — RFC8504-NR-3, RFC8200-REASM-5: overlapping fragments are merged and the result stops the node; the atomic fragment (a should) stops it too |
| IPV6-F-PACKET-TOO-BIG | mandatory | yes (explicit, RFC 4443) | partial | **partial** — RFC4443-PTB-2: the MTU field is 0 |
| IPV6-F-PACKET-SIZE | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-ERROR-REPORT | mandatory | yes (explicit, RFC 4443) | supported | **confirmed** |
| IPV6-F-UPPER-LAYER-CHECKSUM | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-INPUT-VALIDATION | mandatory | yes (implicit, RFC 8200; explicit, RFC 4443) | partial | **partial** — RFC4443-MPR-2: an unknown informational type stops the node |
| IPV6-F-ERROR-SUPPRESSION | mandatory | yes (explicit, RFC 4443) | partial | **partial** — RFC4443-MPR-7, MPR-8: a report is sent about a link-layer multicast or broadcast |
| IPV6-F-ERROR-DELIVERY | mandatory | yes (explicit, RFC 4443) | untested | **unverified** — every core statement is internal; and RFC4443-MPR-3, a supporting statement, stops the node |

Five features `confirmed`, six `partial`, one `unverified`. No `defect` in the strict sense
of the table: every failing feature also has a core check that passed. No `undocumented`,
no `declined` at the feature level.

## Findings

The two findings of pass 1 stand and come first; the level 3 findings follow.

### 1. Packet Too Big says MTU 0, against an explicit claim

The ICMPv6 module names RFC 4443, sends Packet Too Big to the right node with the right
type, and puts 0 in the MTU field ("TODO implement MTU support"). RFC 4443 §3.2 defines the
field as the MTU of the next-hop link, and path MTU discovery (RFC 8201) is nothing but a
reader of that field. A source in this model can never learn a path MTU below its own link
MTU. This is the headline of the pass: a mandatory message, explicitly claimed, that
carries no information.

### 2. Every fragment carries the payload length of the original packet

RFC 8200 §4.5 requires the payload length of each fragment packet to be "changed to contain
the length of this fragment packet only". The model copies the base header into every
fragment and never rewrites the field: a 1280-octet fragment packet and a 276-octet one both
say 1460. The model's own reassembly measures the fragment instead of reading the field, so
the packets reassemble inside a simulation; a recorded trace, an external stack, or any
receiver that follows §4.5 reads the wrong length. Two features read `partial` from this one
field, because the field is both a header rule (HDR-2) and a fragmentation rule (FRAG-4,
FRAG-5).

### 3. Four crafted inputs stop the simulation instead of being discarded

RFC 8200 and RFC 4443 ask a node to discard, silently or with a report, an overlapping
fragment set, an atomic fragment (to process it as a whole), an ICMPv6 informational message
of unknown type, and a report about a packet whose upper-layer protocol it does not
implement. The model answers each with an assertion or a runtime error: the fragment buffer
erases a stale iterator on a single-fragment datagram; the merged overlapping datagram
trips the payload-length assertion; the ICMPv6 type switch throws on a type it does not
know; and the report about an unknown protocol reaches a dispatcher that throws. A model
that stops on crafted or unusual input cannot be used for any scenario that carries it, so
these four rank above the field gaps. The unknown *error* type, by contrast, passes: the
model routes every type below 128 as an error and stays silent.

### 4. A report about a link-layer multicast or broadcast

RFC 4443 §2.4 (e.4) and (e.5) forbid a report about a packet that arrived as a link-layer
multicast or broadcast. The model's ICMPv6 decides from the IPv6 addresses only. The same
gap as in IPv4, found the same way.

### 5. The model cites the obsoleted base document, and it shows

`Ipv6Header.msg`, `Ipv6ExtensionHeaders.msg`, `Ipv6ExtensionHeaders.cc` and `Ipv6FragBuf.cc`
cite RFC 2460, and `Icmpv6.h` cites RFC 2463; both were replaced (2017 and 2006). One
comment in `Ipv6.cc` cites RFC 8200. At level 2 the difference cost nothing. At level 3 the
two reassembly rules that RFC 8200 added after RFC 2460 — overlapping fragments (RFC 5722)
and atomic fragments (RFC 6946) — are the two that fail, and the reassembly code quotes
RFC 2460 §4.5 above the very loop that merges an overlap. The citation is not a cosmetic
gap: it names the version the code follows. A documentation pass that names RFC 8200 and
RFC 4443 in `Ipv6.ned` and in the message definitions, and a reassembly pass against
RFC 8200 §4.5, belong together.

### 6. No claim is contradicted on the normal path

Within nine features and 34 statements, on one topology with one packet size per check, the
model does what RFC 8200 and RFC 4443 describe on the normal path: it forwards, decrements,
discards and reports, fragments at the source and never at the router, and reassembles.
The two gaps are fields, not mechanisms.

## What this document does not establish

- It says nothing about the documents outside the in-scope set: RFC 8201 (path MTU
  discovery), RFC 4861 and RFC 4862 (neighbor discovery and autoconfiguration), the flow
  label and the traffic class.
- It says nothing about level 4 and beyond: the reassembly timer, the rate limit,
  congestion, the other extension headers; nor about the redirect and point-to-point cases
  the mockup cannot produce.
- A `confirmed` verdict means the checks of the feature passed. It does not mean the
  feature is complete.
