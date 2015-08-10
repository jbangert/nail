all: main.pdf propcover.pdf
main.pdf: *.tex *.bib
	latexrun -o $@ main.tex
propcover.pdf: propcover.tex *.bib
	latexrun -o $@ $<
clean:
	rm propcover.pdf
