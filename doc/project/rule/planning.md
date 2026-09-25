# Implementation plan rules

> **Kind:** rule · **Status:** draft · **Seal:** by rule · **Owns:** `PLR-*` · **Stands on:** [architecture.md](architecture.md), [quality.md](quality.md), [testing.md](testing.md), [pull-request.md](pull-request.md), [release.md](release.md), [sealing.md](sealing.md), [documentation.md](documentation.md)

This document proposes requirements for tasks that call for an implementation plan. The proposed
approval process takes effect when a maintainer accepts this policy. An explicit task instruction
can also select this draft procedure.

These rules govern the plan. The linked technical rules govern the implementation. The procedure
and template are in [write-an-implementation-plan.md](../guide/write-an-implementation-plan.md).

## Index

| Rule | Statement |
| --- | --- |
| [PLR-READABLE](#plr-readable) | A maintainer can assess the plan without the conversation history. |
| [PLR-GROUNDED](#plr-grounded) | The plan separates verified facts, proposed decisions, and unresolved assumptions. |
| [PLR-SCOPE](#plr-scope) | The plan defines the intended behavior, acceptance criteria, and scope boundaries. |
| [PLR-DESIGN](#plr-design) | The plan identifies responsible components and explains significant design choices. |
| [PLR-VERIFICATION](#plr-verification) | Each behavior claim has a proposed verification method and explicit coverage limits. |
| [PLR-APPROVAL](#plr-approval) | A human maintainer approves the concrete plan before implementation starts. |
| [PLR-REVISION](#plr-revision) | A material change to the approved scope or design requires renewed approval. |
| [PLR-PROPORTION](#plr-proportion) | Plan detail is proportionate to the change and its risks. |

## The rules

### PLR-READABLE

**A maintainer can assess the plan without the conversation history.**

Write for the INET maintainer who must approve the work. Explain the problem before the proposed edits.
Define unfamiliar terms at first use. State the reason for each significant choice. Link to evidence
that the maintainer needs to check the proposal.

A file list alone does not explain the design. A useful step connects its purpose to the affected
component, the proposed change, and the expected result.

*Enforced at T4 — agent review through the [plan review checklist](../guide/write-an-implementation-plan.md#review-checklist).*

### PLR-GROUNDED

**The plan separates verified facts, proposed decisions, and unresolved assumptions.**

Identify the checkout and revision that the plan describes. Note relevant local changes. Inspect
the current code and effective configuration before you name the affected components.

Link each significant claim about current behavior to its source or observed evidence. Trace the
production caller to the component that owns the behavior. Identify affected consumers. State how
to check each unresolved assumption.

An unresolved fact that can change the design or acceptance criteria blocks implementation of the
dependent step. An independent approved step may proceed.

*Enforced at T4 — agent review through the [plan review checklist](../guide/write-an-implementation-plan.md#review-checklist).*

### PLR-SCOPE

**The plan defines the intended behavior, acceptance criteria, and scope boundaries.**

Describe the current behavior and the required behavior. Give a concrete example when useful.
State observable acceptance criteria. Identify the conditions that must remain true.

Separate work necessary for those criteria from optional follow-up work. Explain why each
implementation step belongs in the requested change. Apply
[PR-SPLIT-DRIVEBY](pull-request.md#pr-split-driveby) to unrelated fixes.

*Enforced at T4 — agent review through the [plan review checklist](../guide/write-an-implementation-plan.md#review-checklist).*

### PLR-DESIGN

**The plan identifies responsible components and explains significant design choices.**

Describe where the behavior belongs and how the components interact. Identify each affected
artifact type, with declarations and consumers outside the main source file. Explain the
effect on compatibility under [release.md](release.md).

Record the existing mechanisms considered for reuse under
[AR-EXT-REUSE](architecture.md#ar-ext-reuse). Justify each proposed addition against its applicable
architecture rule. The justification names its responsibility and required consumers. It explains
why reuse or a smaller change is insufficient.

Cite the applicable rules. Keep their text in the document that owns them. The
[design review map](../guide/write-an-implementation-plan.md#design-review-map) routes each concern
to its owner.

*Enforced at T4 — agent review through the [plan review checklist](../guide/write-an-implementation-plan.md#review-checklist).*

### PLR-VERIFICATION

**Each behavior claim has a proposed verification method and explicit coverage limits.**

Map each acceptance criterion to evidence under [TR-CAT-MATCH](testing.md#tr-cat-match) and
[TR-FOCUSED-EVIDENCE](testing.md#tr-focused-evidence). Describe the trigger, the relevant production
path, and the expected observation.

Distinguish existing checks from checks that the implementation must add. Verify existing commands
and selectors in the checkout. Label proposed commands whose test targets do not yet exist. State
what each check cannot establish.

Separate planned checks from observed results. A plan does not need successful results from code
that does not yet exist. An observed result needs the reproducible evidence required by the test
rules. Identify expected changes to recorded expectations under
[TR-BASELINE-DELIBERATE](testing.md#tr-baseline-deliberate).

*Enforced at T4 — agent review through the [plan review checklist](../guide/write-an-implementation-plan.md#review-checklist).*

### PLR-APPROVAL

**A human maintainer approves the concrete plan before implementation starts.**

Read-only investigation and plan preparation precede approval. The maintainer approves a specific
plan revision and its implementation scope. Record the approval source and scope in the plan.
An agent review cannot grant human approval.

Existing approval remains valid within its stated scope. Approval of a request to investigate or
prepare a plan does not authorize implementation. Approval of this policy does not approve an
individual implementation plan.

Identify separate permissions required by [sealing.md](sealing.md) and
[TR-BASELINE-DELIBERATE](testing.md#tr-baseline-deliberate). Record whether the maintainer's approval
explicitly covers those permissions. Apply each owner's approval requirements.

*Enforced at T5 — the human maintainer approves the plan; T4 — agent review checks the recorded scope.*

### PLR-REVISION

**A material change to the approved scope or design requires renewed approval.**

Update the plan when new evidence changes an approved decision. Explain the change and its reason.
A revision is material if it changes any of these approved elements:

- The intended behavior or acceptance criteria.
- The component that owns the behavior or a public contract.
- The implementation scope or the basis for verification.

Obtain approval before you implement the affected revision. Independent work within the existing
approval may continue. Editorial corrections and routine implementation details within the approved
design do not require renewed approval.

*Enforced at T5 — the human maintainer approves material revisions; T4 — agent review compares the work with the approved plan.*

### PLR-PROPORTION

**Plan detail is proportionate to the change and its risks.**

Explain difficult decisions and material risks in detail. Keep routine edits brief. A small plan
may combine sections when the required information remains clear. Omit inapplicable detail with
a short reason where its absence could confuse a reviewer.

Order implementation steps by their dependencies under
[PR-SERIES-ORDER](pull-request.md#pr-series-order). Give each step a coherent purpose and a
verification method. Use the
[plan template](../guide/write-an-implementation-plan.md#plan-template) as an initial structure.
Avoid empty sections and repeated policy text.

*Enforced at T4 — agent review through the [plan review checklist](../guide/write-an-implementation-plan.md#review-checklist).*
