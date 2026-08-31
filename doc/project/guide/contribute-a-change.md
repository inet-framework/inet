# Contribute a change

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [architecture.md](../rule/architecture.md), [pull-request.md](../rule/pull-request.md), [sealing.md](../rule/sealing.md)

The nine steps from a task to a merged change. The rules work as a design map, not as a reading
assignment: the question is not *does this patch look reasonable?* but **which contracts does this
patch touch, what evidence establishes compliance, and what prevents a regression later?**

## 1. Scope

Name what the change touches: the domain, the contracts, the module composition, the packet content,
the tags, the configuration surface, the observability, the build feature, and the test coverage. A
change whose surface you cannot name is a change you cannot review.

## 2. Seals

If the change is under `src/inet/`, resolve the seals first. A path is sealed if it is listed in
[audit/seal-list.md](../audit/seal-list.md) **or** lives under a listed folder. A sealed path needs
explicit permission before anything else happens. See [sealing.md](../rule/sealing.md).

## 3. The rules that apply

Read only the sections that apply. [architecture.md](../rule/architecture.md) for the structure,
[naming.md](../rule/naming.md) for every new or renamed artifact, [testing.md](../rule/testing.md)
for what the change must ship with, and the domain document when the change is inside its subtrees —
today [domain/ieee80211.md](../domain/ieee80211.md). Read both exception ledgers, so that a known
deviation does not look like a finding.

## 4. The smallest surface

Establish who owns which state before you edit. Then make the smallest change that satisfies the
contracts, the ownership, the observability, the configuration, the determinism and the test rules.

## 5. The mechanisms that already exist

Build through the existing contracts, registries, signals, serializers, lifecycle APIs and feature
descriptors before you invent a new mechanism. A new mechanism is a cost that every later model pays.

## 6. Validate in proportion to risk

Run [enforcement/check-architecture.sh](../enforcement/check-architecture.sh), scoped to the touched
subtree for focused work, and the test categories that match the claim. Keep the exact commands, the
configurations and the resulting statuses; the pull request description needs them.
[guide/run-the-gates.md](run-the-gates.md) lists the gates in order.

## 7. Reconcile, do not re-litigate

Record only a genuinely new deviation, as an `AV-*` row in
[audit/architecture-exceptions.md](../audit/architecture-exceptions.md) or an `NV-*` row in
[audit/naming-exceptions.md](../audit/naming-exceptions.md). A deviation that a ledger already holds
is known, not a finding. A fingerprint baseline changes only with explicit approval and a reviewable
explanation — see [change-a-baseline.md](change-a-baseline.md).

## 8. Submit a reviewable change

Divide the work into commits by concern: the whitespace and mechanical sweeps apart from the logic,
the shared-component change before the model that needs it, the baseline update in its own patch.
Write messages that state the reason. Every rule for this step is in
[pull-request.md](../rule/pull-request.md).

## 9. Sealing last

Sealing is the terminal state of this pipeline, not a shortcut around it. A complete audit, with
every deviation repaired or ledgered, comes before a seal is recorded. See
[audit-a-subsystem.md](audit-a-subsystem.md).
