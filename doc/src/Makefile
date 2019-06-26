# You can set these variables from the command line.
SPHINXOPTS    = -j8
SPHINXBUILD   = sphinx-build
SPHINXPROJ    = INETFramework
SOURCEDIR     = .
BUILDDIR      = _build

.PHONY: pdf html clean deploy help server copy-media

all: html pdf

clean:
	@rm -rf $(BUILDDIR)

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

include tools/sphinx_deployment.mk

server: Makefile copy-media
	@sphinx-autobuild --host 0.0.0.0 --port 8000 "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O) -r .git

pdf: Makefile copy-media
	@$(SPHINXBUILD) -M latexpdf "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

html: Makefile copy-media
	@$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

copy-media: ../../media
	@mkdir -p _build/html
	@# copy the video files also to the output directory. This should be copied by the SPHINXBUILD itself, so this must be FIXED later.
	@rsync -a --copy-links --prune-empty-dirs --exclude '_docs' --include '*/' --include '*.wav' --include '*.mp3' --include '*.mp4' --include '*.m4v' --exclude '*' tutorials _build/html
	@rsync -a --copy-links --prune-empty-dirs --exclude '_docs' --include '*/' --include '*.wav' --include '*.mp3' --include '*.mp4' --include '*.m4v' --exclude '*' showcases _build/html

../../media:
	@echo "*** 'media' folder is not present. Check out using:"
	@echo "*** git clone git@github.com:inet-framework/media.git"
	@false
