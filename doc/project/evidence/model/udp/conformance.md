# UDP — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-10 · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/udp/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/udp/standards.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model says it intends to do, and compares the two at the level of
features, never at the level of a single test.

- Claim scan: 2026-09-09 on `master`. The claims of the model did not change between the two
  passes; part 1 below is unchanged.
  The claim scan did not run again on 2026-09-10. Every source file that part 1 cites is
  identical to the file at the commit above, so the claims still hold.
- Support values: [`coverage.md`](coverage.md#feature-support), from the run of 2026-09-10
  on `master`, commit `0868c36c88`. Every verdict of that run repeats the verdict of the
  earlier pass.

This is the one document of the workflow whose first part reads the model documentation on
purpose. The claims must not travel back into the catalogs, the feature map, or the check
descriptions.

## Part 1 — what the model claims

### The claim of the active module

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [Udp.ned:14](../../../../../src/inet/transportlayer/udp/Udp.ned#L14) | "UDP protocol implementation, for IPv4 (~Ipv4) and IPv6 (~Ipv6)." | protocol claim, no document |
| [Udp.ned:45](../../../../../src/inet/transportlayer/udp/Udp.ned#L45) | "the outgoing packet will have the correctly computed checksum as defined by the RFC" | a reference to "the RFC", no number |
| [Udp.ned:54](../../../../../src/inet/transportlayer/udp/Udp.ned#L54) | "a potentially incorrect checksum that is to be verified as defined by the RFC" | the same |

The module says "the RFC" twice and never says which. For UDP the reader can guess, and the
guess is right; the document does not say so.

### Document references elsewhere in the UDP tree

| Source | Verbatim text | Kind |
| --- | --- | --- |
| [Udp.cc:872](../../../../../src/inet/transportlayer/udp/Udp.cc#L872) | "// Excerpt from RFC 768:" followed by the two sentences of `rfc768.txt:92-95`, quoted verbatim | the strongest reference in the tree: the source text itself, at the line that implements it |
| [headers/udphdr.h:43](../../../../../src/inet/transportlayer/udp/headers/udphdr.h#L43) | "Per RFC 768, September, 1981." | a citation on a BSD-derived header that nothing in `src/` includes; the date is wrong, RFC 768 is dated 28 August 1980 |
| [Udp.cc:88-95](../../../../../src/inet/transportlayer/udp/Udp.cc#L88-L95) | a `TODO` that paraphrases the IPv6 rule "the UDP checksum is not optional" | a rule stated without its document |

RFC 1122 and RFC 8085 appear nowhere under `src/inet/transportlayer/udp/`. This matters more
at level 3 than it did at level 2: RFC 1122 §4.1 is now the document that governs the
checksum feature and the input validation of a host, and the model names it nowhere.

### How this pass reads the claim

As for IPv4: "UDP protocol implementation" is an implicit claim on the Internet Standard
for UDP, RFC 768, which has never been obsoleted. The verbatim excerpt in the code makes
the reading firmer than it was for IPv4 — the model demonstrably works from that text — but
an excerpt on one rule is not a statement of scope. The matrix marks the claim `implicit`.

## Part 2 — conformance matrix

The verdict combines the claim on the governing source document, the support value of the
ledger, and the level of the feature, by the table of step 8.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| UDP-F-DELIVERY | mandatory | yes (implicit, RFC 768) | supported | **confirmed** |
| UDP-F-HEADER | mandatory | yes (implicit, RFC 768) | partial | **partial** — the minimum length of eight, gap 3 |
| UDP-F-CHECKSUM | mandatory | yes (implicit, RFC 768; the modes are documented "as defined by the RFC") | partial | **partial** — the default is a placeholder, gap 1 |
| UDP-F-PORT-UNREACHABLE | optional | yes (implicit, RFC 792 through the ICMP module) | supported | **confirmed** |
| UDP-F-INPUT-VALIDATION | mandatory | no (RFC 1122 is named nowhere in the module) | partial | **out of claim** — the checksum half works, the address half, gap 2, does not |
| UDP-F-ERROR-DELIVERY | mandatory | no (the same) | untested | **out of claim** |
| UDP-F-APP-INTERFACE | mandatory | no (the same) | partial | **out of claim** |

Two features `confirmed`, two `partial`, three `out of claim`. No `defect`: a `defect` needs
a claim, and the model claims RFC 768 alone.

The three `out of claim` rows are the honest reading of a model that names one document. The
model does not claim RFC 1122, so it cannot fail RFC 1122 in this matrix. That does not make
the three gaps less real; it places them. Two readings are open to whoever owns the module:

- The claim is right and the model is a UDP implementation for a simulation, in which case
  the RFC 1122 statements are a wish list and the gaps are known limits.
- The claim is too narrow. A model of a host that is used to study hosts is expected to
  behave like a host, and RFC 1122 is the document that says what a host does. Then the
  first task is to state the claim, and the second is gap 2, the only one where the model
  does nothing at all.

Pass 1 recorded four `confirmed` and nothing else. Nothing in the model changed. What changed
is the set of documents the model is read against; see
[`coverage.md`](coverage.md#feature-support).

## Findings

### 1. The model states no document, one edit away from doing so

`Udp.ned` says "the RFC" and `Udp.cc` quotes RFC 768 by name at the line that implements
its all-ones rule. The number that the module documentation lacks is already in the code.
The only place that cites the document with a date is a dead header, and the date there is
wrong. The house style is one module away
([Igmpv2.ned:25-26](../../../../../src/inet/networklayer/ipv4/Igmpv2.ned#L25-L26)). A
documentation task for the model, not for the tests.

### 2. The checksum: real only on request, verified always when real

Two observations from the code, recorded in
[`results.md`](results.md#model-observations-the-checks-did-not-claim):

- The default `checksumMode` is `"declared"`, a placeholder. The checksum check sets the
  computed mode on one sender and the disabled mode on the other; both behaviors of
  RFC 768 are then observable, and both pass.
- In the computed mode the receiver verifies every nonzero checksum and discards on failure
  ([Udp.cc:948-956](../../../../../src/inet/transportlayer/udp/Udp.cc#L948-L956)), and it
  accepts a zero over IPv4 as "no checksum"
  ([Udp.cc:1012-1017](../../../../../src/inet/transportlayer/udp/Udp.cc#L1012-L1017)). That
  is RFC 768 honored exactly, and RFC 1122's discard rule already in place for level 3. The
  contrast with the IPv4 header checksum, which the model consults only when the header is
  already structurally wrong, is worth a look by whoever owns both.

### 3. Nothing is contradicted at level 2

Within four features and seven statements, on one topology, the model does what RFC 768
describes. UDP is the first protocol in this tree to reach level 2 with every feature
`supported` and no `partial`; it is also the smallest, and the two observations that could
not run — the empty datagram and the program-interface half of the receive operation — are
limits of the tooling, not of the model.

### 4. At level 3 the second finding above became a verdict

Finding 2 of pass 1 read the checksum code and reported two things: the default mode is a
placeholder, and the discard rule of RFC 1122 is already in place. Pass 2 tested both.

- The discard rule holds, from three sides: a wrong value, changed data and a changed
  pseudo header each end in a silent discard. The prediction was right.
- The default is now measured against RFC 1122 §4.1.3.4, which demands the generation of a
  checksum by default. It is gap 1.

The contrast that finding 2 offered to "whoever owns both" is now measured on both sides.
The IPv4 header checksum is consulted only when the header is already structurally wrong,
and `tests/protocol/ipv4/Rfc1122ChecksumDiscard.test` fails for that reason. The UDP
checksum, in the computed mode, is verified every time, and the matching UDP test passes.
Two layers of one model, one rule, two different answers.

### 5. One gap belongs to neither layer under test

Gap 3 stops a run, and its cause is in neither UDP nor IPv4. It is a statistic that the
standard receiving program declares: `dataAge` calls `peekData()` on a packet of zero
length. Any protocol that can deliver an empty payload to an application meets it. A reader
who owns the result filters, and not UDP, is the audience for that gap.

## What this document does not establish

- It says nothing about the options of RFC 9868, which are level 5.
- It establishes nothing about the nine statements of RFC 1122 §4.1 that live at the
  interface between UDP and a program. `UDP-F-ERROR-DELIVERY` is `untested` for that reason,
  and `UDP-F-APP-INTERFACE` is partial for it. An `untested` feature is a coverage gap and
  not a verdict on the model.
- It says nothing about IPv6, where the checksum rules differ and the model has code paths
  this pass did not exercise.
- A `confirmed` verdict means the checks of the feature passed. It does not mean the
  feature is complete.
