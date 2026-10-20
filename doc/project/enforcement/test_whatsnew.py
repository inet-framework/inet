#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import whatsnew  # noqa: E402

TOOL = Path(__file__).with_name("whatsnew.py").resolve()

RELEASED = """INET-4.7 (July 2026) — feature release
--------------------------------------

Notable bug fixes are the following:

1. An old repair

   It was repaired.
"""

OLD_STYLE = f"""Recent changes in the INET Framework
====================================

INET-4.7.x (in development)
---------------------------

Notable backward incompatible changes are the following:

1. Rate control per receiver

   The rate control adapts for each receiver.

     getRate(receiver)

2. A second change

   It changes.

Notable bug fixes are the following:

1. A repair

   It is repaired.

{RELEASED}"""

GUIDE_OLD = """.. _mg:cha:migrationguide:

Migrating Code
==============
Release: |release|

TCP Algorithms
--------------

The algorithm is a module.

.. _mg:sec:old:

Old Section
-----------

Old text.

An Old Subsection
~~~~~~~~~~~~~~~~~

Old subsection text.
"""

GUIDE_NEW = """.. _mg:cha:migrationguide:

Migrating Code
==============
Release: |release|

INET-4.7.x (in development)
---------------------------

.. include-files:: development/*.rst

.. _mg:sec:old:

Old Section
-----------

Old text.

An Old Subsection
~~~~~~~~~~~~~~~~~

Old subsection text.
"""

STUBBED = f"""Recent changes in the INET Framework
====================================

INET-4.7.x (in development)
---------------------------

{whatsnew.STUB}

{RELEASED}"""


class FunctionTest(unittest.TestCase):
    def test_the_entries_of_the_version_in_development(self):
        entries = whatsnew.development_entries(OLD_STYLE)
        self.assertEqual([(n, e.section, e.title) for n, _, e in entries],
                         [(1, "incompatible", "Rate control per receiver"),
                          (2, "incompatible", "A second change"),
                          (1, "fixes", "A repair")])
        self.assertEqual(entries[0][2].body,
                         ["The rate control adapts for each receiver.", "", "  getRate(receiver)"])

    def test_a_released_section_has_no_entry_of_the_version_in_development(self):
        self.assertEqual(whatsnew.development_entries(STUBBED), [])

    def test_the_lists_are_numbered_in_the_order_of_the_file_names(self):
        entries = [whatsnew.Entry("fixes", "tcp-b.txt", "B", ["Text b."]),
                   whatsnew.Entry("fixes", "ipv6-a.txt", "A", ["Text a.", "", "  table"]),
                   whatsnew.Entry("incompatible", "udp-c.txt", "C", ["Text c."])]
        self.assertEqual(whatsnew.render_lists(entries), [
            "Notable backward incompatible changes are the following:", "",
            "1. C", "", "   Text c.", "",
            "Notable bug fixes are the following:", "",
            "1. A", "", "   Text a.", "", "     table", "",
            "2. B", "", "   Text b.", ""])

    def test_a_rendered_list_reads_back_as_the_same_entries(self):
        entries = [e for _, _, e in whatsnew.development_entries(OLD_STYLE)]
        named = [e._replace(name=f"{n:02}.txt") for n, e in enumerate(entries)]
        text = "\n".join(whatsnew.heading_lines("INET-4.7.x (in development)")
                         + whatsnew.render_lists(named))
        self.assertEqual([e for _, _, e in whatsnew.development_entries(text)], entries)

    def test_the_problems_of_an_entry_file(self):
        def problems(text):
            return whatsnew.parse_entry_file(text)[2]
        self.assertEqual(problems("Title\n\nOne paragraph.\n\nTwo.\n"), [])
        self.assertIn("does not end with a newline", problems("Title\n\nText."))
        self.assertIn("has no empty line after the title", problems("Title\nText.\n"))
        self.assertIn("has no text after the title", problems("Title\n\n"))
        self.assertIn("starts its title with a number; the release numbers the entries",
                      problems("3. Title\n\nText.\n"))
        self.assertIn("line 3 ends with a space", problems("Title\n\nText. \n"))
        self.assertIn("line 4 is a second empty line", problems("Title\n\nText.\n\n\nMore.\n"))
        self.assertIn("ends with an empty line", problems("Title\n\nText.\n\n"))

    def test_the_area_of_a_commit_subject(self):
        self.assertEqual(whatsnew.area_of("ipv6: fix: keep a datagram"), "ipv6")
        self.assertEqual(whatsnew.area_of("IEEE 802.11: add HT A-MPDU"), "ieee80211")
        self.assertEqual(whatsnew.area_of("ipv6, mipv6: fix two defects"), "ipv6")
        self.assertEqual(whatsnew.area_of("Add HT compressed Block Ack support"), None)

    def test_a_file_name_does_not_repeat_the_area(self):
        self.assertEqual(whatsnew.file_name("IEEE 802.11 rate control per receiver", "ieee80211"),
                         "ieee80211-rate-control-per-receiver.txt")
        self.assertEqual(whatsnew.file_name("A packet drop reason", "common"),
                         "common-packet-drop-reason.txt")
        self.assertEqual(whatsnew.file_name("The ICMPv6 checksum is written", "icmpv6"),
                         "icmpv6-checksum-is-written.txt")
        self.assertLessEqual(len(whatsnew.file_name("word " * 40, "tcp")), len("tcp-.txt") + 50)

    def test_the_problems_of_a_section_file(self):
        def problems(text):
            return whatsnew.parse_section_file(text)[2]
        self.assertEqual(problems("A Title\n~~~~~~~\n\nText.\n\nSub\n^^^\n\nMore.\n"), [])
        self.assertIn("does not underline its title with ~ on line 2", problems("A Title\n-------\n\nText.\n"))
        self.assertIn("underlines its title with fewer ~ than the title has characters",
                      problems("A Title\n~~~~\n\nText.\n"))
        self.assertIn("has no empty line after the underline", problems("A Title\n~~~~~~~\nText.\n"))
        self.assertIn("line 7 underlines a heading with ~; a subsection uses ^",
                      problems("A Title\n~~~~~~~\n\nText.\n\nSub\n~~~\n\nMore.\n"))

    def test_the_sections_of_the_guide_take_their_labels(self):
        preamble, sections = whatsnew.guide_sections(GUIDE_OLD)
        self.assertEqual([line for line in preamble if line][-1], "Release: |release|")
        self.assertEqual([t for t, _, _ in sections], ["TCP Algorithms", "Old Section"])
        self.assertEqual(sections[0][1], ["The algorithm is a module."])
        self.assertEqual(sections[1][2][0], ".. _mg:sec:old:")
        self.assertIn("An Old Subsection", sections[1][1])

    def test_a_version_heading_is_no_section_change(self):
        added, changed, removed = whatsnew.section_changes(GUIDE_OLD, GUIDE_NEW)
        self.assertEqual((added, changed), ([], []))
        self.assertEqual([t for t, _ in removed], ["TCP Algorithms"])

    def test_an_entry_change_is_matched_by_title(self):
        new = OLD_STYLE.replace("It is repaired.", "It is repaired well.").replace(
            "2. A second change", "2. A second change, renamed").replace(
            "Notable bug fixes", "3. A third change\n\n   New.\n\nNotable bug fixes")
        added, changed, removed = whatsnew.entry_changes(OLD_STYLE, new)
        self.assertEqual([e.title for e in added], ["A third change"])
        self.assertEqual(sorted((o.title, n.title) for o, n in changed),
                         [("A repair", "A repair"), ("A second change", "A second change, renamed")])
        self.assertEqual(removed, [])


class RepositoryTest(unittest.TestCase):
    """A test in a git repository of its own."""

    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Whatsnew Test")

    def tearDown(self):
        self.tempdir.cleanup()

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True, text=True)

    def commit(self, subject):
        self.git("add", "-A")
        self.git("commit", "-q", "-m", subject)

    def head(self):
        return self.git("rev-parse", "HEAD").stdout.strip()

    def run_tool(self, *args):
        return subprocess.run([sys.executable, str(TOOL), *args], cwd=self.root,
                              capture_output=True, text=True)


class CommandTest(RepositoryTest):
    def setUp(self):
        super().setUp()
        self.write("WHATSNEW", STUBBED)
        self.write("doc/whatsnew/README.md", "How to write an entry.\n")
        self.commit("fixture")
        self.base = self.head()

    def test_entry_files_pass(self):
        self.write("doc/whatsnew/fixes/tcp-repair.txt", "A repair\n\nIt is repaired.\n")
        self.commit("tcp: fix: a repair")
        for args in (["check"], ["check", "--range", f"{self.base}..HEAD"]):
            result = self.run_tool(*args)
            self.assertEqual(result.returncode, 0, result.stdout)
            self.assertIn("  ok", result.stdout)

    def test_an_entry_in_whatsnew_fails_with_the_file_to_write(self):
        self.write("WHATSNEW", STUBBED.replace(
            whatsnew.STUB, whatsnew.STUB + "\n\nNotable bug fixes are the following:\n\n"
            "1. A repair of TCP\n\n   It is repaired."))
        self.commit("tcp: fix: a repair")
        result = self.run_tool("check", "--range", f"{self.base}..HEAD")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn('adds the entry "A repair of TCP" to WHATSNEW; write it as '
                      "doc/whatsnew/fixes/tcp-repair-of-tcp.txt", result.stdout)
        result = self.run_tool("check")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("WHATSNEW:11 has the entry \"A repair of TCP\"", result.stdout)

    def test_a_change_of_a_released_section_passes(self):
        self.write("WHATSNEW", STUBBED.replace("It was repaired.", "It was repaired at last."))
        self.commit("doc: fix: a released entry")
        result = self.run_tool("check", "--range", f"{self.base}..HEAD")
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_bad_entry_files_fail(self):
        self.write("doc/whatsnew/fixes/Bad Name.txt", "A repair\n\nIt is repaired.\n")
        self.write("doc/whatsnew/fixes/tcp-same.txt", "A repair\n\nAgain.\n")
        self.write("doc/whatsnew/other/tcp-x.txt", "X\n\nY.\n")
        self.commit("tcp: fix: bad files")
        result = self.run_tool("check", "--range", f"{self.base}..HEAD")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("has a name that is not lower-case words", result.stdout)
        self.assertIn("has the same title as", result.stdout)
        self.assertIn("is not in incompatible/, compatible/ or fixes/", result.stdout)

    def test_a_section_in_the_guide_fails_with_the_file_to_write(self):
        self.write("doc/src/migration-guide/index.rst", GUIDE_NEW)
        self.commit("doc: add: the guide")
        base = self.head()
        self.write("doc/src/migration-guide/index.rst", GUIDE_NEW.replace(
            ".. _mg:sec:old:", "UDP Sockets\n-----------\n\nThe socket changed.\n\n.. _mg:sec:old:"))
        self.commit("udp: change: the socket")
        result = self.run_tool("check", "--range", f"{base}..HEAD")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn('adds the section "UDP Sockets" to doc/src/migration-guide/index.rst; write it as '
                      "doc/src/migration-guide/development/udp-sockets.rst", result.stdout)

    def test_a_version_heading_and_a_section_file_pass(self):
        self.write("doc/src/migration-guide/index.rst", GUIDE_NEW)
        self.commit("doc: add: the guide")
        base = self.head()
        self.write("doc/src/migration-guide/index.rst", GUIDE_NEW.replace(
            "INET-4.7.x (in development)\n---------------------------\n\n.. include-files:: development/*.rst",
            "INET-4.8.x (in development)\n---------------------------\n\n.. include-files:: development/*.rst"
            "\n\nINET-4.8\n--------\n\n.. include-files:: 4.8/*.rst"))
        self.write("doc/src/migration-guide/4.8/udp-sockets.rst", "UDP Sockets\n~~~~~~~~~~~\n\nIt changed.\n")
        self.commit("doc: change: release 4.8")
        for args in (["check", "--range", f"{base}..HEAD"], ["check"]):
            result = self.run_tool(*args)
            self.assertEqual(result.returncode, 0, result.stdout)

    def test_bad_section_files_fail(self):
        self.write("doc/src/migration-guide/index.rst", GUIDE_NEW)
        self.write("doc/src/migration-guide/development/Bad.rst", "Bad\n~~~\n\nText.\n")
        self.write("doc/src/migration-guide/other/udp-x.rst", "X\n~~~\n\nText.\n")
        self.write("doc/src/migration-guide/development/udp-y.rst", "Y Title\n-------\n\nText.\n")
        result = self.run_tool("check")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("has a name that is not lower-case words with hyphens and .rst", result.stdout)
        self.assertIn("is not in development/ or in a version folder", result.stdout)
        self.assertIn("does not underline its title with ~ on line 2", result.stdout)

    def test_a_guide_without_the_directive_fails(self):
        self.write("doc/src/migration-guide/index.rst", GUIDE_OLD)
        result = self.run_tool("check")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("does not include development/*.rst", result.stdout)

    def test_preview_numbers_the_entries(self):
        self.write("doc/whatsnew/incompatible/udp-b.txt", "Second\n\nB.\n")
        self.write("doc/whatsnew/incompatible/tcp-a.txt", "First\n\nA.\n")
        result = self.run_tool("preview")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "INET-4.7.x (in development)\n---------------------------\n\n"
                         "Notable backward incompatible changes are the following:\n\n"
                         "1. First\n\n   A.\n\n2. Second\n\n   B.\n")

    def test_release_writes_the_section_and_removes_the_files(self):
        self.write("doc/whatsnew/fixes/tcp-a.txt", "A repair\n\nIt is repaired.\n")
        self.commit("tcp: fix: a repair")
        result = self.run_tool("release", "INET-4.8 (March 2027) — feature release",
                               "--next", "INET-4.8.x (in development)")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse((self.root / "doc/whatsnew/fixes/tcp-a.txt").exists())
        text = (self.root / "WHATSNEW").read_text(encoding="utf-8")
        self.assertIn("INET-4.8.x (in development)\n---------------------------\n\n"
                      f"{whatsnew.STUB}\n\n"
                      "INET-4.8 (March 2027) — feature release\n"
                      "---------------------------------------\n\n"
                      "Notable bug fixes are the following:\n\n1. A repair\n\n   It is repaired.\n\n"
                      "INET-4.7 (July 2026)", text)
        self.assertNotIn("INET-4.7.x", text)
        self.assertEqual(self.run_tool("check").returncode, 0)

    def test_release_keeps_an_introduction(self):
        self.write("WHATSNEW", STUBBED.replace(whatsnew.STUB, whatsnew.STUB + "\n\nThis release adds X."))
        self.write("doc/whatsnew/fixes/tcp-a.txt", "A repair\n\nIt is repaired.\n")
        result = self.run_tool("release", "INET-4.8", "--next", "INET-4.8.x (in development)")
        self.assertEqual(result.returncode, 0, result.stderr)
        text = (self.root / "WHATSNEW").read_text(encoding="utf-8")
        self.assertIn("INET-4.8\n--------\n\nThis release adds X.\n\nNotable bug fixes", text)


FIXES = "Notable bug fixes are the following:\n\n1. A repair\n"


class RebaseTest(RepositoryTest):
    """A branch from before the move rebases onto a master after the move."""

    def setUp(self):
        super().setUp()
        self.write("WHATSNEW", OLD_STYLE)
        self.write("src/a.cc", "int a = 0;\n")
        self.commit("fixture")
        self.git("branch", "-M", "master")
        self.git("checkout", "-qb", "topic")

    def add_entry(self, before, text):
        whatsnew_path = self.root / "WHATSNEW"
        content = whatsnew_path.read_text(encoding="utf-8")
        self.assertEqual(content.count(before), 1)
        whatsnew_path.write_text(content.replace(before, text + before), encoding="utf-8")

    def move_master(self, source="int a = 0;\n"):
        self.git("checkout", "-q", "master")
        self.write("WHATSNEW", STUBBED)
        self.write("doc/whatsnew/README.md", "How to write an entry.\n")
        self.write("doc/whatsnew/incompatible/ieee80211-rate-control.txt",
                   "Rate control per receiver\n\nThe rate control adapts for each receiver.\n\n  getRate(receiver)\n")
        self.write("doc/whatsnew/incompatible/ieee80211-second-change.txt", "A second change\n\nIt changes.\n")
        self.write("doc/whatsnew/fixes/ipv4-repair.txt", "A repair\n\nIt is repaired.\n")
        self.write("doc/whatsnew/fixes/arp-other.txt", "Another repair\n\nIt is repaired too.\n")
        self.write("src/a.cc", source)
        self.commit("doc: change: the release notes are files")
        self.git("checkout", "-q", "topic")

    def entry(self, path):
        return (self.root / "doc/whatsnew" / path).read_text(encoding="utf-8")

    def in_rebase(self):
        return (self.root / ".git/rebase-merge").is_dir()

    def test_a_branch_rebases_and_its_entries_become_files(self):
        self.add_entry(FIXES, "3. A third change\n\n   It changes too.\n\n")
        self.write("src/b.cc", "int b = 0;\n")
        self.commit("tcp: fix: a third change")
        self.write("WHATSNEW", (self.root / "WHATSNEW").read_text().replace("It changes too.", "It changes, too."))
        self.commit("doc: change: the release note of the third change")
        self.write("WHATSNEW", (self.root / "WHATSNEW").read_text().replace("It is repaired.", "It is repaired well."))
        self.add_entry("INET-4.7 (July", "2. A fourth repair\n\n   It is new.\n\n")
        self.commit("udp: fix: a fourth repair")
        self.move_master()
        result = self.run_tool("rebase", "master")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse(self.in_rebase())
        self.assertIn("PASS: release note checks clean", result.stdout)
        self.assertEqual(self.git("rev-list", "--count", "master..topic").stdout.strip(), "3")
        self.assertEqual((self.root / "WHATSNEW").read_text(encoding="utf-8"), STUBBED)
        self.assertEqual(self.entry("incompatible/tcp-third-change.txt"), "A third change\n\nIt changes, too.\n")
        self.assertEqual(self.entry("fixes/ipv4-repair.txt"), "A repair\n\nIt is repaired well.\n")
        self.assertEqual(self.entry("fixes/udp-fourth-repair.txt"), "A fourth repair\n\nIt is new.\n")
        files = self.git("show", "--name-only", "--format=", "topic~2").stdout.split()
        self.assertEqual(sorted(files), ["doc/whatsnew/incompatible/tcp-third-change.txt", "src/b.cc"])

    def test_another_conflict_stops_and_continue_finishes(self):
        self.add_entry(FIXES, "3. A third change\n\n   It changes too.\n\n")
        self.write("src/a.cc", "int a = 1;\n")
        self.commit("tcp: fix: a third change")
        self.move_master(source="int a = 2;\n")
        result = self.run_tool("rebase", "master")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("also conflicts in src/a.cc", result.stdout)
        self.assertTrue(self.in_rebase())
        self.write("src/a.cc", "int a = 3;\n")
        self.git("add", "src/a.cc")
        result = self.run_tool("rebase", "--continue")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse(self.in_rebase())
        self.assertEqual(self.entry("incompatible/tcp-third-change.txt"), "A third change\n\nIt changes too.\n")

    def test_a_change_of_a_released_section_stops(self):
        self.add_entry(FIXES, "3. A third change\n\n   It changes too.\n\n")
        self.write("WHATSNEW", (self.root / "WHATSNEW").read_text().replace("It was repaired.", "It was repaired at last."))
        self.commit("tcp: fix: a third change")
        self.move_master()
        result = self.run_tool("rebase", "master")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("changes WHATSNEW outside the section of the version in development", result.stdout)
        self.git("rebase", "--abort")


if __name__ == "__main__":
    unittest.main()
