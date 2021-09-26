SRCFILE = unishox2.c
SRCFILE1 = test_unishox2.c
OUTFILE = test_unishox2

default:
	gcc $(CFLAGS) -o $(OUTFILE) $(SRCFILE) $(SRCFILE1)
	gcc $(CFLAGS) -DUNISHOX_API_WITH_OUTPUT_LEN=1 -o $(OUTFILE)-w-olen $(SRCFILE) $(SRCFILE1)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
