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

### Design review map

Use only the rows that apply. Each linked document owns its technical requirements.

| Concern | Authoritative rules |
| --- | --- |
| Reuse and justified additions | [AR-EXT-REUSE](../rule/architecture.md#ar-ext-reuse) |
| Replaceable roles and interface contracts | [AR-ORG-CONTRACTS](../rule/architecture.md#ar-org-contracts), [AR-ORG-CONTRACT-PURITY](../rule/architecture.md#ar-org-contract-purity) |
| Class or module responsibility | [AR-MOD-COMPOSITION](../rule/architecture.md#ar-mod-composition) |
| Member visibility and virtual methods | [AR-EXT-MINIMAL-SURFACE](../rule/architecture.md#ar-ext-minimal-surface), [AR-EXT-VIRTUAL-IS-A-PROMISE](../rule/architecture.md#ar-ext-virtual-is-a-promise) |
| State and object lifetime | [QR-STATE-OWNER](../rule/quality.md#qr-state-owner), [QR-OBJECT-OWNERSHIP](../rule/quality.md#qr-object-ownership) |
| Calls, notifications, and observations | [AR-COM-DIRECT](../rule/architecture.md#ar-com-direct), [AR-COM-NOTIFY](../rule/architecture.md#ar-com-notify), [AR-OBS-SIGNALS](../rule/architecture.md#ar-obs-signals) |
| Units and configuration | [QR-CMT-UNIT](../rule/quality.md#qr-cmt-unit), [AR-CFG-INFER](../rule/architecture.md#ar-cfg-infer), [AR-CFG-PARAMS](../rule/architecture.md#ar-cfg-params) |
| Protocol requirements and model simplifications | [QR-CMT-STANDARD](../rule/quality.md#qr-cmt-standard), applicable [domain rules](../domain/README.md) |
| Compatibility and migration | [release.md](../rule/release.md) |
| Tests and recorded expectations | [testing.md](../rule/testing.md) |
| Commit boundaries and order | [pull-request.md](../rule/pull-request.md) |
| Protected paths and document units | [sealing.md](../rule/sealing.md) |

## 4. Define the steps and verification

Order the steps by their dependencies. Use this compact table for each coherent change:

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

### Review checklist

Record a short answer with evidence or a specific gap for each applicable row. This review evaluates
a proposed plan; it does not claim that implementation tests pass.

| Review question | Rule |
| --- | --- |
| Can a maintainer assess the proposal without the conversation? | [PLR-READABLE](../rule/planning.md#plr-readable) |
| Which facts support the chosen owner and production path? | [PLR-GROUNDED](../rule/planning.md#plr-grounded) |
| What observable result establishes completion? | [PLR-SCOPE](../rule/planning.md#plr-scope) |
| Why does each significant addition need to exist? | [PLR-DESIGN](../rule/planning.md#plr-design) |
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

Adapt this structure to the change. The placeholders describe content; they are not evidence or
approval.

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
