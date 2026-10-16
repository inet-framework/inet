"""The site navigation, shared by the site build and the version builds.

Targets are written as `site:<page>` (an unversioned page in site/), `docs:<part>/<path>` (a page of the
versioned documentation) or `url:<url>`. Each build turns the targets it contains into document references
and the others into absolute links, so that every build shows the same navigation.
"""

from docutils import nodes
from docutils.parsers.rst import Directive
from docutils.statemachine import StringList

from . import config

NAV = [
    (None, [
        ('Download', 'site:Download'),
        ('Showcases', 'docs:showcases/index'),
        ('Tutorials', 'docs:tutorials/index'),
    ]),
    ('Documentation', [
        ('Introduction', 'site:Introduction'),
        ('Installing INET', 'site:Installation'),
        ('Getting Started', 'site:GettingStarted'),
        ("User's Guide", 'docs:users-guide/index'),
        ("Developer's Guide", 'docs:developers-guide/index'),
        ('Migration Guide', 'docs:migration-guide/index'),
        ('NED Reference', 'docs:api/index'),
        ('Python API', 'docs:python-api/inet/index'),
        ('Technical Presentations', 'site:TechnicalPresentations'),
        ('Model Catalog', 'site:Protocols'),
        ('3rd Party Tutorials', 'site:Tutorials'),
        ('Teaching Resources', 'site:TeachingResources'),
    ]),
    ('Development', [
        ('Repository', 'site:Repository'),
        ('Bug Tracker', 'site:BugTracker'),
        ('Mailing List', 'site:MailingList'),
        ('Coding Conventions', 'site:CodingConventions'),
        ('Documenting', 'site:DocumentationGuidelines'),
        ('Test Suite', 'site:TestSuite'),
        ('Submitting Code', 'site:SubmittingCode'),
        ('Plans', 'site:Plans'),
        ('Progress', 'site:Progress'),
    ]),
    ('Contributing', [
        ('Why Contribute', 'site:WhyContribute'),
        ('How Can You Help', 'site:HowCanYouHelp'),
        ('Getting Started', 'site:ContribGettingStarted'),
        ('Contribution Ideas', 'site:ContributionIdeas'),
    ]),
    ('People', [
        ('Contributors', 'site:Contributors'),
        ('Component Advisors', 'site:ComponentAdvisors'),
        ('Community Coordinators', 'site:CommunityCoordinators'),
        ('Core Team', 'site:CoreTeam'),
    ]),
]


def toctree_entry(cfg, title, target):
    kind, _, path = target.partition(':')
    if kind == 'url':
        return f'{title} <{path}>'
    if kind == 'site':
        return path if cfg.mode == 'site' else f'{title} <{cfg.site_url(path)}>'
    if kind == 'docs':
        part = path.split('/', 1)[0]
        if cfg.mode == 'site':
            return f'{title} <{cfg.docs_url(path + ".html")}>'
        if cfg.has_part(part):
            return f'{title} <{path}>'
        if cfg.has_version_part(part):
            return f'{title} <{cfg.page_root_url}{path}.html>'
        return None
    raise ValueError(f'invalid navigation target: {target}')


def navigation_rst(cfg):
    lines = []
    for caption, entries in NAV:
        rendered = [e for e in (toctree_entry(cfg, title, target) for title, target in entries) if e]
        if not rendered:
            continue
        lines += ['.. toctree::', '   :hidden:']
        if caption:
            lines.append(f'   :caption: {caption}')
        lines.append('')
        lines += [f'   {e}' for e in rendered]
        lines.append('')
    return lines


class Navigation(Directive):
    def run(self):
        container = nodes.container()
        self.state.nested_parse(StringList(navigation_rst(config.current()), source='<inet-nav>'),
                                self.content_offset, container)
        return container.children


def setup(app):
    app.add_directive('inet-nav', Navigation)
