import dataclasses
import functools
import os

PARTS = ('users-guide', 'developers-guide', 'migration-guide', 'showcases', 'tutorials', 'python-api', 'api')

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

    @classmethod
    def from_environment(cls):
        env = os.environ.get
        mode = env('INETSITE_MODE', 'site')
        if mode not in ('site', 'version'):
            raise ValueError(f'INETSITE_MODE must be "site" or "version", not "{mode}"')
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
        )

    def has_part(self, part):
        return self.mode == 'version' and part in self.parts

    def has_version_part(self, part):
        return self.mode == 'version' and part in self.version_parts

    @property
    def page_root_url(self):
        if self.mode == 'version':
            return f'{self.baseurl}/docs/{self.version}/'
        return self.baseurl + '/'

    def site_url(self, page):
        return f'{self.baseurl}/{page}.html'

    def docs_url(self, path):
        return f'{self.baseurl}/docs/{self.latest}/{path}'

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
