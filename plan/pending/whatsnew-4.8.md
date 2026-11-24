# The release notes of INET 4.8

## Summary

The release notes of the version in development are 159 small entry files in `doc/whatsnew/`. At the release, `whatsnew.py release` writes them into `WHATSNEW` as three numbered lists. That gives about 1070 lines and 87 backward incompatible entries. The sections of 4.7 and 4.6 are different: an introduction with the highlights, and about 20 large entries, each about one theme. A reader who decides about an upgrade reads that form much faster.

This plan merges the entries into themes, adds the user-visible changes since 4.7 that have no entry, names the external contributors, and writes the introduction. The release tool stays the way to make the release: the themes are entry files, and the introduction is text in the section of the version in development.

## Facts

- Revision: `origin/master` d69fc0eaff (2026-11-24). Branch `topic/whatsnew-4.8`, worktree `/home/levy/workspace/inet-whatsnew-4.8`.
- 1816 commits since `v4.7.0` (2026-07-07), 735 of them change `src/`. 549 of the `src/` commits change no release note.
- `doc/whatsnew/`: 87 incompatible, 32 compatible and 40 fix entries. `whatsnew.py preview --rev origin/master` gives 1069 lines.
- `whatsnew.py release` orders the entries of a list by file name. Text in the section of the version in development, other than the sentence that points to `doc/whatsnew/`, becomes the introduction of the release (`doc/whatsnew/README.md`, "Make a release").
- The 4.7 section points to `src/ChangeLog`, but `src/ChangeLog` has no part for 4.7 or 4.8. Its last part is 4.6.0.
- `src/inet/common/INETDefs.h` needs OMNeT++ 6.4 or later. The CI workflows build with the `omnetpp-6.x` branch, now version 6.5.0.
- Authors of the entry files (the author of the first text of the entry, also for the entries that moved from `WHATSNEW` on 2026-10-20): Miguel González-López 35, Afshin Zanganeh 2, Gyorgy Szaszko 20, tabgab 2, the core team 100.

## Decisions

The owner chose D-1 to D-3 on 2026-11-24.

- **D-1 Themes, as in 4.7.** Merge the small entries and the missing changes into themed entries. Keep the user-visible facts of each small entry.
- **D-2 Prepare, release later.** Do not run `whatsnew.py release`. The themes are entry files in `doc/whatsnew/`, and the introduction is in the section of the version in development. A pull request that lands before the release still adds its own entry to 4.8.
- **D-3 Credits.** Name Miguel González-López and Afshin Zanganeh, in the entry of their work ("contributed by", as 4.6 and 4.7 do) and in the introduction. Do not name tabgab, Gyorgy Szaszko or MakerYuichi.
- **D-4 File names.** A themed file keeps the name `<area>-<topic>.txt` of RR-WHATSNEW-FILE. The tool then orders the entries by area, not by importance. The introduction gives the highlights in the order of importance.
- **D-5 Changed results in one place.** One entry lists the models whose simulation results changed on purpose, and why (RR-FINGERPRINT-NOTE).
- **D-6 No pointer to `src/ChangeLog`.** That file has no part for 4.7 or 4.8.
- **D-7 Kernel versions.** The introduction gives the minimum OMNeT++ version and the version that the CI used (RR-KERNEL-VERSION).

## Steps

1. [ ] Audit. Five agents read the commits of `src/` (transport, link and physical layers, network layer and routing, common and queueing) and the commits outside `src/` (examples, showcases, tests, build, user documentation). Each one reports the user-visible changes without an entry, the entries that a later commit made wrong, and the commits that moved a fingerprint or statistical baseline. No commit.
2. [ ] Themes. Choose the themes from the entries and the audit, and write the list into this plan. Commit: the plan.
3. [ ] Themed entries, one commit for each list: incompatible, compatible, fixes. Each commit removes the small files of its list and adds the themed files.
4. [ ] The entry of the changed results (D-5). Commit.
5. [ ] The introduction in `WHATSNEW` (D-3, D-6, D-7). Commit.
6. [ ] Check. `whatsnew.py check`; a trial `whatsnew.py release` in a scratch clone, compared with the 4.7 section; a reviewer agent that compares the themed text with the 159 old entries and with the audit; `check-pull-request.sh` on the branch.
7. [ ] Move this plan to `plan/done/`. Commit.

## Verification

- No fact of an old entry is lost, unless the audit shows that a later commit made it wrong (step 6, reviewer).
- Each user-visible change of the audit is in a theme or is listed below as left out, with the reason.
- `whatsnew.py check` passes, and the trial release gives one 4.8 section with the introduction and three lists.
