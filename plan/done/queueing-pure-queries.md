# Implementation plan: queries without side effects in classifiers, schedulers and filters

Checkout and revision: `inet-master`, branch `master`, commit `f269c3a26c`. Two untracked Markdown files in the root (`ieee80211-*.md`) are not related to this work. `inet-gpl`, branch `master`, commit `b606e7c`.
Plan revision: 2, 2026-10-14. Revision 2 records the decisions of the maintainer about D-6 and D-9.
Approval: the maintainer approved revision 2 on 2026-10-14 in the discussion of this plan ("implement this in a worktree"). The scope is steps 1 to 20. The approval does not cover the regeneration of fingerprint rows, statistical baselines or test expectations; each moved set needs its own approval (section 8). The maintainer took decisions D-1 to D-6 and D-9 in the same discussion (sections 4.1 and 4.6). On 2026-10-14 the maintainer also approved the moved expectations of section 9.2 ("I agree with them"), and asked for a review of the finished series before it lands ("don't land yet, I want review when all is done").

## 1. Problem and intended behavior

The queueing contracts have two kinds of operation:

- A **query** asks about a packet and moves nothing: `canPushSomePacket()`, `canPushPacket()`, `canPullSomePacket()`, `canPullPacket()`.
- A **commit** moves a packet: `pushPacket()`, `pullPacket()`, and the streaming operations `push*Start/Progress/End()` and `pull*Start/Progress/End()`.

A caller uses a query to decide, and then it commits. This is safe only if two conditions are true: the query changes nothing, and the commit does what the query said.

Classifiers, schedulers and filters do not satisfy these conditions. Their base classes call one decision hook from queries and from commits. The hook is not `const`, and some modules change their state in it. Thus each query changes the state, and the commit can decide differently from the query.

**Example 1, reproduced in issue #1317 with modules of INET only.** A `TokenBasedServer` pulls from a `WrrScheduler` with the weights `"1 1"`. One input holds 100-byte packets, and the other input holds 1000-byte packets. The server reads the length of the offered packet, removes that number of tokens, and then pulls. `PacketSchedulerBase::canPullPacket()` offers the head of the first input that is not empty. But `pullPacket()` serves the input that the weights select. With 16000 tokens of one bit each, the server passes 88000 bits. The correct result is 9600 bits: a small packet, a large packet and a small packet.

**Example 2, read from the code and not run.** A `PacketServer` asks `canPushPacket()` and then pushes the same packet into a `TokenBucketClassifier`. The classifier removes the tokens in the query, and again in the push. In pull mode, the classifier removes tokens on each peek. Then `pullPacket()` can select the other gate and stop the simulation with an error.

**Intended behavior.** Each module of the three families obeys three rules:

- **R1, agreement.** The rule applies in one event, with no other operation between the two calls. If `canPullPacket(g)` returns `p`, then `pullPacket(g)` returns `p`. If `canPushPacket(p, g)` returns true, then `pushPacket(p, g)` accepts `p` on the path that the query examined.
- **R2, pure queries.** A query calls only other queries. It changes no state, emits no signal, draws no random number and schedules no event. Any number of queries, in any order, gives the same simulation results.
- **R3, one commit.** Each packet that goes through a module commits one time. A stream commits one time, at its start. Only the commit and the module's own events change the decision state. The module's own events are its timers and the notifications that it gets.

R2 has one permitted exception, E1: `DynamicClassifier::canPushPacket()` creates the branch of a class that it has not seen before (section 4.4).

## 2. Current implementation and evidence

### 2.1 Classifiers

`PacketClassifierBase` calls `classifyPacket()` through `callClassifyPacket()`. That function removes `const` with a cast marked `KLUDGE` ([PacketClassifierBase.cc:59](../../src/inet/queueing/base/PacketClassifierBase.cc#L59)).

- **Queries that call the hook:** `canPushPacket()` ([L97](../../src/inet/queueing/base/PacketClassifierBase.cc#L97)), `canPullPacket()` ([L170](../../src/inet/queueing/base/PacketClassifierBase.cc#L170)) and `handleCanPullPacketChanged()` ([L217](../../src/inet/queueing/base/PacketClassifierBase.cc#L217)).
- **Commits that call the hook:** `pushPacket()` ([L103](../../src/inet/queueing/base/PacketClassifierBase.cc#L103)), `startPacketStreaming()` ([L74](../../src/inet/queueing/base/PacketClassifierBase.cc#L74)), `pullPacket()` ([L181](../../src/inet/queueing/base/PacketClassifierBase.cc#L181)) and the three pull stream operations (L190–L215). The pull stream operations classify again on each call, because they keep no stream state.

These classifiers change state in the hook:

| Module | State change in `classifyPacket()` | Effect |
| --- | --- | --- |
| `TokenBucketClassifier` ([L26](../../src/inet/queueing/classifier/TokenBucketClassifier.cc#L26)); `SingleRateTwoColorClassifier` extends it in NED | removes tokens, emits `tokensChanged` | A query and a push remove the tokens two times. In pull mode, each peek removes tokens. |
| `MultiTokenBucketClassifier` ([L26](../../src/inet/queueing/classifier/MultiTokenBucketClassifier.cc#L26)); `SingleRateThreeColorClassifier` and `DualRateThreeColorClassifier` extend it in NED | the same | the same |
| `WrrClassifier` ([L42](../../src/inet/queueing/classifier/WrrClassifier.cc#L42)) | decrements and refills `buckets[]` | Each query moves the round forward. |
| `DynamicClassifier` ([L29](../../src/inet/queueing/classifier/DynamicClassifier.cc#L29)) | creates, connects and initializes a branch module | Each query about a new class builds a branch. With `reverseOrder`, the key of a class depends on the number of gates, so one class can get a second branch (pull request #1122). |

The other classifiers change no state in the hook: `ContentBasedClassifier`, `LabelClassifier`, `PacketClassifier`, `PriorityClassifier`, `MarkovClassifier`, `PcpClassifier`, `PcpTrafficClassClassifier`, `EthernetFrameClassifier`, `BehaviorAggregateClassifier`, `MultiFieldClassifier` and `StreamClassifier`. `PacketClassifier` calls `IPacketClassifierFunction::classifyPacket()`, which is already `const`. `MarkovClassifier` changes its state only on its own timer. `PriorityClassifier` and `StreamClassifier` call `canPushPacket()` of their consumers, which is a query.

### 2.2 Schedulers

`PacketSchedulerBase` calls `schedulePacket()` through `callSchedulePacket()`, which also removes `const` with a `KLUDGE` cast ([PacketSchedulerBase.cc:54](../../src/inet/queueing/base/PacketSchedulerBase.cc#L54)).

- **Queries that call the hook:** `canPushSomePacket()` ([L82](../../src/inet/queueing/base/PacketSchedulerBase.cc#L82)), and through it `canPushPacket()`. `handleCanPushPacketChanged()` ([L133](../../src/inet/queueing/base/PacketSchedulerBase.cc#L133)) calls `schedulePacket()` directly. `PriorityScheduler::handleCanPullPacketChanged()` ([L91](../../src/inet/queueing/scheduler/PriorityScheduler.cc#L91)) also calls the hook.
- **Queries that do not call the hook:** `canPullSomePacket()` ([L143](../../src/inet/queueing/base/PacketSchedulerBase.cc#L143)) and `canPullPacket()` ([L153](../../src/inet/queueing/base/PacketSchedulerBase.cc#L153)). They answer for the first input that holds a packet. This is the defect of issue #1317.
- **Commits that call the hook:** `pullPacket()` ([L164](../../src/inet/queueing/base/PacketSchedulerBase.cc#L164)), `startPacketStreaming()` ([L69](../../src/inet/queueing/base/PacketSchedulerBase.cc#L69)), `pushPacket()` ([L93](../../src/inet/queueing/base/PacketSchedulerBase.cc#L93)) and the three push stream operations (L102–L131).
- **Push streams do not work today.** `pushPacketStart()` does not set `inProgressStreamId`, so the assertion in `pushPacketEnd()` fails.

| Module | State change in `schedulePacket()` | Effect |
| --- | --- | --- |
| `WrrScheduler` ([L68](../../src/inet/queueing/scheduler/WrrScheduler.cc#L68)) | decrements and refills `buckets[]` | Each query moves the round forward. The offered packet is wrong (#1317). |
| `HtbScheduler`, inet-gpl, `HtbScheduler.cc:497` | `doEvents()` brings the class modes up to the current time. `htbDequeue()` (L608) deactivates empty classes, changes the DRR deficit and pointer, and calls `chargeClass()`. | `canPullPacket()` throws "not implemented yet" (`HtbScheduler.h:157`). |
| `HlsScheduler`, inet-gpl, `HlsScheduler.cc:514` | `hlsDequeue()` (L470) moves the round-robin pointer, starts new rounds, updates quotas and sets classes idle | The offered packet is wrong (#1317). |
| `PriorityScheduler`, `ContentBasedScheduler`, `LabelScheduler`, `PacketScheduler`, `MarkovScheduler` | none | The offered packet is wrong, except for `PriorityScheduler` without `reverseOrder`. |

`LabelScheduler::schedulePacket()` ([L61](../../src/inet/queueing/scheduler/LabelScheduler.cc#L61)) dereferences the offered packet of each input without a null check. So it crashes when an input is empty.

No module calls `HtbScheduler::canPullPacket()` today, because the call would throw. Thus the inet-gpl fingerprint rows (16 HTB rows and 3 HLS rows in `tests/fingerprint/examples.csv`) record the behavior of a search that runs only at pull time.

### 2.3 Filters

`IPacketFilter::matchesPacket()` is already `const`. `PacketFilterBase` has three other problems:

- `canPullPacket()` ([L174](../../src/inet/queueing/base/PacketFilterBase.cc#L174)) pulls and drops each packet that does not match, inside the `const` query (`KLUDGE` at L191).
- `pushPacketStart()`, `pushPacketEnd()` and `pushPacketProgress()` (L95–L154) call `matchesPacket()` and `processPacket()` again on each call of a stream. Thus the decision can change in the middle of a stream.
- The base class has no commit hook. So modules put their state changes into `matchesPacket()`, `processPacket()` or `dropPacket()`.

| Module | State change | Effect |
| --- | --- | --- |
| `RedDropper` | `matchesPacket()` ([L105](../../src/inet/queueing/filter/RedDropper.cc#L105)) calls `doRandomEarlyDetection()` ([L61](../../src/inet/queueing/filter/RedDropper.cc#L61)) through a cast. That call updates `avg`, `count` and `q_time`, and calls `dblrand()`. The `mutable lastResult` carries the result to `processPacket()` ([L122](../../src/inet/queueing/filter/RedDropper.cc#L122)). | A query and a push update the average two times and draw two random numbers. |
| `StatisticalRateLimiter` | `matchesPacket()` ([L41](../../src/inet/queueing/filter/StatisticalRateLimiter.cc#L41)) calls `dblrand()` | A pull query draws a random number, and each stream call draws again. The override of `canPushPacket()` ([L36](../../src/inet/queueing/filter/StatisticalRateLimiter.cc#L36)) keeps push queries free of draws. |
| `OrdinalBasedDropper` | `processPacket()` ([L46](../../src/inet/queueing/filter/OrdinalBasedDropper.cc#L46)) and `dropPacket()` ([L51](../../src/inet/queueing/filter/OrdinalBasedDropper.cc#L51)) count packets | The count increases on each stream call, and in `canPullPacket()`. |
| `StreamMerger` | `processPacket()` ([L43](../../src/inet/protocolelement/redundancy/StreamMerger.cc#L43)) records sequence numbers | The record grows on each stream call. |
| `ReceiveWithLifetime`, `EligibilityTimeFilter` | none; the verdict depends on the time | In pull mode, the verdict about the head packet can change without an event. |
| The header checkers in `protocolelement`, Ethernet, IEEE 802.1Q, 802.1AE, 802.1R, 802.2 and VLAN | none in `matchesPacket()`; `processPacket()` removes a header from each packet instance | This transformation of each instance is correct. |

### 2.4 Users, tests and fingerprint rows

A text search of the configurations gives these users. Section 7.4 describes the method of the search and its limits.

- **`RedDropper`:** `examples/diffserv/onedomain` (Exp21, Exp22, Exp23, Exp31), `examples/inet/netperfmeter` (4 rows), `examples/inet/redmarker` (TcpSender1), `examples/inet/dctcp` (2 rows), `tutorials/queueing` (config RedDropper), `tests/queueing/RedDropper_1.test`.
- **`StatisticalRateLimiter`:** `showcases/tsn/streamfiltering/statistical` (2 rows), `tests/queueing/Meter_1.test`.
- **`WrrScheduler`, `WrrClassifier`:** `DiffservQueue` in `examples/diffserv/simple_`, `examples/diffserv/onedomain` and `showcases/general/diffserv`; `tutorials/queueing`. No `.test` file exists for these modules.
- **Token bucket classifiers:** `tests/queueing/TokenBucketClassifier_1.test`, `tests/queueing/MultiTokenBucketClassifier_1.test`. No fingerprint row names them.
- **`DynamicClassifier`:** no test and no fingerprint row. Pull request #1122 states that its users (`MacService`, `PeerService`) have no runnable configuration.
- **Through compound modules:** `EthernetQosQueue` contains `EthernetFrameClassifier` and `PriorityScheduler` without `reverseOrder`. It is used in `ethernet.csv` and in about 50 module tests. `tutorials/queueing` uses almost every queueing module (47 rows).
- **Filters through defaults:** all TSN showcases, the layered Ethernet interfaces (`mrp.csv`, `examples/ieee8021*`, `tutorials/stp`) and `examples/ethernet/vlan`.
- **Pull mode through a filter:** only `tests/queueing/Filter_2.test`. That test puts a `PacketFilter` between a `PassivePacketSource` and an `ActivePacketSink`.
- **inet-gpl:** `examples/htb` (16 rows), `examples/hls` (3 rows).
- **simu5g:** no subclass of the three base classes.

Two more queries outside the three families change state. They are not in scope (section 3): `PassivePacketSource::canPullPacket()` creates the offered packet ([PassivePacketSource.cc:57](../../src/inet/queueing/source/PassivePacketSource.cc#L57)), and `CreditBasedGate` updates its credit in a `const` function ([CreditBasedGate.cc:90](../../src/inet/queueing/gate/CreditBasedGate.cc#L90)).

## 3. Scope and acceptance criteria

**Required work and acceptance criteria:**

- **C1.** The decision hooks are `const`: `classifyPacket()`, `schedulePacket()` and `matchesPacket()` (already `const`). The three `KLUDGE` casts in the base classes are removed. A cast that removes `const` stays only for E1.
- **C2.** Each classifier, scheduler and filter in INET and inet-gpl obeys R1, R2 and R3. The probe tests of section 7.1 pass with 0 and with 3 repeated queries, and both runs give the same output.
- **C3.** In the scenario of issue #1317, the server passes 9600 bits, not 88000.
- **C4.** A token bucket classifier removes the tokens of a packet one time, when the packet goes through.
- **C5.** In pull mode, a filter drops a packet that does not match only in its own operations, never in a query.
- **C6.** The contract documentation and a new architecture rule state R1, R2 and R3. `WHATSNEW` tells the authors of subclasses how to migrate.
- **C7.** Each fingerprint row or test expectation that moves has a stated reason and the approval of the maintainer before it is regenerated.

**Conditions that must stay true:**

- Without queries, `WrrScheduler`, `WrrClassifier`, `PriorityScheduler` and the other schedulers serve their inputs in the same order as today.
- The header checkers transform each packet instance as today.
- No NED parameter is added or removed. `DynamicClassifier` refuses `reverseOrder = true`, and `ReceiveWithLifetime` and `EligibilityTimeFilter` refuse pull mode (D-10).

**Follow-up work, not in this plan:**

- **F1.** `PassivePacketSource` creates the offered packet in a query.
- **F2.** `CreditBasedGate` updates its credit in a `const` function.
- **F3.** An audit of all other queries in `queueing` and `protocolelement`, for example queries that emit signals.
- **F4.** A `const Packet *` parameter for `classifyPacket()` and `IPacketClassifierFunction` (D-7).
- **F5.** The parts of pull request #1122 outside classification (D-6).
- **F6.** Pull mode for filters whose verdict changes with time (D-10).

## 4. Proposed design and reasons

### 4.1 Decisions of the maintainer, 2026-10-14

- **D-1.** The commit hooks are named `handlePacketClassified()` and `handlePacketScheduled()`. The filter hook follows the same pattern: `handlePacketFiltered()`.
- **D-2.** The commit occurs when the module holds the packet, before it gives the packet to the next module.
- **D-3.** Subclasses outside INET break. The change has no compatibility layer.
- **D-4.** Filters are part of this work.
- **D-5.** This work replaces pull requests #1319 and #1122. D-6 describes the part of #1122 that is outside this design.

D-3 skips the deprecation step of [RR-DEPRECATE-FIRST](../../doc/project/rule/release.md#rr-deprecate-first). The reason that the rule asks for: the old hook is harmful. A non-`const` hook that the base class calls from queries keeps the defect, so a deprecated copy of it would keep the defect for one more release.

### 4.2 One query hook and one commit hook in each base class

```cpp
// PacketClassifierBase
virtual int classifyPacket(Packet *packet) const = 0;
virtual void handlePacketClassified(Packet *packet, int index) {}

// PacketSchedulerBase
virtual int schedulePacket() const = 0;
virtual void handlePacketScheduled(Packet *packet, int index) {}

// PacketFilterBase; matchesPacket() is already const in IPacketFilter
virtual void handlePacketFiltered(Packet *packet, bool matches) {}
```

**Reasons:**

- The compiler enforces R2 for the decision.
- The base classes own the order of the calls. Thus each subclass gets R1 and R3 without overrides of its own.
- The hooks follow the pattern of `PacketProcessorBase::handlePacketProcessed()`. The function interfaces `IPacketClassifierFunction` and `IPacketSchedulerFunction` are already `const`, so the decision part needs no new mechanism ([AR-EXT-REUSE](../../doc/project/rule/architecture.md#ar-ext-reuse)).
- The commit hooks are protected virtual functions of the base classes, not part of the contracts ([AR-ORG-CONTRACT-PURITY](../../doc/project/rule/architecture.md#ar-org-contract-purity)).
- Each commit hook has consumers, the stateful modules of sections 2.1 to 2.3. That satisfies [AR-EXT-VIRTUAL-IS-A-PROMISE](../../doc/project/rule/architecture.md#ar-ext-virtual-is-a-promise).

**Rejected alternatives:**

- **A correction in each module, the method of #1319.** Each new module must remember the rule, the cast stays, and each pure scheduler needs its own override.
- **A decision object** that the query returns and the commit applies. This makes the API larger. A module can compute its decision again in the commit.
- **A cache that queries fill,** the method of `RedDropper::lastResult`. The module cannot know when the cache becomes stale.

### 4.3 What the base classes call

| Base class operation | Now | After the change |
| --- | --- | --- |
| classifier queries | hook, through the cast | `classifyPacket()` |
| classifier `pushPacket()`, `pullPacket()`, stream start | hook | `classifyPacket()`, then `handlePacketClassified()` |
| scheduler `canPullPacket(g)` | head of the first input that is not empty | `providers[schedulePacket()].canPullPacket()`, or `nullptr` for -1 |
| scheduler `canPullSomePacket(g)` | some input holds a packet | `canPullPacket(g) != nullptr` |
| scheduler push queries, `handleCanPushPacketChanged()` | hook | `schedulePacket()` |
| scheduler `pullPacket()`, `pushPacket()`, stream start | hook | `schedulePacket()`, then `handlePacketScheduled()` |
| filter push | `matchesPacket()` on each call | `matchesPacket()` one time, then `handlePacketFiltered()`, then `processPacket()` and forward, or drop |
| filter `canPullPacket()` | drops packets that do not match | returns the head if it matches, else `nullptr` |

**The commit point (D-2):**

- **Push:** the commit occurs after `take()` and before `pushOrSendPacket()`.
- **Pull:** the commit occurs after the input returns the packet and before `return`.
- **Stream:** the commit occurs one time, at the start. The base class latches the decision (the gate index or the filter verdict) for the rest of the stream, in both directions. The latch is new for classifier pull streams, for filter push streams and for scheduler push streams. For scheduler push streams, `pushPacketStart()` also sets `inProgressStreamId`.
- **Queue state:** a module sees its input queue after the pull when it commits. This matters for HTB and HLS, which test `getNumPackets() <= 1` today.

**Filters in pull mode (D-8).** `canPullPacket()` returns the head packet if it matches, and else `nullptr`. Without back pressure, the filter drops the packets that do not match in two of its own operations:

- In `pullPacket()`, before and after it takes the packet that matches.
- In `handleCanPullPacketChanged()`, before it notifies its collector.

Thus the head packet matches each time that a collector asks. This is true for each filter whose verdict changes only at commits.

### 4.4 Exception E1: `DynamicClassifier` creates a branch in a query

Pull request #1122 gives the reason. The answer of `canPushPacket()` must hold for the packet that the caller then pushes. For a new class, the answer depends on the branch, and the branch does not exist yet. For example, a branch can start with a closed gate. So the query creates the branch, a second query finds it, and the push would create it anyway. The cost: a branch can exist for a class whose packet never arrives.

`classifyPacket()` becomes a lookup in a map. The key of the map is the index that the classifier function returns. `DynamicClassifier` refuses `reverseOrder = true` at initialization, because a key that depends on the number of gates changes when a branch is added. On the pull side, a class without a branch fails as today.

### 4.5 The stateful modules after the change

**Classifiers:**

- **`TokenBucketClassifier`:** `classifyPacket()` compares the token count, and `TokenBucket::getNumTokens()` is already `const`. The commit removes the tokens for index 0 and emits `tokensChanged` before and after the removal, as today.
- **`MultiTokenBucketClassifier`:** `classifyPacket()` finds the first bucket that has enough tokens. The commit removes the tokens from that bucket. The last index means "no bucket", so its commit removes no tokens.
- **`WrrClassifier`:** `classifyPacket()` makes the current selection and changes nothing. For the selected index `i`, the commit does one of two things:
  - If `buckets[i] > 0`, it decrements `buckets[i]`.
  - Else it refills all buckets from `weights[]` and decrements `buckets[i]`.

  The current code refills only when each consumer that can accept a packet has an empty bucket. In that case, the selected bucket is also empty. So the commit gives the same result.
- **`DynamicClassifier`:** section 4.4.

**Schedulers:**

- **`WrrScheduler`:** the selection is the `selectInput()` of #1319. For the selected index `i`, the commit does one of three things:
  - If `buckets[i] > 0`, it decrements `buckets[i]`.
  - Else, if `weights[i] > 0`, it refills all buckets and decrements `buckets[i]`.
  - Else it changes nothing.

  This gives the same order as the current code ([L68](../../src/inet/queueing/scheduler/WrrScheduler.cc#L68)).
- **`HtbScheduler`, `HlsScheduler`:** D-9.
- **The other schedulers:** only `const` is added.

**Filters:**

- **`RedDropper`:** the decision becomes a pure function of the stored state (`avg`, `count`, `q_time`), the queue length, the simulation time and a stored random variate `u`. The commit stores the new `avg`, `count` and `q_time`, does the ECN mark and updates `markNext`. If the decision used `u`, the commit then draws a new `u`. `lastResult` is removed.
- **`StatisticalRateLimiter`:** the decision uses the stored `u` when the drop probability is above zero. If the decision used `u`, the commit draws a new `u`.
- **Random variates (D-12):** the module draws the first variate at initialization. Then it draws a new variate in each commit that used the stored one. Thus the number and the times of the draws do not depend on queries.
- **`OrdinalBasedDropper`:** the commit counts the packet. The overrides of `processPacket()` and `dropPacket()` go away.
- **`StreamMerger`:** the commit records the sequence number. `processPacket()` keeps the change of the stream name, because that change applies to each packet instance.
- **`ReceiveWithLifetime`, `EligibilityTimeFilter`:** these filters refuse pull mode (D-10).

### 4.6 Proposed decisions that need confirmation

- **D-6, the rest of #1122. The maintainer accepted the recommendation below on 2026-10-14.**
  - This plan takes from #1122 only the parts that its design needs: the pure classification, the key on the index of the classifier function, the refusal of `reverseOrder`, and the creation of the branch in `canPushPacket()`, `pushPacket()` and `startPacketStreaming()`.
  - It does not take the rest of #1122:
    - the `aggregatorSubmoduleName` parameter and the support for pull aggregators
    - the initialization of a branch after its connection
    - the answer of `canPushSomePacket()` before the first branch exists
    - the tests of #1122 for those parts
  - #1122 is the base of the per-station 802.11 queue. Recommendation: rebase #1122 onto this work and keep those parts in it. If nobody keeps them, they are lost.
- **D-7. The parameter of `classifyPacket()` stays `Packet *`.** `IPacketClassifierFunction` and each registered classifier function take `Packet *`, so a change would be a second break (F4). Review and the probe tests check that a query does not change the packet.
- **D-8. A filter in pull mode drops a packet that does not match when the packet becomes available, not when a collector asks.**
  - This changes the expected output of `Filter_2.test`. Expected result, to be measured: the drop of `provider-1` moves from 1 s to 0 s, and `provider-2` is created at 0 s.
  - The alternative is a drop in the query, which breaks R2.
  - A look-ahead in the provider is possible only through `IPacketCollection`. That interface does not always give the pull order, for example behind a WRR scheduler.
- **D-9, the method for HTB and HLS. The maintainer accepted the recommendation below on 2026-10-14: method (a), with method (b) as the fallback.** Three methods are possible:
  - **(a) A stored decision.** The search runs at the module's own events: the queue signals that HTB and HLS already get, their timers, and the end of each commit. The search stores the next index, and `schedulePacket()` returns it. The commit charges the class and then runs the search again. This method keeps the algorithms. But the search runs at a different time. So the insertion position of a new HLS class, and the HTB modes at equal time stamps, can differ from today.
  - **(b) A pure search.** The method writes the search again without changes to the state. It keeps the behavior if it is correct, but it rewrites a large part of two GPL modules.
  - **(c) A trial on a copy.** Each query searches on a deep copy of the class tree. This keeps the behavior. But the commit must run the real search on the state before the pull, and D-2 does not permit that.

  Recommendation: (a). If the 19 inet-gpl rows move and a reason cannot be given, use (b).
- **D-10. `ReceiveWithLifetime` and `EligibilityTimeFilter` refuse pull mode.** Their verdict changes with time and no event occurs. Under D-8, an expired head packet would stay until the next event of the provider. No current user pulls through these filters (A-4). The alternative, a timer at the expiry time of the head packet, is F6.
- **D-11. `StatisticalRateLimiter` keeps its override of `canPushPacket()`.** The override is pure, and it keeps the behavior of the TSN showcases.
- **D-12. The random variate of a decision is drawn in advance (section 4.5).**
- **D-13. The probe module goes into `tests/queueing/lib/`.** The opp_repl test runner builds a `lib` folder next to the tests if the folder has a `Makefile` (`opp_repl/test/opp.py:276`). `tests/module/lib` uses the same mechanism.
- **D-14. The work lands as three pull requests, in this order:**
  1. classifiers: steps 1 to 6
  2. schedulers: steps 7 to 12, and the two inet-gpl commits
  3. filters and documentation: steps 13 to 20

### 4.7 Assumptions to check

- **A-1. No filter in a current configuration changes its verdict during a stream.** Check: at step 13, run the fingerprints of `showcases/tsn/cutthroughswitching` and `showcases/tsn/framepreemption`, the Ethernet protocol tests and `tests/module/EthernetFragmentFcsChecker_1.test`. If a result moves, the latch changes behavior, and step 13 needs a revision.
- **A-2. No stream goes through `RedDropper`, `OrdinalBasedDropper`, `StatisticalRateLimiter` or `StreamMerger`.** Check: the same runs as for A-1.
- **A-3. In current configurations, no input calls back into a scheduler while the scheduler pulls from it.** If this is true, the commit after the pull (D-2) keeps the WRR order. Check: the fingerprints of step 8.
- **A-4. No current configuration pulls through `ReceiveWithLifetime` or `EligibilityTimeFilter`.** Check: with D-10, such a configuration stops at initialization. Run `tests/protocol/element/Lifetime*.test`, `tests/validation/tsn/trafficshaping/asynchronousshaper` and the TSN showcases `asynchronousshaper` and `cbsandats`.

## 5. Affected components and compatibility

| Artifact | Change |
| --- | --- |
| `src/inet/queueing/base/Packet{Classifier,Scheduler,Filter}Base.{h,cc}` | the hooks, the call order, the stream latches |
| The classifier subclasses: `queueing/classifier/*`, `linklayer/ieee8021q/Pcp*Classifier`, `linklayer/ethernet/common/EthernetFrameClassifier`, `networklayer/diffserv/{BehaviorAggregate,MultiField}Classifier`, `protocolelement/redundancy/StreamClassifier` | `const`; the stateful modules move their state changes into the commit |
| The scheduler subclasses: `queueing/scheduler/*` | `const`; `WrrScheduler` commit; the `LabelScheduler` null check |
| The filter subclasses: `RedDropper`, `StatisticalRateLimiter`, `OrdinalBasedDropper`, `StreamMerger`, `ReceiveWithLifetime`, `EligibilityTimeFilter` | the commit, the variates, the refusal of pull mode |
| NED documentation of `DynamicClassifier`, `ReceiveWithLifetime` and `EligibilityTimeFilter` | the refusals; no parameter changes |
| `src/inet/queueing/contract/IPassivePacketSink.h`, `IPassivePacketSource.h`, `IPacketFilter.h` | documentation comments only; no signature changes |
| [architecture.md](../../doc/project/rule/architecture.md) | a new rule `AR-QUEUE-QUERIES` next to [AR-QUEUE-ROLES](../../doc/project/rule/architecture.md#ar-queue-roles); the file carries no seal flag |
| `WHATSNEW` | behavior changes and a migration note ([RR-WHATSNEW](../../doc/project/rule/release.md#rr-whatsnew), [RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate)) |
| `tests/queueing/lib/` (new), `tests/queueing/*.test` | the probe and its tests |
| inet-gpl `HtbScheduler`, `HlsScheduler` | one temporary commit together with step 7, then steps 11 and 12 |

**Compatibility.**

- A subclass outside INET that overrides `classifyPacket()` or `schedulePacket()` does not compile with `override`. Without `override`, the class stays abstract. Both failures are loud.
- A filter outside INET that changes state in `matchesPacket()`, `processPacket()` or `dropPacket()` still compiles, but it breaks R1 to R3. `WHATSNEW` tells its author to move that state into `handlePacketFiltered()`.
- simu5g has no subclass.
- [RR-FINGERPRINT-NOTE](../../doc/project/rule/release.md#rr-fingerprint-note) applies to the moved rows of section 7.3.

## 6. Implementation steps and dependencies

The implementation uses a worktree `inet-queueing-pure-queries` on the branch `topic/queueing-pure-queries`. The inet-gpl commits use a worktree with a branch of the same name, and they land on inet-gpl `master` together with the INET pull request that they follow.

Each commit names this plan on a `Plan:` line ([PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan)). Each commit also has a `Change:` trailer, a body wrapped at 72 columns, and no attribution trailer.

Each behavior commit ships its own `WHATSNEW` entry ([TR-SHIP-WITH](../../doc/project/rule/testing.md#tr-ship-with), [CR-OBL-DERIVED](../../doc/project/rule/classification.md#cr-obl-derived)). The split steps 2, 7 and 13 change a public extension point, so each one adds a section to `doc/src/migration-guide/index.rst` ([RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate)). Step 19 keeps only the contract comments and the architecture rule.

The "split" steps 2, 7 and 13 keep the behavior exactly. In those steps, a stateful module keeps its state change inside its now `const` hook, through a local cast that the next step of that module removes. This keeps each commit buildable, and it lets each behavior change be measured alone. The local casts are internal to this series, so D-3 is not affected.

| # | Purpose | Change | Expected behavior | Verification |
| --- | --- | --- | --- | --- |
| 1 | A tool that checks R1 and R2 | `tests/queueing/lib/QueryProbe.{ned,h,cc}` and `Makefile` (section 7.1); `QueryProbe_1.test` with a chain of pure modules (`PacketClassifier` push, `PriorityScheduler` pull, `PacketFilter` push) | Passes on `master` | The new test, in debug mode |
| 2 | Split the classifier hook | `classifyPacket() const`, `handlePacketClassified()`, remove the base cast, latch pull streams; the four stateful classifiers keep their state change through a local cast | No behavior change | All queueing tests; `tests/module/diffserv_*`; fingerprints of `tutorials/queueing`, `examples/diffserv`, `showcases/general/diffserv`, `showcases/tsn/streamfiltering`, `ethernet.csv`: no row moves |
| 3 | `TokenBucketClassifier` removes tokens one time (C4) | The comparison in the query; the removal and the signals in the commit | Tokens are removed one time per packet | Probe variants of `TokenBucketClassifier_1`: with 3 repeats, the output differs on the parent commit and is equal after this step |
| 4 | The same for `MultiTokenBucketClassifier` and its NED extensions | as step 3 | as step 3 | Probe variants of `MultiTokenBucketClassifier_1` |
| 5 | `WrrClassifier` round only at the commit | Section 4.5 | Same order without queries | New `WrrClassifier_1.test` with the probe; `tutorials/queueing` rows that use `WrrClassifier` do not move |
| 6 | `DynamicClassifier`, E1 | Section 4.4 | Branches are created on demand; `reverseOrder` is refused | New `DynamicClassifier_1.test` (two branches, probe) and `DynamicClassifier_2.test` (the refusal message) |
| 7 | Split the scheduler hook | `schedulePacket() const`, `handlePacketScheduled()`, remove the base cast, latch push streams and set `inProgressStreamId`; `WrrScheduler` keeps its local cast; inet-gpl temporary commit: HTB and HLS become `const` with a local cast | No behavior change | Queueing tests; the fingerprints of step 2; inet-gpl build and its 19 rows: no row moves |
| 8 | `WrrScheduler` round only at the commit | Section 4.5 | Same order; in pull mode no query calls the hook yet | Fingerprints of `examples/diffserv`, `showcases/general/diffserv`, `tutorials/queueing`: no row moves (A-3) |
| 9 | `LabelScheduler` skips an empty input | A null check | No crash when an input is empty | New `LabelScheduler_1.test` with an empty input |
| 10 | The scheduler offers the packet that it serves (#1317, C3) | `canPullPacket()` and `canPullSomePacket()` of section 4.3 | R1 for each scheduler | New `WrrScheduler_1.test`: the scenario of #1317, from the test of #1319, with the probe; probe tests for `PriorityScheduler` with `reverseOrder` and for `ContentBasedScheduler`; fingerprints as in step 8, where rows can move (section 7.3) |
| 11 | HTB in inet-gpl | D-9 | `canPullPacket()` works | The 16 HTB rows; a probe run of one HTB configuration |
| 12 | HLS in inet-gpl | D-9 | R1 | The 3 HLS rows; a probe run of one HLS configuration |
| 13 | Split the filter commit | `handlePacketFiltered()`; `matchesPacket()` one time per packet; latch push streams; the stateful filters keep their state changes in place | No behavior change outside streams | Queueing and protocol element tests; fingerprints of the TSN showcases (`cutthroughswitching`, `framepreemption`, `framereplication`, `streamfiltering`), `examples/ethernet/vlan` and a sample of `mrp.csv`; `tests/module/EthernetFragmentFcsChecker_1.test` (A-1, A-2) |
| 14 | No drops in filter queries (C5) | D-8 for `PacketFilterBase`; D-10 | Drops occur when the packet becomes available | `Filter_2.test` with the probe; its expectation changes, so it needs approval; the runs of A-4 |
| 15 | `OrdinalBasedDropper` counts at the commit | Section 4.5 | Same drops | Probe variant of `OrdinalBasedDropper_1`; `tests/protocol/element/AcknowledgeRetransmit.test` |
| 16 | `StreamMerger` records at the commit | Section 4.5 | Same merges | The 5 `framereplication` rows; `tests/validation/tsn/framereplication` |
| 17 | `RedDropper` without a query state | Section 4.5, D-12 | Same RED algorithm; the random stream changes | Probe variant of `RedDropper_1`; the RED rows move (section 7.3) |
| 18 | `StatisticalRateLimiter` without a draw in a query | Section 4.5, D-12 | Same probabilities; the random stream changes | `Meter_1.test`; the 2 `streamfiltering/statistical` rows move |
| 19 | State the rules (C6) | The contract comments; `AR-QUEUE-QUERIES` | — | `check-seals.sh`; the gates before the push |
| 20 | Close the plan | Move this file to `plan/done/` ([PR-SPLIT-MOVE](../../doc/project/rule/pull-request.md#pr-split-move)) | — | — |

**Order and dependencies:**

- Step 1 comes first, because steps 3 to 18 use the probe.
- Steps 3 to 6 need step 2. Steps 8 to 12 need step 7.
- Step 10 needs steps 8 and 9. Before step 10, the query of the base class does not call `schedulePacket()`. After step 10 it does, so a stateful `WrrScheduler` or the crash of `LabelScheduler` would then show in each query.
- Steps 11 and 12 need step 10 and D-9.
- Steps 15 to 18 need steps 13 and 14.

## 7. Verification methods and expected results

### 7.1 The probe module

`QueryProbe` is a module for tests only. It has one input and one output. It passes packets through in push and in pull mode, and it forwards the notifications in both directions. Its parameter `numRepeats` defaults to 3.

- **Push:** for each query, the probe asks its consumer `numRepeats + 1` times. It stops the simulation if the answers differ. Before each push, it asks `canPushPacket()` `numRepeats` more times.
- **Pull:** for each query, the probe asks its provider `numRepeats + 1` times. All answers must be the same packet. Before each pull, it asks `canPullPacket()` `numRepeats` more times and remembers the answer. After the pull, it stops the simulation if the pulled packet is not the offered packet (R1).
- **No side effects:** the probe emits nothing and schedules nothing.

**Placement.** The probe sits on the query side of the module under test: in front of a classifier or a push filter, and behind a scheduler or a pull filter. Do not put a probe directly behind a `PassivePacketSource`, because that source creates its packet in a query (F1).

**Test pairs.** Each probe test is a pair of files that differ only in `numRepeats` (0 and 3). Both files have the same expected output: the order of the packets and the final counts. On the parent commit of a fix, the variant with 3 repeats fails. After the fix, both variants pass.

### 7.2 Commands

These commands run from the root of the worktree. Build the INET library in the mode of the test first ([run-the-gates.md](../../doc/project/guide/run-the-gates.md)).

```sh
make -j$(nproc) MODE=debug
bin/inet_run_queueing_tests -m debug -f '<module>'        # selector form used in #1319; check it in the worktree
( cd tests/fingerprint && ./fingerprinttest -d -F tyf tutorials.csv -m 'tutorials/queueing' )
( cd tests/fingerprint && ./fingerprinttest -d -F tyf examples.csv -m 'diffserv|inet/(redmarker|dctcp|netperfmeter)' )
( cd tests/fingerprint && ./fingerprinttest -d -F tyf showcases.csv -m 'showcases/(tsn|general/diffserv)/' )
```

For inet-gpl, the proposed command is the following. It is not checked yet:

```sh
( cd tests/fingerprint && ./fingerprinttest -d -F tyf examples.csv -m 'examples/(htb|hls)' )
```

Before each push, run the gates of [run-the-gates.md](../../doc/project/guide/run-the-gates.md#before-every-push): both build modes, `check-architecture.sh`, `check-naming.sh`, `check-pull-request.sh`, `check-interfaces.sh` and `check-source-seals.sh`.

Run `inet_run_statistical_tests -m release -w '<directory>'` for each directory whose fingerprint moves. Its statistical baseline then moves too, and [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md) governs the change.

### 7.3 Expected moves of recorded results

These are predictions, not results. Each moved set goes to the maintainer with its measured rows and its reason before any regeneration ([TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)).

| Step | Rows or expectations that can move | Reason |
| --- | --- | --- |
| 3, 4 | none known; a row moves only if a module asks `canPushPacket()` before it pushes into a token bucket classifier | the second removal of tokens stops |
| 10 | rows where a module reads the offered packet of a WRR, content, label or function scheduler, or of a priority scheduler with `reverseOrder`; candidates: `tutorials/queueing`, `examples/diffserv`, `showcases/general/diffserv`. `EthernetQosQueue` rows are not expected to move, because a priority scheduler without `reverseOrder` already offers the correct packet. | the offered packet becomes the served packet |
| 11, 12 | the inet-gpl HTB and HLS rows, depending on D-9 | the search runs at another time |
| 14 | `tests/queueing/Filter_2.test` | the drops occur when the packet becomes available |
| 17 | `examples/diffserv/onedomain` Exp21, Exp22, Exp23, Exp31; `examples/inet/netperfmeter` (4); `examples/inet/redmarker`; `examples/inet/dctcp` (2); `tutorials/queueing` RedDropper; possibly `RedDropper_1.test` | the variate is drawn in advance, so the random stream changes |
| 18 | `showcases/tsn/streamfiltering/statistical` (2); possibly `Meter_1.test` | the same |

If a row moves in a "split" step (2, 7 or 13), or in a step that the table does not name, stop. Find the cause before you continue.

### 7.4 Limits of the evidence

- **The map of fingerprint rows comes from a text search.** The search followed the `extends` chains of the ini files and searched NED files by directory. It did not evaluate overrides, and it cannot see modules that a compound module contains by default. The full list is in `/var/tmp/claude-queueing-coverage.txt`, which is a local file and not part of the repository.
- **Fingerprints show only behavior that changes the trajectory.** A double removal of tokens that does not change a decision is invisible to them. The probe tests cover that case.
- **Gaps in the tests.** No test covers `WrrClassifier`, `WrrScheduler`, `DynamicClassifier`, `LabelScheduler` or the scheduler push streams today. Steps 5, 6, 9 and 10 add tests for the first four. Scheduler push streams stay without a test, because `MarkovScheduler` is the only push scheduler and it overrides the push operations.
- **Release mode.** Release mode is a build check only, unless a step shows a dependency on the mode.

## 8. Risks, open decisions and approval scope

**Decisions:**

- The maintainer decided D-6 and D-9 on 2026-10-14. No decision is open.
- **D-7, D-8 and D-10 to D-14** are proposals. Approval of this plan accepts them.
- D-6 needs an action outside this plan: the author of #1122 rebases it onto this work and keeps the parts that D-6 lists.

**Risks:**

- **More rows can move than section 7.3 lists**, because the map of rows comes from a text search. Mitigation: the rule at the end of section 7.3.
- **HTB and HLS can change their schedules (D-9).** Mitigation: the inet-gpl rows, and method (b) as the fallback.
- **The break (D-3) stops users who have their own schedulers or classifiers.** Mitigation: the migration note in `WHATSNEW`.
- **A callback into a scheduler during its pull can see the state before the commit (A-3).** Mitigation: the check in step 8.

**Permissions:**

- Approval of this plan does not approve the regeneration of any fingerprint row, statistical baseline or test expectation. Each moved set needs its own approval after measurement.
- The plan does not touch the only sealed source path (`src/inet/common/packet/`).
- The new rule `AR-QUEUE-QUERIES` changes an architecture document, which carries no seal flags. The maintainer approves its text as part of this plan.

**Requested approval scope:** steps 1 to 20.

## 9. Progress

The implementation records each step here when its commit exists: the commit, what the implementation found, and the evidence.

| Step | State | Commit | Notes and evidence |
| --- | --- | --- | --- |
| 1 | done | `94374d58e3` | `QueryProbe` and `QueryProbe_1.test`. The lib needs `package.ned` with `@namespace(inet)`, because the `package.ned` of an opp_test test gives the default package the namespace of the test. With a `WrrScheduler` in place of the priority scheduler, the probe stops the run at 1 s: it offers `producer1-1` and serves `producer2-0`. |
| 2 | done | `bb4ea7600c` | `MultiFieldClassifier::classifyPacket()` uses a copy of each filter, because `matches()` keeps intermediate results in the filter. The diffserv classifiers and `MarkovClassifier` call the commit hook in their own `pushPacket()` and `pullPacket()`. Evidence: 58 queueing tests pass; 612 debug module tests pass; the fingerprint subsets equal the base (section 9.1). |
| 3 | done | `0c6e691756` | Before the fix, `TokenBucketClassifier_2` sent all eight packets to the red consumer. |
| 4 | done | `38349125c9` | Before the fix, `MultiTokenBucketClassifier_2` sent all eight packets to the red consumer (checked with the parent version of the two files). |
| 5 | done | `6228f5f6b6` | The weights of the test are 1 and 3. With the weights 1 and 2, a round has 3 packets, and 4 classifications for each push (3 queries and the push) move the round as far as 1, so the test could not fail. Before the fix, all seven packets went to the second consumer. |
| 6 | done | `3048f2e591` | An active source cannot feed a `DynamicClassifier` that has no branch, because the classifier answers `canPushSomePacket()` with no (the back-pressure defect that D-6 leaves in #1122). The tests feed it through a queue and a `PacketServer`, whose `canPushPacket()` query creates the branch (E1). Before the fix, `reverseOrder = true` made four branches for two classes, and `producer-0` and `producer-1` of different classes went through `branch[1]`. |
| 1–6 | evidence | — | After step 6: 65 queueing tests pass; 612 debug module tests pass; the fingerprint subsets equal the base (section 9.1). |
| 7 | done | `c40b2bd166`; inet-gpl `d41241b` | Push streams through `PacketSchedulerBase` do not work on `master`: `pushPacketStart()` records no stream, and `pushPacketEnd()` asserts one. No module uses them, because `MarkovScheduler` overrides them. So step 7 calls the commit hook at `pushPacketStart()` and leaves the rest as it is; a repair is a separate fix ([PR-SPLIT-DRIVEBY](../../doc/project/rule/pull-request.md#pr-split-driveby)). This revises the step 7 row of section 6 ("latch push streams"). The `VoipStream` feature is now built in the worktree, and the six VoIP diffserv rows pass the recorded values. inet-gpl: 8 HTB TCP rows fail on `~tND` at INET `master` with inet-gpl `master` already (checked in temporary worktrees), with the same actual values as at step 7. Evidence: 65 queueing tests, 612 module tests, the fingerprint subsets and the 19 inet-gpl rows equal the base. |
| 8 | done | `f233bee38a` | Refactor: in pull mode no query reaches `schedulePacket()` before step 10. The WRR rows of `tutorials/queueing` and diffserv (VoIP rows included) do not move, so A-3 holds there. |
| 9 | done | `c68201da6b` | Before the fix, `LabelScheduler_1` stops with a segmentation fault. `PacketLabeler` marks only packets that a filter function matches, so the test registers a function that matches all packets. |
| 10 | done | `2dc5e58175` | No row of the fingerprint subsets moves (prediction of section 7.3: candidates only). The function of `Scheduler_1` counted its own calls with a `static` counter, so it broke R2; it now selects by the offered packets and keeps the expected output. Before the fix: `WrrScheduler_1` passes 20 packets for 16000 tokens; the probe stops `WrrScheduler_2` and `QueryProbe_2`. Evidence: 69 queueing tests, 612 module tests, the subsets, and the 19 inet-gpl rows (equal to the base: HLS gets no `canPullPacket()` call in its examples). |
| 11 | done | inet-gpl `dc52f45` | D-9 method (a). The `getNumPackets()` query of HTB scheduled a polling timer 100 µs ahead; HTB now schedules `classModeChangeEvent` at the first `nextEventTime` of its waiting classes (at least 1 ns ahead), updates the decision there, and notifies its collector when a queue reports a packet. The search brings the modes of all levels up to the current time first; the mode at a time depends only on the tokens and the time since the checkpoint. All 16 HTB rows move (section 9.2). In `scenarioUDP1`, `scenarioUDP3` and `scenarioTCP1`, each flow receives the same packets within one packet. A probe behind the HTB scheduler of `serverFDO` (30304 packets) finds no difference between offer and pull, and does not change the result. |
| 12 | done | inet-gpl `648557c` | D-9 method (a). The 3 HLS rows move in `tplx` and `~tND`; `~tNl` stays. In Experiment1, Experiment2L and Experiment3, each flow receives the same packets within two packets. A probe behind the HLS scheduler in Experiment1 (29311 packets) finds no difference between offer and pull, and does not change the result. |
| 7–12 | evidence | — | See the rows above. The commit hooks of HTB and HLS assert that the decision did not change during the pull (A-3); no run met the assertion. |
| 13 | done | `469e6b2c0f` | No filter subclass overrides the push or pull operations. A-1 and A-2 hold: the cut-through, frame preemption, frame replication and stream filtering showcases, `examples/ethernet/vlan`, the 114 MRP rows, the Ethernet and `element` protocol suites and `EthernetFragmentFcsChecker_1` keep their results. |
| 14 | done | `15f9b03d13` | D-8 as planned. The expected output of `Filter_2` changes (the drop of `provider-1` moves from 1 s to 0 s; the sink gets the same packets at the same times); the commit records it. New test `Filter_4` (probe behind a pull filter). A-4 holds: the `Lifetime` protocol tests and the TSN shaper showcases keep their results. |
| 15 | done | `cba0808f98` | A probe cannot show the defect, because the queries were already pure in push mode. The defect is in streams: the new test `OrdinalBasedDropper_2` pushes each packet as a stream, and before the fix the sink got `producer-6` and `producer-8` too. `dropPacket()` keeps only the log lines, so that `OrdinalBasedDropper_1` keeps its expected output. |
| 16 | done | `6fe23d5877` | No configuration and no test streams through a `StreamMerger`; the 6 frame replication rows keep their results. No new test. |
| 17 | done | `f0c3ea31ee` | D-12. New tests `RedDropper_2` (no query) and `RedDropper_3` (probe): before the fix the probe changed the drops; after it both drop the same 26 packets. The advance draw shifts the random sequence, so the commit records the new expected output of `RedDropper_1` and nine fingerprint rows (section 9.2). |
| 18 | done | `eb514d21bb` | D-12. New tests `StatisticalRateLimiter_1` and `_2` pull from a source that tags its packets. Before the fix, the probe stopped `_2` ("A repeated canPullPacket() offers another packet"), and `_1` stalled after 4 events. After it both collect 21 and drop 15 packets. The commit records one moved fingerprint row (section 9.2). |
| 19 | done | `67198b901b`, `e4073332b4` | Two commits, because the contract comments are in `src` and the rule is in `doc`. The seal index counts the new rule unit. |
| 13–19 | evidence | — | Head of the series: debug and release build without undefined symbols; all 1752 fingerprint rows of all csv files run in the series tree and in a base tree at `master`: exactly 10 rows differ (section 9.2), all from steps 17 and 18. Gates: the series gates pass; `check-source-seals.sh` passes; `check-architecture.sh`, `check-naming.sh` and `check-interfaces.sh` fail with the same findings as `master`. Queueing tests: 75 pass with the new expectations. Debug module tests: 612 pass. All protocol suites: the same verdicts as at `master` (375 pass, 48 expected failures, 67 unexpected failures that exist at `master`). inet-gpl: the 19 rows give the values of steps 11 and 12. |
| 14–18 | values | — | After the approval, the commits of steps 14, 17 and 18 (INET) and 11 and 12 (inet-gpl) were rewritten to carry their new values ([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)); the tree of the series head did not change otherwise. The commits of steps 1 to 13 kept their hashes. |
| 20 | done | — | The plan moves to `plan/done/` in the last commit. The series is not pushed and does not land before the review. |

**Test lessons.** `opp_test` checks `%contains-regex` with `MULTILINE` and `DOTALL`. A `.*?` can therefore cross lines, and a long multi-line expectation that does not match backtracks for many minutes. The probe tests use one check for each consumed packet, anchored with `^`, with `[^\n]*` inside the line.

### 9.1 Fingerprint evidence

The subsets of section 7.2 run in debug mode with `-F tyf`. The base is commit `94374d58e3` (no source change).

| Subset | Rows | Base | After step 2 | After step 6 |
| --- | --- | --- | --- | --- |
| `tutorials.csv` `tutorials/queueing` | 47 | 47 pass | 47 pass | 47 pass |
| `examples.csv` (diffserv, redmarker, dctcp, netperfmeter, tcpapp, udpapp, ethernet/vlan) | 36 | 33 pass, 3 errors | the same | the same |
| `showcases.csv` (tsn, general/diffserv) | 47 | 43 pass, 4 errors | the same | the same |
| `ethernet.csv` | 160 | as expected | as expected | as expected |

### 9.2 Recorded values that move (approved)

| Rows | Step | Reason | Evidence that the new values are right |
| --- | --- | --- | --- |
| inet-gpl `examples/htb`, all 16 | 11 | The HTB search runs at its own events; the class mode timer fires at the next mode change instead of a 100 µs poll; a packet at an idle scheduler leaves without the poll delay. | Per-flow received packets equal within one packet in three scenarios; probe run passes. |
| inet-gpl `examples/hls`, all 3 | 12 | The HLS search runs when the input changes, not when the collector pulls. | Per-flow received packets equal within two packets in all three; probe run passes. |
| `examples/diffserv/onedomain` Exp21, Exp22, Exp23, Exp31; `examples/inet/dctcp` DcTcpIncast, TcpRenoIncast; `examples/inet/netperfmeter` SCTP-with-OLIA, TraceFile-Sctp (`~tND` only); `tutorials/queueing` RedDropper | 17 | RED draws its random number in advance (D-12): one more draw at initialization, and each later draw at the commit of the decision before. The shared random sequence shifts. | The RED algorithm and its probabilities do not change; `RedDropper_2` and `RedDropper_3` show that a query no longer changes the drops. |
| `showcases/tsn/streamfiltering/statistical` StreamFiltering | 18 | The same for `StatisticalRateLimiter`. | `StatisticalRateLimiter_1` and `_2` show that a query no longer changes the drops. |
| `tests/queueing/Filter_2.test` expected output | 14 | Drops in pull mode occur when the packet becomes available. | `Filter_4` shows the same drops with a probe. |
| `tests/queueing/RedDropper_1.test` expected output | 17 | The shifted random sequence changes all times of the test. | as for the RED rows |

**Statistical baselines.** A copy of the GitHub "Test: statistical" job (ubuntu 24.04, clang 16, omnetpp-6.x `cf58891643`) regenerated all statistics at INET `master` `f269c3a26c` and at the series head; 147 configurations are stale at `master` already. Exactly 9 configurations differ between the two:

| Configuration | Cause | Statistics commit (branch `topic/queueing-pure-queries` of the `statistics` repository) |
| --- | --- | --- |
| `tutorials/queueing` Filter2 | step 14: the source creates the next packet earlier | `ae4a560` |
| `examples/diffserv/onedomain` Exp21, Exp22, Exp23; `examples/inet/netperfmeter` SCTP-with-OLIA; `tutorials/queueing` RedDropper | step 17: the advance draw of RED | `c8e6493` |
| `showcases/tsn/streamfiltering/statistical` StreamFiltering | step 18: the advance draw of the rate limiter | `c4df49b` |
| `examples/inet/dctcp` DcTcpIncast, TcpRenoIncast | step 17 | not updated: the values are stale at `master` already (the TCP stages); whoever regenerates them after this series gets the right values |

The errors are not results: the three `diffserv/simple_` VoIP rows of each csv need the `VoipStream` feature, and `tsn/gatescheduling/sat` needs `Z3GateScheduleConfigurator`. Neither is built here. So the rows that run a `WrrScheduler` behind a VoIP source are not covered locally.
