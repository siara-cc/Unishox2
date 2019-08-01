SRCFILE = unishox_0_1.c
OUTFILE = unishox

default:
	gcc -o $(OUTFILE) $(SRCFILE)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
