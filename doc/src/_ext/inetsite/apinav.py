"""The navigation of the NED reference.

The type pages are not in the toctree (see conf.py), so the theme shows neither the navigation nor the table
of contents on them. The navigation tree of the reference (packages, namespaces and the categories of types,
e.g. modules, with their types) is written once per version to api/navtree.json and rendered into the sidebar
by _static/apinav.js; the table of contents of a type page is added here.
"""

import json
import os
import re
import xml.etree.ElementTree as ElementTree

from docutils import nodes
from sphinx.environment.adapters.toctree import TocTree
from sphinx_immaterial import nav_adapt

from . import config
from .ned import API_DIR

NAV_FILE = 'navtree.json'


def label_anchor(prefix, name):
    return nodes.make_id(f'neddoc-{prefix}-{name}')


def html_path(filename):
    return os.path.splitext(filename)[0] + '.html'


def read_compounds(path):
    """Returns (name, filename, kind) of the types in an opp_docgen tag file; the kind (e.g. simple, network,
    packet) is None from an opp_docgen that does not record it."""
    root = ElementTree.parse(path).getroot()
    return [(c.findtext('name'), c.findtext('filename'), c.get('typekind')) for c in root.iter('compound')]


def ned_packages(path):
    packages = {}
    for name, filename, _ in read_compounds(path):
        package, _, short = name.rpartition('.')
        packages.setdefault(package, []).append([short, html_path(filename)])
    return packages


def msg_namespaces(path):
    namespaces = {}
    for name, filename, _ in read_compounds(path):
        # The file name has the namespace and the (possibly nested) class name; the name has only the latter.
        stem = os.path.splitext(os.path.basename(filename))[0]
        namespace = '::'.join(stem[len('msg-'):].split('--')[:-len(name.split('::'))])
        namespaces.setdefault(namespace, []).append([name, html_path(filename)])
    return namespaces


def groups(items, page, prefix, default_label='(default)'):
    result = []
    for name in sorted(items, key=str.lower):
        label = name or default_label
        anchor = label_anchor(prefix, name or 'default')
        result.append({'title': label, 'url': f'{page}#{anchor}',
                       'children': [{'title': t, 'url': u} for t, u in sorted(items[name], key=lambda i: i[0].lower())]})
    return result


# The category pages of opp_docgen: the tag file and the kinds of the types each lists (None: all).
CATEGORIES = {
    'networks': ('nedtags.xml', {'network'}),
    'modules': ('nedtags.xml', {'simple', 'compound'}),
    'module-interfaces': ('nedtags.xml', {'moduleinterface'}),
    'channels': ('nedtags.xml', {'channel'}),
    'channel-interfaces': ('nedtags.xml', {'channelinterface'}),
    'message-definitions': ('msgtags.xml', None),
}


def category(path, kinds):
    """The types of the given kinds, by their short names, or their full names where these are ambiguous."""
    types = [(name.rpartition('.')[2], name, html_path(filename)) for name, filename, kind in read_compounds(path)
             if kinds is None or kind in kinds]
    shorts = [short for short, _, _ in types]
    return [{'title': short if shorts.count(short) == 1 else name, 'url': url}
            for short, name, url in sorted(types, key=lambda t: (t[0].lower(), t[1]))]


def file_pages(env):
    """The file pages, by the paths in their titles (e.g. "NED File src/inet/...")."""
    prefix = f'{API_DIR}/files/'
    pages = [(re.sub(r'^\w+ File ', '', env.titles[d].astext()) if d in env.titles else d,
              os.path.relpath(d, API_DIR) + '.html')
             for d in env.found_docs if d.startswith(prefix)]
    return [{'title': title, 'url': url} for title, url in sorted(pages, key=lambda p: p[0].lower())]


def navigation_tree(app):
    src = os.path.join(app.srcdir, API_DIR)
    env = app.env
    trees = {
        f'{API_DIR}/packages': groups(ned_packages(os.path.join(src, 'nedtags.xml')), 'packages.html', 'packages'),
        f'{API_DIR}/namespaces': groups(msg_namespaces(os.path.join(src, 'msgtags.xml')), 'namespaces.html',
                                        'namespaces'),
        f'{API_DIR}/files': file_pages(env),
    }
    # The categories list each type again, so that only the packages and namespaces open to the current page.
    flat = {f'{API_DIR}/files'}
    for name, (tags, kinds) in CATEGORIES.items():
        trees[f'{API_DIR}/{name}'] = category(os.path.join(src, tags), kinds)
        flat.add(f'{API_DIR}/{name}')
    entries = []
    for docname in env.toctree_includes.get(f'{API_DIR}/index', []):
        entry = {'title': env.titles[docname].astext() if docname in env.titles else docname,
                 'url': os.path.relpath(docname, API_DIR) + '.html'}
        if trees.get(docname):
            entry['children'] = trees[docname]
            if docname in flat:
                entry['follow'] = False
        entries.append(entry)
    return entries


def write_navigation(app, exception):
    if exception is not None or app.builder.format != 'html' or not config.current().has_part('api'):
        return
    with open(os.path.join(app.outdir, API_DIR, NAV_FILE), 'w') as f:
        json.dump(navigation_tree(app), f, separators=(',', ':'))


def add_local_toc(app, pagename, templatename, context, doctree):
    page = context.get('page')
    if not pagename.startswith(API_DIR + '/') or doctree is None or not isinstance(page, dict) or page.get('toc'):
        return
    toc = nav_adapt._get_mkdocs_toc(TocTree(app.env).get_toc_for(pagename, app.builder), app.builder)
    if len(toc) == 1:
        context['config']['mdx_configs']['toc']['title'] = toc[0].title
        toc = toc[0].children
    page['toc'] = toc


def setup(app):
    cfg = config.current()
    if not cfg.has_part('api'):
        return
    app.add_js_file('apinav.js', defer='defer')
    app.connect('html-page-context', add_local_toc, priority=900)
    app.connect('build-finished', write_navigation)
