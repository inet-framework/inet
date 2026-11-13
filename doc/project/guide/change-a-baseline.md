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

The body of the message gives the reason ([TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance)):

```
ethernet: reset the PLCA burst counter after the last frame of the burst

The counter was reset one transmission early, so a burst carried one frame
fewer than 802.3cg 148.4.5.4 requires.

The 14 fingerprints below all use PLCA with maxBurstCount > 1 and now show
one more frame per burst. No other configuration moves.
```

"Fingerprints updated" is not a reason. Two years later this message is the only thing that tells a developer with a bisect in hand whether the movement was intended.

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

## A baseline in another repository

The statistical baseline is in the `statistics` repository, and the chart baseline is in the `media` repository. `inet.opp` pins one commit of each ([TR-BASELINE-PIN](../rule/testing.md#tr-baseline-pin)). Steps 1 to 3 apply as they are. The other steps change as follows.

### Get the checkouts and the candidate values

```bash
opp_checkout_test_baselines                 # statistics/ and media/ at the pins of this commit
opp_run_statistical_tests -m release -w '<simulation-directory-regex>' -c '<config-regex>'
opp_run_chart_tests -m release -w '<showcase-directory-regex>'
```

A test and an update never move a checkout. They give a warning when a checkout is not at its pin.

Record the candidate values in the CI environment. When the statistical or the chart tests of a pull request fail, the run uploads the artifact `statistical-candidates` or `chart-candidates`: the new files of the failed tests, `moved-results.txt` with the block for the record, and `environment.txt` with the versions. The local copy of the CI job gives the same files. Values from a developer machine can differ in the last digit, and such a baseline fails in CI ([TR-BASELINE-DELIBERATE](../rule/testing.md#tr-baseline-deliberate)). The update commands change only the files of the tests that move:

```bash
opp_update_statistical_test_results -m release -w '<simulation-directory-regex>' -c '<config-regex>'
opp_update_chart_test_results -m release -w '<showcase-directory-regex>'
```

Check the parent commit first. A test that you update must pass on the parent with the old pins ([TR-BASELINE-PARENT](../rule/testing.md#tr-baseline-parent)). A test that fails on the parent keeps its baseline: take its file out of the update, and name the test in the record.

### Make the cause small

The reviewer accepts the new values through a small change ([TR-BASELINE-ATTRIBUTION](../rule/testing.md#tr-baseline-attribution)). There are three ways to give one, the best first:

1. **Divide the series.** The first commits prepare and move no result. The last commit is small and turns the behavior on, and it moves the pin. The form is `commit`.
2. **A forward reduction.** Write a patch for the parent of the cause commit that gives the new results. Check it: apply the patch to the parent, build, and run the moved tests against the new baseline.
3. **A reverse reduction.** Write a patch for the cause commit that gives the old results. Check it: apply the patch to the cause commit, build, and run the moved tests against the old baseline.

A reduction is an experiment, not code that lands. Its limit is 100 changed lines, and the gate gives a note above 30. Put the file into the folder `attribution/` of the baseline commit. The attribution run below does the checks of each form.

### Commit the values with their record

Make one baseline commit for one INET commit, on a branch with the name of the INET branch. Stage only the files that move. Then get the summary of the staged change:

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

Moved results:
  tests: 141 changed, 0 added, 0 removed
  rows of the changed tests: 138836 changed, 9 added, 9 removed
  folders: examples/wireless 54, showcases/wireless 25, ...
  groups of the changed tests, by the names of their moved rows:
  - 95 test(s): interference cache hit, reception cache hit, ...
  - 44 test(s): more than 12 names each
  - 1 test(s): reception decision cache hit
  names of the moved rows:
  - queueLength:timeavg: 908 changed, -100 % to +3.34e+04 %
  - ...

INET-Subject: physicallayer: change: decide the header and the data of a reception separately
Attribution: commit
```

- Paste the `Moved results:` block as the tool prints it. The gate computes the block again and compares.
- Explain each group of the block. A group of one test is an exception that the reader will ask about.
- For a chart, look at the old and the new image, and say how the picture changes: which curve or bar moves, and by how much.
- `INET-Subject:` is the exact subject of the INET commit that moves the pin.

### Pin the baseline commit

Write the hash of the baseline commit into the `ref` of its entry in `inet.opp`, in the INET commit that causes the move ([PR-SPLIT-BASELINE](../rule/pull-request.md#pr-split-baseline)). Name `statistical` or `chart` in the obligations of its `Change:` trailer. Then run the gate with the local clones:

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

The tool runs the moved tests at the parent with the old pins, and at the cause commit with the new pins. For a reduction, it also runs them with the patch. A run that selects other tests than the moved tests fails. `run` checks out and builds each commit in the work tree, so use a checkout for this purpose only, such as the local copy of the CI job; its options `--build`, `--checkout`, `--statistical` and `--chart` take the commands of that copy. Put the result lines of the tool into the description of the pull request.

### Push and link

Push the baseline branch to the baseline repository. A contributor with no push right opens a pull request from a fork of the baseline repository. Name the branch or the pull request in the description of the INET pull request ([PR-REQ-STORY](../rule/pull-request.md#pr-req-story)). The maintainer approves the values in the baseline repository; that approval is apart from the approval of the source change.

### A commit that changes no result

The `media` repository also holds documentation media: network images, screenshots and videos. Only the images that a chart of an analysis file exports are results; `python3 doc/project/enforcement/baseline_summary.py media` lists the results of a staged change and leaves the other files out. A commit that changes no result needs no record and no attribution. Commit it to the branch of the `media` repository, and move the pin to it in the INET commit that needs the files. A pin can move over more than one commit, but only one of them can change results.

### After a baseline commit or the base changes

A baseline commit gets a new hash when its message or its values change, and when its branch is rebased. A rebase is necessary after another change lands and moves the pin of `master`. The pins of your series then name commits that are gone. First rebase the baseline branch, then let the tool make the series again with the new pins:

```bash
git -C statistics rebase --onto <pin of origin/master> <old pin>
python3 doc/project/enforcement/baseline_pins.py rebase origin/master
```

The tool keeps the messages, the authors and the changes of the commits. It gives the pin moves of the series the commits of the baseline branch in order; a pin move takes as many commits as it took before. It pushes nothing.

### When the change lands

Land the INET pull request only. Do not merge the pull request of the baseline repository with the button of GitHub: a merge commit or a rebase makes a new commit that no INET commit pins.

After each push to INET `master`, the workflow `check-baseline-masters.yml` moves `master` of each baseline repository to its pin by a fast-forward. GitHub then shows the pull request of the baseline repository as merged, because its head is on `master`. When the workflow has no deploy key, it fails and its log gives the command for a maintainer:

```bash
git push git@github.com:inet-framework/statistics.git <pin>:refs/heads/master
```

The push is not a fast-forward when `master` of the baseline repository holds a commit that the pin does not hold, for example a commit that a person pushed by hand. The workflow then fails and changes nothing. Pin that commit in an INET commit, or move `master` back by hand.

## At release time

A release that deliberately moves trajectories says which models moved and why ([RR-FINGERPRINT-NOTE](../rule/release.md#rr-fingerprint-note)). A user whose results differ after an upgrade must be able to find out, in one place, whether their model was one of them.
