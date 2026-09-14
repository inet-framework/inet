# Keep the audit reports out of git

Status: **done** — worktree `/home/levy/workspace/inet-audit-reports-out-of-git`,
branch `topic/audit-reports-out-of-git`, off `master` at `3c215c0754`, commit `6dd84c6fe7`.

## 1. What this changes

An audit report judges one tree at one commit. A re-audit replaces it, and the part that must
outlive it is already somewhere else: a finding becomes a ledger row, a seal becomes a
`seal-list.md` row. The 25 report files under `doc/project/audit/report/` therefore grew the
history without adding a fact that a later reader needs, and they sat in the diff of every change
that touched the audit process.

The reports move to `audit/` at the repository root, `.gitignore` hides the folder, and the
documents that describe the process say so.

## 2. The decisions

| Decision | Why |
| --- | --- |
| the kind folders stay: `audit/subsystem/`, `audit/pull-request/`, `audit/sweep/`, `audit/document/` | the naming rules in `audit/README.md` then need only a prefix change |
| the ledgers and `seal-list.md` stay in git, under `doc/project/audit/` | they are not reports, they are the durable record, and `check-source-seals.sh` reads `seal-list.md` |
| every link to a report becomes an inline code path | a link would resolve on the author's machine and break in every clone, and [check-links.sh](../../doc/project/enforcement/check-links.sh) walks only `doc/project`, so it would not catch it |
| `/audit/` with a leading slash in `.gitignore` | it must not match the tracked `doc/project/audit/` |

Rejected: keep a link and let the gate pass locally. The gate resolves a link against the file
system, so a green run on the author's machine would say nothing about a clone.

## 3. Steps

- [x] 1. Copy the 25 reports to `audit/` and `git rm` the old tree.
- [x] 2. Add `/audit/` to `.gitignore`.
- [x] 3. Turn the 26 report links in `doc/project/` and in the live plan into code paths.
- [x] 4. Rewrite the process text in `audit/README.md`, the two guides, `doc/project/README.md`
      and `rule/sealing.md`.
- [x] 5. Correct the three statements that said git holds the older text of a report.
- [x] 6. Run the gates.

## 4. The seal

`doc/project/rule/sealing.md` carries `Seal: whole`. Its rule `SR-CITE-THE-AUDIT` named the old
folder and called the citation a link. The user gave permission for that document in the
conversation of 2026-09-14. The other ten documents are `Seal: none`, `by rule` or `by row`, and
none of them holds a `🔒` unit, so none needed permission.

## 5. What is left untouched

`plan/done/project-documentation-structure.md` still names `audit/report/`. It is the record of a
finished plan, and rewriting it would falsify what that plan did. `check-links.sh` walks only
`doc/project`, so it does not read the file.
