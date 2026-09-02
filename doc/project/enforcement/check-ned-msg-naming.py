#!/usr/bin/env python3
"""Check the mechanical NED/MSG subset of doc/project/rule/naming.md."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


RECONCILE = " [reconcile: doc/project/audit/naming-exceptions.md]"
PASCAL_RE = re.compile(r"^[A-Z][A-Za-z0-9]*$")
CAMEL_RE = re.compile(r"^[a-z][A-Za-z0-9]*$")
LOWER_RE = re.compile(r"^[a-z][a-z0-9]*$")
NED_TYPE_RE = re.compile(r"^\s*(simple|module|moduleinterface|network|channel|channelinterface)\s+([A-Za-z_]\w*)\b")
NED_PARAMETER_RE = re.compile(
    r"^\s*(?:volatile\s+)?"
    r"(?:bool|int|double|string|xml|object|quantity|typename|"
    r"[A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)*)\s+([A-Za-z_]\w*)\b",
    re.DOTALL,
)
NED_GATE_RE = re.compile(r"^\s*(input|output|inout)\s+([A-Za-z_]\w*)\b", re.DOTALL)
MSG_TYPE_RE = re.compile(r"^\s*(class|struct|packet|message|enum)\s+([A-Za-z_]\w*)\b(.*)$")
MSG_NAMESPACE_RE = re.compile(
    r"^\s*namespace\s+([A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)*)\s*;\s*$"
)
HUNK_RE = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")
CPP_BLOCK_RE = re.compile(r"\bcplusplus(?:\s*\([^)]*\))?\s*\{\{")


@dataclass(frozen=True, order=True)
class Finding:
    path: str
    line: int
    rule: str
    message: str

    def render(self) -> str:
        return f"{self.path}:{self.line}: {self.rule}: {self.message}{RECONCILE}"


@dataclass(frozen=True)
class Target:
    selected_lines: frozenset[int] | None
    structural_rules: frozenset[str] = frozenset()


@dataclass(frozen=True)
class NedSectionStatement:
    kind: str
    text: str
    line_by_offset: tuple[int, ...]

    @property
    def contributing_lines(self) -> frozenset[int]:
        return frozenset(
            line for character, line in zip(self.text, self.line_by_offset)
            if not character.isspace()
        )


@dataclass(frozen=True)
class MsgTypeDeclaration:
    name_line: int
    definition_line: int
    kind: str
    name: str


@dataclass(frozen=True)
class MsgNamespaceDeclaration:
    line: int
    name: str | None


class UsageError(Exception):
    pass


def run_git(root: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", *args], cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    if result.returncode != 0:
        raise UsageError(result.stderr.strip() or f"git {' '.join(args)} failed")
    return result.stdout


def repository_root() -> Path:
    cwd = Path.cwd().resolve()
    result = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise UsageError("run from an INET Git repository root")
    root = Path(result.stdout.strip()).resolve()
    if root != cwd or not (root / "src/inet").is_dir():
        raise UsageError("run from the INET repository root containing src/inet")
    return root


def git_paths(root: Path, args: list[str]) -> set[Path]:
    output = run_git(root, *args)
    return {Path(line) for line in output.splitlines() if line}


def diff_arguments(staged: bool, base: str | None) -> list[str]:
    if base is not None:
        return ["diff", base, "HEAD"]
    if staged:
        return ["diff", "--cached"]
    return ["diff", "HEAD"]


def diff_added_lines(root: Path, path: Path, staged: bool, base: str | None) -> set[int]:
    args = ["diff", "--unified=0", "--no-color"]
    if base is not None:
        args.extend([base, "HEAD"])
    elif staged:
        args.append("--cached")
    else:
        args.append("HEAD")
    args.extend(["--", path.as_posix()])
    lines: set[int] = set()
    for line in run_git(root, *args).splitlines():
        match = HUNK_RE.match(line)
        if match:
            start = int(match.group(1))
            count = int(match.group(2) or "1")
            lines.update(range(start, start + count))
    return lines


def naming_lines(path: Path, text: str) -> list[str]:
    lines = strip_comments(text.splitlines())
    return mask_msg_cplusplus(lines) if path.suffix == ".msg" else lines


def declaration_delimiter(text: str) -> tuple[str, int] | None:
    brace = text.find("{")
    semicolon = text.find(";")
    if brace != -1 and (semicolon == -1 or brace < semicolon):
        return "definition", brace
    if semicolon != -1:
        return "forward", semicolon
    return None


def msg_type_declarations(lines: list[str]) -> list[MsgTypeDeclaration]:
    declarations: list[MsgTypeDeclaration] = []
    pending: tuple[int, str, str] | None = None
    for number, line in enumerate(lines, 1):
        if pending is not None:
            delimiter = declaration_delimiter(line)
            if delimiter is None:
                continue
            if delimiter[0] == "definition":
                name_line, kind, name = pending
                declarations.append(MsgTypeDeclaration(name_line, number, kind, name))
            pending = None
            continue
        match = MSG_TYPE_RE.match(line)
        if match:
            kind, name, rest = match.groups()
            delimiter = declaration_delimiter(rest)
            if delimiter is None:
                pending = (number, kind, name)
            elif delimiter[0] == "definition":
                declarations.append(MsgTypeDeclaration(number, number, kind, name))
    return declarations


def msg_namespace_declarations(lines: list[str]) -> list[MsgNamespaceDeclaration]:
    declarations: list[MsgNamespaceDeclaration] = []
    for number, line in enumerate(lines, 1):
        if not re.match(r"^\s*namespace\b", line):
            continue
        match = MSG_NAMESPACE_RE.match(line)
        name = re.sub(r"\s*::\s*", "::", match.group(1)) if match else None
        declarations.append(MsgNamespaceDeclaration(number, name))
    return declarations


def valid_msg_namespace(name: str | None) -> bool:
    if name is None:
        return False
    parts = name.split("::")
    return len(parts) <= 2 and parts[0] == "inet" and all(LOWER_RE.fullmatch(part) for part in parts)


def declared_type_names(path: Path, lines: list[str]) -> set[str]:
    if path.suffix == ".ned":
        names: set[str] = set()
        for line in lines:
            match = NED_TYPE_RE.match(line)
            if match:
                names.add(match.group(2))
        return names
    return {declaration.name for declaration in msg_type_declarations(lines)}


def ned_package_name(lines: list[str]) -> str | None:
    package: str | None = None
    for line in lines:
        match = re.match(r"^\s*package\s+([A-Za-z0-9_.]+)\s*;", line)
        if match:
            package = match.group(1)
    return package


def structural_transitions(root: Path, path: Path, staged: bool, base: str | None) -> set[str]:
    old_revision = base if base is not None else "HEAD"
    old_lines = naming_lines(path, run_git(root, "show", f"{old_revision}:{path.as_posix()}"))
    if base is not None:
        new_text = run_git(root, "show", f"HEAD:{path.as_posix()}")
    elif staged:
        new_text = run_git(root, "show", f":{path.as_posix()}")
    else:
        new_text = (root / path).read_text(encoding="utf-8")
    new_lines = naming_lines(path, new_text)
    rules: set[str] = set()
    if path.suffix == ".ned":
        old_package = ned_package_name(old_lines)
        new_package = ned_package_name(new_lines)
        if old_package is not None and new_package is None:
            rules.add("package")
    elif path.suffix == ".msg":
        old_namespaces = Counter(
            declaration.name
            for declaration in msg_namespace_declarations(old_lines)
            if valid_msg_namespace(declaration.name)
        )
        new_namespaces = Counter(
            declaration.name
            for declaration in msg_namespace_declarations(new_lines)
            if valid_msg_namespace(declaration.name)
        )
        if old_namespaces - new_namespaces:
            rules.add("namespace")
    if path.name != "package.ned":
        old_names = declared_type_names(path, old_lines)
        new_names = declared_type_names(path, new_lines)
        if path.stem in old_names and path.stem not in new_names:
            rules.add("file-type")
    return rules


def diff_targets(root: Path, staged: bool, base: str | None, scope: Path) -> dict[Path, Target]:
    common = diff_arguments(staged, base)
    pathspec = scope.as_posix()
    full = git_paths(root, [*common, "--name-only", "--diff-filter=ACR", "--", pathspec])
    modified = git_paths(root, [*common, "--name-only", "--diff-filter=M", "--", pathspec])
    if not staged and base is None:
        full |= git_paths(root, ["ls-files", "--others", "--exclude-standard", "--", pathspec])

    targets: dict[Path, Target] = {}
    for path in full:
        target_exists = staged or base is not None or (root / path).is_file()
        if path.suffix in {".ned", ".msg"} and target_exists:
            targets[path] = Target(None)
    for path in modified - full:
        target_exists = staged or base is not None or (root / path).is_file()
        if path.suffix in {".ned", ".msg"} and target_exists:
            selected = diff_added_lines(root, path, staged, base)
            structural_rules = structural_transitions(root, path, staged, base)
            if selected or structural_rules:
                targets[path] = Target(frozenset(selected), frozenset(structural_rules))
    return targets


def source_path(root: Path, name: str) -> tuple[Path, Path]:
    candidate = Path(name)
    absolute = candidate.resolve() if candidate.is_absolute() else (root / candidate).resolve()
    try:
        relative = absolute.relative_to(root)
    except ValueError as exc:
        raise UsageError(f"path is outside the repository: {name}") from exc
    if relative.parts[:2] != ("src", "inet"):
        raise UsageError(f"path is outside src/inet: {name}")
    return absolute, relative


def source_scope(root: Path, name: str) -> Path:
    absolute, relative = source_path(root, name)
    if not absolute.is_dir():
        raise UsageError(f"scope directory not found: {name}")
    return relative


def explicit_targets(root: Path, names: list[str]) -> dict[Path, Target]:
    targets: dict[Path, Target] = {}
    for name in names:
        absolute, relative = source_path(root, name)
        if absolute.is_dir():
            candidates = sorted(
                path for path in absolute.rglob("*")
                if path.is_file() and path.suffix in {".ned", ".msg"}
            )
        elif absolute.is_file():
            if relative.suffix not in {".ned", ".msg"}:
                raise UsageError(f"unsupported file type: {name}; expected .ned or .msg")
            candidates = [absolute]
        else:
            raise UsageError(f"file or directory not found: {name}")
        for candidate in candidates:
            resolved, candidate_relative = source_path(root, str(candidate))
            if not resolved.is_file():
                raise UsageError(f"file not found: {candidate}")
            targets[candidate_relative] = Target(None)
    return targets


def strip_comments(lines: list[str]) -> list[str]:
    cleaned: list[str] = []
    in_block = False
    for line in lines:
        output: list[str] = []
        index = 0
        in_string = False
        while index < len(line):
            if in_block:
                end = line.find("*/", index)
                if end == -1:
                    index = len(line)
                    continue
                in_block = False
                index = end + 2
                continue
            if not in_string and line.startswith("/*", index):
                in_block = True
                index += 2
                continue
            if not in_string and line.startswith("//", index):
                break
            char = line[index]
            if char == '"':
                backslash_run = 0
                cursor = index - 1
                while cursor >= 0 and line[cursor] == "\\":
                    backslash_run += 1
                    cursor -= 1
                if backslash_run % 2 == 0:
                    in_string = not in_string
            output.append(char)
            index += 1
        cleaned.append("".join(output))
    return cleaned


def mask_msg_cplusplus(lines: list[str]) -> list[str]:
    # The OMNeT++ MSG lexer defines the first literal }} as the block delimiter.
    masked: list[str] = []
    in_cplusplus = False
    for line in lines:
        opener = CPP_BLOCK_RE.search(line) if not in_cplusplus else None
        if opener:
            in_cplusplus = True
            masked.append("")
            if "}}" in line[opener.end():]:
                in_cplusplus = False
            continue
        if in_cplusplus:
            masked.append("")
            if "}}" in line:
                in_cplusplus = False
            continue
        masked.append(line)
    return masked


def expected_ned_package(path: Path) -> str:
    relative_dir = path.parent.relative_to(Path("src/inet"))
    parts = ["inet", *relative_dir.parts]
    return ".".join(parts)


def selected(line: int, selected_lines: frozenset[int] | None) -> bool:
    return selected_lines is None or line in selected_lines


def consume_ned_text(
    buffered_text: str,
    buffered_lines: tuple[int, ...],
    text: str,
    line: int,
) -> tuple[list[tuple[str, tuple[int, ...]]], str, tuple[int, ...]]:
    separator = "\n" if buffered_text else ""
    combined = buffered_text + separator + text
    line_by_offset = buffered_lines + (line,) * (len(separator) + len(text))
    statements: list[tuple[str, tuple[int, ...]]] = []
    grouping: list[str] = []
    pairs = {"(": ")", "[": "]", "{": "}"}
    in_string = False
    escaped = False
    start = 0
    for index, character in enumerate(combined):
        if in_string:
            if character == '"' and not escaped:
                in_string = False
            escaped = character == "\\" and not escaped
            if character != "\\":
                escaped = False
            continue
        if character == '"':
            in_string = True
        elif character in pairs:
            grouping.append(pairs[character])
        elif grouping and character == grouping[-1]:
            grouping.pop()
        elif character == ";" and not grouping:
            end = index + 1
            statements.append((combined[start:end], line_by_offset[start:end]))
            start = end
    remainder = combined[start:]
    remainder_lines = line_by_offset[start:]
    if not remainder.strip():
        return statements, "", ()
    return statements, remainder, remainder_lines


def ned_section_statements(lines: list[str]) -> list[NedSectionStatement]:
    statements: list[NedSectionStatement] = []
    section: tuple[str, int] | None = None
    buffered_text = ""
    buffered_lines: tuple[int, ...] = ()
    for number, line in enumerate(lines, 1):
        stripped = line.strip()
        indent = len(line) - len(line.lstrip())
        section_match = re.match(r"^\s*(parameters|gates)\s*:\s*$", line)
        if section_match:
            section = (section_match.group(1), indent)
            buffered_text = ""
            buffered_lines = ()
            continue
        if section is None:
            continue
        kind, section_indent = section
        if (stripped == "}" and indent <= section_indent) or (
            re.match(r"^[A-Za-z][A-Za-z0-9]*\s*:\s*$", stripped)
            and indent <= section_indent
        ):
            section = None
            buffered_text = ""
            buffered_lines = ()
            continue
        completed, buffered_text, buffered_lines = consume_ned_text(
            buffered_text, buffered_lines, line, number
        )
        statements.extend(
            NedSectionStatement(kind, text, statement_lines)
            for text, statement_lines in completed
        )
    return statements


def ned_statement_selected(
    statement: NedSectionStatement,
    selected_lines: frozenset[int] | None,
) -> bool:
    return selected_lines is None or bool(statement.contributing_lines & selected_lines)


def ned_match_line(statement: NedSectionStatement, match: re.Match[str], group: int) -> int:
    return statement.line_by_offset[match.start(group)]


def add(findings: list[Finding], path: Path, line: int, rule: str, message: str) -> None:
    findings.append(Finding(path.as_posix(), line, rule, message))


def check_ned(
    path: Path,
    lines: list[str],
    selected_lines: frozenset[int] | None,
    structural_rules: frozenset[str],
) -> list[Finding]:
    findings: list[Finding] = []
    package: tuple[int, str] | None = None
    declarations: list[tuple[int, str]] = []

    for number, line in enumerate(lines, 1):
        package_match = re.match(r"^\s*package\s+([A-Za-z0-9_.]+)\s*;", line)
        if package_match:
            package = (number, package_match.group(1))

        type_match = NED_TYPE_RE.match(line)
        if type_match:
            name = type_match.group(2)
            declarations.append((number, name))
            if selected(number, selected_lines) and not PASCAL_RE.fullmatch(name):
                add(findings, path, number, "NR-NED-TYPE", f"NED type '{name}' must be PascalCase")

        if selected(number, selected_lines):
            for property_name in ("signal", "statistic"):
                for match in re.finditer(rf"@{property_name}\[([^\]]+)\]", line):
                    name = match.group(1).removesuffix("*")
                    if not CAMEL_RE.fullmatch(name):
                        add(findings, path, number, "NR-NED-SIGNAL", f"{property_name} '{match.group(1)}' must be camelCase (a terminal '*' is allowed)")

    for statement in ned_section_statements(lines):
        if not ned_statement_selected(statement, selected_lines):
            continue
        if statement.kind == "parameters":
            match = NED_PARAMETER_RE.match(statement.text)
            if match and not CAMEL_RE.fullmatch(match.group(1)):
                line = ned_match_line(statement, match, 1)
                add(
                    findings,
                    path,
                    line,
                    "NR-NED-PARAM",
                    f"parameter '{match.group(1)}' must be camelCase",
                )
        elif statement.kind == "gates":
            match = NED_GATE_RE.match(statement.text)
            if match:
                direction, name = match.groups()
                line = ned_match_line(statement, match, 2)
                if not CAMEL_RE.fullmatch(name):
                    add(findings, path, line, "NR-NED-GATE", f"gate '{name}' must be camelCase")
                if direction == "input" and name != "in" and not name.endswith("In"):
                    add(findings, path, line, "NR-NED-GATE", f"input gate '{name}' must be 'in' or end in 'In'")
                if direction == "output" and name != "out" and not name.endswith("Out"):
                    add(findings, path, line, "NR-NED-GATE", f"output gate '{name}' must be 'out' or end in 'Out'")

    if package is not None:
        line, name = package
        if selected(line, selected_lines) or "package" in structural_rules:
            if any(not LOWER_RE.fullmatch(part) for part in name.split(".")):
                add(findings, path, line, "NR-PKG", f"package '{name}' must use lowercase run-together segments")
            expected = expected_ned_package(path)
            if name != expected:
                add(findings, path, line, "NR-PKG", f"package '{name}' must match path package '{expected}'")
    elif selected_lines is None or "package" in structural_rules:
        add(findings, path, 1, "NR-PKG", f"file must declare package '{expected_ned_package(path)}'")

    if path.name != "package.ned" and (selected_lines is None or "file-type" in structural_rules):
        names = {name for _, name in declarations}
        if path.stem not in names:
            line = declarations[0][0] if declarations else 1
            add(findings, path, line, "NR-PKG", f"file stem '{path.stem}' must match a defined NED type")
    return findings


def msg_field_name(line: str) -> str | None:
    stripped = line.strip()
    if not stripped.endswith(";") or stripped.startswith("@"):
        return None
    without_properties = stripped[:-1].split("@", 1)[0].split("=", 1)[0].strip()
    without_array = re.sub(r"\[[^\]]*\]\s*$", "", without_properties).strip()
    match = re.search(r"([A-Za-z_]\w*)\s*$", without_array)
    if not match:
        return None
    prefix = without_array[: match.start()].strip()
    if not prefix or prefix.endswith(("{", ",")):
        return None
    return match.group(1)


def consume_msg_text(
    buffered_text: str,
    buffered_selected: bool,
    text: str,
    text_selected: bool,
) -> tuple[list[tuple[str, bool]], str, bool]:
    statements: list[tuple[str, bool]] = []
    statement: list[str] = []
    statement_selected = False
    in_string = False
    escaped = False
    segments = ((buffered_text, buffered_selected), ("\n" + text, text_selected))
    for segment, segment_selected in segments:
        for char in segment:
            if char == '"' and not escaped:
                in_string = not in_string
            if char == ";" and not in_string:
                statements.append(("".join(statement) + ";", statement_selected or segment_selected))
                statement = []
                statement_selected = False
            else:
                statement.append(char)
                statement_selected = statement_selected or segment_selected
            escaped = char == "\\" and not escaped
            if char != "\\":
                escaped = False
    remainder = "".join(statement)
    if not remainder.strip():
        return statements, "", False
    return statements, remainder, statement_selected


def msg_body_fragment(text: str, initial_depth: int) -> tuple[str, int]:
    depth = initial_depth
    in_string = False
    escaped = False
    for index, char in enumerate(text):
        if char == '"' and not escaped:
            in_string = not in_string
        elif not in_string:
            if char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth <= 0:
                    return text[:index], 0
        escaped = char == "\\" and not escaped
        if char != "\\":
            escaped = False
    return text, depth


def add_msg_field_findings(
    findings: list[Finding], path: Path, number: int, statements: list[tuple[str, bool]]
) -> None:
    for statement, statement_selected in statements:
        if not statement_selected:
            continue
        name = msg_field_name(statement)
        if name is None:
            continue
        if not CAMEL_RE.fullmatch(name):
            add(findings, path, number, "NR-MSG-FIELD", f"MSG field '{name}' must be camelCase")


def check_msg(
    path: Path,
    lines: list[str],
    selected_lines: frozenset[int] | None,
    structural_rules: frozenset[str],
) -> list[Finding]:
    findings: list[Finding] = []
    declarations = msg_type_declarations(lines)
    namespaces = msg_namespace_declarations(lines)
    check_all_namespaces = selected_lines is None or "namespace" in structural_rules

    for namespace in namespaces:
        if not (check_all_namespaces or selected(namespace.line, selected_lines)):
            continue
        if namespace.name is None:
            add(findings, path, namespace.line, "NR-MSG-FIELD", "MSG namespace declaration is malformed")
            continue
        parts = namespace.name.split("::")
        if any(not LOWER_RE.fullmatch(part) for part in parts):
            add(
                findings,
                path,
                namespace.line,
                "NR-PKG",
                f"namespace '{namespace.name}' must use lowercase run-together segments",
            )
        elif parts[0] != "inet" or len(parts) > 2:
            add(
                findings,
                path,
                namespace.line,
                "NR-PKG",
                f"namespace '{namespace.name}' must be 'inet' or an 'inet::<protocol>' sub-namespace",
            )

    namespace_properties = [
        number for number, line in enumerate(lines, 1) if re.search(r"@namespace\s*\(", line)
    ]
    for number in namespace_properties:
        if check_all_namespaces or selected(number, selected_lines):
            add(
                findings,
                path,
                number,
                "NR-MSG-FIELD",
                "MSG files must use a namespace statement, not an @namespace property",
            )

    if not namespaces and check_all_namespaces:
        add(findings, path, 1, "NR-MSG-FIELD", "MSG file must declare an 'inet' namespace")

    if namespaces and declarations:
        first_namespace_line = namespaces[0].line
        first_declaration = declarations[0]
        opening_check_active = check_all_namespaces or selected(first_namespace_line, selected_lines)
        opening_check_active = opening_check_active or selected(first_declaration.name_line, selected_lines)
        opening_check_active = opening_check_active or selected(first_declaration.definition_line, selected_lines)
        if opening_check_active and first_namespace_line > first_declaration.name_line:
            add(
                findings,
                path,
                first_declaration.name_line,
                "NR-MSG-FIELD",
                "MSG namespace statement must precede the first type declaration",
            )

    for declaration in declarations:
        declaration_selected = selected(declaration.name_line, selected_lines) or selected(
            declaration.definition_line, selected_lines
        )
        if declaration_selected and not PASCAL_RE.fullmatch(declaration.name):
            add(
                findings,
                path,
                declaration.name_line,
                "NR-MSG-TYPE",
                f"MSG {declaration.kind} '{declaration.name}' must be PascalCase",
            )

    context_kind: str | None = None
    pending_kind: str | None = None
    brace_depth = 0
    field_buffer = ""
    field_buffer_selected = False

    for number, line in enumerate(lines, 1):
        if context_kind is not None:
            body, new_depth = msg_body_fragment(line, brace_depth)
            statements, field_buffer, field_buffer_selected = consume_msg_text(
                field_buffer, field_buffer_selected, body, selected(number, selected_lines)
            )
            if context_kind in {"class", "struct", "packet", "message"}:
                add_msg_field_findings(findings, path, number, statements)
            brace_depth = new_depth
            if brace_depth <= 0:
                context_kind = None
                brace_depth = 0
                field_buffer = ""
                field_buffer_selected = False
            continue

        entered_kind: str | None = None
        open_index = -1
        if pending_kind is not None:
            delimiter = declaration_delimiter(line)
            if delimiter is None:
                continue
            kind, index = delimiter
            if kind == "definition":
                entered_kind = pending_kind
                open_index = index
            pending_kind = None
            if entered_kind is None:
                continue
        else:
            type_match = MSG_TYPE_RE.match(line)
            if type_match is None:
                continue
            kind, _, rest = type_match.groups()
            delimiter = declaration_delimiter(rest)
            if delimiter is None:
                pending_kind = kind
                continue
            delimiter_kind, index = delimiter
            if delimiter_kind == "forward":
                continue
            entered_kind = kind
            open_index = type_match.start(3) + index

        body, brace_depth = msg_body_fragment(line[open_index + 1:], 1)
        statements, field_buffer, field_buffer_selected = consume_msg_text(
            "", False, body, selected(number, selected_lines)
        )
        if entered_kind in {"class", "struct", "packet", "message"}:
            add_msg_field_findings(findings, path, number, statements)
        if brace_depth > 0:
            context_kind = entered_kind
        else:
            brace_depth = 0
            field_buffer = ""
            field_buffer_selected = False

    if selected_lines is None or "file-type" in structural_rules:
        names = {declaration.name for declaration in declarations}
        if path.stem not in names:
            line = declarations[0].name_line if declarations else 1
            add(findings, path, line, "NR-PKG", f"file stem '{path.stem}' must match a defined MSG type")
    return findings


def check_file(
    root: Path,
    path: Path,
    target: Target,
    staged: bool,
    revision: str | None,
) -> list[Finding]:
    if revision is not None:
        text = run_git(root, "show", f"{revision}:{path.as_posix()}")
    elif staged:
        text = run_git(root, "show", f":{path.as_posix()}")
    else:
        text = (root / path).read_text(encoding="utf-8")
    lines = naming_lines(path, text)
    if path.suffix == ".msg":
        return check_msg(path, lines, target.selected_lines, target.structural_rules)
    return check_ned(path, lines, target.selected_lines, target.structural_rules)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--diff", action="store_true", help="check working-tree additions against HEAD (default)")
    mode.add_argument("--staged", action="store_true", help="check staged additions")
    mode.add_argument("--base", metavar="REF", help="check committed additions from merge-base(REF, HEAD) through HEAD")
    parser.add_argument("--scope", default="src/inet", help="restrict a diff mode to this src/inet subtree")
    parser.add_argument("paths", nargs="*", help="explicit NED/MSG files or directories to scan completely")
    args = parser.parse_args()
    if args.paths and (args.diff or args.staged or args.base is not None or args.scope != "src/inet"):
        parser.error("explicit paths cannot be combined with a diff mode or --scope")
    return args


def main() -> int:
    try:
        args = parse_args()
        root = repository_root()
        base = None
        revision = None
        if args.paths:
            targets = explicit_targets(root, args.paths)
        else:
            scope = source_scope(root, args.scope)
            if args.base is not None:
                base = run_git(root, "merge-base", args.base, "HEAD").strip()
                if not base:
                    raise UsageError(f"'{args.base}' has no merge base with HEAD")
                revision = "HEAD"
            targets = diff_targets(root, args.staged, base, scope)
        findings: list[Finding] = []
        for path in sorted(targets):
            findings.extend(check_file(root, path, targets[path], args.staged and not args.paths, revision))
        findings.sort()
        for finding in findings:
            print(finding.render())
        if findings:
            print(f"FAIL: {len(findings)} mechanical naming candidate(s). Reconcile each with naming-exceptions.md.")
            return 1
        print(f"PASS: checked {len(targets)} applicable NED/MSG file(s); no mechanical naming candidates.")
        return 0
    except UsageError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
