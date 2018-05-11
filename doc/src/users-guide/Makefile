PERL=perl
INKSCAPE=inkscape
DOT=dot
LTOH=$(PERL) ../tools/ltoh
PREPHTML=$(PERL) ../tools/prephtml

DOCNAME=inet-users-guide
TEXMAIN=$(DOCNAME).tex
HTMLTEMPLATE=inet-users-guide.thtml
MDTEMPLATE=inet-users-guide.tmd
VERSION=4.x
HTMLDIR=../../users-guide
PDFDIR=../..
MDDIR=~/inet-framework.github.io/users-guide

ALLTEX = $(wildcard *.tex)
DOTPICS = $(wildcard figures/*.dot)
SVGPICS = $(wildcard figures/*.svg) $(DOTPICS:.dot=.svg)
PDFPICS = $(SVGPICS:.svg=.pdf) $(DOTPICS:.dot=.pdf)

.PHONY : pdf html md

default: pdf html

pdf: $(ALLTEX) $(PDFPICS) figures/*.png
	pdflatex $(TEXMAIN)
	pdflatex $(TEXMAIN)
	pdflatex $(TEXMAIN)
	mkdir -p $(PDFDIR)
	cp $(DOCNAME).pdf $(PDFDIR)

%.pdf: %.svg
	$(INKSCAPE) $< -A $@

%.svg: %.dot
	$(DOT) -Tsvg $< >$@

.SUFFIXES : .svg .dot

%.png: %.svg
	$(INKSCAPE) $< -e $@

%.png: %.dot
	$(DOT) -Tpng $< >$@

html : $(ALLTEX) $(SVGPICS) $(HTMLTEMPLATE) prephtml ltoh ltoh.specs
	CHAPTERS=$$(grep '\include{' $(TEXMAIN) | sed 's/\\include{//;s/}/.tex/' | sed '/^title\.tex/d') && cat $$CHAPTERS > _allinone_.tex
	$(LTOH) _allinone_.tex
	$(PREPHTML) --template $(HTMLTEMPLATE) --productversion $(VERSION) --outfilenamebase $(DOCNAME) _allinone_.html
	rm _allinone_.tex _allinone_.html
	mkdir -p $(HTMLDIR)
	cp $(DOCNAME).html $(HTMLDIR)/index.html
	cp Snippets.* $(HTMLDIR)
	cp *.png $(HTMLDIR)
	cp figures/*.svg $(HTMLDIR)
	cp figures/*.png $(HTMLDIR)
	@if grep 'Unknown LaTeX command' $(DOCNAME).html; then echo 'Error: LaTeX commands missing from ltoh.specs! (see lines above)'; exit 1; fi

md : $(ALLTEX) $(SVGPICS) $(MDTEMPLATE) prephtml ltoh ltoh.specs
	CHAPTERS=$$(grep '\include{' $(TEXMAIN) | sed 's/\\include{//;s/}/.tex/' | sed '/^title\.tex/d') && cat $$CHAPTERS > _allinone_.tex
	$(LTOH) _allinone_.tex
	$(PREPHTML) --ext md --split --template $(MDTEMPLATE) --productversion $(VERSION) --outfilenamebase $(DOCNAME) _allinone_.html
	rm _allinone_.tex _allinone_.html
	mkdir -p $(MDDIR)
	cp *.md Snippets.* $(MDDIR)
	cp *.png $(MDDIR)
	cp figures/*.svg $(MDDIR)
	cp figures/*.png $(MDDIR)
	@if grep 'Unknown LaTeX command' *.md; then echo 'Error: LaTeX commands missing from ltoh.specs! (see lines above)'; exit 1; fi

clean:
	rm -f $(DOCNAME).dvi $(DOCNAME).pdf $(DOCNAME).html $(DOCNAME).md *.md *.aux *.idx *.ilg *.ind *.log *.out *.toc *.bbl *.blg
	rm -rf $(HTMLDIR)
	rm -rf $(MDDIR)
	rm -rf $(PDFDIR)/$(DOCNAME).pdf
	rm -rf eclipse
	rm -f figures/*.pdf
	rm -f `ls figures/*.svg | sed 's/svg$$/png/'`
	rm -f `ls figures/*.dot | sed 's/dot$$/svg/'`
