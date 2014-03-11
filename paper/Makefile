
main.pdf: main.tex *.bib 
	pdflatex $< 
#	for i in `echo *.aux`
#do
#	bibtex `basename $$i`
#done
	bibtex main
	pdflatex $<
	pdflatex $<

