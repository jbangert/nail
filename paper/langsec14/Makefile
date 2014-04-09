
main.pdf: $(wildcard *.tex) $(wildcard *.bib)
	pdflatex main
	bibtex -min-crossrefs=100 main
	pdflatex main
	pdflatex main

clean:
	rm -f main.pdf main.aux main.bbl main.blg main.log main.out

SPELLTEX := $(shell ./bin/get-tex-files.sh main.tex) main.bbl
spell:
	@ for i in $(SPELLTEX); do aspell --mode=tex -p ./aspell.words -c $$i; done
	@ for i in $(SPELLTEX); do perl bin/double.pl $$i; done
	@ for i in $(SPELLTEX); do perl bin/abbrv.pl  $$i; done
	@ bash bin/hyphens.sh $(SPELLTEX)
	@ ( head -1 aspell.words ; tail -n +2 aspell.words | LC_ALL=C sort ) > aspell.words~
	@ mv aspell.words~ aspell.words
.PHONY: spell

