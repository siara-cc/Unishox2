/*
 * Copyright (C) 2020-22 Siara Logics (cc)
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
 * @file unishox3.c
 * @author Arundale Ramanathan, James Z. M. Gao
 * @brief Main code of Unishox2 Compression and Decompression library
 *
 * This file implements the code for the Unishox API function \n
 * defined in unishox3.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>

#include "unishox3.h"

#define HCODE_COUNT 6

enum {EX_NON_REENTRANT };

int short_count = 0;
int long_count = 0;

/// Vertical codes starting from the MSB
uint8_t usx_vcodes[]   = { 0x00, 0x40, 0x60, 0x80, 0x90, 0xA0, 0xB0,
                           0xC0, 0xD0, 0xD8, 0xE0, 0xE4, 0xE8, 0xEC,
                           0xEE, 0xF0, 0xF2, 0xF4, 0xF6, 0xF7, 0xF8,
                           0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

/// Length of each veritical code
uint8_t usx_vcode_lens[] = {  2,    3,    3,    4,    4,    4,    4,
                              4,    5,    5,    6,    6,    6,    7,
                              7,    7,    7,    7,    8,    8,    8,
                              8,    8,    8,    8,    8,    8,    8 };

/// Length of bits used to represent count for each level
const uint8_t count_bit_lens[5] = {2, 4, 7, 11, 16};
/// Cumulative counts represented at each level
const int32_t count_adder[5] = {4, 20, 148, 2196, 67732};
/// Codes used to specify the level that the count belongs to
const uint8_t count_codes[] = {0x02, 0x42, 0x82, 0xC3, 0xE3};
/// Encodes given count to out

/// Mask for retrieving each code to be encoded according to its length
const unsigned int usx_mask[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};

/// Length of bits used to represent delta code for each level
const uint8_t uni_bit_len[6] = {6, 12, 0, 14, 16, 21};
/// Cumulative delta codes represented at each level
const int32_t uni_adder[6] = {0, 64, 0, 4160, 20544, 86080};

/// possible horizontal sets and states
enum {USX_ALPHA = 0, USX_SYM, USX_NUM, USX_DICT, USX_DELTA, USX_PREDEF_DICT};

/// Set (USX_NUM - 2) and vertical code (26) for encoding repeating letters
#define RPT_CODE ((2 << 5) + 26)
/// Set (USX_NUM - 2) and vertical code (27) for encoding terminator
#define TERM_CODE ((2 << 5) + 27)
/// Set (USX_SYM - 1) and vertical code (7) for encoding Line feed \\n
#define LF_CODE ((1 << 5) + 7)
/// Set (USX_NUM - 1) and vertical code (8) for encoding \\r\\n
#define CRLF_CODE ((1 << 5) + 8)
/// Set (USX_NUM - 1) and vertical code (22) for encoding \\r
#define CR_CODE ((1 << 5) + 22)
/// Set (USX_NUM - 1) and vertical code (14) for encoding \\t
#define TAB_CODE  ((1 << 5) + 14)
/// Set (USX_NUM - 2) and vertical code (17) for space character when it appears in USX_NUM state \\r
#define NUM_SPC_CODE ((2 << 5) + 17)

/// Code for special code (110) when state=USX_DELTA
#define UNI_STATE_SPL_CODE 0xC0
/// Length of Code for special code when state=USX_DELTA
#define UNI_STATE_SPL_CODE_LEN 3
/// Code for switch code when state=USX_DELTA
#define UNI_STATE_SW_CODE 0x80
/// Length of Code for Switch code when state=USX_DELTA
#define UNI_STATE_SW_CODE_LEN 2

/// Switch code in USX_ALPHA and USX_NUM 00
#define SW_CODE 0
/// Length of Switch code
#define SW_CODE_LEN 2
/// Terminator bit sequence for Preset 1. Length varies depending on state as per following macros
#define TERM_BYTE_PRESET_1 0
/// Length of Terminator bit sequence when state is lower
#define TERM_BYTE_PRESET_1_LEN_LOWER 6
/// Length of Terminator bit sequence when state is upper
#define TERM_BYTE_PRESET_1_LEN_UPPER 4

/// Offset at which usx_code_94 starts
#define USX_OFFSET_94 33

/// This is a safe call to append_bits() making sure it does not write past olen
#define SAFE_APPEND_BITS(exp) do { \
  const int newidx = (exp); \
  if (newidx < 0) return newidx; \
} while (0)

/// Macro used in the main compress function so that if the output len exceeds given maximum length (olen) it can exit
#define SAFE_APPEND_BITS2(olen, exp) do { \
  const int newidx = (exp); \
  const int __olen = (olen); \
  if (newidx < 0) return __olen >= 0 ? __olen + 1 : (1 - __olen) * 4; \
} while (0)

/// Appends specified number of bits to the output (out) \n
/// If maximum limit (olen) is reached, -1 is returned \n
/// Otherwise clen bits in code are appended to out starting with MSB
int append_bits(char *out, int olen, int ol, uint8_t code, int clen) {

  uint8_t cur_bit;
  uint8_t blen;
  unsigned char a_byte;
  int oidx;

  //printf("%d,%x,%d,%d\n", ol, code, clen, state);

  while (clen > 0) {
     cur_bit = ol % 8;
     blen = clen;
     a_byte = code & usx_mask[blen - 1];
     a_byte >>= cur_bit;
     if (blen + cur_bit > 8)
        blen = (8 - cur_bit);
     oidx = ol / 8;
     if (olen <= oidx)
        return ol;
     out[oidx] = (out[oidx] & (0xFF00 >> cur_bit)) | a_byte;
     code <<= blen;
     ol += blen;
     clen -= blen;
   }
   return ol;
}

int encodeCount(char *out, int olen, int ol, int count) {
  // First five bits are code and Last three bits of codes represent length
  for (int i = 0; i < 5; i++) {
    if (count < count_adder[i]) {
      SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, (count_codes[i] & 0xF8), count_codes[i] & 0x07));
      uint16_t count16 = (count - (i ? count_adder[i - 1] : 0)) << (16 - count_bit_lens[i]);
      if (count_bit_lens[i] > 8) {
        SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, count16 >> 8, 8));
        SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, count16 & 0xFF, count_bit_lens[i] - 8));
      } else
        SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, count16 >> 8, count_bit_lens[i]));
      return ol;
    }
  }
  return ol;
}

/// Encodes the unicode code point given by code to out. prev_code is used to calculate the delta
int encodeUnicode(char *out, int olen, int ol, int32_t code, int32_t prev_code) {
  // First five bits are code and Last three bits of codes represent length
  //const uint8_t codes[8] = {0x00, 0x42, 0x83, 0xA3, 0xC3, 0xE4, 0xF5, 0xFD};
  const uint8_t codes[6] = {0x01, 0x82, 0xC3, 0xE4, 0xF5, 0xFD};
  int32_t till = 0;
  int32_t diff = code - prev_code;
  if (diff < 0)
    diff = -diff;
  //printf("%ld, ", code);
  //printf("Diff: %d\n", diff);
  for (int i = 0; i < 6; i++) {
    if (uni_bit_len[i] == 0) // skip spl. code if in the middle
      continue;
    till += (1 << uni_bit_len[i]);
    if (diff < till) {
      SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, (codes[i] & 0xF8), codes[i] & 0x07));
      //if (diff) {
        SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, prev_code > code ? 0x80 : 0, 1));
        int32_t val = diff - uni_adder[i];
        //printf("Val: %d\n", val);
        if (uni_bit_len[i] > 16) {
          val <<= (24 - uni_bit_len[i]);
          SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, val >> 16, 8));
          SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, (val >> 8) & 0xFF, 8));
          SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, val & 0xFF, uni_bit_len[i] - 16));
        } else
        if (uni_bit_len[i] > 8) {
          val <<= (16 - uni_bit_len[i]);
          SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, val >> 8, 8));
          SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, val & 0xFF, uni_bit_len[i] - 8));
        } else {
          val <<= (8 - uni_bit_len[i]);
          SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, val & 0xFF, uni_bit_len[i]));
        }
      return ol;
    }
  }
  return ol;
}

/// Reads UTF-8 character from in. Also returns the number of bytes occupied by the UTF-8 character in utf8len
int32_t readUTF8(const char *in, int len, int l, int *utf8len) {
  int32_t ret = 0;
  if (l < (len - 1) && (in[l] & 0xE0) == 0xC0 && (in[l + 1] & 0xC0) == 0x80) {
    *utf8len = 2;
    ret = (in[l] & 0x1F);
    ret <<= 6;
    ret += (in[l + 1] & 0x3F);
    if (ret < 0x80)
      ret = 0;
  } else
  if (l < (len - 2) && (in[l] & 0xF0) == 0xE0 && (in[l + 1] & 0xC0) == 0x80
          && (in[l + 2] & 0xC0) == 0x80) {
    *utf8len = 3;
    ret = (in[l] & 0x0F);
    ret <<= 6;
    ret += (in[l + 1] & 0x3F);
    ret <<= 6;
    ret += (in[l + 2] & 0x3F);
    if (ret < 0x0800)
      ret = 0;
  } else
  if (l < (len - 3) && (in[l] & 0xF8) == 0xF0 && (in[l + 1] & 0xC0) == 0x80
          && (in[l + 2] & 0xC0) == 0x80 && (in[l + 3] & 0xC0) == 0x80) {
    *utf8len = 4;
    ret = (in[l] & 0x07);
    ret <<= 6;
    ret += (in[l + 1] & 0x3F);
    ret <<= 6;
    ret += (in[l + 2] & 0x3F);
    ret <<= 6;
    ret += (in[l + 3] & 0x3F);
    if (ret < 0x10000)
      ret = 0;
  }
  return ret;
}

/// Returns minimum value of two longs
long min_of(long c, long i) {
  return c > i ? i : c;
}

/// Returns 4 bit code assuming ch falls between '0' to '9', \n
/// 'A' to 'F' or 'a' to 'f'
uint8_t getBaseCode(char ch) {
  if (ch >= '0' && ch <= '9')
    return (ch - '0') << 4;
  else if (ch >= 'A' && ch <= 'F')
    return (ch - 'A' + 10) << 4;
  else if (ch >= 'a' && ch <= 'f')
    return (ch - 'a' + 10) << 4;
  return 0;
}

/// Enum indicating nibble type - USX_NIB_NUM means ch is a number '0' to '9', \n
/// USX_NIB_HEX_LOWER means ch is between 'a' to 'f', \n
/// USX_NIB_HEX_UPPER means ch is between 'A' to 'F'
enum {USX_NIB_NUM = 0, USX_NIB_HEX_LOWER, USX_NIB_HEX_UPPER, USX_NIB_NOT};
/// Gets 4 bit code assuming ch falls between '0' to '9', \n
/// 'A' to 'F' or 'a' to 'f'
char getNibbleType(char ch) {
  if (ch >= '0' && ch <= '9')
    return USX_NIB_NUM;
  else if (ch >= 'a' && ch <= 'f')
    return USX_NIB_HEX_LOWER;
  else if (ch >= 'A' && ch <= 'F')
    return USX_NIB_HEX_UPPER;
  return USX_NIB_NOT;
}

int compare(const char *v1, uint8_t len1, const char *v2,
        uint8_t len2) {
    int k = 0;
    int lim = (len2 < len1 ? len2 : len1);
    while (k < lim) {
        uint8_t c1 = v1[k];
        uint8_t c2 = v2[k];
        if (k == 0 && c1 >= 'A' && c1 <= 'Z')
          c1 += ('a' - 'A');
        k++;
        if (c1 < c2)
            return -k;
        else if (c1 > c2)
            return k;
    }
    if (len1 == len2)
        return 0;
    k++;
    return (len1 < len2 ? -k : k);
}

int32_t binarySearch(const char *ptr, size_t max_len, int lvl) {
  uint32_t middle, first, sz;
  int cmp, max_cmp;
  max_cmp = 0;
  first = 0;
  sz = wordlist_lens[lvl];
  while (first < sz) {
    middle = (first + sz) >> 1;
    size_t dict_word_len;
    const char *dict_word = getDictWord(lvl, middle, &dict_word_len);
    cmp = compare(ptr, min_of(max_len, dict_word_len), dict_word, dict_word_len);
    if (abs(cmp) > max_cmp)
      max_cmp = abs(cmp);
    if (cmp > 0)
      first = middle + 1;
    else if (cmp < 0)
      sz = middle;
    else
      return middle;
  }
  if (middle)
    middle--;
  while (max_cmp > 2 && middle < wordlist_lens[lvl]) {
    size_t dict_word_len;
    const char *dict_word = getDictWord(lvl, middle, &dict_word_len);
    cmp = compare(ptr, min_of(max_len, dict_word_len), dict_word, dict_word_len);
    if (cmp == 0)
      return middle;
    max_cmp = abs(cmp);
    middle--;
  }
  return -1;
}

/// Fills the usx_code_94 94 letter array based on sets of characters at usx_sets \n
/// For each element in usx_code_94, first 3 msb bits is set (USX_ALPHA / USX_SYM / USX_NUM) \n
/// and the rest 5 bits indicate the vertical position in the corresponding set
unishox3::unishox3() {

  uint8_t usx_sets_default[3][28] = 
    {{  0, ' ', 'e', 't', 'a', 'o', 'i', 'n',
      's', 'r', 'l', 'c', 'd', 'h', 'u', 'p', 'm', 'b',
      'g', 'w', 'f', 'y', 'v', 'k', 'q', 'j', 'x', 'z'},
    { '"', '{', '}', '_', '<', '>', ':', '\n',
        0, '[', ']', '\\', ';', '\'', '\t', '@', '*', '&',
      '?', '!', '^', '|', '\r', '~', '`', 0, 0, 0},
    {  0, ',', '.', '0', '1', '9', '2', '5', '-',
      '/', '3', '4', '6', '7', '8', '(', ')', ' ',
      '=', '+', '$', '%', '#', 0, 0, 0, 0, 0}};
  memcpy(usx_hcodes, (const uint8_t *) "\x00\xE0\x80\xA0\xC0\x40", 6);
  memcpy(usx_hcode_lens, (const uint8_t *) "\x02\x03\x03\x03\x03\x02", 6);
  memset(usx_templates, '\0', sizeof(usx_templates));
  memset(usx_code_94, '\0', sizeof(usx_code_94));
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 28; j++) {
      usx_sets[i][j] = usx_sets_default[i][j];
      uint8_t c = usx_sets[i][j];
      if (c != 0 && c > 32) {
        usx_code_94[c - USX_OFFSET_94] = (i << 5) + j;
        if (c >= 'a' && c <= 'z')
          usx_code_94[c - USX_OFFSET_94 - ('a' - 'A')] = (i << 5) + j;
      }
    }
  }
}

void unishox3::setTemplates(const char **templates) {
  for (int i = 0; i < 5; i++)
    usx_templates[i] = templates[i];
}

void unishox3::setHCodess(uint8_t hcodes[], uint8_t hcode_lens[]) {
  for (int i = 0; i < HCODE_COUNT; i++) {
    usx_hcodes[i] = hcodes[i];
    usx_hcode_lens[i] = hcode_lens[i];
  }
}

/// Appends switch code to out depending on the state (USX_DELTA or other)
int unishox3::append_switch_code(char *out, int olen, int ol, uint8_t state) {
  if (state == USX_DELTA) {
    SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN));
    SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, UNI_STATE_SW_CODE, UNI_STATE_SW_CODE_LEN));
  } else
    SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, SW_CODE, SW_CODE_LEN));
  return ol;
}

/// Switches to given horizontal code - could be a second switch as well
int unishox3::switch_to(char *out, int olen, int ol, uint8_t state, int hcode) {
  SAFE_APPEND_BITS(ol = append_switch_code(out, olen, ol, state));
  SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, usx_hcodes[hcode], usx_hcode_lens[hcode]));
  return ol;
}

/// Appends given horizontal and veritical code bits to out
int unishox3::append_code(char *out, int olen, int ol, uint8_t code, uint8_t *state) {
  uint8_t hcode = code >> 5;
  uint8_t vcode = code & 0x1F;
  if (!usx_hcode_lens[hcode] && hcode != USX_ALPHA)
    return ol;
  switch (hcode) {
    case USX_ALPHA:
      if (*state != USX_ALPHA) {
        SAFE_APPEND_BITS(ol = switch_to(out, olen, ol, *state, USX_ALPHA));
        *state = USX_ALPHA;
      }
      break;
    case USX_SYM:
      SAFE_APPEND_BITS(ol = switch_to(out, olen, ol, *state, USX_SYM));
      break;
    case USX_NUM:
      if (*state != USX_NUM) {
        SAFE_APPEND_BITS(ol = switch_to(out, olen, ol, *state, USX_NUM));
        if (usx_sets[hcode][vcode] >= '0' && usx_sets[hcode][vcode] <= '9')
          *state = USX_NUM;
      }
  }
  SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, usx_vcodes[vcode], usx_vcode_lens[vcode]));
  return ol;
}

//static int prev_pos = 0;
usx3_dict_find unishox3::match_predef_dict(const char *in, int len, int l) {
  size_t max_len = len - l;
  int pos_lvl = LATIN_DICT_LVL_MAX;
  int32_t pos = -1;
  for (; pos_lvl >= 0; pos_lvl--) {
    pos = binarySearch(&in[l], max_len, pos_lvl);
    if (pos != -1) {
      int32_t next = pos + 1;
      while (next < wordlist_lens[pos_lvl]) {
        size_t dict_word_len;
        const char *dict_word = getDictWord(pos_lvl, next, &dict_word_len);
        int cmp = compare(&in[l], min_of(max_len, dict_word_len), dict_word, dict_word_len);
        if (cmp)
          next = wordlist_lens[pos_lvl];
        else
          pos = next;
        next++;
      }
      break;
    }
  }
  return usx3_dict_find(pos_lvl, pos);
}

/// Finds the longest matching sequence from the beginning of the string. \n
/// If a match is found and it is longer than NICE_LEN, it is encoded as a repeating sequence to out \n
/// This is also used for Unicode strings \n
/// This is a crude implementation that is not optimized.  Assuming only short strings \n
/// are encoded, this is not much of an issue.
usx3_longest unishox3::matchOccurance(const char *in, int len, int l) {
  int j, k;
  int longest_dist = -1;
  int longest_len = -1;
  for (j = l - NICE_LEN; j >= 0; j--) {
    for (k = l; k < len && j + k - l < l; k++) {
      if (in[k] != in[j + k - l])
        break;
    }
    while ((((unsigned char) in[k]) >> 6) == 2)
      k--; // Skip partial UTF-8 matches
    //if ((in[k - 1] >> 3) == 0x1E || (in[k - 1] >> 4) == 0x0E || (in[k - 1] >> 5) == 0x06)
    //  k--;
    if ((k - l) > (NICE_LEN - 1)) {
      int match_len = k - l - NICE_LEN;
      int match_dist = l - j - match_len - NICE_LEN + 1;
      if (match_len > longest_len) {
          longest_len = match_len;
          longest_dist = match_dist;
      }
    }
  }
  return usx3_longest(longest_len, longest_dist);
}

/// Starts coding of nibble sets
int unishox3::append_nibble_escape(char *out, int olen, int ol, uint8_t state) {
  SAFE_APPEND_BITS(ol = switch_to(out, olen, ol, state, USX_NUM));
  SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, 0, 2));
  return ol;
}

/// Appends the terminator code depending on the state, preset and whether full terminator needs to be encoded to out or not \n
int unishox3::append_final_bits(char *const out, const int olen, int ol, const uint8_t state, const uint8_t is_all_upper) {
  if (usx_hcode_lens[USX_ALPHA]) {
    if (USX_NUM != state) {
      // TODO: Why switch to num state instead of alpha?
      // for num state, append TERM_CODE directly
      // for other state, switch to Num Set first
      SAFE_APPEND_BITS(ol = switch_to(out, olen, ol, state, USX_NUM));
    }
    SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, usx_vcodes[TERM_CODE & 0x1F], usx_vcode_lens[TERM_CODE & 0x1F]));
  } else {
    // preset 1, terminate at 2 or 3 SW_CODE, i.e., 4 or 6 continuous 0 bits
    // see discussion: https://github.com/siara-cc/Unishox/issues/19#issuecomment-922435580
    SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, TERM_BYTE_PRESET_1, is_all_upper ? TERM_BYTE_PRESET_1_LEN_UPPER : TERM_BYTE_PRESET_1_LEN_LOWER));
  }

  // fill uint8_t with the last bit
  SAFE_APPEND_BITS(ol = append_bits(out, olen, ol, (ol == 0 || out[(ol-1)/8] << ((ol-1)&7) >= 0) ? 0 : 0xFF, (8 - ol % 8) & 7));

  return ol;
}

uint8_t usx_lvl_counts[] = {0x40, 0x60, 0x80, 0xA0, 0xC0, 0x00, 0xE0};
uint8_t usx_lvl_lens[]   = {   3,    3,    3,    3,    3,    2,    3};
int unishox3::encode_dict_matches(const char *in, int len, int l, char *out, int olen, int *ol, uint8_t *state, uint8_t *is_all_upper) {

  bool continuous = false;
  int continuous_bit_ol = 0;
  int continuous_bit_loc = 0;
  int last_suffix_loc = 0;
  int encoded_count = 0;

  while (l < len) {

    usx3_longest longest;
    usx3_dict_find dict_find;
    if (usx_hcode_lens[USX_DICT] && l < (len - NICE_LEN + 1) && !longest.is_found())
      longest = matchOccurance(in, len, l);
    if (usx_hcode_lens[USX_PREDEF_DICT] && in[l] != ' ')
      dict_find = match_predef_dict(in, len, l);
    if (!longest.is_found() && !dict_find.is_found()) {
      if (continuous) {
        if (encoded_count > 2) {
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0xE0, 3));
          long_count+=encoded_count;
        } else {
          uint8_t *start_byte = (uint8_t *) out + (continuous_bit_loc >> 3);
          *start_byte &= ~(0x80 >> (continuous_bit_loc % 8));
          *ol = continuous_bit_ol;
          l = last_suffix_loc;
          short_count+=encoded_count;
        }
      }
      return l;
    }
    encoded_count++;

    if (longest.is_found() && longest.saving() > dict_find.saving()) {
      if (continuous) {
        SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x40, 2)); // end suffix and next repeating sequence
        //printf("~");
      } else {
        SAFE_APPEND_BITS(*ol = switch_to(out, olen, *ol, *state, USX_DICT));
        //printf("%%");
        //printf("Len:%d / Dist:%d/%.*s\n", longest_len, longest_dist, longest_len + NICE_LEN, in + l - longest_dist - NICE_LEN + 1);
        if (*state != USX_DELTA) {
          if (!continuous_bit_loc)
            continuous_bit_loc = *ol;
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x80, 1));
        }
      }
      l += (longest.len + NICE_LEN);
      SAFE_APPEND_BITS(*ol = encodeCount(out, olen, *ol, longest.len));
      SAFE_APPEND_BITS(*ol = encodeCount(out, olen, *ol, longest.dist));
      if (*state == USX_DELTA)
        return l;
    } else {
      if (continuous) {
        if (in[l] >= 'A' && in[l] <= 'Z')
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x80, 5)); // next upper
        SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x00, 2)); // end suffix and next from dictionary
        //printf("`");
      } else {
        //printf("#");
        if (*state != USX_ALPHA) {
          SAFE_APPEND_BITS(*ol = switch_to(out, olen, *ol, *state, USX_ALPHA));
          *state = USX_ALPHA;
        }
        if (*is_all_upper) {
          SAFE_APPEND_BITS(*ol = switch_to(out, olen, *ol, *state, USX_ALPHA));
          *is_all_upper = 0;
        }
        if (in[l] >= 'A' && in[l] <= 'Z')
          SAFE_APPEND_BITS(*ol = switch_to(out, olen, *ol, *state, USX_ALPHA));
        SAFE_APPEND_BITS(*ol = switch_to(out, olen, *ol, *state, USX_PREDEF_DICT));
        if (!continuous_bit_loc)
          continuous_bit_loc = *ol;
        SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x80, 1));
      }
      SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, usx_lvl_counts[dict_find.lvl], usx_lvl_lens[dict_find.lvl])); // appending count level
      int bits_to_append = predict_count_bits[dict_find.lvl];

      size_t dict_word_len;
      const char *dict_word = getDictWord(dict_find.lvl, dict_find.pos, &dict_word_len);
      l += min_of(len - l, dict_word_len);
      //printf("[%s], pos: %d, len: %ld\n", wordlist[pos], pos, min_of(max_len, strlen(wordlist[pos])));
      //printf("%d\n", pos);
      //prev_pos = pos;
      do {
        uint32_t byte_to_append = dict_find.pos << (32-bits_to_append);
        SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, byte_to_append >> 24, bits_to_append > 8 ? 8 : bits_to_append)); // appending count level
        bits_to_append -= 8;
      } while (bits_to_append > 0);
    }

      if (!continuous_bit_ol) {
        continuous_bit_ol = *ol;
        last_suffix_loc = l;
      }
      continuous = true;

    while (true) {
      if (l >= len) {
        SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x80, 3));
        *state = USX_NUM;
        return l;
      }
      if (usx_hcode_lens[USX_DICT] && l < (len - NICE_LEN + 1))
        longest = matchOccurance(in, len, l);
      if (longest.is_found())
        break;
      int c_in = (uint8_t) in[l];
      if ((c_in >= 'A' && c_in <= 'Z') || (c_in >= 'a' && c_in <= 'z') || c_in > 126)
        break; // loop and check if next code found in dict
      if (c_in >= USX_OFFSET_94 && c_in <= 126) {
        int code = usx_code_94[c_in - USX_OFFSET_94];
        SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, (code >> 5) == 2 ? 0x80 : 0xA0, 3));
        if (c_in < '0' || c_in > '9')
          c_in = 0;
        do {
          code &= 0x1F;
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, usx_vcodes[code], usx_vcode_lens[code]));
          l++;
          if (l >= len) {
            if (c_in == 0)
              SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x80, 3));
            *state = USX_NUM;
            return l;
          }
          if (c_in == 0)
            break;
          c_in = in[l];
          code = usx_code_94[c_in - USX_OFFSET_94];
        } while ((code >> 5) == 2);
        if (c_in)
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0x00, 2));
        l--;
      } else {
        if (c_in == 32)
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0xC0, 3)); // 0b110 space
        else {
          int vpos = 0;
          switch (c_in) {
            case '\r':
              if (l < (len - 1) && in[l + 1] == '\n') {
                vpos = 8;
                l++;
              } else
                vpos = 22;
              break;
            case '\n':
              vpos = 7;
              break;
            case '\0':
              vpos = 25;
              break;
            case '\t':
              vpos = 14;
          }
          if (vpos == 0)
            break;
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, 0xA0, 3)); // 0b1111 switch to sym
          SAFE_APPEND_BITS(*ol = append_bits(out, olen, *ol, usx_vcodes[vpos], usx_vcode_lens[vpos]));
        }
      }
      l++;
    }

  }

  return l;

}

// Main API function. See unishox2.h for documentation
int unishox3::compress(const char *in, int len, USX3_API_OUT_AND_LEN(char *out, int olen)) {

  uint8_t state;

  int l, ll, ol;
  char c_in, c_next;
  int prev_uni;
  uint8_t is_upper, is_all_upper;
#if (USX3_API_OUT_AND_LEN(0,1)) == 0
  const int olen = INT_MAX - 1;
  const int rawolen = olen;
  const uint8_t need_full_term_codes = 0;
#else
  const int rawolen = olen;
  uint8_t need_full_term_codes = 0;
  if (olen < 0) {
    need_full_term_codes = 1;
    olen *= -1;
  }
#endif

  ol = 0;
  prev_uni = 0;
  state = USX_ALPHA;
  is_all_upper = 0;
  SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, USX3_MAGIC_BITS, USX3_MAGIC_BIT_LEN)); // magic bit(s)
  for (l=0; l<len; l++) {

    int new_l = encode_dict_matches(in, len, l, out, olen, &ol, &state, &is_all_upper);
    if (new_l > l) {
      //printf("`");
      l = new_l;
      l--;
      continue;
    }

    c_in = in[l];
    if (l && len > 4 && l < (len - 4) && usx_hcode_lens[USX_NUM]) {
      if (c_in == in[l - 1] && c_in == in[l + 1] && c_in == in[l + 2] && c_in == in[l + 3]) {
        int rpt_count = l + 4;
        while (rpt_count < len && in[rpt_count] == c_in)
          rpt_count++;
        rpt_count -= l;
        SAFE_APPEND_BITS2(rawolen, ol = append_code(out, olen, ol, RPT_CODE, &state));
        SAFE_APPEND_BITS2(rawolen, ol = encodeCount(out, olen, ol, rpt_count - 4));
        l += rpt_count;
        l--;
        continue;
      }
    }

    if (l <= (len - 36) && usx_hcode_lens[USX_NUM]) {
      if (in[l + 8] == '-' && in[l + 13] == '-' && in[l + 18] == '-' && in[l + 23] == '-') {
        char hex_type = USX_NIB_NUM;
        int uid_pos = 0;
        for (; uid_pos < 36; uid_pos++) {
          char c_uid = in[l + uid_pos];
          if (c_uid == '-' && (uid_pos == 8 || uid_pos == 13 || uid_pos == 18 || uid_pos == 23))
            continue;
          char nib_type = getNibbleType(c_uid);
          if (nib_type == USX_NIB_NOT)
            break;
          if (nib_type != USX_NIB_NUM) {
            if (hex_type != USX_NIB_NUM && hex_type != nib_type)
              break;
            hex_type = nib_type;
          }
        }
        if (uid_pos == 36) {
          SAFE_APPEND_BITS2(rawolen, ol = append_nibble_escape(out, olen, ol, state));
          SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, (hex_type == USX_NIB_HEX_LOWER ? 0x80 : 0xC0), 3));
          for (uid_pos = l; uid_pos < l + 36; uid_pos++) {
            char c_uid = in[uid_pos];
            if (c_uid != '-')
              SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, getBaseCode(c_uid), 4));
          }
          //printf("GUID:\n");
          l += 35;
          continue;
        }
      }
    }

    if (l < (len - 5) && usx_hcode_lens[USX_NUM]) {
      char hex_type = USX_NIB_NUM;
      int hex_len = 0;
      do {
        char nib_type = getNibbleType(in[l + hex_len]);
        if (nib_type == USX_NIB_NOT)
          break;
        if (nib_type != USX_NIB_NUM) {
          if (hex_type != USX_NIB_NUM && hex_type != nib_type)
            break;
          hex_type = nib_type;
        }
        hex_len++;
      } while (l + hex_len < len);
      if (hex_len > 10 && hex_type == USX_NIB_NUM)
        hex_type = USX_NIB_HEX_LOWER;
      if ((hex_type == USX_NIB_HEX_LOWER || hex_type == USX_NIB_HEX_UPPER) && hex_len > 3) {
        SAFE_APPEND_BITS2(rawolen, ol = append_nibble_escape(out, olen, ol, state));
        SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, (hex_type == USX_NIB_HEX_LOWER ? 0x40 : 0xA0), (hex_type == USX_NIB_HEX_LOWER ? 2 : 3)));
        SAFE_APPEND_BITS2(rawolen, ol = encodeCount(out, olen, ol, hex_len));
        do {
          SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, getBaseCode(in[l++]), 4));
        } while (--hex_len);
        l--;
        continue;
      }
    }

    { // if (usx_templates != NULL) {
      int i;
      for (i = 0; i < 5; i++) {
        if (usx_templates[i] != NULL) {
          int rem = (int)strlen(usx_templates[i]);
          int j = 0;
          for (; j < rem && l + j < len; j++) {
            char c_t = usx_templates[i][j];
            c_in = in[l + j];
            if (c_t == 'f' || c_t == 'F') {
              if (getNibbleType(c_in) != (c_t == 'f' ? USX_NIB_HEX_LOWER : USX_NIB_HEX_UPPER)
                       && getNibbleType(c_in) != USX_NIB_NUM) {
                break;
              }
            } else
            if (c_t == 'r' || c_t == 't' || c_t == 'o') {
              if (c_in < '0' || c_in > (c_t == 'r' ? '7' : (c_t == 't' ? '3' : '1')))
                break;
            } else
            if (c_t != c_in)
              break;
          }
          if (((float)j / rem) > 0.66) {
            //printf("%s\n", usx_templates[i]);
            rem = rem - j;
            SAFE_APPEND_BITS2(rawolen, ol = append_nibble_escape(out, olen, ol, state));
            SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, 0, 2));
            SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, (count_codes[i] & 0xF8), count_codes[i] & 0x07));
            SAFE_APPEND_BITS2(rawolen, ol = encodeCount(out, olen, ol, rem));
            for (int k = 0; k < j; k++) {
              char c_t = usx_templates[i][k];
              if (c_t == 'f' || c_t == 'F')
                SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, getBaseCode(in[l + k]), 4));
              else if (c_t == 'r' || c_t == 't' || c_t == 'o') {
                c_t = (c_t == 'r' ? 3 : (c_t == 't' ? 2 : 1));
                SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, (in[l + k] - '0') << (8 - c_t), c_t));
              }
            }
            l += j;
            l--;
            break;
          }
        }
      }
      if (i < 5)
        continue;
    }

    c_in = in[l];
//printf("%c", c_in);

    is_upper = 0;
    if (c_in >= 'A' && c_in <= 'Z')
      is_upper = 1;
    else {
      if (is_all_upper) {
        is_all_upper = 0;
        SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_ALPHA));
        state = USX_ALPHA;
      }
    }
    if (is_upper && !is_all_upper) {
      if (state == USX_NUM) {
        SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_ALPHA));
        state = USX_ALPHA;
      }
      SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_ALPHA));
      if (state == USX_DELTA) {
        state = USX_ALPHA;
        SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_ALPHA));
      }
    }
    c_next = 0;
    if (l+1 < len)
      c_next = in[l+1];

    if (c_in >= 32 && c_in <= 126) {
      if (is_upper && !is_all_upper) {
        for (ll=l+4; ll>=l && ll<len; ll--) {
          if (in[ll] < 'A' || in[ll] > 'Z')
            break;
        }
        if (ll == l-1) {
          SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_ALPHA));
          state = USX_ALPHA;
          is_all_upper = 1;
        }
      }
      if (state == USX_DELTA && (c_in == ' ' || c_in == '.' || c_in == ',')) {
        uint8_t spl_code = (c_in == ',' ? 0xC0 : (c_in == '.' ? 0xE0 : (c_in == ' ' ? 0 : 0xFF)));
        if (spl_code != 0xFF) {
          uint8_t spl_code_len = (c_in == ',' ? 3 : (c_in == '.' ? 4 : (c_in == ' ' ? 1 : 4)));
          SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN));
          SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, spl_code, spl_code_len));
          continue;
        }
      }
      c_in -= 32;
      if (is_all_upper && is_upper)
        c_in += 32;
      if (c_in == 0) {
        if (state == USX_NUM)
          SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, usx_vcodes[NUM_SPC_CODE & 0x1F], usx_vcode_lens[NUM_SPC_CODE & 0x1F]));
        else
          SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, usx_vcodes[1], usx_vcode_lens[1]));
      } else {
        c_in--;
        SAFE_APPEND_BITS2(rawolen, ol = append_code(out, olen, ol, usx_code_94[(int)c_in], &state));
      }
    } else
    if (c_in == 13 && c_next == 10) {
      SAFE_APPEND_BITS2(rawolen, ol = append_code(out, olen, ol, CRLF_CODE, &state));
      l++;
    } else
    if (c_in == 10) {
      if (state == USX_DELTA) {
        SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN));
        SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, 0xF0, 4));
      } else
        SAFE_APPEND_BITS2(rawolen, ol = append_code(out, olen, ol, LF_CODE, &state));
    } else
    if (c_in == 13) {
      SAFE_APPEND_BITS2(rawolen, ol = append_code(out, olen, ol, CR_CODE, &state));
    } else
    if (c_in == '\t') {
      SAFE_APPEND_BITS2(rawolen, ol = append_code(out, olen, ol, TAB_CODE, &state));
    } else {
      int utf8len;
      int32_t uni = readUTF8(in, len, l, &utf8len);
      if (uni) {
        l += utf8len;
        if (state != USX_DELTA) {
          int32_t uni2 = readUTF8(in, len, l, &utf8len);
          if (uni2) {
            if (state != USX_ALPHA) {
              SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_ALPHA));
            }
            SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_DELTA));
            SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN));
            // SAFE_APPEND_BITS2(rawolen, ol = append_switch_code(out, olen, ol, state)); arun
            // SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]));
            // SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, usx_vcodes[1], usx_vcode_lens[1])); // code for space (' ')
            state = USX_DELTA;
          } else {
            SAFE_APPEND_BITS2(rawolen, ol = switch_to(out, olen, ol, state, USX_DELTA));
          }
        }
        SAFE_APPEND_BITS2(rawolen, ol = encodeUnicode(out, olen, ol, uni, prev_uni));
        //printf("%d:%d:%d\n", l, utf8len, uni);
        prev_uni = uni;
        l--;
      } else {
        int bin_count = 1;
        for (int bi = l + 1; bi < len; bi++) {
          char c_bi = in[bi];
          //if (c_bi > 0x1F && c_bi != 0x7F)
          //  break;
          if (readUTF8(in, len, bi, &utf8len))
            break;
          if (bi < (len - 4) && c_bi == in[bi - 1] && c_bi == in[bi + 1] && c_bi == in[bi + 2] && c_bi == in[bi + 3])
            break;
          bin_count++;
        }
        //printf("Bin:%d:%d:%x:%d\n", l, (unsigned char) c_in, (unsigned char) c_in, bin_count);
        SAFE_APPEND_BITS2(rawolen, ol = append_nibble_escape(out, olen, ol, state));
        SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, 0xE0, 3));
        SAFE_APPEND_BITS2(rawolen, ol = encodeCount(out, olen, ol, bin_count));
        do {
          SAFE_APPEND_BITS2(rawolen, ol = append_bits(out, olen, ol, in[l++], 8));
        } while (--bin_count);
        l--;
      }
    }
  }

  if (need_full_term_codes) {
    const int orig_ol = ol;
    SAFE_APPEND_BITS2(rawolen, ol = append_final_bits(out, olen, ol, state, is_all_upper));
    return (ol / 8) * 4 + (((ol-orig_ol)/8) & 3);
  } else {
    const int rst = (ol + 7) / 8;
    SAFE_APPEND_BITS2(rawolen, ol = append_final_bits(out, rst, ol, state, is_all_upper));
    return rst;
  }
}

// Main API function. See unishox2.h for documentation
int unishox3::compress_simple(const char *in, int len, char *out) {
  return compress(in, len, USX3_API_OUT_AND_LEN(out, INT_MAX - 1));
}

// Reads one bit from in
int readBit(const char *in, int bit_no) {
   return in[bit_no >> 3] & (0x80 >> (bit_no % 8));
}

// Reads next 8 bits, if available
int read8bitCode(const char *in, int len, int bit_no) {
  int bit_pos = bit_no & 0x07;
  int char_pos = bit_no >> 3;
  len >>= 3;
  uint8_t code = (((uint8_t)in[char_pos]) << bit_pos);
  char_pos++;
  if (char_pos < len) {
    code |= ((uint8_t)in[char_pos]) >> (8 - bit_pos);
  } else
    code |= (0xFF >> (8 - bit_pos));
  return code;
}

/// The list of veritical codes is split into 5 sections. Used by readVCodeIdx()
#define SECTION_COUNT 5
/// Used by readVCodeIdx() for finding the section under which the code read using read8bitCode() falls
uint8_t usx_vsections[] = {0x7F, 0xBF, 0xDF, 0xEF, 0xFF};
/// Used by readVCodeIdx() for finding the section vertical position offset
uint8_t usx_vsection_pos[] = {0, 4, 8, 12, 20};
/// Used by readVCodeIdx() for masking the code read by read8bitCode()
uint8_t usx_vsection_mask[] = {0x7F, 0x3F, 0x1F, 0x0F, 0x0F};
/// Used by readVCodeIdx() for shifting the code read by read8bitCode() to obtain the vpos
uint8_t usx_vsection_shift[] = {5, 4, 3, 1, 0};

/// Vertical decoder lookup table - 3 bits code len, 5 bytes vertical pos
/// code len is one less as 8 cannot be accommodated in 3 bits
uint8_t usx_vcode_lookup[36] = {
  (1 << 5) + 0,  (1 << 5) + 0,  (2 << 5) + 1,  (2 << 5) + 2,  // Section 1
  (3 << 5) + 3,  (3 << 5) + 4,  (3 << 5) + 5,  (3 << 5) + 6,  // Section 2
  (3 << 5) + 7,  (3 << 5) + 7,  (4 << 5) + 8,  (4 << 5) + 9,  // Section 3
  (5 << 5) + 10, (5 << 5) + 10, (5 << 5) + 11, (5 << 5) + 11, // Section 4
  (5 << 5) + 12, (5 << 5) + 12, (6 << 5) + 13, (6 << 5) + 14,
  (6 << 5) + 15, (6 << 5) + 15, (6 << 5) + 16, (6 << 5) + 16, // Section 5
  (6 << 5) + 17, (6 << 5) + 17, (7 << 5) + 18, (7 << 5) + 19,
  (7 << 5) + 20, (7 << 5) + 21, (7 << 5) + 22, (7 << 5) + 23,
  (7 << 5) + 24, (7 << 5) + 25, (7 << 5) + 26, (7 << 5) + 27
};

// TODO: Last value check.. Also len check in readBit
/// Returns the position of step code (0, 10, 110, etc.) encountered in the stream
int getStepCodeIdx(const char *in, int *bit_no_p, int len, int limit) {
  int idx = 0;
  while (*bit_no_p < len && readBit(in, *bit_no_p)) {
    idx++;
    (*bit_no_p)++;
    if (idx == limit)
      return idx;
  }
  if (*bit_no_p >= len)
    return 99;
  (*bit_no_p)++;
  return idx;
}

/// Reads specified number of bits and builds the corresponding integer
int32_t getNumFromBits(const char *in, int len, int bit_no, int count) {
   int32_t ret = 0;
   while (count-- && bit_no < len) {
     ret += (readBit(in, bit_no) ? 1 << count : 0);
     bit_no++;
   }
   return count < 0 ? ret : -1;
}

int getBitVal(const char *in, int bit_no, int count) {
   return (in[bit_no >> 3] & (0x80 >> (bit_no % 8)) ? 1 << count : 0);
}

int readCode(const char *in, int *bit_no_p, int len, int set_size) {
  int code = 0;
  for (int i = 0; i < set_size; i++) {
    if (*bit_no_p >= len)
      return 99;
    code += getBitVal(in, *bit_no_p, i);
    (*bit_no_p)++;
    int idx;
    if (set_size == 6) {
      idx = (code == 0 && i == 1 ? 0 : (code == 2 && i == 1 ? 1 : 
            (code == 1 && i == 2 ? 2 : (code == 5 && i == 2 ? 3 :
            (code == 3 && i == 2 ? 4 : (code == 7 && i == 2 ? 5 : 99))))));
    } else {
      idx = (code == 0 && i == 1 ? 0 : (code == 2 && i == 1 ? 1 : 
            (code == 1 && i == 1 ? 2 : (code == 3 && i == 2 ? 3 :
            (code == 7 && i == 2 ? 4 : 99)))));
    }
    if (idx < 99)
      return idx;
  }
  return 99;
}

/// Decodes the count from the given bit stream at in. Also updates bit_no_p
int32_t readCount(const char *in, int *bit_no_p, int len) {
  int idx = readCode(in, bit_no_p, len, 5);
  if (idx == 99)
    return -1;
  if (*bit_no_p + count_bit_lens[idx] - 1 >= len)
    return -1;
  int32_t count = getNumFromBits(in, len, *bit_no_p, count_bit_lens[idx]) + (idx ? count_adder[idx - 1] : 0);
  (*bit_no_p) += count_bit_lens[idx];
  return count;
}

/// Decodes the Unicode codepoint from the given bit stream at in. Also updates bit_no_p \n
/// When the step code is 5, reads the next step code to find out the special code.
int32_t readUnicode(const char *in, int *bit_no_p, int len) {
  int idx = getStepCodeIdx(in, bit_no_p, len, 5);
  if (idx == 99)
    return 0x7FFFFF00 + 99;
  if (idx == 2)
    return 0x7FFFFF00 + idx;
  if (idx >= 0) {
    int sign = (*bit_no_p < len ? readBit(in, *bit_no_p) : 0);
    (*bit_no_p)++;
    if (*bit_no_p + uni_bit_len[idx] - 1 >= len)
      return 0x7FFFFF00 + 99;
    int32_t count = getNumFromBits(in, len, *bit_no_p, uni_bit_len[idx]);
    count += uni_adder[idx];
    (*bit_no_p) += uni_bit_len[idx];
    //printf("Sign: %d, Val:%d", sign, count);
    return sign ? -count : count;
  }
  return 0;
}

/// Macro to ensure that the decoder does not append more than olen bytes to out
#define DEC_OUTPUT_CHAR(out, olen, ol, c) do { \
  char *const obuf = (out); \
  const int oidx = (ol); \
  const int limit = (olen); \
  if (limit <= oidx) return limit + 1; \
  else if (oidx < 0) return 0; \
  else obuf[oidx] = (c); \
} while (0)

/// Macro to ensure that the decoder does not append more than olen bytes to out
#define DEC_OUTPUT_CHARS(olen, exp) do { \
  const int newidx = (exp); \
  const int limit = (olen); \
  if (newidx > limit) return limit + 1; \
} while (0)

/// Write given unicode code point to out as a UTF-8 sequence
int writeUTF8(char *out, int olen, int ol, int uni) {
  if (uni < (1 << 11)) {
    DEC_OUTPUT_CHAR(out, olen, ol++, 0xC0 + (uni >> 6));
    DEC_OUTPUT_CHAR(out, olen, ol++, 0x80 + (uni & 0x3F));
  } else
  if (uni < (1 << 16)) {
    DEC_OUTPUT_CHAR(out, olen, ol++, 0xE0 + (uni >> 12));
    DEC_OUTPUT_CHAR(out, olen, ol++, 0x80 + ((uni >> 6) & 0x3F));
    DEC_OUTPUT_CHAR(out, olen, ol++, 0x80 + (uni & 0x3F));
  } else {
    DEC_OUTPUT_CHAR(out, olen, ol++, 0xF0 + (uni >> 18));
    DEC_OUTPUT_CHAR(out, olen, ol++, 0x80 + ((uni >> 12) & 0x3F));
    DEC_OUTPUT_CHAR(out, olen, ol++, 0x80 + ((uni >> 6) & 0x3F));
    DEC_OUTPUT_CHAR(out, olen, ol++, 0x80 + (uni & 0x3F));
  }
  return ol;
}

/// Decode repeating sequence and appends to out
/// Returns hex character corresponding to the 4 bit nibble
char getHexChar(int32_t nibble, int hex_type) {
  if (nibble >= 0 && nibble <= 9)
    return '0' + nibble;
  else if (hex_type < USX_NIB_HEX_UPPER)
    return 'a' + nibble - 10;
  return 'A' + nibble - 10;
}

/// Decodes the vertical code from the given bitstream at in \n
/// This is designed to use less memory using a 36 uint8_t buffer \n
/// compared to using a 256 uint8_t buffer to decode the next 8 bits read by read8bitCode() \n
/// by splitting the list of vertical codes. \n
/// Decoder is designed for using less memory, not speed. \n
/// Returns the veritical code index or 99 if match could not be found. \n
/// Also updates bit_no_p with how many ever bits used by the vertical code.
int unishox3::readVCodeIdx(const char *in, int len, int *bit_no_p) {
  if (*bit_no_p < len) {
    uint8_t code = read8bitCode(in, len, *bit_no_p);
    int i = 0;
    do {
      if (code <= usx_vsections[i]) {
        uint8_t vcode = usx_vcode_lookup[usx_vsection_pos[i] + ((code & usx_vsection_mask[i]) >> usx_vsection_shift[i])];
        (*bit_no_p) += ((vcode >> 5) + 1);
        if (*bit_no_p > len)
          return 99;
        return vcode & 0x1F;
      }
    } while (++i < SECTION_COUNT);
  }
  return 99;
}

/// Mask for retrieving each code to be decoded according to its length \n
/// Same as usx_mask so redundant
const uint8_t len_masks[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
/// Decodes the horizontal code from the given bitstream at in \n
/// depending on the hcodes defined using usx_hcodes and usx_hcode_lens \n
/// Returns the horizontal code index or 99 if match could not be found. \n
/// Also updates bit_no_p with how many ever bits used by the horizontal code.
int unishox3::readHCodeIdx(const char *in, int len, int *bit_no_p) {
  if (!usx_hcode_lens[USX_ALPHA])
    return USX_ALPHA;
  if (*bit_no_p < len) {
    uint8_t code = read8bitCode(in, len, *bit_no_p);
    for (int code_pos = 0; code_pos < HCODE_COUNT; code_pos++) {
      if (usx_hcode_lens[code_pos] && (code & len_masks[usx_hcode_lens[code_pos] - 1]) == usx_hcodes[code_pos]) {
        *bit_no_p += usx_hcode_lens[code_pos];
        return code_pos;
      }
    }
  }
  return 99;
}

int unishox3::readLvlIdx(const char *in, int len, int *bit_no_p) {
  if (*bit_no_p < len) {
    uint8_t code = read8bitCode(in, len, *bit_no_p);
    for (int code_pos = 0; code_pos < 7; code_pos++) {
      if ((code & len_masks[usx_lvl_lens[code_pos] - 1]) == usx_lvl_counts[code_pos]) {
        *bit_no_p += usx_lvl_lens[code_pos];
        return code_pos;
      }
    }
  }
  return 99;
}

int unishox3::decodeRepeat(const char *in, int len, char *out, int olen, int ol, int *bit_no) {
  int32_t dict_len = readCount(in, bit_no, len) + NICE_LEN;
  if (dict_len < NICE_LEN)
    return -1;
  int32_t dist = readCount(in, bit_no, len) + dict_len - 1;
  if (dist < dict_len - 1)
    return -1;
  const int32_t left = olen - ol;
  //printf("Decode len: %d, dist: %d\n", dict_len, dist);
  if (left <= 0) return olen + 1;
  if (ol - dist < 0)
      return -1;
  memcpy(out + ol, out + ol - dist, min_of(left, dict_len));
  if (left < dict_len) return olen + 1;
  ol += dict_len;
  return ol;
}

// Main API function. See unishox2.h for documentation
int unishox3::decompress(const char *in, int len, USX3_API_OUT_AND_LEN(char *out, int olen)) {

  int dstate;
  int bit_no;
  int h, v;
  uint8_t is_all_upper;

#if (USX3_API_OUT_AND_LEN(0,1)) == 0
  const int olen = INT_MAX - 1;
#endif

  int ol = 0;
  bit_no = USX3_MAGIC_BIT_LEN; // ignore the magic bit
  dstate = h = USX_ALPHA;
  is_all_upper = 0;

  int prev_uni = 0;
  uint8_t is_upper = 0;

  len <<= 3;
  while (bit_no < len) {
    int orig_bit_no = bit_no;
    if (dstate == USX_DELTA || h == USX_DELTA) {
      int32_t delta = readUnicode(in, &bit_no, len);
      if ((delta >> 8) == 0x7FFFFF) {
        int spl_code_idx = delta & 0x000000FF;
        if (spl_code_idx == 99)
          break;
        if (h == USX_DELTA && dstate != USX_DELTA) {
          dstate = h; // continuous delta coding
          continue;
        }
        spl_code_idx = getStepCodeIdx(in, &bit_no, len, 4);
        switch (spl_code_idx) {
          case 0:
            DEC_OUTPUT_CHAR(out, olen, ol++, ' ');
            continue;
          case 1:
            h = readHCodeIdx(in, len, &bit_no);
            if (h == 99) {
              bit_no = len;
              continue;
            }
            if (h == USX_DELTA || h == USX_ALPHA) {
              dstate = h;
              continue;
            }
            if (h == USX_DICT) {
              int rpt_ret = decodeRepeat(in, len, out, olen, ol, &bit_no);
              if (rpt_ret < 0)
                return ol; // if we break here it will only break out of switch
              DEC_OUTPUT_CHARS(olen, ol = rpt_ret);
              h = dstate;
              continue;
            }
            break;
          case 2:
            DEC_OUTPUT_CHAR(out, olen, ol++, ',');
            continue;
          case 3:
            DEC_OUTPUT_CHAR(out, olen, ol++, '.');
            continue;
          case 4:
            DEC_OUTPUT_CHAR(out, olen, ol++, 10);
            continue;
        }
      } else {
        prev_uni += delta;
        DEC_OUTPUT_CHARS(olen, ol = writeUTF8(out, olen, ol, prev_uni));
        //printf("%ld, ", prev_uni);
      }
      if (dstate != USX_DELTA)
        h = dstate;
      if (dstate == USX_DELTA && h == USX_DELTA)
        continue;
    } else
      h = dstate;
    char c = 0;
    v = readVCodeIdx(in, len, &bit_no);
    if (v == 99 || h == 99) {
      bit_no = orig_bit_no;
      break;
    }
    if (v == 0 && h != USX_SYM) {
      if (bit_no >= len)
        break;
      if (h != USX_NUM || dstate != USX_DELTA) {
        h = readHCodeIdx(in, len, &bit_no);
        if (h == 99 || bit_no >= len) {
          bit_no = orig_bit_no;
          break;
        }
      }
      if (h == USX_ALPHA) {
         if (dstate == USX_ALPHA) {
           if (!usx_hcode_lens[USX_ALPHA] && TERM_BYTE_PRESET_1 == (read8bitCode(in, len, bit_no - SW_CODE_LEN) & (0xFF << (8 - (is_all_upper ? TERM_BYTE_PRESET_1_LEN_UPPER : TERM_BYTE_PRESET_1_LEN_LOWER)))))
             break; // Terminator for preset 1
           if (is_all_upper) {
             is_upper = is_all_upper = 0;
           } else {
             if (is_upper)
                is_all_upper = 1;
             else
                is_upper = 1;
           }
           continue;
         } else {
            dstate = USX_ALPHA;
            is_upper = 0;
            continue;
         }
      } else
      if (h == USX_DICT || h == USX_PREDEF_DICT) {
        bool is_cont = bit_no < len ? readBit(in, bit_no++) : 0;
        bool is_rpt = (h == USX_DICT);
        do {
          if (is_rpt) {
            int rpt_ret = decodeRepeat(in, len, out, olen, ol, &bit_no);
            if (rpt_ret < 0)
              break;
            DEC_OUTPUT_CHARS(olen, ol = rpt_ret);
          } else {
            int pos_lvl = readLvlIdx(in, len, &bit_no);
            if (pos_lvl == 99 || pos_lvl > 6)
              break;
            int bits_to_read = predict_count_bits[pos_lvl];
            int32_t pos = getNumFromBits(in, len, bit_no, bits_to_read);
            if (pos < 0 || pos >= wordlist_lens[pos_lvl])
              break;
            size_t dict_word_len;
            const char *dict_word = getDictWord(pos_lvl, pos, &dict_word_len);
            const int left = olen - ol;
            if (left <= 0) return olen + 1;
            memcpy(out + ol, dict_word, min_of(left, dict_word_len));
            if (is_upper)
              out[ol] -= ('a' - 'A');
            is_upper = 0;
            if (left < dict_word_len) return olen + 1;
            ol += min_of(left, dict_word_len);
            bit_no += bits_to_read;
          }
          if (is_cont) {
            bool is_suffix = bit_no < len ? readBit(in, bit_no++) : 0;
            while (is_suffix) {
              bool is_spc_stop = bit_no < len ? readBit(in, bit_no++) : 0;
              if (is_spc_stop) {
                bool is_stop = bit_no < len ? readBit(in, bit_no++) : 0;
                if (is_stop) {
                  is_cont = false;
                  break;
                } else
                  DEC_OUTPUT_CHAR(out, olen, ol++, ' ');
              } else { // switch
                bool is_sym = bit_no < len ? readBit(in, bit_no++) : 0;
                bool is_cont_num = false;
                do {
                  v = readVCodeIdx(in, len, &bit_no);
                  if (!v && !is_cont_num && !is_sym) {
                    is_upper = 1;
                    break;
                  }
                  if (v == 99 || h == 99 || (v == 27 && !is_sym))
                    return ol;
                  char c = 0;
                  if (v == 8 && is_sym) {
                    DEC_OUTPUT_CHAR(out, olen, ol++, '\r');
                    DEC_OUTPUT_CHAR(out, olen, ol++, '\n');
                  } else if (v == 25 && is_sym) {
                    DEC_OUTPUT_CHAR(out, olen, ol++, 0);
                  } else {
                    c = usx_sets[is_sym ? 1 : 2][v];
                    if (c)
                      DEC_OUTPUT_CHAR(out, olen, ol++, c);
                  }
                  if (!is_cont_num && !is_sym && c >= '0' && c <= '9')
                    is_cont_num = true;
                } while (is_cont_num && v);
              }
              is_suffix = bit_no < len ? readBit(in, bit_no++) : 0;
            }
            if (is_cont)
              is_rpt = bit_no < len ? readBit(in, bit_no++) : 0;
          }
        } while (is_cont);
        continue;
      } else
      if (h == USX_DELTA) {
        //printf("Sign: %d, bitno: %d\n", sign, bit_no);
        //printf("Code: %d\n", prev_uni);
        //printf("BitNo: %d\n", bit_no);
        is_upper = 0;
        continue;
      } else {
        is_upper = 0;
        if (h != USX_NUM || dstate != USX_DELTA)
          v = readVCodeIdx(in, len, &bit_no);
        if (v == 99) {
          bit_no = orig_bit_no;
          break;
        }
        if (h == USX_NUM && v == 0) {
          int idx = readCode(in, &bit_no, len, 6);
          if (idx == 99)
            break;
          if (idx == 0) {
            idx = readCode(in, &bit_no, len, 5);
            if (idx >= 5)
              break;
            int32_t rem = readCount(in, &bit_no, len);
            if (rem < 0)
              break;
            if (usx_templates[idx] == NULL)
              break;
            size_t tlen = strlen(usx_templates[idx]);
            if (rem > tlen)
              break;
            rem = tlen - rem;
            int eof = 0;
            for (int j = 0; j < rem; j++) {
              char c_t = usx_templates[idx][j];
              if (c_t == 'f' || c_t == 'r' || c_t == 't' || c_t == 'o' || c_t == 'F') {
                  char nibble_len = (c_t == 'f' || c_t == 'F' ? 4 : (c_t == 'r' ? 3 : (c_t == 't' ? 2 : 1)));
                  const int32_t raw_char = getNumFromBits(in, len, bit_no, nibble_len);
                  if (raw_char < 0) {
                      eof = 1;
                      break;
                  }
                  DEC_OUTPUT_CHAR(out, olen, ol++, getHexChar((char)raw_char,
                      c_t == 'f' ? USX_NIB_HEX_LOWER : USX_NIB_HEX_UPPER));
                  bit_no += nibble_len;
              } else
                DEC_OUTPUT_CHAR(out, olen, ol++, c_t);
            }
            if (eof) break; // reach input eof
          } else
          if (idx == 5) {
            int32_t bin_count = readCount(in, &bit_no, len);
            if (bin_count < 0)
              break;
            if (bin_count == 0) // invalid encoding
              break;
            do {
              const int32_t raw_char = getNumFromBits(in, len, bit_no, 8);
              if (raw_char < 0)
                  break;
              DEC_OUTPUT_CHAR(out, olen, ol++, (char)raw_char);
              bit_no += 8;
            } while (--bin_count);
            if (bin_count > 0) break; // reach input eof
          } else {
            int32_t nibble_count = 0;
            if (idx == 2 || idx == 4)
              nibble_count = 32;
            else {
              nibble_count = readCount(in, &bit_no, len);
              if (nibble_count < 0)
                break;
              if (nibble_count == 0) // invalid encoding
                break;
            }
            do {
              int32_t nibble = getNumFromBits(in, len, bit_no, 4);
              if (nibble < 0)
                  break;
              DEC_OUTPUT_CHAR(out, olen, ol++, getHexChar(nibble, idx < 3 ? USX_NIB_HEX_LOWER : USX_NIB_HEX_UPPER));
              if ((idx == 2 || idx == 4) && (nibble_count == 25 || nibble_count == 21 || nibble_count == 17 || nibble_count == 13))
                DEC_OUTPUT_CHAR(out, olen, ol++, '-');
              bit_no += 4;
            } while (--nibble_count);
            if (nibble_count > 0) break; // reach input eof
          }
          if (dstate == USX_DELTA)
            h = USX_DELTA;
          continue;
        }
      }
    }
    if (is_upper && v == 1) {
      //h = dstate = USX_DELTA; // continuous delta coding
      is_upper = 0;
      continue;
    }
    if (is_all_upper)
      is_upper = 1;
    if (h < 3 && v < 28)
      c = usx_sets[h][v];
    if (c >= 'a' && c <= 'z') {
      dstate = USX_ALPHA;
      if (is_upper)
        c -= 32;
    } else {
      if (c >= '0' && c <= '9') {
        dstate = USX_NUM;
      } else if (c == 0) {
        if (v == 8) {
          DEC_OUTPUT_CHAR(out, olen, ol++, '\r');
          DEC_OUTPUT_CHAR(out, olen, ol++, '\n');
        } else if (h == USX_NUM && v == 26) {
          int32_t count = readCount(in, &bit_no, len);
          if (count < 0)
            break;
          count += 4;
          if (ol <= 0)
            return 0; // invalid encoding
          char rpt_c = out[ol - 1];
          while (count--)
            DEC_OUTPUT_CHAR(out, olen, ol++, rpt_c);
        } else
          break; // Terminator
        if (dstate == USX_DELTA)
          h = USX_DELTA;
        is_upper = 0;
        continue;
      }
    }
    is_upper = 0;
    if (dstate == USX_DELTA)
      h = USX_DELTA;
    DEC_OUTPUT_CHAR(out, olen, ol++, c);
  }

  return ol;

}

// Main API function. See unishox2.h for documentation
int unishox3::decompress_simple(const char *in, int len, char *out) {
  return decompress(in, len, USX3_API_OUT_AND_LEN(out, INT_MAX - 1));
}
