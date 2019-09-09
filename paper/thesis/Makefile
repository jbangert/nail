all: main.pdf propcover.pdf
main.pdf: *.tex *.bib
	$(Q)PATH=$$PATH:. latexrun -o $@ main.tex
propcover.pdf: propcover.tex *.bib
	$(Q)PATH=$$PATH:. latexrun -o $@ $<
clean:
	rm propcover.pdf
