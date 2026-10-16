"""Adds a link to the PDF version on the start page of the manuals that have one (see build.py --pdf)."""

from docutils import nodes

from . import config


def add_pdf_link(app, doctree, docname):
    part, _, rest = docname.partition('/')
    if rest != 'index' or part not in config.current().pdfs or app.builder.format != 'html':
        return
    section = next(iter(doctree.findall(nodes.section)), None)
    if section is None:
        return
    link = nodes.reference('', 'Download as PDF', refuri=f'../inet-{part}.pdf')
    section.insert(1, nodes.paragraph('', '', link, classes=['pdf-download']))


def setup(app):
    app.connect('doctree-resolved', add_pdf_link)
