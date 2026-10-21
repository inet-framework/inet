# Implementation plan: duplicate and corrupted frames in protocol tests

Checkout and revision: the investigation read `inet-master`, `master` at `89d600a341`. The implementation starts from `master` at `dcae92dcbf`: R1 on `topic/ethernet-keep-frame-bit-error`, steps 2 to 6 on `topic/protocol-test-frame-faults`, which starts from the tip of the R1 branch.
Plan revision: 2026-10-21, third draft. The second draft recorded D-1, D-3 and D-4, gave the details of D-2, and added the vocabulary (section 4.7) and the examples (section 4.8). The third draft records D-2 and D-5, makes `corrupt()` on a wireless link fail the reception decision, and renames the wireless tap `TapRadioMedium`.
Approval: approved by the user on 2026-10-21 ("let's implement in worktree"), for the third draft and steps 1 to 6. Before that, the user asked for this design for wired and wireless links, decided that a frame with a bit error must be discarded by the Ethernet receiver because the present behavior is a defect in Ethernet, and decided D-1 to D-5 (section 8).

## 1. Problem and intended behavior

A protocol test can drop, delay or change a frame on a wired link. It cannot do these things:

- Send a frame two times, with a set time between the two copies.
- Make a receiver get a frame with a bit error, by a method that the receiving MAC handles.
- Act on a frame on a wireless link.

These scenarios need the missing functions:

1. **IEEE 802.11 duplicate detection.** A receiver must acknowledge a repeated DATA frame that has the Retry bit, and it must not deliver the frame again. A test needs a copy of the DATA frame on the air, with the Retry bit, after the first ACK. [LegacyDuplicateRemoval.cc](../../src/inet/linklayer/ieee80211/mac/duplicateremoval/LegacyDuplicateRemoval.cc) rejects a frame only when the Retry bit is set, as the standard says. So a copy without the Retry bit is a new frame, and the receiver delivers it again.
2. **A TCP duplicate segment.** Host B gets the same segment 2 ms later. RFC 9293 section 3.10.7.4 tells host B to acknowledge it and to discard it, because the segment is not acceptable.
3. **A corrupted ACK on Wi-Fi.** The station must retransmit the DATA frame with the Retry bit. Today [WifiRetransmission.test](../../tests/protocol/wifi/common/WifiRetransmission.test) needs a MAC subclass of its own to lose one ACK.
4. **A corrupted Ethernet frame.** The receiving MAC must drop it and count it. Today the frame goes up to IPv4 (section 2.2).

The intended result is one set of rule words for wired and wireless links:

```cpp
// Wired: the second data segment arrives two times, 2 ms apart.
.intercept(tap("tap").filterPacket("tcp.destPort == 1000 && tcp.synBit == false").minBytes(100)
             .nth(2).duplicate(0.002))

// Wireless: a copy of the first DATA frame of sta1 on the air 1 ms later, with the Retry bit.
.intercept(tap("radioMedium").filterPacket("ieee80211mac.type == 32").from("sta1").nth(1).duplicate(0.001))
.intercept(tap("radioMedium").copiesOnly().mutate(setRetryBit))

// Wireless, from a point in the program: sta1 gets the next ACK with a bit error.
.meanwhile(tap("radioMedium").filterPacket("ieee80211mac.type == 29").to("sta1").nth(1).corrupt())
```

## 2. Current implementation and evidence

### 2.1 The wired tap

[PacketTap](../../tests/protocol/lib/PacketTap.cc) is a store-and-forward relay with two `inout` gates. A test puts it on a link. It holds an ordered list of rules. The first rule that matches a frame applies, and each rule counts its own occurrences ([PacketTap.cc:77-107](../../tests/protocol/lib/PacketTap.cc#L77-L107)). The actions are `drop`, `delay`, `pass` and `mutate` ([PacketTap.cc:167-197](../../tests/protocol/lib/PacketTap.cc#L167-L197)). No action sends a copy. The mutator is `void(Packet *)`, and it gets only the inner packet, not the `EthernetSignal` on the wire ([PacketTap.cc:160-165](../../tests/protocol/lib/PacketTap.cc#L160-L165)).

The tester installs the rules at startup with `configure(...)`, which has seven arguments ([ProtocolTester.cc:158-171](../../tests/protocol/lib/ProtocolTester.cc#L158-L171)). No ordered step can add a rule. The tap compiles a rule filter one time ([PacketTap.cc:39-48](../../tests/protocol/lib/PacketTap.cc#L39-L48)), so a rule cannot read a capture. A step filter can: [EventPattern.cc:227-249](../../tests/protocol/lib/EventPattern.cc#L227-L249) puts the captured values in place of `{name}` and compiles a new filter.

No test in `tests/` sets the ini parameters of the tap. The 56 test files that use a tap give their rules through the program.

### 2.2 The Ethernet defect

- [EthernetMacPhy.cc:293](../../src/inet/linklayer/ethernet/basic/EthernetMacPhy.cc#L293) and [EthernetCsmaMacPhy.cc:708](../../src/inet/linklayer/ethernet/basic/EthernetCsmaMacPhy.cc#L708) read the bit error flag of the signal only. The FCS mode is `FCS_DECLARED_CORRECT`, so the MAC accepts a frame whose packet has the flag.
- [EthernetCsmaPhy.cc:388](../../src/inet/physicallayer/wired/ethernet/EthernetCsmaPhy.cc#L388) and [PacketReceiverBase.cc:75](../../src/inet/protocolelement/transceiver/base/PacketReceiverBase.cc#L75) write the flag of the signal over the flag of the packet. `PacketReceiverBase::decodePacket` serves every layered Ethernet PHY: `PacketReceiver`, `DestreamingReceiver` and `StreamThroughReceiver`.
- Observed: the last run of `tests/protocol/self/InterceptMutate.test` has `bit error found, sending ICMP_PARAMETER_PROBLEM` on line 5079 of `tests/protocol/self/work/InterceptMutate/test.out`. [Ipv4.cc:307](../../src/inet/networklayer/ipv4/Ipv4.cc#L307) found the flag after the MAC accepted the frame. The test passes only because TCP retransmits.
- [Ipv6.cc:240](../../src/inet/networklayer/ipv6/Ipv6.cc#L240) asserts that no packet has a bit error. Its comment says that the link layer drops such frames. The defect breaks that assumption, so the same mutation on an IPv6 link stops a debug run.
- The overwrite also removes the flag that [StreamingTransmitter.cc:93](../../src/inet/protocolelement/transceiver/StreamingTransmitter.cc#L93) and [StreamThroughTransmitter.cc:146](../../src/inet/protocolelement/transceiver/StreamThroughTransmitter.cc#L146) set on an aborted frame. [PacketTransmitterBase::encodePacket](../../src/inet/protocolelement/transceiver/base/PacketTransmitterBase.cc#L85-L88) does not copy the flag into the signal. No run shows this yet.
- PPP sends the `Packet` itself on the channel and checks its flag ([Ppp.cc:270](../../src/inet/linklayer/ppp/Ppp.cc#L270)). So a flag on the object on the wire works for PPP and for `EthernetMacPhy` today.

### 2.3 The wireless path

No tap is possible on the air. These facts set the place for one:

- Every radio medium NED type uses the C++ class `RadioMedium`. [RadioMedium.ned:47](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.ned#L47) sets the class, and no derived NED type sets another one.
- [RadioMedium::transmitPacket](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L627-L638) makes the transmission and calls `sendToRadio(transmitter, receiver, signal)` for each receiver ([RadioMedium.cc:606](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L606)). The method is public and virtual.
- At the end of a reception, [Radio::endReception](../../src/inet/physicallayer/wireless/common/radio/packetlevel/Radio.cc#L514-L531) calls `receivePacket`, which is virtual and returns a copy of the packet for this receiver ([RadioMedium.cc:640-652](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L640-L652)). Then the radio sends the copy up.
- Two paths give a signal to a receiver: `sendToRadio` and [pickUpSignals](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L730-L748), for a radio that starts to receive while a frame is on the air. Both ask the virtual `isPotentialReceiver(receiver, transmission)` first ([RadioMedium.cc:609](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L609), [RadioMedium.cc:737](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L737)).
- The energy of a frame at a receiver comes through the two protected virtual overloads of `isInterferingTransmission` ([RadioMedium.cc:227-247](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L227-L247)). The overload with a listening serves the carrier sense ([RadioMedium.cc:333-340](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L333-L340)). The overload with a reception serves the SNIR of the other receptions ([RadioMedium.cc:278-288](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L278-L288)). Both get the receiver.
- `transmitPacket` calls the virtual `addTransmission` before it sends any signal ([RadioMedium.cc:632](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L632)).
- The MAC emits `packetSentToLower` and then sends the frame to the radio ([MacProtocolBase.cc:68-73](../../src/inet/linklayer/base/MacProtocolBase.cc#L68-L73)). The radio starts the transmission in a later event. So the medium sees a frame after a step matches that signal.
- The receiver sets the bit error flag of the received packet from the reception decision ([ReceiverBase.cc:133-141](../../src/inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.cc#L133-L141)). The medium gets that decision from the protected virtual `computeReceptionDecision` ([RadioMedium.cc:313-320](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L313-L320)). The radio logs the same decision ([Radio.cc:523](../../src/inet/physicallayer/wireless/common/radio/packetlevel/Radio.cc#L523)).
- The medium refuses a transmission that starts at exactly the same time as another one. The parameter `sameTransmissionStartTimeCheck` has the default `"error"` ([RadioMedium.ned:44](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.ned#L44), [RadioMedium.cc:496-514](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L496-L514)).
- The communication cache lists the radios of the medium (`mapRadios`, [RadioMedium.cc:517](../../src/inet/physicallayer/wireless/common/medium/RadioMedium.cc#L517)).
- The 802.11 receive path discards a frame with a bit error ([Rx.cc:143](../../src/inet/linklayer/ieee80211/mac/Rx.cc#L143)).
- The communication cache owns the transmitter signal and deletes it ([CommunicationCacheBase.cc:261](../../src/inet/physicallayer/wireless/common/base/packetlevel/CommunicationCacheBase.cc#L261)). [Radio::endTransmission](../../src/inet/physicallayer/wireless/common/radio/packetlevel/Radio.cc#L417-L429) does not delete it.
- The Wi-Fi suite network gives the medium a fixed type: `radioMedium: Ieee80211ScalarRadioMedium` ([WifiInfraNetwork.ned:24](../../tests/protocol/wifi/ned/WifiInfraNetwork.ned#L24)).

### 2.4 Pull request #1367

The pull request adds `TriggeredNoiseSource` and an `Interfere` step. The noise is a physical fault: the noise power and the receiver model decide the result, and the pull request says that a loss is not certain. The pull request also puts radio types into `ProtocolTester.cc`, adds two loose fields to `Step`, and uses `injectMsg` for a second step kind. This plan takes two ideas from it: a fault that starts from a matched event, and a fault that goes through the production radio medium.

## 3. Scope and acceptance criteria

### Required work

- **R1.** Ethernet receivers discard a frame when its packet or its signal has a bit error. This is a separate pull request to `master`, before the other work.
- **R2.** Two new actions, `duplicate(spacing, copies = 1)` and `corrupt()`, for wired links (`PacketTap`) and wireless links (`TapRadioMedium`).
- **R3.** New selectors: `from(node)`, `to(node)` and `copiesOnly()`, and `{name}` captures in `filterPacket`.
- **R4.** A rule that starts at a point in the program: `meanwhile(tap(...)...)`.
- **R5.** The guide and the describer show the new words.

### Acceptance criteria

- **A1.** A frame whose inner packet has the bit error flag goes to `packetDropped` of the receiving Ethernet MAC, with the reason `INCORRECTLY_RECEIVED`. This is true for `EthernetMacPhy` and for `EthernetPhyLayer`. IPv4 sends no ICMP Parameter Problem. An IPv6 variant of the test does not stop at the assert.
- **A2.** Wired `duplicate`: the receiving MAC emits `packetReceivedFromLower` two times for the same tree ID. The second time is `spacing` after the first, with exact equality, because the channel times are deterministic.
- **A3.** Wireless `duplicate`: each receiver in range starts a second reception `spacing` after the first. With `to(X)`, only X starts the second reception.
- **A4.** `corrupt()`: the receiving MAC discards the frame, on Ethernet, on PPP and on 802.11. On 802.11, the sender of the frame retransmits it with the Retry bit.
- **A5.** All 56 test files that use `PacketTap` keep their verdicts.
- **A6.** Each new word has a self test that fails when the word has no effect.
- **A7.** Wireless `drop()`: the radio of the receiver does not change its reception state while the frame is on the air. With `corrupt()` in its place, the radio goes to the receive state, and the MAC discards the frame.

### Follow-up work, not in this plan

- [Ipv4.cc:310](../../src/inet/networklayer/ipv4/Ipv4.cc#L310) divides the header length by the header length. The result is always 1, so every bit error becomes a header error. This is a separate model finding.
- The noise fault of pull request #1367, as an action of its own.
- Half-duplex Ethernet and streaming links. The tap does not forward transmission updates.
- `corrupt()` in place of the MAC subclass of `WifiRetransmission.test`.
- Intercepting NED types for other radio media (802.15.4, unit disk), when a test needs one.

### Preserved conditions

- Each action, selector and ini parameter of `PacketTap` keeps its meaning.
- Only R1 changes `src/`.

## 4. Proposed design and reasons

### 4.1 Rule semantics: one definition for both media

- The rules are ordered. The interceptor makes one decision for each pair of a frame and a receiver. The first rule whose selectors match and whose occurrence condition holds applies. If no rule applies, the frame passes unchanged.
- **A rule counts frames, not receptions.** On a wired link, a frame has one receiver, so this is the present behavior. On a wireless link, a rule without `to()` that selects the third frame acts at each receiver of that frame.
- A copy that `duplicate` makes goes through the list again, but not through the rule that made it. A rule with `copiesOnly()` selects only copies, and its `nth` counts copies. Thus "duplicate the frame, then change the copy" is two rules.
- The original frame that a `duplicate` rule selects passes unchanged. The rule uses up the original, as `pass` does.

### 4.2 Actions

| Action | Wired: `PacketTap` | Wireless: `TapRadioMedium` |
| --- | --- | --- |
| `drop()` | Discard the frame (no change). | The frame does not exist for this receiver. The receiver gets no signal and no energy: no reception, no effect on its carrier sense, and no interference with its other receptions. The other receivers do not change. |
| `delay(t)` | Forward the frame after `t` (no change). | Refused when the rule is installed. A radio signal cannot arrive late. |
| `pass()` | Forward unchanged (no change). | Deliver unchanged. |
| `mutate(fn)` | Run `fn` on the inner packet (no change). | Run `fn` in `receivePacket`, on the copy that this receiver gets. |
| `corrupt()` | Set the bit error flag on the object on the wire: the `EthernetSignal`, or the `Packet` on PPP. | Deliver the signal as usual. In `computeReceptionDecision`, make the decision of the data part (or of the whole signal) fail. The receiver then sets the bit error flag itself, and the radio log, the error indication and the MAC agree. |
| `duplicate(spacing, copies)` | Copy k starts on the outgoing channel k × `spacing` after the original starts, or later while the channel is busy. | Copy k is a new transmission from the same transmitter, k × `spacing` after the original starts. Each receiver in range gets it (D-4). Nothing holds it back: an overlap is a collision, and the receivers model it. With `to(X)`, the copies do not exist for the other receivers, as with `drop()`. The spacing must be above zero, because the medium refuses two transmissions that start at the same time. |

**Why `drop()` on a wireless link also removes the energy (D-1).** The name of an action must say what occurs. A frame that a receiver senses but cannot decode is a corrupted frame, and `corrupt()` is the word for it. So `drop()` removes the frame completely for that receiver. A third level, "the receiver senses energy but finds no frame", is not offered, because no scenario needs it now.

**Why `corrupt()` sets the flag on the outer object on a wired link.** The bit error model of an OMNeT++ channel does the same thing. Thus the receiver follows its production path, with or without R1. After R1, `mutate` with `setBitError(true)` on the inner packet has the same result.

**Why a wireless copy is a new transmission.** A frame on the air reaches every receiver in range. The medium computes each arrival from the start of the transmission. A second signal of the same transmission at a later time would contradict the arrival data of the medium. A new transmission keeps the medium consistent, and the communication cache owns it.

### 4.3 Selectors

- **`from(node)` and `to(node)`.** On a wired link, the tap finds the node on each side at initialization, from the path of its gates. On a wireless link, the nodes are the nodes that contain the transmitter radio and the receiver radio.
- **`{name}` in `filterPacket`.** Move the substitution code of `EventPattern::matchesExpression` into a free function that the steps and the rules both call. An unresolved name is a non-match, as in a step. The tester gives each interceptor a const pointer to its capture store.
- **`copiesOnly()`.** See section 4.1.

### 4.4 The start and the end of a rule (D-2)

A rule is active between its start and its end. Only an active rule examines frames.

| Program word | The rule starts | The rule ends |
| --- | --- | --- |
| `.intercept(rule)` | At the start of the run (no change). | At the end of the run. With `within(d)` on the rule: `d` after its start. |
| `.meanwhile(rule)` (new overload) | When the cursor gets to this step. | The same. |

These statements define an active rule:

1. **The cursor does not wait.** `meanwhile(rule)` starts the rule, and the next step starts at the same simulation time. A guard in `meanwhile(never(...))` does the same.
2. **The rule counts from its start.** `nth(1)` is the first frame that the rule selects after its start. Frames before the start do not count.
3. **The rule acts where the interceptor sees the frame.** A wired tap sees a frame when the frame has arrived completely at the tap. The radio medium sees a frame when the transmission starts. A step observes the frame earlier, at the sender (section 2.3, the item about `packetSentToLower`). Thus a rule that a step starts can act on the frame that the step observed.
4. **A rule has no verdict.** A guard can fail the test, and the run waits for its window. A rule cannot fail the test, and the run does not wait for it. Its effect shows only in the steps that observe the receiver.
5. **The program text gives the priority.** All rules of one interceptor are in the order of the program text, also the rules that start later. The first active rule that matches applies, and an inactive rule is skipped. So a `pass()` rule that comes first in the text protects its frames from every later rule.
6. **A rule reads captures late.** A rule puts the captured values in place of `{name}` each time it examines a frame. Before the capture exists, the rule matches nothing. So write `meanwhile(rule)` after the step that makes the capture.
7. **`within(d)` ends the rule** `d` after its start (new). After the end, a frame goes to the later rules. With this word, "corrupt every frame to sta1 for 10 ms" is one rule. `within` is already the word for the window of a step and of a guard, so it gets no second name.

Statement 3 gives the most important pattern: observe a frame at the sender, and then act on that frame.

```cpp
.once(on("hostA.tcp").signal("packetSentToLower").filterPacket("tcp.destPort == 1000")
          .capture("seq", "tcp.sequenceNo").within(0.5))               // t1: host A sends the segment
.meanwhile(tap("tap").filterPacket("tcp.sequenceNo == {seq}").nth(1)
             .corrupt())                                               // starts at t1
// t1 + transmission time: the segment arrives at the tap, and the rule corrupts it.
```

**Why an overload of `meanwhile`.** `meanwhile` already means "start this beside the steps that follow, and do not wait". A rule from a step is the same idea. A second verb would give one idea two names. The alternative is a new verb, for example `.arm(rule)`, with the same semantics. Example E3 in section 4.8 shows the two forms.

### 4.5 Components

| Proposed addition | Responsibility | Required consumer | Reuse option | Reason for the choice |
| --- | --- | --- | --- | --- |
| `InterceptionRules` (library) | The ordered rules, their compiled filters, their counters and the records of copies. It selects the rule for a frame, a sender, a receiver and a copy mark. | `PacketTap`, `TapRadioMedium` | `PacketTap::Rule` and `selectRule` | Move this code, do not copy it. Two interceptors must have the same semantics. |
| `ITap` (library) | `addInterception(const Interception&, const CaptureStore *)` | `ProtocolTester::installInterceptions` and the `meanwhile` step | `PacketTap::configure` | The two interceptors have different bases (`SimpleModule`, `RadioMedium`), and the tester needs one contract. The seven arguments of `configure` go. |
| `TapRadioMedium` (library, C++) | A subclass of `RadioMedium`. In `addTransmission`, it makes the decision for each radio of the medium, before the medium sends a signal or computes energy. Each later call reads that decision: `isPotentialReceiver` (no signal), the two `isInterferingTransmission` overloads (no energy), `computeReceptionDecision` (`corrupt`), `receivePacket` (`mutate`). `handleMessage` starts the copies. One decision point for all receivers keeps the count of frames in a fixed order. | The wireless tests | The noise source of #1367, a MAC subclass in each test, a replacement error model, a plug-in slot in `RadioMedium` (D-5) | The user rejected the plug-in slot (D-5). Noise does not give a certain result. A MAC subclass is specific to one MAC type and one test. An error model cannot make a copy, and each receiver type has its own one. The medium is one module in each network, and it sees each pair of a frame and a receiver. |
| `TapRadioMedium` and `Ieee80211ScalarTapRadioMedium` (library, NED) | `TapRadioMedium` extends `RadioMedium`, and `Ieee80211ScalarTapRadioMedium` extends `Ieee80211ScalarRadioMedium`. Both set `@class(TapRadioMedium)`. | The Wi-Fi suite, and the C++ class, which needs a NED type of the same name ([NR-CPP-TYPE](../../doc/project/rule/naming.md#nr-cpp-type)) | None | The ini file cannot set the C++ class of a module. A test with another medium type declares a derived type of one line. |

**The names (2026-10-21).** In this API, `tap(...)` is a noun: the builder takes the noun, and the step keeps the verb `intercept`. So the module that `tap(...)` names is a tap, and the names say "tap":

- `PacketTap` is a tap, so `Tap` is its concept, and `Packet` says what it taps.
- `TapRadioMedium` is a radio medium, so `RadioMedium` is its concept, and `Tap` is the variant before it, as `Scalar` is in `ScalarRadioMedium` ([NR-NED-ROLE](../../doc/project/rule/naming.md#nr-ned-role)). The earlier name `InterceptRadioMedium` used a verb as the variant.
- A derived NED type keeps the words of its base in their order, puts the standard prefix first, and puts `Tap` just before `RadioMedium`: `Ieee80211ScalarTapRadioMedium`, `UnitDiskTapRadioMedium`.
- `ITap` is the contract that each tap implements ([NR-NED-TYPE](../../doc/project/rule/naming.md#nr-ned-type)). The earlier name was `IPacketInterceptor`.
| Fields of `Interception` | `from`, `to`, `copiesOnly`, `within`, `copies`, `spacing`, and the actions `duplicate` and `corrupt` | The test programs | None | None |

In `WifiInfraNetwork.ned`, the medium becomes `radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium`. A test then sets `*.radioMedium.typename = "Ieee80211ScalarTapRadioMedium"`. The default type stays the same, so no fingerprint changes.

The ini parameters of `PacketTap` do not get the new actions. No test sets those parameters ([QR-DESIGN-MINIMAL](../../doc/project/rule/quality.md#qr-design-minimal)).

### 4.6 R1: the Ethernet repair

Four places keep the flag of the packet:

- `EthernetMacPhy.cc:293` and `EthernetCsmaMacPhy.cc:708`: after `decapsulate`, `hasBitError = signal->hasBitError() || packet->hasBitError()`.
- `EthernetCsmaPhy.cc:388` and `PacketReceiverBase.cc:75`: set the flag of the packet when the signal has the flag, and never clear it.

Other protocol-element receivers also use `PacketReceiverBase`, so the pull request says so. R1 goes on its own branch to `master` first, because it is a shared defect and the test framework work depends on it.

### 4.7 The vocabulary

A rule is one clause. It has this form:

```text
.intercept( tap(<interceptor>) <selectors> <one action> [.describe(<text>)] )   // active from the start
.meanwhile( tap(<interceptor>) <selectors> <one action> [.describe(<text>)] )   // active from this step
```

All selectors of a rule must hold. A rule has exactly one action. The status column compares with `master` at `89d600a341`.

| Word | Kind | Status | Wired link: `PacketTap` | Wireless link: `TapRadioMedium` |
| --- | --- | --- | --- | --- |
| `.intercept(rule)` | program step | no change | The rule is active from the start of the run. | The same. |
| `.meanwhile(rule)` | program step | new overload (D-2) | The rule is active from this step. The cursor does not wait. | The same. |
| `tap(name)` | builder | wider (D-3) | Names a `PacketTap` module. | Names the radio medium module. |
| `filterPacket(expr)` | selector | wider | A `PacketFilter` expression over the frame. Now it can contain `{name}` captures. | The same, over the frame that the transmitter sends. |
| `minBytes(n)` | selector | no change | The frame has at least `n` bytes. | The same. |
| `from(node)` | selector | new | The frame comes into the tap from the side of `node`. | The transmitter radio is in `node`. |
| `to(node)` | selector | new | The frame leaves the tap on the side of `node`. | The receiver radio is in `node`. |
| `copiesOnly()` | selector | new | Only the copies that a `duplicate` rule made. | The same. |
| `nth(k)` | selector | clearer | The k-th frame that the rule selects after its start. | The same. A rule counts frames, not receptions. |
| `fromNth(k)` | selector | clearer | The k-th frame and each frame after it, from the start of the rule. | The same. |
| `within(d)` | selector | new (D-2) | The rule ends `d` after its start. | The same. |
| `pass()` | action | no change | Forward unchanged. | Deliver unchanged. |
| `drop()` | action | new on wireless (D-1) | Discard the frame. | The frame does not exist for the receiver: no signal and no energy. |
| `delay(t)` | action | no change | Forward after `t`. | Refused when the rule is installed. |
| `mutate(fn)` | action | new on wireless | Run `fn` on the inner packet, then forward. | Run `fn` on the copy that the receiver gets. |
| `corrupt()` | action | new | Set the bit error flag on the object on the wire. The receiving MAC discards the frame. | The receiver gets the signal, and the packet that it decodes has the bit error flag. The receiving MAC discards it. |
| `duplicate(spacing, copies = 1)` | action | new | Forward the original, then `copies` copies, each `spacing` after the one before it. A copy waits while the channel is busy. | The same as new transmissions from the same transmitter. Each receiver in range gets them (D-4). |
| `describe(text)` | text | no change | The phrase that the describer prints. | The same. |

These words do not change: the step words (`once`, `never`, `nextTimes`, the count words, `unordered`, `anyOf`, `delivery`, `inject`) and the pattern words (`on`, `signal`, `filterPacket`, `capture`, `within`, `notBefore`). The ini parameters of `PacketTap` do not change.

These combinations are refused when the rule is installed, with an error message that names the rule:

- `delay(t)` on the radio medium.
- `duplicate(...)` with a spacing of 0 on the radio medium, because the medium refuses two transmissions that start at the same time.
- `copiesOnly()` together with `duplicate(...)`, because a copy of a copy has no use.
- `from(node)` or `to(node)` with a node that is not on either side of the tap.

### 4.8 Examples

The examples use the networks of the present suites: `hostA`, `tap`, `hostB` on a wired link, and `WifiInfraNetwork` (`sta1`, `ap`, `sta2`) on a wireless link. The checks use only the present step words. Steps 3 to 5 make the exact filters of the real tests.

**E1. Wired, TCP duplicate segment (scenario 2).** The first data segment arrives two times, 2 ms apart. Host B acknowledges the copy, and TCP delivers the data one time.

```cpp
ProtocolTest("tcp_duplicate_segment")
    .intercept(tap("tap").from("hostA").minBytes(100).nth(1).duplicate(0.002)
                 .describe("the first data segment, and one copy 2 ms later"))
    .once(on("hostB.tcp").signal("packetReceivedFromLower").filterPacket("tcp.destPort == 1000")
              .capture("seq", "tcp.sequenceNo").describe("the original").within(0.5))
    .meanwhile(exactlyTimes(1, on("hostB.tcp").signal("packetSentToUpper").within(0.1)))   // one delivery to the application
    .once(on("hostB.tcp").signal("packetReceivedFromLower").filterPacket("tcp.sequenceNo == {seq}")
              .notBefore(0.002).describe("the copy, 2 ms later").within(0.0021))
    .once(on("hostB.tcp").signal("packetSentToLower").filterPacket("tcp.ackBit == true")
              .describe("host B acknowledges the copy").within(0.001));
```

**E2. Wired, corrupt a frame that a step observed.** The rule starts when host A sends the segment, and it corrupts the same segment at the tap. The MAC of host B must discard it, and IPv4 must not see it.

```cpp
ProtocolTest("ethernet_corrupt_frame")
    .once(on("hostA.tcp").signal("packetSentToLower").filterPacket("tcp.destPort == 1000")
              .capture("seq", "tcp.sequenceNo").within(0.5))
    .meanwhile(tap("tap").filterPacket("tcp.sequenceNo == {seq}").nth(1).corrupt()
                 .describe("the segment that host A just sent"))
    .meanwhile(never(on("hostB.ipv4").signal("packetReceivedFromLower")
                         .filterPacket("tcp.sequenceNo == {seq}").within(0.01)))
    .once(on("hostB.eth[0].mac").signal("packetDropped").within(0.01)
              .describe("the MAC of host B discards the frame"));
```

**E3. Wired, a rule from a step, in the two forms of D-2.** Corrupt the first data segment after the handshake. With `intercept` alone, the rule must count the frames of the handshake. With a start point, it does not.

```cpp
// D-2 recommended: an overload of meanwhile
.once(on("hostA.tcp").signal("packetSentToLower").filterPacket("tcp.synBit == false && tcp.ackBit == true")
          .describe("the last segment of the handshake").within(0.5))
.meanwhile(tap("tap").from("hostA").minBytes(100).nth(1).corrupt())

// D-2 alternative: a new verb, the same semantics
.once(...the same step...)
.arm(tap("tap").from("hostA").minBytes(100).nth(1).corrupt())
```

**E4. Wireless, 802.11 duplicate detection (scenario 1).** A copy of the first DATA frame of sta1 goes on the air 1 ms later, with the Retry bit. The AP acknowledges both, and sta2 gets the data one time.

```cpp
ProtocolTest("wifi_duplicate_with_retry")
    .intercept(tap("radioMedium").from("sta1").filterPacket("ieee80211mac.type == 32").nth(1)
                 .duplicate(0.001).describe("the first DATA frame of sta1, again 1 ms later"))
    .intercept(tap("radioMedium").copiesOnly().mutate(setRetryBit)
                 .describe("the copy has the Retry bit"))
    .meanwhile(exactlyTimes(1, on("sta2.udp").signal("packetSentToUpper").within(2.5)))
    .once(on("ap.wlan[0].mac").signal("packetReceivedFromLower")
              .filterPacket("ieee80211mac.type == 32").within(2.0)
              .describe("the original DATA frame at the AP"))
    .meanwhile(exactlyTimes(2, on("ap.wlan[0].mac").signal("packetSentToLower")
                                   .filterPacket("ieee80211mac.type == 29").within(0.002)))   // two ACKs
    .once(on("ap.wlan[0].mac").signal("packetReceivedFromLower")
              .filterPacket("ieee80211mac.type == 32").notBefore(0.001).within(0.0011)
              .describe("the copy, 1 ms after the original"));
```

A guard counts from the step before it, so the guard of the two ACKs comes after the step of the original frame. The spacing of 1 ms must put the copy after the ACK and after the frame that the AP forwards to sta2. Otherwise the AP transmits while the copy arrives, and it cannot receive the copy. Step 5 measures these times and sets the spacing.

Without the second rule, the copy has no Retry bit. Then the AP must deliver the data two times, because the standard rejects only a frame with the Retry bit. That is the test `WifiDuplicateWithoutRetry` of step 5.

**E5. Wireless, a burst of corrupted ACKs (scenario 3).** After the first DATA frame of sta1, every ACK to sta1 for 10 ms has a bit error. Sta1 must retransmit the DATA frame with the Retry bit.

`sequenceNumberOf` and `isRetryOf` are small lambdas in the test file, as in `WifiRetransmission.test`.

```cpp
ProtocolTest("wifi_corrupt_ack_burst")
    .once(on("sta1.wlan[0].mac").signal("packetSentToLower").filterPacket("ieee80211mac.type == 32")
              .capture("seq", sequenceNumberOf).within(2.0))
    .meanwhile(tap("radioMedium").to("sta1").filterPacket("ieee80211mac.type == 29")
                 .corrupt().within(0.01).describe("every ACK to sta1 for 10 ms"))
    .once(on("sta1.wlan[0].mac").signal("packetSentToLower").filterPacket("ieee80211mac.type == 32")
              .assertEvent(isRetryOf("seq")).within(0.01)
              .describe("the same DATA frame again, with the Retry bit"));
```

**E6. Wireless, `drop()` and `corrupt()` side by side (D-1).** The two rules differ in what sta2 senses.

```cpp
.intercept(tap("radioMedium").to("sta2").filterPacket("ieee80211mac.type == 32").nth(1).drop())
// sta2 gets nothing: no reception, no busy medium, and its MAC does not use EIFS.

.intercept(tap("radioMedium").to("sta2").filterPacket("ieee80211mac.type == 32").nth(1).corrupt())
// sta2 receives the frame and senses the busy medium, but the packet has a bit error.
// The MAC of sta2 discards it and uses EIFS for its next access.
```

**E7. Wired, the order of priority (statement 5 of section 4.4).** The first rule protects every SYN, also from the rule that starts later.

```cpp
.intercept(tap("tap").filterPacket("tcp.synBit == true").pass().describe("never touch a SYN"))
.once(...a step...)
.meanwhile(tap("tap").from("hostA").drop().describe("drop all frames of host A from here"))
// A SYN of host A after this step still passes, because the first rule comes first in the text.
```

**E8. A rewrite of `WifiBlockAckDuplicateAddba.test`.** The present test has two classes: `DuplicateAddbaMac` (22 lines) loses one ACK, and `DuplicateAddbaHcf` (77 lines) counts dispatches and ACKs, checks the retry with `ASSERT`, and holds the ADDBA Response in its queue. Each part has a replacement:

| Part of the present test | Replacement |
| --- | --- |
| `DuplicateAddbaMac` loses the ACK of the request | A rule: `.meanwhile(tap("radioMedium").to("sta1").filterPacket("ieee80211mac.type == 29").nth(1).drop())`, after the step that observes the request. |
| The signal `addbaReception` | `packetReceivedFromLower` of the AP MAC with `has(Ieee80211AddbaRequest)`. |
| The `ASSERT`s on the retry | `assertPacket` for the Retry bit, the dialog token and the TID; one small lambda for the sequence number and the transmitter. `SequenceNumberCyclic` is opaque, so a filter cannot compare it. |
| `dispatches == 1` | Its effect: each dispatch sends an ADDBA Response ([RecipientBlockAckAgreementHandler.cc:162-165](../../src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.cc#L162-L165)). So the AP sends exactly one ADDBA Response without the Retry bit, and none with a failure status. The MAC removes a management duplicate without a signal ([RecipientQosMacDataService.cc:124](../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosMacDataService.cc#L124)), so a test cannot see the removal itself. |
| `requestAcks == 2` and `requestAcksTransmitted == 2` | The radio of sta2 decodes the first ACK, because the rule drops it only for sta1. Sta1 receives the second ACK. |
| The held response and its release timer | An ini line: `*.ap.wlan[*].mac.hcf.edca.edcaf[3].aifsn = 15`. Management frames use `AC_VO` ([Hcf.cc:286-288](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc#L286-L288)), and `edcaf[3]` is `AC_VO`. The AP starts its response at the earliest 2·SIFS + ACK + 15 slots after the request ends. Sta1 starts its retry at the latest 2·SIFS + 10 slots + the PHY receive start delay after the request ends. The margin is at least the ACK duration plus 5 slots, minus that delay. |
| `hasOriginator(...)` | The DATA frame of sta1 with `Ieee80211DataHeader.ackPolicy == 3` (`BLOCK_ACK`). |

```cpp
Define_ProtocolTest(wifi_block_ack_duplicate_addba)
{
    auto sequenceNumber = [](const PacketEvent& e) {
        return cValue(e.packet->peekAtFront<Ieee80211AddbaRequest>()->getSequenceNumber().get());
    };
    return ProtocolTest("wifi_block_ack_duplicate_addba")
        .once(on("sta1.wlan[0].mac").signal("packetSentToLower")
                  .filterPacket("has(Ieee80211AddbaRequest) && Ieee80211AddbaRequest.retry == false")
                  .capture("token", "Ieee80211AddbaRequest.dialogToken")
                  .capture("tid", "Ieee80211AddbaRequest.tid")
                  .capture("seq", sequenceNumber)
                  .describe("the original ADDBA Request").within(1.2))
        .meanwhile(tap("radioMedium").to("sta1").filterPacket("ieee80211mac.type == 29").nth(1).drop()
                     .describe("the ACK of the original request, for sta1 only"))
        .meanwhile(exactlyTimes(1, on("ap.wlan[0].mac").signal("packetSentToLower")
                  .filterPacket("has(Ieee80211AddbaResponse) && Ieee80211AddbaResponse.retry == false")
                  .describe("one ADDBA Response, so one dispatch").within(0.02)))
        .meanwhile(never(on("ap.wlan[0].mac").signal("packetSentToLower")
                  .filterPacket("has(Ieee80211AddbaResponse) && Ieee80211AddbaResponse.statusCode != 0")
                  .describe("no response that refuses a second agreement").within(0.02)))
        .once(on("sta2.wlan[0].radio").signal("packetSentToUpper").filterPacket("ieee80211mac.type == 29")
                  .describe("the first ACK is on the air: sta2 decodes it").within(0.001))
        .once(on("ap.wlan[0].mac").signal("packetReceivedFromLower").filterPacket("has(Ieee80211AddbaRequest)")
                  .assertPacket("Ieee80211AddbaRequest.retry == true && Ieee80211AddbaRequest.dialogToken == {token}"
                                " && Ieee80211AddbaRequest.tid == {tid}")
                  .assertEvent([](const MatchContext& c) {
                      auto request = c.event.packet->peekAtFront<Ieee80211AddbaRequest>();
                      return request->getSequenceNumber().get() == c.get("seq").intValue() &&
                             request->getTransmitterAddress() == MacAddress("02:00:00:00:00:03");
                  })
                  .describe("the retry of the same request at the AP").within(0.01))
        .once(on("sta1.wlan[0].mac").signal("packetReceivedFromLower").filterPacket("ieee80211mac.type == 29")
                  .describe("the ACK of the retry at sta1").within(0.001))
        .once(on("sta1.wlan[0].mac").signal("packetReceivedFromLower").filterPacket("has(Ieee80211AddbaResponse)")
                  .describe("the ADDBA Response, after the retry").within(0.01))
        .once(on("sta1.wlan[0].mac").signal("packetSentToLower")
                  .filterPacket("has(Ieee80211DataHeader) && Ieee80211DataHeader.ackPolicy == 3")
                  .describe("DATA with the Block Ack policy").within(0.02));
}
```

```ini
[General]
*.tester.testName = "wifi_block_ack_duplicate_addba"
*.radioMedium.typename = "Ieee80211ScalarTapRadioMedium"
# The ADDBA Response of the AP waits AIFS = SIFS + 15 slots. The retry of sta1 starts earlier,
# so the AP receives the retry while its response is still in the queue.
*.ap.wlan[*].mac.hcf.edca.edcaf[3].aifsn = 15
*.sta1.app[0].sendInterval = 0.1ms
*.sta1.app[0].stopTime = 1.02s
sim-time-limit = 1.1s
include ../../ini/_blockackLifecycle.ini
```

The steps also check the order. If the AP sends its response before the retry, sta1 accepts it, and the AP acknowledges the retry and sends no second response. Then the step "the ADDBA Response, after the retry" misses its deadline, and the test fails.

These differences from the present test remain:

- The AIFS of the AP sets the order, not a blocked queue. If a change of the PHY timing removes the margin, the test fails. It does not pass in silence.
- The test observes the dispatch only through the response that the dispatch sends.
- Sta1 no longer senses the first ACK, because `drop()` removes its energy for sta1. In the present test, the radio of sta1 received the ACK, and the MAC deleted it. This moves the retry of sta1 a little earlier, which increases the margin.
- One run must confirm three assumptions: sta2 is in the receive state when the first ACK is on the air, the filter knows the class name `Ieee80211AddbaRequest` at the MAC, and the margin of the AIFS holds. The timings and the class name come from reading the code; no run has checked them yet.

## 5. Affected components and compatibility

- **`src/`, R1 only.** Ethernet now drops a frame whose packet has the flag. Today only the abort paths of the streaming transmitters set that flag without the flag of the signal. The fingerprints of tests that abort a streaming transmission can change. Examine each change before you update a baseline ([change-a-baseline.md](../../doc/project/guide/change-a-baseline.md)). R1 gets a release note file in `doc/whatsnew/`.
- **`tests/protocol/lib`.** New API words. `PacketTap::configure` changes; only `ProtocolTester` calls it.
- **`tests/protocol/wifi/ned/WifiInfraNetwork.ned`.** The `like` declaration.
- No production NED type changes.

## 6. Implementation steps and dependencies

| Step | Purpose | Files and owner | Proposed change | Expected behavior | Verification |
| --- | --- | --- | --- | --- | --- |
| 1 [x] | R1 | `src/inet/linklayer/ethernet/basic/`, `src/inet/physicallayer/wired/ethernet/EthernetCsmaPhy.cc`, `src/inet/protocolelement/transceiver/base/PacketReceiverBase.cc`; new tests in `tests/protocol/ethernet/` | Section 4.6. Tests: a tap sets the flag on the inner packet, for IPv4 and for IPv6, for `EthernetMacPhy` and for `EthernetPhyLayer`. | A1 | The new tests fail before the repair and pass after it. Run the Ethernet suite and the fingerprint tests. |
| 2 [x] | Shared rules | `tests/protocol/lib/`: new `InterceptionRules`, `ITap`; `PacketTap`, `ProtocolTester` | Move the rules and the selection out of `PacketTap`. No behavior change. | A5 | Run every protocol suite. The verdicts do not change. |
| 3 [x] | R2 and R3 on wired links | `PacketTap`, `ProtocolTest.h`, `ProtocolTestDescriber.cc` | `from`, `to`, `copiesOnly`, `duplicate`, `corrupt` | A2, A4 (Ethernet, PPP), A6 | New self tests: `InterceptDuplicate` (exact times of the two receptions), `InterceptCorrupt` (MAC drop, no ICMP), `InterceptDirection`, `InterceptCopiesOnly`. A TCP test for scenario 2 in `tests/protocol/tcp/rfc/`. |
| 4 [x] | `{name}` in rule filters and R4 | `EventPattern.cc`, `InterceptionRules`, `ProtocolTest.h`, `ProtocolTester.cc` | Section 4.3 and section 4.4, with `within(d)` and the priority by program text | A rule selects by a captured value. A rule that starts at a step does not count the frames before that step. A rule with `within(d)` stops after `d`. | New self tests: `InterceptCapture`, `InterceptFromStep`, `InterceptWithin`, `InterceptPriority`. |
| 5 [x] | R2 and R3 on wireless links | New `TapRadioMedium`, `Ieee80211ScalarTapRadioMedium.ned`; `WifiInfraNetwork.ned` | Section 4.2, wireless column | A3, A4 (802.11), A7 | New Wi-Fi tests: `WifiDuplicateWithRetry` (two ACKs, one delivery), `WifiDuplicateWithoutRetry` (two deliveries), `WifiCorruptAck` (retransmission with the Retry bit), `WifiDropFrame` (example E6, both rules). Rewrite `WifiBlockAckDuplicateAddba.test` as in example E8, and remove its two classes and its NED file section. |
| 6 [x] | R5 | `tests/protocol/lib/AUTHORING.md` | Section 7 and the cookbook. Correct the statement of line 264, which becomes true after R1. | The guide agrees with the code. | Read it against the self tests. |

Step 1 is on its own branch, with its pull request to `master`. It is two commits, because `PacketReceiverBase` is a shared component ([PR-SPLIT-UPSTREAM](../../doc/project/rule/pull-request.md#pr-split-upstream)): first the shared receiver, then the Ethernet receivers. The plan itself is the first commit of that branch, because the R1 commits name it. Steps 2 to 6 are one commit each on a second branch, which starts from the R1 branch until R1 lands. Step 3 needs step 2, and step 5 needs steps 2 and 3.

## 7. Verification methods and expected results

Follow [run-the-gates.md](../../doc/project/guide/run-the-gates.md) for a fresh debug library. From the checkout root:

```sh
inet_run_protocol_tests -p inet -m debug -w ethernet
inet_run_protocol_tests -p inet -m debug -w self -f 'Intercept'
inet_run_protocol_tests -p inet -m debug            # A5: every suite
tests/protocol/wifi/run-tests.sh                     # the Wi-Fi suite has its own runner
```

Expected results:

- Step 1, observed on 2026-10-21. The step is the commits `protocolelement: fix: a receiver keeps the bit error of the packet` and `ethernet: fix: discard a frame that carries its own bit error`. It adds four tests to `tests/protocol/ethernet/`: `BitErrorFrameDiscarded` (EthernetMacPhy), `BitErrorFrameDiscardedHalfDuplex` (EthernetCsmaMacPhy, a test beyond the plan), `BitErrorFrameDiscardedLayered` (PacketReceiverBase) and `BitErrorFrameDiscardedIpv6`.
  - Before the repair, the three IPv4 tests fail because IPv4 of the receiver gets the datagram, and the IPv6 test stops at the assert in `Ipv6.cc:240`. After the repair, the Ethernet suite passes (5 tests). At the state of the first fix commit, its own test `BitErrorFrameDiscardedLayered` passes alone.
  - No test reaches `EthernetCsmaPhy`, because a relay cannot sit on the multidrop bus of 10BASE-T1S.
  - The fingerprint runner runs 753 simulations in this tree, 110 of them with Ethernet or TSN: frame preemption, cut-through, 10BASE-T1S and others. Each one gives the same result and the same computed fingerprint as `master`. The runner skips the other CSV entries on `master` too, and the features `TcpLwip`, VoIP, Z3 and OSG are off in this tree. So no fingerprint obligation follows.
  - `check-pull-request.sh`, `check-source-seals.sh` and the architecture checks of the two changed subtrees pass. The findings of `check-naming.sh` and `check-interfaces.sh` are in other places and also exist on `master`.
- Step 2, observed on 2026-10-21 (`tests: refactor: a tap keeps its rules in InterceptionRules behind ITap`). Each of the 803 protocol tests gives the same verdict as before. The baseline at the R1 tip has 383 PASS, 47 expected FAIL, 306 expected SKIP (packetdrill) and 67 unexpected FAIL, all of them older than this work. A lesson for the runs: the runner builds the test library again before each suite, so an edit of the library during a run breaks the run.
- Step 3, observed on 2026-10-21 (`tests: add: a tap can duplicate and corrupt a frame, and select by node`, then `tcp: add: a duplicate segment draws an empty acknowledgment`). Decisions and facts:
  - A copy counts from the moment the original starts on the outgoing channel: the tap remembers the copies of a frame in its queue and schedules them when the frame starts. `getEncapsulatedPacket()` gives a private copy of a shared inner packet, so a change to a copy cannot change the original.
  - The tap finds the node behind each side from the end of the path of its gate. It refuses a rule whose `from` or `to` names a node on neither side (`InterceptUnknownNode`).
  - The describer rendered `pass()` as "drop"; it renders each action now.
  - `InterceptCopiesOnly` first had the `copiesOnly()` rule after the duplicate rule. The duplicate rule takes the original first, so the test passed without `copiesOnly()` too. The `copiesOnly()` rule comes first now, and then the selector decides the result.
  - The TCP check `Duplicate segment` in `checks/segment-acceptance.md` uses a transfer of 1000 octets in one segment. With the common transfer of 5000 octets, host B has only 4344 octets when the copy arrives (an initial window of three segments, and Nagle holds the rest until the delayed acknowledgment), so the next sequence number expected is not known in advance.
  - Each of the six self tests fails when its word has no effect (eight variants). 809 protocol tests keep their verdicts and the six new ones pass; the TCP suite passes 27 tests.
- Step 4, observed on 2026-10-21 (`tests: refactor: one function matches an expression with its captures`, then `tests: add: a tap rule can start at a step, end, and read captures`). Decisions and facts:
  - `matchesWithCaptures()` replaces two copies of the substitution code in `EventPattern.cc`; a rule of a tap calls it when its expression names a capture.
  - `meanwhile(tap(...))` adds the rule to `ProtocolTest::interceptions` and a step of the new kind `StepType::Intercept`. The tester installs all rules at startup, in the order of the text. A rule of `meanwhile(...)` waits, and the step starts it through `ITap::startInterception()`. `ITap::addInterception()` returns the handle of the rule and gets the capture store.
  - `within(d)` keeps a rule active while the time is less than its start plus `d`.
  - Each of the four self tests fails when its word has no effect. `InterceptPriority` puts a startup rule after a `meanwhile(...)` rule in the text, so that the order of the text and the order of the start times give different results. 814 protocol tests keep their verdicts and the four new ones pass.
- Steps 3 to 5: each new self test also runs once with the rule removed, and it must then fail (A6). Record this check in the commit message.
- Step 5, observed on 2026-10-21 (`tests: add: the radio medium can be a tap`, then `tests: change: WifiBlockAckDuplicateAddba needs no classes of its own`). Decisions and facts:
  - `TapRadioMedium` decides in `addTransmission` for each radio that `communicationCache->mapRadios` lists, before the medium sends a signal. A frame that reaches several receivers is offered once for each, with the transmission id as its frame id, and a rule counts it once (`InterceptionRules::Frame::frameId`).
  - The from/to nodes of the rules are checked when the first frame starts, because the radios register in a later stage of initialization than the one in which a tester in the NED installs the rules.
  - A copy is a new transmission through `transmitPacket`, and the medium owns its signal like any other. `createTransmission` of the 802.11 scalar transmitter makes a copy at a later time without trouble (every duplicate test).
  - `to(node)` names the receiver radio. The radio of sta1 also hears the ACK that sta2 sends to the AP, so a rule or a step that means "the ACK to sta1" filters by `ieee80211mac.receiverAddress.getInt() == {sta1}`, with the address captured from an earlier frame.
  - Finding: a copy is a frame that its transmitter does not know about. The station that sent the original gets the ACK of the copy, and `Dcf` stops the simulation with "Unknown control frame". IEEE 802.11 has no rule that makes a station act on an ACK that it does not expect. The two duplicate tests keep that ACK away from the station with a rule; the repair of `Dcf` is outside this plan.
  - `WifiDropRemovesEnergy` shows that a dropped frame leaves no energy: without the drop rule, the overlapping copy makes the AP discard the original.
  - The rewrite of `WifiBlockAckDuplicateAddba.test` removes its two classes (99 lines of C++) and its NED section, and goes from 174 to 73 lines. Without the AIFSN line, the AP sends its response first, and the retry never reaches it, so the line decides the order.
  - Each of the five new Wi-Fi tests and the rewritten test fail when their rule has no effect. 819 protocol tests keep their verdicts and the five new ones pass.
- Step 6, done on 2026-10-21 (`tests: comment: the authoring guide shows the new words of a tap`): section 7 of `AUTHORING.md` has the form of a rule, the tables of selectors and actions for both kinds of tap, the rules of a rule from a step, and the radio medium as a tap; the cookbook has three new entries.

## 8. Risks, unresolved decisions, and approval scope

### Decisions for the maintainer

- **D-1. The meaning of `drop()` on a wireless link. Decided 2026-10-21.** The user: do not call it a drop if it only corrupts the frame. So `drop()` removes the frame completely for the receiver, with its energy (section 4.2). The first draft kept the energy; that meaning is gone. The overrides of section 2.3 make this possible without changes to `src/`.
- **D-2. The start and the end of a rule. Agreed 2026-10-21.** Section 4.4 gives the details, and E3 shows the rejected form.
  - D-2a. An overload of `meanwhile`. The new verb `arm(...)` is rejected.
  - D-2b. `within(d)` ends a rule.
  - D-2c. The order of the program text gives the priority.
- **D-3. The builder word for the medium. Agreed 2026-10-21:** `tap("radioMedium")`.
- **D-4. The receivers of a wireless copy. Agreed 2026-10-21:** each receiver in range gets the copy, and `to(X)` limits it.
- **D-5. A subclass or a plug-in slot in the medium. Decided 2026-10-21:** the subclass `TapRadioMedium` stays in the protocol test library, with one derived NED type for each medium type that a test uses. The user rejected an optional `interceptor` slot in `RadioMedium.ned`. These forms were examined: one decision method with three outcomes; that method plus a hook before the transmission; and a hook for each step of the medium (seven methods). The first form also failed for `mutate`, because a changed copy at the same start time breaks `sameTransmissionStartTimeCheck`. This decision applies "for now", so a later revision can return to the slot.

### Risks

- Wireless `drop()` removes the energy only if every computation of energy goes through `isInterferingTransmission`. Resolved in step 5: `WifiDropRemovesEnergy` fails without the drop rule and passes with it.
- The transmitter radio does not know about a wireless copy. It can receive while the copy is on the air, and its MAC does not see the copy. This came true in step 5 as the `Dcf` finding above; the guide says it.
- `createTransmission` of the transmitter can read the state of the radio at the time of the copy. Resolved in step 5 for the 802.11 scalar transmitter: the copies work.
- R1 can change fingerprints through the abort paths of the streaming transmitters (section 5). Resolved in step 1: no computed fingerprint moves in the local run.
- A wired copy that waits for a busy channel follows the frame before it with no inter-frame gap. Resolved by reading the code: `EthernetMacPhy` keeps the gap only before it transmits (`WAIT_IFG_STATE`), and its receive path does not check the gap between two frames. No test sends two frames back to back on purpose.

### Approval scope

The approval of this plan covers steps 1 to 6. Step 1 changes production source on its own branch. Implementation starts in a dedicated worktree after approval.
