
main.pdf: main.tex *.bib 
	pdflatex $< 
#	for i in `echo *.aux`
#do
#	bibtex `basename $$i`
#done
	bibtex -min-crossrefs=100 main
	pdflatex $<
	pdflatex $<

clean:
	rm -f main.pdf main.aux main.bbl main.blg main.log main.out

