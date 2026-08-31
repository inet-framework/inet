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

## 4. Commit it alone

**The baseline update is its own commit, directly after the commit that changes behavior**
([PR-SPLIT-BASELINE](../rule/pull-request.md#pr-split-baseline),
[TR-BASELINE-COMMIT](../rule/testing.md#tr-baseline-commit)). No source change in it.

The message names the commit that causes the change and gives the reason
([TR-BASELINE-PROVENANCE](../rule/testing.md#tr-baseline-provenance)):

```
tests(fingerprint): update the Ethernet baselines for the PLCA burst fix

<sha> repairs the burst counter, which was reset one transmission early.
The 14 configurations below all use PLCA with maxBurstCount > 1 and now
show one more frame per burst, which is what 802.3cg 148.4.5.4 requires.
```

"Fingerprints updated" is not a reason. Two years later this message is the only thing that tells a
bisecting developer whether the movement was intended.

## 5. Name it in the pull request

The description lists every baseline update
([PR-REQ-STORY](../rule/pull-request.md#pr-req-story)). A reviewer who has to discover a baseline
change by reading the file list has already been failed by the description.

## At release time

A release that deliberately moves trajectories says which models moved and why
([RR-FINGERPRINT-NOTE](../rule/release.md#rr-fingerprint-note)). A user whose results differ after an
upgrade must be able to find out, in one place, whether their model was one of them.
