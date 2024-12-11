SRCFILE = unishox3.cpp
SRCFILE1 = ../test_unishox3_beta.cpp
OUTFILE = ../u3b
COMPILE_OPTS=-O2 -g -I.
M_FLAGS=-mbmi2 -mpopcnt
# -fsanitize=address

default:
	g++ -std=c++11 $(CFLAGS) $(COMPILE_OPTS) -o $(OUTFILE) $(SRCFILE) $(SRCFILE1) -lmarisa $(M_FLAGS)

install: default
	cp $(OUTFILE) /usr/bin/

clean:
	$(RM) $(OUTFILE)
