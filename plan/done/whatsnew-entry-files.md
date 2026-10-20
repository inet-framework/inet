# Release notes as files of their own

Status: **done**. Landed on `master` by a fast-forward and pushed on 2026-10-20, at the maintainer's request.

- Branch `topic/whatsnew-entry-files`, in the worktree `inet-whatsnew-entry-files`, from `origin/master` at `666f0f0bc2` (2026-10-16). No local edits.
- Approval: the maintainer approved the scope on 2026-10-20, in the conversation that found the problem. The four decisions of that approval are D-1 to D-4 below. No sealed path is in the scope ([seal-list.md](../../doc/project/audit/seal-list.md): `rule/release.md`, `rule/classification.md` and `rule/pull-request.md` have no closed unit). No baseline moves, because no source file changes.
- The decision numbers start at D-1.

## 1. The problem

Many open pull requests cannot be rebased onto `master`, because each one conflicts in `WHATSNEW`. The rule [RR-WHATSNEW](../../doc/project/rule/release.md#rr-whatsnew) says what an entry must say, but no rule, gate, template or merge driver says where the entry goes. Each pull request therefore adds its entry as the next number at the end of a list of the version in development. All parallel pull requests change the same lines and choose the same number. When one of them lands, all the others conflict.

The measurement of 2026-10-20, a test merge of each open pull request with `git merge-tree` against `origin/master` `666f0f0bc2`:

| | Pull requests |
| --- | --- |
| Open | 101 |
| Add to `WHATSNEW` | 64 |
| Conflict with `master` | 70 |
| Conflict in `WHATSNEW` | 29 |
| `WHATSNEW` is the only conflict | 14 (#1252, #1322, #1324, #1326, #1334, #1337, #1340, #1342, #1351, #1353, #1354, #1356, #1358, #1362) |

An example: #1354 adds item "50." at line 402, but `master` already has items up to 56.

The numbers on `master` are already wrong. Item "20. Half-duplex Ethernet drops a frame after 16 collisions, not 17" comes after item 56 (commit `f6f082f3ed`, made when the list had 19 items). The list of bug fixes goes from 6 to 9, because the Ethernet branch counted on items 7 and 8 that `master` never had. Two earlier commits repaired numbers by hand: `040c7a9e40` "restore consecutive WHATSNEW entry numbers" and `333ea6383f` "number the WHATSNEW entry of the QoS DATA lifetime 48".

Two more shared files cause the same type of conflict: `doc/src/migration-guide/index.rst` in 18 pull requests and `tests/fingerprint/*.csv` in 37. They are out of the scope of this plan (section 6).

## 2. The design

### Entry files

Each release note of the version in development is a file of its own:

```
doc/whatsnew/<section>/<area>-<topic>.txt
```

- `<section>` is `incompatible`, `compatible` or `fixes`. These are the three lists of the section of a version in `WHATSNEW`: "Notable backward incompatible changes", "Notable backward compatible changes" and "Notable bug fixes".
- `<area>` is the area of the commit subject, in lower case, for example `tcp`, `ieee80211`, `ipv6` or `queueing`. `<topic>` is a few words of the title. The name has only lower-case letters, digits and hyphens.
- Line 1 is the title, without a number. Line 2 is empty. The body follows: paragraphs with one empty line between them, each prose paragraph on one line ([documentation.md](../../doc/project/rule/documentation.md)). A table or a configuration line keeps the indent that it has relative to the body, as now.

Two pull requests that add two entries add two different files. Git then has nothing to merge, so they cannot conflict. Two pull requests can still conflict when both change the same entry, but that is a real conflict.

### WHATSNEW

`WHATSNEW` keeps the released versions. The heading of the version in development stays, with one sentence that points to `doc/whatsnew/` and to the command that shows the entries as one text. Nobody writes a number any more: the tool numbers the entries when it makes the section. The wrong numbers of section 1 therefore disappear with the move.

### The order

The tool puts the entries of one section in the order of their file names (D-2). Because a name starts with its area, the entries of one area stand together. The author sees the position from the name, and the order needs no git history.

### The tool

One Python script, `doc/project/enforcement/whatsnew.py`, beside the other gates and their tests. The enforcement test workflow already runs every `test_*.py` of that folder ([enforcement-tests.yml](../../.github/workflows/enforcement-tests.yml)). Commands:

- `preview`: prints the section of the version in development, numbered, as the release will show it.
- `check`: checks the entry files (folder, name, format) and that the section of the version in development in `WHATSNEW` has no entry. With `--range`, it checks each commit of the range, and it reads only git objects, because the pull-request workflow must treat the content of a pull request as data ([check-pull-request.yml](../../.github/workflows/check-pull-request.yml)).
- `release`: writes the numbered section into `WHATSNEW` under the new version heading, puts a new heading for the next version in development above it, and removes the entry files.
- `rebase`: rebases a branch, and moves the `WHATSNEW` change of each commit into entry files (next subsection).

### The rule and the gate

A new rule RR-WHATSNEW-FILE in [release.md](../../doc/project/rule/release.md) says where an entry goes and why. RR-WHATSNEW keeps what an entry says. The obligation `whatsnew` of [classification.md](../../doc/project/rule/classification.md) points to the entry files. The release checklist of [releng/README.md](../../releng/README.md) uses `whatsnew.py release`; the "update-whatsnew script" that it names does not exist in the tree.

A new series gate `check-whatsnew.sh` runs `whatsnew.py check --range` in [check_pull_request.py](../../doc/project/enforcement/check_pull_request.py), beside the other series gates. A pull request that adds an entry to `WHATSNEW` then fails with a message that names the entry file to write.

### The rebase of an open pull request

After the move, each of the 64 open pull requests that add to `WHATSNEW` conflicts in `WHATSNEW` one more time. `whatsnew.py rebase <upstream>` runs `git rebase <upstream>`. When a commit stops on a conflict in `WHATSNEW`, the tool compares the section of the version in development in that commit with the same section in its parent. It matches the entries by title:

- An entry that the commit adds becomes a new entry file. The area comes from the commit subject; for a `doc:` or `plan:` commit, it comes from the most frequent other area of the rebased commits.
- An entry that the commit changes or removes changes or removes the entry file with that title.

Then the tool takes `WHATSNEW` from the new base and continues the rebase. It stops when another file conflicts, when the commit changes `WHATSNEW` outside the section of the version in development, or when an entry file is not found. The author or the maintainer then resolves the conflict and runs `whatsnew.py rebase --continue`. At the end, the tool runs `check --range` over the rebased commits.

### Rejected options

- **`merge=union` in `.gitattributes`.** Git keeps the lines of both sides, so two entries with the same number stand one after the other. The numbers stay wrong, and a changed entry appears twice.
- **Entries without numbers in `WHATSNEW`.** Git still conflicts when two branches add lines at the same place.
- **The order of the git history.** It gives today's landing order, but the author cannot see or choose the position, and the order changes when a file moves (D-2).
- **Keep the 80 entries of 4.7.x in `WHATSNEW`, new ones as files.** The version then has two places, and a pull request that changes an old entry still conflicts (D-1).

## 3. The steps

| Step | Commit | Check |
| --- | --- | --- |
| 1 ✅ | `plan: add: release notes as files of their own` (`e78a4beaab`) | — |
| 2 ✅ | `doc: add: a tool that checks and shows the release note files` (`882053b0dd`) | `test_whatsnew.py` (new): parse, render, check, release |
| 3 ✅ | `doc: change: the release notes of 4.7.x are files of their own` (`9bc2f3d958`, D-5) | `whatsnew.py preview` gives the 80 entries of `origin/master`, the same text, numbered again |
| 4 ✅ | `doc: format: three release notes keep each paragraph on one line` (`f98587dd7e`) | the Ethernet entries of `f6f082f3ed`, `3c7861ddd7` and `136e6c2198`; `git diff --word-diff` shows no changed word |
| 5 ✅ | `doc: add: rule RR-WHATSNEW-FILE, a release note is a file of its own` (`96bdc7762a`) | `check-links.sh`, `check-seals.sh --write` (the index of `rule/release.md` has 10 rules) |
| 6 ✅ | `doc: add: the pull-request gates check the release notes` (`61a8c25ea0`) | `test_check_pull_request.py`, `test_whatsnew.py`; the audit of #1354 |
| 7 ✅ | `doc: add: a rebase that moves the release notes of a branch into files` (`7c43c6608b`) | `test_whatsnew.py`: three commits that add, change and add entries rebase without a stop; another conflict stops, and `--continue` finishes |
| 8 ✅ | `doc: location: the CSMA/CD statistics are backward compatible changes` (`e3cf85f04b`, D-13) | — |
| 9 ✅ | `plan: change: record the rebase of the open pull requests` | the trial of section 7 |
| 10 ✅ | `plan: location: release notes as files of their own, done` | — |

Each commit carries a `Change:` trailer with the group `whatsnew-entry-files`. The gates `check-links.sh`, `check-seals.sh`, `check-commits.sh` and `check-classification.sh` run over `origin/master..HEAD` before each commit, and `check-pull-request.sh` over the whole series at the end.

## 4. The checks

- **The move loses no text (step 3).** A script reads the section of 4.7.x from `origin/master:WHATSNEW` and from `whatsnew.py preview`, and compares the entries by section and title, with the numbers removed. The bodies must be equal. Only the order and the numbers change.
- **The gate fails on the old way (step 6).** `check-pull-request.sh` over #1354 must report the entry in `WHATSNEW`; over the series of this plan, it must pass.
- **The rebase works on real pull requests (steps 7 and 9).** For each open pull request that adds to `WHATSNEW`, `whatsnew.py rebase` onto the end of this branch, in a scratch worktree. The record gives, for each one, whether `WHATSNEW` still conflicts, which other files conflict, and whether the entry files pass `check`. Expected: the 14 pull requests whose only conflict is `WHATSNEW` rebase without a stop. Nothing is pushed to a pull request.
- **What the checks cannot show.** They do not show that an entry stands in the right section, or that its text is good for a user. RR-WHATSNEW stays a T4 review rule.

## 5. The decisions

- **D-1 — All entries of 4.7.x move.** The maintainer chose the move of all 80 entries over a mixed state (section 2, rejected options). The version in development then has one place.
- **D-2 — The order is the file name.** The maintainer chose the order of the file names, with the area first, over the landing order.
- **D-3 — `WHATSNEW` only.** The migration guide has the same problem, but it needs a Sphinx directive and its own decision about the order. It gets a plan of its own.
- **D-4 — A rebase tool, tested, nothing pushed.** The maintainer asked for a tool that converts the open pull requests, and for a test on them. Each landing stays the decision of the maintainer.
- **D-5 — The move is `behavior.change`, not `location`.** The depth `location` keeps the text ([classification.md](../../doc/project/rule/classification.md)). The move also removes the numbers, and it adds the sentence of the stub and `doc/whatsnew/README.md`.
- **D-6 — The names of the 80 entries are chosen by hand.** The area of the commit that added a title was often a component (`TokenBucketClassifier`, `IPathLoss`) or `doc`, and such names put an entry far from its area. The 80 names use 19 areas: `arp`, `common`, `dhcp`, `ethernet`, `icmp`, `icmpv6`, `ieee80211`, `ipsec`, `ipv4`, `ipv6`, `mpls`, `networklayer`, `packet`, `protocolelement`, `queueing`, `quic`, `radio`, `tcp` and `udp`.
- **D-7 — The tool lives beside the gates.** `doc/project/enforcement/` holds the gates, their tests and their helpers, and the enforcement test workflow already runs every `test_*.py` there. The release checklist and `doc/whatsnew/README.md` give the path.
- **D-8 — The gate flags only the entries that a commit adds or changes.** A commit of a branch from before the move has the old entries of `master` in its `WHATSNEW`. A check of the whole tree would flag a commit that only corrects a released section. The check of the working tree, without `--range`, flags every entry.
- **D-9 — The body of a rendered entry has an indent of three spaces.** 75 of the 80 entries of 4.7.x used three spaces, and 5 entries with a number of two digits used four.
- **D-10 — The rebase matches the entries by title, and an entry with a new title by its section and number.** This handles a commit that changes its own entry of an earlier commit, a commit that changes an entry of `master`, and a commit that gives an entry a new title. The tool stops when a commit changes a released section of `WHATSNEW`, because then `WHATSNEW` of the new base is not the full resolution.
- **D-11 — A name drops a leading article before it drops the area.** The trial showed `icmpv6-icmpv6-checksum-…` for "The ICMPv6 checksum …".
- **D-12 — A branch from before the move takes the script from `master`.** The branch has no `whatsnew.py`; `doc/whatsnew/README.md` gives the `git show origin/master:…` command.
- **D-13 — The CSMA/CD statistics are backward compatible changes.** The entries of `3c7861ddd7` and `136e6c2198` are additions (`ethernet: add:`), but they stood in the bug fixes on `master`. The maintainer agreed on 2026-10-20 to move them to `compatible/`; the text does not change.

## 6. Out of scope

- The migration guide, `doc/src/migration-guide/index.rst` (D-3).
- The fingerprint baselines, `tests/fingerprint/*.csv`. A commit carries its own baseline rows ([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)), so parallel pull requests change the same rows. [architecture.md](../../doc/project/rule/architecture.md) already asks for baseline stores that parallel branches do not conflict over. That is a different design.
- The citations by line number into `WHATSNEW` in `doc/project/evidence/model/*/conformance.md`. They were wrong before this change, and the move changes the line numbers again.

## 7. The record

**The move (step 3).** `whatsnew.py preview` gives the same 80 entries as the section of 4.7.x of `origin/master` `666f0f0bc2`, compared by section, title and body: 57 backward incompatible changes, 15 backward compatible changes and 8 bug fixes. The released sections of `WHATSNEW` are unchanged, byte for byte. `whatsnew.py check` passes.

**The gate (step 6).** `check-pull-request.sh` over the series passes the six gates. Over #1354 it reports one violation: `3d0ea6d40 adds the entry "The default Router Lifetime follows MaxRtrAdvInterval" to WHATSNEW; write it as doc/whatsnew/incompatible/ipv6-default-router-lifetime-follows-maxrtradvinterval.txt`. The 175 enforcement tests pass.

**The trial (steps 7 and 9).** On 2026-10-20, each of the 101 open pull requests was rebased with `whatsnew.py rebase` onto `7c43c6608b`, in a scratch worktree, and then discarded. Nothing was pushed.

| Pull requests | Rebase without a stop | Stop |
| --- | --- | --- |
| Add to `WHATSNEW`: 64 | 30 | 34, none of them in `WHATSNEW` |
| Do not add to `WHATSNEW`: 37 | 15 | 22 |

- The 30 include the 14 whose only conflict was `WHATSNEW`, and the 16 that merged without a conflict before the move: #1122, #1123, #1232, #1246, #1250, #1252, #1254, #1256, #1264, #1266, #1284, #1290, #1292, #1294, #1297, #1306, #1322, #1324, #1326, #1334, #1337, #1340, #1342, #1343, #1351, #1353, #1354, #1356, #1358, #1362.
- The first stop of the other 34 is in other files: 10 only in `tests/fingerprint/*.csv`, 4 only in the migration guide, 9 in the migration guide and in source, and 11 in source or other files. These conflicts are outside this plan (section 6).
- The rebased #1354 has one commit: its source change, its test, its baseline rows and the new entry file, and no change to `WHATSNEW`.
- The tool made 62 entry files. 12 notes said that no other release note has the area, for example `figures`, `recorder`, `mipv6`, `pmipv6` and `ipv6networkconfigurator`. A maintainer can rename such a file in the landing.

**After the landing.**

- After the landing, an author of an open pull request needs to know about `whatsnew.py rebase`. The gate message names the file to write, and `doc/whatsnew/README.md` gives the command.

