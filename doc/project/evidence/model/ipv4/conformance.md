# IPv4 — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-08 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/ipv4/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/ipv4/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan: 2026-09-08, worktree at commit `63ef8b0322`. `Ipv4.ned` and `Icmp.ned` are
  byte-identical to the tree of pass 1, and the scan gave the same result.
- Support values: [`coverage.md`](coverage.md#feature-support), from the run of 2026-09-08.

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

A grep for `RFC 791`, `RFC791`, `RFC 792` and `RFC792` over `src/inet/networklayer/ipv4/`
and `src/inet/networklayer/contract/ipv4/` returns exactly two lines, both in legacy headers
derived from BSD:

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [headers/ip.h:44](../../../../../src/inet/networklayer/ipv4/headers/ip.h#L44) | "Per RFC 791, September 1981." | citation on a legacy definition header |
| [headers/ip_icmp.h:44](../../../../../src/inet/networklayer/ipv4/headers/ip_icmp.h#L44) | "Per RFC 792, September 1981." | citation on a legacy definition header |

Both files carry a Berkeley version banner and define C structures. The active datagram
class of the model is the generated `Ipv4Header` of `Ipv4Header.msg`, not `struct ip`. A
citation on a vestigial header is not an implementation claim.

### How this pass reads the claim

The model claims the protocols but pins no version. Two readings are possible, and the
choice changes the whole matrix, so the reading is recorded here rather than assumed:

- **Strict reading** — a claim needs a document. Then nothing is claimed, every feature
  falls into `undocumented` or `out of claim`, and the matrix says nothing about the model.
- **Reading of this pass** — "Implements the IPv4 protocol" is an implicit claim on the
  Internet Standard for IPv4, which is RFC 791, and "ICMP implementation" an implicit claim
  on RFC 792. Both hold the status Internet Standard, neither is obsoleted, and no competing
  version of either exists. See [`standards.md`](../../protocol/ipv4/standards.md#document-list).

This pass uses the second reading and marks the claim `implicit` in the matrix. The missing
version statement is not lost by this choice: it is the first finding below.

## Part 2 — conformance matrix

The verdict combines the claim on the governing source document, the support value of the
ledger, and the level of the feature, by the table of step 8.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| IPV4-F-DELIVERY | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-HEADER | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-TTL | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-FRAGMENTATION | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-REASSEMBLY | unstated | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-DONT-FRAGMENT | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-IDENTIFICATION | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-HEADER-CHECKSUM | mandatory | yes (implicit, RFC 791) | partial | **partial** — RFC791-CKSUM-2 has not run; level 3 |
| IPV4-F-MIN-SIZE | mandatory | yes (implicit, RFC 791) | supported | **confirmed** |
| IPV4-F-ERROR-REPORT | optional | yes (implicit, RFC 792) | supported | **confirmed** |

Nine features `confirmed`, one `partial`. No `defect`, no `unverified`, no `undocumented`,
no `declined`.

## Findings

### 1. The model states no version for either protocol

No active module of the IPv4 tree names RFC 791 or RFC 792. This costs nothing today,
because neither document has ever been replaced, and a great deal at the first override:
when RFC 6864 enters the in-scope set, nothing in the model answers whether it intends the
1981 identification rule or the 2013 one. The project has the house style for the fix one
module away:

> "This module implements both IGMPv2 host and router logic as specified in RFC 2236."
> — [Igmpv2.ned:25-26](../../../../../src/inet/networklayer/ipv4/Igmpv2.ned#L25-L26)

A documentation change in `Ipv4.ned` and `Icmp.ned` would turn every `implicit` in the
matrix into an explicit claim. This is a task for the model, not for the tests.

### 2. The header checksum: computed on request, verified only in part

Two observations from the code, both recorded in
[`results.md`](results.md#model-observations-the-checks-did-not-claim):

- The default `checksumMode` is `"declared"`, under which the field is a placeholder. The
  level 2 check passed with the computed mode enabled, which the model offers and the check
  document requires. Within a simulation the default is a legitimate shortcut.
- On receipt, the checksum is consulted only when the header is already structurally wrong
  (`Ipv4.cc:282`). A structurally correct header with a wrong checksum passes. That is the
  verification half of the feature, RFC791-CKSUM-2, and it needs a corrupted datagram in
  flight, which is the level 3 toolset. The matrix therefore says `partial` and not
  `defect`; the code reading makes CKSUM-2 the first candidate for a `defect` when level 3
  runs. The tests do not touch the model; the observation waits for its check.

### 3. No claim is contradicted at level 2

Within ten features and twenty-one statements, on one topology with one datagram size per
check, the model does what RFC 791 and RFC 792 describe on the normal path. Pass 1 left two
mandatory features untested; this pass tested them, and both passed.

## What this document does not establish

- It says nothing about the documents outside the in-scope set. RFC 1122, RFC 6864 and
  RFC 2474 all change RFC 791, and the model is neither claimed nor checked against them.
- It says nothing about level 3 and beyond: corrupted input, interleaved fragment trains,
  the reassembly timer, the options.
- A `confirmed` verdict means the checks of the feature passed. It does not mean the
  feature is complete.
