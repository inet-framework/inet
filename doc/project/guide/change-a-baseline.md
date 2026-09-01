# Change a baseline

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rule/testing.md](../rule/testing.md), [rule/pull-request.md](../rule/pull-request.md)

How to change a recorded expectation — a fingerprint `.csv`, a statistical baseline, an expected
output — and what must go with it.

**A baseline is a claim, not a side effect.** It says *these values are correct*. Regenerating one to
make CI green is the fastest way to lose every regression guarantee the suite gives, and in a large
diff nobody sees it happen.

## 1. Establish that the change is intended

Before you regenerate anything, answer in words: **which behavior moved, and why is the new behavior
the right one?** A standard clause, a repaired defect, a deliberate model change. If you cannot
answer, the fingerprint has found a regression and the baseline is correct as it stands.

## 2. Check the scope of the movement

```bash
cd tests/fingerprint && ./runtest
```

A change that moves far more configurations than expected is telling you something. A serializer
change that moves a mobility fingerprint is not a serializer change.

## 3. Regenerate

Regenerate only the baselines the change actually moves. A blanket regeneration sweeps up unrelated
drift and hides it under your reason.

## 4. Commit it with the change that causes it

**The regenerated values go in the same commit as the source change that moves them**
([PR-SPLIT-BASELINE](../rule/pull-request.md#pr-split-baseline),
[TR-BASELINE-COMMIT](../rule/testing.md#tr-baseline-commit)). The commit then passes its own
fingerprint test, and `git bisect` over the suite gives a true answer.

The body of the message gives the reason
([TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance)):

```
ethernet: reset the PLCA burst counter after the last frame of the burst

The counter was reset one transmission early, so a burst carried one frame
fewer than 802.3cg 148.4.5.4 requires.

The 14 fingerprints below all use PLCA with maxBurstCount > 1 and now show
one more frame per burst. No other configuration moves.
```

"Fingerprints updated" is not a reason. Two years later this message is the only thing that tells a
developer with a bisect in hand whether the movement was intended.

The reviewer reads the source change alone with `git show <sha> -- . ':!tests'`.

### When the update stands alone

A re-record that no single commit causes keeps a commit of its own: a compiler, tool or solver
version change, or drift that came from outside the branch. Then the message must carry the whole
reason, because no commit beside it can:

```
tests(fingerprint): re-record the 11 z3-dependent gsandcs baselines for clang-23

The SAT solver of clang-23 chooses a different model, which changes the
schedule these configurations record. The behavior of INET does not change.
```

Do not put such a re-record inside an unrelated fix. It has no cause there.

## 5. Name it in the pull request

The description lists every baseline update
([PR-REQ-STORY](../rule/pull-request.md#pr-req-story)). A reviewer who has to discover a baseline
change by reading the file list has already been failed by the description.

## At release time

A release that deliberately moves trajectories says which models moved and why
([RR-FINGERPRINT-NOTE](../rule/release.md#rr-fingerprint-note)). A user whose results differ after an
upgrade must be able to find out, in one place, whether their model was one of them.
