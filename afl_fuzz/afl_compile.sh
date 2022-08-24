afl-clang -DUNISHOX_API_WITH_OUTPUT_LEN=1 $COMPILE_OPTS -fsanitize=address -static-libsan -static-libstdc++ ../unishox2.c test_fuzz.c

