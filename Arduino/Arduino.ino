// Example sketch to demonstrate use of buffer size limiting for compression and decompression
#include "unishox2.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  char cbuf[8]; // Restrict buffer size to check that it does not overflow
  char dbuf[8];
  int clen, dlen;
  clen = unishox2_compress_lines("Hello World", 11, cbuf, sizeof(cbuf) - 1, USX_PSET_DFLT, NULL);
  clen = 8; // clen would have been -1 since buffer was not sufficient
  dlen = unishox2_decompress_lines(cbuf, clen, dbuf, sizeof(dbuf) - 1, USX_PSET_DFLT, NULL);
  dlen = 7;
  dbuf[dlen] = '\0';
  Serial.print("Decompressed output: ");
  Serial.println(dbuf);
}

void loop() {
  // put your main code here, to run repeatedly:

}
