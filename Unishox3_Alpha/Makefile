SRCFILE = unishox3.cpp
# wordlist_bin.o
SRCFILE1 = ../test_unishox3_alpha.cpp
OUTFILE = ../usx3
COMPILE_OPTS=-O3 -I. -lmarisa
# -fsanitize=address

default:
#	ld -r -b binary -o wordlist_bin.o wordlist.bin
	g++ -std=c++98 $(CFLAGS) $(COMPILE_OPTS) -o $(OUTFILE) $(SRCFILE) $(SRCFILE1)
	#g++ -std=c99 $(CFLAGS) $(COMPILE_OPTS) -DUNISHOX_API_WITH_OUTPUT_LEN=1 -o $(OUTFILE)-w-olen $(SRCFILE) $(SRCFILE1)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
