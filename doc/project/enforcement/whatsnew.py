#!/usr/bin/env python3
"""The release notes and the migration guide sections of the version in development: one file for
each entry in doc/whatsnew/, and one file for each section in doc/src/migration-guide/development/.

Rules RR-WHATSNEW-FILE and RR-MIGRATE-FILE in doc/project/rule/release.md say why they are files.
The file doc/whatsnew/README.md says how to write one. The commands, from the repository root:

  whatsnew.py preview [--rev REV]
      Print the section of the version in development, numbered, as the release will show it.
  whatsnew.py check [--rev REV | --range BASE..HEAD]
      Check the entry files and the section files, and that the section of the version in
      development in WHATSNEW has no entry. With --range, check each commit of the range, and also
      that no commit adds a section to the index.rst of the migration guide; this reads only git
      objects, so the pull-request workflow can run it on untrusted content.
  whatsnew.py release HEADING --next HEADING
      Write the entries into WHATSNEW as the section HEADING, put the heading of the next version in
      development above it, and remove the entry files. In the migration guide, move the section
      files of development/ into the folder of the release, for example 4.8/, and write the two
      headings.
  whatsnew.py rebase UPSTREAM | --continue
      Rebase the current branch onto UPSTREAM. When a commit stops on a conflict in WHATSNEW, write
      the entries that the commit adds, changes or removes in the section of the version in
      development as entry files, take WHATSNEW from the new base, and continue. Do the same for
      the sections of the index.rst of the migration guide. Stop on any other conflict; resolve it,
      then run whatsnew.py rebase --continue.

Exit status 0 = clean, 1 = a violation or a stop, 2 = invalid usage.
"""
import argparse
import os
import re
import subprocess
import sys
from collections import namedtuple
from pathlib import Path

ENTRY_DIR = "doc/whatsnew"
WHATSNEW = "WHATSNEW"
RULE = "RR-WHATSNEW-FILE"
# The three lists of a version, in the order of WHATSNEW: the folder and the heading of each.
SECTIONS = [
    ("incompatible", "Notable backward incompatible changes are the following:"),
    ("compatible", "Notable backward compatible changes are the following:"),
    ("fixes", "Notable bug fixes are the following:"),
]
STUB = ("The entries of this version are files in doc/whatsnew/ until the release. "
        "To see them as one text, run doc/project/enforcement/whatsnew.py preview.")
INDENT = "   "
NAME = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*\.txt$")
NUMBERED = re.compile(r"^(\d+)\.\s+(\S.*?)\s*$")
UNDERLINE = re.compile(r"^-{3,}\s*$")
ARTICLES = {"a", "an", "the"}

# An entry: its section folder, its file name (None for an entry read from WHATSNEW), its title,
# and its body as a list of lines without the indent of the body.
Entry = namedtuple("Entry", "section name title body")


def git(*args, check=True):
    out = subprocess.run(["git", *args], capture_output=True, text=True)
    if check and out.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)}: {out.stderr.strip()}")
    return out


class WorkTree:
    """The files of the working tree."""

    def __init__(self, root="."):
        self.root = Path(root)

    def read(self, path):
        file = self.root / path
        return file.read_text(encoding="utf-8") if file.is_file() else None

    def list(self, folder):
        base = self.root / folder
        if not base.is_dir():
            return []
        return sorted(str(p.relative_to(self.root)) for p in base.rglob("*") if p.is_file())


class GitTree:
    """The files of one commit, read as git objects and never as files on disk."""

    def __init__(self, rev):
        self.rev = rev

    def read(self, path):
        out = git("show", f"{self.rev}:{path}", check=False)
        return out.stdout if out.returncode == 0 else None

    def list(self, folder):
        out = git("ls-tree", "-r", "-z", "--name-only", self.rev, "--", folder)
        return sorted(path for path in out.stdout.split("\0") if path)


# --- WHATSNEW ---------------------------------------------------------------------------------

def section_of(line):
    """The section folder that a list heading of WHATSNEW starts, or None."""
    if line.startswith(" "):
        return None
    text = line.strip().lower()
    if not text.endswith(":") and "the following" not in text:
        return None
    if re.search(r"backward[- ]incompatible", text):
        return "incompatible"
    if re.search(r"backward[- ]compatible", text):
        return "compatible"
    if "bug fixes" in text:
        return "fixes"
    return None


def version_headings(lines):
    """The line indexes of the version headings: a line with a line of dashes below it."""
    return [i for i in range(len(lines) - 1)
            if lines[i].strip() and UNDERLINE.match(lines[i + 1])]


def development_section(lines):
    """(start, end) of the section of the version in development: the heading line and the line
    after the last line of the section. None if WHATSNEW has no such section."""
    headings = version_headings(lines)
    for n, i in enumerate(headings):
        if "(in development)" in lines[i]:
            end = headings[n + 1] if n + 1 < len(headings) else len(lines)
            return i, end
    return None


def dedent(body):
    """Remove the indent of the first line from each line, and the empty lines at both ends."""
    body = [line.rstrip() for line in body]
    while body and not body[0]:
        body.pop(0)
    while body and not body[-1]:
        body.pop()
    if not body:
        return []
    width = len(body[0]) - len(body[0].lstrip(" "))
    return [line[min(width, len(line) - len(line.lstrip(" "))):] for line in body]


def parse_entries(lines):
    """The numbered entries of a section of WHATSNEW, as (number, line index, Entry)."""
    found, section, current = [], None, None
    for i, line in enumerate(lines):
        key = section_of(line)
        if key:
            section, current = key, None
            continue
        match = NUMBERED.match(line)
        if match and section:
            current = [int(match[1]), i, section, match[2], []]
            found.append(current)
        elif current is not None:
            current[4].append(line)
    return [(number, i, Entry(section, None, title, dedent(body)))
            for number, i, section, title, body in found]


def development_entries(text):
    """The numbered entries of the section of the version in development in a WHATSNEW text."""
    if text is None:
        return []
    lines = text.splitlines()
    bounds = development_section(lines)
    if not bounds:
        return []
    start, end = bounds
    return [(number, start + i, entry) for number, i, entry in parse_entries(lines[start:end])]


def render_lists(entries):
    """The three numbered lists of a version, in the order of the file names."""
    out = []
    for key, heading in SECTIONS:
        items = sorted((e for e in entries if e.section == key), key=lambda e: e.name or "")
        if not items:
            continue
        out += [heading, ""]
        for number, entry in enumerate(items, 1):
            out += [f"{number}. {entry.title}", ""]
            out += [INDENT + line if line else "" for line in entry.body]
            out.append("")
    return out


def heading_lines(heading):
    return [heading, "-" * len(heading), ""]


# --- the entry files --------------------------------------------------------------------------

def parse_entry_file(text):
    """(title, body, problems) of the text of one entry file."""
    problems = []
    if not text.endswith("\n"):
        problems.append("does not end with a newline")
    if "\t" in text:
        problems.append("has a tab")
    lines = text.splitlines()
    for n, line in enumerate(lines, 1):
        if line != line.rstrip():
            problems.append(f"line {n} ends with a space")
    if not lines or not lines[0].strip():
        return None, [], problems + ["has no title on line 1"]
    title = lines[0].strip()
    if lines[0] != title:
        problems.append("starts its title with a space")
    if NUMBERED.match(title):
        problems.append("starts its title with a number; the release numbers the entries")
    if len(lines) < 2 or lines[1].strip():
        problems.append("has no empty line after the title")
    body = lines[2:]
    if not any(line.strip() for line in body):
        problems.append("has no text after the title")
    elif not body[0].strip():
        problems.append("has more than one empty line after the title")
    if body and not body[-1].strip():
        problems.append("ends with an empty line")
    for n in range(len(body) - 1):
        if not body[n].strip() and not body[n + 1].strip():
            problems.append(f"line {n + 3} is a second empty line")
            break
    return title, [line.rstrip() for line in body], problems


def read_entries(tree):
    """(entries, problems) of all entry files of a tree; a problem is (path, text)."""
    entries, problems, titles = [], [], {}
    sections = {key for key, _ in SECTIONS}
    for path in tree.list(ENTRY_DIR):
        parts = path[len(ENTRY_DIR) + 1:].split("/")
        if parts == ["README.md"]:
            continue
        if len(parts) != 2 or parts[0] not in sections:
            problems.append((path, "is not in incompatible/, compatible/ or fixes/ of doc/whatsnew/"))
            continue
        if not NAME.match(parts[1]):
            problems.append((path, "has a name that is not lower-case words with hyphens and .txt"))
        title, body, file_problems = parse_entry_file(tree.read(path))
        problems += [(path, p) for p in file_problems]
        if title is None:
            continue
        if title in titles:
            problems.append((path, f"has the same title as {titles[title]}"))
        titles.setdefault(title, path)
        entries.append(Entry(parts[0], parts[1], title, body))
    return entries, problems


def entry_text(entry):
    return "\n".join([entry.title, ""] + entry.body) + "\n"


def area_of(subject):
    """The area of a commit subject in lower case, for example 'ieee80211' for 'IEEE 802.11: ...'."""
    match = re.match(r"^\s*([^:]{1,40}):", subject or "")
    if not match:
        return None
    area = re.sub(r"[^a-z0-9]", "", match[1].split(",")[0].lower())
    return area or None


def file_name(title, area=None, limit=50, suffix=".txt"):
    """A file name for an entry: the area, then the words of the title, at most `limit` characters."""
    words = re.findall(r"[a-z0-9]+", title.lower())
    while words and words[0] in ARTICLES:
        words = words[1:]
    if area:
        for k in range(min(4, len(words)), 0, -1):
            if "".join(words[:k]) == area:
                words = words[k:]
                break
    while words and words[0] in ARTICLES:
        words = words[1:]
    kept = []
    for word in words:
        if kept and len("-".join(kept + [word])) > limit:
            break
        kept.append(word)
    stem = "-".join(([area] if area else []) + kept) or "entry"
    return stem + suffix


# --- the migration guide ----------------------------------------------------------------------

GUIDE_DIR = "doc/src/migration-guide"
GUIDE = f"{GUIDE_DIR}/index.rst"
GUIDE_RULE = "RR-MIGRATE-FILE"
DEVELOPMENT = "development"
FOLDER = re.compile(r"^(development|\d+(\.\d+)*)$")
SECTION_NAME = re.compile(r"^[a-z0-9]+(-[a-z0-9]+)*\.rst$")
INCLUDE = ".. include-files::"
LABEL = re.compile(r"^\.\. _.*:\s*$")

# A section file of the migration guide: its folder, its file name, its title, and its body.
Section = namedtuple("Section", "folder name title body")


def is_underline(line, char):
    text = line.rstrip()
    return len(text) >= 3 and set(text) == {char}


def parse_section_file(text):
    """(title, body, problems) of the text of one section file."""
    problems = []
    if not text.endswith("\n"):
        problems.append("does not end with a newline")
    if "\t" in text:
        problems.append("has a tab")
    lines = text.splitlines()
    for n, line in enumerate(lines, 1):
        if line != line.rstrip():
            problems.append(f"line {n} ends with a space")
    if not lines or not lines[0].strip():
        return None, [], problems + ["has no title on line 1"]
    title = lines[0].strip()
    if lines[0] != title:
        problems.append("starts its title with a space")
    if len(lines) < 2 or not is_underline(lines[1], "~"):
        problems.append("does not underline its title with ~ on line 2")
    elif len(lines[1].rstrip()) < len(title):
        problems.append("underlines its title with fewer ~ than the title has characters")
    if len(lines) < 3 or lines[2].strip():
        problems.append("has no empty line after the underline")
    body = lines[3:]
    if not any(line.strip() for line in body):
        problems.append("has no text after the title")
    elif not body[0].strip():
        problems.append("has more than one empty line after the underline")
    if body and not body[-1].strip():
        problems.append("ends with an empty line")
    for n in range(1, len(body)):
        if body[n - 1].strip() and any(is_underline(body[n], c) for c in "=-~"):
            problems.append(f"line {n + 4} underlines a heading with {body[n][0]}; a subsection uses ^")
            break
    return title, [line.rstrip() for line in body], problems


def read_sections(tree):
    """(sections, problems) of all section files of a tree; a problem is (path, text)."""
    sections, problems, titles = [], [], {}
    for path in tree.list(GUIDE_DIR):
        parts = path[len(GUIDE_DIR) + 1:].split("/")
        if len(parts) == 1 or not parts[-1].endswith(".rst"):
            continue
        if len(parts) != 2 or not FOLDER.match(parts[0]):
            problems.append((path, f"is not in {DEVELOPMENT}/ or in a version folder of {GUIDE_DIR}/"))
            continue
        if not SECTION_NAME.match(parts[1]):
            problems.append((path, "has a name that is not lower-case words with hyphens and .rst"))
        title, body, file_problems = parse_section_file(tree.read(path))
        problems += [(path, p) for p in file_problems]
        if title is None:
            continue
        if title in titles:
            problems.append((path, f"has the same title as {titles[title]}"))
        titles.setdefault(title, path)
        sections.append(Section(parts[0], parts[1], title, body))
    return sections, problems


def section_text(section):
    return "\n".join([section.title, "~" * len(section.title), ""] + section.body) + "\n"


def section_bounds(lines):
    """(start, head, end) of each section of index.rst with a - underline: the first label above it,
    its title line, and the line after its last line."""
    heads = [i - 1 for i in range(1, len(lines))
             if is_underline(lines[i], "-") and lines[i - 1].strip()
             and not is_underline(lines[i - 1], lines[i - 1].strip()[0])]
    starts = []
    for head in heads:
        start = head
        while start > 0 and (LABEL.match(lines[start - 1]) or not lines[start - 1].strip()):
            start -= 1
        while start < head and not LABEL.match(lines[start]):
            start += 1
        starts.append(start)
    return [(starts[n], head, starts[n + 1] if n + 1 < len(heads) else len(lines))
            for n, head in enumerate(heads)]


def guide_sections(text):
    """(preamble, sections) of an index.rst text: the lines before the first section, and for each
    section with a - underline (title, body, lines), where the lines start at the labels above it."""
    lines = (text or "").splitlines()
    bounds = section_bounds(lines)
    sections = []
    for start, head, end in bounds:
        body = lines[head + 2:end]
        while body and not body[0].strip():
            body.pop(0)
        while body and not body[-1].strip():
            body.pop()
        sections.append((lines[head].strip(), body, lines[start:end]))
    return lines[:bounds[0][0] if bounds else len(lines)], sections


def demote(body):
    """The body of a section of index.rst as the body of a section file: a ~ subsection becomes ^."""
    return [("^" * len(line.rstrip()) if n and body[n - 1].strip() and is_underline(line, "~") else line)
            for n, line in enumerate(body)]


def guide_release(heading, next_heading):
    """(index.rst text, moves, error) of a release: the text with the heading of the next version
    in development above the heading of the release, and the (source, target) of each section file
    of development/. (None, [], None) when the tree has no migration guide."""
    path = Path(GUIDE)
    if not path.is_file():
        return None, [], None
    lines = path.read_text(encoding="utf-8").splitlines()
    block = next(((start, end) for start, head, end in section_bounds(lines)
                  if "(in development)" in lines[head]), None)
    if not block:
        return None, [], f"{GUIDE} has no heading of the version in development"
    match = re.match(r"INET-(\S+)", heading)
    folder = match[1] if match else ""
    if not FOLDER.match(folder) or folder == DEVELOPMENT:
        return None, [], f"the heading \"{heading}\" names no version such as INET-4.8"
    target = Path(GUIDE_DIR) / folder
    sources = sorted((Path(GUIDE_DIR) / DEVELOPMENT).glob("*.rst"))
    if sources and target.exists():
        return None, [], f"{target} exists already"
    new = heading_lines(next_heading) + [f"{INCLUDE} {DEVELOPMENT}/*.rst", ""]
    if sources:
        new += heading_lines(heading) + [f"{INCLUDE} {folder}/*.rst", ""]
    text = lines[:block[0]] + new + lines[block[1]:]
    return "\n".join(text).rstrip("\n") + "\n", [(source, target / source.name) for source in sources], None


def is_version_section(body):
    """A version heading only includes the files of its folder."""
    return bool(body) and all(line.startswith(INCLUDE) for line in body if line.strip())


def section_changes(old_text, new_text):
    """The sections that new_text adds, changes or removes in an index.rst text against old_text,
    matched by title, without the version headings: (added, changed, removed) as (title, body)."""
    old = {t: b for t, b, _ in guide_sections(old_text)[1] if not is_version_section(b)}
    new = {t: b for t, b, _ in guide_sections(new_text)[1] if not is_version_section(b)}
    return ([(t, new[t]) for t in new if t not in old],
            [(t, new[t]) for t in new if t in old and old[t] != new[t]],
            [(t, old[t]) for t in old if t not in new])


# --- the commands -----------------------------------------------------------------------------

def preview(tree):
    lines = (tree.read(WHATSNEW) or "").splitlines()
    bounds = development_section(lines)
    heading = lines[bounds[0]] if bounds else "(in development)"
    entries, _ = read_entries(tree)
    return "\n".join(heading_lines(heading) + render_lists(entries)).rstrip("\n") + "\n"


def flag(text):
    print(f"  VIOLATION: {text}")


def suggestion(entry, area):
    return f"{ENTRY_DIR}/{entry.section}/{file_name(entry.title, area)}"


def check_tree(tree):
    """The violations of one tree: every entry in WHATSNEW, every problem of an entry file."""
    found = 0
    for _, i, entry in development_entries(tree.read(WHATSNEW)):
        flag(f"WHATSNEW:{i + 1} has the entry \"{entry.title}\" in the section of the version in "
             f"development; write it as {suggestion(entry, None)}")
        found += 1
    for path, problem in read_entries(tree)[1]:
        flag(f"{path} {problem}")
        found += 1
    return found


def entry_changes(old_text, new_text):
    """The entries that new_text adds, changes or removes in the section of the version in
    development, against old_text, matched by title: (added, changed, removed), where changed is a
    list of (old, new). An entry whose title changed keeps its section and number."""
    old = {e.title: (n, e) for n, _, e in development_entries(old_text)}
    new = {e.title: (n, e) for n, _, e in development_entries(new_text)}
    added = [new[t][1] for t in new if t not in old]
    removed = [old[t][1] for t in old if t not in new]
    changed = [(old[t][1], new[t][1]) for t in new if t in old and old[t][1] != new[t][1]]
    for entry in list(added):
        number = new[entry.title][0]
        twin = next((r for r in removed
                     if r.section == entry.section and old[r.title][0] == number), None)
        if twin:
            added.remove(entry)
            removed.remove(twin)
            changed.append((twin, entry))
    return added, changed, removed


def commits_touching(commit_range, files, folder):
    """(sha, parent, touched paths, short sha, subject) of each commit of a range that touches one
    of the files or a path below the folder."""
    for sha in git("rev-list", "--reverse", commit_range).stdout.split():
        parents = git("rev-list", "--parents", "-n", "1", sha).stdout.split()[1:]
        if not parents:
            continue
        paths = git("diff-tree", "-r", "-z", "--name-only", "--no-commit-id", parents[0], sha).stdout.split("\0")
        touched = [p for p in paths if p in files or p.startswith(folder + "/")]
        if touched:
            yield sha, parents[0], touched, sha[:9], git("log", "-1", "--format=%s", sha).stdout.strip()


def check_range(commit_range):
    """The violations of each commit of a range that touches WHATSNEW or doc/whatsnew/."""
    found = 0
    for sha, parent, touched, short, subject in commits_touching(commit_range, [WHATSNEW], ENTRY_DIR):
        tree = GitTree(sha)
        if WHATSNEW in touched:
            added, changed, _ = entry_changes(GitTree(parent).read(WHATSNEW), tree.read(WHATSNEW))
            for entry in added:
                flag(f"{short} adds the entry \"{entry.title}\" to WHATSNEW; write it as "
                     f"{suggestion(entry, area_of(subject))}")
            for _, entry in changed:
                flag(f"{short} changes the entry \"{entry.title}\" in WHATSNEW; change its file "
                     f"in {ENTRY_DIR}/")
            found += len(added) + len(changed)
        for path, problem in read_entries(tree)[1]:
            if path in touched:
                flag(f"{short} {path} {problem}")
                found += 1
    return found


def check_guide_tree(tree):
    """The violations of the migration guide of one tree."""
    found = 0
    guide = tree.read(GUIDE)
    if guide is not None and f"{INCLUDE} {DEVELOPMENT}/*.rst" not in guide.splitlines():
        flag(f"{GUIDE} does not include {DEVELOPMENT}/*.rst below the heading of the version in development")
        found += 1
    for path, problem in read_sections(tree)[1]:
        flag(f"{path} {problem}")
        found += 1
    return found


def check_guide_range(commit_range):
    """The violations of each commit of a range that touches the migration guide."""
    found = 0
    for sha, parent, touched, short, subject in commits_touching(commit_range, [GUIDE], GUIDE_DIR):
        tree = GitTree(sha)
        if GUIDE in touched:
            added = section_changes(GitTree(parent).read(GUIDE), tree.read(GUIDE))[0]
            for title, _ in added:
                name = file_name(title, area_of(subject), suffix=".rst")
                flag(f"{short} adds the section \"{title}\" to {GUIDE}; write it as "
                     f"{GUIDE_DIR}/{DEVELOPMENT}/{name}")
            found += len(added)
        for path, problem in read_sections(tree)[1]:
            if path in touched:
                flag(f"{short} {path} {problem}")
                found += 1
    return found


def check(args):
    found = 0
    for rule, text, by_range, by_tree in (
            (RULE, f"a release note is a file of its own in {ENTRY_DIR}/", check_range, check_tree),
            (GUIDE_RULE, f"a migration section is a file of its own in {GUIDE_DIR}/{DEVELOPMENT}/",
             check_guide_range, check_guide_tree)):
        print(f"== {rule}: {text} ==")
        count = by_range(args.range) if args.range else by_tree(GitTree(args.rev) if args.rev else WorkTree())
        if not count:
            print("  ok")
        print()
        found += count
    if found:
        print(f"FAIL: see {RULE} and {GUIDE_RULE} in doc/project/rule/release.md, and {ENTRY_DIR}/README.md.")
        return 1
    print("PASS: release note checks clean. Whether an entry or a section is in the right place, and says")
    print("      what a user needs, still needs T4 review (RR-WHATSNEW, RR-BREAK-MIGRATE).")
    return 0


def release(args):
    tree = WorkTree()
    lines = (tree.read(WHATSNEW) or "").splitlines()
    bounds = development_section(lines)
    if not bounds:
        print("whatsnew.py: WHATSNEW has no section of the version in development", file=sys.stderr)
        return 2
    if development_entries(tree.read(WHATSNEW)):
        print("whatsnew.py: WHATSNEW has entries in the section of the version in development; "
              "run whatsnew.py check", file=sys.stderr)
        return 1
    entries, problems = read_entries(tree)
    if problems:
        for path, problem in problems:
            print(f"whatsnew.py: {path} {problem}", file=sys.stderr)
        return 1
    guide, moves, error = guide_release(args.heading, args.next)
    if error:
        print(f"whatsnew.py: {error}", file=sys.stderr)
        return 1
    start, end = bounds
    # The text of the section other than the stub sentence is the introduction of the release.
    intro = dedent([line for line in lines[start + 2:end] if line.strip() != STUB])
    released = heading_lines(args.heading) + (intro + [""] if intro else []) + render_lists(entries)
    text = lines[:start] + heading_lines(args.next) + [STUB, ""] + released + lines[end:]
    Path(WHATSNEW).write_text("\n".join(text).rstrip("\n") + "\n", encoding="utf-8")
    for entry in entries:
        (Path(ENTRY_DIR) / entry.section / entry.name).unlink()
    print(f"WHATSNEW has the section \"{args.heading}\" with {len(entries)} entries, and "
          f"{len(entries)} entry files are removed.")
    if guide is not None:
        Path(GUIDE).write_text(guide, encoding="utf-8")
        for source, target in moves:
            target.parent.mkdir(parents=True, exist_ok=True)
            source.rename(target)
        print(f"{GUIDE} has the heading \"{args.next}\" above the heading of the release, and "
              f"{len(moves)} section files moved from {DEVELOPMENT}/ into the folder of the release.")
    if not intro:
        print(f"Write the introduction of \"{args.heading}\" in WHATSNEW.")
    print(f"Then run: git add -A {WHATSNEW} {ENTRY_DIR} {GUIDE_DIR}")
    return 0


# --- the rebase ---------------------------------------------------------------------------------

def git_path(name):
    return Path(git("rev-parse", "--git-path", name).stdout.strip())


def rebase_in_progress():
    return git_path("rebase-merge").is_dir() or git_path("rebase-apply").is_dir()


def outside_development(text):
    """The lines of a WHATSNEW text without the section of the version in development."""
    lines = (text or "").splitlines()
    bounds = development_section(lines)
    return lines if not bounds else lines[:bounds[0]] + lines[bounds[1]:]


def branch_area():
    """The most frequent area of the commits that the rebase replays, other than doc and plan."""
    state = git_path("rebase-merge")
    if not state.is_dir():
        return None
    onto, head = ((state / name).read_text().strip() for name in ("onto", "orig-head"))
    base = git("merge-base", onto, head).stdout.strip()
    areas = [area_of(s) for s in git("log", "--format=%s", f"{base}..{head}").stdout.splitlines()]
    areas = [a for a in areas if a and a not in ("doc", "plan")]
    return max(sorted(set(areas)), key=areas.count) if areas else None


def write_entry(path, entry):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(entry_text(entry), encoding="utf-8")
    git("add", "--", str(path))


def convert(sha):
    """Resolve the conflict in WHATSNEW of the commit that the rebase stopped on: write the entry
    changes of the commit as entry files, and take WHATSNEW from HEAD. (done, messages)."""
    short, subject = sha[:9], git("log", "-1", "--format=%s", sha).stdout.strip()
    old_text, new_text = GitTree(f"{sha}^").read(WHATSNEW), GitTree(sha).read(WHATSNEW)
    if outside_development(old_text) != outside_development(new_text):
        return False, [f"{short} changes WHATSNEW outside the section of the version in development; "
                       "resolve WHATSNEW by hand"]
    added, changed, removed = entry_changes(old_text, new_text)
    files = {e.title: e for e in read_entries(WorkTree())[0]}
    for old, _ in changed:
        if old.title not in files:
            return False, [f"{short} changes the entry \"{old.title}\", and no entry file has that title; "
                           "resolve WHATSNEW by hand"]
    area = area_of(subject)
    if area in (None, "doc", "plan"):
        area = branch_area()
    messages = []
    for entry in added:
        if entry.title in files:
            current = files[entry.title]
            if (current.section, current.body) == (entry.section, entry.body):
                messages.append(f"{short} keeps {ENTRY_DIR}/{current.section}/{current.name}, which has the same entry")
            else:
                changed.append((current, entry))
            continue
        stem = file_name(entry.title, area)[:-len(".txt")]
        path, n = Path(ENTRY_DIR) / entry.section / f"{stem}.txt", 2
        while path.exists():
            path, n = path.with_name(f"{stem}-{n}.txt"), n + 1
        write_entry(path, entry)
        messages.append(f"{short} adds {path}")
        if area and not any(e.name.startswith(f"{area}-") for e in files.values()):
            messages.append(f"note: no other release note has the area '{area}'; rename {path} if another area fits")
    for old, new in changed:
        current = files[old.title]
        path = Path(ENTRY_DIR) / current.section / current.name
        if new.section != current.section:
            git("rm", "-q", "--", str(path))
            path = Path(ENTRY_DIR) / new.section / current.name
        write_entry(path, new)
        messages.append(f"{short} changes {path}")
    for entry in removed:
        if entry.title in files:
            current = files[entry.title]
            path = Path(ENTRY_DIR) / current.section / current.name
            git("rm", "-q", "--", str(path))
            messages.append(f"{short} removes {path}")
    git("checkout", "HEAD", "--", WHATSNEW)
    if not messages:
        messages.append(f"{short} changes no entry")
    return True, messages


def write_section(path, section):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(section_text(section), encoding="utf-8")
    git("add", "--", str(path))


def convert_guide(sha):
    """Resolve the conflict in the index.rst of the migration guide of the commit that the rebase
    stopped on: write the section changes of the commit as section files of development/, and take
    index.rst from HEAD. (done, messages)."""
    short, subject = sha[:9], git("log", "-1", "--format=%s", sha).stdout.strip()
    old_text, new_text = GitTree(f"{sha}^").read(GUIDE), GitTree(sha).read(GUIDE)
    if guide_sections(old_text)[0] != guide_sections(new_text)[0]:
        return False, [f"{short} changes the part of {GUIDE} above its sections; resolve it by hand"]
    added, changed, removed = section_changes(old_text, new_text)
    kept = {t for t, b, _ in guide_sections(GitTree("HEAD").read(GUIDE))[1] if not is_version_section(b)}
    files = {section.title: section for section in read_sections(WorkTree())[0]}
    for title, _ in changed + removed:
        if title in kept:
            return False, [f"{short} changes the section \"{title}\", which stays in {GUIDE}; resolve it by hand"]
    for title, _ in changed:
        if title not in files:
            return False, [f"{short} changes the section \"{title}\", and no section file has that title; "
                           f"resolve {GUIDE} by hand"]
    area = area_of(subject)
    if area in (None, "doc", "plan"):
        area = branch_area()
    messages = []
    for title, body in added:
        if title in files:
            changed.append((title, body))
            continue
        stem = file_name(title, area, suffix="")
        path, n = Path(GUIDE_DIR) / DEVELOPMENT / f"{stem}.rst", 2
        while path.exists():
            path, n = path.with_name(f"{stem}-{n}.rst"), n + 1
        write_section(path, Section(DEVELOPMENT, path.name, title, demote(body)))
        messages.append(f"{short} adds {path}")
    for title, body in changed:
        current = files[title]
        path = Path(GUIDE_DIR) / current.folder / current.name
        if current.body == demote(body):
            messages.append(f"{short} keeps {path}, which has the same section")
            continue
        write_section(path, current._replace(body=demote(body)))
        messages.append(f"{short} changes {path}")
    for title, _ in removed:
        if title in files:
            current = files[title]
            path = Path(GUIDE_DIR) / current.folder / current.name
            git("rm", "-q", "--", str(path))
            messages.append(f"{short} removes {path}")
    git("checkout", "HEAD", "--", GUIDE)
    if not messages:
        messages.append(f"{short} changes no section")
    return True, messages


def rebase(args):
    state = git_path("whatsnew-rebase")
    editor = {**os.environ, "GIT_EDITOR": "true"}
    if args.upstream:
        if rebase_in_progress():
            print("whatsnew.py: a rebase is in progress; run whatsnew.py rebase --continue", file=sys.stderr)
            return 2
        upstream = git("rev-parse", "--verify", f"{args.upstream}^{{commit}}").stdout.strip()
        state.write_text(upstream + "\n")
        started = subprocess.run(["git", "rebase", upstream], env=editor, capture_output=True, text=True)
        if started.returncode != 0 and not rebase_in_progress():
            state.unlink(missing_ok=True)
            print(f"whatsnew.py: git rebase did not start: {started.stderr.strip()}", file=sys.stderr)
            return 1
    else:
        if not rebase_in_progress() or not state.is_file():
            print("whatsnew.py: no rebase of whatsnew.py is in progress", file=sys.stderr)
            return 2
        upstream = state.read_text().strip()
        subprocess.run(["git", "rebase", "--continue"], env=editor, capture_output=True, text=True)
    while rebase_in_progress():
        sha = git("rev-parse", "--verify", "-q", "REBASE_HEAD", check=False).stdout.strip()
        conflicts = [p for p in git("diff", "--name-only", "-z", "--diff-filter=U").stdout.split("\0") if p]
        if not sha or not conflicts:
            print("whatsnew.py: the rebase stopped without a conflict; see git status", file=sys.stderr)
            return 1
        for path, converter in ((WHATSNEW, convert), (GUIDE, convert_guide)):
            if path in conflicts:
                done, messages = converter(sha)
                for message in messages:
                    print(f"whatsnew.py: {message}")
                if not done:
                    return 1
        others = [p for p in conflicts if p not in (WHATSNEW, GUIDE)]
        if others:
            print(f"whatsnew.py: {sha[:9]} also conflicts in {', '.join(others)}. Resolve them, add them, "
                  "then run whatsnew.py rebase --continue")
            return 1
        if git("diff", "--cached", "--quiet", check=False).returncode == 0:
            print(f"whatsnew.py: {sha[:9]} is empty after the conversion, and the rebase skips it")
            subprocess.run(["git", "rebase", "--skip"], env=editor, capture_output=True, text=True)
        else:
            subprocess.run(["git", "rebase", "--continue"], env=editor, capture_output=True, text=True)
    state.unlink(missing_ok=True)
    return check(argparse.Namespace(range=f"{upstream}..HEAD", rev=None))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    commands = parser.add_subparsers(dest="command", required=True)
    command = commands.add_parser("preview", help="print the section of the version in development")
    command.add_argument("--rev", help="read the files of this commit, not of the working tree")
    command = commands.add_parser("check", help="check the entry files and WHATSNEW")
    group = command.add_mutually_exclusive_group()
    group.add_argument("--rev", help="check the files of this commit, not of the working tree")
    group.add_argument("--range", help="check each commit of BASE..HEAD that touches the release notes")
    command = commands.add_parser("release", help="write the entries into WHATSNEW for a release")
    command.add_argument("heading", help='for example "INET-4.8 (March 2027) — feature release"')
    command.add_argument("--next", required=True, help='for example "INET-4.8.x (in development)"')
    command = commands.add_parser("rebase", help="rebase, and move the WHATSNEW entries into files")
    group = command.add_mutually_exclusive_group(required=True)
    group.add_argument("upstream", nargs="?", help="the branch to rebase onto, for example origin/master")
    group.add_argument("--continue", dest="continue_", action="store_true",
                       help="continue after the other conflicts are resolved")
    args = parser.parse_args(argv)
    try:
        os.chdir(git("rev-parse", "--show-toplevel").stdout.strip())
        if args.command == "preview":
            sys.stdout.write(preview(GitTree(args.rev) if args.rev else WorkTree()))
            return 0
        if args.command == "check":
            return check(args)
        if args.command == "rebase":
            return rebase(args)
        return release(args)
    except RuntimeError as error:
        print(f"whatsnew.py: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
