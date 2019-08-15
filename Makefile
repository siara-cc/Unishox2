SRCFILE = unishox1.c
OUTFILE = unishox1

default:
	gcc -o $(OUTFILE) $(SRCFILE)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
