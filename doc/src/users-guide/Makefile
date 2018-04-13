DOCNAME=inet-users-guide

DOTPICS = $(wildcard figures/*.dot)
SVGPICS = $(wildcard figures/*.svg)
PDFPICS = $(SVGPICS:.svg=.pdf) $(DOTPICS:.dot=.pdf)
PNGPICS = $(SVGPICS:.svg=.png) $(DOTPICS:.dot=.png)

INKSCAPE=inkscape
DOT=dot

pdf: *.tex $(PDFPICS) figures/*.png
	pdflatex $(DOCNAME).tex
	bibtex $(DOCNAME)
	makeindex $(DOCNAME)
	pdflatex $(DOCNAME).tex
	pdflatex $(DOCNAME).tex
	rm -f *.aux *.idx *.ilg *.ind *.log *.out *.toc *.blg *.bbl
	mv $(DOCNAME).pdf ../..

%.pdf: %.svg
	$(INKSCAPE) $< -A $@

%.svg: %.dot
	$(DOT) -Tsvg $< >$@

clean:
	rm -f $(DOCNAME).pdf *.aux *.idx *.ilg *.ind *.log *.out *.toc *.bbl *.blg
	rm -rf ../../$(DOCNAME).pdf
	rm -f figures/*.pdf
	rm -f `ls figures/*.svg | sed 's/svg$$/png/'`
	rm -f `ls figures/*.dot | sed 's/dot$$/svg/'`
