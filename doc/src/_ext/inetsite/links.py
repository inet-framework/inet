"""Makes the links between the pages of the site relative, so that the output works wherever it is served
from, for example from a local server. Sphinx gets absolute URLs, because a toctree only takes docnames and
absolute URLs, and these and the root-relative links of the pages are rewritten once all pages are written.
The canonical links and the sitemap stay absolute.
"""

import os
import posixpath
import re
import urllib.parse

from sphinx.util import logging

from . import config

logger = logging.getLogger(__name__)


def relative_url(page, target):
    """Returns the URL of target relative to page, both given as paths from the site root."""
    path, suffix = re.match(r'([^?#]*)(.*)', target, re.S).groups()
    url = posixpath.relpath(path or '.', posixpath.dirname(page) or '.')
    if not path or path.endswith('/'):
        url += '/'
    return url + suffix


def link_pattern(baseurl):
    parts = urllib.parse.urlsplit(baseurl)
    site = re.escape((parts.netloc + parts.path.rstrip('/')).encode())
    return re.compile(rb'(<a\s[^>]*?\bhref=")(?:(?:https?:)?//' + site + rb'(?=[/"?#])|(?=/(?!/)))/?([^"]*)"')


def make_links_relative(outdir, baseurl, page_root):
    pattern = link_pattern(baseurl)
    site = urllib.parse.urlsplit(baseurl).netloc.encode()
    pages = links = 0
    for dirpath, _, filenames in os.walk(outdir):
        for name in filenames:
            if not name.endswith('.html'):
                continue
            file = os.path.join(dirpath, name)
            with open(file, 'rb') as f:
                data = f.read()
            if site not in data and b'href="/' not in data:
                continue
            page = page_root + os.path.relpath(file, outdir).replace(os.sep, '/')

            def replace(match):
                target = relative_url(page, match[2].decode())
                return match[1] + target.encode() + b'"'

            data, count = pattern.subn(replace, data)
            if count:
                with open(file, 'wb') as f:
                    f.write(data)
                pages += 1
                links += count
    return pages, links


def on_build_finished(app, exception):
    if exception or app.builder.format != 'html':
        return
    cfg = config.current()
    page_root = cfg.page_root_url.removeprefix(cfg.baseurl + '/')
    pages, links = make_links_relative(app.outdir, cfg.baseurl, page_root)
    logger.info(f'made {links} links in {pages} pages relative')


def setup(app):
    app.connect('build-finished', on_build_finished)
