#!/usr/bin/env python3
"""The baseline gate: the pins of a commit series, and the record of each baseline commit that a
pin moves to. It is the mechanical half of TR-BASELINE-PIN, TR-BASELINE-PROVENANCE,
TR-BASELINE-ATTRIBUTION and TR-BASELINE-PARENT (doc/project/rule/testing.md), and of
PR-SPLIT-BASELINE for a baseline in another repository.

  TR-BASELINE-PIN          each commit pins the commit of each baseline repository with a full hash;
                           a pin moves only forward, over at most one commit that changes results;
                           the pinned commit is in the baseline repository
  TR-BASELINE-PROVENANCE   the baseline commit, which is the commit that changes results, has its
                           record: the computed "Moved results:" block, an analysis, and the
                           trailers INET-Subject (or INET-Commit) and Attribution
  TR-BASELINE-PARENT       each changed result file differs in a value
  TR-BASELINE-ATTRIBUTION  the form of the attribution holds: the size of the cause commit or of
                           the reduction, a reduction that applies, files that only come or only go
  PR-SPLIT-BASELINE        the pin moves in the commit that causes the move
  CR-OBL-BASELINE          the Change: trailer of the cause commit names the baseline that moves
  CR-OBL-INERT             a commit below the behavior level moves no pin over a result

A result is a .sca file of the statistics repository, or an image of the media repository that a
chart of an analysis file of the INET tree exports. A commit of a baseline repository that changes
no result, such as documentation media, needs no record, and a pin can move over it.

The gate reads git objects only. It never runs content of the series, so the pull-request
workflow can run it on untrusted content. Whether an analysis is true, and whether a reduction
gives the results that it claims, needs a run and a reviewer; this gate cannot say.

Usage, from the INET repository root:
  check-baselines.sh BASE..HEAD
  check-baselines.sh BASE..HEAD --baseline-repository statistical=statistics --baseline-repository chart=media

Without --baseline-repository, the gate gets the baseline commits from the repository that inet.opp
names at BASE. With it, the gate reads a local clone; that run does not prove that a commit is in
the repository of the project.

Exit status 0 = clean, 1 = violations, 2 = invalid usage or the gate cannot run.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import baseline_summary  # noqa: E402
from baseline_pins import PinError, git, is_full_hash, read_baselines  # noqa: E402

FAIL_LINES = 100
NOTE_LINES = 30
ANALYSIS_WORDS = 20
FORMS = ("commit", "forward", "reverse", "first-record", "removal", "environment")
REDUCTION_FORMS = ("forward", "reverse")
# a path whose content can change a result: model source, simulations and the Python of the project
SOURCE_PATH = re.compile(r"^(src|examples|showcases|tutorials|tests/networks|python)/")
NOT_SOURCE = re.compile(r"(^|/)doc/|\.(rst|md|txt|png|jpe?g|svg|gif|pdf)$")


class Report:
    def __init__(self):
        self.violations = 0
        self.quiet = True

    def section(self, title):
        print()
        print(f"== {title} ==")
        self.quiet = True

    def flag(self, text):
        print(f"  VIOLATION: {text}")
        self.violations += 1
        self.quiet = False

    def note(self, text):
        print(f"  note: {text}")
        self.quiet = False

    def end_section(self):
        if self.quiet:
            print("  ok")


class BaselineRepository:
    """Access to the commits of one baseline repository: a local clone, or a partial clone that the
    gate makes from the URL."""

    def __init__(self, kind, source, cache_folder):
        self.kind = kind
        self.source = source
        self.is_local = os.path.isdir(source)
        if self.is_local:
            self.folder = source
        else:
            self.folder = os.path.join(cache_folder, kind)
            os.makedirs(self.folder)
            git("init", "-q", "--bare", ".", cwd=self.folder)
            git("remote", "add", "origin", source, cwd=self.folder)
            # a clone without the file contents: the gate gets only the contents that it reads
            git("config", "remote.origin.promisor", "true", cwd=self.folder)
            git("config", "remote.origin.partialclonefilter", "blob:none", cwd=self.folder)

    def get(self, commits):
        """Returns the commits that the repository does not have. An error that is not about one
        commit, such as no network, is a RuntimeError."""
        if self.is_local:
            return [commit for commit in commits if not self.has(commit)]
        out = git("fetch", "-q", "--no-tags", "--filter=blob:none", "origin", *commits, cwd=self.folder, check=False)
        if out.returncode == 0:
            return []
        if git("ls-remote", "--exit-code", "origin", "HEAD", cwd=self.folder, check=False).returncode != 0:
            raise RuntimeError(f"cannot read the {self.kind} baseline repository {self.source}:\n{out.stderr.strip()}")
        return [commit for commit in commits
                if git("fetch", "-q", "--no-tags", "--filter=blob:none", "origin", commit, cwd=self.folder, check=False).returncode != 0]

    def has(self, commit):
        return git("cat-file", "-e", f"{commit}^{{commit}}", cwd=self.folder, check=False).returncode == 0

    def parents(self, commit):
        return git("rev-list", "--parents", "-n", "1", commit, cwd=self.folder).stdout.split()[1:]

    def is_ancestor(self, ancestor, commit):
        return git("merge-base", "--is-ancestor", ancestor, commit, cwd=self.folder, check=False).returncode == 0

    def chain(self, old, new):
        """The commits above the old pin up to the new pin, the oldest first."""
        return git("rev-list", "--reverse", f"{old}..{new}", cwd=self.folder).stdout.split()

    def has_merge(self, old, new):
        return bool(git("rev-list", "--merges", f"{old}..{new}", cwd=self.folder).stdout.strip())

    def changed_paths(self, commit):
        return [path for path in git("diff-tree", "-r", "--no-commit-id", "--no-renames", "--name-only", "-z", commit,
                                     cwd=self.folder).stdout.split("\0") if path]

    def message(self, commit):
        return git("log", "-1", "--format=%B", commit, cwd=self.folder).stdout

    def subject(self, commit):
        return git("log", "-1", "--format=%s", commit, cwd=self.folder).stdout.strip()

    def trailer_lines(self, commit):
        """The lines of the trailer block of the commit message."""
        return [line for line in git("log", "-1", "--format=%(trailers:only)", commit, cwd=self.folder).stdout.splitlines() if line.strip()]

    def trailers(self, commit):
        """{key: [values]} of the trailer block of the commit message."""
        out = git("log", "-1", "--format=%(trailers:only,unfold)", commit, cwd=self.folder).stdout
        trailers = {}
        for line in out.splitlines():
            key, separator, value = line.partition(":")
            if separator:
                trailers.setdefault(key.strip(), []).append(value.strip())
        return trailers

    def blob(self, commit, path):
        """The object name of a file at a commit, or None."""
        out = git("rev-parse", "--verify", "--quiet", f"{commit}:{path}", cwd=self.folder, check=False)
        return out.stdout.strip() or None

    def fetch_contents(self, old, new):
        """Gets the contents of the files that differ between two commits, in one request."""
        if self.is_local:
            return
        raw = git("diff-tree", "-r", "--no-renames", "--raw", "--no-abbrev", old, new, cwd=self.folder).stdout
        blobs = sorted({name for line in raw.splitlines() for name in line.split()[2:4] if set(name) != {"0"}})
        if blobs:
            subprocess.run(["git", "-c", "fetch.negotiationAlgorithm=noop", "fetch", "-q", "--no-tags", "--no-write-fetch-head",
                            "--filter=blob:none", "--stdin", "origin"], cwd=self.folder, input="\n".join(blobs) + "\n",
                           capture_output=True, text=True)


def short(commit):
    return commit[:9]


def subject_of(commit):
    return git("log", "-1", "--format=%s", commit).stdout.strip()


def source_lines(commit):
    """The changed lines of the commit in the files that can change a result."""
    total = 0
    for line in git("show", "--numstat", "--format=", "--no-renames", commit).stdout.splitlines():
        fields = line.split("\t")
        if len(fields) == 3 and fields[0] != "-" and SOURCE_PATH.search(fields[2]) and not NOT_SOURCE.search(fields[2]):
            total += int(fields[0]) + int(fields[1])
    return total


def patch_lines(text):
    return sum(1 for line in text.splitlines() if line[:1] in "+-" and not line.startswith(("+++", "---")))


def patch_applies(text, tree):
    """True when the patch applies to the tree of a commit. Nothing is checked out and nothing runs."""
    with tempfile.TemporaryDirectory() as folder:
        patch = os.path.join(folder, "reduction.patch")
        with open(patch, "w", encoding="utf-8") as f:
            f.write(text)
        env = dict(os.environ, GIT_INDEX_FILE=os.path.join(folder, "index"))
        if subprocess.run(["git", "read-tree", tree], env=env, capture_output=True).returncode != 0:
            return False
        return subprocess.run(["git", "apply", "--cached", "--check", patch], env=env, capture_output=True).returncode == 0


def change_fields(commit):
    """The fields of the Change: trailer of a commit; an empty list when the commit has no trailer."""
    value = git("log", "-1", "--format=%(trailers:key=Change,valueonly)", commit).stdout.strip()
    return [field.strip() for field in value.split("|")] if value else []


def obligations_of(commit):
    """The obligations field of the Change: trailer of a commit, or None when the commit has no trailer."""
    fields = change_fields(commit)
    return fields[2].split() if len(fields) >= 3 else None


def analysis_words(message, trailer_lines=()):
    """The number of words of the prose of a baseline commit message: the body without the block
    of the moved results and without the lines of the trailer block."""
    words = 0
    in_block = False
    trailer_lines = {line.strip() for line in trailer_lines}
    for line in message.splitlines()[1:]:
        if line.strip() == baseline_summary.HEADING:
            in_block = True
        elif not line.strip():
            in_block = False
        elif not in_block and line.strip() not in trailer_lines:
            words += len(line.split())
    return words


def check_pins(report, commits, base, baselines):
    """TR-BASELINE-PIN on the INET side. Returns [(commit, parent, kind, old ref, new ref)] for each
    pin that moves, and the baseline entries of the base."""
    report.section("TR-BASELINE-PIN: each commit pins the commit of each baseline repository")
    moves = []
    previous, parent = baselines, base
    for commit in commits:
        try:
            current = read_baselines(commit)
        except PinError as error:
            report.flag(f"{short(commit)} {error}")
            parent = commit
            continue
        for kind in sorted(set(previous) | set(current)):
            old, new = previous.get(kind), current.get(kind)
            if new is None:
                report.flag(f"{short(commit)} removes the baseline entry of the {kind} tests")
                continue
            if old is not None and (new.get("repository"), new.get("folder")) != (old.get("repository"), old.get("folder")):
                report.flag(f"{short(commit)} changes the repository or the folder of the {kind} baseline; a series does not move a baseline repository")
            new_ref, old_ref = new.get("ref"), (old or {}).get("ref")
            if not is_full_hash(new_ref):
                if is_full_hash(old_ref) or new_ref is not None:
                    report.flag(f"{short(commit)} the pin of the {kind} baseline is {new_ref!r}, not a full commit hash")
            elif not is_full_hash(old_ref):
                report.note(f"{short(commit)} adds the first pin of the {kind} baseline, {short(new_ref)}; the gate does not examine that commit")
            elif new_ref != old_ref:
                moves.append((commit, parent, kind, old_ref, new_ref))
        previous, parent = current, commit
    report.end_section()
    return moves


def check_record(report, repository, kind, commit, parent, old, new, is_result):
    """TR-BASELINE-PROVENANCE, TR-BASELINE-PARENT and the form of TR-BASELINE-ATTRIBUTION for one
    baseline commit. Returns (form, text of the reduction, object name of the reduction, cause): the
    cause is the value of INET-Commit, or None when INET-Subject names the cause."""
    where = f"{short(commit)} -> {kind} {short(new)}"
    files = baseline_summary.changed_files(repository.folder, old, new)
    results = [(status, path) for status, path in files if is_result(path)]
    reductions = [(status, path) for status, path in files if path.startswith(baseline_summary.ATTRIBUTION_FOLDER)]
    trailers = repository.trailers(new)
    message = repository.message(new)

    # the link back to the cause
    subjects, causes = trailers.get("INET-Subject", []), trailers.get("INET-Commit", [])
    if len(subjects) + len(causes) != 1:
        report.flag(f"{where} needs one trailer that names the cause: INET-Subject with the subject of the INET commit, or INET-Commit for a cause on the target branch")
    elif subjects and subjects[0] != subject_of(commit):
        report.flag(f"{where} INET-Subject is not the subject of {short(commit)}: '{subjects[0]}'")

    # the attribution
    values = trailers.get("Attribution", [])
    form, reduction_path = None, None
    if len(values) != 1:
        report.flag(f"{where} needs one Attribution trailer: {', '.join(FORMS)}")
    else:
        fields = values[0].split()
        if fields and fields[0] in FORMS and len(fields) == (2 if fields[0] in REDUCTION_FORMS else 1):
            form = fields[0]
            reduction_path = fields[1] if form in REDUCTION_FORMS else None
        else:
            report.flag(f"{where} Attribution is '{values[0]}'; the forms are commit, forward FILE, reverse FILE, first-record, removal, environment")
    reduction_text, reduction_blob = None, None
    if reduction_path:
        reduction_blob = repository.blob(new, reduction_path)
        if not reduction_path.startswith(baseline_summary.ATTRIBUTION_FOLDER) or reduction_blob is None:
            report.flag(f"{where} names the reduction {reduction_path}, which is not a file in {baseline_summary.ATTRIBUTION_FOLDER} of the baseline commit")
            reduction_blob = None
        else:
            reduction_text = baseline_summary.read_file(repository.folder, new, reduction_path)
    for status, path in reductions:
        if path != reduction_path or status != "A":
            report.flag(f"{where} changes {path}; a baseline commit adds only the reduction that its Attribution names to the folder {baseline_summary.ATTRIBUTION_FOLDER}")

    # the summary and the analysis
    repository.fetch_contents(old, new)
    moved = baseline_summary.analyze(repository.folder, old, new, is_result)
    expected = baseline_summary.block(moved)
    found = baseline_summary.find_block(message)
    if found is None:
        report.flag(f"{where} has no '{baseline_summary.HEADING}' block; the block is:")
        print("\n".join("      " + line for line in expected))
    elif [line.strip() for line in found] != [line.strip() for line in expected]:
        report.flag(f"{where} has a '{baseline_summary.HEADING}' block that is not the computed block; the block is:")
        print("\n".join("      " + line for line in expected))
    groups = baseline_summary.count_groups(moved)
    if groups > 1:
        report.note(f"{where} moves its tests in {groups} ways (the groups of the block), one explanation each? If the ways have different causes, the INET commit holds more than one change (PR-SPLIT-ONE-CHANGE)")
    words = analysis_words(message, repository.trailer_lines(new))
    if words < ANALYSIS_WORDS:
        report.flag(f"{where} has no analysis: {words} words of prose; say why the change causes each group of the moved results")

    # only results that move
    for status, path, changed, added, removed in moved["tests"]:
        if status == "M" and not changed and not added and not removed:
            report.flag(f"{where} changes {path} and no value in it; a baseline changes only for a test that moves (TR-BASELINE-PARENT)")
    statuses = {status for status, _ in results}
    if form == "first-record" and statuses - {"A"}:
        report.flag(f"{where} has the form first-record and changes or removes {sum(1 for s, _ in results if s != 'A')} file(s) that exist")
    if form == "removal" and statuses - {"D"}:
        report.flag(f"{where} has the form removal and adds or changes {sum(1 for s, _ in results if s != 'D')} file(s)")
    return form, reduction_text, reduction_blob, causes[0] if len(causes) == 1 and not subjects else None


def check_attribution(report, base, commit, parent, kind, new, form, reduction_text, cause):
    """TR-BASELINE-ATTRIBUTION, PR-SPLIT-BASELINE, CR-OBL-BASELINE and CR-OBL-INERT against the cause commit."""
    where = f"{short(commit)} -> {kind} {short(new)}"
    lines = source_lines(commit)
    if form == "commit":
        if lines == 0:
            report.flag(f"{where} has the form commit, and {short(commit)} changes no source; the pin moves in the commit that causes the move (PR-SPLIT-BASELINE), or the form is environment")
        elif lines > FAIL_LINES:
            report.flag(f"{where} has the form commit, and {short(commit)} changes {lines} source lines, above {FAIL_LINES}; make the commit that moves the results small, or give a forward or reverse reduction")
        elif lines > NOTE_LINES:
            report.note(f"{where} has the form commit, and {short(commit)} changes {lines} source lines, above {NOTE_LINES}: can a smaller change show the same move?")
    elif form == "environment":
        if lines:
            report.flag(f"{where} has the form environment, and {short(commit)} changes {lines} source lines; an environment change has no source cause")
    elif form in REDUCTION_FORMS and reduction_text is not None:
        size = patch_lines(reduction_text)
        if size > FAIL_LINES:
            report.flag(f"{where} has a reduction of {size} changed lines, above {FAIL_LINES}")
        elif size > NOTE_LINES:
            report.note(f"{where} has a reduction of {size} changed lines, above {NOTE_LINES}: can a smaller change show the same move?")
        tree, name = (parent, "the parent of the cause commit") if form == "forward" else (commit, "the cause commit")
        if not patch_applies(reduction_text, tree):
            report.flag(f"{where} has a {form} reduction that does not apply to {name}, {short(tree)}")
    if cause is not None:
        if git("merge-base", "--is-ancestor", cause, base, check=False).returncode != 0:
            report.flag(f"{where} names the cause INET-Commit {cause[:9]}, which is not a commit of the target branch; a cause in the series has the trailer INET-Subject")
        if lines:
            report.flag(f"{where} names its cause with INET-Commit, and {short(commit)} changes {lines} source lines; a commit that changes source is the cause of its own pin move")
    obligations = obligations_of(commit)
    if obligations is not None and "?" not in obligations and kind not in obligations:
        report.flag(f"{short(commit)} moves the pin of the {kind} baseline over a result, and its Change: trailer does not name '{kind}' (CR-OBL-BASELINE)")
    fields = change_fields(commit)
    if len(fields) >= 2 and "behavior" not in fields[1]:
        report.flag(f"{short(commit)} claims '{fields[1]}' and moves the pin of the {kind} baseline over a result (CR-OBL-INERT)")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("range", metavar="BASE..HEAD")
    parser.add_argument("--baseline-repository", action="append", default=[], metavar="KIND=PATH",
                        help="read the baseline commits of a test kind from a local clone, or from another URL")
    args = parser.parse_args(argv)
    if ".." not in args.range or "..." in args.range or git("rev-parse", args.range, check=False).returncode != 0:
        print(f"error: '{args.range}' is not a valid commit range", file=sys.stderr)
        return 2
    sources = {}
    for value in args.baseline_repository:
        kind, separator, source = value.partition("=")
        if not separator or not source:
            print(f"error: --baseline-repository needs KIND=PATH, not '{value}'", file=sys.stderr)
            return 2
        sources[kind] = source

    base = git("rev-parse", args.range.split("..")[0] or "HEAD").stdout.strip()
    commits = git("log", "--format=%H", "--reverse", "--first-parent", args.range).stdout.split()
    report = Report()
    print(f"== the series: {args.range} ==")
    print(f"  {len(commits)} commit(s)")
    try:
        baselines = read_baselines(base)
    except PinError as error:
        print(f"error: the base of the series: {error}", file=sys.stderr)
        return 2
    moves = check_pins(report, commits, base, baselines)

    with tempfile.TemporaryDirectory() as cache_folder:
        repositories = {}
        try:
            report.section("TR-BASELINE-PIN: a pin moves forward, over at most one commit with results, in the baseline repository")
            print(f"  {len(moves)} pin move(s)")
            usable = []
            for kind in sorted({move[2] for move in moves}):
                entry = baselines.get(kind) or read_baselines(next(move[0] for move in moves if move[2] == kind))[kind]
                source = sources.get(kind) or entry["repository"]
                if not os.path.isdir(source) and "://" not in source and not source.startswith("git@"):
                    source = f"https://github.com/{source}.git"
                repository = repositories[kind] = BaselineRepository(kind, source, cache_folder)
                if repository.is_local:
                    report.note(f"the {kind} baseline commits are read from the local clone {source}; this run does not prove that they are in the repository of the project")
                pins = sorted({ref for move in moves if move[2] == kind for ref in move[3:5]})
                missing = repository.get(pins)
                for commit, parent, move_kind, old, new in moves:
                    if move_kind != kind:
                        continue
                    if new in missing:
                        report.flag(f"{short(commit)} pins {kind} {short(new)}, which is not in the baseline repository; push the baseline branch, or open a pull request in the baseline repository")
                    elif old in missing:
                        report.flag(f"{short(commit)} moves the {kind} pin from {short(old)}, which is not in the baseline repository")
                    elif not repository.is_ancestor(old, new):
                        report.flag(f"{short(commit)} pins {kind} {short(new)}, which does not stand on the pin of the parent, {short(old)}; rebase the baseline branch and write the new pins")
                    elif repository.has_merge(old, new):
                        report.flag(f"{short(commit)} moves the {kind} pin over a merge commit; the history of a baseline is a line")
                    else:
                        images = ()
                        if kind == "chart":
                            images = baseline_summary.chart_images_at(commit, entry) | baseline_summary.chart_images_at(parent, entry)
                        is_result = baseline_summary.result_filter(kind, images)
                        chain = repository.chain(old, new)
                        with_results = [c for c in chain if any(is_result(path) for path in repository.changed_paths(c))]
                        if len(with_results) > 1:
                            report.flag(f"{short(commit)} moves the {kind} pin over {len(with_results)} baseline commits that change results; one cause commit has one baseline commit")
                        elif with_results:
                            usable.append((commit, parent, kind, repository.parents(with_results[0])[0], with_results[0], is_result))
                            if len(chain) > 1:
                                print(f"  {short(commit)} moves the {kind} pin over {len(chain)} commits; {short(with_results[0])} changes results and has the record")
                        else:
                            print(f"  {short(commit)} moves the {kind} pin over {len(chain)} commit(s) that change no result; they need no record")
            report.end_section()

            report.section("TR-BASELINE-PROVENANCE and TR-BASELINE-PARENT: the record of each baseline commit")
            records = []
            for commit, parent, kind, old, new, is_result in usable:
                records.append((commit, parent, kind, new, *check_record(report, repositories[kind], kind, commit, parent, old, new, is_result)))
            report.end_section()

            report.section("TR-BASELINE-ATTRIBUTION: a small change that causes the move")
            for commit, parent, kind, new, form, reduction_text, reduction_blob, cause in records:
                check_attribution(report, base, commit, parent, kind, new, form, reduction_text, cause)
            by_commit = {}
            for commit, _, kind, new, form, _, reduction_blob, _ in records:
                by_commit.setdefault(commit, []).append((kind, form, reduction_blob))
            for commit, entries in by_commit.items():
                if len({(form, blob) for _, form, blob in entries}) > 1:
                    report.flag(f"{short(commit)} moves {len(entries)} pins, and their baseline commits do not have the same attribution; one cause has one attribution, with the same reduction file")
            report.end_section()

            if usable:
                print()
                print("== the baseline commits (the record, for the reviewer) ==")
                for commit, parent, kind, old, new, _ in usable:
                    print(f"  {short(commit)} {subject_of(commit)}")
                    print(f"    {kind} {short(old)} -> {short(new)}")
                    print("\n".join("      " + line for line in repositories[kind].message(new).rstrip().splitlines()))
        except RuntimeError as error:
            print(f"error: {error}", file=sys.stderr)
            return 2

    print()
    if report.violations:
        print("FAIL: see doc/project/rule/testing.md (TR-BASELINE-*) and doc/project/guide/change-a-baseline.md.")
        return 1
    print("PASS: the pins and the baseline records are clean. Whether an analysis is true, and whether a")
    print("      reduction gives the results that it claims, needs a run and a reviewer.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
