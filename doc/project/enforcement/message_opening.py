# Reads the opening of a commit body or a pull request description, and says when it starts with
# an action and names no problem.
#
# The first of the reviewer's questions in doc/project/rule/pull-request.md is "which problem does
# the change solve?". A word list cannot judge an answer, but it can see the clear case: a first
# paragraph that starts with an imperative action ("Add X. Rename Y.") and contains no word that a
# statement of a problem or a reason uses. Across 924 commit bodies on master, 1.9 % have that form;
# a reason-word test alone flags about 20 %, many of them good messages. So only the combination
# gives a note.
#
# Used by check-commits.sh (standard input: a commit body) and by check_pr_description.py (import).
# The opening skips reference lines such as "Fixes #12." and dependency sections, and a list that
# comes first counts as the opening.
# Output on standard input use: "action-first<TAB><first sentence>", or nothing.
import re
import sys

REASON = re.compile(r"""\b(because|since|so\s+that|in\s+order\s+to|otherwise|instead|without|unless|
 needs?|needed|requires?|required|must|should|cannot|can't|could\s+not|unable|
 fails?|failed|failing|failure|wrong|incorrect|invalid|missing|lacks?|broken|breaks?|crash(es|ed)?|
 errors?|assert(s|ion|ions)?|bugs?|defects?|problems?|issues?|symptoms?|regressions?|mismatch(es)?|
 violat\w*|exceed\w*|overrun\w*|leak\w*|stale|inconsistent\w*|duplicat\w*|unused|dead|obsolete|
 clauses?|rfc\s*\d+|ieee|standard|requirements?|R-[A-Z]+|spec\w*|
 why|reason\w*|avoid\w*|prevent\w*|allows?|enables?|lets|so\s+the|so\s+a|so\s+it|
 only|never|no\s+longer|still|yet|already|hides?|hidden|confus\w*|unclear|slow\w*|expensive|costs?)\b""",
                    re.I | re.X)

IMPERATIVE = re.compile(r"^(add|remove|delete|change|use|make|move|rename|replace|update|introduce|"
                        r"implement|refactor|extract|split|merge|drop|bump|clean|cleanup|convert|switch|"
                        r"set|apply|enable|disable|expose|keep|let|give|record|define|create)\b", re.I)

TRAILER = re.compile(r"^(Change|Plan|Fixes|Closes|Refs|Co-Authored-By|Signed-off-by|Reviewed-by):")

# A short paragraph that only points at other work, such as "Fixes #1194." or "Depends on #1269.
# Merge #1269 first.", comes before the opening and is not the opening.
REFERENCE = re.compile(r"^(fixe[sd]|close[sd]|resolve[sd]|refs?|related\s+to|see(\s+also)?|depends\s+on|"
                       r"supersedes|replaces|stacked\s+on|follows|follow-up\s+to|part\s+of|merge|requires)\b.*#\d+",
                       re.I)
REFERENCE_WORDS = 25
# A section that only names the work this change builds on also comes before the opening.
PREAMBLE_HEADING = re.compile(r"^#+\s*(dependenc\w*|depends|stacked|prerequisites?|related|references?)\b", re.I)
LIST_ITEM = re.compile(r"^([-*+]|\d+[.)])\s+")
# A rule line or an HTML element (a badge, an image) is markup, not text.
MARKUP = re.compile(r"^(-{3,}|\*{3,}|_{3,}|<[a-zA-Z/!])")


def blocks(text):
    """The blocks of a text in order, as (kind, text) with kind 'prose', 'list', 'code', 'table' or
    'heading'. HTML comments, trailers and quotes are left out."""
    text = re.sub(r"<!--.*?-->", "", text, flags=re.S)
    out, kind, lines, fenced = [], None, [], False

    def flush():
        nonlocal kind, lines
        if lines:
            out.append((kind, " ".join(lines)))
        kind, lines = None, []

    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith("```"):
            if not fenced:
                flush()
                out.append(("code", ""))
            fenced = not fenced
        elif fenced:
            continue
        elif not stripped or TRAILER.match(stripped) or stripped.startswith(">") or MARKUP.match(stripped):
            flush()
        elif stripped.startswith("#"):
            flush()
            out.append(("heading", stripped))
        else:
            item = LIST_ITEM.match(stripped)
            found = "table" if stripped.startswith("|") else "list" if item else kind or "prose"
            if found != kind:
                flush()
                kind = found
            lines.append(LIST_ITEM.sub("", stripped) if item else stripped)
    flush()
    return out


def is_preamble(kind, block):
    """A reference paragraph, which comes before the opening."""
    return kind == "prose" and bool(REFERENCE.match(block)) and len(block.split()) <= REFERENCE_WORDS


def opening(text):
    """The opening as (kind, text), kind 'prose' or 'list', or (None, "") when the text has none.
    Reference paragraphs and dependency sections are skipped, and so are code blocks and tables."""
    preamble = False
    for kind, block in blocks(text):
        if kind == "heading":
            preamble = bool(PREAMBLE_HEADING.match(block))
        elif not preamble and kind in ("prose", "list") and not is_preamble(kind, block):
            return kind, block
    return None, ""


def first_paragraph(text):
    """The opening text: the first prose paragraph, or a list that comes first."""
    return opening(text)[1]


def action_first(paragraph):
    """The first sentence when the paragraph starts with an action and names no problem, else None."""
    if not paragraph or not IMPERATIVE.match(paragraph) or REASON.search(paragraph):
        return None
    return re.split(r"(?<=[.:;])\s", paragraph, maxsplit=1)[0]


if __name__ == "__main__":
    sentence = action_first(first_paragraph(sys.stdin.read()))
    if sentence:
        print(f"action-first\t{sentence[:100]}")
