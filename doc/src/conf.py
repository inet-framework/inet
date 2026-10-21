# Sphinx configuration of inet.omnetpp.org; run it through ./build (see README.md), not sphinx-build directly.
#
# The same configuration builds two kinds of output, selected by INETSITE_MODE:
#   site     the unversioned website pages (sources in site/, on the website branch only)
#   version  the documentation of one INET version (manuals, NED and Python API),
#            assembled by tools/build.py from an INET tree
# Everything else comes in through INETSITE_* environment variables set by tools/build.py.

import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '_ext'))
sys.path.insert(0, HERE)

from inetsite import config

cfg = config.current()

project = 'INET Framework'
author = 'INET community'
copyright = 'INET community'
version = release = cfg.version or ''
language = 'en'

extensions = [
    'sphinx_immaterial',
    'sphinx_immaterial.graphviz',
    'sphinx.ext.extlinks',
    'sphinx.ext.todo',
    'sphinx.ext.napoleon',
    'sphinxcontrib.katex',
    'sphinxcontrib.jquery',
    'sphinxcontrib.images',
    'sphinx_design',
    'IPython.sphinxext.ipython_console_highlighting',
    'tools.includefiles',
    'inetsite',
]

root_doc = 'index'
exclude_patterns = ['_build', '_extra', 'Thumbs.db', '.DS_Store', '**/_docs', '**/README.md']
# The section files of the migration guide are parts of migration-guide/index.rst (tools/includefiles.py).
exclude_patterns.append('migration-guide/*/*.rst')
if cfg.mode == 'site':
    html_extra_path = [os.path.join(HERE, 'site', '_extra')]
html_static_path = [os.path.join(HERE, '_static')]
html_css_files = ['inet.css']
html_favicon = os.path.join(HERE, '_static', 'favicon.ico')
html_title = 'INET Framework'
html_baseurl = cfg.page_root_url
html_copy_source = False
html_show_sourcelink = False
html_last_updated_fmt = None

suppress_warnings = []
if cfg.mode == 'version':
    # NED type pages are reached from the package and type listings, not from the navigation tree:
    # a toctree over thousands of pages would be embedded into every page.
    suppress_warnings.append('toc.not_included')

katex_prerender = False

extlinks = {
    'wiki': ('https://en.wikipedia.org/wiki/%s', '%s'),
    'docs': (cfg.docs_url('%s'), '%s'),
}

images_config = {
    'override_image_directive': True,
    'default_show_title': False,
}

todo_include_todos = False

html_theme = 'sphinx_immaterial'
html_theme_options = {
    'icon': {
        'repo': 'fontawesome/brands/github',
    },
    'site_url': cfg.baseurl + '/',
    'repo_url': 'https://github.com/inet-framework/inet',
    'repo_name': 'inet-framework/inet',
    'globaltoc_collapse': True,
    'toc_title_is_page_title': True,
    'features': [
        'content.code.copy',
        'navigation.tabs',
        'navigation.top',
        'search.highlight',
        'search.share',
        'toc.follow',
        'toc.sticky',
    ],
    'palette': [
        {
            'media': '(prefers-color-scheme: light)',
            'scheme': 'default',
            'primary': 'light-blue',
            'accent': 'blue',
            'toggle': {'icon': 'material/lightbulb-outline', 'name': 'Switch to dark mode'},
        },
        {
            'media': '(prefers-color-scheme: dark)',
            'scheme': 'slate',
            'primary': 'light-blue',
            'accent': 'lime',
            'toggle': {'icon': 'material/lightbulb', 'name': 'Switch to light mode'},
        },
    ],
    'analytics': {'provider': 'google', 'property': 'G-WPFKHJQDQW'},
    'version_dropdown': cfg.mode == 'version',
    'version_json': '../versions.json',
    'scope': '/',
}
if not cfg.fetch_fonts:
    html_theme_options['font'] = False

if cfg.mode == 'version':
    rst_prolog = cfg.read_prolog()
    if cfg.latex:
        extensions.append('sphinx.ext.imgconverter')
    latex_engine = 'pdflatex'
    latex_elements = {'papersize': 'a4paper'}
    latex_documents = [(f'{part}/index', f'inet-{part}.tex', f'INET Framework {title}', author, 'manual', False)
                       for part, title in (('users-guide', "User's Guide"), ('developers-guide', "Developer's Guide"),
                                           ('migration-guide', 'Migration Guide'))
                       if cfg.has_part(part)]
    if cfg.has_part('python-api'):
        extensions.append('autoapi.extension')
        suppress_warnings.append('autoapi.python_import_resolution')
        autoapi_generate_api_docs = True
        autoapi_add_toctree_entry = False
        autoapi_python_use_implicit_namespaces = True
        autoapi_root = 'python-api'
        autoapi_dirs = [os.path.join(cfg.inet_root, 'python', 'inet')]
        autoapi_python_class_content = 'both'
        autoapi_options = ['members', 'show-inheritance', 'show-module-summary', 'inherited-members']
        autoapi_member_order = 'groupwise'
        autoapi_own_page_level = 'class'
