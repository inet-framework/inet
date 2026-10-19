# Implementation plan: the acceptance filter in the pull handshake, and the selective dequeue

Checkout and revision: `inet-master`, branch `master` at `1b5dba65d3`. Two untracked files at the repository root (`ieee80211-edca-management-recovery.md`, `ieee80211-txop-boundaries-and-rate-selection.md`) are not related to this plan.
Plan revision: draft 3, 2026-10-16. Draft 2 records the decisions of the owner (section 8). Draft 3 records the work (section 9): the fingerprint check compares before with after (section 7), and the internal code follows D-6, which the owner asked for after a review of the first internal names.
Approval: the owner approved the implementation of steps 1 to 5 of draft 2 on 2026-10-16 ("let's implement it"), on the branch `topic/queueing-filtered-pull` from `master` at `8f45b42ca2`. The owner decided D-4, D-5, the test category and the recommendation for the MAC on the same day.
Status: landed on master on 2026-10-19 with the approval of the owner, as a fast-forward of master `ee4afb459d` to `538ce90a74`; not pushed. Section 9 records the work. The use in the IEEE 802.11 MAC belongs to pull request #1343, which replaces its own first commit with this API.
Consumer: pull request inet-framework/inet#1343, branch `work/compressed-blockack-s3b`. Its first commit `45f4da2e28` ("queueing: add: preserve pull effects for selected packet extraction", parent `bba1db4c81`) solves the same problem in a different way. This plan replaces that commit.

## 1. Problem and intended behavior

A pull consumer cannot always accept every packet. For example, a MAC sends to two peers. The head of its queue goes to peer A, but peer A cannot receive now. The next packet goes to peer B, which can receive. The MAC can only take the head, or wait. So the packet for peer B waits behind the packet for peer A. This is head-of-line blocking.

Pull request #1343 meets this case in IEEE 802.11. The head data frame waits for the ADDBA response of peer A. A later frame for peer B, with the same access category, is eligible. The MAC must serve the frame for peer B and keep the frame for peer A in the queue.

The push direction of the queueing model already has the matching mechanism. The passive sink answers `canPushPacket(packet, gate)` for a specific packet. The active source holds the packets, so it can try each of them. The pull direction has no counterpart. The active sink is the caller, but the packets are at the callee. The sink can see only the packet that the source chooses (`canPullPacket(gate)`). It cannot ask the source to choose among the packets that it accepts.

**Intended behavior.** The active sink gives its acceptance condition to the passive source as an `IPacketFilterFunction`. The source answers and pulls as if it held only the packets that the filter accepts. Every effect is the effect of a normal pull of that packet: ownership, residence metadata, signals, statistics, scheduler state and animation. `IPacketQueue` gets the same operation in its gate-less form, as `dequeuePacket(filter)`.

The symmetry is the reason for the design:

| | Push | Pull |
| --- | --- | --- |
| The caller (active side) | the source, which holds the packets | the sink, which wants a packet |
| The callee (passive side) | the sink | the source, which holds the packets |
| The acceptance condition of the sink | a method of the callee: `canPushPacket(packet, gate)` | **new:** an object from the caller: `filter->matchesPacket(packet)` |
| Where the search happens | at the caller | at the callee |

The acceptance condition always belongs to the sink. The search always happens where the packets are.

## 2. Current implementation and evidence

**The handshake.** [IPassivePacketSource.h](../../src/inet/queueing/contract/IPassivePacketSource.h#L35) declares `canPullSomePacket(gate)`, `canPullPacket(gate)` and `pullPacket(gate)`. `canPullPacket(gate)` returns the packet that the next `pullPacket(gate)` returns. [IPassivePacketSink.h](../../src/inet/queueing/contract/IPassivePacketSink.h#L50) declares `canPushPacket(packet, gate)`, the acceptance test of the push direction.

**The queue API.** [IPacketQueue.h](../../src/inet/queueing/contract/IPacketQueue.h#L38) declares `dequeuePacket()`. [PacketQueueBase.cc](../../src/inet/queueing/base/PacketQueueBase.cc#L49) implements it as `pullPacket(outputGate)` plus `drop()`. `PacketQueueBase` is the only direct implementer of `IPacketQueue`.

**The effects of a pull, for each layer:**

- [PacketQueue::pullPacket](../../src/inet/queueing/queue/PacketQueue.cc#L118) takes the front packet out. It adds a `PEK_QUEUED` packet event and the `QueueingTimeTag`, emits `packetPulled`, and animates the pull.
- [PacketSchedulerBase::pullPacket](../../src/inet/queueing/base/PacketSchedulerBase.cc#L162) calls `schedulePacket()`, pulls from that input, calls `handlePacketScheduled()` and `handlePacketProcessed()`, and emits `packetPulled`.
- [CompoundPacketQueueBase::pullPacket](../../src/inet/queueing/queue/CompoundPacketQueueBase.cc#L74) pulls from its provider and emits `packetPulled`.

The queue statistics `queueingTime`, `outgoingPackets` and `outgoingDataRate` use `packetPulled` only ([PacketQueue.ned](../../src/inet/queueing/queue/PacketQueue.ned#L78)).

**The scheduler decisions.** Each scheduler tests its inputs through the pull side. [PriorityScheduler::schedulePacket](../../src/inet/queueing/scheduler/PriorityScheduler.cc#L81) uses `canPullSomePacket()` in priority order (`getInputGateIndex()` applies `reverseOrder`). [WrrScheduler::schedulePacket](../../src/inet/queueing/scheduler/WrrScheduler.cc#L68) uses `canPullSomePacket()`. It updates its credit in [handlePacketScheduled](../../src/inet/queueing/scheduler/WrrScheduler.cc#L92). `ContentBasedScheduler` and `LabelScheduler` use `canPullPacket()`.

**Removal is disposal, not service.** [PacketQueue::removePacket](../../src/inet/queueing/queue/PacketQueue.cc#L139) emits `packetRemoved` and records no residence metadata. Most callers dispose of the packet: lifetime expiry in [Hcf.cc](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc#L175), carrier loss in [CarrierBasedLifeTimer.cc](../../src/inet/protocolelement/lifetime/CarrierBasedLifeTimer.cc#L34), and overflow in [CompoundPacketQueueBase.cc](../../src/inet/queueing/queue/CompoundPacketQueueBase.cc#L64). The `packetRemoved` signal does not say why the packet left. So a statistic cannot use it to separate a send from a drop. Thus `removePacket()` cannot also be the selective dequeue.

**The 802.11 consumer.** The EDCAF pending queue is an `IPacketQueue` ([Edcaf.h](../../src/inet/linklayer/ieee80211/mac/channelaccess/Edcaf.h#L48)). The MAC dequeues with `dequeuePacket()` ([OriginatorQosMacDataService.cc](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc#L117)). INET ships two pending queue types:

- [PendingQueue](../../src/inet/linklayer/ieee80211/mac/queue/PendingQueue.ned) is a `PacketQueue` with a comparator. It is the default.
- [CompoundPendingQueue](../../src/inet/linklayer/ieee80211/mac/queue/CompoundPendingQueue.ned) has a classifier, three `PacketQueue`s and a `PriorityScheduler`. The module test [Ieee80211MgmtApHcfQueueDrop_1.test](../../tests/module/Ieee80211MgmtApHcfQueueDrop_1.test) uses the same structure.

**The solution in pull request #1343.** Commit `45f4da2e28` adds the optional interface `ISelectivePacketCollection` with `peekEligiblePacket(predicate)` and `extractPacket(packet)`. `PacketQueue`, `PriorityScheduler` and `CompoundPacketQueueBase` implement it. A provider that does not support it throws the new exception `UnsupportedPacketSelection`. The MAC finds the interface with `dynamic_cast`, catches the exception, and has a fallback path. This is a second composition path beside the pull path. Only priority scheduling supports it.

**The implementers of `IPassivePacketSource`.** A search of `src/` and `tests/` finds these classes that implement the interface or one of its derived interfaces (`IPacketSource`, `IPacketQueue`, `IPacketPuller`, `IPacketFlow`, `IPacketFilter`, `IPacketMeter`, `IPacketMarker`):

- Base classes: `PassivePacketSourceBase`, `PacketQueueBase`, `PacketSchedulerBase`, `PacketFlowBase` (with `PacketMeterBase` and `PacketMarkerBase` under it), `PacketFilterBase`, `PacketClassifierBase`, `PacketPullerBase`.
- Concrete classes without a passive source base: `EmptyPacketSource`, `PacketDemultiplexer`, `MarkovClassifier` (through `PacketClassifierBase`), `PacketStreamer`, `PacketDestreamer`, `PreemptableStreamer`.
- Test sources: `TestSource` in [PacketGate_1.test](../../tests/module/PacketGate_1.test) and in [PriorityScheduler_1.test](../../tests/module/PriorityScheduler_1.test).
- `PassivePacketSourceRef` wraps the interface for callers.

Simu5G (`../simu5g/src`) implements none of these interfaces.

**The filter type exists.** [IPacketFilterFunction](../../src/inet/queueing/contract/IPacketFilterFunction.h) declares `matchesPacket(const Packet *) const`. [PacketFilterFunction.h](../../src/inet/queueing/function/PacketFilterFunction.h#L16) has a function-pointer adapter. A filter with state, such as the 802.11 eligibility test, implements the interface in a small class.

## 3. Scope and acceptance criteria

**Required.** Part A adds the acceptance filter to the pull handshake, with real support in `PassivePacketSource`, `EmptyPacketSource`, `PacketQueue`, `PriorityScheduler`, `WrrScheduler` and `CompoundPacketQueueBase`. Part B adds `IPacketQueue::dequeuePacket(filter)`. These parts cover both 802.11 pending queue types and compound queues with WRR.

**Acceptance criteria:**

- **AC-1.** `PassivePacketSource` returns its next packet if the filter accepts it, and `nullptr` otherwise. `EmptyPacketSource` returns `nullptr`.
- **AC-2.** `PacketQueue` returns the first packet in queue order that the filter accepts. The pull takes exactly that packet out. It records `PEK_QUEUED` and `QueueingTimeTag` and emits `packetPulled` one time. It transfers the ownership to the caller. The other packets keep their order.
- **AC-3.** `PriorityScheduler` serves the highest-priority input that holds an accepted packet. It respects `reverseOrder`, calls `handlePacketScheduled()` and `handlePacketProcessed()`, and emits `packetPulled` one time.
- **AC-4.** `WrrScheduler` does its normal round over the inputs that hold an accepted packet. It updates its credit as for a normal pull.
- **AC-5.** `CompoundPacketQueueBase` passes the filter to its provider. In a structure like `CompoundPendingQueue`, each layer (compound queue, scheduler, inner queue) emits `packetPulled` one time.
- **AC-6.** The query and the pull agree. If `canPullPacket(gate, filter)` returns a packet, a following `pullPacket(gate, filter)` in the same event, with no other operation between them, returns that packet.
- **AC-7.** A part without support throws `cRuntimeError("Invalid operation")` from the query and from the pull, before any change of state.
- **AC-8.** `dequeuePacket(filter)` on a `PacketQueue` and on a compound queue equals the filtered pull at the output gate plus `drop()`. The ownership goes to the caller.
- **AC-9.** No `I<Stem>` class gets a method body (`check-interfaces.sh`).

**Preserved conditions.** The unfiltered behavior of every part does not change: pull, push, the collection operations, removal, signals, statistics and fingerprints. `removePacket()` keeps its meaning. No part gets new state.

**Out of scope.** The follow-ups are in section 8. They are not necessary for the acceptance criteria.

## 4. Proposed design and reasons

**D-1. The acceptance condition of the sink enters the pull handshake.** This is the pull counterpart of `canPushPacket(packet, gate)` (section 1). In a compound queue, a dequeue is a chain of pulls. Only the pull path applies the effects of every layer, so the filter must travel on that path. These alternatives were considered and rejected:

| Option | Why it is not used |
| --- | --- |
| Iterate with `getPacket()` and remove with `removePacket()` | Removal is disposal. It records no residence metadata, emits `packetRemoved` instead of `packetPulled`, and skips the scheduler effects. |
| Make `removePacket()` the selective dequeue, and keep its signal | Most removals are drops (section 2). A statistic cannot separate a send from a drop. The queue statistics would not show a frame that skips the head. |
| `ISelectivePacketCollection`, as in pull request #1343 | It adds a second composition path beside the pull path. It is an optional capability with a new exception type and `try`/`catch` fallbacks. It supports priority scheduling only, and it does not run the gate checks. |
| Choice by collection order through `getPacket()` | It skips the scheduler and gate effects. `WrrScheduler` has no collection order (its `getPacket()` throws). |
| Put a temporary filter on the inner queues, and then run a normal pull | It leaves hidden state in other modules for the length of a call. |

**D-2. The contract.** These are the declarations and their documentation. The text follows the style of the existing `canPullPacket(gate)` documentation.

```cpp
// IPassivePacketSource
/**
 * Returns the packet that pullPacket(gate, filter) pulls at the given gate,
 * or nullptr if this source holds no packet that the filter accepts. The
 * filter is the acceptance condition of the sink. The result is the packet
 * that canPullPacket(gate) would return if this source held only the packets
 * that the filter accepts.
 *
 * This function is a query: it must not change the state of the simulation.
 * If it returns a packet, then pullPacket() with the same gate and filter, in
 * the same event and with no other operation between the two calls, returns
 * that packet.
 *
 * A source that cannot make this choice throws an error. The gate must be a
 * valid gate of this module and it must support pulling packets. The filter
 * must not be nullptr, and it must be a query.
 */
virtual Packet *canPullPacket(const cGate *gate, const IPacketFilterFunction *filter) const = 0;

/**
 * Pulls the packet that canPullPacket(gate, filter) returns. The effects are
 * the effects of pullPacket(gate) for that packet. The ownership of the
 * packet is transferred to the sink.
 *
 * The source must hold a packet that the filter accepts. The other
 * conditions are the same as for canPullPacket(gate, filter).
 */
virtual Packet *pullPacket(const cGate *gate, const IPacketFilterFunction *filter) = 0;

// IPacketQueue
/**
 * Dequeues the packet that dequeuePacket() would dequeue if the queue held
 * only the packets that the filter accepts. The effects are the effects of
 * dequeuePacket() for that packet. The ownership of the packet is
 * transferred from the queue to the caller.
 *
 * The queue must hold a packet that the filter accepts. The filter must not
 * be nullptr, and it must be a query.
 */
virtual Packet *dequeuePacket(const IPacketFilterFunction *filter) = 0;
```

The filter sees each packet as the queue that holds it stores it. This is the same view that `getPacket()` gives. A query can call the filter many times. A filtered call is about whole packets only. During a packet stream, it gives the same error as an unfiltered pull (`checkPacketStreaming()`).

**D-3. The interfaces hold declarations only. Every Base throws.** [AR-ORG-CONTRACT-PURITY](../../doc/project/rule/architecture.md#ar-org-contract-purity) forbids a method body in an `I<Stem>` class. The default goes into the `*Base` classes. That default is `throw cRuntimeError("Invalid operation")` in the query and in the pull. The streaming operations already work this way for parts that do not support them.

A Base does not supply a "head packet if accepted" fallback. A Base does not know how many packets its subclass holds. A subclass with many packets would then give a quiet wrong answer: it would wait although it holds an accepted packet. That is the quiet failure that [AR-ORG-CONTRACTS](../../doc/project/rule/architecture.md#ar-org-contracts) forbids. The contract declares "unsupported" as a distinct outcome, and the error keeps it distinct.

A concrete class that knows its packets opts in:
- `PassivePacketSource` holds one next packet, so "its packet if accepted" is exact for it.
- `EmptyPacketSource` holds no packet.
- `PacketQueue`, `PriorityScheduler`, `WrrScheduler` and `CompoundPacketQueueBase` make the real choice (sections 6 and 7).

**D-4. The method names are overloads.** `canPullPacket(gate, filter)`, `pullPacket(gate, filter)` and `dequeuePacket(filter)` use the names of the unfiltered operations, because they are the same operations with a condition. INET builds with `-Wno-overloaded-virtual` ([src/makefrag](../../src/makefrag#L103)). C++ name hiding affects only a call through a subclass type that overrides one overload. The callers use the interfaces and `PassivePacketSourceRef`.

**D-5. `IPacketQueue` gets no new query.** A caller uses `canPullPacket(gate, filter)` at the output gate of the queue. The 802.11 MAC already looks up that gate in pull request #1343. This keeps the contract minimal ([AR-EXT-MINIMAL-SURFACE](../../doc/project/rule/architecture.md#ar-ext-minimal-surface)). The owner accepted this (section 8).

**D-6. The internal code reuses the current names and structure.** The code adds no internal name of its own. The owner rejected a first set of helper names in review, because they did not fit the symmetric names of the queueing bases:

- **The scheduler decision is an overload.** `PacketSchedulerBase` gets the protected overload `schedulePacket(filter)` and its checked call `callSchedulePacket(filter)`. They mirror `schedulePacket()` and `callSchedulePacket()`, as `classifyPacket(packet)` is the decision of a classifier with an argument. The filtered query uses `schedulePacket(filter)`, and the filtered pull uses `callSchedulePacket(filter)`, as the pair without a filter does. The base version of `schedulePacket(filter)` throws an error. Each scheduler owns its decision, so the extension point is a protected virtual method ([AR-EXT-VIRTUAL-IS-A-PROMISE](../../doc/project/rule/architecture.md#ar-ext-virtual-is-a-promise)).
- **The filtered pull of a scheduler carries its own effects.** Its block of effects repeats the one of `pullPacket()`, as `pullPacket()`, `pullPacketStart()` and `pullPacketEnd()` already each carry their own.
- **A scheduler decides in one place.** `PriorityScheduler` and `WrrScheduler` decide only in `schedulePacket(filter)`, and their `schedulePacket()` calls it with `nullptr`, which means that every packet matches. Without a filter, an input counts if `canPullSomePacket()` is true, as before. The schedulers cannot use a filter that matches every packet instead, because the decision passes the filter on to the inputs: an input without support for the filter, such as the gate in a `GatingPriorityQueue`, throws an error, and `canPullPacket()` of a `PassivePacketSource` creates the next packet, which would change the fingerprints. Comments at `anyPacket` and at `schedulePacket(filter)` give these reasons. `WrrScheduler` updates its credit in the current `handlePacketScheduled()`, so the round stays correct.
- **The pull of a queue without a filter is the pull with a filter that matches every packet.** `PacketQueue::pullPacket(gate)` calls `pullPacket(gate, &anyPacket)`, with a file-local `static const CPacketFilterFunction`. The first packet that it matches is the front of the queue, so the behavior does not change, and the effects of a pull stay in one place.

**D-7. No new state, signal or parameter.** The filter lives only for the length of one call, in the caller. Each filtered pull emits the signals of the normal pull ([QR-DESIGN-MINIMAL](../../doc/project/rule/quality.md#qr-design-minimal)).

**D-8. Removal does not change.** `removePacket()` stays the operation for a packet that leaves without service. The selective dequeue is the service path.

## 5. Affected components and compatibility

| Component | After step 1 | Real choice in |
| --- | --- | --- |
| `IPassivePacketSource`, `IPacketQueue` | new pure virtual declarations | steps 1 and 5 |
| `PassivePacketSourceRef` | forwards `canPullPacket(filter)` and `pullPacket(filter)` | step 1 |
| `PassivePacketSource`, `EmptyPacketSource` | exact | step 1 |
| `PacketQueueBase` | throws; `dequeuePacket(filter)` in step 5 | — |
| `PacketQueue` (and `InProgressQueue` under it) | throws | step 2 |
| `PacketSchedulerBase` | throws | step 3, for `PriorityScheduler` and `WrrScheduler` |
| `CompoundPacketQueueBase` | throws | step 4 |
| `PacketFlowBase`, `PacketFilterBase`, `PacketClassifierBase`, `PacketPullerBase`, and the classes under them | throw | follow-up F-1 |
| `PacketDemultiplexer`, `PacketStreamer`, `PacketDestreamer`, `PreemptableStreamer` | throw | follow-up F-1 |
| `TestSource` in two module tests | throws | — |

None of the subclasses of `PacketQueue`, `PacketSchedulerBase` or `CompoundPacketQueueBase` in `src/` overrides `pullPacket(gate)` or `canPullPacket(gate)`. So no subclass loses an effect when its base starts to make the real choice. The compound queue in the module test overrides neither of them.

**Compatibility** ([release.md](../../doc/project/rule/release.md)):

- **C++ API.** A class outside INET that implements `IPassivePacketSource` or `IPacketQueue` directly fails to compile until it declares the new methods. This loud break is intended under AR-ORG-CONTRACT-PURITY. A class that extends a Base compiles, and it throws "Invalid operation" on a filtered call. Step 1 adds a migration-guide entry ([RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate)) to [doc/src/migration-guide/index.rst](../../doc/src/migration-guide/index.rst). It also adds a `WHATSNEW` entry ([RR-WHATSNEW](../../doc/project/rule/release.md#rr-whatsnew)).
- **Documentation.** The paragraph about the push and pull handshake in [ch-queueing.rst](../../doc/src/users-guide/ch-queueing.rst#L58) gets one sentence about the acceptance filter of the pull direction. The documentation calls it the *acceptance filter*, so that a reader does not confuse it with the queueing filter element `IPacketFilter`.
- **NED, signals, parameters and results.** None change. Nothing in INET calls the filtered operations until pull request #1343 does. So no fingerprint and no statistic changes.

## 6. Implementation steps and dependencies

The work happens in a dedicated worktree on the branch `topic/queueing-filtered-pull`, from `master`. Each step is one commit, or a refactor commit and a behavior commit. Each commit has a `Plan:` line ([PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan)).

| Step | Purpose | Files and responsible component | Proposed change | Expected behavior | Verification |
| --- | --- | --- | --- | --- | --- |
| 1 | Add the contract. An interface change and its implementations must build together, so this is one commit. | `IPassivePacketSource.h`, `PassivePacketSourceRef.h`, every Base and direct implementer in section 5, the two module test sources, `PassivePacketSource`, `EmptyPacketSource`, `WHATSNEW`, the migration guide, `ch-queueing.rst` | Two pure virtual declarations (D-2); forwards in the Ref; "Invalid operation" in every Base and direct implementer (D-3); exact behavior in the two sources | AC-1, AC-7 for the parts that throw, AC-9; no unfiltered change | new test `_1` (section 7), all queueing tests, the two changed module tests, fingerprints |
| 2 | `PacketQueue` makes the real choice. | `PacketQueue.h/.cc` | `canPullPacket(gate, filter)`: the first accepted packet in queue order. `pullPacket(gate, filter)` takes it out with the effects of a pull, and `pullPacket(gate)` calls it with a filter that matches every packet (D-6). | AC-2, AC-6 | new test `_1` |
| 3 | The priority and WRR schedulers make the real choice. | `PacketSchedulerBase.h/.cc`, `PriorityScheduler.h/.cc`, `WrrScheduler.h/.cc` | D-6. Without a filter, the decision stays as it is. | AC-3, AC-4, AC-6 | new test `_2` |
| 4 | Compound queues pass the filter on. | `CompoundPacketQueueBase.h/.cc` | Filtered query and pull through `provider`; then `take()` and `packetPulled`, as in `pullPacket(gate)` | AC-5 | new test `_3` |
| 5 | Add the selective dequeue (part B). | `IPacketQueue.h`, `PacketQueueBase.h/.cc`, `WHATSNEW`, the migration guide | Pure virtual `dequeuePacket(filter)`; `PacketQueueBase` implements it as `pullPacket(outputGate, filter)` plus `drop()` | AC-8 | new test `_3` |
| 6 | Document the symmetry of the two handshakes, as the owner asked during the review. | the NED files and C++ class comments of `IPassivePacketSink`, `IPassivePacketSource`, `IActivePacketSource`, `IActivePacketSink`, and the class comment of `IPacketQueue` | Describe which side decides when a packet moves, which side holds and chooses the packets, and that the acceptance condition belongs to the sink: a method of the passive sink in push, a filter from the active sink in pull | no behavior change | the NED files load in the queueing tests; commit gates |

Steps 2 to 5 depend on step 1. Step 4 needs steps 2 and 3 for its tests. Step 5 needs step 1 only, but it comes last so that its test can use a compound queue.

## 7. Verification methods and expected results

These are planned checks, not observed results. Each command runs from the checkout root with a fresh library in the given mode ([run-the-gates.md](../../doc/project/guide/run-the-gates.md)).

**New focused tests (proposed; they do not exist yet).** These are module tests with a small C++ active sink that pulls with a filter. The filter accepts packets by name. The test in commit `45f4da2e28` of pull request #1343 checks storage, ownership, residence metadata and per-layer events. It can be the starting point.

- `tests/module/PacketFilteredPull_1.test`, for steps 1 and 2:
  - `PassivePacketSource` with a filter that accepts and with one that refuses (AC-1).
  - `EmptyPacketSource` (AC-1).
  - A `PacketQueue` with three packets, where the filter refuses the head. The test checks the chosen packet, the order of the rest, `QueueingTimeTag`, `PEK_QUEUED`, one `packetPulled`, and the owner (AC-2).
  - The query and the pull agree (AC-6).
  - A `PacketGate` in front of a queue throws before any change (AC-7).
- `tests/module/PacketFilteredPull_2.test`, for step 3:
  - `PriorityScheduler` over three queues, with and without `reverseOrder` (AC-3).
  - `WrrScheduler` with weights 2 and 1. The filter refuses the input that holds the credit, so the test checks the sequence of inputs and the round (AC-4).
- `tests/module/PacketFilteredPull_3.test`, for steps 4 and 5:
  - The structure of `CompoundPendingQueue`. The test counts `packetPulled` at each layer (AC-5).
  - `dequeuePacket(filter)` on a `PacketQueue` and on the compound queue (AC-8).

```bash
make -j$(nproc) MODE=debug
inet_run_module_tests -m debug -f 'PacketFilteredPull_|PacketGate_1|PriorityScheduler_1|Ieee80211MgmtAp(Hcf)?QueueDrop_1'
```

Check that the selector picks every intended case after the new tests exist. `PacketGate_1` and `PriorityScheduler_1` cover the changed test sources. The two `Ieee80211MgmtAp*QueueDrop_1` tests cover the unfiltered path of both pending queue types.

**Regression of the unfiltered behavior.** Step 1 touches every passive source base, so all 75 queueing tests are directly related. On this host, the runners need `-p inet`; they take the project root from `INET_ROOT`:

```bash
inet_run_queueing_tests -p inet -m debug
```

**Fingerprints.** This work must not change any fingerprint, because no simulation calls the new methods. So the check compares before with after: each case runs in a worktree at the parent of the first code commit, whose sources equal master (`4e0ae31b4d` after the rebase), and in the worktree of the branch, each with its own library and NED files. The check compares the `tplx` fingerprints and also prints the value of the CSV file. The selection is every case of `tests/fingerprint/{tutorials,showcases,examples}.csv` under `tutorials/queueing`, `showcases/tsn` and `examples/wireless` (175 cases). The script is `/var/tmp/claude-qfp/fpba.sh`. Expected result: before and after are the same in every case. No recorded expectation changes ([TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate)). The Python runner `inet_run_fingerprint_tests` is not usable for this check: it reads `tests/fingerprint/store.json`, which was last synchronized with the CSV files on 2026-09-09; 71 later commits changed the CSV files only. For example, the store records `02cf-58d1/tplx` for `examples/wireless/mactest` `-c MacDcf`, while `examples.csv` and master both give `031d-548a/tplx`. CI runs `fingerprinttest`, which reads the CSV files.

**Mechanical gates, before every push:**

```bash
make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release
doc/project/enforcement/check-interfaces.sh
doc/project/enforcement/check-architecture.sh src/inet/queueing
doc/project/enforcement/check-pull-request.sh --base origin/master
```

**What these checks cannot establish:**

- They cannot show that the 802.11 MAC uses the filter correctly. That evidence belongs to pull request #1343.
- They cannot show that code outside INET compiles. The migration guide covers that.
- The fingerprints show that the unfiltered paths do not change. They say nothing about the filtered paths.

## 8. Risks, unresolved decisions, and approval scope

**Decisions of the owner (2026-10-16, in the design conversation):**

- **D-4.** Overloads. Separate names such as `canPullAcceptedPacket()` are not used.
- **D-5.** No gate-less query on `IPacketQueue`.
- **The test category for the new tests.** Module tests, because the consumer is C++ code and not a NED module.
- **The recommendation for the MAC.** The second approach in the risk below: filtered only to skip a blocked head.

**Risks:**

- **An external subclass of `PacketQueue` or `PacketSchedulerBase` that overrides `pullPacket(gate)`.** It keeps its extra effect on the unfiltered path only. Its filtered pull uses the base implementation. The migration guide must say that such a class also overrides the filtered method.
- **A custom 802.11 pending queue with a part that does not support the filter.** For example, a gate, or a `PacketScheduler` with a scheduler function. That queue throws "Invalid operation" when the MAC makes a filtered call. There are two approaches for the MAC:
  - The MAC always uses the filtered dequeue. This is simple, and the error is loud.
  - The MAC uses `dequeuePacket()` when the head is accepted, and the filtered dequeue only to skip a blocked head. Then such queues fail only when a skip is necessary.

  The owner recommends the second approach to the author of pull request #1343. A configuration that never blocks the head then keeps its current behavior with every pending queue.

**Follow-ups, not in this plan:**

- **F-1.** Pass-through parts make the real choice:
  - `PacketFlowBase` passes the filter on.
  - `PacketGateBase` applies `isOpen()` and `canPacketFlowThrough()` to the accepted packet.
  - Review `EligibilityTimeGate` and `BackPressureBarrier`, which override the pull methods.
  - Review the classifiers, the demultiplexer, the pullers and the streamers.
- **F-2.** `ContentBasedScheduler`, `LabelScheduler` and `MarkovScheduler` override `schedulePacket(filter)`. `PacketScheduler` needs a change of `IPacketSchedulerFunction`.
- **F-3.** The A-MSDU subframes leave the pending queue through `removePacket()` ([OriginatorQosMacDataService.cc](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc#L47)). So they are not counted as outgoing packets, and they get no residence metadata. With this plan they can leave through `dequeuePacket()` with a filter that accepts exactly that packet. This changes 802.11 statistics, so it needs its own change.
- **F-4.** An overflow drop in `CompoundPacketQueueBase` emits `packetRemoved` and `packetDropped`, and the compound `queueLength` subtracts both ([CompoundPacketQueueBase.ned](../../src/inet/queueing/queue/CompoundPacketQueueBase.ned#L33)). This was found by reading the source only, not by a run. It is not related to this plan ([PR-SPLIT-DRIVEBY](../../doc/project/rule/pull-request.md#pr-split-driveby)).
- **F-5.** `PriorityScheduler::getPacket()` ignores `reverseOrder`. This is not related to this plan.
- **F-6.** `removeAllPackets(filter)` for disposal. No consumer needs it now.

**Approval scope.** No sealed path is affected: the seal list holds only `common/packet/`. No recorded expectation changes. The adaptation of the 802.11 MAC belongs to pull request #1343 and its author. The approval of this plan covers steps 1 to 5 on `topic/queueing-filtered-pull`.

## 9. Implementation record

The worktree is `../inet-queueing-filtered-pull`. Its debug build started from a copy of a matching build, and only the changed files were compiled again.

| Step | Status | Commit | Evidence |
| --- | --- | --- | --- |
| 1 | done | `d504981c96` | New test `PacketFilteredPull_1` (sources, gate) PASS. `PacketGate_1`, `PriorityScheduler_1` and both `Ieee80211MgmtAp*QueueDrop_1` PASS. 75 queueing tests PASS. Debug build without undefined INET symbols. `check-interfaces.sh` reports 15 classes, the same 15 as on master; no class of this work is among them. |
| 2 | done | `50dffdd1e7` | `PacketFilteredPull_1` PASS with the queue checks. With a query that ignores the filter, it FAILs at "the queue offers the first packet that the filter matches". Focused module tests and 75 queueing tests PASS. |
| 3 | done | `6827d19198` | New test `PacketFilteredPull_2` PASS: both priority orders, the WRR round after a filtered pull, and the refusal of `LabelScheduler`. Without `handlePacketScheduled()` in the filtered pull, it FAILs at "the filtered pull spent the token of the second input, so a new round starts". |
| 4 | done | `c5f385bf28` | New test `PacketFilteredPull_3` PASS through `CompoundPendingQueue`: one `packetPulled` each at the compound queue, the scheduler and the unicast queue. With a compound query that ignores the filter, it FAILs at "the compound queue offers the frame that the filter matches". |
| 5 | done | `f0c439b64b` | `PacketFilteredPull_3` PASS with `dequeuePacket(filter)` on the compound queue and on a plain queue. With a dequeue that ignores the filter, it FAILs. |
| 6 | done | `c003b805c9` | Comments only. The 75 queueing tests and the 3 new tests PASS, so the NED files still load. Commit and classification gates PASS; `check-interfaces.sh` reports the same 15 old findings. |

**Final evidence, at `f0c439b64b` after the rebase onto master `ee4afb459d`:**

- Debug and release builds pass without undefined INET symbols and without new warnings.
- The whole debug module suite: 734 PASS, the 3 new tests among them.
- The 75 queueing tests PASS.
- Fingerprints (section 7), before `4e0ae31b4d` against after `f0c439b64b`, measured on `b793b9d41a`, which differs from `f0c439b64b` only in two comments: 174 of 175 cases calculate the same `tplx` fingerprint before and after, and in each of them the value equals the CSV file. All 43 runnable TSN cases are among them. The case `showcases/tsn/gatescheduling/sat` cannot run in either build, because the build excludes the Z3 feature; its two error texts differ only in the path of the tree. The evidence is in `/var/tmp/claude-qfp/fpba-rb.txt`. Before the rebase, the same comparison on the old base gave the same result.
- `check-commits.sh` and `check-classification.sh` on the series, `check-pull-request.sh --base master`, `check-architecture.sh src/inet/queueing`, `check-links.sh`, `check-seals.sh` and `check-source-seals.sh --base master` PASS. `check-interfaces.sh` and `check-naming.sh` fail with the same 15 and 23 old findings as master; none is in a file of this work.

Facts found during the work:

- **Name hiding.** `PacketQueueBase` declares only the overloads with a filter, so it hid `pullPacket(gate)`, which its `dequeuePacket()` calls. Two `using` declarations make the other overloads visible again. The migration guide tells other projects to do the same.
- **The test runners need `-p inet` on this host.** Without it, the search for a project file climbs to `~/.omnetpp`, which is a directory, and the runner stops.
- **`tests/queueing` holds 75 tests**, not 77 as the first draft said.
- **The fingerprint baseline.** The "before" library is a debug build of the detached worktree `../inet-4e0ae31b4d`. It started from a copy of the build of the branch, and make compiled 222 source files again: the changed ones and the ones that include a changed header. The library holds none of the new methods.
- **The internal names changed after review.** The first version of the series had four internal helpers: `scheduleAcceptedPacket()`, `processPulledPacket()`, `pullQueuedPacket()` and `selectInput()`, with three refactor commits for them. The owner rejected these names on 2026-10-16. The branch was rewritten from step 1 on with the structure of D-6: the refactor commits disappeared, steps 2 and 3 changed, and steps 4 and 5 stayed as they were. The checks that fail with broken code ran again for step 3; steps 2, 4 and 5 keep the checked code unchanged.
- **Step 6 came from the review.** After step 5, the owner asked to describe the symmetry of the push and pull handshakes in the documentation of the core interfaces. The NED files hold the concept, because the C++ class comments point to them; the C++ class comments name the methods.
- **The rebase onto master.** During the work, master dropped the first two plan commits and gained 66 commits. On 2026-10-19 the branch was rebased onto master `ee4afb459d`; it now starts with the three plan commits. Only `WHATSNEW` changed on both sides: the item of this work is now item 55, after the 54 incompatible changes on master. The worktrees held a stale generated pair `TcpUrgentTag_m.h` and `TcpUrgentTag_m.cc`, whose `.msg` master removed; it was deleted before the checks, so neither library contains it.
- **The tests use explicit checks, not `ASSERT`.** A failed check throws an error with its text, so a test fails in a release build too.
- **`PacketFilteredPull_1` uses `InteractiveGate`** for the refusal check, because `PacketGate` needs `openTime` and `closeTime`.
- **`PacketFilteredPull_3` uses the real `CompoundPendingQueue`** with IEEE 802.11 data frames and `MgmtOverMulticastOverUnicastClassifier`, as the test of pull request #1343 does. No generic classifier function sorts packets by name.
- **A filtered query of `PacketSchedulerBase` throws during a packet stream**, with the error text of `checkPacketStreaming()`, because the query is `const` and cannot call that function.
