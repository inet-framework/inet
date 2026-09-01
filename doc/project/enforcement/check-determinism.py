#!/usr/bin/env python3
"""Find forbidden nondeterministic calls in C++ code, excluding comments and literals."""

from __future__ import annotations

import re
import sys
from pathlib import Path


SOURCE_SUFFIXES = {".cc", ".h", ".icc"}
EXCLUDED_PARTS = {"visualizer", "thirdparty", "external"}
FORBIDDEN = re.compile(
    r"std::random_device|std::chrono::|"
    r"(?<![A-Za-z0-9_])rand\s*\(|"
    r"(?<![A-Za-z0-9_])time\s*\("
)
RAW_STRING = re.compile(r'(?:u8|u|U|L)?R"([^ ()\\\t\r\n]{0,16})\(')
RESULT_FILTERS_ALLOW = re.compile(
    r"(?:startTime = time\(nullptr\);|time_t t = time\(nullptr\);)"
)


def blank(output: list[str], start: int, end: int) -> None:
    for index in range(start, end):
        if output[index] != "\n":
            output[index] = " "


def strip_comments_and_literals(text: str) -> str:
    """Blank C++ comments and string/character literals while preserving lines."""
    output = list(text)
    index = 0
    while index < len(text):
        if text.startswith("//", index):
            end = text.find("\n", index + 2)
            end = len(text) if end == -1 else end
            blank(output, index, end)
            index = end
            continue
        if text.startswith("/*", index):
            close = text.find("*/", index + 2)
            end = len(text) if close == -1 else close + 2
            blank(output, index, end)
            index = end
            continue
        raw = RAW_STRING.match(text, index)
        if raw is not None:
            terminator = ")" + raw.group(1) + '"'
            close = text.find(terminator, raw.end())
            end = len(text) if close == -1 else close + len(terminator)
            blank(output, index, end)
            index = end
            continue
        if text[index] in {'"', "'"}:
            quote = text[index]
            end = index + 1
            escaped = False
            while end < len(text):
                char = text[end]
                if char == quote and not escaped:
                    end += 1
                    break
                if char == "\n" and quote == '"' and not escaped:
                    break
                escaped = char == "\\" and not escaped
                if char != "\\":
                    escaped = False
                end += 1
            blank(output, index, end)
            index = end
            continue
        index += 1
    return "".join(output)


def allowed(relative: Path, code: str) -> bool:
    return (
        relative.as_posix() == "src/inet/common/ResultFilters.cc"
        and RESULT_FILTERS_ALLOW.fullmatch(code.strip()) is not None
    )


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {Path(sys.argv[0]).name} src/inet/<subtree>", file=sys.stderr)
        return 2
    root = Path.cwd().resolve()
    scope = Path(sys.argv[1])
    if not scope.is_dir():
        print(f"error: source scope not found: {scope}", file=sys.stderr)
        return 2

    findings: list[str] = []
    try:
        for path in sorted(scope.rglob("*")):
            if not path.is_file() or path.suffix not in SOURCE_SUFFIXES:
                continue
            relative = path.resolve().relative_to(root)
            if EXCLUDED_PARTS.intersection(relative.parts):
                continue
            original_lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
            code_lines = strip_comments_and_literals("\n".join(original_lines)).splitlines()
            for line_number, (original, code) in enumerate(zip(original_lines, code_lines), start=1):
                if FORBIDDEN.search(code) and not allowed(relative, code):
                    findings.append(f"{relative}:{line_number}:{original}")
    except (OSError, ValueError) as error:
        print(f"error: determinism scan failed: {error}", file=sys.stderr)
        return 2

    for finding in findings:
        print(finding)
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
