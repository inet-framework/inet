#!/usr/bin/env python3
"""Builds inet.omnetpp.org: the website pages and the documentation of INET versions.

Runs inside the doc-builder image (see doc/src/build, which starts the container). By default it builds the
website pages (doc/src/site, present on the website branch only) and the documentation of the INET tree it is part
of, as the version in versions.toml that has no ref (dev). Other versions are built from git with --versions.

Output layout:

    <out>/                      website pages
    <out>/docs/<label>/         documentation of one INET version (with --pdf, also inet-<manual>.pdf)
    <out>/docs/versions.json    version switcher data
    <out>/docs/latest/...       redirects to the latest version
    <out>/docs/<manual>/...     redirects from the pre-versioning URLs to the latest version
    <out>/docs/inet-<manual>.pdf  copies of the latest version's PDFs, at their pre-versioning URLs
"""

import argparse
import hashlib
import html
import json
import os
import posixpath
import re
import shutil
import signal
import subprocess
import sys
import tomllib
import zlib
from xml.etree import ElementTree

from sphinx.search import js_index

SRC = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TREE = os.path.dirname(os.path.dirname(SRC))
SITE = os.path.join(SRC, 'site')
SITE_URL = 'https://inet.omnetpp.org'
PARTS = ('users-guide', 'developers-guide', 'migration-guide', 'python-api', 'api')
# Built once, from the tree of the site (like the version without a ref), and published in docs/ without a
# version; they link to the documentation of that version.
SHARED_PARTS = ('showcases', 'tutorials')
MANUALS = ('users-guide', 'developers-guide', 'migration-guide')
PDF_MANUALS = ('users-guide', 'developers-guide', 'migration-guide')
# Parts built by a Sphinx run of their own and merged into the output of the others: Sphinx's parallel reader
# sends the whole environment back from every batch of documents, and AutoAPI's object tree in it would make
# the reading of the thousands of NED reference pages several times slower. For the same reason, these runs
# are not parallel.
SEPARATE_PARTS = ('python-api',)
PART_SOURCES = {
    'users-guide': 'doc/src/users-guide',
    'developers-guide': 'doc/src/developers-guide',
    'migration-guide': 'doc/src/migration-guide',
    'showcases': 'showcases',
    'tutorials': 'tutorials',
}
# The start page of each part; AutoAPI's is that of the top-level package.
PART_ROOTS = {part: f'{part}/index' for part in PARTS + SHARED_PARTS} | {'python-api': 'python-api/inet/index'}
PART_TITLES = {
    'users-guide': "User's Guide",
    'developers-guide': "Developer's Guide",
    'migration-guide': 'Migration Guide',
    'showcases': 'Showcases',
    'tutorials': 'Tutorials',
    'api': 'NED Reference',
    'python-api': 'Python API',
}
DOCGEN_EXCLUDES = 'examples,tests,showcases,tutorials'
# What --sample builds of the parts that are too large to work on the site with: the NED reference of these
# source directories, and these showcases and tutorials. Paths that a version lacks are left out.
SAMPLE = {
    'api': ('src/inet/applications/ethernet', 'src/inet/node/ethernet'),
    'showcases': ('wireless/hiddennode', 'visualizer/canvas/spectrum', 'tsn/trafficshaping/creditbasedshaper',
                  'emulation/mininet'),
    'tutorials': ('configurator',),
}
# The files of the source root that the NED reference needs besides the sampled directories.
SAMPLE_API_FILES = ('src/inet/package.ned', 'src/inet/inet-index.ned', 'WHATSNEW')
SITE_REDIRECTS = {
    'index.php/index.html': 'index.html',
    'doc/INET/index.html': 'docs/{latest}/api/index.html',
    'inet-showcases/index.html': 'docs/showcases/index.html',
    'inet-tutorials/index.html': 'docs/tutorials/index.html',
}
# What the builds without a NED reference of their own need of the latest version's, kept with that version's
# build (see REFS_DIR in _ext/inetsite/config.py).
REFS_DIR = '.inetsite-refs'

CONFIG_FILES = ('conf.py', 'global.rst', '_ext', '_static', 'tools/build.py', 'tools/includefiles.py')
SIZE_LIMIT = 900 * 1024 * 1024
# The search index format of sphinx_immaterial, which lists the pages by URL instead of docname and filename.
SEARCH_INDEX_KEYS = {'docurls', 'titles', 'terms', 'titleterms', 'objects', 'objtypes', 'objnames', 'envversion',
                     'alltitles', 'indexentries'}
SITEMAP_NAMESPACE = 'http://www.sitemaps.org/schemas/sitemap/0.9'


class BuildError(Exception):
    pass


def log(message):
    print(f'build: {message}', file=sys.stderr, flush=True)


def run(cmd, **kwargs):
    log(' '.join(cmd))
    subprocess.run(cmd, check=True, **kwargs)


def git(repo, *args):
    return subprocess.run(['git', '-C', repo, *args], check=True, capture_output=True, text=True).stdout.strip()


def load_versions(path):
    with open(path, 'rb') as f:
        data = tomllib.load(f)
    versions = data.get('version', [])
    latest = [v['label'] for v in versions if v.get('latest')]
    if len(latest) != 1:
        raise BuildError(f'{path}: exactly one version must have latest = true')
    labels = [v['label'] for v in versions]
    if len(set(labels)) != len(labels) or 'latest' in labels:
        raise BuildError(f'{path}: version labels must be unique and must not be "latest"')
    if sum(1 for v in versions if not v.get('ref')) > 1:
        raise BuildError(f'{path}: at most one version can be without a ref (the tree being built)')
    for v in versions:
        unknown = set(v.get('parts', PARTS)) - set(PARTS)
        if unknown:
            raise BuildError(f'{path}: version {v["label"]}: unknown parts: {", ".join(sorted(unknown))}')
    return data, versions, latest[0]


def tree_hash(*paths):
    files = []
    for path in paths:
        if os.path.isfile(path):
            files.append(path)
            continue
        for dirpath, dirnames, filenames in os.walk(path):
            dirnames[:] = [d for d in dirnames if d != '__pycache__']
            files += [os.path.join(dirpath, name) for name in filenames]
    h = hashlib.sha256()
    for file in sorted(files):
        h.update(os.path.relpath(file, SRC).encode())
        with open(file, 'rb') as f:
            h.update(f.read())
    return h.hexdigest()


def git_tree_key(root):
    """Identifies the committed content of a clean git checkout, without the site pages (so that editing them
    does not rebuild the version), or returns None if the tree is not a clean checkout."""
    try:
        if os.path.realpath(git(root, 'rev-parse', '--show-toplevel')) != os.path.realpath(root):
            return None
        if git(root, 'status', '--porcelain', '--', '.', ':(exclude)doc/src/site'):
            return None
        entries = []
        for tree, skip in (('HEAD', 'doc'), ('HEAD:doc', 'src'), ('HEAD:doc/src', 'site')):
            entries += [line for line in git(root, 'ls-tree', tree).splitlines() if line.split('\t')[1] != skip]
    except (subprocess.CalledProcessError, FileNotFoundError):
        return None
    return hashlib.sha256('\n'.join(entries).encode()).hexdigest()


def directory_size(path):
    total = 0
    for dirpath, _, filenames in os.walk(path):
        for name in filenames:
            file = os.path.join(dirpath, name)
            if not os.path.islink(file):
                total += os.path.getsize(file)
    return total


def redirect_page(target_url, canonical_url):
    url = html.escape(target_url, quote=True)
    canonical = html.escape(canonical_url, quote=True)
    return ('<!DOCTYPE html>\n<html><head><meta charset="utf-8">\n'
            f'<link rel="canonical" href="{canonical}">\n'
            f'<meta http-equiv="refresh" content="0; url={url}">\n'
            f'<script>location.replace("{url}" + location.hash)</script>\n'
            f'</head><body><a href="{url}">{url}</a></body></html>\n')


def write_redirect(path, target_url, canonical_url=None):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w') as f:
        f.write(redirect_page(target_url, canonical_url or target_url))


def merge_output(outdir, other_outdir, parts):
    """Merges the output of a Sphinx run that built the given parts with the same root into outdir."""
    for part in parts:
        shutil.copytree(os.path.join(other_outdir, part), os.path.join(outdir, part))
    for name in os.listdir(other_outdir):
        source, target = os.path.join(other_outdir, name), os.path.join(outdir, name)
        if not os.path.exists(target):
            if os.path.isdir(source):
                shutil.copytree(source, target)
            else:
                shutil.copy(source, target)
    merge_search_index(os.path.join(outdir, 'searchindex.js'), os.path.join(other_outdir, 'searchindex.js'), parts)
    merge_inventory(os.path.join(outdir, 'objects.inv'), os.path.join(other_outdir, 'objects.inv'), parts)
    merge_sitemap(os.path.join(outdir, 'sitemap.xml'), os.path.join(other_outdir, 'sitemap.xml'))


def merge_search_index(path, other_path, parts):
    """Adds the pages of the given parts in the search index other_path to the search index path."""
    with open(path) as f:
        index = js_index.loads(f.read())
    with open(other_path) as f:
        other = js_index.loads(f.read())
    if set(index) != SEARCH_INDEX_KEYS or set(other) != SEARCH_INDEX_KEYS or index['envversion'] != other['envversion']:
        raise BuildError(f'cannot merge {other_path} into {path}: unknown search index format')
    prefixes = tuple(part + '/' for part in parts)
    sources = [(index, [True] * len(index['docurls'])),
               (other, [url.startswith(prefixes) for url in other['docurls']])]
    titles = {}
    for data, kept in sources:
        for url, title, keep in zip(data['docurls'], data['titles'], kept):
            if keep:
                if url in titles:
                    raise BuildError(f'cannot merge {other_path} into {path}: both have {url}')
                titles[url] = title
    urls = sorted(titles)
    position = {url: i for i, url in enumerate(urls)}
    merged = {
        'docurls': urls,
        'titles': [titles[url] for url in urls],
        'envversion': index['envversion'],
        'objtypes': {},
        'objnames': {},
        'objects': {},
        'alltitles': {},
        'indexentries': {},
    }
    objtype_ids = {}
    terms = {'terms': {}, 'titleterms': {}}
    for data, kept in sources:
        doc = [position[url] if keep else None for url, keep in zip(data['docurls'], kept)]
        objtype = {}
        for i, name in data['objtypes'].items():
            if name not in objtype_ids:
                objtype_ids[name] = str(len(objtype_ids))
                merged['objtypes'][objtype_ids[name]] = name
                merged['objnames'][objtype_ids[name]] = data['objnames'][i]
            objtype[int(i)] = int(objtype_ids[name])
        for key in terms:
            for term, ids in data[key].items():
                for i in ids if isinstance(ids, list) else [ids]:
                    if doc[i] is not None:
                        terms[key].setdefault(term, set()).add(doc[i])
        for prefix, entries in data['objects'].items():
            for i, objtype_id, *rest in entries:
                if doc[i] is not None:
                    merged['objects'].setdefault(prefix, []).append([doc[i], objtype[objtype_id], *rest])
        for key in ('alltitles', 'indexentries'):
            for name, entries in data[key].items():
                for i, *rest in entries:
                    if doc[i] is not None:
                        merged[key].setdefault(name, []).append([doc[i], *rest])
    for key, term_docs in terms.items():
        merged[key] = {term: min(ids) if len(ids) == 1 else sorted(ids) for term, ids in term_docs.items()}
    with open(path, 'w') as f:
        f.write(js_index.dumps(merged))


def merge_inventory(path, other_path, parts):
    """Adds the objects of the pages of the given parts in the Sphinx inventory other_path to the inventory path."""
    def read(file):
        with open(file, 'rb') as f:
            header = b''.join(f.readline() for _ in range(4))
            return header, zlib.decompress(f.read()).decode().splitlines()
    header, lines = read(path)
    _, other_lines = read(other_path)
    prefixes = tuple(part + '/' for part in parts)
    known = set(lines)
    for line in other_lines:
        match = re.match(r'(.+?)\s+(\S+)\s+(-?\d+)\s+?(\S*)\s+(.*)', line)
        if match and match[4].startswith(prefixes) and line not in known:
            lines.append(line)
    with open(path, 'wb') as f:
        f.write(header + zlib.compress(''.join(line + '\n' for line in lines).encode(), 9))


def merge_sitemap(path, other_path):
    ElementTree.register_namespace('', SITEMAP_NAMESPACE)
    tree = ElementTree.parse(path)
    root = tree.getroot()
    locs = {loc.text for loc in root.iter(f'{{{SITEMAP_NAMESPACE}}}loc')}
    for url in ElementTree.parse(other_path).getroot().iter(f'{{{SITEMAP_NAMESPACE}}}url'):
        if url.findtext(f'{{{SITEMAP_NAMESPACE}}}loc') not in locs:
            root.append(url)
    tree.write(path)


class Builder:
    def __init__(self, args):
        self.args = args
        self.cache = os.path.abspath(args.cache)
        self.out = os.path.abspath(args.out)
        self.baseurl = args.baseurl.rstrip('/')
        self.tree = os.path.abspath(args.inet) if args.inet else TREE
        self.data, self.versions, self.latest = load_versions(os.path.join(SRC, 'versions.toml'))
        known = {v['label']: v for v in self.versions}
        tree_version = next((v for v in self.versions if not v.get('ref')), {'label': 'dev'})
        self.dev_label = tree_version['label']
        self.only = None
        if args.only:
            self.only = [p for p in args.only.split(',') if p]
            unknown = set(self.only) - {'site', *PARTS, *SHARED_PARTS}
            if unknown:
                raise BuildError(f'--only: unknown parts: {", ".join(sorted(unknown))}')
        self.shared_parts = [p for p in SHARED_PARTS if self.only is None or p in self.only]
        unversioned = self.only is not None and not set(self.only) & set(PARTS)
        if unversioned and (args.versions or args.pdf):
            raise BuildError('--versions and --pdf need a versioned part in --only')
        if unversioned:
            self.versions = []
        elif args.versions == 'all':
            # What is published: the released versions (with a ref), without those that have none of the parts of
            # --only (e.g. one with the NED reference only).
            self.versions = [v for v in self.versions if v.get('ref') and self.parts_of(v)]
        elif args.versions is not None:
            wanted = [v for v in args.versions.split(',') if v]
            missing = [w for w in wanted if w not in known]
            if missing:
                raise BuildError(f'not in versions.toml: {", ".join(missing)}')
            self.versions = [known[w] for w in wanted]
        else:
            self.versions = [tree_version]
        built = [v['label'] for v in self.versions]
        # A build without the latest version, e.g. a local one, points the links and redirects to one it has.
        if built and self.latest not in built:
            self.latest = built[0]
        self.with_site = (self.only is None or 'site' in self.only) and os.path.isdir(SITE)
        if self.only is not None and 'site' in self.only and not self.with_site:
            raise BuildError(f'no site pages in {SITE} (they are on the website branch)')
        self.force = None if args.force is None else {f for f in args.force.split(',') if f}
        if args.pdf and not shutil.which('latexmk'):
            raise BuildError('--pdf needs the PDF toolchain (TeX Live): the -pdf variant of the doc-builder image')
        self.media_ref = None
        # Sample builds have their own cache entries, so switching between sample and full builds rebuilds neither.
        self.suffix = '-sample' if args.sample else ''
        self.config_hash = tree_hash(*(os.path.join(SRC, f) for f in CONFIG_FILES))

    def environment(self, **values):
        env = dict(os.environ)
        # The web fonts of the theme, downloaded on a cache miss. The doc-builder image comes with them; elsewhere
        # they are downloaded once and kept with the other cached build results.
        env.setdefault('SPHINX_IMMATERIAL_EXTERNAL_RESOURCE_CACHE_DIR', os.path.join(self.cache, 'sphinx-immaterial'))
        env.update({
            'INETSITE_BASEURL': self.baseurl,
            'INETSITE_LATEST': self.latest,
            'INETSITE_FETCH_FONTS': '0' if self.args.no_fonts else '1',
            'INETSITE_SAMPLE': '1' if self.args.sample else '0',
        })
        env.update({f'INETSITE_{k.upper()}': v for k, v in values.items()})
        return env

    def doctrees(self, name):
        """Returns the doctrees directory of a Sphinx run, emptied if the configuration files have changed: Sphinx
        sees a change of the configuration values, but not of the extensions (e.g. of the navigation)."""
        path = os.path.join(self.cache, 'doctrees', name)
        stamp = os.path.join(path, '.inetsite-config')
        if not os.path.exists(stamp) or open(stamp).read() != self.config_hash:
            shutil.rmtree(path, ignore_errors=True)
            os.makedirs(path)
            with open(stamp, 'w') as f:
                f.write(self.config_hash)
        return path

    def sphinx(self, srcdir, outdir, doctrees, env, jobs=None):
        cmd = ['sphinx-build', '-b', 'html', '-c', SRC, '-d', doctrees, '-j', jobs or self.args.jobs]
        if self.args.quiet:
            cmd.append('-q')
        cmd += [srcdir, outdir]
        run(cmd, env=env)

    def sphinx_pdf(self, srcdir, builddir, doctrees, env):
        cmd = ['sphinx-build', '-M', 'latexpdf', srcdir, builddir, '-c', SRC, '-d', doctrees]
        if self.args.quiet:
            cmd.append('-q')
            env = dict(env, LATEXMKOPTS='-silent')
        run(cmd, env=env)

    # -- sources --------------------------------------------------------------------------------------------

    def resolve(self, url, ref):
        """Returns (full ref name, commit sha) of a branch, tag or HEAD in a remote repository."""
        os.makedirs(self.cache, exist_ok=True)
        out = subprocess.run(['git', 'ls-remote', url, ref, f'{ref}^{{}}'], check=True, capture_output=True,
                             text=True, cwd=self.cache).stdout
        refs = dict(reversed(line.split('\t')) for line in out.splitlines())
        for name in (f'refs/heads/{ref}', f'refs/tags/{ref}^{{}}', f'refs/tags/{ref}', ref):
            if name in refs:
                return name.removesuffix('^{}'), refs[name]
        raise BuildError(f'{ref} not found in {url}')

    def export(self, url, ref, sha, name):
        """Exports the tree of a commit into the cache, fetching only that commit."""
        path = os.path.join(self.cache, 'src', name)
        stamp = os.path.join(path, '.inetsite-commit')
        if os.path.exists(stamp) and open(stamp).read() == sha:
            return path
        repo = os.path.join(self.cache, 'git', name + '.git')
        if not os.path.isdir(repo):
            run(['git', 'init', '--bare', '--quiet', repo])
        run(['git', '-C', repo, 'fetch', '--depth', '1', '--quiet', url, f'+{ref}:refs/inetsite/fetched'])
        fetched = git(repo, 'rev-parse', 'refs/inetsite/fetched^{commit}')
        if fetched != sha:
            raise BuildError(f'{url} {ref} moved during the build ({sha[:12]} -> {fetched[:12]}); run again')
        shutil.rmtree(path, ignore_errors=True)
        os.makedirs(path)
        archive = subprocess.Popen(['git', '-C', repo, 'archive', sha], stdout=subprocess.PIPE)
        subprocess.run(['tar', '-x', '-C', path], stdin=archive.stdout, check=True)
        if archive.wait() != 0:
            raise BuildError(f'git archive {sha} failed')
        with open(stamp, 'w') as f:
            f.write(sha)
        return path

    def media_commit(self):
        if self.media_ref is None:
            self.media_ref = self.resolve(self.data['media_repository'], 'HEAD')
        return self.media_ref

    def local_media(self, inet_root):
        """Returns the media checkout to use, or None if the media repository is to be exported from git."""
        if self.args.media:
            return os.path.abspath(self.args.media)
        link = os.path.join(inet_root, 'media') if inet_root else None
        if link and os.path.exists(link) and not (os.path.islink(link) and self.is_cached(os.readlink(link))):
            return link
        return None

    def link_media(self, inet_root, media):
        """Makes <inet_root>/media point to the media checkout (exporting it first if media is None)."""
        if media is None:
            ref, sha = self.media_commit()
            media = self.export(self.data['media_repository'], ref, sha, 'media')
        link = os.path.join(inet_root, 'media')
        if os.path.realpath(link) == os.path.realpath(media):
            return media
        if os.path.exists(link) and not os.path.islink(link):
            raise BuildError(f'--media: {link} is a directory; remove it or omit --media')
        if os.path.islink(link):
            os.remove(link)
        os.symlink(media, link)
        log(f'{link} -> {media}')
        return media

    def is_cached(self, path):
        return os.path.realpath(path).startswith(os.path.realpath(self.cache) + os.sep)

    # -- version builds -------------------------------------------------------------------------------------

    def parts_of(self, version):
        parts = version.get('parts', PARTS)
        return [p for p in PARTS if p in parts and (self.only is None or p in self.only)]

    def assemble_sources(self, label, inet_root, parts, index):
        """Assembles the sources of the given parts, with the root document index = (docname, text)."""
        work = os.path.join(self.cache, 'work', label)
        os.makedirs(work, exist_ok=True)
        for name in os.listdir(work):
            path = os.path.join(work, name)
            if os.path.islink(path) or os.path.isfile(path):
                os.remove(path)
            else:
                shutil.rmtree(path)
        for part in parts:
            if part in PART_SOURCES:
                source = os.path.join(inet_root, PART_SOURCES[part])
                if not os.path.isdir(source):
                    raise BuildError(f'{label}: {source} does not exist')
                if self.args.sample and part in SAMPLE:
                    self.sample_part(source, os.path.join(work, part), part)
                else:
                    os.symlink(source, os.path.join(work, part))
        if 'api' in parts:
            if self.args.sample:
                source, ned_path = self.sample_api_source(label, inet_root)
            else:
                source, ned_path = inet_root, None
            self.generate_api(source, os.path.join(work, 'api'), ned_path)
        docname, text = index
        with open(os.path.join(work, docname + '.rst'), 'w') as f:
            f.write(text)
        return work

    def sample_part(self, source, target, part):
        """Makes target a part with only the sampled documents of source, under an index page of its own."""
        entries = [e for e in SAMPLE[part] if os.path.isfile(os.path.join(source, e, 'doc', 'index.rst'))]
        for entry in entries:
            os.makedirs(os.path.dirname(os.path.join(target, entry)), exist_ok=True)
            os.symlink(os.path.join(source, entry), os.path.join(target, entry))
        title = PART_TITLES[part]
        lines = ['=' * len(title), title, '=' * len(title), '',
                 f'A sample of the {title.lower()} (built with ``doc/src/build --sample``).', '',
                 '.. toctree::', '   :maxdepth: 1', '']
        lines += [f'   {entry}/doc/index' for entry in entries]
        with open(os.path.join(target, 'index.rst'), 'w') as f:
            f.write('\n'.join(lines) + '\n')

    def sample_api_source(self, label, inet_root):
        """Copies the sampled NED and MSG sources into a project of their own, and the others into a library that
        resolves the types they use (e.g. the icons of submodules). Returns the project and the library's source
        root."""
        root = os.path.join(self.cache, 'sample', label, 'inet')
        library = os.path.join(self.cache, 'sample', label, 'library')
        for path in (root, library):
            shutil.rmtree(path, ignore_errors=True)
        os.makedirs(root)
        with open(os.path.join(root, '.nedfolders'), 'w') as f:
            f.write('src\n')
        for path in SAMPLE_API_FILES + SAMPLE['api']:
            source = os.path.join(inet_root, path)
            if os.path.isdir(source):
                shutil.copytree(source, os.path.join(root, path),
                                ignore=lambda d, names: [n for n in names if not n.endswith(('.ned', '.msg'))
                                                         and not os.path.isdir(os.path.join(d, n))])
            elif os.path.isfile(source):
                os.makedirs(os.path.dirname(os.path.join(root, path)), exist_ok=True)
                shutil.copy(source, os.path.join(root, path))
        # A type of both the project and the library would be taken from the library, and not documented.
        sampled = set(SAMPLE_API_FILES + SAMPLE['api'])
        for dirpath, dirnames, filenames in os.walk(os.path.join(inet_root, 'src')):
            rel = os.path.relpath(dirpath, inet_root)
            dirnames[:] = [d for d in dirnames if os.path.join(rel, d) not in sampled]
            for name in filenames:
                path = os.path.join(rel, name)
                if name.endswith(('.ned', '.msg')) and path not in sampled:
                    os.makedirs(os.path.join(library, rel), exist_ok=True)
                    shutil.copy(os.path.join(dirpath, name), os.path.join(library, path))
        return root, os.path.join(library, 'src')

    def generate_api(self, inet_root, outdir, ned_path=None):
        cmd = ['opp_docgen', '--format=rst', '-o', outdir, '-x', DOCGEN_EXCLUDES]
        if ned_path:
            cmd.append(f'--ned-path={ned_path}')
        run(cmd + [inet_root])
        index = os.path.join(outdir, 'index.rst')
        with open(index) as f:
            text = f.read()
        # Only the overview pages go into the navigation; the thousands of type and file pages are reached
        # through the listings (see suppress_warnings in conf.py).
        text = re.sub(r'^   (types|files)/.*\n', '', text, flags=re.M)
        with open(index, 'w') as f:
            f.write(text)

    def version_index(self, label, parts, version_parts):
        title = f'INET {label} Documentation'
        lines = [':hide-toc:', '', '=' * len(title), title, '=' * len(title), '', '.. inet-nav::', '']
        for part in version_parts:
            if part in parts:
                line = f'- :doc:`{PART_TITLES[part]} <{PART_ROOTS[part]}>`'
            else:
                line = f'- `{PART_TITLES[part]} <{PART_ROOTS[part]}.html>`__'
            if part in self.pdf_parts(version_parts):
                line += f' (`PDF <inet-{part}.pdf>`__)'
            lines.append(line)
        return '\n'.join(lines) + '\n'

    def pdf_parts(self, parts):
        return [p for p in parts if p in PDF_MANUALS] if self.args.pdf else []

    def build_pdfs(self, label, inet_root, parts, cached, env):
        pdf_parts = self.pdf_parts(parts)
        if not pdf_parts:
            return
        log(f'version {label}: building the PDFs of {", ".join(pdf_parts)}')
        work = self.assemble_sources(label + '-pdf', inet_root, pdf_parts,
                                     ('index', self.version_index(label, pdf_parts, pdf_parts)))
        builddir = os.path.join(self.cache, 'pdf', label)
        shutil.rmtree(builddir, ignore_errors=True)
        env = dict(env, INETSITE_PARTS=','.join(pdf_parts), INETSITE_VERSION_PARTS=','.join(pdf_parts),
                   INETSITE_LATEX='1')
        self.sphinx_pdf(work, builddir, self.doctrees(label + '-pdf'), env)
        for part in pdf_parts:
            shutil.copy(os.path.join(builddir, 'latex', f'inet-{part}.pdf'), cached)

    def version_key(self, content, parts, **extra):
        key = {'content': content, 'media': self.media_commit()[1], 'parts': parts, 'config': self.config_hash,
               'image': os.environ.get('INETSITE_IMAGE', ''), 'baseurl': self.baseurl, 'pdf': self.args.pdf,
               'sample': self.args.sample, 'latest': self.latest, **extra}
        return hashlib.sha256(json.dumps(key, sort_keys=True).encode()).hexdigest()

    def forced(self, label):
        return self.force is not None and (self.force == {'all'} or label in self.force)

    def api_refs(self, what):
        """Returns the build of the latest version, of this run or an earlier one, whose NED reference the builds
        without one of their own link to, or '' if there is none."""
        path = os.path.join(self.cache, 'out', self.latest + self.suffix)
        if os.path.isfile(os.path.join(path, REFS_DIR, 'nedtags.xml')):
            return path
        log(f'no NED reference of version {self.latest}: the NED types of {what} are not linked')
        return ''

    def build_version(self, version):
        label = version['label']
        parts = self.parts_of(version)
        if not parts:
            raise BuildError(f'version {label}: nothing to build')
        api = '' if 'api' in parts else self.api_refs(f'version {label}')
        cached = os.path.join(self.cache, 'out', label + self.suffix)
        stamp = os.path.join(cached, '.inetsite-key')
        from_tree = not version.get('ref')
        if from_tree:
            content = git_tree_key(self.tree)
        else:
            ref, content = self.resolve(self.data['inet_repository'], version['ref'])
        media = self.local_media(self.tree if from_tree else None)
        # A local media checkout can change without a commit, so only builds from git exports are cached.
        refs = tree_hash(os.path.join(api, REFS_DIR)) if api else ''
        key = self.version_key(content, parts, refs=refs) if content and media is None else None
        if key and not self.forced(label) and os.path.exists(stamp) and open(stamp).read() == key:
            log(f'version {label}: unchanged, reusing the cached build')
            self.copy_version(label, cached)
            return
        if from_tree:
            inet_root = self.tree
        else:
            inet_root = self.export(self.data['inet_repository'], ref, content, 'inet-' + label)
        media_root = self.link_media(inet_root, media)
        log(f'version {label}: building {", ".join(parts)} from {inet_root}')
        shutil.rmtree(cached, ignore_errors=True)
        runs = [[p for p in parts if p not in SEPARATE_PARTS]] + [[p] for p in parts if p in SEPARATE_PARTS]
        runs = [run_parts for run_parts in runs if run_parts]
        for i, run_parts in enumerate(runs):
            name = label + self.suffix if i == 0 else f'{label}{self.suffix}-{"-".join(run_parts)}'
            index = ('index', self.version_index(label, run_parts, parts))
            work = self.assemble_sources(name, inet_root, run_parts, index)
            outdir = cached if i == 0 else os.path.join(self.cache, 'parts', name)
            shutil.rmtree(outdir, ignore_errors=True)
            env = self.environment(mode='version', version=label, parts=','.join(run_parts),
                                   version_parts=','.join(parts), inet_root=inet_root, media_root=media_root,
                                   pdfs=','.join(self.pdf_parts(parts)), api_version=self.latest, api_docs=api)
            self.sphinx(work, outdir, self.doctrees(name), env,
                        jobs='1' if run_parts[0] in SEPARATE_PARTS else None)
            if 'api' in run_parts:
                os.makedirs(os.path.join(cached, REFS_DIR))
                for tag_file in ('nedtags.xml', 'msgtags.xml'):
                    shutil.copy(os.path.join(work, 'api', tag_file), os.path.join(cached, REFS_DIR))
            if i > 0:
                merge_output(cached, outdir, run_parts)
        self.build_pdfs(label, inet_root, parts, cached, env)
        if key:
            with open(stamp, 'w') as f:
                f.write(key)
        self.copy_version(label, cached)

    def copy_version(self, label, cached):
        target = os.path.join(self.out, 'docs', label)
        shutil.rmtree(target, ignore_errors=True)
        shutil.copytree(cached, target, ignore=shutil.ignore_patterns('.inetsite-key', '.buildinfo', REFS_DIR))

    # -- shared parts ---------------------------------------------------------------------------------------

    def shared_index(self, parts):
        title = 'INET Showcases and Tutorials'
        lines = [':hide-toc:', '', '=' * len(title), title, '=' * len(title), '', '.. inet-nav::', '']
        lines += [f'- :doc:`{PART_TITLES[part]} <{PART_ROOTS[part]}>`' for part in parts]
        return '\n'.join(lines) + '\n'

    def build_shared(self):
        parts = self.shared_parts
        name = '_shared' + self.suffix
        # The manuals to link to: those of the version without a ref if this run builds it, otherwise those of the
        # latest version, of this run or an earlier one. An earlier build of the former is not published.
        manuals_label = self.dev_label if any(v['label'] == self.dev_label for v in self.versions) else self.latest
        manuals = os.path.join(self.cache, 'out', manuals_label + self.suffix)
        if not os.path.isfile(os.path.join(manuals, 'objects.inv')):
            log(f'no build of version {manuals_label}: the links of {", ".join(parts)} to its manuals are left out')
            manuals = ''
        cached = os.path.join(self.cache, 'out', name)
        stamp = os.path.join(cached, '.inetsite-key')
        content = git_tree_key(self.tree)
        media = self.local_media(self.tree)
        api = self.api_refs(', '.join(parts))
        linked = (([os.path.join(manuals, 'objects.inv')] if manuals else [])
                  + ([os.path.join(api, REFS_DIR)] if api else []))
        refs = tree_hash(*linked) if linked else ''
        key = self.version_key(content, parts, refs=refs, manuals=manuals_label) if content and media is None else None
        if key and not self.forced('shared') and os.path.exists(stamp) and open(stamp).read() == key:
            log(f'{", ".join(parts)}: unchanged, reusing the cached build')
        else:
            media_root = self.link_media(self.tree, media)
            log(f'building {", ".join(parts)} from {self.tree}')
            work = self.assemble_sources(name, self.tree, parts, ('overview', self.shared_index(parts)))
            shutil.rmtree(cached, ignore_errors=True)
            env = self.environment(mode='shared', parts=','.join(parts), inet_root=self.tree, media_root=media_root,
                                   manuals_version=manuals_label, manuals_docs=manuals, api_version=self.latest,
                                   api_docs=api)
            self.sphinx(work, cached, self.doctrees(name), env)
            if key:
                with open(stamp, 'w') as f:
                    f.write(key)
        shutil.copytree(cached, os.path.join(self.out, 'docs'), dirs_exist_ok=True,
                        ignore=shutil.ignore_patterns('.inetsite-key', '.buildinfo'))

    # -- site -----------------------------------------------------------------------------------------------

    def build_site(self):
        log('building the site pages')
        self.sphinx(SITE, self.out, self.doctrees('_site'), self.environment(mode='site'))
        with open(os.path.join(self.out, '.nojekyll'), 'w'):
            pass
        for path, target in SITE_REDIRECTS.items():
            self.write_site_redirect(path, target.format(latest=self.latest))

    def write_site_redirect(self, path, target):
        """Writes a redirect from path to target, both relative to the site root."""
        write_redirect(os.path.join(self.out, path), posixpath.relpath(target, posixpath.dirname(path) or '.'),
                       f'{self.baseurl}/{target}')

    def write_versions_json(self, labels):
        entries = [{'version': label, 'title': label, 'aliases': ['latest'] if label == self.latest else []}
                   for label in labels]
        os.makedirs(os.path.join(self.out, 'docs'), exist_ok=True)
        with open(os.path.join(self.out, 'docs', 'versions.json'), 'w') as f:
            json.dump(entries, f, indent=2)

    def write_latest_redirects(self):
        latest_dir = os.path.join(self.out, 'docs', self.latest)
        if not os.path.isdir(latest_dir):
            return
        count = 0
        for dirpath, dirnames, filenames in os.walk(latest_dir):
            dirnames[:] = [d for d in dirnames if not d.startswith('_')]
            for name in filenames:
                if not name.endswith('.html'):
                    continue
                rel = os.path.relpath(os.path.join(dirpath, name), latest_dir).replace(os.sep, '/')
                target = f'docs/{self.latest}/{rel}'
                self.write_site_redirect(f'docs/latest/{rel}', target)
                count += 1
                if rel.split('/', 1)[0] in MANUALS or rel == 'index.html':
                    self.write_site_redirect(f'docs/{rel}', target)
                    count += 1
        log(f'wrote {count} redirects to version {self.latest}')
        # A PDF cannot redirect, so the old URLs get copies.
        for name in os.listdir(latest_dir):
            if name.endswith('.pdf'):
                shutil.copy(os.path.join(latest_dir, name), os.path.join(self.out, 'docs', name))

    def write_legacy_api_redirects(self):
        """Redirects for the NED reference formerly published at doc.omnetpp.org/inet/api-current/, whose
        neddoc/ pages had the same paths as the api/ pages now."""
        api_dir = os.path.join(self.out, 'docs', self.latest, 'api')
        if not os.path.isdir(api_dir):
            raise BuildError(f'--legacy-api-redirects needs the API of the latest version ({self.latest})')
        target = os.path.abspath(self.args.legacy_api_redirects)
        shutil.rmtree(target, ignore_errors=True)
        api_url = f'{self.baseurl}/docs/latest/api'
        write_redirect(os.path.join(target, 'index.html'), f'{api_url}/index.html')
        for dirpath, dirnames, filenames in os.walk(api_dir):
            dirnames[:] = [d for d in dirnames if not d.startswith('_')]
            for name in filenames:
                if name.endswith('.html'):
                    rel = os.path.relpath(os.path.join(dirpath, name), api_dir).replace(os.sep, '/')
                    write_redirect(os.path.join(target, 'neddoc', rel), f'{api_url}/{rel}')
        log(f'wrote the legacy API redirects into {target}')

    def check_size(self):
        size = directory_size(self.out)
        log(f'site size: {size / 1024 / 1024:.0f} MB')
        if size > SIZE_LIMIT and not self.args.no_size_limit:
            raise BuildError(f'the site is {size / 1024 / 1024:.0f} MB, more than the {SIZE_LIMIT / 1024 / 1024:.0f} MB '
                             'limit; remove old versions from versions.toml')

    def build(self):
        if not self.args.keep_out:
            shutil.rmtree(self.out, ignore_errors=True)
        os.makedirs(self.out, exist_ok=True)
        os.makedirs(self.cache, exist_ok=True)
        if self.with_site:
            self.build_site()
        elif self.only is None:
            log(f'no site pages in {SITE} (they are on the website branch); building the documentation only')
        # The latest version first, whose NED reference the builds without one of their own link to.
        for version in sorted(self.versions, key=lambda v: v['label'] != self.latest):
            self.build_version(version)
        if self.shared_parts:
            self.build_shared()
        if self.versions:
            self.write_versions_json([v['label'] for v in self.versions])
            self.write_latest_redirects()
        if self.args.legacy_api_redirects:
            self.write_legacy_api_redirects()
        self.check_size()
        if self.args.serve:
            self.serve()

    def serve(self):
        log(f'serving {self.out} at http://localhost:{self.args.port}/ (Ctrl-C to stop)')
        run([sys.executable, '-m', 'http.server', '--directory', self.out, '--bind', '::', str(self.args.port)])


def exit_on_signal(signum, frame):
    sys.exit(128 + signum)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--versions', metavar='LABELS',
                        help='comma-separated labels from versions.toml to build, or "all" for the released versions '
                             '(those with a ref, which are published) (default: the version without a ref, built from '
                             'the INET tree)')
    parser.add_argument('--inet', metavar='PATH', help='INET tree for the version without a ref (default: the tree '
                                                       'this script is in)')
    parser.add_argument('--media', metavar='PATH', help='media repository checkout (default: <inet>/media, or '
                                                        'exported from git if there is none)')
    parser.add_argument('--only', metavar='PARTS',
                        help=f'build only these parts: site (the website pages), {",".join(SHARED_PARTS)} '
                             f'(built once, from the INET tree), and of each version {",".join(PARTS)} '
                             '(default: all)')
    parser.add_argument('--pdf', action='store_true', help=f'also build PDFs of the {", ".join(PDF_MANUALS)} '
                                                           '(needs the -pdf image)')
    parser.add_argument('--sample', action='store_true',
                        help='build only a sample of the NED reference (about 20 types), the showcases and the '
                             'tutorials: a small site for working on the site itself')
    parser.add_argument('--force', nargs='?', const='all', metavar='LABELS',
                        help='rebuild these versions, or "shared" for the showcases and tutorials (default: all), '
                             'even if their cached build is current')
    parser.add_argument('--serve', action='store_true', help='serve the output over HTTP after building')
    parser.add_argument('--port', type=int, default=8000)
    parser.add_argument('--baseurl', default=SITE_URL,
                        help=f'absolute URL of the site, for the canonical links and the sitemap (default: {SITE_URL})')
    parser.add_argument('--out', default=os.path.join(TREE, 'doc', 'site'),
                        help='output directory (default: doc/site)')
    parser.add_argument('--cache', default=os.path.join(SRC, '_build'), help='cache directory (default: doc/src/_build)')
    parser.add_argument('--keep-out', action='store_true', help='do not empty the output directory first')
    # Each Sphinx process holds the whole environment of a version (gigabytes with the NED reference).
    parser.add_argument('--jobs', default=str(min(os.cpu_count() or 1, 8)),
                        help='parallel Sphinx processes (default: the number of CPUs, at most 8)')
    parser.add_argument('--no-fonts', action='store_true',
                        help='use the system fonts instead of the web fonts of the theme (for offline builds outside '
                             'the doc-builder image, which has them)')
    parser.add_argument('--no-size-limit', action='store_true', help=f'do not fail above {SIZE_LIMIT // 1024 // 1024} MB')
    parser.add_argument('--quiet', action='store_true', help='only print Sphinx warnings')
    parser.add_argument('--legacy-api-redirects', metavar='DIR',
                        help='also write redirects from the former doc.omnetpp.org/inet/api-current/ pages into DIR')
    args = parser.parse_args()
    # Exiting by an exception lets subprocess.run kill the running child (e.g. the HTTP server), which would
    # otherwise outlive the build.
    for signum in (signal.SIGTERM, signal.SIGHUP):
        signal.signal(signum, exit_on_signal)
    try:
        Builder(args).build()
    except (BuildError, subprocess.CalledProcessError) as e:
        sys.exit(f'build: error: {e}')
    except KeyboardInterrupt:
        sys.exit(128 + signal.SIGINT)


if __name__ == '__main__':
    main()
