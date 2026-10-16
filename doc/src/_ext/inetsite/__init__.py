"""Sphinx extensions of inet.omnetpp.org."""

from . import lexers, links, media, nav, ned, pdf, posts


def setup(app):
    for module in (lexers, links, media, nav, ned, pdf, posts):
        module.setup(app)
    return {'parallel_read_safe': True, 'parallel_write_safe': True}
