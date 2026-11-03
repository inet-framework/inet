INET documentation and the inet.omnetpp.org website
===================================================

This directory builds https://inet.omnetpp.org with Sphinx: the website pages; for each released INET version
listed in `versions.toml`, the User's, Developer's and Migration Guides, the Python API, and the NED reference
(generated with `opp_docgen`), under `docs/<version>/`; and the showcases and tutorials, which are not
versioned: they are built once, from the INET tree of the build, under `docs/showcases/` and `docs/tutorials/`.
Their references to the manuals and to NED types link to the latest version. The documentation of the build's
own tree (`dev`) is built locally only, where they link to it.

- `users-guide/`, `developers-guide/`, `migration-guide/`: the documentation, versioned with the INET
  sources; `../../showcases`, `../../tutorials`: the showcases and tutorials
- `site/`: the website pages; news posts are the dated `YYYY-MM-DD-*.rst` files
- `conf.py`, `_ext/`, `_static/`, `global.rst`: the Sphinx configuration
- the Sphinx extensions of the doc-builder image ([omnetpp/doc-builder](https://github.com/omnetpp/doc-builder), described in its `image/README.md`),
  which `conf.py` and `_ext/` build on: `omnetpp_immaterial`, the extension of `sphinx_immaterial` that gives the
  OMNeT++ sites their look (`_static/inet.css` styles it for this site), `omnetpp_lexers`, the highlighting of the
  `ned`, `msg`, `ini` and `fp` code blocks, `omnetpp_neddoc`, the `:ned:` and `:msg:` roles and the diagrams of
  the NED reference (`conf.py` points the builds without a reference of their own to that of another build), and
  `omnetpp_media`, the `video`, `audio`, `youtube` and `vimeo` directives (`_ext/inetsite/media.py` links the
  files of the media repository)
- `build`, `tools/build.py`: the build; `versions.toml`: the published versions; `.doc-builder-image`: the
  container image the build runs in

The site is published from the `website` branch, which points to a commit of `master`:
`.github/workflows/site.yml` builds and deploys it to GitHub Pages whenever it is pushed, so that changes on
`master` are published only when `website` is moved to them. It publishes the site pages, the showcases and
tutorials of that commit, and the released versions of `versions.toml`, built from their tags
(`--versions all --pdf`), not the documentation of the commit itself as `dev`.

Building locally
----------------

Requires podman, or docker with `ENGINE=docker`. With `ENGINE=none`, the build runs without a container (needs the
Python packages of the doc-builder image, its `omnetpp_*` extensions among them, `opp_docgen` and graphviz, and for
`--pdf` TeX Live with `latexmk`). Run from anywhere:

    doc/src/build                          # the site pages and the docs of this tree as "dev"
    doc/src/build --serve                  # the same, then served at http://localhost:8000/
    doc/src/build --only showcases --serve # only the showcases
    doc/src/build --only showcases,tutorials,api
    doc/src/build --only site --serve      # only the website pages
    doc/src/build --versions all --pdf     # everything that is published: what CI builds
    doc/src/build --pdf --only users-guide # the User's Guide, also as PDF
    doc/src/build --versions 4.7           # one released version, built from its tag
    doc/src/build --sample --serve         # a small site (about 30 MB, in seconds) for working on the site

The parts for `--only` are `site` (the website pages), `showcases` and `tutorials`, and of each version
`users-guide`, `developers-guide`, `migration-guide`, `python-api` and `api` (the NED reference). Without
`--only`, all of them are built. The showcases and tutorials link their references to the manuals and their
`:ned:` references to `dev` if the run builds it with those parts, otherwise to the `latest` version, of this run
or of an earlier one in the cache, never to an earlier build of `dev`, which is not published. Without a build to
link to, or for a document, label or type that it does not have, those references render as plain text.
`--inet PATH` builds another INET tree (e.g. a second workspace) instead of this one.

The site's links to the documentation, and the `docs/latest/` and old `docs/<manual>/` redirects, point to the
`latest` version of `versions.toml`. A build without that version points them to the first version it builds
instead, i.e. `dev` in a local build.

`--sample` builds only a few NED types, showcases and tutorials (listed in `SAMPLE` in `tools/build.py`), and
the rest in full; combine it with `--only` to leave parts out. Its builds are cached apart from the full ones.

`--pdf` also builds the User's, Developer's and Migration Guides as PDFs (`docs/<label>/inet-<manual>.pdf`),
with the TeX Live of the doc-builder image. `releng/makedist` uses it for the release tarball.

The output goes to `doc/site/`. The cache in `doc/src/_build/` keeps the Sphinx environments, so rebuilds
are incremental, and versions built from git are only rebuilt when their commit, the media repository or
the build configuration changes (or with `--force`).

Showcase and tutorial images come from the media repository. A checkout at `<inet>/media` is used if there
is one; otherwise the build exports the media repository into the cache and links it there. Videos are
not part of the site: they are linked from https://inet-framework.github.io/media/ (`INETSITE_MEDIA_URL`
overrides it).

`doc/src/build --help` lists all options.

Editing the website pages
-------------------------

The pages are edited on `master` like the rest of the documentation; check them with
`doc/src/build --only site --serve`.

To publish `master` (the pages, the documentation, the build configuration, `versions.toml`), move `website` to
it and push it:

    jj bookmark set website -r master
    jj git push -b website

or `git push origin master:website`. `website` only ever moves forward along `master`, so it needs no merges.

Releasing a new INET version
----------------------------

1. On `master`, add the version to `versions.toml` (normally as the new `latest`, with the release tag as
   `ref`), and prune old versions if the site gets close to the size limit (the build fails above 900 MB;
   a version takes about 300 MB).
2. Add a news post to `doc/src/site/` and update `doc/src/site/Download.rst`, on `master`.
3. Move `website` to `master` and push it (see above).

Setting up the automatic build
------------------------------

One-time settings, by a repository admin:

1. The doc-builder image (`ghcr.io/omnetpp/doc-builder`, built from [omnetpp/doc-builder](https://github.com/omnetpp/doc-builder))
   must be public on GHCR. `.doc-builder-image` names the tag the site is built with: the moving major tag
   (`:v1`), as for the `omnetpp/doc-builder@v1` action, so compatible releases of the image are picked up without a
   change here (the cached builds follow the content of the image it names).
2. Settings → Pages → Build and deployment → Source: **GitHub Actions**.
3. Settings → Environments → `github-pages` → Deployment branches and tags: allow `website`.
4. Settings → Pages → Custom domain: `inet.omnetpp.org` (a domain can be used by one repository only, so
   remove it from the previous one first), then Enforce HTTPS. The DNS record is a CNAME to
   `inet-framework.github.io`.
5. In `inet-framework/media`: Settings → Pages → Deploy from a branch: `master`, `/ (root)`, which serves the
   videos at https://inet-framework.github.io/media/.

Pushing `website` then builds and deploys the site. To rebuild by hand: Actions → "Build and deploy
inet.omnetpp.org" → Run workflow on `website`; the `force` input rebuilds cached versions. Runs on other
branches build the site as an artifact without deploying it.
