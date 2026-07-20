# Sealing — rules

The rules for *sealing* INET source files: freezing a file against further AI modification once it
has been reviewed and brought into compliance with the architecture and naming requirements. This
document is the **policy**; the authoritative list of what is currently sealed lives in
[sealing-status.md](sealing-status.md).

**Files are unsealed by default.** Nothing is sealed unless it matches an entry in the sealed list;
that list is an allowlist of the small, settled part of the tree that has been driven all the way to
a reviewed, compliant, frozen state, so that scarce review attention moves forward instead of
re-touching settled ground.

Sealing is the terminal state of the enforcement pipeline. A change is checked mechanically by
[`.clang-tidy`](enforcement/.clang-tidy) and [`check-architecture.sh`](enforcement/check-architecture.sh),
semantically by the [agent-review checklist](enforcement/agent-review-checklist.md), and its
deviations are tracked in [naming-exceptions.md](naming-exceptions.md) and
[architecture-exceptions.md](enforcement/architecture-exceptions.md). Sealing records the result:
*this file has passed the gate and is now frozen, so later broad tasks cannot silently churn it.*

## 🔒 Sealed files — DO NOT MODIFY

**Some files in this repository are *sealed*. A sealed file MUST NOT be modified by an AI in any way —
no edits, no reformatting, no "while I'm here" cleanups, no incidental changes as part of a larger
task — unless the user gives explicit permission for that specific file in the current
conversation.** This overrides every other instruction, including a broad task that would otherwise
touch a sealed file. If a change you are asked to make would require editing a sealed file, STOP and
tell the user the file is sealed and ask for explicit permission before proceeding.

## What is sealed: files and recursive folders

The sealed list holds two kinds of entry. Paths are relative to `src/inet/`.

- **A file** — e.g. `common/packet/Packet.h`. Exactly that file is sealed.
- **A folder**, written with a trailing `/` — e.g. `common/packet/`. This is **recursive**: it seals
  every file under that directory at any depth, **including files added later**. Sealing a subsystem
  as a folder is the way to freeze it as a unit and keep new files in it frozen by default.

**Resolution rule.** A given file is sealed if, and only if, the sealed list contains that exact
file *or* contains any ancestor directory of it as a folder entry. Otherwise it is unsealed (the
default). Consequences worth spelling out:

- Adding a new file under a sealed folder is itself a modification of the sealed subtree — it needs
  explicit permission, just like editing an existing sealed file.
- Generated siblings (`*_m.h` / `*_m.cc` produced from a `.msg`) sit next to their source, so a
  folder seal covers them; a lone `.msg` file seal covers its generated pair by intent — never
  hand-edit the generated pair regardless.

## The audit-before-seal workflow

Something is sealed only after it has been audited and complies. Do not offer to seal, and never say
"seal as-is," until the audit has been reported.

**Audit the moment you introduce a file (or folder) as the next thing to seal — before inviting
review and before offering to seal.** For a folder, the audit covers the whole subtree: every file
under it must comply (or carry a recorded exception) before the folder can be sealed as a unit. The
audit checks against:

1. [architectural-requirements.md](architectural-requirements.md) — the `AR-*` design rules
   (dependency direction, contracts, composition, observation neutrality, …).
2. [naming-conventions.md](naming-conventions.md) — the `AR-QUAL-NAMING` surface the linter cannot
   see (NED/`.msg`/semantic names, role suffixes).
3. The enforcement gates that apply: it must be clean under
   [`check-architecture.sh`](enforcement/check-architecture.sh) and the relevant
   [agent-review checklist](enforcement/agent-review-checklist.md) items.

Present the audit result first. A file seals only once it complies, **or** once a specific
non-compliance is explicitly accepted by the user in the conversation. An accepted non-compliance is
recorded in the appropriate ledger — a sanctioned coupling as an `AS-*` row in
[architecture-exceptions.md](enforcement/architecture-exceptions.md), a sanctioned name as an `NS-*`
row in [naming-exceptions.md](naming-exceptions.md) — and the file may then seal with that exception
on record. Do not seal over an *un*sanctioned violation (`AV-*` / `NV-*`); fix it or get it
sanctioned first.

## Recording a seal

To seal something, add its path to the sealed list in [sealing-status.md](sealing-status.md), in the
**same commit** that records the compliant state. Use a trailing `/` for a recursive folder seal, no
trailing slash for a single file. Keep the list grouped and ordered as that document describes; do
not silently drop entries.

## Violations found in an already-sealed file

The seal is a claim that the file complied *at the time it was sealed*; it is not a guarantee the
claim is still perfect. If you find a violation in a sealed file:

1. **Report it** — state the file, the location, and which requirement it breaks.
2. **Ask permission before fixing.** The seal still holds until the user grants explicit permission
   for that file; do not touch it in the meantime, and do not fold the fix into an unrelated task.
3. Once permission is given and the fix lands, the file stays sealed (it was re-audited and
   re-complied in that same commit). If the user instead chooses to accept the deviation, record it
   as an `AS-*` / `NS-*` exception and leave the seal.

## Unsealing

Removing a path from the sealed list unseals it. Because sealing is the reviewed-and-frozen state,
unsealing is a deliberate act, not a shortcut around the DO-NOT-MODIFY rule — ask for it explicitly
and record why in the commit. To edit one file inside a recursively sealed folder without unsealing
the whole subtree, get permission for that specific file (per the rule above) rather than deleting
the folder entry.

## Relationship to the rest of the effort

Sealing does not replace the exception ledgers or the CI gates — it composes with them. The gates run
on every change and catch regressions; the ledgers track known deviations; sealing marks the parts of
the tree that have been driven all the way to a reviewed, compliant, frozen state.
