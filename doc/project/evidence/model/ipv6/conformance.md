# IPv6 — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-23 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/ipv6/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/ipv6/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan: 2026-09-09, worktree at commit `95f9805952`. A grep for `RFC 8200`,
  `RFC8200`, `RFC 2460`, `RFC2460`, `RFC 4443`, `RFC4443`, `RFC 2463`, `RFC 8504`,
  `RFC 5722`, `RFC 6946` and `RFC 8021` over `src/inet/networklayer/ipv6/`,
  `src/inet/networklayer/icmpv6/` and `src/inet/networklayer/contract/ipv6/`, plus a read of
  the module documentation comments. The pass 2 additions (RFC 8504 and the four documents
  it restates) return no line. The claim scan did not run again for this refresh; see the
  note at the top of Part 1.
- Support values, from this run:
  - Date: 2026-09-23 18:26 +0200
  - INET: branch `topic/standards-tests-wave0`, commit `28536bd0a5` (on `master`), tree clean
  - Trees: src `16dc528e10`, tests/protocol `6f0a6bdb05`
  - OMNeT++: 6.4.0, commit `cf58891643`
  - Build: debug, built from this commit
  - Compiler: Ubuntu clang version 23.0.0
  - Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
  - Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/ipv6$'`
- Ledger state: [`coverage.md`](coverage.md#feature-support), from the run above, on
  `master` commit `28536bd0a5`. Eight verdicts flip from FAIL to PASS against the
  2026-09-10 snapshot (commit `0868c36c88`), all repaired on 2026-09-14; see
  [`coverage.md`](coverage.md#pass-log).

This is the one document of the workflow whose first part reads the model documentation on
purpose. The claims must not travel back into the catalogs, the feature map, or the check
descriptions.

## Part 1 — what the model claims

Part 1 was read on 2026-09-09 and this refresh did not read the claims again.

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
| IPV6-F-HEADER | mandatory | yes (implicit, RFC 8200; RFC 2460 cited) | supported | **confirmed** — RFC8200-HDR-2 repaired (`84bc80aa8b`) |
| IPV6-F-DELIVERY | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-HOP-LIMIT | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-SOURCE-FRAGMENTATION | optional | yes (RFC 2460 §4.5 cited on the fragment header) | supported | **confirmed** — RFC8200-FRAG-4, FRAG-5 repaired (`84bc80aa8b`) |
| IPV6-F-REASSEMBLY | mandatory | yes (RFC 2460 §4.5 cited in the reassembly code) | supported | **confirmed** — RFC8504-NR-3, RFC8200-REASM-5 (overlapping fragments) and RFC8200-REASM-6 (the atomic fragment, a should) repaired (`ab3b1f1ea0`) |
| IPV6-F-PACKET-TOO-BIG | mandatory | yes (explicit, RFC 4443) | supported | **confirmed** — RFC4443-PTB-2 repaired (`84bc80aa8b`) |
| IPV6-F-PACKET-SIZE | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-ERROR-REPORT | mandatory | yes (explicit, RFC 4443) | supported | **confirmed** |
| IPV6-F-UPPER-LAYER-CHECKSUM | mandatory | yes (implicit, RFC 8200) | supported | **confirmed** |
| IPV6-F-INPUT-VALIDATION | mandatory | yes (implicit, RFC 8200; explicit, RFC 4443) | supported | **confirmed** — RFC4443-MPR-2 repaired (`40c9f04e21`) |
| IPV6-F-ERROR-SUPPRESSION | mandatory | yes (explicit, RFC 4443) | supported | **confirmed** — RFC4443-MPR-7, MPR-8 repaired (`37ddbfa7a8`) |
| IPV6-F-ERROR-DELIVERY | mandatory | yes (explicit, RFC 4443) | untested | **unverified** — every core statement is internal; RFC4443-MPR-3, a supporting statement, no longer stops the node (repaired, `cbe91d57c5`) |

Eleven features `confirmed`, one `unverified`. No `defect` in the strict sense of the
table: every core check that ran passed, or has since been repaired. No `undocumented`,
no `declined` at the feature level.

## Findings

The two findings of pass 1 come first, both since repaired; the level 3 findings follow —
findings 3 and 4 are since repaired, finding 5 partly.

### 1. Packet Too Big said MTU 0, against an explicit claim

**Repaired by `84bc80aa8b`, on 2026-09-14.** The ICMPv6 module names RFC 4443, sends Packet
Too Big to the right node with the right type, and used to put 0 in the MTU field ("TODO
implement MTU support"). RFC 4443 §3.2 defines the field as the MTU of the next-hop link,
and path MTU discovery (RFC 8201) is nothing but a reader of that field; a source could
never learn a path MTU below its own link MTU. This was the headline of the pass: a
mandatory message, explicitly claimed, that carried no information. `Icmpv6::sendErrorMessage`
takes the MTU now, and `Ipv6` passes the one it already reads from the outgoing interface.

### 2. Every fragment carried the payload length of the original packet

**Repaired by `84bc80aa8b`, on 2026-09-14.** RFC 8200 §4.5 requires the payload length of
each fragment packet to be "changed to contain the length of this fragment packet only".
The model used to copy the base header into every fragment and never rewrite the field: a
1280-octet fragment packet and a 276-octet one both said 1460. `Ipv6::fragmentAndSend` builds
each fragment's payload length from that fragment's own share now. IPV6-F-HEADER and
IPV6-F-SOURCE-FRAGMENTATION both read `confirmed`; the field that put them at `partial` is
fixed.

### 3. Four crafted inputs stopped the simulation instead of being discarded

**Repaired by `ab3b1f1ea0`, `40c9f04e21` and `cbe91d57c5`, on 2026-09-14.** RFC 8200 and
RFC 4443 ask a node to discard, silently or with a report, an overlapping fragment set, an
atomic fragment (to process it as a whole), an ICMPv6 informational message of unknown type,
and a report about a packet whose upper-layer protocol it does not implement. The model used
to answer each with an assertion or a runtime error: the fragment buffer erased a stale
iterator on a single-fragment datagram; the merged overlapping datagram tripped the
payload-length assertion; the ICMPv6 type switch threw on a type it did not know; and the
report about an unknown protocol reached a dispatcher that threw. A model that stops on
crafted or unusual input cannot be used for any scenario that carries it, which is why these
four ranked above the field gaps. Each of the four discards now, silently or with a report,
as the standards ask. The unknown *error* type passed throughout: the model routes every
type below 128 as an error and stays silent.

### 4. A report about a link-layer multicast or broadcast

**Repaired by `37ddbfa7a8`, on 2026-09-14.** RFC 4443 §2.4 (e.4) and (e.5) forbid a report
about a packet that arrived as a link-layer multicast or broadcast. The model's ICMPv6 used
to decide from the IPv6 addresses only; it also reads the `MacAddressInd` tag of the frame
the packet arrived in now. The same gap existed in IPv4, found the same way, and was
repaired the same way.

### 5. The model cited the obsoleted base document, and it showed

**Partly repaired by `ab3b1f1ea0` and `84bc80aa8b`, on 2026-09-14.** `Ipv6Header.msg`,
`Ipv6ExtensionHeaders.msg`, `Ipv6ExtensionHeaders.cc` and most of `Ipv6FragBuf.cc` still cite
RFC 2460, and `Icmpv6.h` still cites RFC 2463; both were replaced (2017 and 2006). At level 2
the difference cost nothing. At level 3 the two reassembly rules that RFC 8200 added after
RFC 2460 — overlapping fragments (RFC 5722) and atomic fragments (RFC 6946) — were the two
that failed, and the reassembly code quoted RFC 2460 §4.5 above the very loop that merges an
overlap. Both rules pass now: `Ipv6FragBuf` computes the reassembled payload length by
RFC 8200 §3, and the new comment at the fix cites RFC 8200 by name. The short-fragment and
oversized-offset rules beside it, unaffected by the repair, still cite RFC 2460. A
documentation pass that names RFC 8200 and RFC 4443 in `Ipv6.ned` and in the message
definitions, and that updates the remaining RFC 2460 and RFC 2463 citations, is still owed.

### 6. No claim was contradicted on the normal path

Within nine features and 34 statements, on one topology with one packet size per check, the
model does what RFC 8200 and RFC 4443 describe on the normal path: it forwards, decrements,
discards and reports, fragments at the source and never at the router, and reassembles.
The two gaps were fields, not mechanisms, and both are repaired now (`84bc80aa8b`).

## What this document does not establish

- It says nothing about the documents outside the in-scope set: RFC 8201 (path MTU
  discovery), RFC 4861 and RFC 4862 (neighbor discovery and autoconfiguration), the flow
  label and the traffic class.
- It says nothing about level 4 and beyond: the reassembly timer, the rate limit,
  congestion, the other extension headers; nor about the redirect and point-to-point cases
  the mockup cannot produce.
- A `confirmed` verdict means the checks of the feature passed. It does not mean the
  feature is complete.
