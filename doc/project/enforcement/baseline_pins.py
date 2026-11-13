#!/usr/bin/env python3
"""Reads the baseline pins of INET commits.

inet.opp declares a baseline repository for some test kinds (statistical, chart), in the baseline
entry of the kind in test_parameters. The ref field of that entry is the pin: the commit of the
baseline repository that the INET commit passes with (TR-BASELINE-PIN). This module reads the
entries of a commit from git objects only. It never runs inet.opp, so a gate can use it on the
content of a pull request.

Usage, from the repository root:
  baseline_pins.py show [COMMIT]    the baseline entries of the commit (default HEAD)
  baseline_pins.py moved COMMIT     the test kinds whose pin differs from the pin of the parent

Exit status 0 = done, 2 = invalid usage or an inet.opp that cannot be read.
"""
import ast
import re
import subprocess
import sys

OPP_FILE = "inet.opp"
FULL_HASH = re.compile(r"[0-9a-f]{40}")


class PinError(Exception):
    """inet.opp of a commit does not give its baseline entries."""


def git(*args, cwd=None, check=True):
    out = subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True)
    if check and out.returncode != 0:
        raise RuntimeError(f"git {' '.join(args)} failed:\n{out.stderr.strip()}")
    return out


def is_full_hash(ref):
    return isinstance(ref, str) and FULL_HASH.fullmatch(ref) is not None


def parse_baselines(text):
    """{kind: baseline entry} of the text of an inet.opp. The text is data: it is parsed, not run."""
    try:
        tree = ast.parse(text, mode="eval")
    except SyntaxError as error:
        raise PinError(f"{OPP_FILE} is not a Python expression: {error.msg}, line {error.lineno}") from None
    if not isinstance(tree.body, ast.Call):
        raise PinError(f"{OPP_FILE} is not one call of SimulationProject")
    for keyword in tree.body.keywords:
        if keyword.arg == "test_parameters":
            try:
                test_parameters = ast.literal_eval(keyword.value)
            except ValueError:
                raise PinError(f"test_parameters of {OPP_FILE} is not a literal") from None
            if not isinstance(test_parameters, dict):
                raise PinError(f"test_parameters of {OPP_FILE} is not a dict")
            baselines = {}
            for kind, parameters in test_parameters.items():
                if isinstance(parameters, dict) and parameters.get("baseline"):
                    if not isinstance(parameters["baseline"], dict):
                        raise PinError(f"the baseline entry of the {kind} tests is not a dict")
                    baselines[kind] = parameters["baseline"]
            return baselines
    return {}


def read_baselines(commit, cwd=None):
    """{kind: baseline entry} at the commit; empty when the commit has no inet.opp."""
    out = git("show", f"{commit}:{OPP_FILE}", cwd=cwd, check=False)
    if out.returncode != 0:
        return {}
    return parse_baselines(out.stdout)


def read_pins(commit, cwd=None):
    """{kind: ref} at the commit, for the kinds whose baseline entry has a ref."""
    return {kind: baseline["ref"] for kind, baseline in read_baselines(commit, cwd).items() if baseline.get("ref")}


def moved_pins(commit, cwd=None):
    """[(kind, old ref, new ref)] for the pins of the commit that differ from the pins of its first
    parent. A pin that the commit adds or removes has None on the other side."""
    parent = git("rev-parse", "--verify", "--quiet", f"{commit}^", cwd=cwd, check=False).stdout.strip()
    old = read_pins(parent, cwd) if parent else {}
    new = read_pins(commit, cwd)
    return [(kind, old.get(kind), new.get(kind)) for kind in sorted(set(old) | set(new)) if old.get(kind) != new.get(kind)]


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    try:
        if len(argv) in (1, 2) and argv[0] == "show":
            for kind, baseline in read_baselines(argv[1] if len(argv) == 2 else "HEAD").items():
                print(kind, baseline.get("repository"), baseline.get("ref"), baseline.get("folder"))
        elif len(argv) == 2 and argv[0] == "moved":
            # a pin that a commit adds is its first recording, not a move
            print(" ".join(kind for kind, old, new in moved_pins(argv[1]) if old and new))
        else:
            print(__doc__, file=sys.stderr)
            return 2
    except (PinError, RuntimeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
