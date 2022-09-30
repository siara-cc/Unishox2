SRCFILE = unishox3.cpp
SRCFILE1 = ../test_unishox3.cpp
OUTFILE = usx3
COMPILE_OPTS=-O3 -I.
# -fsanitize=address

default:
	g++ -std=c++98 $(CFLAGS) $(COMPILE_OPTS) -o $(OUTFILE) $(SRCFILE) $(SRCFILE1)
	#g++ -std=c99 $(CFLAGS) $(COMPILE_OPTS) -DUNISHOX_API_WITH_OUTPUT_LEN=1 -o $(OUTFILE)-w-olen $(SRCFILE) $(SRCFILE1)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
