"""Sphinx extensions of inet.omnetpp.org."""

from . import devrefs, links, media, nav, pdf, posts


def setup(app):
    for module in (devrefs, links, media, nav, pdf, posts):
        module.setup(app)
    return {'parallel_read_safe': True, 'parallel_write_safe': True}
