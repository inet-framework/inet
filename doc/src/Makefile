# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = sphinx-build
SPHINXPROJ    = INETFramework
SOURCEDIR     = .
BUILDDIR      = _build

.PHONY: developers_guide users_guide help server

all: developers_guide users_guide

developers_guide:
	cd developers-guide && make

users_guide:
	cd users-guide && make

clean:
	cd users-guide && make clean
	cd developers-guide && make clean

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

include tools/sphinx_deployment.mk

server: Makefile
	@sphinx-autobuild "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O) -r .git

pdf: Makefile
	@$(SPHINXBUILD) -M latexpdf "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

html: Makefile
	@$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	@# copy the video files also to the output directory. This should be copied by the SPHINXBUILD itself, so this must be FIXED later.
	@rsync -a --copy-links --prune-empty-dirs --exclude '_docs' --include '*/' --include '*.mp4' --include '*.m4v' --exclude '*' tutorials _build/html
	@rsync -a --copy-links --prune-empty-dirs --exclude '_docs' --include '*/' --include '*.mp4' --include '*.m4v' --exclude '*' showcases _build/html

