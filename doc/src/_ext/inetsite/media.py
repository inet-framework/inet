"""The video and audio directives of omnetpp_media (doc-builder image), with the files of the media repository.

Video and audio files are not part of the generated site: they live in the inet-framework/media repository,
which is served separately (see INETSITE_MEDIA_URL). INET documents refer to them through the committed
`doc/media` symlinks that point into the media checkout, so a relative path is resolved through those
symlinks and turned into a URL on the media site. omnetpp_media handles the rest: the few small files that are
committed into INET itself are copied into the site.
"""

import html
import os

import omnetpp_media

from . import config


class MediaRepository:
    def media_url(self, uri):
        cfg = config.current()
        if cfg.media_root and '://' not in uri and not uri.startswith('/'):
            env = self.state.document.settings.env
            path = os.path.realpath(env.relfn2path(uri, env.docname)[1])
            media_root = os.path.realpath(cfg.media_root)
            if path.startswith(media_root + os.sep):
                return html.escape(cfg.media_url + '/' + os.path.relpath(path, media_root).replace(os.sep, '/')), None
        return super().media_url(uri)


class Video(MediaRepository, omnetpp_media.Video):
    pass


class VideoNoLoop(MediaRepository, omnetpp_media.VideoNoLoop):
    pass


class Audio(MediaRepository, omnetpp_media.Audio):
    pass


def setup(app):
    app.setup_extension('omnetpp_media')
    for name, directive in (('video', Video), ('video_noloop', VideoNoLoop), ('video_no_loop', VideoNoLoop),
                            ('audio', Audio)):
        app.add_directive(name, directive, override=True)
