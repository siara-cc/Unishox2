SRCFILE = unishox1.c
#SRCFILE1 = test_unishox2.c
OUTFILE = unishox1

default:
	gcc -o $(OUTFILE) $(SRCFILE)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
