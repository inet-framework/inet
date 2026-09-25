# MPLS — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc3031/catalog.md](../../standard/rfc3031/catalog.md), [rfc3032/catalog.md](../../standard/rfc3032/catalog.md), [rfc3443/catalog.md](../../standard/rfc3443/catalog.md), [rfc5462/catalog.md](../../standard/rfc5462/catalog.md), [features.md](../../protocol/mpls/features.md)

The single place that holds the changing state of the MPLS workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from this run:

- Date: 2026-09-29 18:34 +0200
- INET: branch `topic/standards-tests-mpls-level2-fixes`, commit `db2fd89622`, tree clean
- Trees: src `8006b0e093`, tests/protocol `31f57dca8a`
- OMNeT++: 6.4.0, debug build from this commit, Ubuntu clang 23.0.0, Ubuntu 26.04.1 LTS
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mpls$'`
- Suite: 21 tests, 20 PASS, 1 FAIL (expected), so the suite reports PASS
- Target level: 2

The analysis of every failure is in [`results.md`](results.md#the-model-gaps).

## Statement coverage

`Status` is the state of the workflow, not of the standard:

| Status | Means |
| --- | --- |
| `selected` | a check targets it |
| `covered` | a selected check establishes it as a side effect |
| `owed` | the model claims the behavior and **no check exists yet**; the note names what a check would need |
| `later` | it needs a toolset of a higher level — a label that no binding covers or a wrong control packet at level 3, a timer at level 4, a tunnel at level 5 |
| `no check` | the model does not claim the behavior, or a permission that no observation can fail, so [the third principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test) asks for no test |

A verdict belongs to the test. Where a test fails at a later observation and the observation
of a statement held, the row says PASS and names the observation in the note; where a failure
kept a test from the observation a statement needs, the row says FAIL and says so. A PASS
"with no weight" is a pass that the model cannot fail, because it does not do the thing that
the statement limits.

| Status | RFC 3031 | RFC 3032 | RFC 3443 | RFC 5462 | Together |
| --- | --- | --- | --- | --- | --- |
| `selected` | 24 | 39 | 10 | 1 | 74 |
| `covered` | 12 | 8 | 5 | 0 | 25 |
| `owed` | 0 | 2 | 0 | 0 | 2 |
| `later` | 5 | 18 | 0 | 0 | 23 |
| `no check` | 8 | 14 | 9 | 2 | 33 |
| all | 49 | 81 | 24 | 3 | 157 |

### RFC 3031

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC3031-LBL-1](../../standard/rfc3031/catalog.md#rfc3031-lbl-1) | no check | — | — | — | label distribution, a protocol of its own (LDP, RSVP-TE) out of the in-scope set |
| [RFC3031-LBL-2](../../standard/rfc3031/catalog.md#rfc3031-lbl-2) | no check | — | — | — | label distribution, a protocol of its own (LDP, RSVP-TE) out of the in-scope set |
| [RFC3031-LBL-3](../../standard/rfc3031/catalog.md#rfc3031-lbl-3) | covered | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3031-LBL-4](../../standard/rfc3031/catalog.md#rfc3031-lbl-4) | covered | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3031-STK-1](../../standard/rfc3031/catalog.md#rfc3031-stk-1) | selected | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | `Rfc3032TwoLabelStack` | PASS |  |
| [RFC3031-STK-2](../../standard/rfc3031/catalog.md#rfc3031-stk-2) | covered | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | `Rfc3032TwoLabelStack` | PASS |  |
| [RFC3031-NHLFE-1](../../standard/rfc3031/catalog.md#rfc3031-nhlfe-1) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-NHLFE-2](../../standard/rfc3031/catalog.md#rfc3031-nhlfe-2) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-NHLFE-3](../../standard/rfc3031/catalog.md#rfc3031-nhlfe-3) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-NHLFE-4](../../standard/rfc3031/catalog.md#rfc3031-nhlfe-4) | selected | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | `Rfc3032TwoLabelStack` | PASS |  |
| [RFC3031-NHLFE-5](../../standard/rfc3031/catalog.md#rfc3031-nhlfe-5) | covered | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-NHLFE-6](../../standard/rfc3031/catalog.md#rfc3031-nhlfe-6) | covered | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-NHLFE-7](../../standard/rfc3031/catalog.md#rfc3031-nhlfe-7) | selected | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | `Rfc3032TwoLabelStack` | PASS |  |
| [RFC3031-ILM-1](../../standard/rfc3031/catalog.md#rfc3031-ilm-1) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-ILM-2](../../standard/rfc3031/catalog.md#rfc3031-ilm-2) | later | — | — | — | level 3: more than one NHLFE for a label or a FEC, on a mockup with two paths |
| [RFC3031-FTN-1](../../standard/rfc3031/catalog.md#rfc3031-ftn-1) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-FTN-2](../../standard/rfc3031/catalog.md#rfc3031-ftn-2) | later | — | — | — | level 3: more than one NHLFE for a label or a FEC, on a mockup with two paths |
| [RFC3031-SWAP-1](../../standard/rfc3031/catalog.md#rfc3031-swap-1) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-SWAP-2](../../standard/rfc3031/catalog.md#rfc3031-swap-2) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-SWAP-3](../../standard/rfc3031/catalog.md#rfc3031-swap-3) | later | — | — | — | level 3: a packet that no binding covers |
| [RFC3031-SWAP-4](../../standard/rfc3031/catalog.md#rfc3031-swap-4) | covered | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-LSP-1](../../standard/rfc3031/catalog.md#rfc3031-lsp-1) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-LSP-2](../../standard/rfc3031/catalog.md#rfc3031-lsp-2) | covered | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-LSP-3](../../standard/rfc3031/catalog.md#rfc3031-lsp-3) | covered | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-LSP-4](../../standard/rfc3031/catalog.md#rfc3031-lsp-4) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-LSP-5](../../standard/rfc3031/catalog.md#rfc3031-lsp-5) | no check | — | — | — | a permission that no observation can fail |
| [RFC3031-LSP-6](../../standard/rfc3031/catalog.md#rfc3031-lsp-6) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3031-LSP-7](../../standard/rfc3031/catalog.md#rfc3031-lsp-7) | covered | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | `Rfc3032TwoLabelStack` | PASS |  |
| [RFC3031-PHP-1](../../standard/rfc3031/catalog.md#rfc3031-php-1) | selected | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | `Rfc3031PenultimateHopPopping` | PASS |  |
| [RFC3031-PHP-2](../../standard/rfc3031/catalog.md#rfc3031-php-2) | selected | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | `Rfc3031PenultimateHopPopping` | PASS |  |
| [RFC3031-PHP-3](../../standard/rfc3031/catalog.md#rfc3031-php-3) | selected | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | `Rfc3031PenultimateHopPopping` | PASS |  |
| [RFC3031-PHP-4](../../standard/rfc3031/catalog.md#rfc3031-php-4) | covered | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | `Rfc3031PenultimateHopPopping` | PASS |  |
| [RFC3031-PHP-5](../../standard/rfc3031/catalog.md#rfc3031-php-5) | covered | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | `Rfc3031PenultimateHopPopping` | PASS |  |
| [RFC3031-PHP-6](../../standard/rfc3031/catalog.md#rfc3031-php-6) | selected | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | `Rfc3031PenultimateHopPopping` | PASS |  |
| [RFC3031-PHP-7](../../standard/rfc3031/catalog.md#rfc3031-php-7) | no check | — | — | — | label distribution, a protocol of its own (LDP, RSVP-TE) out of the in-scope set |
| [RFC3031-PHP-8](../../standard/rfc3031/catalog.md#rfc3031-php-8) | no check | — | — | — | label distribution, a protocol of its own (LDP, RSVP-TE) out of the in-scope set |
| [RFC3031-INV-1](../../standard/rfc3031/catalog.md#rfc3031-inv-1) | later | — | — | — | level 3: a packet that no binding covers |
| [RFC3031-NOL-1](../../standard/rfc3031/catalog.md#rfc3031-nol-1) | later | — | — | — | level 3: a packet that no binding covers |
| [RFC3031-TTL-1](../../standard/rfc3031/catalog.md#rfc3031-ttl-1) | selected | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3031-TTL-2](../../standard/rfc3031/catalog.md#rfc3031-ttl-2) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3031-TTL-3](../../standard/rfc3031/catalog.md#rfc3031-ttl-3) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3031-TTL-4](../../standard/rfc3031/catalog.md#rfc3031-ttl-4) | selected | [The TTL of the first label](../../protocol/mpls/checks/ttl.md#the-ttl-of-the-first-label) | `Rfc3032FirstLabelTtl` | PASS |  |
| [RFC3031-TTL-5](../../standard/rfc3031/catalog.md#rfc3031-ttl-5) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3031-TTL-6](../../standard/rfc3031/catalog.md#rfc3031-ttl-6) | selected | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3031-TTL-7](../../standard/rfc3031/catalog.md#rfc3031-ttl-7) | no check | — | — | — | no link of INET lacks a TTL field; `Mpls.ned:39-40` names Frame Relay and ATM, which INET does not have |
| [RFC3031-TTL-8](../../standard/rfc3031/catalog.md#rfc3031-ttl-8) | no check | — | — | — | no link of INET lacks a TTL field; `Mpls.ned:39-40` names Frame Relay and ATM, which INET does not have |
| [RFC3031-NULL-1](../../standard/rfc3031/catalog.md#rfc3031-null-1) | selected | [The Implicit NULL label](../../protocol/mpls/checks/reserved-labels.md#the-implicit-null-label) | `Rfc3032ImplicitNull` | PASS |  |
| [RFC3031-NULL-2](../../standard/rfc3031/catalog.md#rfc3031-null-2) | no check | — | — | — | label distribution, a protocol of its own (LDP, RSVP-TE) out of the in-scope set |
| [RFC3031-NULL-3](../../standard/rfc3031/catalog.md#rfc3031-null-3) | covered | [Penultimate hop popping](../../protocol/mpls/checks/forwarding.md#penultimate-hop-popping) | `Rfc3031PenultimateHopPopping` | PASS |  |

### RFC 3032

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC3032-ENC-1](../../standard/rfc3032/catalog.md#rfc3032-enc-1) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-2](../../standard/rfc3032/catalog.md#rfc3032-enc-2) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-3](../../standard/rfc3032/catalog.md#rfc3032-enc-3) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-4](../../standard/rfc3032/catalog.md#rfc3032-enc-4) | covered | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-5](../../standard/rfc3032/catalog.md#rfc3032-enc-5) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-6](../../standard/rfc3032/catalog.md#rfc3032-enc-6) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-7](../../standard/rfc3032/catalog.md#rfc3032-enc-7) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-8](../../standard/rfc3032/catalog.md#rfc3032-enc-8) | selected | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | `Rfc3032TwoLabelStack` | PASS |  |
| [RFC3032-ENC-9](../../standard/rfc3032/catalog.md#rfc3032-enc-9) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC3032-ENC-10](../../standard/rfc3032/catalog.md#rfc3032-enc-10) | selected | [A label stack of two entries](../../protocol/mpls/checks/encoding.md#a-label-stack-of-two-entries) | `Rfc3032TwoLabelStack` | PASS |  |
| [RFC3032-ENC-11](../../standard/rfc3032/catalog.md#rfc3032-enc-11) | selected | [The IPv4 Explicit NULL label](../../protocol/mpls/checks/reserved-labels.md#the-ipv4-explicit-null-label) | `Rfc3032ExplicitNull` | PASS |  |
| [RFC3032-ENC-12](../../standard/rfc3032/catalog.md#rfc3032-enc-12) | owed | — | — | — | the Router Alert label, which `MplsPacket.msg:16` names: local software in the LSR, and a rule for it |
| [RFC3032-ENC-13](../../standard/rfc3032/catalog.md#rfc3032-enc-13) | owed | — | — | — | the Router Alert label, which `MplsPacket.msg:16` names: local software in the LSR, and a rule for it |
| [RFC3032-ENC-14](../../standard/rfc3032/catalog.md#rfc3032-enc-14) | no check | — | — | — | not claimed: the model labels IPv4 only (`Mpls.cc:69-73`), and the User's Guide names plain IPv4 |
| [RFC3032-ENC-15](../../standard/rfc3032/catalog.md#rfc3032-enc-15) | selected | [The Implicit NULL label](../../protocol/mpls/checks/reserved-labels.md#the-implicit-null-label) | `Rfc3032ImplicitNull` | PASS |  |
| [RFC3032-ENC-16](../../standard/rfc3032/catalog.md#rfc3032-enc-16) | no check | — | — | — | label distribution, a protocol of its own (LDP, RSVP-TE) out of the in-scope set |
| [RFC3032-NLP-1](../../standard/rfc3032/catalog.md#rfc3032-nlp-1) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3032-NLP-2](../../standard/rfc3032/catalog.md#rfc3032-nlp-2) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3032-NLP-3](../../standard/rfc3032/catalog.md#rfc3032-nlp-3) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3032-NLP-4](../../standard/rfc3032/catalog.md#rfc3032-nlp-4) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3032-NLP-5](../../standard/rfc3032/catalog.md#rfc3032-nlp-5) | selected | [Label switching along an LSP](../../protocol/mpls/checks/forwarding.md#label-switching-along-an-lsp) | `Rfc3031LabelSwitching` | PASS |  |
| [RFC3032-NLP-6](../../standard/rfc3032/catalog.md#rfc3032-nlp-6) | later | — | — | — | level 3: an ICMP message about a labeled packet (RFC 3032 §2.3) |
| [RFC3032-NLP-7](../../standard/rfc3032/catalog.md#rfc3032-nlp-7) | later | — | — | — | level 3: a packet that no binding covers |
| [RFC3032-ICMP-1](../../standard/rfc3032/catalog.md#rfc3032-icmp-1) | later | — | — | — | level 3: an ICMP message about a labeled packet (RFC 3032 §2.3) |
| [RFC3032-ICMP-2](../../standard/rfc3032/catalog.md#rfc3032-icmp-2) | later | — | — | — | level 3: an ICMP message about a labeled packet (RFC 3032 §2.3) |
| [RFC3032-ICMP-3](../../standard/rfc3032/catalog.md#rfc3032-icmp-3) | later | — | — | — | level 3: an ICMP message about a labeled packet (RFC 3032 §2.3) |
| [RFC3032-ICMP-4](../../standard/rfc3032/catalog.md#rfc3032-icmp-4) | later | — | — | — | level 3: an ICMP message about a labeled packet (RFC 3032 §2.3) |
| [RFC3032-ICMP-5](../../standard/rfc3032/catalog.md#rfc3032-icmp-5) | later | — | — | — | level 3: an ICMP message about a labeled packet (RFC 3032 §2.3) |
| [RFC3032-ICMP-6](../../standard/rfc3032/catalog.md#rfc3032-icmp-6) | later | — | — | — | level 3: an ICMP message about a labeled packet (RFC 3032 §2.3) |
| [RFC3032-TTL-1](../../standard/rfc3032/catalog.md#rfc3032-ttl-1) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3032-TTL-2](../../standard/rfc3032/catalog.md#rfc3032-ttl-2) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3032-TTL-3](../../standard/rfc3032/catalog.md#rfc3032-ttl-3) | selected | [A TTL that reaches zero](../../protocol/mpls/checks/ttl.md#a-ttl-that-reaches-zero) | `Rfc3032TtlExpiry` | PASS |  |
| [RFC3032-TTL-4](../../standard/rfc3032/catalog.md#rfc3032-ttl-4) | covered | [A TTL that reaches zero](../../protocol/mpls/checks/ttl.md#a-ttl-that-reaches-zero) | `Rfc3032TtlExpiry` | PASS |  |
| [RFC3032-TTL-5](../../standard/rfc3032/catalog.md#rfc3032-ttl-5) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3032-TTL-6](../../standard/rfc3032/catalog.md#rfc3032-ttl-6) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3032-TTL-7](../../standard/rfc3032/catalog.md#rfc3032-ttl-7) | covered | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlWithPush` | PASS | the check reads the top entry only, as the statement allows |
| [RFC3032-TTL-8](../../standard/rfc3032/catalog.md#rfc3032-ttl-8) | covered | [The TTL of the first label](../../protocol/mpls/checks/ttl.md#the-ttl-of-the-first-label) | `Rfc3032FirstLabelTtl` | PASS |  |
| [RFC3032-TTL-9](../../standard/rfc3032/catalog.md#rfc3032-ttl-9) | selected | [The TTL of the first label](../../protocol/mpls/checks/ttl.md#the-ttl-of-the-first-label) | `Rfc3032FirstLabelTtl` | PASS |  |
| [RFC3032-TTL-10](../../standard/rfc3032/catalog.md#rfc3032-ttl-10) | selected | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3032-FRAG-1](../../standard/rfc3032/catalog.md#rfc3032-frag-1) | covered | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS |  |
| [RFC3032-FRAG-2](../../standard/rfc3032/catalog.md#rfc3032-frag-2) | no check | — | — | — | not claimed: no parameter for the Maximum Initially Labeled IP Datagram Size |
| [RFC3032-FRAG-3](../../standard/rfc3032/catalog.md#rfc3032-frag-3) | no check | — | — | — | not claimed: no parameter for the Maximum Initially Labeled IP Datagram Size |
| [RFC3032-FRAG-4](../../standard/rfc3032/catalog.md#rfc3032-frag-4) | no check | — | — | — | not claimed: no parameter for the Maximum Initially Labeled IP Datagram Size |
| [RFC3032-FRAG-5](../../standard/rfc3032/catalog.md#rfc3032-frag-5) | no check | — | — | — | not claimed: no parameter for the Maximum Initially Labeled IP Datagram Size |
| [RFC3032-FRAG-6](../../standard/rfc3032/catalog.md#rfc3032-frag-6) | no check | — | — | — | not claimed: no parameter for the Maximum Initially Labeled IP Datagram Size |
| [RFC3032-FRAG-7](../../standard/rfc3032/catalog.md#rfc3032-frag-7) | covered | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS | with no weight: a permission that the model does not use; it fragments |
| [RFC3032-FRAG-8](../../standard/rfc3032/catalog.md#rfc3032-frag-8) | selected | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS |  |
| [RFC3032-FRAG-9](../../standard/rfc3032/catalog.md#rfc3032-frag-9) | selected | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS |  |
| [RFC3032-FRAG-10](../../standard/rfc3032/catalog.md#rfc3032-frag-10) | covered | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS | with no weight: a permission that the model does not use; it fragments |
| [RFC3032-FRAG-11](../../standard/rfc3032/catalog.md#rfc3032-frag-11) | selected | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS |  |
| [RFC3032-FRAG-12](../../standard/rfc3032/catalog.md#rfc3032-frag-12) | selected | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS |  |
| [RFC3032-FRAG-13](../../standard/rfc3032/catalog.md#rfc3032-frag-13) | selected | [A too-big labeled datagram that may not be fragmented](../../protocol/mpls/checks/fragmentation.md#a-too-big-labeled-datagram-that-may-not-be-fragmented) | `Rfc3032TooBigDontFragment`, `Rfc3032TooBigIcmp` | PASS |  |
| [RFC3032-FRAG-14](../../standard/rfc3032/catalog.md#rfc3032-frag-14) | selected | [A too-big labeled datagram that may not be fragmented](../../protocol/mpls/checks/fragmentation.md#a-too-big-labeled-datagram-that-may-not-be-fragmented) | `Rfc3032TooBigDontFragment`, `Rfc3032TooBigIcmp` | PASS |  |
| [RFC3032-FRAG-15](../../standard/rfc3032/catalog.md#rfc3032-frag-15) | selected | [A too-big labeled datagram that may not be fragmented](../../protocol/mpls/checks/fragmentation.md#a-too-big-labeled-datagram-that-may-not-be-fragmented) | `Rfc3032TooBigDontFragment`, `Rfc3032TooBigIcmp` | PASS |  |
| [RFC3032-FRAG-16](../../standard/rfc3032/catalog.md#rfc3032-frag-16) | no check | — | — | — | not claimed: the model labels IPv4 only (`Mpls.cc:69-73`), and the User's Guide names plain IPv4 |
| [RFC3032-FRAG-17](../../standard/rfc3032/catalog.md#rfc3032-frag-17) | no check | — | — | — | not claimed: the model labels IPv4 only (`Mpls.cc:69-73`), and the User's Guide names plain IPv4 |
| [RFC3032-FRAG-18](../../standard/rfc3032/catalog.md#rfc3032-frag-18) | no check | — | — | — | not claimed: the model labels IPv4 only (`Mpls.cc:69-73`), and the User's Guide names plain IPv4 |
| [RFC3032-FRAG-19](../../standard/rfc3032/catalog.md#rfc3032-frag-19) | no check | — | — | — | not claimed: the model labels IPv4 only (`Mpls.cc:69-73`), and the User's Guide names plain IPv4 |
| [RFC3032-FRAG-20](../../standard/rfc3032/catalog.md#rfc3032-frag-20) | later | — | — | — | level 5: a tunnel (RFC 3031 §3.27) |
| [RFC3032-FRAG-21](../../standard/rfc3032/catalog.md#rfc3032-frag-21) | later | — | — | — | level 5: a tunnel (RFC 3031 §3.27) |
| [RFC3032-PPP-1](../../standard/rfc3032/catalog.md#rfc3032-ppp-1) | selected | [The MPLS Control Protocol](../../protocol/mpls/checks/links.md#the-mpls-control-protocol) | `Rfc3032PppMplscp` | FAIL (expected) | observation 2: no frame of the PPP Protocol 8281 hex before the first labeled packet, [gap 7](results.md#gap-7-unimplemented-feature--no-mpls-control-protocol) |
| [RFC3032-PPP-2](../../standard/rfc3032/catalog.md#rfc3032-ppp-2) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-3](../../standard/rfc3032/catalog.md#rfc3032-ppp-3) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-4](../../standard/rfc3032/catalog.md#rfc3032-ppp-4) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-5](../../standard/rfc3032/catalog.md#rfc3032-ppp-5) | selected | [The MPLS Control Protocol](../../protocol/mpls/checks/links.md#the-mpls-control-protocol) | `Rfc3032PppMplscp` | FAIL (expected) | observation 2: no frame of the PPP Protocol 8281 hex before the first labeled packet, [gap 7](results.md#gap-7-unimplemented-feature--no-mpls-control-protocol) |
| [RFC3032-PPP-6](../../standard/rfc3032/catalog.md#rfc3032-ppp-6) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-7](../../standard/rfc3032/catalog.md#rfc3032-ppp-7) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-8](../../standard/rfc3032/catalog.md#rfc3032-ppp-8) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-9](../../standard/rfc3032/catalog.md#rfc3032-ppp-9) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-10](../../standard/rfc3032/catalog.md#rfc3032-ppp-10) | later | — | — | — | level 3 for a wrong MPLS Control Protocol packet, level 4 for a timer |
| [RFC3032-PPP-11](../../standard/rfc3032/catalog.md#rfc3032-ppp-11) | selected | [The MPLS Control Protocol](../../protocol/mpls/checks/links.md#the-mpls-control-protocol) | `Rfc3032PppMplscp` | FAIL (expected) | observation 2: no frame of the PPP Protocol 8281 hex before the first labeled packet, [gap 7](results.md#gap-7-unimplemented-feature--no-mpls-control-protocol) |
| [RFC3032-PPP-12](../../standard/rfc3032/catalog.md#rfc3032-ppp-12) | selected | [Labeled packets on a PPP link](../../protocol/mpls/checks/links.md#labeled-packets-on-a-ppp-link) | `Rfc3032PppEncapsulation` | PASS |  |
| [RFC3032-PPP-13](../../standard/rfc3032/catalog.md#rfc3032-ppp-13) | selected | [Labeled packets on a PPP link](../../protocol/mpls/checks/links.md#labeled-packets-on-a-ppp-link) | `Rfc3032PppEncapsulation` | PASS |  |
| [RFC3032-PPP-14](../../standard/rfc3032/catalog.md#rfc3032-ppp-14) | no check | — | — | — | not claimed: multicast, and the LLC/SNAP encapsulation |
| [RFC3032-PPP-15](../../standard/rfc3032/catalog.md#rfc3032-ppp-15) | covered | [A labeled datagram too big for the next link](../../protocol/mpls/checks/fragmentation.md#a-labeled-datagram-too-big-for-the-next-link) | `Rfc3032TooBigFragments` | PASS |  |
| [RFC3032-PPP-16](../../standard/rfc3032/catalog.md#rfc3032-ppp-16) | selected | [Labeled packets on a PPP link](../../protocol/mpls/checks/links.md#labeled-packets-on-a-ppp-link) | `Rfc3032PppEncapsulation` | PASS |  |
| [RFC3032-LAN-1](../../standard/rfc3032/catalog.md#rfc3032-lan-1) | selected | [Labeled packets on an Ethernet link](../../protocol/mpls/checks/links.md#labeled-packets-on-an-ethernet-link) | `Rfc3032EthernetEncapsulation` | PASS |  |
| [RFC3032-LAN-2](../../standard/rfc3032/catalog.md#rfc3032-lan-2) | selected | [Labeled packets on an Ethernet link](../../protocol/mpls/checks/links.md#labeled-packets-on-an-ethernet-link) | `Rfc3032EthernetEncapsulation` | PASS |  |
| [RFC3032-LAN-3](../../standard/rfc3032/catalog.md#rfc3032-lan-3) | selected | [Labeled packets on an Ethernet link](../../protocol/mpls/checks/links.md#labeled-packets-on-an-ethernet-link) | `Rfc3032EthernetEncapsulation` | PASS |  |
| [RFC3032-LAN-4](../../standard/rfc3032/catalog.md#rfc3032-lan-4) | no check | — | — | — | not claimed: multicast, and the LLC/SNAP encapsulation |
| [RFC3032-LAN-5](../../standard/rfc3032/catalog.md#rfc3032-lan-5) | no check | — | — | — | not claimed: multicast, and the LLC/SNAP encapsulation |

### RFC 3443

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC3443-TERM-1](../../standard/rfc3443/catalog.md#rfc3443-term-1) | covered | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3443-TERM-2](../../standard/rfc3443/catalog.md#rfc3443-term-2) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3443-TERM-3](../../standard/rfc3443/catalog.md#rfc3443-term-3) | selected | [A TTL that reaches zero](../../protocol/mpls/checks/ttl.md#a-ttl-that-reaches-zero) | `Rfc3032TtlExpiry` | PASS |  |
| [RFC3443-TERM-4](../../standard/rfc3443/catalog.md#rfc3443-term-4) | covered | [A TTL that reaches zero](../../protocol/mpls/checks/ttl.md#a-ttl-that-reaches-zero) | `Rfc3032TtlExpiry` | PASS |  |
| [RFC3443-MOD-1](../../standard/rfc3443/catalog.md#rfc3443-mod-1) | selected | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3443-MOD-2](../../standard/rfc3443/catalog.md#rfc3443-mod-2) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |
| [RFC3443-MOD-3](../../standard/rfc3443/catalog.md#rfc3443-mod-3) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |
| [RFC3443-MOD-4](../../standard/rfc3443/catalog.md#rfc3443-mod-4) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |
| [RFC3443-ITTL-1](../../standard/rfc3443/catalog.md#rfc3443-ittl-1) | covered | [The TTL of the first label](../../protocol/mpls/checks/ttl.md#the-ttl-of-the-first-label) | `Rfc3032FirstLabelTtl` | PASS |  |
| [RFC3443-ITTL-2](../../standard/rfc3443/catalog.md#rfc3443-ittl-2) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3443-ITTL-3](../../standard/rfc3443/catalog.md#rfc3443-ittl-3) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |
| [RFC3443-ITTL-4](../../standard/rfc3443/catalog.md#rfc3443-ittl-4) | selected | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3443-ITTL-5](../../standard/rfc3443/catalog.md#rfc3443-ittl-5) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3443-OTTL-1](../../standard/rfc3443/catalog.md#rfc3443-ottl-1) | selected | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3443-OTTL-2](../../standard/rfc3443/catalog.md#rfc3443-ottl-2) | selected | [The TTL at each LSR](../../protocol/mpls/checks/ttl.md#the-ttl-at-each-lsr) | `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`, `Rfc3443TtlAfterTwoPops` | PASS |  |
| [RFC3443-OTTL-3](../../standard/rfc3443/catalog.md#rfc3443-ottl-3) | covered | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3443-OTTL-4](../../standard/rfc3443/catalog.md#rfc3443-ottl-4) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |
| [RFC3443-OTTL-5](../../standard/rfc3443/catalog.md#rfc3443-ottl-5) | selected | [The TTL after the last label](../../protocol/mpls/checks/ttl.md#the-ttl-after-the-last-label) | `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate` | PASS |  |
| [RFC3443-PUSH-1](../../standard/rfc3443/catalog.md#rfc3443-push-1) | selected | [The TTL of the first label](../../protocol/mpls/checks/ttl.md#the-ttl-of-the-first-label) | `Rfc3032FirstLabelTtl` | PASS |  |
| [RFC3443-PUSH-2](../../standard/rfc3443/catalog.md#rfc3443-push-2) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |
| [RFC3443-PUSH-3](../../standard/rfc3443/catalog.md#rfc3443-push-3) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |
| [RFC3443-IMPL-1](../../standard/rfc3443/catalog.md#rfc3443-impl-1) | no check | — | — | — | a permission that no observation can fail |
| [RFC3443-IMPL-2](../../standard/rfc3443/catalog.md#rfc3443-impl-2) | covered | [A TTL that reaches zero](../../protocol/mpls/checks/ttl.md#a-ttl-that-reaches-zero) | `Rfc3032TtlExpiry` | PASS | observation 3: the TTL stops at zero, and the LSR discards the packet |
| [RFC3443-IMPL-3](../../standard/rfc3443/catalog.md#rfc3443-impl-3) | no check | — | — | — | not claimed: the Pipe and the Short Pipe Models, an optional TTL treatment |

### RFC 5462

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC5462-TC-1](../../standard/rfc5462/catalog.md#rfc5462-tc-1) | selected | [The label stack entry](../../protocol/mpls/checks/encoding.md#the-label-stack-entry) | `Rfc3032LabelStackEntry` | PASS |  |
| [RFC5462-TC-2](../../standard/rfc5462/catalog.md#rfc5462-tc-2) | no check | — | — | — | a permission: a QoS or an ECN use of the Traffic Class field (RFC 3270 and RFC 5129 are level 5) |
| [RFC5462-TC-3](../../standard/rfc5462/catalog.md#rfc5462-tc-3) | no check | — | — | — | a permission: a QoS or an ECN use of the Traffic Class field (RFC 3270 and RFC 5129 are level 5) |

## The coverage debt: the checks this pass owes

Two statements are `owed`: the model claims the behavior, and no check of this pass reaches it.
The field comment of `MplsHeader` says that the label 1 "represents the router alert label"
(`MplsPacket.msg:16`), and RFC 3032 §2.1 gives that label its meaning: the packet goes to local
software in the LSR, and the label comes back on top when the packet goes on. RFC 3032 names no
such software and no rule for what it does, so a check needs both from the document of a
protocol that uses the label.

| Statement | Strength | What a check needs |
| --- | --- | --- |
| [RFC3032-ENC-12](../../standard/rfc3032/catalog.md#rfc3032-enc-12) | description | local software in the LSR that takes a packet with the label 1 at the top, and a rule for what it does |
| [RFC3032-ENC-13](../../standard/rfc3032/catalog.md#rfc3032-enc-13) | should (lower case) | local software in the LSR that takes a packet with the label 1 at the top, and a rule for what it does |

The `later` statements are not debt of this level: 23 need a label that no binding covers,
an ICMP message about a labeled packet, a second path, a wrong control packet, a timer or a
tunnel. The 33 `no check` statements are behaviors that the model does not claim — label
distribution, a link without a TTL field, a labeled IPv6 datagram, the Maximum Initially Labeled
IP Datagram Size, multicast, and the Pipe and Short Pipe Models — and permissions that no
observation can fail.

## Feature support

The rule of the guide: `supported` when every core statement passed, `partial` when some passed
and some failed or have no check, `not supported` when every core statement that ran failed,
`untested` when no core statement has a check.

| Feature | Level | Support | Why |
| --- | --- | --- | --- |
| [MPLS-F-LABEL-STACK-ENCODING](../../protocol/mpls/features.md#mpls-f-label-stack-encoding) | mandatory | supported | every core statement passed |
| [MPLS-F-RESERVED-LABELS](../../protocol/mpls/features.md#mpls-f-reserved-labels) | mandatory | partial | 2 core statements passed, 0 failed, 2 have no check |
| [MPLS-F-LABEL-FORWARDING](../../protocol/mpls/features.md#mpls-f-label-forwarding) | mandatory | supported | every core statement passed |
| [MPLS-F-INGRESS-LABELING](../../protocol/mpls/features.md#mpls-f-ingress-labeling) | mandatory | supported | every core statement passed |
| [MPLS-F-EGRESS-DECAPSULATION](../../protocol/mpls/features.md#mpls-f-egress-decapsulation) | mandatory | supported | every core statement passed |
| [MPLS-F-PENULTIMATE-HOP-POPPING](../../protocol/mpls/features.md#mpls-f-penultimate-hop-popping) | mandatory | supported | every core statement passed |
| [MPLS-F-TTL](../../protocol/mpls/features.md#mpls-f-ttl) | mandatory | supported | every core statement passed |
| [MPLS-F-PIPE-MODELS](../../protocol/mpls/features.md#mpls-f-pipe-models) | optional | untested | no core statement has a check: no check |
| [MPLS-F-TRAFFIC-CLASS](../../protocol/mpls/features.md#mpls-f-traffic-class) | optional | untested | no core statement has a check: no check |
| [MPLS-F-INVALID-LABEL](../../protocol/mpls/features.md#mpls-f-invalid-label) | mandatory | untested | no core statement has a check: later |
| [MPLS-F-ICMP](../../protocol/mpls/features.md#mpls-f-icmp) | optional | untested | no core statement has a check: later |
| [MPLS-F-FRAGMENTATION](../../protocol/mpls/features.md#mpls-f-fragmentation) | mandatory | partial | 5 core statements passed, 0 failed, 2 have no check |
| [MPLS-F-PPP](../../protocol/mpls/features.md#mpls-f-ppp) | mandatory | partial | 3 core statements passed, 2 failed |
| [MPLS-F-LAN](../../protocol/mpls/features.md#mpls-f-lan) | mandatory | supported | every core statement passed |

- **Since the repairs of 2026-09-25**, MPLS-F-TTL and MPLS-F-LAN are supported; in the level 2
  run they were partial and not supported ([gaps 1 to 4](results.md#the-model-gaps)).
- **MPLS-F-RESERVED-LABELS and MPLS-F-FRAGMENTATION are partial only by statements without a
  check**: the IPv6 Explicit NULL label and the IPv6 rules of fragmentation, which the model does
  not claim, and the reserved values 4 to 15, which belong to label distribution.
- **MPLS-F-PPP is partial by the MPLS Control Protocol**
  ([gap 7](results.md#gap-7-unimplemented-feature--no-mpls-control-protocol)).
- **MPLS-F-INVALID-LABEL is untested at this level**: its two core statements are level 3 by the
  standards map, as [`checks.md`](../../protocol/mpls/checks.md#statements-this-pass-wrote-no-check-for) says.

## Achieved level

**Level 2, reached, for the normal path.** Target: level 2, from
[`standards.md`](../../protocol/mpls/standards.md#target-level).

| Half of the criterion | State | Evidence |
| --- | --- | --- |
| Every normal-path mandatory mechanism of the base documents appears as a feature | holds | the catalogs hold every normative statement of the in-scope sections of RFC 3031, RFC 3032, RFC 3443 and RFC 5462, 157 entries; the feature map has 14 features and places every entry |
| Every mandatory feature has a core check that ran and has a verdict | holds for the normal path | 10 of the 11 mandatory features have core checks that ran: 21 tests, 20 PASS, 1 FAIL (expected). The eleventh, the discard of a label without a binding, is level 3 by the standards map |

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | **reached** | the 2 `owed` statements are the debt of the level; see [the coverage debt](#the-coverage-debt-the-checks-this-pass-owes) |
| 3, Edge | not started | a label without a binding (RFC 3031 §3.18, §3.22), a pop of an unlabeled packet, the ICMP message of RFC 3032 §2.3, a second NHLFE, and wrong MPLS Control Protocol packets |
| 4, Dynamics | not started | the timers of the MPLS Control Protocol; RFC 3031 states no timer of its own |
| 5, Complete | not started | RFC 3031 §3.20, §3.26, §3.27, and RFC 3270, RFC 5129, RFC 5332, RFC 5586, RFC 6790, RFC 7274 |

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 3031, RFC 3032, RFC 3443, RFC 5462 downloaded; the standards map; the claims of the model | no run; zero claims of any kind, and one verified TTL defect found while checking the claim question |
| 2 | 2026-09-24 | **2, reached** | catalogs of RFC 3031 (49), RFC 3032 (81), RFC 3443 (24) and RFC 5462 (3); 14 features; 15 checks; 21 tests; the conformance matrix | 5 PASS, 16 FAIL, 6 of them declared expected; seven gaps of the model: four defects and three unimplemented features; 2 statements owed |
| 2, repairs | 2026-09-25 | **2, reached** | six gaps repaired on `topic/standards-tests-mpls-level2-fixes`: the TTL (gaps 1 to 3), Ethernet (gap 4), the reserved labels (gap 5), fragmentation (gap 6); gap 7 has a plan | 20 PASS, 1 FAIL declared expected (the MPLS Control Protocol); no fingerprint and no statistical result moves; 2 statements owed |

## Out of scope

The sections of the four documents outside the in-scope set are listed at the end of each
catalog: [RFC 3031](../../standard/rfc3031/catalog.md#out-of-scope-in-this-catalog),
[RFC 3032](../../standard/rfc3032/catalog.md#out-of-scope-in-this-catalog),
[RFC 3443](../../standard/rfc3443/catalog.md#out-of-scope-in-this-catalog) and
[RFC 5462](../../standard/rfc5462/catalog.md#out-of-scope-in-this-catalog).
