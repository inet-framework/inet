INKSCAPE=inkscape
DOT=dot
LTOH=../tools/ltoh
PREPHTML=../tools/prephtml

PRODUCT="INET Framework"
TITLE="INET __VERSION__ Developer's Guide"
DOCNAME=inet-developers-guide
TEXMAIN=$(DOCNAME).tex
HTMLTEMPLATE=inet-developers-guide.thtml
RSTTEMPLATE=inet-developers-guide.rst
VERSION=4.0
HTMLDIR=../../developers-guide
PDFDIR=../..
MDDIR=~/www/inet.omnetpp.org/developers-guide

ALLTEX = $(wildcard *.tex)
ALLRST = $(ALLTEX:.tex=.rst)
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

%.rst: %.tex
	@../tools/tex2rst $<

.SUFFIXES : .svg .dot

%.png: %.svg
	$(INKSCAPE) $< -e $@

%.png: %.dot
	$(DOT) -Tpng $< >$@

html : $(ALLTEX) $(SVGPICS) $(HTMLTEMPLATE) $(PREPHTML) $(LTOH) ltoh.specs
	CHAPTERS=$$(grep '\include{' $(TEXMAIN) | sed 's/\\include{//;s/}/.tex/' | sed '/^title\.tex/d') && cat $$CHAPTERS > _allinone_.tex
	$(LTOH) _allinone_.tex
	$(PREPHTML) --template $(HTMLTEMPLATE) --productversion $(VERSION) --product $(PRODUCT) --title $(TITLE) --outfilenamebase $(DOCNAME) _allinone_.html
	rm _allinone_.tex _allinone_.html
	mkdir -p $(HTMLDIR)
	cp $(DOCNAME).html $(HTMLDIR)/index.html
	cp lib/* $(HTMLDIR)
	cp *.png $(HTMLDIR)
	cp figures/*.svg $(HTMLDIR)
	cp figures/*.png $(HTMLDIR)
	@if grep 'Unknown LaTeX command' $(DOCNAME).html; then echo 'Error: LaTeX commands missing from ltoh.specs! (see lines above)'; exit 1; fi

rst : $(ALLRST) $(SVGPICS)

clean:
	rm -f $(DOCNAME).dvi $(DOCNAME).pdf $(DOCNAME).html $(DOCNAME).md *.md *.aux *.idx *.ilg *.ind *.log *.out *.toc *.bbl *.blg
	rm -rf $(HTMLDIR)
	rm -rf $(MDDIR)
	rm -rf $(PDFDIR)/$(DOCNAME).pdf
	rm -rf eclipse
	rm -f figures/*.pdf
	rm -f `ls figures/*.svg | sed 's/svg$$/png/'`
	rm -f `ls figures/*.dot | sed 's/dot$$/svg/'`
