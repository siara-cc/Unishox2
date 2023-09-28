#include "../../../../Unishox3_Alpha/unishox3.h"

unishox3 usx3;
extern "C" {
int unishox3a_compress_simple(const char *in, int len, char *out) {
   return usx3.compress(in, len, out);
}

int unishox3a_decompress_simple(const char *in, int len, char *out) {
   return usx3.decompress(in, len, out);
}
}
