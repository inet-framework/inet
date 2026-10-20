# Migration guide sections as files of their own

Status: **done**. Landed on `master` by a fast-forward and pushed on 2026-10-20, at the maintainer's request.

- Branch `topic/migration-guide-entry-files`, in the worktree `inet-migration-guide-entry-files`, from `origin/master` at `a65e4d59f2` (2026-10-20). No local edits.
- Approval: on 2026-10-20 the maintainer asked for this plan, as the follow-up that [whatsnew-entry-files.md](../done/whatsnew-entry-files.md) left out of its scope (its D-3), and chose the version groups (D-1). No sealed path is in the scope. No baseline moves, because no model source changes.
- The decision numbers start at D-1.

## 1. The problem

Each pull request that breaks a model adds its section at the top of [doc/src/migration-guide/index.rst](../../doc/src/migration-guide/index.rst), as [RR-BREAK-MIGRATE](../../doc/project/rule/release.md#rr-break-migrate) asks. All parallel pull requests therefore change the same lines, and each landing makes the others conflict. This is the same problem that `WHATSNEW` had.

The measurements of 2026-10-20:

- A test merge against `origin/master` `666f0f0bc2`: 18 of the 70 open pull requests that conflict with `master` conflict in the migration guide.
- The trial of `whatsnew.py rebase` onto `7c43c6608b`: of the 34 pull requests that still stop, 13 stop in the migration guide, and for 4 of them it is the only file.

The guide has no version groups. Since `v4.7.0` (2026-07-07), pull requests added 20 sections above the guide of INET 3.x from 2019, and nothing tells a reader which version a section belongs to.

## 2. The design

### Section files

Each section of the migration guide for the version in development is a file of its own:

```
doc/src/migration-guide/development/<area>-<topic>.rst
```

- The name follows the release notes ([doc/whatsnew/README.md](../../doc/whatsnew/README.md)): the area of the commit subject in lower case, then a few words of the title.
- Line 1 is the title, line 2 underlines it with `~`, line 3 is empty, and the reStructuredText body follows. A subsection uses `^`. The body has no heading of the levels `=`, `-` or `~`, because they belong to the guide, to the version and to the section.

### Version groups

[index.rst](../../doc/src/migration-guide/index.rst) gets one heading for each version, with the `-` underline, and with the same text as the version heading in `WHATSNEW`. Below each heading, a directive includes the files of the folder of that version, in the order of their file names:

```
INET-4.7.x (in development)
---------------------------

.. include-files:: development/*.rst
```

The sections of the guide of INET 3.x stay in `index.rst` below the version groups. They also use `-` and `~`, so the levels agree.

At the release, `whatsnew.py release` moves the files of `development/` into the folder of the version (`4.8/` for "INET-4.8 …"), gives that folder the heading of the release, and puts the heading of the next version in development above it, with `development/*.rst` again.

### The directive

`include-files` is a Sphinx extension of about 60 lines in [doc/src/tools/](../../doc/src/tools/), where `tools.doxylink` already lives. It inserts each file that the pattern matches, in the order of the names, and records it as a dependency of the page. An incremental build also reads the page again when a file is added to or removed from the folder. When no file matches, it shows one sentence. `conf.py` excludes the section files from the source files, because each one is a part of a page and not a page.

### The tool and the gate

The existing tool [whatsnew.py](../../doc/project/enforcement/whatsnew.py) gets the migration guide too, because the rebase and the release must handle both files in one run:

- `check` also checks the section files (folder, name, format), and with `--range` it flags a commit that adds a section heading to `index.rst`. A version heading that only includes a folder is allowed, because the release writes one.
- `rebase` also resolves a conflict in `index.rst`: it matches the sections of the stopped commit and its parent by title, writes the added and changed sections as files of `development/`, and takes `index.rst` from the new base. It stops when the commit changes a part that stays in `index.rst`: the title, or a section of INET 3.x.
- `release` also moves the folder and writes the headings.

The gate `check-whatsnew.sh` runs `whatsnew.py check --range`, so it checks both rules. No new gate is necessary.

### The rule

A new rule RR-MIGRATE-FILE in [release.md](../../doc/project/rule/release.md) says where a migration section goes and why. RR-BREAK-MIGRATE keeps what a section owes. The obligation `migration` of [classification.md](../../doc/project/rule/classification.md) names the section files.

### Rejected options

- **One flat folder for all versions.** Simpler, but after the next release the sections of two versions stand mixed (D-1).
- **One `.. include::` line for each file in `index.rst`.** Two pull requests that add two lines at the same place still conflict.
- **A `toctree` with `:glob:`.** Each section becomes a page of its own, and the guide is no longer one page.
- **Fold the files into `index.rst` at the release, as `WHATSNEW` does.** The documentation of `master` must show the sections before the release, so the build must read the files in any case.

## 3. The steps

| Step | Commit | Check |
| --- | --- | --- |
| 1 ✅ | `plan: add: migration guide sections as files of their own` (`19eb39c65b`) | — |
| 2 ✅ | `doc: add: a Sphinx directive that includes the files of a folder` (`3d3677df50`) | a Sphinx build of a small test project: order, an empty folder, an incremental build after an added, a changed and a removed file, `-j 2` |
| 3 ✅ | `doc: change: the migration sections of 4.7.x are files of their own` (`1c1ceab2f6`) | the text build of the guide gives the same 20 sections and the same text, before and after (D-5) |
| 4 ✅ | `doc: add: rule RR-MIGRATE-FILE, a migration section is a file of its own` (`515578736f`) | `check-links.sh`, `check-seals.sh --write` (the index of `rule/release.md` has 11 rules) |
| 5 ✅ | `doc: add: the release note checks cover the migration guide` (`dff3e2efd0`) | `test_whatsnew.py`; the audit of #1225 |
| 6 ✅ | `doc: add: the rebase and the release handle the migration guide` (`c89209a2af`) | `test_whatsnew.py`: a branch that adds a section with a subsection and changes a moved one rebases without a stop; a change of a section of INET 3.x stops; a release moves the folder |
| 7 ✅ | `plan: change: record the rebase of the open pull requests` | the trial of section 7 |
| 8 ✅ | `plan: location: migration guide sections as files of their own, done` | — |

Each commit carries a `Change:` trailer with the group `migration-guide-entry-files`. The gates `check-links.sh`, `check-seals.sh`, `check-commits.sh` and `check-classification.sh` run over `origin/master..HEAD` before each commit, and `check-pull-request.sh` over the whole series at the end.

## 4. The checks

- **The move loses no text (step 3).** `sphinx-build -b text` of the guide, before and after. The two texts must have the same sections, compared by title and body. Only the order, the underline of the titles and the new version heading change.
- **The documentation builds without a new warning (steps 2 and 3).** A `dummy` build with `DOC_BUILD_TARGET=users-guide,developers-guide`, before and after.
- **The rebase works on real pull requests (steps 6 and 7).** The trial of `whatsnew-entry-files.md` again, onto the end of this branch: for each open pull request, whether it rebases without a stop, and which files stop it. Expected: the 4 pull requests whose only remaining stop was the migration guide rebase without a stop. Nothing is pushed.
- **What the checks cannot show.** They do not show that a section stands in the right version, or that it tells a user enough. RR-BREAK-MIGRATE stays a T4 review rule.

## 5. The decisions

- **D-1 — Version groups.** The maintainer chose the version groups over one flat folder: a reader who upgrades reads only the sections below the newer headings.
- **D-2 — The order is the file name, inside each version.** The same as the release notes (D-2 of `whatsnew-entry-files.md`).
- **D-3 — One tool and one gate for both files.** The rebase and the release must handle `WHATSNEW` and the guide in one run, and a commit that breaks a model usually owes both.
- **D-4 — The rebase tool is tested on the open pull requests, and nothing is pushed.** As D-4 of `whatsnew-entry-files.md`.
- **D-5 — The move also removes six warnings.** Six of the 20 sections underlined their titles with fewer `-` than the titles had characters, and Sphinx warned "Title underline too short" for each. The files underline each title to its full length. The text of the guide does not change.
- **D-6 — The names of the 20 sections are chosen by hand**, with the areas of the release notes: `arp`, `ieee80211`, `ipsec`, `packet`, `queueing` and `tcp`.
- **D-7 — The gate flags only a section that a commit adds.** A branch from before the move has the sections of 4.7.x in its own `index.rst`, and a commit of it can change one of them; the rebase moves that change into the file. A heading whose text only includes a folder is a version heading and passes, because the release writes one. The check of a tree also flags an `index.rst` without `.. include-files:: development/*.rst`.
- **D-8 — The rebase matches the sections by title only.** A section has no number, so a section with a new title is a removal and an addition. The rebase stops when the commit changes the lines above the sections, or a section that stays in `index.rst`, because the base has its own version of those lines.
- **D-9 — The release takes the folder from the heading.** "INET-4.8 …" gives `4.8/`. A heading without a version stops the release before it changes a file. Without a section file in `development/`, the release writes no heading of the release into the guide. After a release, the directive shows "No change of this version needs a migration step." below the new heading of the version in development.

## 6. Out of scope

- The fingerprint baselines, `tests/fingerprint/*.csv` (section 6 of `whatsnew-entry-files.md`).
- The title of the guide, "Migrating Code from INET 3.x". It no longer describes the guide, but a new title is a separate decision.

## 7. The record

**The directive (step 2).** A test project built with Sphinx 9.1.0 and docutils 0.21.2 gave the files in the order of their names, the sentence for an empty folder, a second read of the page after an added, a changed and a removed file, no read without a change, and no warning, also with `-j 2`.

**The move (step 3).** `sphinx-build -b text` with `DOC_BUILD_TARGET=users-guide,developers-guide`, before and after: the migration guide has the same 38 blocks, compared by title and body, and one more block for the heading "INET-4.7.x (in development)". The log of warnings went from 89 lines to 41: six warnings "Title underline too short" are gone (D-5), and no warning is new.

**The gate (step 5).** Over #1225, `check-whatsnew.sh` reports two violations: the entry in `WHATSNEW`, and `49e762915 adds the section "Ipv6NetworkConfigurator Gateway-Only Routes" to doc/src/migration-guide/index.rst; write it as doc/src/migration-guide/development/ipv6-ipv6networkconfigurator-gateway-only-routes.rst`. The 189 enforcement tests pass.

**The trial (step 6).** On 2026-10-20, each of the 101 open pull requests was rebased with `whatsnew.py rebase` onto `dff3e2efd0`, in a scratch worktree, and then discarded. Nothing was pushed.

| Pull requests | Rebase without a stop | Stop |
| --- | --- | --- |
| Add to `WHATSNEW`: 64 | 32 (30 in the trial of `whatsnew-entry-files.md`) | 32 |
| Do not add to `WHATSNEW`: 37 | 15 | 22 |

- No pull request stops in `WHATSNEW` or in the migration guide.
- The first stop of the other 54 is in `tests/fingerprint/*.csv` alone for 26, and in source or other files for 28, most of them in the 802.11 MIB and management. These conflicts are outside this plan.
- The tool made 15 section files of `development/`. #1225, rebased, builds its section below the heading of 4.7.x without a new warning.
- The trial stops at the first conflict of a pull request, so a pull request that stopped on the guide before can now stop on a later file. The numbers count the result, not the files that each pull request still has to resolve.

**After the landing.**

- An author of an open pull request that adds a section to the guide runs `whatsnew.py rebase`, as for `WHATSNEW` (`doc/whatsnew/README.md`).
- The fingerprint CSVs cause 37 of the 54 stops. They stay as they are: the maintainer decided on 2026-10-20 that a rebase must resolve a conflict in a baseline for real.

