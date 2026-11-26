# Change a baseline

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rule/testing.md](../rule/testing.md), [rule/pull-request.md](../rule/pull-request.md)

How to change a recorded expectation — a fingerprint in `tests/fingerprint/store.json`, a statistical baseline, a chart baseline, an expected output — and what must go with it. Steps 1 to 6 use the fingerprint as the example. [A baseline in another repository](#a-baseline-in-another-repository) gives the differences for the statistical and chart baselines.

**A baseline is a claim, not a side effect.** It says *these values are correct*. Regenerating one to make CI green is the fastest way to lose every regression guarantee the suite gives, and in a large diff nobody sees it happen.

## 1. Establish that the change is intended

Before you regenerate anything, answer in words: **which behavior moved, and why is the new behavior the right one?** A standard clause, a repaired defect, a deliberate model change. If you cannot answer, the fingerprint has found a regression and the baseline is correct as it stands.

## 2. Obtain explicit approval

Present the exact baselines or configurations expected to move, the source change that causes the movement, and the reason the new values will be right. Obtain explicit approval from the human responsible for accepting the change before regenerating anything ([TR-BASELINE-DELIBERATE](../rule/testing.md#tr-baseline-deliberate)). Permission to make the source change does not by itself authorize rewriting its expectations.

If investigation is still needed, keep the failing outputs as evidence. Investigation may calculate candidate values, but those values do not replace a tracked baseline until approval is recorded.

## 3. Check the scope of the movement

```bash
opp_run_fingerprint_tests -f '<affected-case-regex>'
```

Start with an explicit filter for the configurations directly related to the changed contract and record the command, mode, configuration, run/seed, status and generated artifacts ([TR-FOCUSED-EVIDENCE](../rule/testing.md#tr-focused-evidence)). A broader integration run may then look for collateral movement, but it does not replace the focused evidence. A change that moves far more configurations than expected is telling you something. A serializer change that moves a mobility fingerprint is not a serializer change.

## 4. Regenerate

After approval, regenerate only the baselines the change actually moves. A blanket regeneration sweeps up unrelated drift and hides it under your reason. For fingerprints, run `opp_update_fingerprint_test_results -f '<affected-case-regex>'` with the filter of step 3, and read `git diff tests/fingerprint/store.json`.

## 5. Commit it with the change that causes it

**The regenerated values go in the same commit as the source change that moves them** ([PR-SPLIT-BASELINE](../rule/pull-request.md#pr-split-baseline), [TR-BASELINE-COMMIT](../rule/testing.md#tr-baseline-commit)). The commit then passes its own fingerprint test, and `git bisect` over the suite gives a true answer.

The body of the message gives the reason ([TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance)): the behavior that moves, and the `Baseline changes:` list with a reason on each line. Get the computed list of the staged store:

```bash
python3 doc/project/enforcement/fingerprint_moves.py --list <(git show HEAD:tests/fingerprint/store.json) tests/fingerprint/store.json
```

Copy the list into the message, and write the reason of each line after " — ". A line that starts with 4 or more spaces continues the reason:

```
ethernet: reset the PLCA burst counter after the last frame of the burst

The counter was reset one transmission early, so a burst carried one frame fewer than 802.3cg 148.4.5.4 requires.

Baseline changes:
  fingerprint runs changed in tplx,~tND,~tNl (14; examples/ethernet 14) — these runs use PLCA
    with maxBurstCount > 1, and each burst now carries one more frame
```

"Fingerprints updated" is not a reason. Two years later this message is the only thing that tells a developer with a bisect in hand whether the movement was intended. Each line of the list is one way in which the runs move. Two lines usually have two causes: give each its own reason, or divide the commit ([PR-SPLIT-ONE-CHANGE](../rule/pull-request.md#pr-split-one-change)).

**Make the code that moves the values small** ([TR-BASELINE-ATTRIBUTION](../rule/testing.md#tr-baseline-attribution)). The reviewer accepts the new values because the reviewer can read the change and see that it is correct. A commit of 100 changed lines is wrong when 10 of its lines move the same fingerprints: put the other 90 lines into a commit before it that moves no fingerprint. The gate counts the changed source lines of the commit. Above 100 lines it fails, and above 30 lines it gives a note.

If the commit cannot be small, give a reduction: a small patch that shows the same move. Put the patch at the end of the message, after a line `Reduction:`, and name its direction in the trailer `Attribution:`. `reverse` is a patch for the commit that gives the old fingerprints; `forward` is a patch for the parent that gives the new fingerprints. Write the patch with `git diff`, and keep it in the message as it is:

```
Baseline changes:
  fingerprint runs changed in tplx,tyf (7; examples/tcp 7) — TcpWestwood takes an RTT sample
    from each ACK of new data, so its congestion window grows faster

Reduction:
diff --git a/src/inet/transportlayer/tcp/flavours/TcpWestwood.cc b/src/inet/transportlayer/tcp/flavours/TcpWestwood.cc
...

Change: ...
Attribution: reverse
```

A reduction is an experiment, not code that lands. The check `Check: baseline attribution` applies the patch, builds, and runs the moved fingerprint tests. A fingerprint change that only adds, removes or renames runs, or that changes no source line, needs no attribution.

The reviewer reads the source change alone with `git show <sha> -- . ':!tests'`.

### When the update stands alone

A re-record that no single commit causes keeps a commit of its own: a compiler, tool or solver version change, or drift that came from outside the branch. Then the message must carry the whole reason, because no commit beside it can:

```
tests(fingerprint): re-record the 11 z3-dependent gsandcs baselines for clang-23

The SAT solver of clang-23 chooses a different model, which changes the
schedule these configurations record. The behavior of INET does not change.
```

Do not put such a re-record inside an unrelated fix. It has no cause there.

## 6. Name it in the pull request

The description lists every baseline update ([PR-REQ-STORY](../rule/pull-request.md#pr-req-story)). A reviewer who has to discover a baseline change by reading the file list has already been failed by the description.

Give the description a section `## Baseline changes` ([PR-REQ-BASELINES](../rule/pull-request.md#pr-req-baselines)). It has one entry for each commit that changes the fingerprint store or has a pin trailer whose step changes results: the exact subject of the commit, then a summary of its baseline changes in your words. A series can have more than one such commit, and the section is the one place where the reviewer sees all of them:

```
## Baseline changes

- ethernet: reset the PLCA burst counter after the last frame of the burst
  14 fingerprint runs of examples/ethernet move: each PLCA burst carries one more frame.
```

The lists with a reason for each line stay in the commits. The report of the gate prints the computed lists beside the section.

## A baseline in another repository

The statistical baseline is in the `statistics` repository, and the chart baseline is in the `media` repository. The commit that moves a result names one commit of each in a pin trailer of its message, and `inet.opp` does not change ([TR-BASELINE-PIN](../rule/testing.md#tr-baseline-pin)). Steps 1 to 3 apply as they are. The other steps change as follows.

### Get the checkouts and record the values

```bash
opp_checkout_test_baselines                 # statistics/ and media/ at the baseline of this commit
opp_run_statistical_tests -m release -w '<simulation-directory-regex>' -c '<config-regex>'
opp_run_chart_tests -m release -w '<showcase-directory-regex>'
```

The command finds the baseline of the commit from the pin trailers of the series and the marks on `master` of each baseline repository, and prints how. A test and an update never move a checkout. They give a warning when a checkout does not have the files of the baseline of the commit.

Record the new values on your machine. The update commands change only the files of the tests that move:

```bash
opp_update_statistical_test_results -m release -w '<simulation-directory-regex>' -c '<config-regex>'
opp_update_chart_test_results -m release -w '<showcase-directory-regex>'
```

Check the parent commit first. A test that you update must pass on the parent against the baseline of the parent ([TR-BASELINE-PARENT](../rule/testing.md#tr-baseline-parent)). A test that fails on the parent keeps its baseline: take its file out of the update, and name the test in the record.

### Make the cause small

The reviewer accepts the new values through a small change ([TR-BASELINE-ATTRIBUTION](../rule/testing.md#tr-baseline-attribution)). There are three ways to give one, the best first:

1. **Divide the series.** The first commits prepare and move no result. The last commit is small and turns the behavior on, and it has the pin trailer. The form is `commit`.
2. **A forward reduction.** Write a patch for the parent of the cause commit that gives the new results. Check it: apply the patch to the parent, build, and run the moved tests against the new baseline.
3. **A reverse reduction.** Write a patch for the cause commit that gives the old results. Check it: apply the patch to the cause commit, build, and run the moved tests against the old baseline.

A reduction is an experiment, not code that lands. Its limit is 100 changed lines, and the gate gives a note above 30. Put the file into the folder `attribution/` of the baseline commit. The attribution run below does the checks of each form.

### Commit the values with their record

Make one baseline commit for one INET commit, on a branch with the name of the INET branch. The branch starts on the head of `master` of the baseline repository, and the next steps of the series go on the same branch:

```bash
git -C statistics fetch origin
git -C statistics switch -c <INET branch> origin/master
```

Record on a series that stands on the head of INET `master`. Then the checkout of the baseline and the head of `master` of the baseline repository have the same files, and the values come from the code that they land on. Stage only the files that move. Then get the computed list of the staged change:

```bash
python3 doc/project/enforcement/baseline_summary.py statistics
python3 doc/project/enforcement/baseline_summary.py media
```

The message of the baseline commit holds the record ([TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance)):

```
physicallayer: update the statistics after the separate part decisions

The INET commit decides the errors of the header and of the data of a
reception separately. Two groups of results move.

The cache counters of the radio medium move in all 141 configurations.
The medium computes three decisions for each reception instead of one,
so "reception decision computation count" grows by the factor 3 ...

The trajectory moves in 44 of the configurations. A reception can draw
two random numbers instead of one ...

Baseline changes:
  statistical tests changed, large (95; examples/wireless 38, ...): interference cache hit, reception cache hit and 4 more — the medium
    computes three decisions for each reception instead of one, so the cache counters grow
  statistical tests changed, large (44; examples/manetrouting 19, ...): queueLength, packetDrop and 1104 more — a reception can
    draw two random numbers, so the drops and the queues move
  statistical tests changed, large (1; showcases/wireless 1): reception decision cache hit — this configuration has no
    interference, so only the decision cache counts more

INET-Subject: physicallayer: change: decide the header and the data of a reception separately
Attribution: commit
```

- Copy the list as the tool prints it, and write a reason after " — " on each line. Keep the computed text and " — " on the first line of the item; a line that starts with 4 or more spaces continues only the reason. The gate computes the list again, compares the lines, and fails a line with no reason.
- A line of one test is an exception that the reader will ask about. Its reason says why that test moves in another way.
- More prose in the message is welcome, but it does not replace a reason on a line.
- For a chart, look at the old and the new image, and say how the picture changes: which curve or bar moves, and by how much.
- `INET-Subject:` is the exact subject of the cause commit: the INET commit that has the pin trailer.

### Pin the baseline commit

Add the pin trailer to the message of the INET commit that causes the move ([PR-SPLIT-BASELINE](../rule/pull-request.md#pr-split-baseline)), with the full hash of the last commit of its step on the baseline branch. Do not change `inet.opp`. Name `statistical` or `chart` in the obligations of its `Change:` trailer:

```
Change: src.physicallayer | behavior.change | statistical test whatsnew
Statistical-Baseline: <full hash of the baseline commit>
```

Then run the gate with the local clones:

```bash
doc/project/enforcement/check-baselines.sh origin/master..HEAD \
    --baseline-repository statistical=statistics --baseline-repository chart=media
```

The gate does not run a test. Prove the attribution with a run ([TR-BASELINE-ATTRIBUTION](../rule/testing.md#tr-baseline-attribution)):

```bash
doc/project/enforcement/check-baseline-attribution.sh plan origin/master..HEAD \
    --baseline-repository statistical=statistics --baseline-repository chart=media
doc/project/enforcement/check-baseline-attribution.sh run origin/master..HEAD \
    --baseline-repository statistical=statistics --baseline-repository chart=media
```

The tool runs the moved tests at the parent against the baseline of the parent, and at the cause commit against its own baseline. For a reduction, it also runs them with the patch, against the baseline that the form names. A run that selects other tests than the moved tests fails. `run` checks out and builds each commit in the work tree, so use a checkout for this purpose only, such as the local copy of the CI job; its options `--build`, `--checkout`, `--statistical` and `--chart` take the commands of that copy. The check `Check: baseline attribution` does the same runs for each pull request, and its job summary holds the result. A run in the copy of the CI job gives the result before the push.

### Push and link

Push the baseline branch to the baseline repository. A contributor with no push right opens a pull request from a fork of the baseline repository. Name the branch or the pull request in the description of the INET pull request ([PR-REQ-STORY](../rule/pull-request.md#pr-req-story)). The maintainer approves the values in the baseline repository; that approval is apart from the approval of the source change.

Then check that the baseline branch can land on `master` of the baseline repository with no overlap. The pull request audit posts the same result as the commit status `Baseline merge`:

```bash
python3 doc/project/enforcement/baseline_masters.py check HEAD --base origin/master
```

### A commit that changes no result

The `media` repository also holds documentation media: network images, screenshots and videos. Only the images that a chart of an analysis file exports are results; `python3 doc/project/enforcement/baseline_summary.py media` lists the results of a staged change and leaves the other files out. A commit that changes no result needs no record and no attribution. Commit it to the branch of the `media` repository, and name it in the pin trailer of the INET commit that needs the files. A step can go over more than one commit, but only one of them can change results.

### After a baseline commit changes

A baseline commit gets a new hash when its message or its values change, for example after a new record. The pin trailers of your series then name commits that are gone. Let the tool write the new hashes into the series:

```bash
python3 doc/project/enforcement/baseline_pins.py retrail origin/master --baseline-repository statistical=statistics
```

The tool gives the pin trailers of the series the commits of the baseline branch in order; a pin takes as many commits as it took before. It keeps the messages, the authors and the files of the commits, and changes only the pin trailers. Without `--baseline <KIND>=<REVISION>`, the baseline branch is the `HEAD` of the clone. It pushes nothing.

### After another change lands

Do nothing, unless the status `Baseline merge` of your pull request is red. Another change that lands does not change your pins: the landing job puts your steps onto the new head of `master` of the baseline repository when your change lands, and the pull request tests already test against that result. A red status names the files that your baseline branch and `master` of the baseline repository both change. Then rebase the INET series onto `origin/master`, start the baseline branch again on the new head of `master` of the baseline repository, record the common results again, and run `retrail`.

### A series from before the trailer form

A series that moves the `ref` of `inet.opp` conflicts with the commit that changed `inet.opp` to the trailer form, and the gate rejects a new `ref`. Let the tool make the series again on `origin/master`:

```bash
python3 doc/project/enforcement/baseline_pins.py convert origin/master
```

The tool takes the `ref` changes out of `inet.opp`, and gives each commit that moved a `ref` the pin trailer with that `ref`. It keeps the messages, the authors and the other changes of the commits. It stops and changes nothing when another file has a conflict. It pushes nothing.

### When the change lands

Land the INET pull request only. Do not merge the pull request of the baseline repository with the button of GitHub.

After each push to INET `master`, the workflow `check-baseline-masters.yml` puts the steps of the new cause commits onto `master` of each baseline repository, the oldest first, and adds a mark: a commit with no change of files and a line `Baseline-Of: <INET hash> <baseline hash>` for each landed cause commit.

- A step that starts on the head of `master` lands by a fast-forward. GitHub then shows the pull request of the baseline repository as merged, because its head is on `master`.
- Another step lands by a rebase onto the head of `master`, with its messages and authors. The mark has a line `Baseline-Pull-Request: #<number>` for the pull request whose head is the pin. GitHub does not mark that pull request as merged, so the log and the job summary of the workflow name it, and a maintainer closes it, with no comment. Its state is then "Closed", not "Merged".

Then the workflow posts the status `Baseline merge` again on the open pull requests with a pin trailer, and the statistical and chart tests of `master` start. When the workflow has no deploy key, it fails and its log says so.

**If a landing stops.** At an overlap or a conflict, the workflow pushes the steps before the stopped INET commit with their mark, and fails. A later run tries that commit again, because no mark names it. Until then INET `master` has no baseline for that commit, and the statistical and chart tests of `master` stop with "no mark names it". A maintainer repairs it:

1. Check out the stopped INET commit and build it. Check out the head of `master` of the baseline repository: `git -C statistics fetch origin` and `git -C statistics switch --detach origin/master`. A mark changes no file, so this checkout has the files of the baseline of the parent of the stopped commit.
2. On that checkout, record the moved tests of the step again, also the common files, and commit the values with their record, as above.
3. Add the mark, and push to `master` of the baseline repository. When a pull request of the baseline repository holds the old pin, add its line too, and close that pull request:

```bash
git -C statistics commit --allow-empty -m "Baseline of INET <short hash>: <subject>" \
    -m "Baseline-Of: <full INET hash> <full hash of the new baseline commit>
Baseline-Pull-Request: #<number>"
git -C statistics push origin HEAD:refs/heads/master
```

The push is a fast-forward. Then start the workflow "Check: baseline masters" by hand on `master` (Actions, then "Run workflow"). Nothing else starts it before the next push to INET `master`. The run finds the mark and has nothing to land, it posts the status `Baseline merge` again, and its end starts the statistical and chart tests of `master`. Its later runs start after the marked commit.

## At release time

A release that deliberately moves trajectories says which models moved and why ([RR-FINGERPRINT-NOTE](../rule/release.md#rr-fingerprint-note)). A user whose results differ after an upgrade must be able to find out, in one place, whether their model was one of them.
