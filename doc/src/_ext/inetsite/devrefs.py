"""The references of the shared parts (showcases, tutorials) to the documents and labels of the manuals (e.g.
:doc:`/users-guide/ch-emulation`, :ref:`ug:sec:tsn:framereplication`), which are built by a Sphinx run of their
own: those of the version built from the same tree if the run builds it, otherwise those of the latest version.
They are resolved from that build's objects.inv; the references that it does not have render as plain text.
"""

import os
import posixpath
import re
import zlib

from docutils import nodes

from . import config


def read_inventory(path):
    """Returns {(type, name): (location, display name)} of a Sphinx inventory (version 2)."""
    with open(path, 'rb') as f:
        for _ in range(4):
            f.readline()
        lines = zlib.decompress(f.read()).decode().splitlines()
    items = {}
    for line in lines:
        match = re.match(r'(.+?)\s+(\S+:\S+)\s+(-?\d+)\s+(\S*)\s+(.*)', line)
        if not match:
            continue
        name, kind, _, location, display = match.groups()
        if location.endswith('$'):
            location = location[:-1] + name
        items[(kind, name)] = (location, name if display == '-' else display)
    return items


def setup(app):
    cfg = config.current()
    path = os.path.join(cfg.manuals_docs, 'objects.inv')
    if cfg.mode != 'shared' or not os.path.isfile(path):
        return
    inventory = read_inventory(path)

    def missing_reference(app, env, node, contnode):
        if node.get('refdomain') != 'std' or node.get('reftype') not in ('doc', 'ref'):
            return None
        target = node['reftarget']
        if node['reftype'] == 'doc':
            if target.startswith('/'):
                target = target[1:]
            else:
                target = posixpath.normpath(posixpath.join(posixpath.dirname(node['refdoc']), target))
            item = inventory.get(('std:doc', target))
        else:
            item = inventory.get(('std:label', target.lower()))
        if item is None:
            return None
        location, display = item
        if not node.get('refexplicit'):
            contnode = nodes.inline(display, display, classes=contnode.get('classes', []))
        return nodes.reference('', '', contnode, internal=False, refuri=cfg.manuals_url(location), reftitle=display)

    app.connect('missing-reference', missing_reference)
