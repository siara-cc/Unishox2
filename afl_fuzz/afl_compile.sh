afl-clang -DUNISHOX_API_WITH_OUTPUT_LEN=1 -g -fsanitize=address ../unishox2.c test_fuzz.c

