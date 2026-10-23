INET documentation and the inet.omnetpp.org website
===================================================

This directory builds https://inet.omnetpp.org with Sphinx: the website pages; for each released INET version
listed in `versions.toml`, the User's, Developer's and Migration Guides, the Python API, and the NED reference
(generated with `opp_docgen`), under `docs/<version>/`; and the showcases and tutorials, which are not
versioned: they are built once, from the INET tree of the build (`master`, or `website` on the site), under
`docs/showcases/` and `docs/tutorials/`. Their references to the manuals and to NED types link to the latest
version. The documentation of the build's own tree (`dev`) is built locally only.

- `users-guide/`, `developers-guide/`, `migration-guide/`: the documentation, versioned with the INET
  sources; `../../showcases`, `../../tutorials`: the showcases and tutorials
- `site/`: the website pages; news posts are the dated `YYYY-MM-DD-*.rst` files. They exist **only on the
  `website` branch**, which is `master` plus these pages
- `conf.py`, `_ext/`, `_static/`, `global.rst`: the Sphinx configuration
- `build`, `tools/build.py`: the build; `versions.toml`: the published versions; `.doc-builder-image`: the
  container image the build runs in

The `website` branch is built and deployed to GitHub Pages by `.github/workflows/site.yml` whenever it is pushed.
It publishes the released versions of `versions.toml`, built from their tags (`--versions all --pdf`), not
its own tree as `dev`.

Building locally
----------------

Requires podman, or docker with `ENGINE=docker`. With `ENGINE=none`, the build runs without a container
(needs the Python packages of the doc-builder image, `omnetpp_immaterial` among them, `opp_docgen` and graphviz). Run from anywhere:

    doc/src/build                          # the docs of this tree as "dev" (+ the site pages on website)
    doc/src/build --serve                  # the same, then served at http://localhost:8000/
    doc/src/build --only showcases --serve # only the showcases
    doc/src/build --versions 4.7 --only showcases,tutorials,api
    doc/src/build --only site --serve      # only the website pages (website branch)
    doc/src/build --versions all --pdf     # everything that is published: what CI builds
    doc/src/build --pdf --only users-guide # the User's Guide, also as PDF
    doc/src/build --versions 4.7           # one released version, built from its tag
    doc/src/build --sample --serve         # a small site (about 30 MB, in seconds) for working on the site

The parts for `--only` are `site` (the website pages), `showcases` and `tutorials`, and of each version
`users-guide`, `developers-guide`, `migration-guide`, `python-api` and `api` (the NED reference). Without
`--only`, all of them are built. Only the released versions have the `api` part, the NED reference: the `dev`
documentation and the showcases and tutorials link their `:ned:` references to that of the `latest` version, of
this run or of an earlier one in the cache, and render them as plain text without one, or for a type that it does
not have. The showcases and tutorials link to the manuals of `dev` if the run builds it, otherwise to those
of the `latest` version, of this run or of an earlier one in the cache; without one, or for a document or label
that it does not have, those links render as plain text. `--inet PATH` builds another INET tree (e.g. a second workspace) instead of this one.

The site's links to the documentation, and the `docs/latest/` and old `docs/<manual>/` redirects, point to the
`latest` version of `versions.toml`. A build without that version points them to the first version it builds
instead, i.e. `dev` in a local build.

`--sample` builds only a few NED types, showcases and tutorials (listed in `SAMPLE` in `tools/build.py`), and
the rest in full (add `--versions dev,4.7` for a sample of the NED reference); combine it with `--only` to leave
parts out. Its builds are cached apart from the full ones.

`--pdf` also builds the User's, Developer's and Migration Guides as PDFs (`docs/<label>/inet-<manual>.pdf`),
in the `-pdf` variant of the image, which adds TeX Live. `releng/makedist` uses it for the release tarball.

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

Work on the `website` branch, and push it to publish. With jj:

    jj new website                # edit doc/src/site/..., check with doc/src/build --only site --serve
    jj describe -m "site: ..."
    jj bookmark set website -r @
    jj git push -b website

With git: `git switch website`, commit, `git push origin website`.

To publish changes of `master` (documentation, build configuration, `versions.toml`), merge it into `website`:

    jj new website master -m "doc: merge master"
    jj bookmark set website -r @
    jj git push -b website

or `git switch website && git merge master && git push origin website`. Never merge `website` into `master`.

Releasing a new INET version
----------------------------

1. On `master`, add the version to `versions.toml` (normally as the new `latest`, with the release tag as
   `ref`), and prune old versions if the site gets close to the size limit (the build fails above 900 MB;
   a version takes about 300 MB).
2. Merge `master` into `website`, add a news post to `doc/src/site/` and update `doc/src/site/Download.rst`.
3. Push `website`.

Setting up the automatic build
------------------------------

One-time settings, by a repository admin:

1. The doc-builder image (`ghcr.io/omnetpp/doc-builder`, built from omnetpp-dev `doc/src/site/doc-builder`)
   must be public on GHCR. `.doc-builder-image` names the tag the site is built with (the cached builds follow
   the content of the image it names); its `-pdf` variant (same tag plus `-pdf`) must exist too.
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
