/*
 * Copyright (C) 2020 Siara Logics (cc)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Arundale Ramanathan
 *
 */
/**
 * @file test_unishox2.c
 * @author Arundale Ramanathan, James Z. M. Gao
 * @brief Demo / Test program for Unishox2 Compression and Decompression
 *
 * This file run comprehensive tests on the Unishox2 API \n
 * It also provides command line options for demonstration \n
 * of its features.
 */
#include "unishox2.h"

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

/// Internal function to call compress function in unishox2.c
int unishox2_compress_preset_lines(const char *in, int len, UNISHOX_API_OUT_AND_LEN(char *out, int olen), int preset, struct us_lnk_lst *prev_lines) {
  switch (preset) {
    case 0:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_DFLT, prev_lines);
    case 1:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_ONLY, prev_lines);
    case 2:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_NUM_ONLY, prev_lines);
    case 3:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_NUM_SYM_ONLY, prev_lines);
    case 4:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_NUM_SYM_ONLY_TXT, prev_lines);
    case 5:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_ALPHA, prev_lines);
    case 6:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_DICT, prev_lines);
    case 7:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_SYM, prev_lines);
    case 8:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_UMLAUT, prev_lines);
    case 9:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_NO_DICT, prev_lines);
    case 10:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_NO_UNI, prev_lines);
    case 11:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_NO_UNI_FAVOR_TEXT, prev_lines);
    case 12:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_URL, prev_lines);
    case 13:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_JSON, prev_lines);
    case 14:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_JSON_NO_UNI, prev_lines);
    case 15:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_XML, prev_lines);
    case 16:
      return unishox2_compress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_HTML, prev_lines);
  }
  return 0;
}

/// Internal function to call decompress function in unishox2.c
int unishox2_decompress_preset_lines(const char *in, int len, UNISHOX_API_OUT_AND_LEN(char *out, int olen), int preset, struct us_lnk_lst *prev_lines) {
  switch (preset) {
    case 0:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_DFLT, prev_lines);
    case 1:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_ONLY, prev_lines);
    case 2:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_NUM_ONLY, prev_lines);
    case 3:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_NUM_SYM_ONLY, prev_lines);
    case 4:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_ALPHA_NUM_SYM_ONLY_TXT, prev_lines);
    case 5:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_ALPHA, prev_lines);
    case 6:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_DICT, prev_lines);
    case 7:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_SYM, prev_lines);
    case 8:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_FAVOR_UMLAUT, prev_lines);
    case 9:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_NO_DICT, prev_lines);
    case 10:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_NO_UNI, prev_lines);
    case 11:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_NO_UNI_FAVOR_TEXT, prev_lines);
    case 12:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_URL, prev_lines);
    case 13:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_JSON, prev_lines);
    case 14:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_JSON_NO_UNI, prev_lines);
    case 15:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_XML, prev_lines);
    case 16:
      return unishox2_decompress_lines(in, len, UNISHOX_API_OUT_AND_LEN(out, olen), USX_PSET_HTML, prev_lines);
  }
  return 0;
}

int test_ushx_cd_with_len(char *input, int len, int preset) {

  char cbuf[200];
  char dbuf[251];
  int clen = unishox2_compress_preset_lines(input, len, UNISHOX_API_OUT_AND_LEN(cbuf, sizeof cbuf), preset, NULL);
  if (clen > (int)sizeof cbuf) {
    printf("Compress Overflow\n");
    return 0;
  }
  printf("\n\n");
  int dlen = unishox2_decompress_preset_lines(cbuf, clen, UNISHOX_API_OUT_AND_LEN(dbuf, sizeof dbuf), preset, NULL);
  if (dlen > (int)sizeof dbuf) {
    printf("Decompress Overflow\n");
    return 0;
  } else if (dlen < (int)sizeof dbuf)
    dbuf[dlen] = '\0';
  if (dlen != len) {
    dbuf[sizeof dbuf - 1] = '\0';
    printf("Fail len: %d, %d:\n%s\n%s\n", len, dlen, input, dbuf);
    return 0;
  }
  if (memcmp(input, dbuf, len)) {
    printf("Fail cmp:\n%s\n%s\n", input, dbuf);
    return 0;
  }
  float perc = (float)(len - clen);
  perc /= len;
  perc *= 100;
  printf("%s: %d/%d=", input, clen, len);
  printf("%.2f%%\n", perc);

#if (UNISHOX_API_OUT_AND_LEN(0,1)) == 1
  // check compress overflow
  for (int i = 1; i <= 16 && clen - i >= 0; ++i) {
    char cbuf_cut[sizeof cbuf];
    const int clen_cut = unishox2_compress_preset_lines(input, len, cbuf_cut, clen - i, preset, NULL);
    if (clen_cut != clen - i + 1) { // should overflow
      printf("Fail compress len overflow: %d, %d\n", clen - i, clen_cut);
      return 0;
    }
    if (memcmp(cbuf, cbuf_cut, clen - i)) {
      printf("Fail compress overflow cmp\n");
      return 0;
    }
  }

  // check decompress overflow
  for (int i = 1; i <= 16 && len - i >= 0; ++i) {
    memset(dbuf, 0, sizeof dbuf);
    dlen = unishox2_decompress_preset_lines(cbuf, clen, dbuf, len - i, preset, NULL);
    if (dlen != len - i + 1) { // should overflow
      dbuf[sizeof dbuf - 1] = '\0';
      printf("Fail decompress len overflow: %d, %d:\n%s\n%s\n", len, dlen, input, dbuf);
      return 0;
    }
    dbuf[len - i] = '\0';
    if (strncmp(input, dbuf, len - i)) {
      printf("Fail decompress overflow cmp:\n%s\n%s\n", input, dbuf);
      return 0;
    }
  }

  // test terminator, only valid when olen parameter is used in *_compress api
  char cbuf_term[sizeof cbuf + 6];
  const int clen_term_raw = unishox2_compress_preset_lines(input, len, UNISHOX_API_OUT_AND_LEN(cbuf_term, -(int)(sizeof cbuf_term)), preset, NULL);
  int clen_term = clen_term_raw / 4;
  const int clen_term_size = clen_term_raw % 4;
  if (clen_term > (int)sizeof cbuf_term) {
      printf("Fail, overflow for full term codes\n");
      return 0;
  }
  if (clen != clen_term - clen_term_size) {
    printf("Fail compress len with term codes: %d, %d\n", clen, clen_term - clen_term_size);
    return 0;
  }
  if (memcmp(cbuf, cbuf_term, clen)) {
    printf("Fail compress with term codes cmp\n");
    return 0;
  }
  if ((unsigned char)cbuf_term[clen_term-1] != (preset == 1 ? 0 : 0xFF)) {
    printf("term size = %d, last byte is not 0 or 0xFF: %X\n", clen_term_size, (unsigned char)cbuf[clen-1]);
    return 0;
  }
  cbuf_term[clen_term++] = cbuf_term[0];
  cbuf_term[clen_term++] = cbuf_term[1];
  cbuf_term[clen_term++] = cbuf_term[2];

  for (int i = 1; i <= 7 && clen + i <= (int)sizeof cbuf_term; ++i) {
    memset(dbuf, 0, sizeof(dbuf));
    dlen = unishox2_decompress_preset_lines(cbuf_term, clen + i, UNISHOX_API_OUT_AND_LEN(dbuf, sizeof dbuf), preset, NULL);
    if (dlen > (int)sizeof dbuf) {
      printf("Decompress Overflow for testing terminator\n");
      return 0;
    } else if (dlen < (int)sizeof dbuf)
      dbuf[dlen] = '\0';
    if (dlen != len) {
      dbuf[sizeof dbuf - 1] = '\0';
      printf("Fail len (term+%d): %d, %d:\n%s\n%s\n", i, len, dlen, input, dbuf);
      return 0;
    }
    if (strncmp(input, dbuf, len)) {
      printf("Fail cmp (term+%d):\n%s\n%s\n", i, input, dbuf);
      return 0;
    }
  }
#endif

  return 1;

}

/// Helper function for unit tests
int test_ushx_cd(char *input, int preset) {
  int len = (int)strlen(input);
  return test_ushx_cd_with_len(input, len, preset);
}

int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}

// From https://stackoverflow.com/questions/19758270/read-varint-from-linux-sockets#19760246
// Encode an unsigned 64-bit varint.  Returns number of encoded bytes.
// 'buffer' must have room for up to 10 bytes.
/// Internal helper function
int encode_unsigned_varint(uint8_t *buffer, uint64_t value) {
  int encoded = 0;
  do {
    uint8_t next_byte = value & 0x7F;
    value >>= 7;
    if (value)
      next_byte |= 0x80;
    buffer[encoded++] = next_byte;
  } while (value);
  return encoded;
}

/// Internal helper function
uint64_t decode_unsigned_varint(const uint8_t *data, int *decoded_bytes) {
  int i = 0;
  uint64_t decoded_value = 0;
  int shift_amount = 0;
  do {
    decoded_value |= (uint64_t)(data[i] & 0x7F) << shift_amount;     
    shift_amount += 7;
  } while ((data[i++] & 0x80) != 0);
  *decoded_bytes = i;
  return decoded_value;
}

/// Internal helper function
void print_string_as_hex(char *in, int len) {

  int l;
  printf("String in hex:\n");
  for (l=0; l<len; l++) {
    printf("%02x, ", (unsigned char) in[l]);
  }
  printf("\n");
}

/// Internal helper function
void print_bytes(char *in, int len, const char *title) {

  int l;
  uint8_t bit;
  printf("%s %d bytes\n", title, len);
  printf("Bytes in decimal:\n");
  for (l=0; l<len; l++)
    printf("%u, ", (unsigned char) in[l]);
  printf("\nBytes in hex:\n");
  for (l=0; l<len; l++)
    printf("\\x%02x", (unsigned char) in[l]);
  printf("\nBytes in binary:\n");
  for (l=0; l<len*8; l++) {
    bit = (in[l/8]>>(7-l%8))&0x01;
    printf("%d", bit);
    if (l%8 == 7) printf(" ");
  }
  printf("\n");

}

/// Internal helper function
uint32_t getTimeVal() {
#ifdef _MSC_VER
    return GetTickCount() * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000) + tv.tv_usec;
#endif
}

/// Internal helper function
double timedifference(uint32_t t0, uint32_t t1) {
    double ret = t1;
    ret -= t0;
    ret /= 1000;
    return ret;
}

/// Internal helper function
int presetForUnicode(int preset) {
  switch (preset) {
  case  1: return 0; // false
  case  2: return 0; // false
  case  3: return 0; // false
  case  4: return 0; // false
  case 10: return 0; // false
  case 11: return 0; // false
  case 14: return 0; // false
  default: break;
  }

  return  1; // true
}

/// This is the unit-test function
int run_unit_tests(int argc, char *argv[]) {

   int preset = 0;
   if (argc > 2)
     preset = atoi(argv[2]);

   if (preset < 0 || 16 < preset) {
     printf("invalid preset %d\n", preset);
     return 1;
   }

   // check template
   {
     char cbuf[128];
     char dbuf[128];
     char *hex = ":AAAAAA-bbbbbb";
     const int len = strlen(hex);
     const int clen = unishox2_compress_lines(hex, len, UNISHOX_API_OUT_AND_LEN(cbuf, sizeof cbuf), USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_DFLT, (const char *[]){":FFFFFF", "-ffffff", 0, 0, 0} , NULL);
     const int dlen = unishox2_decompress_lines(cbuf, clen, UNISHOX_API_OUT_AND_LEN(dbuf, sizeof dbuf), USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_DFLT, (const char *[]){":FFFFFF", "-ffffff", 0, 0, 0} , NULL);
     if (dlen != len) {
       printf("Fail len (template): %d, %d:\n%s\n%s\n", len, dlen, hex, dbuf);
       return 1;
     }
     if (strncmp(hex, dbuf, len)) {
       printf("Fail cmp (template):\n%s\n%s\n", hex, dbuf);
       return 0;
     }
   }

    // Basic
    if (!test_ushx_cd("Hello", preset)) return 1;
    if (!test_ushx_cd("Hello World", preset)) return 1;
    if (!test_ushx_cd("The quick brown fox jumped over the lazy dog", preset)) return 1;
    if (!test_ushx_cd("HELLO WORLD", preset)) return 1;
    if (!test_ushx_cd("HELLO WORLD HELLO WORLD", preset)) return 1;

    // Numbers
    if (preset != 1 && !test_ushx_cd("Hello1", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Hello1 World2", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Hello123", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("12345678", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("12345678 12345678", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("HELLO WORLD 1234 hello world12", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("HELLO 234 WORLD", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("9 HELLO, WORLD", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("H1e2l3l4o5 w6O7R8L9D", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("8+80=88", preset)) return 1;

    // Symbols
    if (preset != 1 && preset != 2 && !test_ushx_cd("~!@#$%^&*()_+=-`;'\\|\":,./?><", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("if (!test_ushx_cd(\"H1e2l3l4o5 w6O7R8L9D\", preset)) return 1;", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("Hello\tWorld\tHow\tare\tyou?", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("Hello~World~How~are~you?", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("Hello\rWorld\rHow\rare\ryou?", preset)) return 1;

    // Repeat
    if (preset != 1 && !test_ushx_cd("-----------------///////////////", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("-----------------Hello World1111111111112222222abcdef12345abcde1234_////////Hello World///////", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("-----------------///////////////", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("------------------------------------", preset)) return 1;

    // Nibbles
    if (preset != 1 && !test_ushx_cd("fa01b51e-7ecc-4e3e-be7b-918a4c2c891c", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Fa01b51e-7ecc-4e3e-be7b-918a4c2c891c", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("fa01b51e-7ecc-4e3e-be7b-9182c891c", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("760FBCA3-272E-4F1A-BF88-8472DF6BD994", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("760FBCA3-272E-4F1A-BF88-8472DF6Bd994", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("760FBCA3-272E-4F1A-BF88-8472DF6Bg994", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("FBCA3-272E-4F1A-BF88-8472DF6BD994", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Hello 1 5347a688-d8bf-445d-86d1-b470f95b007fHello World", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("01234567890123", preset)) return 1;

    // Templates
    if (preset != 1 && !test_ushx_cd("2020-12-31", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("1934-02", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("2020-12-31T12:23:59.234Z", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("1899-05-12T23:59:59.23434", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("1899-05-12T23:59:59", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("2020-12-31T12:23:59.234Zfa01b51e-7ecc-4e3e-be7b-918a4c2c891c", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("顔に(993) 345-3495あり", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("HELLO(993) 345-3495WORLD", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("顔に1899-05-12T23:59:59あり", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("HELLO1899-05-12T23:59:59WORLD", preset)) return 1;

    if (preset != 1 && !test_ushx_cd("Cada buhonero alaba sus agujas. - A peddler praises his needles (wares).", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Cada gallo canta en su muladar. - Each rooster sings on its dung-heap.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Cada martes tiene su domingo. - Each Tuesday has its Sunday.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Cada uno habla de la feria como le va en ella. - Our way of talking about things reflects our relevant experience, good or bad.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("Dime con quien andas y te diré quién eres.. - Tell me who you walk with, and I will tell you who you are.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Donde comen dos, comen tres. - You can add one person more in any situation you are managing.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("El amor es ciego. - Love is blind", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("El amor todo lo iguala. - Love smoothes life out.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("El tiempo todo lo cura. - Time cures all.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("La avaricia rompe el saco. - Greed bursts the sack.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("La cara es el espejo del alma. - The face is the mirror of the soul.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("La diligencia es la madre de la buena ventura. - Diligence is the mother of good fortune.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("La fe mueve montañas. - Faith moves mountains.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("La mejor palabra siempre es la que queda por decir. - The best word is the one left unsaid.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("La peor gallina es la que más cacarea. - The worst hen is the one that clucks the most.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("La sangre sin fuego hierve. - Blood boils without fire.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("La vida no es un camino de rosas. - Life is not a path of roses.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Las burlas se vuelven veras. - Bad jokes become reality.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Las desgracias nunca vienen solas. - Misfortunes never come one at a time.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Lo comido es lo seguro. - You can only be really certain of what is already in your belly.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("Los años no pasan en balde. - Years don't pass in vain.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Los celos son malos consejeros. - Jealousy is a bad counsellor.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Los tiempos cambian. - Times change.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("Mañana será otro día. - Tomorrow will be another day.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("Ningún jorobado ve su joroba. - No hunchback sees his own hump.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("No cantan dos gallos en un gallinero. - Two roosters do not crow in a henhouse.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("No hay harina sin salvado. - No flour without bran.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("No por mucho madrugar, amanece más temprano.. - No matter if you rise early because it does not sunrise earlier.", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("No se puede hacer tortilla sin romper los huevos. - One can't make an omelette without breaking eggs.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("No todas las verdades son para dichas. - Not every truth should be said.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("No todo el monte es orégano. - The whole hillside is not covered in spice.", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("Nunca llueve a gusto de todos. - It never rains to everyone's taste.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Perro ladrador, poco mordedor.. - A dog that barks often seldom bites.", preset)) return 1;
    if (preset != 1 && !test_ushx_cd("Todos los caminos llevan a Roma. - All roads lead to Rome.", preset)) return 1;

    // Unicode
    if (presetForUnicode(preset) && !test_ushx_cd("案ずるより産むが易し。 - Giving birth to a baby is easier than worrying about it.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("出る杭は打たれる。 - The stake that sticks up gets hammered down.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("知らぬが仏。 - Not knowing is Buddha. - Ignorance is bliss.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("見ぬが花。 - Not seeing is a flower. - Reality can't compete with imagination.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("花は桜木人は武士 - Of flowers, the cherry blossom; of men, the warrior.", preset)) return 1;

    if (presetForUnicode(preset) && !test_ushx_cd("小洞不补，大洞吃苦 - A small hole not mended in time will become a big hole much more difficult to mend.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("读万卷书不如行万里路 - Reading thousands of books is not as good as traveling thousands of miles", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("福无重至,祸不单行 - Fortune does not come twice. Misfortune does not come alone.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("风向转变时,有人筑墙,有人造风车 - When the wind changes, some people build walls and have artificial windmills.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("父债子还 - Father's debt, son to give back.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("害人之心不可有 - Do not harbour intentions to hurt others.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("今日事，今日毕 - Things of today, accomplished today.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("空穴来风,未必无因 - Where there's smoke, there's fire.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("良药苦口 - Good medicine tastes bitter.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("人算不如天算 - Man proposes and God disposes", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("师傅领进门，修行在个人 - Teachers open the door. You enter by yourself.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("授人以鱼不如授之以渔 - Teach a man to take a fish is not equal to teach a man how to fish.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("树倒猢狲散 - When the tree falls, the monkeys scatter.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("水能载舟，亦能覆舟 - Not only can water float a boat, it can sink it also.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("朝被蛇咬，十年怕井绳 - Once bitten by a snake for a snap dreads a rope for a decade.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("一分耕耘，一分收获 - If one does not plow, there will be no harvest.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("有钱能使鬼推磨 - If you have money you can make the devil push your grind stone.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("一失足成千古恨，再回头已百年身 - A single slip may cause lasting sorrow.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("自助者天助 - Those who help themselves, God will help.", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("早起的鸟儿有虫吃 - Early bird gets the worm.", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("This is first line,\r\nThis is second line", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("{\"menu\": {\n  \"id\": \"file\",\n  \"value\": \"File\",\n  \"popup\": {\n    \"menuitem\": [\n      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},\n      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},\n      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}\n    ]\n  }\n}}", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("{\"menu\": {\r\n  \"id\": \"file\",\r\n  \"value\": \"File\",\r\n  \"popup\": {\r\n    \"menuitem\": [\r\n      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},\r\n      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},\r\n      {\"value\":\"Close\", \"onclick\": \"CloseDoc()\"}\r\n    ]\r\n  }\r\n}}", preset)) return 1;
    if (preset != 1 && preset != 2 && !test_ushx_cd("https://siara.cc", preset)) return 1;

    if (presetForUnicode(preset) && !test_ushx_cd("符号\"δ\"表", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("学者地”[3]。学者", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("한데......아무", preset)) return 1;

    // English
    if (preset != 1 && !test_ushx_cd("Beauty is not in the face. Beauty is a light in the heart.", preset)) return 1;
    // Spanish
    if (presetForUnicode(preset) && !test_ushx_cd("La belleza no está en la cara. La belleza es una luz en el corazón.", preset)) return 1;
    // French
    if (presetForUnicode(preset) && !test_ushx_cd("La beauté est pas dans le visage. La beauté est la lumière dans le coeur.", preset)) return 1;
    // Portugese
    if (presetForUnicode(preset) && !test_ushx_cd("A beleza não está na cara. A beleza é a luz no coração.", preset)) return 1;
    // Dutch
    if (preset != 1 && !test_ushx_cd("Schoonheid is niet in het gezicht. Schoonheid is een licht in het hart.", preset)) return 1;

    // German
    if (presetForUnicode(preset) && !test_ushx_cd("Schönheit ist nicht im Gesicht. Schönheit ist ein Licht im Herzen.", preset)) return 1;
    // Spanish
    if (presetForUnicode(preset) && !test_ushx_cd("La belleza no está en la cara. La belleza es una luz en el corazón.", preset)) return 1;
    // French
    if (presetForUnicode(preset) && !test_ushx_cd("La beauté est pas dans le visage. La beauté est la lumière dans le coeur.", preset)) return 1;
    // Italian
    if (presetForUnicode(preset) && !test_ushx_cd("La bellezza non è in faccia. La bellezza è la luce nel cuore.", preset)) return 1;
    // Swedish
    if (presetForUnicode(preset) && !test_ushx_cd("Skönhet är inte i ansiktet. Skönhet är ett ljus i hjärtat.", preset)) return 1;
    // Romanian
    if (presetForUnicode(preset) && !test_ushx_cd("Frumusețea nu este în față. Frumusețea este o lumină în inimă.", preset)) return 1;
    // Ukranian
    if (presetForUnicode(preset) && !test_ushx_cd("Краса не в особі. Краса - це світло в серці.", preset)) return 1;
    // Greek
    if (presetForUnicode(preset) && !test_ushx_cd("Η ομορφιά δεν είναι στο πρόσωπο. Η ομορφιά είναι ένα φως στην καρδιά.", preset)) return 1;
    // Turkish
    if (presetForUnicode(preset) && !test_ushx_cd("Güzellik yüzünde değil. Güzellik, kalbin içindeki bir ışıktır.", preset)) return 1;
    // Polish
    if (presetForUnicode(preset) && !test_ushx_cd("Piękno nie jest na twarzy. Piękno jest światłem w sercu.", preset)) return 1;

    // Africans
    if (preset != 1 && preset != 2 && !test_ushx_cd("Skoonheid is nie in die gesig nie. Skoonheid is 'n lig in die hart.", preset)) return 1;
    // Swahili
    if (preset != 1 && !test_ushx_cd("Beauty si katika uso. Uzuri ni nuru moyoni.", preset)) return 1;
    // Zulu
    if (preset != 1 && !test_ushx_cd("Ubuhle abukho ebusweni. Ubuhle bungukukhanya enhliziyweni.", preset)) return 1;
    // Somali
    if (preset != 1 && !test_ushx_cd("Beauty ma aha in wajiga. Beauty waa iftiin ah ee wadnaha.", preset)) return 1;

    // Russian
    if (preset != 1 && !test_ushx_cd("Красота не в лицо. Красота - это свет в сердце.", preset)) return 1;
    // Arabic
    if (presetForUnicode(preset) && !test_ushx_cd("الجمال ليس في الوجه. الجمال هو النور الذي في القلب.", preset)) return 1;
    // Persian
    if (presetForUnicode(preset) && !test_ushx_cd("زیبایی در چهره نیست. زیبایی نور در قلب است.", preset)) return 1;
    // Pashto
    if (presetForUnicode(preset) && !test_ushx_cd("ښکلا په مخ کې نه ده. ښکلا په زړه کی یوه رڼا ده.", preset)) return 1;
    // Azerbaijani
    if (presetForUnicode(preset) && !test_ushx_cd("Gözəllik üzdə deyil. Gözəllik qəlbdə bir işıqdır.", preset)) return 1;
    // Uzbek
    if (preset != 1 && preset != 2 && !test_ushx_cd("Go'zallik yuzida emas. Go'zallik - qalbdagi nur.", preset)) return 1;
    // Kurdish
    if (presetForUnicode(preset) && !test_ushx_cd("Bedewî ne di rû de ye. Bedewî di dil de ronahiyek e.", preset)) return 1;
    // Urdu
    if (presetForUnicode(preset) && !test_ushx_cd("خوبصورتی چہرے میں نہیں ہے۔ خوبصورتی دل میں روشنی ہے۔", preset)) return 1;

    // Hindi
    if (presetForUnicode(preset) && !test_ushx_cd("सुंदरता चेहरे में नहीं है। सौंदर्य हृदय में प्रकाश है।", preset)) return 1;
    // Bangla
    if (presetForUnicode(preset) && !test_ushx_cd("সৌন্দর্য মুখে নেই। সৌন্দর্য হৃদয় একটি আলো।", preset)) return 1;
    // Punjabi
    if (presetForUnicode(preset) && !test_ushx_cd("ਸੁੰਦਰਤਾ ਚਿਹਰੇ ਵਿੱਚ ਨਹੀਂ ਹੈ. ਸੁੰਦਰਤਾ ਦੇ ਦਿਲ ਵਿਚ ਚਾਨਣ ਹੈ.", preset)) return 1;
    // Telugu
    if (presetForUnicode(preset) && !test_ushx_cd("అందం ముఖంలో లేదు. అందం హృదయంలో ఒక కాంతి.", preset)) return 1;
    // Tamil
    if (presetForUnicode(preset) && !test_ushx_cd("அழகு முகத்தில் இல்லை. அழகு என்பது இதயத்தின் ஒளி.", preset)) return 1;
    // Marathi
    if (presetForUnicode(preset) && !test_ushx_cd("सौंदर्य चेहरा नाही. सौंदर्य हे हृदयातील एक प्रकाश आहे.", preset)) return 1;
    // Kannada
    if (presetForUnicode(preset) && !test_ushx_cd("ಸೌಂದರ್ಯವು ಮುಖದ ಮೇಲೆ ಇಲ್ಲ. ಸೌಂದರ್ಯವು ಹೃದಯದಲ್ಲಿ ಒಂದು ಬೆಳಕು.", preset)) return 1;
    // Gujarati
    if (presetForUnicode(preset) && !test_ushx_cd("સુંદરતા ચહેરા પર નથી. સુંદરતા હૃદયમાં પ્રકાશ છે.", preset)) return 1;
    // Malayalam
    if (presetForUnicode(preset) && !test_ushx_cd("സൗന്ദര്യം മുഖത്ത് ഇല്ല. സൗന്ദര്യം ഹൃദയത്തിലെ ഒരു പ്രകാശമാണ്.", preset)) return 1;
    // Nepali
    if (presetForUnicode(preset) && !test_ushx_cd("सौन्दर्य अनुहारमा छैन। सौन्दर्य मुटुको उज्यालो हो।", preset)) return 1;
    // Sinhala
    if (presetForUnicode(preset) && !test_ushx_cd("රූපලාවන්ය මුහුණේ නොවේ. රූපලාවන්ය හදවත තුළ ඇති ආලෝකය වේ.", preset)) return 1;

    // Chinese
    if (presetForUnicode(preset) && !test_ushx_cd("美是不是在脸上。 美是心中的亮光。", preset)) return 1;
    // Javanese
    if (preset != 1 && !test_ushx_cd("Beauty ora ing pasuryan. Kaendahan iku cahya ing sajroning ati.", preset)) return 1;
    // Japanese
    if (presetForUnicode(preset) && !test_ushx_cd("美は顔にありません。美は心の中の光です。", preset)) return 1;
    // Filipino
    if (preset != 1 && !test_ushx_cd("Ang kagandahan ay wala sa mukha. Ang kagandahan ay ang ilaw sa puso.", preset)) return 1;
    // Korean
    if (presetForUnicode(preset) && !test_ushx_cd("아름다움은 얼굴에 없습니다。아름다움은 마음의 빛입니다。", preset)) return 1;
    // Vietnam
    if (presetForUnicode(preset) && !test_ushx_cd("Vẻ đẹp không nằm trong khuôn mặt. Vẻ đẹp là ánh sáng trong tim.", preset)) return 1;
    // Thai
    if (presetForUnicode(preset) && !test_ushx_cd("ความงามไม่ได้อยู่ที่ใบหน้า ความงามเป็นแสงสว่างในใจ", preset)) return 1;
    // Burmese
    if (presetForUnicode(preset) && !test_ushx_cd("အလှအပမျက်နှာပေါ်မှာမဟုတ်ပါဘူး။ အလှအပစိတ်နှလုံးထဲမှာအလင်းကိုဖြစ်ပါတယ်။", preset)) return 1;
    // Malay
    if (preset != 1 && !test_ushx_cd("Kecantikan bukan di muka. Kecantikan adalah cahaya di dalam hati.", preset)) return 1;

    // Emoji
    if (presetForUnicode(preset) && !test_ushx_cd("🤣🤣🤣🤣🤣🤣🤣🤣🤣🤣🤣", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd("😀😃😄😁😆😅🤣😂🙂🙃😉😊😇🥰😍🤩😘😗😚😙😋😛😜🤪😝🤑🤗🤭🤫🤔🤐🤨😐😑😶😏😒🙄😬🤥😌😔😪🤤😴😷🤒🤕🤢", preset)) return 1;

    // Binary
    if (presetForUnicode(preset) && !test_ushx_cd("Hello\x80\x83\xAE\xBC\xBD\xBE", preset)) return 1;
    if (presetForUnicode(preset) && !test_ushx_cd_with_len("Hello world\x0 with nulls\x0", 24, preset)) return 1;

    return 0;
}

/**
 * <pre>
 * Usage: test_unishox2 \"string\" [preset_number]
 *               (or)
 *        test_unishox2 [action] [in_file] [out_file] [preset_number]
 *
 *          action:
 *          -t    run tests
 *          -c    compress
 *          -d    decompress
 *          -g    generate C header file
 *          -G    generate C header file using additional compression (slower)
 *
 *          preset_number:
 *          0    Optimum - favors all including JSON, XML, URL and HTML (default)
 *          1    Alphabets [a-z], [A-Z] and space only
 *          2    Alphanumeric [a-z], [A-Z], [0-9], [.,/()-=+$%%#] and space only
 *          3    Alphanumeric and symbols only
 *          4    Alphanumeric and symbols only (Favor English text)
 *          5    Favor Alphabets
 *          6    Favor Dictionary coding
 *          7    Favor Symbols
 *          8    Favor Umlaut
 *          9    No dictionary
 *          10   No Unicode
 *          11   No Unicode, favour English text
 *          12   Favor URLs
 *          13   Favor JSON
 *          14   Favor JSON (No Unicode)
 *          15   Favor XML
 *          16   Favor HTML
 * </pre>
 */
int main(int argc, char *argv[]) {

char cbuf[4096];
char dbuf[8192];
long len, tot_len, clen, ctot=0;
size_t dlen;
float perc=0.F;
FILE *fp, *wfp;
int bytes_read;
uint32_t tStart;

tStart = getTimeVal();

if (argc >= 4 && strcmp(argv[1], "-c") == 0) {
   int preset = 0;
   if (argc > 4)
     preset = atoi(argv[4]);
   tot_len = 0;
   fp = fopen(argv[2], "rb");
   if (fp == NULL) {
      perror(argv[2]);
      return 1;
   }
   wfp = fopen(argv[3], "wb");
   if (wfp == NULL) {
      perror(argv[3]);
      return 1;
   }
   do {
     bytes_read = (int)fread(cbuf, 1, sizeof(cbuf), fp);
     if (bytes_read > 0) {
        clen = unishox2_compress_preset_lines(cbuf, bytes_read, UNISHOX_API_OUT_AND_LEN(dbuf, sizeof dbuf), preset, NULL);
        ctot += clen;
        tot_len += bytes_read;
        if (clen > 0) {
           fputc(clen >> 8, wfp);
           fputc(clen & 0xFF, wfp);
           if (clen != (long)fwrite(dbuf, 1, clen, wfp)) {
              perror("fwrite");
              return 1;
           }
        }
     }
   } while (bytes_read > 0);
   perc = (float)(tot_len-ctot);
   perc /= tot_len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, tot_len);
   printf("%.2f%%\n", perc);
} else
if (argc >= 4 && strcmp(argv[1], "-d") == 0) {
   int preset = 0;
   if (argc > 4)
     preset = atoi(argv[4]);
   fp = fopen(argv[2], "rb");
   if (fp == NULL) {
      perror(argv[2]);
      return 1;
   }
   wfp = fopen(argv[3], "wb");
   if (wfp == NULL) {
      perror(argv[3]);
      return 1;
   }
   do {
     //memset(dbuf, 0, sizeof(dbuf));
     if (feof(fp))
       break;
     int len_to_read = fgetc(fp) << 8;
     if (feof(fp))
       break;
     len_to_read += fgetc(fp);
     if (feof(fp))
       break;
     bytes_read = (int)fread(dbuf, 1, len_to_read, fp);
     if (bytes_read > 0) {
        dlen = unishox2_decompress_preset_lines(dbuf, bytes_read, UNISHOX_API_OUT_AND_LEN(cbuf, sizeof cbuf), preset, NULL);
        if (dlen > 0) {
           if (dlen != fwrite(cbuf, 1, dlen, wfp)) {
              perror("fwrite");
              return 1;
           }
        }
     }
   } while (bytes_read > 0);
} else
if (argc >= 4 && (strcmp(argv[1], "-g") == 0 || 
      strcmp(argv[1], "-G") == 0 ||
      strcmp(argv[1], "-gb") == 0)) {
   int preset = 0;
   if (argc > 4)
     preset = atoi(argv[4]);
   if (strcmp(argv[1], "-g") == 0)
     preset = 9; // = USX_PSET_NO_DICT;
   fp = fopen(argv[2], "r");
   if (fp == NULL) {
      perror(argv[2]);
      return 1;
   }
   sprintf(cbuf, "%s.h", argv[3]);
   wfp = fopen(cbuf, "w");
   if (wfp == NULL) {
      perror(argv[3]);
      return 1;
   }
   tot_len = 0;
   ctot = 0;
   struct us_lnk_lst *cur_line = NULL;
   if (strcmp(argv[1], "-gb") != 0) {
     fputs("#ifndef __", wfp);
     fputs(argv[3], wfp);
     fputs("_UNISHOX2_COMPRESSED__\n", wfp);
     fputs("#define __", wfp);
     fputs(argv[3], wfp);
     fputs("_UNISHOX2_COMPRESSED__\n", wfp);
   }
   int line_ctr = 0;
   int max_len = 0;
   const size_t short_buf_len = strlen(argv[3]) + 100;
   char* short_buf = malloc(short_buf_len);
   while (fgets(cbuf, sizeof(cbuf), fp) != NULL) {
      // compress the line and look in previous lines
      // add to linked list
      len = (long)strlen(cbuf);
      if (cbuf[len - 1] == '\n' || cbuf[len - 1] == '\r') {
         len--;
         cbuf[len] = 0;
      }
      if (is_empty(cbuf))
        continue;
      if (len > 0) {
        struct us_lnk_lst *ll;
        ll = cur_line;
        cur_line = (struct us_lnk_lst *) malloc(sizeof(struct us_lnk_lst));
        cur_line->data = (char *) malloc(len + 1);
        strncpy(cur_line->data, cbuf, len);
        cur_line->previous = ll;
        clen = unishox2_compress_preset_lines(cbuf, len, UNISHOX_API_OUT_AND_LEN(dbuf, sizeof dbuf), preset, cur_line);
        if (clen > 0) {
            perc = (float)(len-clen);
            perc /= len;
            perc *= 100;
            //print_compressed(dbuf, clen);
            //printf("len: %ld/%ld=", clen, len);
            //printf("%.2f %s\n", perc, cbuf);
            tot_len += len;
            ctot += clen;
            if (strcmp(argv[1], "-gb") != 0) {
              snprintf(short_buf, short_buf_len, "const uint8_t %s_%d[] PROGMEM = {", argv[3], line_ctr++);
              fputs(short_buf, wfp);
              int len_len = encode_unsigned_varint((uint8_t *) short_buf, clen);
              for (int i = 0; i < len_len; i++) {
                snprintf(short_buf, 10, "%u, ", (uint8_t) short_buf[i]);
                fputs(short_buf, wfp);
              }
              for (int i = 0; i < clen; i++) {
                if (i) {
                  strcpy(short_buf, ", ");
                  fputs(short_buf, wfp);
                }
                snprintf(short_buf, 6, "%u", (uint8_t) dbuf[i]);
                fputs(short_buf, wfp);
              }
              strcpy(short_buf, "};\n");
              fputs(short_buf, wfp);
            } else {
              fwrite(dbuf, 1, clen, wfp);
              fwrite("\r\n", 1, 2, wfp);
            }
        }
        if (len > max_len)
          max_len = len;
        dlen = unishox2_decompress_preset_lines(dbuf, clen, UNISHOX_API_OUT_AND_LEN(cbuf, sizeof cbuf - 1), preset, cur_line);
        cbuf[dlen] = 0;
        //printf("\n%s\n", cbuf);
      }
   }
   perc = (float)(tot_len-ctot);
   perc /= tot_len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, tot_len);
   printf("%.2f%%\n", perc);
   if (strcmp(argv[1], "-gb") != 0) {
     snprintf(short_buf, short_buf_len, "const uint8_t * const %s[] PROGMEM = {", argv[3]);
     fputs(short_buf, wfp);
   }
   for (int i = 0; i < line_ctr; i++) {
     if (i) {
       strcpy(short_buf, ", ");
       fputs(short_buf, wfp);
     }
     snprintf(short_buf, strlen(argv[3]) + 15, "%s_%d", argv[3], i);
     fputs(short_buf, wfp);
   }
   strcpy(short_buf, "};\n");
   if (strcmp(argv[1], "-gb") != 0) {
     fputs(short_buf, wfp);
     snprintf(short_buf, short_buf_len, "#define %s_line_count %d\n", argv[3], line_ctr);
     fputs(short_buf, wfp);
     snprintf(short_buf, short_buf_len, "#define %s_max_len %d\n", argv[3], max_len);
     fputs(short_buf, wfp);
     fputs("#endif\n", wfp);
   }
   free(short_buf);
} else
if (argc >= 2 && strcmp(argv[1], "-t") == 0) {
  return run_unit_tests(argc, argv);
} else
if (argc == 4 && strcmp(argv[1], "-di") == 0) {
  char *input = argv[2];
  int clen = atoi(argv[3]);
  //char *input = "\252!\355\347;멠<\322\336\346\070\205X\200v\367b\002\332l\213\022\n\003P\374\267\002\266e\207.\210r:\021\225\224\243\353\204\305\352\255\017L/(HH4i\223~\270-\223\206\221\246\212\261\221e\254\375\341\350\037\240X\211ǉ\325\330u\365\303ʂ\200гM\236&\375\377\071%'?V\025\070\374\026\346s\323$\276\350F\224\r-\226\347ɋ\317\344\214\v\032U\303\353\215\335GX\202\371B\302\355\a\247\273\356C\372\a-\262\006\\\343\"ZH|\357\034\001";
  //clen = strlen(input);
  //print_bytes(argv[2], clen, "Input:");
  print_bytes(input, clen, "Input:");
  dlen = unishox2_decompress_simple(input, clen, dbuf);
  dbuf[dlen] = '\0';
  printf("Decompressed: [%s], len: %zu", dbuf, dlen);
} else
if (argc == 2 || (argc == 3 && atoi(argv[2]) > 0)) {
   int preset = 0;
   if (argc >= 3)
     preset = atoi(argv[2]);
   len = (long)strlen(argv[1]);
   printf("String: %s, Len:%ld\n", argv[1], len);
   //print_string_as_hex(argv[1], len);
   memset(cbuf, 0, sizeof(cbuf));
   ctot = unishox2_compress_preset_lines(argv[1], len, UNISHOX_API_OUT_AND_LEN(cbuf, sizeof cbuf), preset, NULL);
   print_bytes(cbuf, ctot, "Coompressed:");
   memset(dbuf, 0, sizeof(dbuf));
   dlen = unishox2_decompress_preset_lines(cbuf, ctot, UNISHOX_API_OUT_AND_LEN(dbuf, sizeof dbuf - 1), preset, NULL);
   dbuf[dlen] = 0;
   printf("\nDecompressed: %s\n", dbuf);
   if (strncmp(dbuf, argv[1], len))
     printf("\nERROR: DECOMPRESSED STRING DOES NOT MATCH ORIGINAL");
   //print_compressed(dbuf, dlen);
   perc = (float)(len-ctot);
   perc /= len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, len);
   printf("%.2f%%\n", perc);
} else {
   printf("Unishox (byte format version: %s)\n", UNISHOX_VERSION);
   printf("----------------------------------\n");
   printf("Usage: unishox2 \"string\" [preset_number]\n");
   printf("              (or)\n");
   printf("       unishox2 [action] [in_file] [out_file] [preset_number]\n");
   printf("\n");
   printf("         [action]:\n");
   printf("         -t    run tests\n");
   printf("         -c    compress\n");
   printf("         -d    decompress\n");
   printf("         -g    generate C header file\n");
   printf("         -G    generate C header file using additional compression (slower)\n");
   printf("\n");
   printf("         [preset_number]:\n");
   printf("         0    Optimum - favors all including JSON, XML, URL and HTML (default)\n");
   printf("         1    Alphabets [a-z], [A-Z] and space only\n");
   printf("         2    Alphanumeric [a-z], [A-Z], [0-9], [.,/()-=+$%%#] and space only\n");
   printf("         3    Alphanumeric and symbols only\n");
   printf("         4    Alphanumeric and symbols only (Favor English text)\n");
   printf("         5    Favor Alphabets\n");
   printf("         6    Favor Dictionary coding\n");
   printf("         7    Favor Symbols\n");
   printf("         8    Favor Umlaut\n");
   printf("         9    No dictionary\n");
   printf("         10   No Unicode\n");
   printf("         11   No Unicode, favour English text\n");
   printf("         12   Favor URLs\n");
   printf("         13   Favor JSON\n");
   printf("         14   Favor JSON (No Unicode)\n");
   printf("         15   Favor XML\n");
   printf("         16   Favor HTML\n");
   return 1;
}

printf("\nElapsed: %0.3lf ms\n", timedifference(tStart, getTimeVal()));

return 0;

}
