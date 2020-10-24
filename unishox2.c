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

#define UNISHOX_VERSION "2.0"

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

#include "unishox2.h"

typedef unsigned char byte;

enum {USX_ALPHA = 0, USX_SYM, USX_NUM, USX_DICT, USX_DELTA};
byte usx_sets[][28] = {{  0, ' ', 'e', 't', 'a', 'o', 'i', 'n',
                        's', 'r', 'l', 'c', 'd', 'h', 'u', 'p', 'm', 'b', 
                        'g', 'w', 'f', 'y', 'v', 'k', 'q', 'j', 'x', 'z'},
                       {'"', '{', '}', '_', '<', '>', ':', '\n',
                          0, '[', ']', '\\', ';', '\'', '\t', '@', '*', '&',
                        '?', '!', '^', '|', '\r', '~', '`', 0, 0, 0},
                       {  0, ',', '.', '/', '(', ')', '-', '1',
                        '0', '9', '2', '3', '4', '5', '6', '7', '8', ' ',
                        '=', '+', '$', '%', '#', 0, 0, 0, 0, 0}};

//                       Alpha,  Sym, Num,  Dict, Delta
byte usx_hcodes[]     = { 0x00, 0x40, 0xE0, 0x80,  0xC0};
byte usx_hcode_lens[] = {    2,    2,    3,    2,     3};
byte usx_vcodes[]   = { 0x00, 0x40, 0x60, 0x80, 0x90, 0xA0, 0xB0,
                        0xC0, 0xD0, 0xD8, 0xE0, 0xE4, 0xE8, 0xEC,
                        0xEE, 0xF0, 0xF2, 0xF4, 0xF6, 0xF7, 0xF8,
                        0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };
byte usx_vcode_lens[] = {  2,    3,    3,    4,    4,    4,    4,
                           4,    5,    5,    6,    6,    6,    7,
                           7,    7,    7,    7,    8,    8,    8,
                           8,    8,    8,    8,    8,    8,    8 };

// Stores position of letter in usx_sets.
// First 3 bits - position in usx_hcodes
// Next  5 bits - position in usx_vcodes
byte usx_code_94[94];

char *usx_cfg_seq[] = {"\": \"", "\": ", "</", "=\"", "\\", "://"};
byte usx_cfg_len[] = {4, 2, 2, 2, 2, 3};
byte usx_cfg_codes[] = {(1 << 3) + 25, (1 << 3) + 26, (1 << 3) + 27, (2 << 3) + 23, (2 << 3) + 24, (2 << 3) + 25};

const int UTF8_MASK[] = {0xE0, 0xF0, 0xF8};
const int UTF8_PREFIX[] = {0xC0, 0xE0, 0xF0};

byte to_match_repeats = 1;
#define USE_64K_LOOKUP 1
#if USE_64K_LOOKUP == 1
byte lookup[65536];
#endif
#define NICE_LEN 5

#define RPT_CODE 26
#define TERM_CODE ((2 << 5) + 27)
#define LF_CODE ((1 << 5) + 7)
#define CRLF_CODE ((1 << 5) + 8)
#define CR_CODE ((1 << 5) + 22)
#define TAB_CODE  ((1 << 5) + 14)
#define NUM_SPC_CODE ((2 << 5) + 17)

#define UNI_STATE_SPL_CODE 0xF8
#define UNI_STATE_SPL_CODE_LEN 5
#define UNI_STATE_SW_CODE 0x80
#define UNI_STATE_SW_CODE_LEN 2

#define SW_CODE 0
#define SW_CODE_LEN 2

#define USX_OFFSET_94 33

byte is_inited = 0;
void init_coder() {
  if (is_inited)
    return;
  memset(usx_code_94, '\0', sizeof(usx_code_94));
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 28; j++) {
      byte c = usx_sets[i][j];
      if (c != 0 && c != 32) {
        usx_code_94[c - USX_OFFSET_94] = (i << 5) + j;
        printf("%c/%c: %d, ", c, c - USX_OFFSET_94, usx_code_94[c - USX_OFFSET_94]);
        if (c >= 'a' && c <= 'z')
          usx_code_94[c - USX_OFFSET_94 - ('a' - 'A')] = (i << 5) + j;
      }
    }
  }
  is_inited = 1;
}

unsigned int usx_mask[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
int append_bits(char *out, int ol, byte code, int clen) {

  byte cur_bit;
  byte blen;
  unsigned char a_byte;

   //printf("%d,%x,%d,%d\n", ol, code, clen, state);

  while (clen > 0) {
     cur_bit = ol % 8;
     blen = clen;
     a_byte = code & usx_mask[blen - 1];
     a_byte >>= cur_bit;
     if (blen + cur_bit > 8)
        blen = (8 - cur_bit);
     if (cur_bit == 0)
        out[ol / 8] = a_byte;
     else
        out[ol / 8] |= a_byte;
     code <<= blen;
     ol += blen;
     clen -= blen;
   }
   return ol;
}

int append_switch_code(char *out, int ol, byte state) {
  if (state == USX_DELTA) {
    ol = append_bits(out, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN);
    ol = append_bits(out, ol, UNI_STATE_SW_CODE, UNI_STATE_SW_CODE_LEN);
  } else
    ol = append_bits(out, ol, SW_CODE, SW_CODE_LEN);
  return ol;
}

int append_code(char *out, int ol, byte code, byte *state) {
  byte hcode = code >> 5;
  byte vcode = code & 0x1F;
  switch (hcode) {
    case USX_ALPHA:
      if (*state != USX_ALPHA) {
        ol = append_switch_code(out, ol, *state);
        ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
        *state = USX_ALPHA;
      }
      break;
    case USX_SYM:
      ol = append_switch_code(out, ol, *state);
      ol = append_bits(out, ol, usx_hcodes[USX_SYM], usx_hcode_lens[USX_SYM]);
      break;
    case USX_NUM:
      if (*state != USX_NUM) {
        ol = append_switch_code(out, ol, *state);
        ol = append_bits(out, ol, usx_hcodes[USX_NUM], usx_hcode_lens[USX_NUM]);
        if (usx_sets[hcode][vcode] >= '0' && usx_sets[hcode][vcode] <= '9')
          *state = USX_NUM;
      }
  }
  return append_bits(out, ol, usx_vcodes[vcode], usx_vcode_lens[vcode]);
}

int encodeCount(char *out, int ol, int count) {
  // First five bits are code and Last three bits of codes represent length
  const byte codes[7] = {0x01, 0x82, 0xC3, 0xE5, 0xED, 0xF5, 0xFD};
  const byte bit_len[7] = {2, 5, 7, 9, 12, 16, 17};
  const uint16_t adder[7] = {0, 4, 36, 164, 676, 4772, 0};
  int till = 0;
  for (int i = 0; i < 6; i++) {
    till += (1 << bit_len[i]);
    if (count < till) {
      ol = append_bits(out, ol, (codes[i] & 0xF8), codes[i] & 0x07);
      uint16_t count16 = (count - adder[i]) << (16 - bit_len[i]);
      if (bit_len[i] > 8) {
        ol = append_bits(out, ol, count16 >> 8, 8);
        ol = append_bits(out, ol, count16 & 0xFF, bit_len[i] - 8);
      } else
        ol = append_bits(out, ol, count16 >> 8, bit_len[i]);
      return ol;
    }
  }
  return ol;
}

const byte uni_bit_len[5] = {6, 12, 14, 16, 21};
const int32_t uni_adder[5] = {0, 64, 4160, 20544, 86080};

int encodeUnicode(char *out, int ol, int32_t code, int32_t prev_code) {
  byte spl_code = (code == ',' ? 0xC0 : 
    ((code == '.' || code == 0x3002) ? 0xE0 : (code == ' ' ? 0 : 
     (code == 13 ? 0xF0 : 0xFF))));
  if (spl_code != 0xFF) {
    int spl_code_len = (code == ',' ? 3 : 
      ((code == '.' || code == 0x3002) ? 4 : (code == ' ' ? 1 :
      (code == 13 ? 4 : 0xFF))));
    ol = append_bits(out, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN);
    ol = append_bits(out, ol, spl_code, spl_code_len);
    return ol;
  }
  // First five bits are code and Last three bits of codes represent length
  //const byte codes[8] = {0x00, 0x42, 0x83, 0xA3, 0xC3, 0xE4, 0xF5, 0xFD};
  const byte codes[6] = {0x01, 0x82, 0xC3, 0xE4, 0xF5, 0xFD};
  int32_t till = 0;
  int orig_ol = ol;
  for (int i = 0; i < 5; i++) {
    till += (1 << uni_bit_len[i]);
    int32_t diff = abs(code - prev_code);
    if (diff < till) {
      ol = append_bits(out, ol, (codes[i] & 0xF8), codes[i] & 0x07);
      //if (diff) {
        ol = append_bits(out, ol, prev_code > code ? 0x80 : 0, 1);
        int32_t val = diff - uni_adder[i];
        if (uni_bit_len[i] > 16) {
          val <<= (24 - uni_bit_len[i]);
          ol = append_bits(out, ol, val >> 16, 8);
          ol = append_bits(out, ol, (val >> 8) & 0xFF, 8);
          ol = append_bits(out, ol, val & 0xFF, uni_bit_len[i] % 16);
        } else
        if (uni_bit_len[i] > 8) {
          val <<= (16 - uni_bit_len[i]);
          ol = append_bits(out, ol, val >> 8, 8);
          ol = append_bits(out, ol, val & 0xFF, uni_bit_len[i] % 8);
        } else
          ol = append_bits(out, ol, val & 0xFF, uni_bit_len[i]);
      //}
      //printf("bits:%d\n", ol-orig_ol);
      return ol;
    }
  }
  return ol;
}

int readUTF8(const char *in, int len, int l, int *utf8len) {
  int bc = 0;
  int uni = 0;
  byte c_in = in[l];
  for (; bc < 3; bc++) {
    if (UTF8_PREFIX[bc] == (c_in & UTF8_MASK[bc]) && len - (bc + 1) > l) {
      int j = 0;
      uni = c_in & ~UTF8_MASK[bc] & 0xFF;
      do {
        uni <<= 6;
        uni += (in[l + j + 1] & 0x3F);
      } while (j++ < bc);
      break;
    }
  }
  if (bc < 3) {
    *utf8len = bc + 1;
    return uni;
  }
  return 0;
}

int matchOccurance(const char *in, int len, int l, char *out, int *ol, byte *state, byte *is_all_upper) {
  int j, k;
  int longest_dist = 0;
  int longest_len = 0;
  for (j = l - NICE_LEN; j >= 0; j--) {
    for (k = l; k < len && j + k - l < l; k++) {
      if (in[k] != in[j + k - l])
        break;
    }
    while ((((unsigned char) in[k]) >> 6) == 2)
      k--; // Skip partial UTF-8 matches
    //if ((in[k - 1] >> 3) == 0x1E || (in[k - 1] >> 4) == 0x0E || (in[k - 1] >> 5) == 0x06)
    //  k--;
    if (k - l > NICE_LEN - 1) {
      int match_len = k - l - NICE_LEN;
      int match_dist = l - j - NICE_LEN + 1;
      if (match_len > longest_len) {
        longest_len = match_len;
        longest_dist = match_dist;
      }
    }
  }
  if (longest_len) {
    *ol = append_switch_code(out, *ol, *state);
    *ol = append_bits(out, *ol, usx_hcodes[USX_DICT], usx_hcode_lens[USX_DICT]);
    //printf("Len:%d / Dist:%d\n", longest_len, longest_dist);
    *ol = encodeCount(out, *ol, longest_len);
    *ol = encodeCount(out, *ol, longest_dist);
    l += (longest_len + NICE_LEN);
    l--;
    return l;
  }
  return -l;
}

int matchLine(const char *in, int len, int l, char *out, int *ol, struct us_lnk_lst *prev_lines, byte *state, byte *is_all_upper) {
  int last_ol = *ol;
  int last_len = 0;
  int last_dist = 0;
  int last_ctx = 0;
  int line_ctr = 0;
  int j = 0;
  do {
    int i, k;
    int line_len = strlen(prev_lines->data);
    int limit = (line_ctr == 0 ? l : line_len);
    for (; j < limit; j++) {
      for (i = l, k = j; k < line_len && i < len; k++, i++) {
        if (prev_lines->data[k] != in[i])
          break;
      }
      while ((((unsigned char) prev_lines->data[k]) >> 6) == 2)
        k--; // Skip partial UTF-8 matches
      if ((k - j) >= NICE_LEN) {
        if (last_len) {
          if (j > last_dist)
            continue;
          //int saving = ((k - j) - last_len) + (last_dist - j) + (last_ctx - line_ctr);
          //if (saving < 0) {
          //  //printf("No savng: %d\n", saving);
          //  continue;
          //}
          *ol = last_ol;
        }
        last_len = (k - j);
        last_dist = j;
        last_ctx = line_ctr;
        *ol = append_switch_code(out, *ol, *state);
        *ol = append_bits(out, *ol, usx_hcodes[USX_DICT], usx_hcode_lens[USX_DICT]);
        *ol = encodeCount(out, *ol, last_len - NICE_LEN);
        *ol = encodeCount(out, *ol, last_dist);
        *ol = encodeCount(out, *ol, last_ctx);
        /*
        if ((*ol - last_ol) > (last_len * 4)) {
          last_len = 0;
          *ol = last_ol;
        }*/
        printf("Len: %d, Dist: %d, Line: %d\n", last_len, last_dist, last_ctx);
        j += last_len;
      }
    }
    line_ctr++;
    prev_lines = prev_lines->previous;
  } while (prev_lines && prev_lines->data != NULL);
  if (last_len) {
    l += last_len;
    l--;
    return l;
  }
  return -l;
}

int unishox1_compress(const char *in, int len, char *out, struct us_lnk_lst *prev_lines) {

  char *ptr;
  byte bits;
  byte state;

  int l, ll, ol;
  char c_in, c_next;
  int prev_uni;
  byte is_upper, is_all_upper, is_sentence_start;

  init_coder();
  ol = 0;
  prev_uni = 0;
#if USE_64K_LOOKUP == 1
  memset(lookup, 0, sizeof(lookup));
#endif
  state = USX_ALPHA;
  is_all_upper = 0;
  is_sentence_start = 1;
  for (l=0; l<len; l++) {

    c_in = in[l];

    if (state != USX_DELTA && l && l < len - 4) {
      if (c_in == in[l - 1] && c_in == in[l + 1] && c_in == in[l + 2] && c_in == in[l + 3]) {
        int rpt_count = l + 4;
        while (rpt_count < len && in[rpt_count] == c_in)
          rpt_count++;
        rpt_count -= l;
        ol = append_switch_code(out, ol, state);
        ol = append_bits(out, ol, usx_hcodes[USX_NUM], usx_hcode_lens[USX_NUM]);
        ol = append_bits(out, ol, usx_vcodes[RPT_CODE], usx_vcode_lens[RPT_CODE]);
        ol = encodeCount(out, ol, rpt_count - 4);
        l += rpt_count;
        l--;
        continue;
      }
    }

    for (int i = 0; i < sizeof(usx_cfg_len); i++) {
      if (len - usx_cfg_len[i] > 0 && l < len - usx_cfg_len[i]) {
        if (memcmp(usx_cfg_seq[i], in + l, usx_cfg_len[i]) == 0) {
          byte hcode = usx_cfg_codes[i] >> 5;
          byte vcode = usx_cfg_codes[i] & 0x1F;
          ol = append_bits(out, ol, SW_CODE, SW_CODE_LEN);
          ol = append_bits(out, ol, usx_hcodes[hcode], usx_hcode_lens[hcode]);
          ol = append_bits(out, ol, usx_vcodes[vcode], usx_vcode_lens[vcode]);
        }
      }
    }

    if (to_match_repeats && l < (len - NICE_LEN + 1)) {
      if (prev_lines) {
        l = matchLine(in, len, l, out, &ol, prev_lines, &state, &is_all_upper);
        if (l > 0) {
          continue;
        }
        l = -l;
      } else {
    #if USE_64K_LOOKUP == 1
        uint16_t to_lookup = c_in ^ in[l + 1] + ((in[l + 2] ^ in[l + 3]) << 8);
        if (lookup[to_lookup]) {
    #endif
          l = matchOccurance(in, len, l, out, &ol, &state, &is_all_upper);
          if (l > 0) {
            continue;
          }
          l = -l;
    #if USE_64K_LOOKUP == 1
        } else
          lookup[to_lookup] = 1;
    #endif
      }
    }

    is_upper = 0;
    if (c_in >= 'A' && c_in <= 'Z')
      is_upper = 1;
    else {
      if (is_all_upper) {
        is_all_upper = 0;
        ol = append_switch_code(out, ol, state);
        ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
      }
    }
    if (is_upper) {
      ol = append_switch_code(out, ol, state);
      ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
    }
    c_next = 0;
    if (l+1 < len)
      c_next = in[l+1];

    if (c_in >= 32 && c_in <= 126) {
      if (is_upper && !is_all_upper) {
        for (ll=l+5; ll>=l && ll<len; ll--) {
          if (in[ll] < 'A' || in[ll] > 'Z')
            break;
        }
        if (ll == l-1) {
          ol = append_switch_code(out, ol, state);
          ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
          is_all_upper = 1;
        }
      }
      c_in -= 32;
      if (is_all_upper && is_upper)
        c_in += 32;
      if (c_in == 0) {
        if (state == USX_NUM)
          ol = append_bits(out, ol, usx_vcodes[NUM_SPC_CODE & 0x1F], usx_vcode_lens[NUM_SPC_CODE & 0x1F]);
        else
          ol = append_bits(out, ol, usx_vcodes[1], usx_vcode_lens[1]);
      } else {
        c_in--;
        ol = append_code(out, ol, usx_code_94[c_in], &state);
      }
    } else
    if (c_in == 13 && c_next == 10) {
      ol = append_switch_code(out, ol, state);
      ol = append_bits(out, ol, usx_hcodes[USX_SYM], usx_hcode_lens[USX_SYM]);
      ol = append_bits(out, ol, usx_vcodes[CRLF_CODE & 0x1F], CRLF_CODE >> 5);
      l++;
    } else
    if (c_in == 10) {
      ol = append_switch_code(out, ol, state);
      ol = append_bits(out, ol, usx_hcodes[USX_SYM], usx_hcode_lens[USX_SYM]);
      ol = append_bits(out, ol, usx_vcodes[LF_CODE & 0x1F], LF_CODE >> 5);
    } else
    if (c_in == 13) {
      ol = append_switch_code(out, ol, state);
      ol = append_bits(out, ol, usx_hcodes[USX_SYM], usx_hcode_lens[USX_SYM]);
      ol = append_bits(out, ol, usx_vcodes[CR_CODE & 0x1F], CR_CODE >> 5);
    } else
    if (c_in == '\t') {
      ol = append_switch_code(out, ol, state);
      ol = append_bits(out, ol, usx_hcodes[USX_SYM], usx_hcode_lens[USX_SYM]);
      ol = append_bits(out, ol, usx_vcodes[TAB_CODE & 0x1F], TAB_CODE >> 5);
    } else {
      int utf8len;
      int uni = readUTF8(in, len, l, &utf8len);
      if (uni) {
        l += utf8len;
        if (state != USX_DELTA) {
          ol = append_switch_code(out, ol, state);
          ol = append_bits(out, ol, usx_hcodes[USX_SYM], usx_hcode_lens[USX_SYM]);
          int uni2 = readUTF8(in, len, l + 1, &utf8len);
          if (uni2) {
            state = USX_DELTA;
            ol = append_bits(out, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN);
            ol = append_bits(out, ol, UNI_STATE_SW_CODE, UNI_STATE_SW_CODE_LEN);
          }
        }
        ol = encodeUnicode(out, ol, uni, prev_uni);
        //printf("%d:%d:%d,", l, utf8len, uni);
        if (uni != 0x3002)
          prev_uni = uni;
      } else {
        printf("Bin:%d:%x\n", (unsigned char) c_in, (unsigned char) c_in);
        ol = encodeUnicode(out, ol, uni, prev_uni);
      }
    }
  }
  if (state == USX_DELTA) {
    ol = append_bits(out, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN);
    ol = append_bits(out, ol, UNI_STATE_SW_CODE, UNI_STATE_SW_CODE_LEN);
  }
  bits = ol % 8;
  int ret = ol/8+(ol%8?1:0);
  if (bits) {
    ol = append_switch_code(out, ol, state);
    ol = append_bits(out, ol, usx_hcodes[USX_SYM], usx_hcode_lens[USX_SYM]);
    ol = append_bits(out, ol, usx_vcodes[CR_CODE & 0x1F], CR_CODE >> 5);
  }
  //printf("\n%ld\n", ol);
  return ret;

}

int readBit(const char *in, int bit_no) {
   return in[bit_no >> 3] & (0x80 >> (bit_no % 8));
}

byte len_masks[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};

int read8bitCode(const char *in, int len, int *bit_no_p, int char_pos) {
  int bit_pos = *bit_no_p % 8;
  byte b = in[char_pos];
  byte code = (b << bit_pos);
  if (bit_pos && ++char_pos < len) {
    b = in[char_pos];
    b >>= (8 - bit_pos);
    code |= b;
  }
  return code;
}

// Decoder is designed for using less memory, not speed
#define SECTION_COUNT 5
byte usx_vsections[] = {0x7F, 0xBF, 0xDF, 0xEF, 0xFF};
byte usx_vsection_pos[] = {0, 4, 8, 12, 20};
byte usx_vsection_mask[] = {0x7F, 0x3F, 0x1F, 0x0F, 0x0F};
byte usx_vsection_shift[] = {5, 4, 3, 1, 0};

// Vertical decoder lookup table - 3 bits code len, 5 bytes vertical pos
// code len is one less as 8 cannot be accommodated in 3 bits
byte usx_vcode_lookup[36] = {
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

int readVCodeIdx(const char *in, int len, int *bit_no_p) {
  int char_pos = *bit_no_p >> 3;
  if (char_pos < len) {
    byte code = read8bitCode(in, len, bit_no_p, char_pos);
    int i = 0;
    do {
      if (code <= usx_vsections[i]) {
        byte vcode = usx_vcode_lookup[usx_vsection_pos[i] + ((code & usx_vsection_mask[i]) >> usx_vsection_shift[i])];
        (*bit_no_p) += ((vcode >> 5) + 1);
        return vcode & 0x1F;
      }
    } while (++i < SECTION_COUNT);
  }
  return 99;
}

int readHCodeIdx(const char *in, int len, int *bit_no_p) {
  int char_pos = *bit_no_p >> 3;
  if (char_pos < len) {
    byte code = read8bitCode(in, len, bit_no_p, char_pos);
    for (int code_pos = 0; code_pos < 5; code_pos++) {
      if ((code & len_masks[usx_hcode_lens[code_pos] - 1]) == usx_hcodes[code_pos]) {
        *bit_no_p += usx_hcode_lens[code_pos];
        return code_pos;
      }
    }
  }
  return 99;
}

// TODO: Last value check.. Also len check in readBit
int getStepCodeIdx(const char *in, int len, int *bit_no_p) {
  int idx = 0;
  while (readBit(in, *bit_no_p)) {
    idx++;
    (*bit_no_p)++;
    if (idx == 4)
      break;
  }
  (*bit_no_p)++;
  return idx;
}

int32_t getNumFromBits(const char *in, int bit_no, int count) {
   int32_t ret = 0;
   while (count--) {
     ret += (readBit(in, bit_no) ? 1 << count : 0);
     bit_no++;
   }
   return ret;
}

int readCount(const char *in, int *bit_no_p, int len) {
  const byte bit_len[7]   = {2, 5,  7,   9,  12,   16, 17};
  const uint16_t adder[7] = {0, 4, 36, 164, 676, 4772,  0};
  int idx = getStepCodeIdx(in, len, bit_no_p);
  if (idx > 6)
    return 0;
  int count = getNumFromBits(in, *bit_no_p, bit_len[idx]) + adder[idx];
  (*bit_no_p) += bit_len[idx];
  return count;
}

int32_t readUnicode(const char *in, int *bit_no_p, int len) {
  int idx = getStepCodeIdx(in, len, bit_no_p);
  if (idx >= 5) {
    int idx = readHCodeIdx(in, len, bit_no_p);
    return 0x7FFFFF00 + idx;
  }
  if (idx >= 0) {
    int sign = readBit(in, *bit_no_p);
    (*bit_no_p)++;
    int32_t count = getNumFromBits(in, *bit_no_p, uni_bit_len[idx]);
    count += uni_adder[idx];
    (*bit_no_p) += uni_bit_len[idx];
    return sign ? -count : count;
  }
  return 0;
}

void writeUTF8(char *out, int *ol, int uni) {
  if (uni < (1 << 11)) {
    out[(*ol)++] = (0xC0 + (uni >> 6));
    out[(*ol)++] = (0x80 + (uni & 63));
  } else
  if (uni < (1 << 16)) {
    out[(*ol)++] = (0xE0 + (uni >> 12));
    out[(*ol)++] = (0x80 + ((uni >> 6) & 63));
    out[(*ol)++] = (0x80 + (uni & 63));
  } else {
    out[(*ol)++] = (0xF0 + (uni >> 18));
    out[(*ol)++] = (0x80 + ((uni >> 12) & 63));
    out[(*ol)++] = (0x80 + ((uni >> 6) & 63));
    out[(*ol)++] = (0x80 + (uni & 63));
  }
}

int decodeRepeat(const char *in, int len, char *out, int ol, int *bit_no, struct us_lnk_lst *prev_lines) {
  if (prev_lines) {
    int dict_len = readCount(in, bit_no, len) + NICE_LEN;
    int dist = readCount(in, bit_no, len);
    int ctx = readCount(in, bit_no, len);
    struct us_lnk_lst *cur_line = prev_lines;
    while (ctx--)
      cur_line = cur_line->previous;
    memmove(out + ol, cur_line->data + dist, dict_len);
    ol += dict_len;
  } else {
    int dict_len = readCount(in, bit_no, len) + NICE_LEN;
    int dist = readCount(in, bit_no, len) + NICE_LEN - 1;
    memcpy(out + ol, out + ol - dist, dict_len);
    ol += dict_len;
  }
  return ol;
}

int unishox1_decompress(const char *in, int len, char *out, struct us_lnk_lst *prev_lines) {

  int dstate;
  int bit_no;
  int h, v;
  byte is_all_upper;

  init_coder();
  int ol = 0;
  bit_no = 0;
  dstate = h = USX_ALPHA;
  is_all_upper = 0;

  int prev_uni = 0;

  len <<= 3;
  out[ol] = 0;
  while (bit_no < len) {
    int orig_bit_no = bit_no;
    if (dstate == USX_DELTA || h == USX_DELTA) {
      if (dstate != USX_DELTA && h == USX_DELTA)
        h = dstate;
      int32_t delta = readUnicode(in, &bit_no, len);
      if ((delta >> 8) == 0x7FFFFF) {
        int spl_code_idx = delta & 0x000000FF;
        if (spl_code_idx == 2)
          break;
        switch (spl_code_idx) {
          case 0:
            out[ol++] = ' ';
            break;
          case 1:
            h = readHCodeIdx(in, len, &bit_no);
            if (h == USX_DELTA || h == USX_ALPHA) {
              dstate = h;
              continue;
            }
            break;
          case 2:
            out[ol++] = ',';
            break;
          case 3:
            if (prev_uni > 0x3000)
              writeUTF8(out, &ol, 0x3002);
            else
              out[ol++] = '.';
            break;
          case 4:
            out[ol++] = 10;
        }
      } else {
        prev_uni += delta;
        writeUTF8(out, &ol, prev_uni);
      }
      if (dstate == USX_DELTA && h == USX_DELTA)
        continue;
    } else
      h = dstate;
    char c = 0;
    byte is_upper = is_all_upper;
    v = readVCodeIdx(in, len, &bit_no);
    if (v == 99 || h == 99) {
      bit_no = orig_bit_no;
      break;
    }
    if (v == 0) {
      if (bit_no >= len)
        break;
      h = readHCodeIdx(in, len, &bit_no);
      if (h == 99 || bit_no >= len) {
        bit_no = orig_bit_no;
        break;
      }
      if (h == USX_ALPHA) {
         if (dstate == USX_ALPHA) {
           if (is_all_upper) {
             is_upper = is_all_upper = 0;
             continue;
           }
           v = readVCodeIdx(in, len, &bit_no);
           if (v == 99) {
             bit_no = orig_bit_no;
             break;
           }
           if (v == 0) {
              h = readHCodeIdx(in, len, &bit_no);
              if (h == 99) {
                bit_no = orig_bit_no;
                break;
              }
              if (h == USX_ALPHA) {
                 is_all_upper = 1;
                 continue;
              }
           }
           is_upper = 1;
         } else {
            dstate = USX_ALPHA;
            continue;
         }
      } else
      if (h == USX_DICT) {
        ol = decodeRepeat(in, len, out, ol, &bit_no, prev_lines);
        continue;
      } else
      if (h == USX_DELTA) {
        //printf("Sign: %d, bitno: %d\n", sign, bit_no);
        //printf("Code: %d\n", prev_uni);
        //printf("BitNo: %d\n", bit_no);
        continue;
      } else
        v = readVCodeIdx(in, len, &bit_no);
    }
    // TODO: Binary    out[ol++] = readCount(in, &bit_no, len);
    if (h < 3 && v < 28)
      c = usx_sets[h][v];
    if (c >= 'a' && c <= 'z') {
      dstate = USX_ALPHA;
      if (is_upper)
        c -= 32;
    } else {
      if (c >= '1' && c <= '9')
        dstate = USX_NUM;
      else if (c == 0) {
        if (v == 8) {
          out[ol++] = '\r';
          out[ol++] = '\n';
        } else if (h == USX_NUM && v == 26) {
          int count = readCount(in, &bit_no, len);
          count += 4;
          char rpt_c = out[ol - 1];
          while (count--)
            out[ol++] = rpt_c;
        } else if (h == USX_SYM && v > 24) {
          v -= 25;
          memcpy(out, usx_cfg_seq[v], usx_cfg_len[v]);
          ol += usx_cfg_len[v];
        } else if (h == USX_SYM && v > 22 && v < 26) {
          v -= (23 + 3);
          memcpy(out, usx_cfg_seq[v], usx_cfg_len[v]);
          ol += usx_cfg_len[v];
        } else
          break; // Terminator
        continue;
      }
    }
    out[ol++] = c;
  }

  return ol;

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

void print_string_as_hex(char *in, int len) {

  int l;
  byte bit;
  printf("String in hex:\n");
  for (l=0; l<len; l++) {
    printf("%x, ", (unsigned char) in[l]);
  }
  printf("\n");

}

void print_compressed(char *in, int len) {

  int l;
  byte bit;
  printf("Compressed bytes in decimal:\n");
  for (l=0; l<len; l++) {
    printf("%d, ", in[l]);
  }
  printf("\n\nCompressed bytes in binary:\n");
  for (l=0; l<len*8; l++) {
    bit = (in[l/8]>>(7-l%8))&0x01;
    printf("%d", bit);
    if (l%8 == 7) printf(" ");
  }
  printf("\n");

}

uint32_t getTimeVal() {
#ifdef _MSC_VER
    return GetTickCount() * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000) + tv.tv_usec;
#endif
}

double timedifference(uint32_t t0, uint32_t t1) {
    double ret = t1;
    ret -= t0;
    ret /= 1000;
    return ret;
}

int main(int argv, char *args[]) {

char cbuf[1024];
char dbuf[1024];
long len, tot_len, clen, ctot, dlen, l;
float perc;
FILE *fp, *wfp;
int bytes_read;
char c_in;
uint32_t tStart;

tStart = getTimeVal();

if (argv == 4 && strcmp(args[1], "-c") == 0) {
   tot_len = 0;
   ctot = 0;
   fp = fopen(args[2], "rb");
   if (fp == NULL) {
      perror(args[2]);
      return 1;
   }
   wfp = fopen(args[3], "wb+");
   if (wfp == NULL) {
      perror(args[3]);
      return 1;
   }
   do {
     bytes_read = fread(cbuf, 1, sizeof(cbuf), fp);
     if (bytes_read > 0) {
        clen = unishox1_compress(cbuf, bytes_read, dbuf, NULL);
        ctot += clen;
        tot_len += bytes_read;
        if (clen > 0) {
           fputc(clen >> 8, wfp);
           fputc(clen & 0xFF, wfp);
           if (clen != fwrite(dbuf, 1, clen, wfp)) {
              perror("fwrite");
              return 1;
           }
        }
     }
   } while (bytes_read > 0);
   perc = (tot_len-ctot);
   perc /= tot_len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, tot_len);
   printf("%.2f%%\n", perc);
} else
if (argv == 4 && strcmp(args[1], "-d") == 0) {
   fp = fopen(args[2], "rb");
   if (fp == NULL) {
      perror(args[2]);
      return 1;
   }
   wfp = fopen(args[3], "wb+");
   if (wfp == NULL) {
      perror(args[3]);
      return 1;
   }
   do {
     //memset(dbuf, 0, sizeof(dbuf));
     int len_to_read = fgetc(fp) << 8;
     len_to_read += fgetc(fp);
     bytes_read = fread(dbuf, 1, len_to_read, fp);
     if (bytes_read > 0) {
        dlen = unishox1_decompress(dbuf, bytes_read, cbuf, NULL);
        if (dlen > 0) {
           if (dlen != fwrite(cbuf, 1, dlen, wfp)) {
              perror("fwrite");
              return 1;
           }
        }
     }
   } while (bytes_read > 0);
} else
if (argv == 4 && (strcmp(args[1], "-g") == 0 || 
      strcmp(args[1], "-G") == 0)) {
   if (strcmp(args[1], "-g") == 0)
     to_match_repeats = 0;
   fp = fopen(args[2], "r");
   if (fp == NULL) {
      perror(args[2]);
      return 1;
   }
   sprintf(cbuf, "%s.h", args[3]);
   wfp = fopen(cbuf, "w+");
   if (wfp == NULL) {
      perror(args[3]);
      return 1;
   }
   tot_len = 0;
   ctot = 0;
   struct us_lnk_lst *cur_line = NULL;
   fputs("#ifndef __", wfp);
   fputs(args[3], wfp);
   fputs("_UNISHOX1_COMPRESSED__\n", wfp);
   fputs("#define __", wfp);
   fputs(args[3], wfp);
   fputs("_UNISHOX1_COMPRESSED__\n", wfp);
   int line_ctr = 0;
   int max_len = 0;
   while (fgets(cbuf, sizeof(cbuf), fp) != NULL) {
      // compress the line and look in previous lines
      // add to linked list
      len = strlen(cbuf);
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
        clen = unishox1_compress(cbuf, len, dbuf, cur_line);
        if (clen > 0) {
            perc = (len-clen);
            perc /= len;
            perc *= 100;
            //print_compressed(dbuf, clen);
            printf("len: %ld/%ld=", clen, len);
            printf("%.2f %s\n", perc, cbuf);
            tot_len += len;
            ctot += clen;
            char short_buf[strlen(args[3]) + 100];
            snprintf(short_buf, sizeof(short_buf), "const byte %s_%d[] PROGMEM = {", args[3], line_ctr++);
            fputs(short_buf, wfp);
            int len_len = encode_unsigned_varint((byte *) short_buf, clen);
            for (int i = 0; i < len_len; i++) {
              snprintf(short_buf, 10, "%u, ", (byte) short_buf[i]);
              fputs(short_buf, wfp);
            }
            for (int i = 0; i < clen; i++) {
              if (i) {
                strcpy(short_buf, ", ");
                fputs(short_buf, wfp);
              }
              snprintf(short_buf, 6, "%u", (byte) dbuf[i]);
              fputs(short_buf, wfp);
            }
            strcpy(short_buf, "};\n");
            fputs(short_buf, wfp);
        }
        if (len > max_len)
          max_len = len;
        dlen = unishox1_decompress(dbuf, clen, cbuf, cur_line);
        cbuf[dlen] = 0;
        printf("\n%s\n", cbuf);
      }
   }
   perc = (tot_len-ctot);
   perc /= tot_len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, tot_len);
   printf("%.2f%%\n", perc);
   char short_buf[strlen(args[3]) + 100];
   snprintf(short_buf, sizeof(short_buf), "const byte * const %s[] PROGMEM = {", args[3]);
   fputs(short_buf, wfp);
   for (int i = 0; i < line_ctr; i++) {
     if (i) {
       strcpy(short_buf, ", ");
       fputs(short_buf, wfp);
     }
     snprintf(short_buf, strlen(args[3]) + 15, "%s_%d", args[3], i);
     fputs(short_buf, wfp);
   }
   strcpy(short_buf, "};\n");
   fputs(short_buf, wfp);
   snprintf(short_buf, sizeof(short_buf), "#define %s_line_count %d\n", args[3], line_ctr);
   fputs(short_buf, wfp);
   snprintf(short_buf, sizeof(short_buf), "#define %s_max_len %d\n", args[3], max_len);
   fputs(short_buf, wfp);
   fputs("#endif\n", wfp);
} else
if (argv == 2) {
   len = strlen(args[1]);
   printf("String: %s, Len:%ld\n", args[1], len);
   //print_string_as_hex(args[1], len);
   memset(cbuf, 0, sizeof(cbuf));
   ctot = unishox1_compress(args[1], len, cbuf, NULL);
   print_compressed(cbuf, ctot);
   memset(dbuf, 0, sizeof(dbuf));
   dlen = unishox1_decompress(cbuf, ctot, dbuf, NULL);
   dbuf[dlen] = 0;
   printf("\nDecompressed: %s\n", dbuf);
   //print_compressed(dbuf, dlen);
   perc = (len-ctot);
   perc /= len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, len);
   printf("%.2f%%\n", perc);
} else {
   printf("Unishox (byte format version: %s)\n", UNISHOX_VERSION);
   printf("---------------------------------\n");
   printf("Usage: unishox1 \"string\" or unishox1 [action] [in_file] [out_file] [encoding]\n");
   printf("\n");
   printf("Actions:\n");
   printf("  -c    compress\n");
   printf("  -d    decompress\n");
   printf("  -g    generate C header file\n");
   printf("  -G    generate C header file using additional compression (slower)\n");
   return 1;
}

printf("\nElapsed: %0.3lf ms\n", timedifference(tStart, getTimeVal()));

return 0;

}
