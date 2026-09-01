#!/usr/bin/env python3
"""Find forbidden nondeterministic calls in C++ code, excluding comments and literals."""

from __future__ import annotations

import re
import sys
from dataclasses import dataclass
from pathlib import Path


SOURCE_SUFFIXES = {".cc", ".h", ".icc"}
RAW_STRING = re.compile(r'(?:u8|u|U|L)?R"([^ ()\\\t\r\n]{0,16})\(')
CPP_TOKEN = re.compile(r"[A-Za-z_]\w*|::|->|[^\s]")
RESULT_FILTERS_ALLOW = re.compile(
    r"(?:startTime = time\(nullptr\);|time_t t = time\(nullptr\);)"
)


@dataclass(frozen=True)
class Token:
    text: str
    line: int


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


def cpp_tokens(code: str) -> list[Token]:
    tokens: list[Token] = []
    line = 1
    previous = 0
    for match in CPP_TOKEN.finditer(code):
        line += code.count("\n", previous, match.start())
        tokens.append(Token(match.group(0), line))
        previous = match.start()
    return tokens


def is_identifier(token: Token) -> bool:
    return token.text[0].isalpha() or token.text[0] == "_"


def is_root_std(tokens: list[Token], index: int) -> bool:
    if tokens[index].text != "std" or index == 0:
        return tokens[index].text == "std"
    previous = tokens[index - 1].text
    if previous in {".", "->"}:
        return False
    if previous != "::":
        return True
    return index < 2 or not is_identifier(tokens[index - 2])


def forbidden_lines(code: str) -> set[int]:
    tokens = cpp_tokens(code)
    lines: set[int] = set()
    for index, token in enumerate(tokens):
        if token.text == "std" and is_root_std(tokens, index) and index + 2 < len(tokens):
            is_random_device = tokens[index + 2].text == "random_device"
            is_chrono_scope = (
                tokens[index + 2].text == "chrono"
                and index + 3 < len(tokens)
                and tokens[index + 3].text == "::"
            )
            if tokens[index + 1].text == "::" and (is_random_device or is_chrono_scope):
                lines.add(token.line)

        if token.text not in {"rand", "time"} or index + 1 >= len(tokens) or tokens[index + 1].text != "(":
            continue
        if index == 0 or tokens[index - 1].text not in {".", "->", "::"}:
            lines.add(token.line)
        elif tokens[index - 1].text == "::":
            if index < 2 or not is_identifier(tokens[index - 2]):
                lines.add(token.line)
            elif tokens[index - 2].text == "std" and is_root_std(tokens, index - 2):
                lines.add(token.line)
    return lines


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
            original_text = path.read_text(encoding="utf-8", errors="replace")
            original_lines = original_text.splitlines()
            code_lines = strip_comments_and_literals(original_text).splitlines()
            for line_number in sorted(forbidden_lines("\n".join(code_lines))):
                original = original_lines[line_number - 1]
                code = code_lines[line_number - 1]
                if not allowed(relative, code):
                    findings.append(f"{relative}:{line_number}:{original}")
    except (OSError, ValueError) as error:
        print(f"error: determinism scan failed: {error}", file=sys.stderr)
        return 2

    for finding in findings:
        print(finding)
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
