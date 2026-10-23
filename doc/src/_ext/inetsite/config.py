import dataclasses
import functools
import os

VERSIONED_PARTS = ('users-guide', 'developers-guide', 'migration-guide', 'python-api', 'api')
# Built once, from the tree of the site, and published without a version (mode "shared").
SHARED_PARTS = ('showcases', 'tutorials')
PARTS = VERSIONED_PARTS + SHARED_PARTS
# The directory of a version build's output (not published) with what other builds need to link to it: the
# opp_docgen tag files of its NED reference.
REFS_DIR = '.inetsite-refs'

SRC_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


@dataclasses.dataclass
class Config:
    mode: str
    baseurl: str
    version: str
    latest: str
    parts: frozenset
    version_parts: frozenset
    inet_root: str
    media_root: str
    media_url: str
    fetch_fonts: bool
    pdfs: frozenset
    latex: bool
    sample: bool
    manuals_version: str
    manuals_docs: str
    api_version: str
    api_docs: str

    @classmethod
    def from_environment(cls):
        env = os.environ.get
        mode = env('INETSITE_MODE', 'site')
        if mode not in ('site', 'version', 'shared'):
            raise ValueError(f'INETSITE_MODE must be "site", "version" or "shared", not "{mode}"')
        baseurl = env('INETSITE_BASEURL', 'https://inet.omnetpp.org').rstrip('/')
        parts = frozenset(p for p in env('INETSITE_PARTS', ','.join(PARTS)).split(',') if p)
        # All parts of the version; those not in parts are built by another Sphinx run (see build.py).
        version_parts = frozenset(p for p in env('INETSITE_VERSION_PARTS', ','.join(parts)).split(',') if p)
        unknown = (parts | version_parts) - set(PARTS)
        if unknown:
            raise ValueError(f'unknown INETSITE_PARTS or INETSITE_VERSION_PARTS: {", ".join(sorted(unknown))}')
        return cls(
            mode=mode,
            baseurl=baseurl,
            version=env('INETSITE_VERSION', ''),
            latest=env('INETSITE_LATEST', 'latest'),
            parts=parts,
            version_parts=version_parts | parts,
            inet_root=env('INETSITE_INET_ROOT', ''),
            media_root=env('INETSITE_MEDIA_ROOT', ''),
            media_url=env('INETSITE_MEDIA_URL', 'https://inet-framework.github.io/media').rstrip('/'),
            fetch_fonts=env('INETSITE_FETCH_FONTS', '1') == '1',
            pdfs=frozenset(p for p in env('INETSITE_PDFS', '').split(',') if p),
            latex=env('INETSITE_LATEX', '0') == '1',
            sample=env('INETSITE_SAMPLE', '0') == '1',
            manuals_version=env('INETSITE_MANUALS_VERSION', 'dev'),
            manuals_docs=env('INETSITE_MANUALS_DOCS', ''),
            api_version=env('INETSITE_API_VERSION', ''),
            api_docs=env('INETSITE_API_DOCS', ''),
        )

    def has_part(self, part):
        return self.mode in ('version', 'shared') and part in self.parts

    def has_version_part(self, part):
        return self.mode == 'version' and part in self.version_parts

    @property
    def page_root_url(self):
        if self.mode == 'version':
            return f'{self.baseurl}/docs/{self.version}/'
        if self.mode == 'shared':
            return f'{self.baseurl}/docs/'
        return self.baseurl + '/'

    def site_url(self, page):
        return f'{self.baseurl}/{page}.html'

    def docs_url(self, path):
        if path.split('/', 1)[0] in SHARED_PARTS:
            return f'{self.baseurl}/docs/{path}'
        return f'{self.baseurl}/docs/{self.latest}/{path}'

    def manuals_url(self, path):
        """The URL of a page of the manuals that the shared parts link to."""
        return f'{self.baseurl}/docs/{self.manuals_version}/{path}'

    def api_url(self, path):
        """The URL of a page of the NED reference that a build without one of its own links to."""
        return f'{self.baseurl}/docs/{self.api_version}/{path}'

    def read_prolog(self):
        # The version's own prolog matches its content.
        for path in (os.path.join(self.inet_root, 'doc', 'src', 'global.rst'), os.path.join(SRC_DIR, 'global.rst')):
            if os.path.isfile(path):
                with open(path) as f:
                    return f.read()
        return ''


@functools.cache
def current():
    return Config.from_environment()
