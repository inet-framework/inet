"""
    The ``include-files`` directive: include the reStructuredText files that a glob pattern matches,
    in the order of their file names.

    Example, in ``migration-guide/index.rst``::

        .. include-files:: development/*.rst

    The pattern is relative to the folder of the page. When no file matches, the directive shows
    one sentence. Each included file is a dependency of the page, and an incremental build also
    reads the page again when a file is added to or removed from the folder. The included files
    must be excluded from the source files of Sphinx (``exclude_patterns``), because each one is a
    part of a page and not a page.

    Rule RR-MIGRATE-FILE in doc/project/rule/release.md says why the migration guide uses it.
"""
import glob
import os

from docutils import nodes
from docutils.parsers.rst import Directive

EMPTY = "No change of this version needs a migration step."


def matches(pattern):
    return sorted(glob.glob(pattern), key=os.path.basename)


class IncludeFiles(Directive):
    required_arguments = 1
    has_content = False

    def run(self):
        env = self.state.document.settings.env
        folder = os.path.dirname(self.state.document.current_source)
        pattern = os.path.normpath(os.path.join(folder, self.arguments[0]))
        paths = matches(pattern)
        env.include_files.setdefault(env.docname, {})[pattern] = paths
        if not paths:
            return [nodes.paragraph(text=EMPTY)]
        # insert_input() puts its lines before the lines of an earlier call, so the last file goes first.
        for path in reversed(paths):
            env.note_dependency(path)
            with open(path, encoding="utf-8") as file:
                lines = file.read().splitlines()
            self.state_machine.insert_input(lines + [""], path)
        return []


def init(app):
    if not hasattr(app.env, "include_files"):
        app.env.include_files = {}


def purge(app, env, docname):
    env.include_files.pop(docname, None)


def merge(app, env, docnames, other):
    env.include_files.update({d: other.include_files[d] for d in docnames if d in other.include_files})


def outdated(app, env, added, changed, removed):
    """The pages whose patterns match another set of files than at the last read."""
    return [docname for docname, patterns in env.include_files.items()
            if any(matches(pattern) != paths for pattern, paths in patterns.items())]


def setup(app):
    app.add_directive("include-files", IncludeFiles)
    app.connect("builder-inited", init)
    app.connect("env-purge-doc", purge)
    app.connect("env-merge-info", merge)
    app.connect("env-get-outdated", outdated)
    return {"version": "1.0", "env_version": 1, "parallel_read_safe": True, "parallel_write_safe": True}
