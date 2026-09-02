# Review a code change

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [architecture.md](../rule/architecture.md), [quality.md](../rule/quality.md), [testing.md](../rule/testing.md)

How to review a working-tree diff, staged diff, individual commit, commit range, integrated branch,
or pull-request code change for correctness and rule compliance. For a branch or pull request, also
apply [review-a-pull-request.md](review-a-pull-request.md) to its commit series and `PR-*` rules.

Correctness review and rule compliance are two passes over the same scope. Inspect the changed
contracts independently first; then run the canonical tier-4 checklists. This keeps checklist wording
from becoming the only search strategy without weakening the rule-backed gate.

## 1. Establish the target

Resolve an exact base and head, or state an explicit working-tree or staged scope. Do not review an
assumed pull-request description or a stale line range. Record:

- the reviewed commit, range, or working-tree state;
- every changed file and any generated input it changes;
- build and runtime feature gates;
- the effective configurations that reach the changed behavior; and
- the relevant standard and revision when the change implements normative behavior.

For a working tree, distinguish unstaged changes, staged changes, and untracked files. For a branch,
use its merge base rather than an arbitrary ancestor. Keep the exact identifiers in the report so
another reviewer can reproduce the scope.

## 2. Inventory changed contracts

Classify each change by the contracts it can affect:

- API;
- ownership and lifetime;
- lifecycle;
- protocol state;
- packet, chunk, and tag representation;
- serialization;
- configuration;
- timing;
- observability;
- build integration; and
- test behavior.

Use the inventory to select callers, configurations, generated consumers, standards evidence, and
tests. A changed file can participate in several contracts, and a contract can span several artifact
kinds.

## 3. Perform an independent correctness pass

Review the changed contract before running the rule checklist. Trace, as applicable:

- effective callers;
- implementations and overrides;
- adapters and replaceable types;
- owners and consumers;
- semantic siblings;
- success, refusal, timeout, cancellation, teardown, restart, and stale-completion paths;
- configuration and generated consumers; and
- direct test coverage.

Follow pre-existing code only where the change calls it, depends on it, changes its contract, or
makes its behavior newly reachable. Do not turn a diff review into an unrelated subsystem audit.

## 4. Prove each correctness finding

A correctness finding requires all six elements:

| Element | Required evidence |
| --- | --- |
| Invariant | What must remain true, and which component owns it? |
| Reachable trigger | Which supported input, configuration, state, or event sequence reaches the path? |
| Failure mechanism | Which exact branch, lookup, transition, ownership transfer, or serialization step breaks the invariant? |
| Observable consequence | What becomes wrong in packet behavior, state, timing, memory, output, or execution? |
| Connection to the change | How did the reviewed change introduce, expose, or make the defect relevant? |
| Focused verification | What is the smallest check that fails before and passes after the correction? |

Do not report pattern-matched suspicion as a defect. If the evidence proves the defect but not the
precise repair, state the correction direction at the owning contract or boundary instead of
inventing a one-line fix.

## 5. Derive severity from impact

Derive severity from the proved defect's:

- reachability;
- consequence;
- presence in a supported configuration;
- blast radius; and
- recoverability.

Use the repository's applicable severity labels, if any, only after evaluating those facts. A label
from a historical checklist or the apparent importance of a prompt does not determine severity.
Questions, missing evidence, and optional hardening do not carry defect severity.

## 6. Run the compliance pass

After the independent correctness pass, run the
[general tier-4 checklist](../enforcement/checklist/general.md). Add the
[IEEE 802.11 checklist](../enforcement/checklist/ieee80211.md) when its documented scope applies.
Respect the canonical exception ledgers, scope each verdict to the reviewed diff, and do not report a
pre-existing deviation as a new violation.

Every tier-4 checklist item is backed by its cited canonical rule. The checklists do not replace the
correctness pass, and a correctness concern without a rule violation does not become a checklist
`FLAG`.

## 7. Use one reporting taxonomy

| Term | Meaning |
| --- | --- |
| **Correctness finding** | A proved defect or regression. It carries severity derived from reachability and consequence. |
| **Checklist `FLAG`** | A clear violation of the canonical rule cited by that checklist item. |
| **Checklist `QUESTION`** | A plausible conflict with a cited rule that requires genuine human/T5 judgment. It carries no defect severity. |
| **Not verified** | A required check was not run or the necessary evidence is unavailable. |
| **Residual risk / untested path** | A bounded review gap that is not a proved defect. |
| **PR `PARTIAL`** | A verdict used only in a composite pull-request `PR-*` rule report when part of a rule passes and part does not; it is never a tier-4 checklist verdict. |

When one mechanism is both a runtime defect and a canonical rule violation, report the correctness
finding once. The corresponding checklist row briefly references that finding instead of duplicating
its proof and correction direction.

## 8. Write the report

Present the result in this order:

1. actionable correctness findings, ordered by severity;
2. consequential questions requiring a decision;
3. reviewed scope;
4. validation performed;
5. not-verified items and residual risks; and
6. canonical checklist output.

For each finding, identify the exact file and line, give the six-part proof compactly, state the
smallest correction direction, and name the focused verification. Record exact commands, working
directory, configuration, run and seed where applicable, filters, exit statuses, and relevant
artifacts for validation that was performed. Say directly when no actionable correctness finding
remains; do not invent a low-value comment to avoid an empty review.
