# RFC 791 checks — run results and model analysis (pass 1)

> **Kind:** report · **Status:** snapshot 2026-09-02 · **Seal:** none · **Owns:** — · **Stands on:** [rfc791-checklist.md](rfc791-checklist.md), [rfc792-checklist.md](rfc792-checklist.md), [rfc791-checks.md](rfc791-checks.md)

Step 7 artifact of the standards test workflow (see
[derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)).
This is the first document of the workflow that may reference code.

- Date: 2026-09-02. Tree: inet-master (branch `master`, docs at commit `6208e77255`,
  model library built 2026-09-01).
- Command:

  ```sh
  inet_run_protocol_tests -p inet -w ipv4
  ```

  (`-p inet` skips the project discovery, which crashes on a `~/.omnetpp` directory.)

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc791TtlDecrement.test | RFC791-TTL-1, RFC791-TTL-3 | PASS |
| Rfc791FragmentReassembly.test | RFC791-FRAG-1..4, RFC791-REASM-1 | PASS |
| Rfc791DontFragment.test | RFC791-FRAG-5, RFC792-DU-4 | PASS |
| Fragmentation.test (pre-existing) | — | PASS |

The INET model conforms to all selected statements. No `%# expected-result: FAIL` marker
was necessary in this pass.

## Failure history during authoring

Both initial failures were test errors, not model gaps. The classification matters for the
workflow: fix the test, not the expectation.

1. **Wrong MTU parameter path.** The first run used `*.router.eth[1].mtu = 576B`. The key
   matched nothing, OMNeT++ applied the default 1500-octet MTU, and the router forwarded
   the 1028-octet datagram in one piece. The MTU parameter lives on the MAC module:
   `*.router.eth[1].mac.mtu = 576B`
   ([EthernetMacPhy.ned:126](../../../../../src/inet/linklayer/ethernet/basic/EthernetMacPhy.ned#L126)).
   Lesson: a scenario knob that fails silently voids the stimulus. The fragment test caught
   it because its second step demands a fragment; a weaker test would have passed for the
   wrong reason.
2. **Filter details in the packet expressions.**
   - A unit-bearing field needs a unit literal: `udp.totalLengthField == 1008B`, not
     `== 1008`.
   - The dissector protocol name for ICMP is `icmpv4`, not `icmp`:
     `icmpv4.type == 3 && icmpv4.code == 4`.
   In both cases the wrong expression is a silent non-match, and the step times out.
   Lesson for step 6: on a deadline miss, first print the frames (a tester without
   `testName` logs the trace) and compare the field spellings.

## Model analysis — where INET implements the checked behavior

- **TTL decrement (RFC791-TTL-1):** the forward path decrements once per hop —
  [Ipv4.cc:323](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L323). Observed: 32 on
  link 1, 31 on link 2 and at host B.
- **TTL zero handling (RFC791-TTL-2, candidate):**
  [Ipv4.cc:943-951](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L943-L951) drops and
  sends ICMP time exceeded. A future test can reuse the mockup with TTL 1.
- **Fragmentation (RFC791-FRAG-2..4):**
  [Ipv4.cc:931](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L931) `fragmentAndSend`;
  line 990 computes `fragmentLength = ((mtu - headerLength) / 8) * 8` — exactly the RFC
  8-octet rule. Observed: fragments at octet offsets 0 and 552 with one identification
  value, MF pattern true/false.
- **DF discard with report (RFC791-FRAG-5, RFC792-DU-4):**
  [Ipv4.cc:976-985](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L976-L985) discards and
  calls `icmp->sendPtbMessage(packet, mtu)`. Observed: an `IcmpPtb` chunk with type 3,
  code 4 toward host A. The chunk also carries the next-hop MTU (576) — that is the RFC
  1191 extension of the field that RFC 792 leaves unused. A catalog entry for RFC 1191 can
  pick this up in a later pass.
- **Reassembly (RFC791-REASM-1):**
  [Ipv4.cc:819-852](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L819-L852)
  `reassembleAndDeliver` with the fragment buffer. Observed: host B's UDP received one
  1008-octet datagram; the sink application received the 1000-octet payload.

## Feature support derived from the verdicts

Step 7 closes with the update of the support column of [`features.md`](features.md). The
derivation is mechanical: a feature is `supported` when every core check ran and passed,
`partial` when a core check passed and another core check failed as a model gap or did not
run, `not supported` when every core check that ran failed as a model gap, and `untested`
when no core check ran.

| Feature | Core checks and their state | Support |
| --- | --- | --- |
| IPV4-F-TTL | RFC791-TTL-1 PASS; RFC791-TTL-2 did not run | partial |
| IPV4-F-FRAGMENTATION | RFC791-FRAG-1..4 all PASS | supported |
| IPV4-F-REASSEMBLY | RFC791-REASM-1 PASS | supported |
| IPV4-F-DONT-FRAGMENT | RFC791-FRAG-5 PASS | supported |
| IPV4-F-IDENTIFICATION | RFC791-FRAG-3 PASS; RFC791-ID-1 did not run | partial |
| IPV4-F-HEADER-CHECKSUM | neither core check ran | untested |
| IPV4-F-MIN-SIZE | neither core check ran | untested |
| IPV4-F-ERROR-REPORT | RFC792-DU-4 PASS; RFC792-TE-1 did not run | partial |

No feature is `not supported`, because no test failed against the model. Every `partial`
and every `untested` value in this pass comes from a missing check, not from a model gap.

Two bounds on these values:

1. A `supported` feature is supported as far as its checks reach. IPV4-F-FRAGMENTATION
   rests on one datagram size and one MTU.
2. An observation that no check claims does not raise a support value. The run carried a
   1008-octet datagram end to end, which exceeds the 576-octet floor of IPV4-F-MIN-SIZE,
   yet that feature stays `untested`.

## Sharpening candidates for the next pass

- **Exactly-two-fragments gap.** The final `never` step opens after the reassembled
  delivery, some tens of microseconds after the last fragment left the router. An extra
  fragment inside that gap would escape. A standing negative rule (or a scope-strict mode
  over a window that runs beside later steps) would close it; consider a framework
  extension.
- Assert the fragment total lengths (572 B and 476 B) with unit literals.
- Assert that the ICMP report embeds the original internet header (RFC 792 requires the
  header plus 64 bits of data); the trace shows INET embeds it.
- Add the TTL-zero test (RFC791-TTL-2 + RFC792-TE-1) on the same mockup with TTL 1.
- Pair RFC791-CKSUM-1 with the TTL scenario: the checksum must change across the hop.
