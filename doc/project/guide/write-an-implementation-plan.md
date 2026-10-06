# Write an implementation plan

> **Kind:** procedure · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [planning.md](../rule/planning.md), [contribute-a-change.md](contribute-a-change.md), [documentation.md](../rule/documentation.md)

Use this procedure for a task that calls for an implementation plan. Its proposed requirements are
in [planning.md](../rule/planning.md). That document states when the draft approval process applies.

## 1. Establish the current behavior

Follow the discovery route in [contribute-a-change.md](contribute-a-change.md).
Record the checkout revision and relevant local changes. Trace the behavior through its actual
caller and effective configuration. Separate observed facts from assumptions under
[PLR-GROUNDED](../rule/planning.md#plr-grounded).

## 2. Define the change

State the intended behavior and acceptance criteria under
[PLR-SCOPE](../rule/planning.md#plr-scope). Mark optional work as a separate follow-up.
Explain compatibility effects through the design review map below.

Establish the required model detail before you select an implementation. Identify the applicable standard conditions, the selected model's supported detail, and the required observable result. Preserve mandatory behavior within that scope. Use [AR-MOD-FIDELITY](../rule/architecture.md#ar-mod-fidelity) to assess changes to the model's level of detail. A hypothetical correction to frame handling in a packet-level radio model does not itself require new hardware processing delays. An existing model limitation does not excuse a new violation of the supported contract.

## 3. Choose the design

Use [PLR-DESIGN](../rule/planning.md#plr-design) to describe component responsibilities.
Compare relevant reuse options before you propose an addition. A useful comparison records these
fields:

| Field | Content |
| --- | --- |
| Proposed addition | The class, interface, signal, parameter, or other mechanism |
| Responsibility | The behavior or information that it owns |
| Required consumer | The caller, observer, or documented external extension |
| Reuse option | The existing mechanism that could meet the need |
| Reason for the choice | The specific limitation of reuse and the cost of the addition |

### State, event, and lifecycle decisions

Use [QR-DESIGN-MINIMAL](../rule/quality.md#qr-design-minimal) to choose local, retained, or derived values from their required lifetime. Check whether existing state already represents the required condition. Assess any new event boundary, random draw, or parameter evaluation before you change the control path. Explain only the decisions that affect the proposed behavior.

For lifecycle work, trace the concrete handlers and inherited behavior under [AR-LIFE-OPERATIONS](../rule/architecture.md#ar-life-operations). Distinguish initialization, ordinary stop/start, crash, and module destruction. Ordinary stop/start uses the existing module; initialization does not repeat. In [OperationalMixin](../../../src/inet/common/lifecycle/OperationalMixinImpl.h), the normal message handler also serves start and graceful stop. For example, a new guard that rejects all messages during graceful stop can prevent a required completion. Check the existing lifecycle state before you add another flag.

Keep reusable resources alive across stop/start when their owner requires reuse. The [UdpBasicApp source](../../../src/inet/applications/udpapp/UdpBasicApp.cc) illustrates a timer that initialization creates and lifecycle start reuses. Its stop handler cancels the timer; its destructor releases it. Use the existing owner's cancellation API when the contract only requires cancellation. See the [lifecycle guide](../../src/users-guide/ch-lifecycle.rst) for supported operations.

Separate existing lifecycle limitations from supported guarantees and defects that the proposed change introduces. For IEEE 802.11, consult the [lifecycle analysis](../design/ieee80211-anatomy.md#7-lifecycle-operations) before you infer support from the node's lifecycle interface. Confirm that analysis against the active source. For example, a MAC with empty stop handlers does not establish that its child timers stop. Include repairs to existing limitations only when the requested behavior depends on them or the task explicitly includes them.

### Design review map

Use only the rows that apply. Each linked document owns its technical requirements.

| Concern | Authoritative rules |
| --- | --- |
| Required behavior, reachable failures, and minimal safeguards | [QR-DESIGN-MINIMAL](../rule/quality.md#qr-design-minimal) |
| Reuse and justified additions | [AR-EXT-REUSE](../rule/architecture.md#ar-ext-reuse) |
| Replaceable roles and interface contracts | [AR-ORG-CONTRACTS](../rule/architecture.md#ar-org-contracts), [AR-ORG-CONTRACT-PURITY](../rule/architecture.md#ar-org-contract-purity) |
| Class or module responsibility | [AR-MOD-COMPOSITION](../rule/architecture.md#ar-mod-composition) |
| Member visibility and virtual methods | [AR-EXT-MINIMAL-SURFACE](../rule/architecture.md#ar-ext-minimal-surface), [AR-EXT-VIRTUAL-IS-A-PROMISE](../rule/architecture.md#ar-ext-virtual-is-a-promise) |
| State and object lifetime | [QR-STATE-OWNER](../rule/quality.md#qr-state-owner), [QR-OBJECT-OWNERSHIP](../rule/quality.md#qr-object-ownership) |
| Initialization and supported lifecycle operations | [AR-LIFE-STAGES](../rule/architecture.md#ar-life-stages), [AR-LIFE-OPERATIONS](../rule/architecture.md#ar-life-operations) |
| Calls, notifications, and observations | [AR-COM-DIRECT](../rule/architecture.md#ar-com-direct), [AR-COM-NOTIFY](../rule/architecture.md#ar-com-notify), [AR-OBS-SIGNALS](../rule/architecture.md#ar-obs-signals) |
| Units and configuration | [QR-CMT-UNIT](../rule/quality.md#qr-cmt-unit), [AR-CFG-INFER](../rule/architecture.md#ar-cfg-infer), [AR-CFG-PARAMS](../rule/architecture.md#ar-cfg-params) |
| Protocol requirements and model detail | [QR-CMT-STANDARD](../rule/quality.md#qr-cmt-standard), [AR-MOD-FIDELITY](../rule/architecture.md#ar-mod-fidelity), applicable [domain rules](../domain/README.md) |
| Compatibility and migration | [release.md](../rule/release.md) |
| Tests and recorded expectations | [testing.md](../rule/testing.md) |
| Commit boundaries and order | [pull-request.md](../rule/pull-request.md) |
| Protected paths and document units | [sealing.md](../rule/sealing.md) |

## 4. Define the steps and verification

Order the steps by their dependencies. Map each step to one commit or a short series under [PLR-PROPORTION](../rule/planning.md#plr-proportion). A commit may cover multiple steps when the plan explains why those changes belong together. For example, a hypothetical interface change and its implementation updates need one commit to preserve a buildable tree. Use prose or this table to explain each coherent change:

| Purpose | Files and responsible component | Proposed change | Expected behavior | Verification |
| --- | --- | --- | --- | --- |
| `<why this step is necessary>` | `<paths and owner>` | `<specific change>` | `<observable result>` | `<case and check>` |

Apply [PLR-VERIFICATION](../rule/planning.md#plr-verification) to the verification column.
Identify the relevant boundaries and failure paths. For a defect, explain how the proposed check
exposes the original failure. Use [run-the-gates.md](run-the-gates.md) for the applicable build and
check procedure.

## 5. Review the plan

Use the checklist below before you request approval. Resolve assumptions that affect the design under
[PLR-GROUNDED](../rule/planning.md#plr-grounded).
Scale the detail under [PLR-PROPORTION](../rule/planning.md#plr-proportion).

Investigation is sufficient when these facts support a concrete decision:

- The applicable requirement and model scope are clear.
- The actual path, responsible owner, and reachable failure or required new behavior are clear.
- The smallest adequate change and direct verification are clear.

Continue investigation when a specific unresolved fact can change that decision. Successful tests for code that does not yet exist are not a prerequisite for a plan.

### Review checklist

Use the plan's existing explanation as the answer to each applicable row. Add evidence or a specific gap where an answer is missing. A separate checklist report is unnecessary when the plan already supplies the answers. This review evaluates a proposed plan; it does not claim that implementation tests pass.

| Review question | Rule |
| --- | --- |
| Can a maintainer assess the proposal without the conversation? | [PLR-READABLE](../rule/planning.md#plr-readable) |
| Which facts support the chosen owner and production path? | [PLR-GROUNDED](../rule/planning.md#plr-grounded) |
| What observable result establishes completion? | [PLR-SCOPE](../rule/planning.md#plr-scope) |
| Why does each significant addition need to exist? | [PLR-DESIGN](../rule/planning.md#plr-design) |
| Which requirement and reachable failure justify each safeguard? | [QR-DESIGN-MINIMAL](../rule/quality.md#qr-design-minimal) |
| Which check reaches each claimed behavior, and what remains unverified? | [PLR-VERIFICATION](../rule/planning.md#plr-verification) |
| Which plan revision and permissions does the approval cover? | [PLR-APPROVAL](../rule/planning.md#plr-approval) |
| Does a revision change an approved decision? | [PLR-REVISION](../rule/planning.md#plr-revision) |
| Is the detail sufficient for the decision and proportionate to the risk? | [PLR-PROPORTION](../rule/planning.md#plr-proportion) |

## 6. Record approval and maintain the plan

Present the concrete plan to the maintainer under
[PLR-APPROVAL](../rule/planning.md#plr-approval). Record the actual approval source and scope.
Keep material revisions subject to [PLR-REVISION](../rule/planning.md#plr-revision).

Store the plan under `plan/pending/` according to
[DR-NAME](../rule/documentation.md#dr-name). Move it to `plan/done/` when the change lands.
Use [PR-MSG-PLAN](../rule/pull-request.md#pr-msg-plan) for commit references.

## Plan template

Choose the compact form below when one explanation covers the owner, path, change, and verification. Use the expanded form when interactions need separate explanation. Both forms follow the same plan rules. The placeholders describe content; they are not evidence or approval.

### Compact worked example

This hypothetical example assumes that an edit removed timer cancellation from `UdpBasicApp::handleStopOperation()`. The linked source currently contains that cancellation. The example proposes a repair; it does not report a current defect or executed tests.

**Checkout and approval:** `<checkout, revision, relevant local edit; plan revision; actual approval source and scope, or pending>`.

**Problem and evidence:** After client shutdown, the send timer must not invoke the application while it is down. The hypothetical edit leaves that timer scheduled. [UdpBasicApp](../../../src/inet/applications/udpapp/UdpBasicApp.cc) creates the timer during initialization and reuses it on start. [OperationalMixin](../../../src/inet/common/lifecycle/OperationalMixinImpl.h) rejects self-messages while the application is down. Thus, a later timeout reaches an invalid lifecycle state.

**Change and preserved behavior:** Restore cancellation through the timer owner's existing API in `handleStopOperation()`. Retain the timer object for restart. Preserve the existing socket closure, delayed stop completion, configuration, and cumulative counters. The existing cancellation mechanism prevents the old timeout; no new lifecycle flag or generation counter is necessary. One commit contains the handler repair and its focused regression coverage.

**Verification:** Derive a proposed `tests/module/udpapp_lifecycle_timer_restart.test` from [udpapp_lifecycle_6.test](../../../tests/module/udpapp_lifecycle_6.test). Choose a send interval that places the pending timeout after shutdown completes. Stop before that timeout, away from its exact timestamp.

Advance beyond that timeout while the client remains down. Check that no send callback occurs during that interval. Restart the client. Check that it sends again and releases the timer at final teardown.

Use the [focused test procedure](run-the-gates.md#during-focused-development) for execution with a fresh debug library. From the checkout root, the proposed command selects the existing case and the new variant:

```sh
inet_run_module_tests -m debug -f 'udpapp_lifecycle_(6|timer_restart)\.test'
```

Confirm that the selector includes both cases after the new test exists. The existing case covers shutdown and teardown; it does not establish restart behavior. The hypothetical edit should fail when the old timeout reaches the stopped application. The repair should suppress that timeout and permit later sends after restart. The new variant's results remain unverified until implementation and execution.

### Expanded template

```markdown
# Implementation plan: <subject>

Checkout and revision: <repository, revision, relevant local changes>
Plan revision: <revision or date that identifies the reviewed text>
Approval: <pending, or actual approval source and scope>

## 1. Problem and intended behavior
<Current behavior, required behavior, and a concrete example.>

## 2. Current implementation and evidence
<Actual caller, responsible component, consumers, source links, and observed evidence.>

## 3. Scope and acceptance criteria
<Required work, optional follow-up work, observable criteria, and preserved conditions.>

## 4. Proposed design and reasons
<Responsibilities, interactions, reuse options, justified additions, and rule references.>

## 5. Affected components and compatibility
<Affected artifact types, existing configurations, external consumers, and migration needs.>

## 6. Implementation steps and dependencies
<Purpose, owner and files, proposed change, expected behavior, and verification for each step.>

## 7. Verification methods and expected results
<Existing or proposed tests, exact commands, build mode, selectors, expected observations, and gaps.>
<Keep planned checks separate from observed results and their evidence.>

## 8. Risks, unresolved decisions, and approval scope
<Design-critical assumptions, required decisions, and any separate permissions.>
<For a revision, identify the changed decision and whether renewed approval is necessary.>
```
