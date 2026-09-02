#!/usr/bin/env python3
"""Find forbidden nondeterministic calls in C++ code, excluding comments and literals."""

from __future__ import annotations

import re
import sys
from dataclasses import dataclass, field
from pathlib import Path


SOURCE_SUFFIXES = {".cc", ".h", ".icc"}
RAW_STRING = re.compile(r'(?:u8|u|U|L)?R"([^ ()\\\t\r\n]{0,16})\(')
CPP_TOKEN = re.compile(r"[A-Za-z_]\w*|::|->|[^\s]")
RESULT_FILTERS_ALLOW = re.compile(
    r"(?:startTime = time\(nullptr\);|time_t t = time\(nullptr\);)"
)
RANDOM_DEVICE = ("std", "random_device")
AMBIENT_CLOCKS = {
    ("std", "chrono", "system_clock"),
    ("std", "chrono", "steady_clock"),
    ("std", "chrono", "high_resolution_clock"),
}
GROUPING_PREFIX_KEYWORDS = {"case", "co_return", "co_yield", "return", "throw"}


@dataclass(frozen=True)
class Token:
    text: str
    line: int


@dataclass
class Scope:
    aliases: dict[str, tuple[str, ...]] = field(default_factory=dict)
    using_namespaces: list[tuple[str, ...]] = field(default_factory=list)


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


def is_name_start(tokens: list[Token], index: int) -> bool:
    if not is_identifier(tokens[index]) or index == 0:
        return is_identifier(tokens[index])
    if tokens[index - 1].text in {".", "->"}:
        return False
    return tokens[index - 1].text != "::" or index < 2 or not is_identifier(tokens[index - 2])


def qualified_name(tokens: list[Token], index: int) -> tuple[tuple[str, ...], int] | None:
    if index < len(tokens) and tokens[index].text == "::":
        index += 1
    if index >= len(tokens) or not is_identifier(tokens[index]):
        return None
    parts = [tokens[index].text]
    index += 1
    while index + 1 < len(tokens) and tokens[index].text == "::" and is_identifier(tokens[index + 1]):
        parts.append(tokens[index + 1].text)
        index += 2
    return tuple(parts), index


def visible_alias(scopes: list[Scope], name: str) -> tuple[str, ...] | None:
    for scope in reversed(scopes):
        if name in scope.aliases:
            return scope.aliases[name]
    return None


def canonical_names(name: tuple[str, ...], scopes: list[Scope]) -> set[tuple[str, ...]]:
    if not name:
        return set()
    if name[0] == "std":
        return {name}
    alias = visible_alias(scopes, name[0])
    if alias is not None:
        return {alias + name[1:]}
    return {
        namespace + name
        for scope in scopes
        for namespace in scope.using_namespaces
    }


def is_tracked_name(name: tuple[str, ...]) -> bool:
    return name == ("std",) or name == RANDOM_DEVICE or name[:2] == ("std", "chrono")


def unique_tracked_name(name: tuple[str, ...], scopes: list[Scope]) -> tuple[str, ...] | None:
    names = {candidate for candidate in canonical_names(name, scopes) if is_tracked_name(candidate)}
    return next(iter(names)) if len(names) == 1 else None


def statement_end(tokens: list[Token], index: int) -> int | None:
    while index < len(tokens) and tokens[index].text != ";":
        index += 1
    return index if index < len(tokens) else None


def parse_namespace_alias(tokens: list[Token], index: int, scopes: list[Scope]) -> int | None:
    if (
        index + 3 >= len(tokens)
        or tokens[index].text != "namespace"
        or not is_identifier(tokens[index + 1])
        or tokens[index + 2].text != "="
    ):
        return None
    parsed = qualified_name(tokens, index + 3)
    if parsed is None:
        return None
    name, end = parsed
    if end >= len(tokens) or tokens[end].text != ";":
        return None
    target = unique_tracked_name(name, scopes)
    if target is not None:
        scopes[-1].aliases[tokens[index + 1].text] = target
    return end + 1


def parse_using(tokens: list[Token], index: int, scopes: list[Scope], lines: set[int]) -> int | None:
    if tokens[index].text != "using":
        return None
    semicolon = statement_end(tokens, index + 1)
    if semicolon is None:
        return None

    if index + 2 < semicolon and tokens[index + 1].text == "namespace":
        parsed = qualified_name(tokens, index + 2)
        if parsed is not None and parsed[1] == semicolon:
            target = unique_tracked_name(parsed[0], scopes)
            if target is not None:
                scopes[-1].using_namespaces.append(target)
    elif index + 2 < semicolon and is_identifier(tokens[index + 1]) and tokens[index + 2].text == "=":
        parsed = qualified_name(tokens, index + 3)
        if parsed is not None:
            target = unique_tracked_name(parsed[0], scopes)
            if target is not None:
                scopes[-1].aliases[tokens[index + 1].text] = target
                if target == RANDOM_DEVICE:
                    lines.add(tokens[parsed[1] - 1].line)
    else:
        parsed = qualified_name(tokens, index + 1)
        if parsed is not None and parsed[1] == semicolon:
            target = unique_tracked_name(parsed[0], scopes)
            if target is not None:
                scopes[-1].aliases[parsed[0][-1]] = target
                if target == RANDOM_DEVICE:
                    lines.add(tokens[parsed[1] - 1].line)
    return semicolon + 1


def parse_typedef(tokens: list[Token], index: int, scopes: list[Scope], lines: set[int]) -> int | None:
    if tokens[index].text != "typedef":
        return None
    semicolon = statement_end(tokens, index + 1)
    if semicolon is None:
        return None
    parsed = qualified_name(tokens, index + 1)
    if parsed is not None and semicolon > parsed[1] and is_identifier(tokens[semicolon - 1]):
        target = unique_tracked_name(parsed[0], scopes)
        if target is not None:
            scopes[-1].aliases[tokens[semicolon - 1].text] = target
            if target == RANDOM_DEVICE:
                lines.add(tokens[parsed[1] - 1].line)
    return semicolon + 1


def grouping_parentheses_before(tokens: list[Token], index: int) -> int:
    count = 0
    cursor = index - 1
    if cursor >= 0 and tokens[cursor].text == "::" and (cursor == 0 or not is_identifier(tokens[cursor - 1])):
        cursor -= 1
    while cursor >= 0 and tokens[cursor].text == "(":
        count += 1
        cursor -= 1
    if count == 0 or cursor < 0:
        return count
    previous = tokens[cursor]
    is_call = is_identifier(previous) and previous.text not in GROUPING_PREFIX_KEYWORDS
    return count - 1 if is_call or previous.text in {")", "]", "}", ">"} else count


def constructed_clock_now_line(
    tokens: list[Token],
    name_start: int,
    name_end: int,
    candidates: set[tuple[str, ...]],
) -> int | None:
    if not any(candidate in AMBIENT_CLOCKS for candidate in candidates) or name_end + 1 >= len(tokens):
        return None
    if (tokens[name_end].text, tokens[name_end + 1].text) not in {("{", "}"), ("(", ")")}:
        return None

    cursor = name_end + 2
    for _ in range(grouping_parentheses_before(tokens, name_start)):
        if cursor >= len(tokens) or tokens[cursor].text != ")":
            return None
        cursor += 1
    if cursor + 2 >= len(tokens):
        return None
    if tokens[cursor].text == "." and tokens[cursor + 1].text == "now" and tokens[cursor + 2].text == "(":
        return tokens[cursor + 1].line
    return None


def forbidden_lines(code: str) -> set[int]:
    tokens = cpp_tokens(code)
    lines: set[int] = set()
    scopes = [Scope()]
    index = 0
    while index < len(tokens):
        next_index = parse_namespace_alias(tokens, index, scopes)
        if next_index is None:
            next_index = parse_using(tokens, index, scopes, lines)
        if next_index is None:
            next_index = parse_typedef(tokens, index, scopes, lines)
        if next_index is not None:
            index = next_index
            continue

        token = tokens[index]
        if token.text == "{":
            scopes.append(Scope())
        elif token.text == "}" and len(scopes) > 1:
            scopes.pop()
        elif is_name_start(tokens, index):
            parsed = qualified_name(tokens, index)
            assert parsed is not None
            name, end = parsed
            candidates = canonical_names(name, scopes)
            if RANDOM_DEVICE in candidates:
                lines.add(token.line)
            if end < len(tokens) and tokens[end].text == "(" and name[-1] in {"rand", "time"}:
                if len(name) == 1 or ("std", name[-1]) in candidates:
                    lines.add(tokens[end - 1].line)
            if end < len(tokens) and tokens[end].text == "(" and name[-1] == "now":
                if any(candidate[:-1] in AMBIENT_CLOCKS for candidate in candidates):
                    lines.add(tokens[end - 1].line)
            object_now_line = constructed_clock_now_line(tokens, index, end, candidates)
            if object_now_line is not None:
                lines.add(object_now_line)
            index = end
            continue
        index += 1
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
