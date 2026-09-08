# IPv4 checks — run results and model analysis (pass 2, level 2)

> **Kind:** report · **Status:** snapshot 2026-09-08 · **Seal:** none · **Owns:** — · **Stands on:** [rfc791/catalog.md](../../standard/rfc791/catalog.md), [rfc792/catalog.md](../../standard/rfc792/catalog.md), [checks.md](../../protocol/ipv4/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the IPv4
workflow that may reference code.

- Date: 2026-09-08. Tree: `inet-rfc-tests-ipv4`, branch `topic/rfc-tests-ipv4`, commit
  `63ef8b0322`. The IPv4 source is identical to `master`.
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w ipv4
  ```

  (`-p inet` skips the project discovery, which crashes on a `~/.omnetpp` directory.
  `build.sh` defaults to release mode and then fails to find the debug INET library;
  `MODE=debug` is needed.)

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc791DatagramDelivery.test | RFC791-FWD-1, DLV-1, PROTO-1, HDR-1, HDR-2, HDR-3 | PASS |
| Rfc791TtlDecrement.test | RFC791-TTL-1, TTL-3, CKSUM-1 | PASS |
| Rfc791TtlExpiry.test | RFC791-TTL-2, RFC792-TE-1 | PASS |
| Rfc791FragmentReassembly.test | RFC791-FRAG-1..4, REASM-1 | PASS |
| Rfc791DontFragment.test | RFC791-FRAG-5, RFC792-DU-4 | PASS |
| Rfc791Identification.test | RFC791-ID-1 | PASS |
| Rfc791MinimumSizes.test | RFC791-FRAG-6, REASM-2 | PASS |
| Fragmentation.test (pre-existing) | — | PASS |

Summary: 8 PASS in 2.0 s. The model conforms to every selected statement. No
`%# expected-result: FAIL` marker was necessary.

## Deviations between the English observations and the test steps

Each one is also stated in the `%description` of its test. None weakens an expectation.

1. **TTL decrement, observations 2 and 4 in one step.** The TTL decrease and the changed
   checksum are two properties of the one datagram on link 2. The engine consumes an event
   once, so a later step cannot re-examine it; the two observations are one step with two
   conditions.
2. **Datagram delivery, observation 2, address comparison by predicate.** A `.packet()`
   expression cannot compare two addresses by value: the filter engine holds an address as
   an opaque pointer and compares pointers by identity. The step reads both addresses with
   a typed predicate and compares their text forms.
3. **Datagram delivery, observation 4, the program's receipt observed at UDP.** The sink
   application emits only `packetReceived`, which the tester does not turn into an event.
   The step observes UDP's upward handoff of the 100-octet payload
   (`Udp::sendUp`, `packetSentToUpper`), the last recognisable event before the program.
4. **Minimum sizes, observation 4, no UDP field on a fragment.** The IPv4 dissector does not
   descend into a fragment, so `udp.destPort` never matches one. The step selects the
   fragment by `ipv4.moreFragments == true`, which datagram 1 never carries.
5. **TTL expiry and don't fragment, the discard observed at the gateway.** A `never` step on
   link 2 that is anchored after the ICMP report arrives at host A opens too late: a
   forbidden forward leaves the gateway at the moment of the decision, before the report
   crosses link 1. Both checks therefore observe the gateway's discard record
   (`router.ipv4.ip`, `packetDropped`, filtered by the identification) right after the
   stimulus, and keep the `never` as the guard for the rest of the window. The check
   document states this order.

## Scenario settings beyond the templates

- `**.checksumMode = "computed"` in the TTL decrement test. The model's default,
  `"declared"`, writes a fixed placeholder into the checksum field and never recomputes it
  (see the model observations). The check document requires computed checksums, so this
  is scenario configuration, not a weakened assertion.
- `*.router.eth[1].mac.mtu = 68B` in the minimum-sizes test. The Ethernet MAC accepted it;
  the fragmentation arithmetic gave 48 data octets per fragment and 12 fragments for the
  576-octet datagram, as the check document computes.
- Two sender applications on host A in the minimum-sizes test, because one instance carries
  one message length.

## Failure history during authoring

Every initial failure was a test error. None was a model gap.

1. `*.ipv4.ip.checksumMode` matched nothing, because the pattern lacks the host segment
   (`<network>.<host>.ipv4.ip`). `**.checksumMode` matches. The same defect sat in the
   restored templates as `*.ipv4.arp.typename = "GlobalArp"`, which had never taken effect
   — the traces carried ARP frames throughout pass 1. All seven tests now use
   `**.ipv4.arp.typename`. Lesson, as in the guide: an ini key with a wrong path applies
   nothing, silently.
2. `udp.destPort` on a fragment: a silent non-match, found by comparing the raw packet dump
   with the tester's trace.
3. The sink's `packetReceived` signal: a deadline miss with zero candidate events.
4. Found in review, not by a failure: the late `never` anchor (deviation 5). Both tests
   passed before and after the repair; the repair changes what a pass proves, not whether it
   passes.

## Model analysis — where INET implements the checked behavior

All line numbers were re-verified on this tree. `Ipv4.cc`, `Ipv4.ned`, `Ipv4Header.msg` and
`Icmp.ned` are byte-identical to the tree of pass 1; the six IPv4 files that changed since
are serializer signature renames with no change of behavior.

- **Delivery and forwarding (FWD-1, DLV-1, PROTO-1):** the forward path decrements and
  routes ([Ipv4.cc:419](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L419)); local
  delivery resolves the protocol field and dispatches by it
  ([Ipv4.cc:909-911](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L909-L911)) and hands
  the packet to the transport gate
  ([Ipv4.cc:878-883](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L878-L883)).
- **Header fields (HDR-1..3):** the header class declares `version = 4`, `headerLength` in
  octets and `totalLengthField`
  ([Ipv4Header.msg:174-181](../../../../../src/inet/networklayer/ipv4/Ipv4Header.msg#L174-L181)).
  Observed on the wire: version 4, header length 20 B, total length 128 B for 100 octets
  of data.
- **TTL decrement (TTL-1):** once per hop,
  [Ipv4.cc:323](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L323). Observed 32 on
  link 1, 31 on link 2.
- **TTL expiry (TTL-2, RFC792-TE-1):**
  [Ipv4.cc:943-952](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L943-L952) tests
  `getTimeToLive() <= 0` after the decrement, emits the drop, and sends
  `ICMP_TIME_EXCEEDED`. Observed: the discard record at the router, type 11 code 0 at host
  A, nothing on link 2.
- **Header checksum (CKSUM-1):** recomputed in `fragmentAndSend` only when
  `checksumMode == CHECKSUM_COMPUTED`
  ([Ipv4.cc:967-970](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L967-L970)); the
  arithmetic is `Ipv4Header::updateChecksum`
  ([Ipv4Header.cc:70-95](../../../../../src/inet/networklayer/ipv4/Ipv4Header.cc#L70-L95)).
  Observed with computed mode: the two checksums differ, by 256 in the one's complement sum.
- **Fragmentation (FRAG-1..4, FRAG-6):** `fragmentAndSend` at
  [Ipv4.cc:931](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L931); the 8-octet rule
  at [Ipv4.cc:990](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L990). Observed: the
  68-octet datagram crosses the 68-octet link whole.
- **Don't fragment (FRAG-5, RFC792-DU-4):**
  [Ipv4.cc:976-985](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L976-L985) emits the
  drop and calls `icmp->sendPtbMessage(packet, mtu)`; the message carries the next-hop MTU,
  which is the RFC 1191 extension of the field RFC 792 leaves unused.
- **Reassembly (REASM-1, REASM-2):** `reassembleAndDeliver`,
  [Ipv4.cc:819-853](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L819-L853). Observed:
  twelve fragments reassembled into the 576-octet datagram at host B.
- **Identification (ID-1, FRAG-3):** one counter per module, incremented per datagram,
  [Ipv4.cc:1089](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L1089); fragments share
  the header copy ([Ipv4.cc:1017](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L1017)).

## Model observations the checks did not claim

1. **The header checksum is not computed by default.**
   [Ipv4.ned:97](../../../../../src/inet/networklayer/ipv4/Ipv4.ned#L97):
   `string checksumMode @enum("declared", "computed") = default("declared");`. In the
   default mode the field holds a placeholder and the recompute of RFC791-CKSUM-1 never
   runs. Within a simulation this is a legitimate shortcut; a test that reads the field must
   turn it off.
2. **A bad checksum alone does not cause a discard.**
   [Ipv4.cc:282](../../../../../src/inet/networklayer/ipv4/Ipv4.cc#L282):
   `if (!ipv4Header->isCorrect() && !ipv4Header->verifyChecksum())`. The checksum is
   consulted only when the header is already structurally wrong; a structurally correct
   header with a wrong checksum passes. That is RFC791-CKSUM-2, "discarded at once by the
   entity which detects the error". The check needs a corrupted datagram in flight, which
   is the level 3 toolset, so this is recorded as an observation and not judged. It is the
   first candidate for a `defect` at level 3.
3. **The identification counter is per module, not per source, destination and protocol.**
   It over-satisfies RFC791-ID-1 and under-serves RFC 6864, which is out of scope.

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger,
[`coverage.md`](coverage.md#feature-support), by the rules of step 7 and of the levels.

## Sharpening candidates for the next pass

- **Level 3 starts with RFC791-CKSUM-2** (observation 2 above), then RFC791-REASM-3, and
  RFC 1122 and RFC 6864 in the in-scope set.
- A standing negative rule in the framework, or a `never` that runs beside later steps,
  would let the two negative checks drop the discard-record step and stay on the wire.
- Assert the fragment total lengths (572 B and 476 B) with unit literals in the fragment
  check.
- A 576-octet datagram that arrives whole, for the other half of RFC791-REASM-2.
- Identification uniqueness over many datagrams, as a statistical check.
