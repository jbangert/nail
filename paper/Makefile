
main.pdf: $(wildcard *.tex) $(wildcard *.bib)
	pdflatex main
	bibtex -min-crossrefs=100 main
	pdflatex main
	pdflatex main

clean:
	rm -f main.pdf main.aux main.bbl main.blg main.log main.out

