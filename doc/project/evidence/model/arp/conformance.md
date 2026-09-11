# ARP — model claims and conformance matrix

> **Kind:** report · **Status:** snapshot 2026-09-11 · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/arp/standards.md), [features.md](../../protocol/arp/features.md), [coverage.md](coverage.md)

Step 8 artifact of the standards test workflow. The tests tell what the model does. This
document adds what the model intends: which standards it claims to implement, and how the
claim compares with the feature support that [`coverage.md`](coverage.md) measured.

- Date: 2026-09-11 10:50 +0200
- INET: branch `master`, commit `223ba89ce5`, tree clean
- OMNeT++: 6.4.0
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-31-generic x86_64
- Command: `inet_run_protocol_tests -p inet -w arp`
- Ledger state: [`coverage.md`](coverage.md#feature-support), 7 features supported, 1 partial,
  1 untested.

## Part 1 — the claims

Every place in the model that names a standard of the ARP family:

| Claim | Where | What it says |
| --- | --- | --- |
| none | [`Arp.ned:15-16`](../../../../../src/inet/networklayer/arp/ipv4/Arp.ned) | "Implements the Address Resolution Protocol for IPv4 and IEEE 802 6-byte MAC addresses." The documentation comment of the module names **no document at all**. |
| RFC 826 | [`Arp.cc:238`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) | "Recipe a'la RFC 826:", above a comment that quotes the reception algorithm in full. |
| RFC 826, RFC 1868, RFC 903 | [`ArpPacket.msg:20-23`](../../../../../src/inet/networklayer/arp/ipv4/ArpPacket.msg) | One citation per opcode value: 1 and 2 from RFC 826, 2 also from RFC 1868, and 3 and 4 from RFC 903. |
| RFC 5227 | [`Arp.cc:485-486`](../../../../../src/inet/networklayer/arp/ipv4/Arp.cc) | "A client should send out 'ARP Probe' to probe the newly received IPv4 address. Refer to RFC 5227, IPv4 Address Conflict Detection", above `sendArpProbe`. |
| RFC 1122 §6.4 | [`ch-ipv4.rst:157-158`](../../../../../doc/src/users-guide/ch-ipv4.rst) | The user guide cites RFC 1122 §6.4 for computing a hardware address from a broadcast or multicast protocol address, which is the case where ARP is not used at all. |

The user guide also states two scope decisions without citing a document
([`ch-ipv4.rst:147-158`](../../../../../doc/src/users-guide/ch-ipv4.rst)): the module does
IPv4-to-MAC translation only, and it does not do the reverse direction, RARP.

Mapped onto the standards map, [`standards.md`](../../protocol/arp/standards.md#document-list):

| Document | In the in-scope set | Claimed | Note |
| --- | --- | --- | --- |
| RFC 826 | yes, `base` | yes, twice, both in a comment | The two citations are the whole claim: a source comment that says "a'la" — like — and an enum comment. No documentation comment of any module names it. |
| RFC 1122, the three ARP clauses | yes, `companion` | **no** | Nothing in the model names §2.3.2, §2.3.3 or §2.4. The one RFC 1122 citation points at §6.4, which is a different subject. |
| RFC 5494 | yes, `updates` | **no** | Nothing names it. The number spaces it governs are constants in the serializer. |
| RFC 5227 | no, level 4 | yes, in a comment | A claimed document outside the in-scope set. It is a scope gap for a later pass, not a verdict. The method it stands above has no caller. |
| RFC 903 | no, level 5 | named, and declined | The opcode enum names it, and the model throws on both of its opcodes, which is a deliberate refusal. The user guide says the same in words. |
| RFC 1868 | no, level 5 | named | One citation, on the reply opcode. The model implements nothing of UNARP. |

**The finding of the survey.** The model implements the Address Resolution Protocol and it
never claims a document in a place a reader would look. The documentation comment of the
`Arp` module, which is what the module reference publishes, names the protocol by its name
and no standard by its number. The two RFC 826 citations are both in a comment that a reader
of the documentation never sees, and one of them is hedged: "a'la". This is a documentation
task, and it is cheap — the code follows RFC 826 step by step, so the claim would be true.

The second half of the finding is larger. The model meets five requirements of RFC 1122 that
RFC 826 does not state — the cache flush, the configurable lifetime, the request rate, the
saved datagram, and the silence after a failure — and it claims none of them. Two of the five
it exceeds. A reader of the model has no way to learn that it was built to a second document,
and a reader of RFC 1122 has no way to learn that the model answers it.

## Part 2 — the conformance matrix

Feature by feature, not test by test. A feature is `claimed` when the claims of part 1 cover
its governing source document. The verdict combines the claim, the support value of the
ledger, and the level of the feature, by the table of the guide.

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [ARP-F-RESOLUTION](../../protocol/arp/features.md#arp-f-resolution) | mandatory | yes, RFC 826 | supported | **confirmed** |
| [ARP-F-REPLY](../../protocol/arp/features.md#arp-f-reply) | mandatory | yes, RFC 826 | supported | **confirmed** |
| [ARP-F-PACKET-FORMAT](../../protocol/arp/features.md#arp-f-packet-format) | mandatory | yes, RFC 826 | supported | **confirmed** for the layout; the two number-space statements of RFC 5494 are `undocumented` |
| [ARP-F-INPUT-VALIDATION](../../protocol/arp/features.md#arp-f-input-validation) | mandatory | yes, RFC 826 | partial | **partial** — the gap checks are RFC826-RECV-2, RECV-3 and the non-reply branch of RECV-10 |
| [ARP-F-CACHE](../../protocol/arp/features.md#arp-f-cache) | mandatory | RFC 826 yes, RFC 1122 §2.3.2.1 no | supported | **undocumented** for the flush half, **confirmed** for the table half |
| [ARP-F-QUEUE](../../protocol/arp/features.md#arp-f-queue) | optional | no | supported | **undocumented** |
| [ARP-F-FLOOD-PREVENTION](../../protocol/arp/features.md#arp-f-flood-prevention) | mandatory | no | supported | **undocumented** |
| [ARP-F-NO-ERROR-REPORT](../../protocol/arp/features.md#arp-f-no-error-report) | mandatory | no | supported | **undocumented** |
| [ARP-F-GENERALIZATION](../../protocol/arp/features.md#arp-f-generalization) | optional | yes, RFC 826 | untested | **unverified** |

Three confirmed, one partial, four undocumented, one unverified. No `defect`: every feature
the model claims and does not fully do is `partial`, which means a core check of it passed
too.

The headlines for the next pass, by the rule of the guide:

1. **The one `partial`.** `ARP-F-INPUT-VALIDATION` is mandatory and claimed, and three checks
   of it fail. All three are the same defect in shape: RFC 826 asks for a silent discard and
   the model stops the run. The details and the two gap numbers are in
   [`results.md`](results.md#the-failures).
2. **The four `undocumented`.** These are a documentation task for the model and not a test
   task. Each one is a requirement of RFC 1122 that the model meets and never names. The
   cheapest fix is one sentence in the documentation comment of the `Arp` module, which today
   names no document at all.
3. **The one `unverified`.** `ARP-F-GENERALIZATION` is optional, so it is a coverage gap and
   not a model verdict. It needs a second hardware type, and the packet class of the model
   has no field for one; see gap 2 in [`results.md`](results.md#the-failures).
4. **The claim outside the set.** RFC 5227 is claimed in a comment and is out of scope until
   level 4. The two methods that answer it, `Arp::sendArpProbe` and
   `Arp::sendArpGratuitous`, have no caller anywhere in the tree, so the claim rests on code
   that nothing runs. That is the first question of the level 4 pass.
