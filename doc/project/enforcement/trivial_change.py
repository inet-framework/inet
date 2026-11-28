#!/usr/bin/env python3
"""The push check of PR-LAND-TRIVIAL in doc/project/rule/pull-request.md: is the change of a branch trivial?

A trivial change can go to master with no pull request (PR-LAND-WAYS). The gate looks at the commits from the
merge base of BASE and HEAD to HEAD:

  files     each changed file is trivial: a change of an .rst or .md file, or a change of a .cc, .h or .ned
            file that changes only comments and keeps the line number of each line of code. Never trivial:
            a file under .github/, _scripts/, doc/project/enforcement/ or tests/, a .txt, .ini or .msg
            file, a document with a seal unit, the seal registry, a sealed source path, a new, removed or
            renamed C++ or NED file, and a change of the file mode
  commits   no commit has a pin trailer, such as Statistical-Baseline:, because it moves a baseline at the
            landing; and the gates of the pull-request audit pass, without the description

For C++, the lexer knows line comments (also when a backslash at the end continues them), block
comments, strings, character literals, raw strings and digit separators. For NED, it knows line
comments, strings in double or single quotes, and property values, in which // is not a comment.
It replaces each comment with a space, keeps the line breaks, and compares the old and the new text
line by line, without the spaces at the end of a line.

The gate reads the commits only as git objects, so the trusted push-check workflow can run it on a
pushed branch. It runs check-source-seals.sh and check-pull-request.sh from its own folder.

Usage (from the INET repository root):
  doc/project/enforcement/check-trivial-change.sh --base origin/master --head HEAD
  doc/project/enforcement/check-trivial-change.sh --base <sha> --head <sha> --required-contexts-file contexts.txt

Exit status 0 = trivial, 1 = not trivial (the change needs a pull request), 2 = the gate cannot run.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
RULES = "doc/project/rule/pull-request.md"
NEVER_FOLDERS = (".github/", "_scripts/", "doc/project/enforcement/", "tests/")
NEVER_SUFFIXES = (".txt", ".ini", ".msg")
DOCUMENT_SUFFIXES = (".rst", ".md")
CODE_SUFFIXES = {".cc": "c++", ".h": "c++", ".ned": "ned"}
SEAL_REGISTRY = ("doc/project/audit/seal-list.md",)
# the header of a document under doc/project, as check_seals.py reads it
SEAL_HEADER = re.compile(r"^>\s*\*\*Kind:\*\*.*?\*\*Seal:\*\*\s*([^·]+?)\s*(?:·|$)", re.M)
PIN_TRAILER = re.compile(r"^([A-Za-z][A-Za-z-]*-Baseline):", re.M)
REGULAR_MODES = ("100644", "100755")


class CannotRun(Exception):
    pass


def git(*args, check=True):
    out = subprocess.run(["git", *args], capture_output=True)
    if check and out.returncode != 0:
        raise CannotRun(f"git {' '.join(args)}: {out.stderr.decode(errors='replace').strip()}")
    return out


def blob(commit, path):
    """The content of a file at a commit, as text with one character for each byte."""
    return git("show", f"{commit}:{path}").stdout.decode("latin-1")


# ---------------------------------------------------------------- the lexer

IDENTIFIER = re.compile(r"[A-Za-z0-9_]")
RAW_PREFIX = re.compile(r"(?:u8|u|U|L)?R$")


def strip_cpp(text):
    """C++ text with each comment replaced by a space, and with the line breaks of the comments kept."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            out.append(" ")
            i += 2
            while i < n and text[i] != "\n":
                # a backslash before the line break continues the comment on the next line
                if text[i] == "\\" and text.startswith("\n", i + 1):
                    out.append("\n")
                    i += 2
                    continue
                if text[i] == "\\" and text.startswith("\r\n", i + 1):
                    out.append("\r\n")
                    i += 3
                    continue
                i += 1
        elif text.startswith("/*", i):
            out.append(" ")
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            out.append("\n" * text.count("\n", i, end))
            i = end
        elif c == '"':
            j = i
            while j > 0 and IDENTIFIER.match(text[j - 1]):
                j -= 1
            prefix = text[j:i]
            if RAW_PREFIX.search(prefix) and RAW_PREFIX.search(prefix).start() == 0:
                open_paren = text.find("(", i + 1, i + 18)
                delimiter = text[i + 1:open_paren] if open_paren >= 0 else None
                if delimiter is not None and not re.search(r'[\s()\\"]', delimiter):
                    close = text.find(f"){delimiter}\"", open_paren + 1)
                    end = n if close < 0 else close + len(delimiter) + 2
                    out.append(text[i:end])
                    i = end
                    continue
            i = quoted(text, i, '"', out)
        elif c == "'":
            i = quoted(text, i, "'", out)
        elif c.isdigit() or (c == "." and i + 1 < n and text[i + 1].isdigit()):
            # a number, with its digit separators: 1'000'000
            j = i + 1
            while j < n:
                if text[j] in "eEpP" and j + 1 < n and text[j + 1] in "+-":
                    j += 2
                elif IDENTIFIER.match(text[j]) or text[j] == ".":
                    j += 1
                elif text[j] == "'" and j + 1 < n and IDENTIFIER.match(text[j + 1]):
                    j += 2
                else:
                    break
            out.append(text[i:j])
            i = j
        elif IDENTIFIER.match(c):
            j = i + 1
            while j < n and IDENTIFIER.match(text[j]):
                j += 1
            out.append(text[i:j])
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def quoted(text, i, quote, out):
    """Copies the literal that starts with the quote at i, to its end or to the end of its line."""
    j, n = i + 1, len(text)
    while j < n and text[j] != quote and text[j] != "\n":
        j += 2 if text[j] == "\\" else 1
    end = min(j + 1, n) if j < n and text[j] == quote else min(j, n)
    out.append(text[i:end])
    return end


NED_PROPERTY = re.compile(r"@[A-Za-z0-9_:.-]+\s*(?:\[[A-Za-z0-9_*?{}:.-]*\]\s*)?\(")


def strip_ned(text):
    """NED text with each comment replaced by a space. In a property value, such as @doc(...), // is
    not a comment, and only a double quote starts a string."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            out.append(" ")
            end = text.find("\n", i)
            i = n if end < 0 else end
        elif c in "\"'":
            i = quoted(text, i, c, out)
        elif c == "@" and NED_PROPERTY.match(text, i):
            j, depth = NED_PROPERTY.match(text, i).end(), 1
            while j < n and depth > 0:
                if text[j] == '"':
                    part = []
                    j = quoted(text, j, '"', part)
                    continue
                if text[j] in "({[":
                    depth += 1
                elif text[j] in ")}]":
                    depth -= 1
                j += 1
            out.append(text[i:j])
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def code_lines(text, language):
    """The lines of the text without comments, without the spaces at the end of a line, and without the
    empty lines at the end of the file: none of these moves a line of code."""
    stripped = strip_cpp(text) if language == "c++" else strip_ned(text)
    lines = [line.rstrip(" \t") for line in stripped.split("\n")]
    while lines and not lines[-1]:
        lines.pop()
    return lines


def first_code_change(old, new, language):
    """None when the two texts differ only in comments and keep each line of code on its line, or else
    the line number in the new text of the first difference."""
    a, b = code_lines(old, language), code_lines(new, language)
    for number, (x, y) in enumerate(zip(a, b), start=1):
        if x != y:
            return number
    return None if len(a) == len(b) else min(len(a), len(b)) + 1


# ---------------------------------------------------------------- the file rules

def seal_unit(commit, path):
    """The seal unit of a document under doc/project at a commit, or None."""
    if git("cat-file", "-e", f"{commit}:{path}", check=False).returncode != 0:
        return None
    match = SEAL_HEADER.search(git("show", f"{commit}:{path}").stdout.decode("utf-8", "replace"))
    return match.group(1).strip() if match else None


def changed_files(base, head):
    """(old mode, new mode, status, path) of each changed file; a rename is a removal and an addition."""
    raw = git("diff", "--raw", "--no-renames", "-z", "--no-abbrev", base, head).stdout.decode("utf-8", "surrogateescape")
    fields = raw.split("\0")
    result, i = [], 0
    while i + 1 < len(fields) and fields[i].startswith(":"):
        old_mode, new_mode, _, _, status = fields[i][1:].split(" ")
        result.append((old_mode, new_mode, status[0], fields[i + 1]))
        i += 2
    return result


def file_verdict(base, head, old_mode, new_mode, status, path):
    """(trivial, reason) for one changed file."""
    for folder in NEVER_FOLDERS:
        if path.startswith(folder):
            return False, f"never trivial: under {folder}"
    if path in SEAL_REGISTRY:
        return False, "never trivial: the seal registry"
    suffix = Path(path).suffix
    if suffix in NEVER_SUFFIXES:
        return False, f"never trivial: a {suffix} file"
    modes = {m for m in (old_mode, new_mode) if m != "000000"}
    if not modes <= set(REGULAR_MODES) or len(modes) > 1:
        return False, f"a change of the file mode ({old_mode} to {new_mode})"
    if suffix in DOCUMENT_SUFFIXES:
        if path.startswith("doc/project/") and suffix == ".md":
            units = {u for u in (seal_unit(base, path), seal_unit(head, path)) if u is not None}
            if any(u != "none" for u in units):
                return False, f"never trivial: a document with the seal unit '{sorted(u for u in units if u != 'none')[0]}'"
        return True, "a document"
    if suffix in CODE_SUFFIXES:
        if status != "M":
            return False, {"A": "a new", "D": "a removed"}.get(status, "a changed") + " C++ or NED file"
        line = first_code_change(blob(base, path), blob(head, path), CODE_SUFFIXES[suffix])
        if line is not None:
            return False, f"a change of code, or of the line of code, at line {line}"
        return True, "comments only, each line of code on its line"
    return False, "not a document, and not a C++ or NED file"


# ---------------------------------------------------------------- the gate

def run_gate(script, *args):
    out = subprocess.run(["bash", str(HERE / script), *args], capture_output=True, text=True)
    return out.returncode, out.stdout + out.stderr


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--base", required=True, help="master, for example origin/master")
    parser.add_argument("--head", default="HEAD", help="the head of the branch (default HEAD)")
    parser.add_argument("--required-contexts-file", help="the required contexts of master, for the audit")
    args = parser.parse_args(argv)
    reasons = []
    try:
        tip = git("rev-parse", "--verify", f"{args.base}^{{commit}}").stdout.decode().strip()
        head = git("rev-parse", "--verify", f"{args.head}^{{commit}}").stdout.decode().strip()
        base = git("merge-base", tip, head).stdout.decode().strip()
        commits = git("rev-list", f"{base}..{head}").stdout.decode().split()
        print(f"== the push check: {base[:10]}..{head[:10]}, {len(commits)} commit(s) ==")
        if base != tip:
            print(f"  note: master moved since the branch started; rebase the branch on master before the push to master (PR-LAND-TRIVIAL)")
        if not commits:
            reasons.append("the branch has no commit that master does not have")

        print()
        print("== PR-LAND-TRIVIAL: each changed file is trivial ==")
        files = changed_files(base, head)
        for old_mode, new_mode, status, path in files:
            trivial, reason = file_verdict(base, head, old_mode, new_mode, status, path)
            print(f"  {'trivial' if trivial else 'NOT TRIVIAL'}: {path}: {reason}")
            if not trivial:
                reasons.append(f"{path}: {reason}")
        if not files:
            print("  no changed file")

        print()
        print("== PR-LAND-TRIVIAL: no pin trailer ==")
        quiet = True
        for commit in commits:
            message = git("log", "-1", "--format=%B", commit).stdout.decode("utf-8", "replace")
            for key in sorted(set(PIN_TRAILER.findall(message))):
                print(f"  NOT TRIVIAL: {commit[:10]} has the pin trailer {key}:, which moves a baseline at the landing")
                reasons.append(f"{commit[:10]}: the pin trailer {key}:")
                quiet = False
        if quiet:
            print("  ok")
    except CannotRun as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    print()
    print("== PR-LAND-TRIVIAL: no sealed source path ==")
    status, output = run_gate("check-source-seals.sh", "--base", base, "--head", head)
    if status == 0:
        print("  ok")
    elif status == 1:
        print("  NOT TRIVIAL: the change touches a sealed source path:")
        print("".join(f"    {line}\n" for line in output.strip().splitlines()), end="")
        reasons.append("a sealed source path")
    else:
        print(f"error: check-source-seals.sh cannot run:\n{output}", file=sys.stderr)
        return 2

    print()
    print("== PR-LAND-TRIVIAL: the gates of the pull-request audit, without the description ==")
    audit = ["--base", tip, "--head", head]
    if args.required_contexts_file:
        audit += ["--required-contexts-file", args.required_contexts_file]
    status, output = run_gate("check-pull-request.sh", *audit)
    summary = [line for line in output.splitlines() if re.match(r"^  check-\S+\s+(pass|FAIL|ERROR|not run)", line)]
    print("".join(f"  {line.strip()}\n" for line in summary), end="")
    if status == 1:
        print("  NOT TRIVIAL: the audit has a violation; run check-pull-request.sh for the details")
        reasons.append("a violation of the audit")
    elif status != 0:
        print(f"error: check-pull-request.sh cannot run:\n{output[-2000:]}", file=sys.stderr)
        return 2

    print()
    if reasons:
        print(f"NOT TRIVIAL: this change needs a pull request ({len(reasons)} reason(s), PR-LAND-TRIVIAL of {RULES}).")
        return 1
    print("TRIVIAL: the change can go to master with no pull request (PR-LAND-WAYS); push the same commit to master.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
