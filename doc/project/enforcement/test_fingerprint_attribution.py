#!/usr/bin/env python3
"""Tests of the size limits and the reductions that the baseline gate (check-baselines.sh) holds for a commit
that changes fingerprint values (TR-BASELINE-ATTRIBUTION for tests/fingerprint/store.json).
"""

from pathlib import Path
import sys
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import check_baselines  # noqa: E402
import test_check_baselines as base  # noqa: E402
import test_fingerprint_changes as fp  # noqa: E402

OLD = [fp.entry("examples/a")]
MOVED = [fp.entry("examples/a", fingerprint="2222-2222")]


def removing_patch(count, total, path="src/inet/a/A.cc"):
    """A reverse reduction: it removes the last count of the total lines v1... that the commit appended."""
    before = total - count
    context = "int a = 0;" if before == 0 else f"int v{before};"
    return (f"--- a/{path}\n+++ b/{path}\n@@ -{before + 1},{count + 1} +{before + 1} @@\n {context}\n"
            + "".join(f"-int v{before + 1 + i};\n" for i in range(count)))


class AttributionTestCase(fp.FingerprintGateTestCase):
    def moved_commit(self, source, **options):
        """A seed store and a commit that moves a fingerprint value and appends source lines."""
        self.seed(OLD)
        return self.store_commit(MOVED, source=source, **options)

    def assert_flags(self, text, result=None):
        result = result or self.fingerprint_gate()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn(text, result.stdout)
        return result

    def assert_clean(self, result=None):
        result = result or self.fingerprint_gate()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.violations(result), [])
        return result


class CommitFormTest(AttributionTestCase):
    def test_a_commit_of_30_source_lines_that_moves_a_value_has_no_note(self):
        self.moved_commit(30)
        result = self.assert_clean()
        self.assertNotIn("source lines, above", result.stdout)

    def test_a_commit_of_31_source_lines_that_moves_a_value_gives_a_note(self):
        self.moved_commit(31)
        result = self.assert_clean()
        self.assertIn("changes 31 source lines, above 30, and moves fingerprint values", result.stdout)

    def test_a_commit_of_100_source_lines_that_moves_a_value_gives_only_a_note(self):
        self.moved_commit(100)
        result = self.assert_clean()
        self.assertIn("changes 100 source lines, above 30", result.stdout)

    def test_a_commit_of_101_source_lines_that_moves_a_value_fails(self):
        self.moved_commit(101)
        self.assert_flags("changes 101 source lines, above 100, and moves fingerprint values")

    def test_a_commit_with_no_source_line_has_no_limit(self):
        self.seed(OLD, files={"src/inet/a/NOTES.md": "x\n", "examples/a/doc/x.txt": "x\n"})
        self.store_commit(MOVED, files={"src/inet/a/NOTES.md": "changed\n", "examples/a/doc/x.txt": "changed\n"})
        result = self.assert_clean()
        self.assertNotIn("source lines", result.stdout)

    def test_a_commit_that_only_adds_runs_has_no_limit_for_200_source_lines(self):
        self.seed(OLD)
        self.store_commit(OLD + [fp.entry("examples/b")], source=200)
        result = self.assert_clean()
        self.assertNotIn("source lines", result.stdout)

    def test_a_commit_that_only_removes_runs_has_no_limit_for_200_source_lines(self):
        self.seed(OLD + [fp.entry("examples/b", fingerprint="7777-7777")])
        self.store_commit(OLD, source=200)
        self.assert_clean()

    def test_a_commit_that_only_adds_ingredient_sets_has_no_limit_for_200_source_lines(self):
        self.seed(OLD)
        self.store_commit(OLD + [fp.entry("examples/a", ingredients="x", fingerprint="aaaa-bbbb")], source=200)
        self.assert_clean()

    def test_a_commit_that_only_changes_the_limit_has_no_limit_for_200_source_lines(self):
        self.seed(OLD)
        self.store_commit([fp.entry("examples/a", limit="20s")], source=200)
        self.assert_clean()

    def test_a_commit_that_adds_a_run_and_moves_a_value_has_the_limit(self):
        self.seed(OLD)
        self.store_commit(MOVED + [fp.entry("examples/b")], source=101)
        self.assert_flags("above 100, and moves fingerprint values")

    def test_the_size_limit_holds_for_each_commit_of_the_series(self):
        self.seed(OLD)
        self.store_commit(MOVED, source=1)
        self.store_commit([fp.entry("examples/a", fingerprint="3333-3333")], source=101)
        self.assert_flags("changes 101 source lines, above 100")


class ReductionFormTest(AttributionTestCase):
    def test_a_reverse_reduction_that_applies_to_the_commit_passes(self):
        self.moved_commit(1, reduction=base.REVERSE_PATCH, trailers=["Attribution: reverse"])
        self.assert_clean()

    def test_a_forward_reduction_that_applies_to_the_parent_passes(self):
        self.moved_commit(1, reduction=base.FORWARD_PATCH, trailers=["Attribution: forward"])
        self.assert_clean()

    def test_a_reduction_makes_a_large_commit_pass(self):
        self.moved_commit(200, reduction=removing_patch(1, 200), trailers=["Attribution: reverse"])
        self.assert_clean()

    def test_the_size_of_a_reduction_of_30_lines_gives_no_note(self):
        self.moved_commit(40, reduction=removing_patch(30, 40), trailers=["Attribution: reverse"])
        result = self.assert_clean()
        self.assertNotIn("has a reduction of", result.stdout)

    def test_the_size_of_a_reduction_of_31_lines_gives_a_note(self):
        self.moved_commit(40, reduction=removing_patch(31, 40), trailers=["Attribution: reverse"])
        result = self.assert_clean()
        self.assertIn("has a reduction of 31 changed lines, above 30", result.stdout)

    def test_the_size_of_a_reduction_of_100_lines_gives_only_a_note(self):
        self.moved_commit(100, reduction=removing_patch(100, 100), trailers=["Attribution: reverse"])
        result = self.assert_clean()
        self.assertIn("has a reduction of 100 changed lines, above 30", result.stdout)

    def test_the_size_of_a_reduction_of_101_lines_fails(self):
        self.moved_commit(101, reduction=removing_patch(101, 101), trailers=["Attribution: reverse"])
        self.assert_flags("has a reduction of 101 changed lines, above 100")

    def test_a_forward_reduction_that_applies_only_to_the_commit_fails(self):
        self.moved_commit(1, reduction=base.REVERSE_PATCH, trailers=["Attribution: forward"])
        self.assert_flags("has a forward reduction that does not apply to the parent of the commit")

    def test_a_reverse_reduction_that_applies_only_to_the_parent_fails(self):
        self.moved_commit(1, reduction=base.FORWARD_PATCH.replace("+int v1;", "+int other;"), trailers=["Attribution: reverse"])
        self.assert_flags("has a reverse reduction that does not apply to the commit")

    def test_a_patch_that_does_not_apply_fails(self):
        self.moved_commit(1, reduction=base.BAD_PATCH, trailers=["Attribution: reverse"])
        self.assert_flags("has a reverse reduction that does not apply to the commit")

    def test_a_reverse_attribution_with_no_reduction_fails(self):
        self.moved_commit(1, trailers=["Attribution: reverse"])
        self.assert_flags("has the Attribution reverse and no patch after a line 'Reduction:'")

    def test_a_forward_attribution_with_no_reduction_fails(self):
        self.moved_commit(1, trailers=["Attribution: forward"])
        self.assert_flags("has the Attribution forward and no patch after a line 'Reduction:'")

    def test_a_reduction_with_no_attribution_fails(self):
        self.moved_commit(1, reduction=base.REVERSE_PATCH)
        self.assert_flags("has a 'Reduction:' patch and no Attribution forward or reverse")

    def test_a_reduction_with_the_attribution_commit_fails(self):
        self.moved_commit(1, reduction=base.REVERSE_PATCH, trailers=["Attribution: commit"])
        self.assert_flags("has a 'Reduction:' patch and no Attribution forward or reverse")

    def test_the_attribution_commit_with_no_reduction_passes(self):
        self.moved_commit(1, trailers=["Attribution: commit"])
        self.assert_clean()

    def test_an_unknown_form_fails(self):
        self.moved_commit(1, trailers=["Attribution: first-record"])
        self.assert_flags("has the Attribution 'first-record'; for fingerprints the forms are commit, forward and reverse")

    def test_a_form_with_a_file_name_fails(self):
        self.moved_commit(1, reduction=base.REVERSE_PATCH, trailers=["Attribution: reverse x.patch"])
        self.assert_flags("has the Attribution 'reverse x.patch'; for fingerprints the forms are")

    def test_two_attribution_trailers_fail(self):
        self.moved_commit(1, reduction=base.REVERSE_PATCH, trailers=["Attribution: reverse", "Attribution: reverse"])
        self.assert_flags("has 2 Attribution trailers; one cause has one attribution")

    def test_a_reduction_with_an_empty_context_line_that_lost_its_space_applies(self):
        # git strips the trailing space of a line of a message, so the empty context line is an empty line
        self.seed(OLD, files={"src/inet/a/B.cc": "int x;\n\nint y;\n"})
        patch = "--- a/src/inet/a/B.cc\n+++ b/src/inet/a/B.cc\n@@ -1,4 +1,3 @@\n int x;\n\n int y;\n-int z;\n"
        self.write(self.inet, "src/inet/a/B.cc", "int x;\n\nint y;\nint z;\n")
        self.store_commit(MOVED, reduction=patch, trailers=["Attribution: reverse"])
        message = self.git(self.inet, "log", "-1", "--format=%B").stdout
        self.assertIn("int x;\n\n int y;", message)
        self.assert_clean()


class ReductionInMessageTest(unittest.TestCase):
    PATCH = "--- a/f\n+++ b/f\n@@ -1 +1,2 @@\n a\n+b\n"

    def test_it_gives_the_patch_between_the_reduction_line_and_the_trailers(self):
        message = f"subject\n\nbody\n\nReduction:\n{self.PATCH}\nChange: src | behavior.change | fingerprint\nAttribution: forward\n"
        trailers = ["Change: src | behavior.change | fingerprint", "Attribution: forward"]
        self.assertEqual(check_baselines.reduction_in_message(message, trailers), self.PATCH)

    def test_it_drops_the_blank_lines_before_the_trailers(self):
        message = f"subject\n\nReduction:\n{self.PATCH}\n\n\nAttribution: reverse\n"
        self.assertEqual(check_baselines.reduction_in_message(message, ["Attribution: reverse"]), self.PATCH)

    def test_it_gives_the_patch_to_the_end_of_a_message_with_no_trailers(self):
        self.assertEqual(check_baselines.reduction_in_message(f"subject\n\nReduction:\n{self.PATCH}", []), self.PATCH)

    def test_a_message_with_no_reduction_line_gives_none(self):
        self.assertIsNone(check_baselines.reduction_in_message("subject\n\nbody\n\nAttribution: reverse\n", ["Attribution: reverse"]))

    def test_a_reduction_line_with_nothing_after_it_gives_none(self):
        self.assertIsNone(check_baselines.reduction_in_message("subject\n\nReduction:\n\nAttribution: reverse\n", ["Attribution: reverse"]))


class PinnedReductionTest(AttributionTestCase):
    """A commit that moves the statistical pin and changes fingerprints has one cause, so one attribution."""

    def pinned_commit(self, trailers=(), reduction=None):
        self.seed(OLD)
        pin = self.record(self.stat, {base.SCA_A: base.sca(sent="11"), base.PATCH: base.REVERSE_PATCH},
                          attribution=f"reverse {base.PATCH}")
        self.write(self.inet, "inet.opp", base.opp_text({**self.pins, "statistical": pin}))
        self.pins["statistical"] = pin
        self.store_commit(MOVED, source=1, trailer=base.CHANGE + "statistical fingerprint", trailers=list(trailers),
                          reduction=reduction)

    def test_a_fingerprint_commit_with_no_attribution_uses_the_reduction_of_the_pinned_baseline_commit(self):
        self.pinned_commit()
        self.assert_clean()

    def test_the_attribution_commit_in_the_message_fails_against_a_pinned_reverse_reduction(self):
        self.pinned_commit(trailers=["Attribution: commit"])
        self.assert_flags("one cause has one attribution")

    def test_a_reduction_in_the_message_that_differs_from_the_pinned_one_fails(self):
        self.pinned_commit(trailers=["Attribution: reverse"], reduction=base.REVERSE_PATCH.replace("v1", "v1;\n-int v2"))
        self.assert_flags("differs from the reduction file of its baseline commits")

    def test_a_reduction_in_the_message_that_equals_the_pinned_one_passes(self):
        self.pinned_commit(trailers=["Attribution: reverse"], reduction=base.REVERSE_PATCH)
        self.assert_clean()


if __name__ == "__main__":
    unittest.main()
