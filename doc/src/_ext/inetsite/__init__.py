"""Sphinx extensions of inet.omnetpp.org."""

from . import apinav, lexers, links, media, nav, ned, pdf, posts


def setup(app):
    for module in (apinav, lexers, links, media, nav, ned, pdf, posts):
        module.setup(app)
    return {'parallel_read_safe': True, 'parallel_write_safe': True}
