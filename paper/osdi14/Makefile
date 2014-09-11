Q=@

main.pdf: $(wildcard *.tex) $(wildcard *.bib) $(wildcard code/*.tex)
	$(Q)PATH=$$PATH:. latexrun --bibtex-args=-min-crossrefs=100 main.tex

code/fmt.tex:
	pygmentize -f latex -S default \
		| grep -vw 'PY@tok@m' \
		| grep -vw 'PY@tok@mi' \
		| grep -vw 'PY@tok@o' \
		> $@~
	mv $@~ $@

code/%.tex: code/%.c
	pygmentize -P mathescape -f latex $< > $@

pygmentize: code/fmt.tex $(patsubst %.c,%.tex,$(wildcard code/*.c))

clean:
	rm -f main.pdf

SPELLTEX := $(shell ./bin/get-tex-files.sh main.tex) latex.out/main.bbl
spell:
	@ for i in $(SPELLTEX); do aspell -l en_us --mode=tex -p ./aspell.words -c $$i; done
	@ for i in $(SPELLTEX); do perl bin/double.pl $$i; done
	@ for i in $(SPELLTEX); do perl bin/abbrv.pl  $$i; done
	@ bash bin/hyphens.sh $(SPELLTEX)
	@ ( head -1 aspell.words ; tail -n +2 aspell.words | LC_ALL=C sort ) > aspell.words~
	@ mv aspell.words~ aspell.words
.PHONY: spell

