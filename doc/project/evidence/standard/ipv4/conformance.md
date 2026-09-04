# IPv4 — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-04 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](features.md), [standards.md](standards.md), [rfc791-results.md](rfc791-results.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan date: 2026-09-04, worktree at commit `530115b0fc`.
- Support values: the run of 2026-09-02, from
  [`rfc791-results.md`](rfc791-results.md#feature-support-derived-from-the-verdicts).
- The two dates describe the same model. The commits between `6208e77255` and `530115b0fc`
  touch `doc/` only; `git diff 6208e77255 530115b0fc -- src/` is empty.

This is the one document of the workflow whose first part reads the model documentation on
purpose. The claims must not travel back into the catalogs, the feature map, or the check
descriptions.

## Part 1 — what the model claims

### The claims of the active modules

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [Ipv4.ned:30-31](../../../../../src/inet/networklayer/ipv4/Ipv4.ned#L30-L31) | "Implements the IPv4 protocol. The protocol header is represented by the ~Ipv4Header message class." | protocol claim, no document |
| [Icmp.ned:12](../../../../../src/inet/networklayer/ipv4/Icmp.ned#L12) | "ICMP implementation." | protocol claim, no document |

These two comments are the whole module-level claim. Neither names a document, a number, or
a year.

### Document references elsewhere in the IPv4 tree

A grep for `RFC 791`, `RFC791`, `RFC 792`, and `RFC792` over
`src/inet/networklayer/ipv4/` and `src/inet/networklayer/contract/ipv4/` returns exactly two
lines, and both sit in legacy headers derived from BSD:

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [headers/ip.h:44](../../../../../src/inet/networklayer/ipv4/headers/ip.h#L44) | "Per RFC 791, September 1981." (above "Definitions for internet protocol version 4.") | citation on a legacy definition header |
| [headers/ip_icmp.h:44](../../../../../src/inet/networklayer/ipv4/headers/ip_icmp.h#L44) | "Per RFC 792, September 1981." (above "Interface Control Message Protocol Definitions.") | citation on a legacy definition header |

Both files carry a Berkeley version banner and define C structures and constants. The
active datagram class of the model is the generated `Ipv4Header` of `Ipv4Header.msg`, not
`struct ip`. A citation on a vestigial header is not an implementation claim.

The remaining references in the tree are narrower still: one constant or one field each.
They are listed for completeness and none of them claims a feature.

| Source | Verbatim text | What it covers |
| --- | --- | --- |
| [Ipv4FragBuf.h:80](../../../../../src/inet/networklayer/ipv4/Ipv4FragBuf.h#L80) | "Timeout should be between 60 seconds and 120 seconds (RFC1122)." | one timeout constant |
| [headers/ip_icmp.h:75](../../../../../src/inet/networklayer/ipv4/headers/ip_icmp.h#L75) | "ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191)" | one constant |
| [headers/ip.h:160](../../../../../src/inet/networklayer/ipv4/headers/ip.h#L160) | "default ttl, from RFC 1340" | one constant |
| [IcmpHeader.msg:28](../../../../../src/inet/networklayer/ipv4/IcmpHeader.msg#L28) | "2 bytes, RFC 1071" | the checksum field |
| [Ipv4Header.msg:113](../../../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L113) | "RFC 781" | the timestamp option class |
| [Ipv4.cc:478](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L478) | "RFC 1112, section 6.1" | one multicast code path |

### How this pass reads the claim

The model claims the protocols but pins no version. Two readings are possible, and the
choice changes the whole matrix, so the reading is recorded here rather than assumed:

- **Strict reading** — a claim needs a document. Then nothing is claimed, every feature
  falls into `undocumented` or `out of claim`, and the matrix says nothing about the
  model.
- **Reading of this pass** — "Implements the IPv4 protocol" is an implicit claim on the
  Internet Standard for IPv4, which is RFC 791, and "ICMP implementation" is an implicit
  claim on RFC 792. Both documents hold the status Internet Standard, neither is obsoleted,
  and no competing version of either exists. See
  [`standards.md`](standards.md#document-list).

This pass uses the second reading, and marks the claim strength `implicit` in the matrix.
The missing version statement is not lost by this choice: it is the first finding below.

## Part 2 — conformance matrix

The matrix works at feature level. The verdict combines the claim on the governing source
document of a feature with the support value of [`features.md`](features.md).

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| IPV4-F-TTL | mandatory | yes (implicit, RFC 791) | partial | **partial** — RFC791-TTL-2 never ran |
| IPV4-F-FRAGMENTATION | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-REASSEMBLY | unstated | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-DONT-FRAGMENT | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-IDENTIFICATION | mandatory | yes (implicit, RFC 791) | partial | **partial** — RFC791-ID-1 never ran |
| IPV4-F-HEADER-CHECKSUM | unstated | yes (implicit, RFC 791) | untested | **unverified** |
| IPV4-F-MIN-SIZE | mandatory | yes (implicit, RFC 791) | untested | **unverified** |
| IPV4-F-ERROR-REPORT | optional | yes (implicit, RFC 792) | partial | **partial** — RFC792-TE-1 never ran |

Three features are `confirmed`, three are `partial`, two are `unverified`. No feature is a
`defect`, and none is `undocumented`.

## Findings

### 1. The model states no version for either protocol

No active module of the IPv4 tree names RFC 791 or RFC 792. The only two mentions sit on
legacy BSD headers that the simulation does not use for the datagram.

This costs nothing today, because neither document has ever been replaced. It costs a great
deal at the first override. The moment RFC 6864 enters the in-scope set, the question "does
this model intend the 1981 identification rule or the 2013 one?" has no answer in the model,
and the same holds for RFC 2474 over the type of service byte.

The project already has a house style for the fix, one module away:

> "This module implements both IGMPv2 host and router logic as specified in RFC 2236."
> — [Igmpv2.ned:25-26](../../../../../src/inet/networklayer/ipv4/Igmpv2.ned#L25-L26)

A documentation change in `Ipv4.ned` and `Icmp.ned` in that style would turn every
`implicit` in the matrix into an explicit claim. This is a task for the model, not for the
tests.

### 2. Two mandatory features are unverified

`IPV4-F-HEADER-CHECKSUM` and `IPV4-F-MIN-SIZE` carry no check that ran. Both are the
headline of the next pass, per step 8 of the workflow. Neither is a statement about the
model: an unverified feature is a gap in the tests.

- The checksum recompute (RFC791-CKSUM-1) needs no new tooling. It pairs with the existing
  TTL mockup: the checksum must change across the hop where the TTL changes.
- The minimum sizes (RFC791-FRAG-6, RFC791-REASM-2) need only two more scenario constants.

### 3. No claim is contradicted

No feature reached the `defect` verdict, and the run produced no
`%# expected-result: FAIL`. Within the reach of eight features and nine checks, the model
does what the two documents describe.

The reach is the limit of the statement. Three features rest on a single check, and the
whole matrix rests on one datagram size, one MTU, and one topology.

## What this document does not establish

- It says nothing about the documents outside the in-scope set. RFC 1122, RFC 6864, and
  RFC 2474 all change RFC 791, and the model is neither claimed nor checked against them.
- It says nothing about the areas the catalogs put out of scope: options, type of service,
  the security annex, the reassembly timer, and the other ICMP messages.
- A `confirmed` verdict means the checks of the feature passed. It does not mean the
  feature is complete.
