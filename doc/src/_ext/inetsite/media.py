"""Video and audio directives.

Video and audio files are not part of the generated site: they live in the inet-framework/media repository,
which is served separately (see INETSITE_MEDIA_URL). INET documents refer to them through the committed
`doc/media` symlinks that point into the media checkout, so a relative path is resolved through those
symlinks and turned into a URL on the media site.
"""

import html
import os

from docutils import nodes
from docutils.parsers.rst import Directive, directives
from sphinx.util import logging

from . import config

logger = logging.getLogger(__name__)


def align(argument):
    return directives.choice(argument, ('left', 'center', 'right'))


class MediaDirective(Directive):
    has_content = True
    required_arguments = 1
    option_spec = {
        'width': directives.unchanged,
        'height': directives.unchanged,
        'align': align,
    }

    def media_url(self, uri):
        if '://' in uri or uri.startswith('/'):
            return uri
        cfg = config.current()
        env = self.state.document.settings.env
        path = os.path.realpath(os.path.join(os.path.dirname(env.doc2path(env.docname)), uri))
        media_root = os.path.realpath(cfg.media_root) if cfg.media_root else None
        if media_root and path.startswith(media_root + os.sep):
            return cfg.media_url + '/' + os.path.relpath(path, media_root).replace(os.sep, '/')
        logger.warning('media file is not in the media repository: %s', uri, location=(env.docname, self.lineno))
        return uri

    def attributes(self):
        attrs = {
            'width': self.options.get('width', ''),
            'height': self.options.get('height', ''),
            'align': self.options.get('align', 'center'),
            'caption': ''.join(self.content),
        }
        return {k: html.escape(v) for k, v in attrs.items()}

    def raw(self, text):
        return [nodes.raw('', text, format='html')]


class Video(MediaDirective):
    loop = True

    def run(self):
        a = self.attributes()
        src = html.escape(self.media_url(self.arguments[0]))
        loop = ' loop' if self.loop else ''
        return self.raw(f'<video autoplay{loop} controls onclick="this.paused ? this.play() : this.pause();" '
                        f'src="{src}" width="{a["width"]}" height="{a["height"]}" class="align-{a["align"]}"></video>')


class VideoNoLoop(Video):
    loop = False


class Audio(MediaDirective):
    def run(self):
        a = self.attributes()
        src = html.escape(self.media_url(self.arguments[0]))
        return self.raw(f'<audio controls class="align-{a["align"]}" src="{src}">Audio -- {a["caption"]}</audio>')


class Youtube(MediaDirective):
    def run(self):
        a = self.attributes()
        video_id = html.escape(self.arguments[0])
        return self.raw(f'<div style="text-align: {a["align"]}"><iframe src="https://www.youtube.com/embed/{video_id}" '
                        f'width="{a["width"]}" height="{a["height"]}" frameborder="0" allowfullscreen '
                        f'class="align-{a["align"]}"></iframe><div class="video-caption"><i>{a["caption"]}</i></div></div>')


class Vimeo(MediaDirective):
    def run(self):
        a = self.attributes()
        video_id = html.escape(self.arguments[0])
        return self.raw(f'<iframe src="https://player.vimeo.com/video/{video_id}" width="{a["width"]}" '
                        f'height="{a["height"]}" frameborder="0" allowfullscreen class="align-{a["align"]}"></iframe>')


def setup(app):
    app.add_directive('video', Video)
    app.add_directive('video_noloop', VideoNoLoop)
    app.add_directive('audio', Audio)
    app.add_directive('youtube', Youtube)
    app.add_directive('vimeo', Vimeo)
