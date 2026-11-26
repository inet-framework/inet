#!/usr/bin/env python3
"""The baseline gate: the pins of a commit series, and the record of each baseline commit that a
pin moves to. It is the mechanical half of TR-BASELINE-PIN, TR-BASELINE-PROVENANCE,
TR-BASELINE-ATTRIBUTION and TR-BASELINE-PARENT (doc/project/rule/testing.md), and of
PR-SPLIT-BASELINE for a baseline in another repository.

  TR-BASELINE-PIN          each commit pins the commit of each baseline repository with a full hash,
                           in the ref of inet.opp or in a pin trailer of its message (the trailer
                           form); a pin moves only forward, over at most one commit that changes
                           results; the pinned commit is in the baseline repository
  TR-BASELINE-PROVENANCE   the baseline commit, which is the commit that changes results, has its
                           record: the computed "Baseline changes:" list with a reason on each
                           line, and the trailers INET-Subject (or INET-Commit) and Attribution
  TR-BASELINE-PARENT       each changed result file differs in a value
  TR-BASELINE-ATTRIBUTION  the form of the attribution holds: the size of the cause commit or of
                           the reduction, a reduction that applies, files that only come or only go
  PR-SPLIT-BASELINE        the pin moves in the commit that causes the move
  CR-OBL-BASELINE          the Change: trailer of the cause commit names the baseline that moves
  CR-OBL-INERT             a commit below the behavior level moves no pin over a result
  PR-REQ-BASELINES         the description of the pull request names each step that changes a
                           baseline, with a summary of its baseline changes

The same rules hold for the fingerprints in tests/fingerprint/store.json of the INET tree. The INET
commit that changes the store holds the "Baseline changes:" list of its fingerprint changes, with a
reason on each line. When it changes fingerprint values, the code that moves them is small: the
commit itself, or a reduction patch in its message after a line "Reduction:", with the trailer
"Attribution: forward" or "Attribution: reverse". A commit that only adds, removes or renames runs
or ingredient sets, or that changes no source line, has no size limit.

In the trailer form, the first pin of a kind in the series moves from the base of its baseline branch:
the merge base of the pin and the master branch of the baseline repository. Whether the baseline
branch can land on that master branch with no common file is the status "Baseline merge" of
baseline_masters.py check, because it changes when another pull request lands.

A result is a .sca file of the statistics repository, or an image of the media repository that a
chart of an analysis file of the INET tree exports. A commit of a baseline repository that changes
no result, such as documentation media, needs no record, and a pin can move over it.

The gate reads git objects only. It never runs content of the series, so the pull-request
workflow can run it on untrusted content. Whether a reason is true, and whether a reduction gives
the results that it claims, needs a run and a reviewer; this gate cannot say.

Usage, from the INET repository root:
  check-baselines.sh BASE..HEAD
  check-baselines.sh BASE..HEAD --baseline-repository statistical=statistics --baseline-repository chart=media
  check-baselines.sh BASE..HEAD --description-file FILE    also check the description of the pull request
  check-baselines.sh BASE..HEAD --pr NUMBER                the same, with the description from GitHub

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
import fingerprint_moves  # noqa: E402
from baseline_pins import PinError, git, is_full_hash, is_trailer_form, master_of, pin_trailers, read_baselines  # noqa: E402

FAIL_LINES = 100
NOTE_LINES = 30
FORMS = ("commit", "forward", "reverse", "first-record", "removal", "environment")
FINGERPRINT_FORMS = ("commit", "forward", "reverse")
STORE = "tests/fingerprint/store.json"
REDUCTION_HEADING = "Reduction:"
SECTION = "baseline changes"
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
            self._master = None
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

    def master(self):
        """The commit of the master branch of the baseline repository."""
        if self.is_local:
            return master_of(self.folder)
        if self._master is None:
            git("fetch", "-q", "--no-tags", "--filter=blob:none", "origin", "+refs/heads/master:refs/remotes/origin/master", cwd=self.folder)
            self._master = git("rev-parse", "refs/remotes/origin/master", cwd=self.folder).stdout.strip()
        return self._master

    def branch_base(self, pin):
        """The base of the baseline branch of a pin: its merge base with the master branch, or None."""
        return git("merge-base", pin, self.master(), cwd=self.folder, check=False).stdout.strip() or None

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
        # --recount: a patch in a commit message can lose an empty line at its end
        return subprocess.run(["git", "apply", "--cached", "--check", "--recount", patch], env=env, capture_output=True).returncode == 0


def change_fields(commit):
    """The fields of the Change: trailer of a commit; an empty list when the commit has no trailer."""
    value = git("log", "-1", "--format=%(trailers:key=Change,valueonly)", commit).stdout.strip()
    return [field.strip() for field in value.split("|")] if value else []


def obligations_of(commit):
    """The obligations field of the Change: trailer of a commit, or None when the commit has no trailer."""
    fields = change_fields(commit)
    return fields[2].split() if len(fields) >= 3 else None


def check_list(report, where, message, expected):
    """TR-BASELINE-PROVENANCE: the message holds the computed "Baseline changes:" list, with a reason on each line."""
    problems = baseline_summary.check_block(message, expected)
    for problem in problems:
        report.flag(f"{where} has {problem}" if problem.startswith(("no '", "a '")) else f"{where}: {problem}")
    if problems and any(problem.startswith(("no '", "a '")) for problem in problems):
        print(f"      the list is (write the reason of each line after the dash):")
        print("\n".join("        " + line for line in [baseline_summary.HEADING] + [f"  {line}{baseline_summary.SEPARATOR}" for line in expected]))
    changed = sum(1 for line in expected if " changed" in line.split("(")[0])
    if changed > 1:
        report.note(f"{where} has {changed} lines of changed entries: do their reasons name one cause? If not, the commit holds more than one change (PR-SPLIT-ONE-CHANGE)")


def message_of(commit):
    return git("log", "-1", "--format=%B", commit).stdout


def trailers_of(commit):
    """{key: [values]} and the lines of the trailer block of an INET commit."""
    out = git("log", "-1", "--format=%(trailers:only,unfold)", commit).stdout
    trailers, lines = {}, [line for line in out.splitlines() if line.strip()]
    for line in lines:
        key, separator, value = line.partition(":")
        if separator:
            trailers.setdefault(key.strip(), []).append(value.strip())
    return trailers, lines


def reduction_in_message(message, trailer_lines):
    """The patch after the line "Reduction:" of a commit message, up to its trailers, or None."""
    lines = message.splitlines()
    starts = [index for index, line in enumerate(lines) if line.strip() == REDUCTION_HEADING]
    if not starts:
        return None
    end, trailers = len(lines), {line.strip() for line in trailer_lines}
    while end > starts[0] + 1 and (not lines[end - 1].strip() or lines[end - 1].strip() in trailers):
        end -= 1
    text = "\n".join(lines[starts[0] + 1:end])
    return text + "\n" if text.strip() else None


def store_at(commit):
    out = git("show", f"{commit}:{STORE}", check=False)
    return fingerprint_moves.parse(out.stdout if out.returncode == 0 else "")


def fingerprint_changes(commit):
    """[(kind, line)] of the fingerprint changes of a commit, against its first parent."""
    parent = git("rev-parse", "--verify", "--quiet", f"{commit}^", check=False).stdout.strip()
    if parent and git("diff", "--quiet", parent, commit, "--", STORE, check=False).returncode == 0:
        return []
    return fingerprint_moves.categories(store_at(parent) if parent else {}, store_at(commit))


def same_patch(a, b):
    return [line.rstrip() for line in a.strip().splitlines()] == [line.rstrip() for line in b.strip().splitlines()]


def check_fingerprint_attribution(report, commit, pinned):
    """TR-BASELINE-ATTRIBUTION for a commit that changes fingerprint values. pinned is [(form,
    reduction text)] of the baseline commits that the same commit pins: one cause has one attribution."""
    where = f"{short(commit)} fingerprints"
    parent = git("rev-parse", f"{commit}^").stdout.strip()
    lines = source_lines(commit)
    trailers, trailer_lines = trailers_of(commit)
    values = trailers.get("Attribution", [])
    reduction = reduction_in_message(message_of(commit), trailer_lines)
    pinned_forms = {form for form, _ in pinned if form in FINGERPRINT_FORMS}
    if len(values) > 1:
        report.flag(f"{where} has {len(values)} Attribution trailers; one cause has one attribution")
        return
    if values:
        fields = values[0].split()
        if len(fields) != 1 or fields[0] not in FINGERPRINT_FORMS:
            report.flag(f"{where} has the Attribution '{values[0]}'; for fingerprints the forms are commit, forward and reverse, with the patch after a line '{REDUCTION_HEADING}'")
            return
        form = fields[0]
        if pinned_forms - {form}:
            report.flag(f"{where} has the Attribution {form}, and its baseline commits have {', '.join(sorted(pinned_forms))}; one cause has one attribution")
    else:
        form = pinned_forms.pop() if len(pinned_forms) == 1 else "commit"
    if form in REDUCTION_FORMS:
        pinned_texts = [text for pinned_form, text in pinned if pinned_form == form and text]
        if reduction is None and pinned_texts:
            reduction = pinned_texts[0]
        elif reduction is not None and any(not same_patch(reduction, text) for text in pinned_texts):
            report.flag(f"{where} has a reduction in its message that differs from the reduction file of its baseline commits; one cause has one reduction")
    if lines == 0:
        return  # a tool or version change: the reasons of the list name it
    if form == "commit":
        if reduction is not None:
            report.flag(f"{where} has a '{REDUCTION_HEADING}' patch and no Attribution forward or reverse")
        elif lines > FAIL_LINES:
            report.flag(f"{where}: {short(commit)} changes {lines} source lines, above {FAIL_LINES}, and moves fingerprint values; make the commit that moves them small, or give a forward or reverse reduction after a line '{REDUCTION_HEADING}'")
        elif lines > NOTE_LINES:
            report.note(f"{where}: {short(commit)} changes {lines} source lines, above {NOTE_LINES}, and moves fingerprint values: can a smaller change move the same fingerprints?")
        return
    if reduction is None:
        report.flag(f"{where} has the Attribution {form} and no patch after a line '{REDUCTION_HEADING}'")
        return
    size = patch_lines(reduction)
    if size > FAIL_LINES:
        report.flag(f"{where} has a reduction of {size} changed lines, above {FAIL_LINES}")
    elif size > NOTE_LINES:
        report.note(f"{where} has a reduction of {size} changed lines, above {NOTE_LINES}: can a smaller change move the same fingerprints?")
    tree, name = (parent, "the parent of the commit") if form == "forward" else (commit, "the commit")
    if not patch_applies(reduction, tree):
        report.flag(f"{where} has a {form} reduction that does not apply to {name}, {short(tree)}")


def check_fingerprints(report, commits, records):
    """TR-BASELINE-PROVENANCE, TR-BASELINE-ATTRIBUTION and CR-OBL-BASELINE for the fingerprint store.
    Returns {commit: [(kind, line)]} of the commits that change fingerprint entries."""
    report.section("TR-BASELINE-PROVENANCE and TR-BASELINE-ATTRIBUTION: the fingerprint changes of each commit")
    pinned = {}
    for commit, _, _, _, form, reduction_text, _, _ in records:
        pinned.setdefault(commit, []).append((form, reduction_text))
    steps = {}
    for commit in commits:
        changes = fingerprint_changes(commit)
        if not changes:
            continue
        steps[commit] = changes
        check_list(report, f"{short(commit)} fingerprints", message_of(commit), [line for _, line in changes])
        obligations = obligations_of(commit)
        if obligations is not None and "?" not in obligations and "fingerprint" not in obligations:
            report.flag(f"{short(commit)} changes the fingerprint store, and its Change: trailer does not name 'fingerprint' (CR-OBL-BASELINE)")
        if any(kind == "changed" for kind, _ in changes):
            check_fingerprint_attribution(report, commit, pinned.get(commit, []))
    print(f"  {len(steps)} commit(s) change fingerprint entries")
    report.end_section()
    return steps


def description_section(description):
    """The text of the section "Baseline changes" of a description, or None. The section ends at the
    next Markdown heading. An HTML comment, such as the text of the template, is not part of it."""
    lines = re.sub(r"<!--.*?-->", "", description, flags=re.S).splitlines()
    for index, line in enumerate(lines):
        if line.strip().lstrip("#").strip().strip("*").strip().rstrip(":").strip().lower() == SECTION:
            end = index + 1
            while end < len(lines) and not lines[end].lstrip().startswith("#"):
                end += 1
            return "\n".join(lines[index + 1:end])
    return None


def check_description(report, description, steps):
    """PR-REQ-BASELINES: the description names each step, with a summary of its baseline changes."""
    report.section("PR-REQ-BASELINES: the description summarizes the baseline changes of each step")
    if not steps:
        print("  no commit changes a baseline")
    elif description is None:
        report.note("no description given; the summary of the baseline changes in the pull request is not checked")
    else:
        section = description_section(description)
        if section is None:
            report.flag(f"the description has no section 'Baseline changes', and {len(steps)} commit(s) change baselines; name each one with a summary of its baseline changes")
        else:
            subjects = [subject_of(step) for step in steps]
            for subject in subjects:
                start = section.find(subject)
                if start < 0:
                    report.flag(f"the section 'Baseline changes' of the description does not name the commit '{subject}'")
                    continue
                rest = section[start + len(subject):]
                ends = [rest.find(other) for other in subjects if other != subject and rest.find(other) >= 0]
                summary = rest[:min(ends)] if ends else rest
                if not summary.strip(" \t\n:-—*`.,;()"):
                    report.flag(f"the section 'Baseline changes' of the description names the commit '{subject}' with no summary")
    report.end_section()


def read_description(args):
    """The description of the pull request, from a file or from GitHub, or None."""
    if args.description_file:
        with open(args.description_file, encoding="utf-8") as f:
            return f.read()
    if args.pr:
        out = subprocess.run(["gh", "pr", "view", str(args.pr), "--json", "body", "--jq", ".body"], capture_output=True, text=True)
        if out.returncode != 0:
            raise RuntimeError(f"cannot read the description of #{args.pr}:\n{out.stderr.strip()}")
        return out.stdout
    return None


def check_trailer_pin(report, commit, kind, old, new):
    """TR-BASELINE-PIN for one commit of the trailer form. Returns the pin of its trailer, or None."""
    if old is not None and not is_trailer_form(old):
        report.note(f"{short(commit)} changes the {kind} baseline to the trailer form, with the trailer {new['trailer']}")
    elif old is not None and old.get("trailer") != new["trailer"]:
        report.flag(f"{short(commit)} changes the trailer key of the {kind} baseline from {old.get('trailer')!r} to {new['trailer']!r}; a series does not change it")
    values = pin_trailers(commit, new["trailer"])
    if len(values) > 1:
        report.flag(f"{short(commit)} has {len(values)} trailers {new['trailer']}; a commit has one pin of the {kind} baseline")
    elif values and not is_full_hash(values[0]):
        report.flag(f"{short(commit)} the trailer {new['trailer']} is {values[0]!r}, not a full commit hash")
    elif values:
        return values[0]
    return None


def check_pins(report, commits, base, baselines):
    """TR-BASELINE-PIN on the INET side. Returns [(commit, parent, kind, old pin, new pin)] for each
    pin that moves. For the trailer form, the old pin of the first move of a kind is None: the base of
    the baseline branch, which the baseline repository gives."""
    report.section("TR-BASELINE-PIN: each commit pins the commit of each baseline repository")
    moves = []
    last = {}
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
            if new.get("ref") and new.get("trailer"):
                report.flag(f"{short(commit)} the {kind} baseline entry has a ref and a trailer; it has one of them")
                continue
            if is_trailer_form(new):
                pin = check_trailer_pin(report, commit, kind, old, new)
                if pin is not None:
                    moves.append((commit, parent, kind, last.get(kind), pin))
                    last[kind] = pin
                continue
            if is_trailer_form(old):
                report.flag(f"{short(commit)} gives the {kind} baseline a ref, and its parent has the trailer form; a commit names its pin with the trailer {old['trailer']}")
                continue
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

    # the list of the changes, with a reason on each line
    repository.fetch_contents(old, new)
    moved = baseline_summary.analyze(repository.folder, old, new, is_result)
    check_list(report, where, message, baseline_summary.lines(moved))

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
    description = parser.add_mutually_exclusive_group()
    description.add_argument("--description-file", help="a file that holds the description of the pull request")
    description.add_argument("--pr", type=int, help="a pull request number, to read its description")
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
                pins = sorted({ref for move in moves if move[2] == kind for ref in move[3:5] if ref})
                missing = repository.get(pins)
                for commit, parent, move_kind, old, new in moves:
                    if move_kind != kind:
                        continue
                    # the hint for a first pin of the trailer form: its step starts at the merge base with master
                    hint = ""
                    if old is None and new not in missing:
                        # the trailer form: the first pin moves from the base of its baseline branch
                        old = repository.branch_base(new)
                        if old is None:
                            report.flag(f"{short(commit)} pins {kind} {short(new)}, which has no common commit with master of the baseline repository; the baseline branch must stand on master")
                            continue
                        if old == new:
                            print(f"  {short(commit)} pins {kind} {short(new)}, which is on master of the baseline repository already")
                            continue
                        hint = (f"; the step starts at {short(old)}, the merge base of the pin and master of the baseline repository: "
                                f"a baseline branch that does not stand on master takes in other commits, so rebase it onto master")
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
                            report.flag(f"{short(commit)} moves the {kind} pin over {len(with_results)} baseline commits that change results; one cause commit has one baseline commit{hint}")
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

            fingerprint_steps = check_fingerprints(report, commits, records)
            steps = [commit for commit in commits if commit in fingerprint_steps or any(record[0] == commit for record in records)]
            check_description(report, read_description(args), steps)

            if fingerprint_steps:
                print()
                print("== the fingerprint changes (for the reviewer) ==")
                for commit, changes in fingerprint_steps.items():
                    print(f"  {short(commit)} {subject_of(commit)}")
                    print("\n".join("    " + line for _, line in changes))
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
    print("PASS: the pins and the baseline records are clean. Whether a reason is true, and whether a")
    print("      reduction gives the results that it claims, needs a run and a reviewer.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
