SRCFILE = unishox2.c
SRCFILE1 = test_unishox2.c
OUTFILE = test_unishox2

default:
	gcc -o $(OUTFILE) $(SRCFILE) $(SRCFILE1)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
