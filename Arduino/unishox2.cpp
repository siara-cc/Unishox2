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
 * Port for Particle (particle.io) / Aruino - Jonathan Greenblatt
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "unishox2.h"

typedef unsigned char byte;
const char * USX_FREQ_SEQ_DFLT[] = {"\": \"", "\": ", "</", "=\"", "\":\"", "://"};
const char * USX_FREQ_SEQ_TXT[] = {" the ", " and ", "tion", " with", "ing", "ment"};
const char * USX_FREQ_SEQ_URL[] = {"https://", "www.", ".com", "http://", ".org", ".net"};
const char * USX_FREQ_SEQ_JSON[] = {"\": \"", "\": ", "\",", "}}}", "\":\"", "}}"};
const char * USX_FREQ_SEQ_HTML[]  = {"</", "=\"", "div", "href", "class", "<p>"};
const char * USX_FREQ_SEQ_XML[]  = {"</", "=\"", "\">", "<?xml version=\"1.0\"", "xmlns:", "://"};
const char * USX_TEMPLATES[] = {"tfff-of-tfTtf:rf:rf.fffZ", "tfff-of-tf", "(fff) fff-ffff", "tf:rf:rf", 0};


enum {USX_ALPHA = 0, USX_SYM, USX_NUM, USX_DICT, USX_DELTA, USX_NUM_TEMP};
const byte usx_sets[][28] = {{  0, ' ', 'e', 't', 'a', 'o', 'i', 'n',
                        's', 'r', 'l', 'c', 'd', 'h', 'u', 'p', 'm', 'b',
                        'g', 'w', 'f', 'y', 'v', 'k', 'q', 'j', 'x', 'z'},
                       {'"', '{', '}', '_', '<', '>', ':', '\n',
                          0, '[', ']', '\\', ';', '\'', '\t', '@', '*', '&',
                        '?', '!', '^', '|', '\r', '~', '`', 0, 0, 0},
                       {  0, ',', '.', '0', '1', '9', '2', '5', '-',
                        '/', '3', '4', '6', '7', '8', '(', ')', ' ',
                        '=', '+', '$', '%', '#', 0, 0, 0, 0, 0}};

// Stores position of letter in usx_sets.
// First 3 bits - position in usx_hcodes
// Next  5 bits - position in usx_vcodes
byte usx_code_94[94];

const byte usx_vcodes[]   = { 0x00, 0x40, 0x60, 0x80, 0x90, 0xA0, 0xB0,
                        0xC0, 0xD0, 0xD8, 0xE0, 0xE4, 0xE8, 0xEC,
                        0xEE, 0xF0, 0xF2, 0xF4, 0xF6, 0xF7, 0xF8,
                        0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };
const byte usx_vcode_lens[] = {  2,    3,    3,    4,    4,    4,    4,
                           4,    5,    5,    6,    6,    6,    7,
                           7,    7,    7,    7,    8,    8,    8,
                           8,    8,    8,    8,    8,    8,    8 };

const byte usx_freq_codes[] = {(1 << 5) + 25, (1 << 5) + 26, (1 << 5) + 27, (2 << 5) + 23, (2 << 5) + 24, (2 << 5) + 25};

const int UTF8_MASK[] = {0xE0, 0xF0, 0xF8};
const int UTF8_PREFIX[] = {0xC0, 0xE0, 0xF0};

#define NICE_LEN 5

#define RPT_CODE ((2 << 5) + 26)
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
      if (c != 0 && c > 32) {
        usx_code_94[c - USX_OFFSET_94] = (i << 5) + j;
        if (c >= 'a' && c <= 'z')
          usx_code_94[c - USX_OFFSET_94 - ('a' - 'A')] = (i << 5) + j;
      }
    }
  }
  is_inited = 1;
}

const unsigned int usx_mask[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
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

int append_code(char *out, int ol, byte code, byte *state, const byte usx_hcodes[], const byte usx_hcode_lens[]) {
  byte hcode = code >> 5;
  byte vcode = code & 0x1F;
  if (!usx_hcode_lens[hcode] && hcode != USX_ALPHA)
    return ol;
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

const byte count_bit_lens[5] = {2, 4, 7, 11, 16};
const int32_t count_adder[5] = {4, 20, 148, 2196, 67732};
const byte count_codes[] = {0x01, 0x82, 0xC3, 0xE4, 0xF4};
int encodeCount(char *out, int ol, int count) {
  // First five bits are code and Last three bits of codes represent length
  for (int i = 0; i < 5; i++) {
    if (count < count_adder[i]) {
      ol = append_bits(out, ol, (count_codes[i] & 0xF8), count_codes[i] & 0x07);
      uint16_t count16 = (count - (i ? count_adder[i - 1] : 0)) << (16 - count_bit_lens[i]);
      if (count_bit_lens[i] > 8) {
        ol = append_bits(out, ol, count16 >> 8, 8);
        ol = append_bits(out, ol, count16 & 0xFF, count_bit_lens[i] - 8);
      } else
        ol = append_bits(out, ol, count16 >> 8, count_bit_lens[i]);
      return ol;
    }
  }
  return ol;
}

const byte uni_bit_len[5] = {6, 12, 14, 16, 21};
const int32_t uni_adder[5] = {0, 64, 4160, 20544, 86080};

int encodeUnicode(char *out, int ol, int32_t code, int32_t prev_code) {
  // First five bits are code and Last three bits of codes represent length
  //const byte codes[8] = {0x00, 0x42, 0x83, 0xA3, 0xC3, 0xE4, 0xF5, 0xFD};
  const byte codes[6] = {0x01, 0x82, 0xC3, 0xE4, 0xF5, 0xFD};
  int32_t till = 0;
  int32_t diff = code - prev_code;
  if (diff < 0)
    diff = -diff;
  //printf("%ld, ", code);
  //printf("Diff: %d\n", diff);
  for (int i = 0; i < 5; i++) {
    till += (1 << uni_bit_len[i]);
    if (diff < till) {
      ol = append_bits(out, ol, (codes[i] & 0xF8), codes[i] & 0x07);
      //if (diff) {
        ol = append_bits(out, ol, prev_code > code ? 0x80 : 0, 1);
        int32_t val = diff - uni_adder[i];
        //printf("Val: %d\n", val);
        if (uni_bit_len[i] > 16) {
          val <<= (24 - uni_bit_len[i]);
          ol = append_bits(out, ol, val >> 16, 8);
          ol = append_bits(out, ol, (val >> 8) & 0xFF, 8);
          ol = append_bits(out, ol, val & 0xFF, uni_bit_len[i] - 16);
        } else
        if (uni_bit_len[i] > 8) {
          val <<= (16 - uni_bit_len[i]);
          ol = append_bits(out, ol, val >> 8, 8);
          ol = append_bits(out, ol, val & 0xFF, uni_bit_len[i] - 8);
        } else {
          val <<= (8 - uni_bit_len[i]);
          ol = append_bits(out, ol, val & 0xFF, uni_bit_len[i]);
        }
      //}
      //printf("bits:%d\n", ol-orig_ol);
      return ol;
    }
  }
  return ol;
}

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

int matchOccurance(const char *in, int len, int l, char *out, int *ol, byte *state, const byte usx_hcodes[], const byte usx_hcode_lens[]) {
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
    if ((k - l) > (NICE_LEN - 1)) {
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
    //printf("Len:%d / Dist:%d/%.*s\n", longest_len, longest_dist, longest_len + NICE_LEN, in + l - longest_dist - NICE_LEN + 1);
    *ol = encodeCount(out, *ol, longest_len);
    *ol = encodeCount(out, *ol, longest_dist);
    l += (longest_len + NICE_LEN);
    l--;
    return l;
  }
  return -l;
}

int matchLine(const char *in, int len, int l, char *out, int *ol, struct us_lnk_lst *prev_lines, byte *state, const byte usx_hcodes[], const byte usx_hcode_lens[]) {
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
        //printf("Len: %d, Dist: %d, Line: %d\n", last_len, last_dist, last_ctx);
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

byte getBaseCode(char ch) {
  if (ch >= '0' && ch <= '9')
    return (ch - '0') << 4;
  else if (ch >= 'A' && ch <= 'F')
    return (ch - 'A' + 10) << 4;
  else if (ch >= 'a' && ch <= 'f')
    return (ch - 'a' + 10) << 4;
  return 0;
}

enum {USX_NIB_NUM = 0, USX_NIB_HEX_LOWER, USX_NIB_HEX_UPPER, USX_NIB_NOT};
char getNibbleType(char ch) {
  if (ch >= '0' && ch <= '9')
    return USX_NIB_NUM;
  else if (ch >= 'a' && ch <= 'f')
    return USX_NIB_HEX_LOWER;
  else if (ch >= 'A' && ch <= 'F')
    return USX_NIB_HEX_UPPER;
  return USX_NIB_NOT;
}

int append_nibble_escape(char *out, int ol, byte state, const byte usx_hcodes[], const byte usx_hcode_lens[]) {
  ol = append_switch_code(out, ol, state);
  ol = append_bits(out, ol, usx_hcodes[USX_NUM], usx_hcode_lens[USX_NUM]);
  ol = append_bits(out, ol, 0, 2);
  return ol;
}

int unishox2_compress_lines(const char *in, int len, char *out, const byte usx_hcodes[], const byte usx_hcode_lens[], const char *usx_freq_seq[], const char *usx_templates[], struct us_lnk_lst *prev_lines) {

  byte state;

  int l, ll, ol;
  char c_in, c_next;
  int prev_uni;
  byte is_upper, is_all_upper;

  init_coder();
  ol = 0;
  prev_uni = 0;
  state = USX_ALPHA;
  is_all_upper = 0;
  ol = append_bits(out, ol, 0x80, 1); // magic bit
  for (l=0; l<len; l++) {

    if (usx_hcode_lens[USX_DICT] && l < (len - NICE_LEN + 1)) {
      if (prev_lines) {
        l = matchLine(in, len, l, out, &ol, prev_lines, &state, usx_hcodes, usx_hcode_lens);
        if (l > 0) {
          continue;
        }
        l = -l;
      } else {
          l = matchOccurance(in, len, l, out, &ol, &state, usx_hcodes, usx_hcode_lens);
          if (l > 0) {
            continue;
          }
          l = -l;
      }
    }

    c_in = in[l];
    if (l && len > 4 && l < (len - 4) && usx_hcode_lens[USX_NUM]) {
      if (c_in == in[l - 1] && c_in == in[l + 1] && c_in == in[l + 2] && c_in == in[l + 3]) {
        int rpt_count = l + 4;
        while (rpt_count < len && in[rpt_count] == c_in)
          rpt_count++;
        rpt_count -= l;
        ol = append_code(out, ol, RPT_CODE, &state, usx_hcodes, usx_hcode_lens);
        ol = encodeCount(out, ol, rpt_count - 4);
        l += rpt_count;
        l--;
        continue;
      }
    }

    if (l <= (len - 36) && usx_hcode_lens[USX_NUM]) {
      if (in[l + 8] == '-' && in[l + 13] == '-' && in[l + 18] == '-' && in[l + 23] == '-') {
        char hex_type = USX_NIB_NUM;
        int uid_pos = l;
        for (; uid_pos < l + 36; uid_pos++) {
          char c_uid = in[uid_pos];
          if (c_uid == '-')
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
        if (uid_pos == l + 36) {
          ol = append_nibble_escape(out, ol, state, usx_hcodes, usx_hcode_lens);
          ol = append_bits(out, ol, (hex_type == USX_NIB_HEX_LOWER ? 0xC0 : 0xF0),
                 (hex_type == USX_NIB_HEX_LOWER ? 3 : 5));
          for (uid_pos = l; uid_pos < l + 36; uid_pos++) {
            char c_uid = in[uid_pos];
            if (c_uid != '-')
              ol = append_bits(out, ol, getBaseCode(c_uid), 4);
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
        ol = append_nibble_escape(out, ol, state, usx_hcodes, usx_hcode_lens);
        ol = append_bits(out, ol, (hex_type == USX_NIB_HEX_LOWER ? 0x80 : 0xE0), (hex_type == USX_NIB_HEX_LOWER ? 2 : 4));
        ol = encodeCount(out, ol, hex_len);
        do {
          ol = append_bits(out, ol, getBaseCode(in[l++]), 4);
        } while (--hex_len);
        l--;
        continue;
      }
    }

    if (usx_templates != NULL) {
      int i;
      for (i = 0; i < 5; i++) {
        if (usx_templates[i]) {
          int rem = strlen(usx_templates[i]);
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
            ol = append_nibble_escape(out, ol, state, usx_hcodes, usx_hcode_lens);
            ol = append_bits(out, ol, 0, 1);
            ol = append_bits(out, ol, (count_codes[i] & 0xF8), count_codes[i] & 0x07);
            ol = encodeCount(out, ol, rem);
            for (int k = 0; k < j; k++) {
              char c_t = usx_templates[i][k];
              if (c_t == 'f' || c_t == 'F')
                ol = append_bits(out, ol, getBaseCode(in[l + k]), 4);
              else if (c_t == 'r' || c_t == 't' || c_t == 'o') {
                c_t = (c_t == 'r' ? 3 : (c_t == 't' ? 2 : 1));
                ol = append_bits(out, ol, (in[l + k] - '0') << (8 - c_t), c_t);
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

    if (usx_freq_seq != NULL) {
      int i;
      for (i = 0; i < 6; i++) {
        int seq_len = strlen(usx_freq_seq[i]);
        if (len - seq_len >= 0 && l <= len - seq_len) {
          if (memcmp(usx_freq_seq[i], in + l, seq_len) == 0 && usx_hcode_lens[usx_freq_codes[i] >> 5]) {
            ol = append_code(out, ol, usx_freq_codes[i], &state, usx_hcodes, usx_hcode_lens);
            l += seq_len;
            l--;
            break;
          }
        }
      }
      if (i < 6)
        continue;
    }

    c_in = in[l];

    is_upper = 0;
    if (c_in >= 'A' && c_in <= 'Z')
      is_upper = 1;
    else {
      if (is_all_upper) {
        is_all_upper = 0;
        ol = append_switch_code(out, ol, state);
        ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
        state = USX_ALPHA;
      }
    }
    if (is_upper && !is_all_upper) {
      if (state == USX_NUM) {
        ol = append_switch_code(out, ol, state);
        ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
        state = USX_ALPHA;
      }
      ol = append_switch_code(out, ol, state);
      ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
      if (state == USX_DELTA) {
        state = USX_ALPHA;
        ol = append_switch_code(out, ol, state);
        ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
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
          ol = append_switch_code(out, ol, state);
          ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
          state = USX_ALPHA;
          is_all_upper = 1;
        }
      }
      if (state == USX_DELTA && (c_in == ' ' || c_in == '.' || c_in == ',')) {
        byte spl_code = (c_in == ',' ? 0xC0 : (c_in == '.' ? 0xE0 : (c_in == ' ' ? 0 : 0xFF)));
        if (spl_code != 0xFF) {
          byte spl_code_len = (c_in == ',' ? 3 : (c_in == '.' ? 4 : (c_in == ' ' ? 1 : 4)));
          ol = append_bits(out, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN);
          ol = append_bits(out, ol, spl_code, spl_code_len);
          continue;
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
        ol = append_code(out, ol, usx_code_94[(uint8_t)c_in], &state, usx_hcodes, usx_hcode_lens);
      }
    } else
    if (c_in == 13 && c_next == 10) {
      ol = append_code(out, ol, CRLF_CODE, &state, usx_hcodes, usx_hcode_lens);
      l++;
    } else
    if (c_in == 10) {
      if (state == USX_DELTA) {
        ol = append_bits(out, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN);
        ol = append_bits(out, ol, 0xF0, 4);
      } else
        ol = append_code(out, ol, LF_CODE, &state, usx_hcodes, usx_hcode_lens);
    } else
    if (c_in == 13) {
      ol = append_code(out, ol, CR_CODE, &state, usx_hcodes, usx_hcode_lens);
    } else
    if (c_in == '\t') {
      ol = append_code(out, ol, TAB_CODE, &state, usx_hcodes, usx_hcode_lens);
    } else {
      int utf8len;
      int32_t uni = readUTF8(in, len, l, &utf8len);
      if (uni) {
        l += utf8len;
        if (state != USX_DELTA) {
          int32_t uni2 = readUTF8(in, len, l, &utf8len);
          if (uni2) {
            if (state != USX_ALPHA) {
              ol = append_switch_code(out, ol, state);
              ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
            }
            ol = append_switch_code(out, ol, state);
            ol = append_bits(out, ol, usx_hcodes[USX_ALPHA], usx_hcode_lens[USX_ALPHA]);
            ol = append_bits(out, ol, usx_vcodes[1], usx_vcode_lens[1]); // code for space (' ')
            state = USX_DELTA;
          } else {
            ol = append_switch_code(out, ol, state);
            ol = append_bits(out, ol, usx_hcodes[USX_DELTA], usx_hcode_lens[USX_DELTA]);
          }
        }
        ol = encodeUnicode(out, ol, uni, prev_uni);
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
        ol = append_nibble_escape(out, ol, state, usx_hcodes, usx_hcode_lens);
        ol = append_bits(out, ol, 0xF8, 5);
        ol = encodeCount(out, ol, bin_count);
        do {
          ol = append_bits(out, ol, in[l++], 8);
        } while (--bin_count);
        l--;
      }
    }
  }
  int ret = ol/8+(ol%8?1:0);
  if (ol % 8) {
    if (state == USX_DELTA)
      ol = append_bits(out, ol, UNI_STATE_SPL_CODE, UNI_STATE_SPL_CODE_LEN);
    ol = append_code(out, ol, TERM_CODE, &state, usx_hcodes, usx_hcode_lens);
  }
  //printf("\n%ld\n", ol);
  return ret;

}

int unishox2_compress(const char *in, int len, char *out, const byte usx_hcodes[], const byte usx_hcode_lens[], const char *usx_freq_seq[], const char *usx_templates[]) {
  return unishox2_compress_lines(in, len, out, usx_hcodes, usx_hcode_lens, usx_freq_seq, usx_templates, NULL);
}

int unishox2_compress_simple(const char *in, int len, char *out) {
  return unishox2_compress_lines(in, len, out, USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_DFLT, USX_TEMPLATES, NULL);
}

int readBit(const char *in, int bit_no) {
   return in[bit_no >> 3] & (0x80 >> (bit_no % 8));
}

int read8bitCode(const char *in, int len, int *bit_no_p) {
  int bit_pos = *bit_no_p & 0x07;
  int char_pos = *bit_no_p >> 3;
  byte code = (((byte)in[char_pos]) << bit_pos);
  if (((*bit_no_p) + bit_pos) < len) {
    code |= ((byte)in[++char_pos]) >> (8 - bit_pos);
  } else
    code |= (0xFF >> (8 - bit_pos));
  return code;
}

// Decoder is designed for using less memory, not speed
#define SECTION_COUNT 5
const byte usx_vsections[] = {0x7F, 0xBF, 0xDF, 0xEF, 0xFF};
const byte usx_vsection_pos[] = {0, 4, 8, 12, 20};
const byte usx_vsection_mask[] = {0x7F, 0x3F, 0x1F, 0x0F, 0x0F};
const byte usx_vsection_shift[] = {5, 4, 3, 1, 0};

// Vertical decoder lookup table - 3 bits code len, 5 bytes vertical pos
// code len is one less as 8 cannot be accommodated in 3 bits
const byte usx_vcode_lookup[36] = {
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
  if (*bit_no_p < len) {
    byte code = read8bitCode(in, len, bit_no_p);
    int i = 0;
    do {
      if (code <= usx_vsections[i]) {
        byte vcode = usx_vcode_lookup[usx_vsection_pos[i] + ((code & usx_vsection_mask[i]) >> usx_vsection_shift[i])];
        (*bit_no_p) += ((vcode >> 5) + 1);
        if (*bit_no_p > len)
          return 99;
        return vcode & 0x1F;
      }
    } while (++i < SECTION_COUNT);
  }
  return 99;
}

byte len_masks[] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
int readHCodeIdx(const char *in, int len, int *bit_no_p, const byte usx_hcodes[], const byte usx_hcode_lens[]) {
  if (!usx_hcode_lens[USX_ALPHA])
    return USX_ALPHA;
  if (*bit_no_p < len) {
    byte code = read8bitCode(in, len, bit_no_p);
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
int getStepCodeIdx(const char *in, int len, int *bit_no_p, int limit) {
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

// TODO: Check length
int32_t getNumFromBits(const char *in, int len, int bit_no, int count) {
   int32_t ret = 0;
   while (count-- && bit_no < len) {
     ret += (readBit(in, bit_no) ? 1 << count : 0);
     bit_no++;
   }
   return ret;
}

int readCount(const char *in, int *bit_no_p, int len) {
  int idx = getStepCodeIdx(in, len, bit_no_p, 4);
  if (idx == 99)
    return -1;
  if (*bit_no_p + count_bit_lens[idx] - 1 >= len)
    return -1;
  int count = getNumFromBits(in, len, *bit_no_p, count_bit_lens[idx]) + (idx ? count_adder[idx - 1] : 0);
  (*bit_no_p) += count_bit_lens[idx];
  return count;
}

int32_t readUnicode(const char *in, int *bit_no_p, int len) {
  int idx = getStepCodeIdx(in, len, bit_no_p, 5);
  if (idx == 99)
    return 0x7FFFFF00 + 99;
  if (idx == 5) {
    int idx = getStepCodeIdx(in, len, bit_no_p, 4);
    return 0x7FFFFF00 + idx;
  }
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

void writeUTF8(char *out, int *ol, int uni) {
  if (uni < (1 << 11)) {
    out[(*ol)++] = (0xC0 + (uni >> 6));
    out[(*ol)++] = (0x80 + (uni & 0x3F));
  } else
  if (uni < (1 << 16)) {
    out[(*ol)++] = (0xE0 + (uni >> 12));
    out[(*ol)++] = (0x80 + ((uni >> 6) & 0x3F));
    out[(*ol)++] = (0x80 + (uni & 0x3F));
  } else {
    out[(*ol)++] = (0xF0 + (uni >> 18));
    out[(*ol)++] = (0x80 + ((uni >> 12) & 0x3F));
    out[(*ol)++] = (0x80 + ((uni >> 6) & 0x3F));
    out[(*ol)++] = (0x80 + (uni & 0x3F));
  }
}

int decodeRepeat(const char *in, int len, char *out, int ol, int *bit_no, struct us_lnk_lst *prev_lines) {
  if (prev_lines) {
    int dict_len = readCount(in, bit_no, len) + NICE_LEN;
    if (dict_len < 0)
      return ol;
    int dist = readCount(in, bit_no, len);
    if (dist < 0)
      return ol;
    int ctx = readCount(in, bit_no, len);
    if (ctx < 0)
      return ol;
    struct us_lnk_lst *cur_line = prev_lines;
    while (ctx--)
      cur_line = cur_line->previous;
    memmove(out + ol, cur_line->data + dist, dict_len);
    ol += dict_len;
  } else {
    int dict_len = readCount(in, bit_no, len) + NICE_LEN;
    if (dict_len < 0)
      return ol;
    int dist = readCount(in, bit_no, len) + NICE_LEN - 1;
    if (dist < 0)
      return ol;
    //printf("Decode len: %d, dist: %d\n", dict_len - NICE_LEN, dist - NICE_LEN + 1);
    memcpy(out + ol, out + ol - dist, dict_len);
    ol += dict_len;
  }
  return ol;
}

char getHexChar(int nibble, int hex_type) {
  if (nibble >= 0 && nibble <= 9)
    return '0' + nibble;
  else if (hex_type < 3)
    return 'a' + nibble - 10;
  return 'A' + nibble - 10;
}

int unishox2_decompress_lines(const char *in, int len, char *out, const byte usx_hcodes[], const byte usx_hcode_lens[], const char *usx_freq_seq[], const char *usx_templates[], struct us_lnk_lst *prev_lines) {

  int dstate;
  int bit_no;
  int h, v;
  byte is_all_upper;

  init_coder();
  int ol = 0;
  bit_no = 1; // ignore the magic bit
  dstate = h = USX_ALPHA;
  is_all_upper = 0;

  int prev_uni = 0;

  len <<= 3;
  out[ol] = 0;
  while (bit_no < len) {
    int orig_bit_no = bit_no;
    if (dstate == USX_DELTA || h == USX_DELTA) {
      if (dstate != USX_DELTA)
        h = dstate;
      int32_t delta = readUnicode(in, &bit_no, len);
      if ((delta >> 8) == 0x7FFFFF) {
        int spl_code_idx = delta & 0x000000FF;
        if (spl_code_idx == 99)
          break;
        switch (spl_code_idx) {
          case 0:
            out[ol++] = ' ';
            continue;
          case 1:
            h = readHCodeIdx(in, len, &bit_no, usx_hcodes, usx_hcode_lens);
            if (h == 99) {
              bit_no = len;
              continue;
            }
            if (h == USX_DELTA || h == USX_ALPHA) {
              dstate = h;
              continue;
            }
            if (h == USX_DICT) {
              ol = decodeRepeat(in, len, out, ol, &bit_no, prev_lines);
              h = dstate;
              continue;
            }
            break;
          case 2:
            out[ol++] = ',';
            continue;
          case 3:
            out[ol++] = '.';
            continue;
          case 4:
            out[ol++] = 10;
            continue;
        }
      } else {
        prev_uni += delta;
        writeUTF8(out, &ol, prev_uni);
        //printf("%ld, ", prev_uni);
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
    if (v == 0 && h != USX_SYM) {
      if (bit_no >= len)
        break;
      if (h != USX_NUM || dstate != USX_DELTA) {
        h = readHCodeIdx(in, len, &bit_no, usx_hcodes, usx_hcode_lens);
        if (h == 99 || bit_no >= len) {
          bit_no = orig_bit_no;
          break;
        }
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
              h = readHCodeIdx(in, len, &bit_no, usx_hcodes, usx_hcode_lens);
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
      } else {
        if (h != USX_NUM || dstate != USX_DELTA)
          v = readVCodeIdx(in, len, &bit_no);
        if (v == 99) {
          bit_no = orig_bit_no;
          break;
        }
        if (h == USX_NUM && v == 0) {
          int idx = getStepCodeIdx(in, len, &bit_no, 5);
          if (idx == 0) {
            idx = getStepCodeIdx(in, len, &bit_no, 4);
            int rem = readCount(in, &bit_no, len);
            if (rem < 0)
              break;
            rem = strlen(usx_templates[idx]) - rem;
            for (int j = 0; j < rem; j++) {
              char c_t = usx_templates[idx][j];
              if (c_t == 'f' || c_t == 'r' || c_t == 't' || c_t == 'o' || c_t == 'F') {
                  char nibble_len = (c_t == 'f' || c_t == 'F' ? 4 : (c_t == 'r' ? 3 : (c_t == 't' ? 2 : 1)));
                  out[ol++] = getHexChar(getNumFromBits(in, len, bit_no, nibble_len),
                      c_t == 'f' ? USX_NIB_HEX_LOWER : USX_NIB_HEX_UPPER);
                  bit_no += nibble_len;
              } else
                out[ol++] = c_t;
            }
          } else
          if (idx == 5) {
            int bin_count = readCount(in, &bit_no, len);
            if (bin_count < 0)
              break;
            do {
              out[ol++] = getNumFromBits(in, len, bit_no, 8);
              bit_no += 8;
            } while (--bin_count);
          } else {
            int nibble_count = 0;
            if (idx == 2 || idx == 4)
              nibble_count = 32;
            else {
              nibble_count = readCount(in, &bit_no, len);
              if (nibble_count < 0)
                break;
            }
            do {
              int nibble = (int) getNumFromBits(in, len, bit_no, 4);
              out[ol++] = getHexChar(nibble, idx);
              if ((idx == 2 || idx == 4) && (nibble_count == 25 || nibble_count == 21 || nibble_count == 17 || nibble_count == 13))
                out[ol++] = '-';
              bit_no += 4;
            } while (--nibble_count);
          }
          if (dstate == USX_DELTA)
            h = USX_DELTA;
          continue;
        }
      }
    }
    if (is_upper && v == 1) {
      h = dstate = USX_DELTA; // continuous delta coding
      continue;
    }
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
          out[ol++] = '\r';
          out[ol++] = '\n';
        } else if (h == USX_NUM && v == 26) {
          int count = readCount(in, &bit_no, len);
          if (count < 0)
            break;
          count += 4;
          char rpt_c = out[ol - 1];
          while (count--)
            out[ol++] = rpt_c;
        } else if (h == USX_SYM && v > 24) {
          v -= 25;
          memcpy(out + ol, usx_freq_seq[v], strlen(usx_freq_seq[v]));
          ol += strlen(usx_freq_seq[v]);
        } else if (h == USX_NUM && v > 22 && v < 26) {
          v -= (23 - 3);
          memcpy(out + ol, usx_freq_seq[v], strlen(usx_freq_seq[v]));
          ol += strlen(usx_freq_seq[v]);
        } else
          break; // Terminator
        if (dstate == USX_DELTA)
          h = USX_DELTA;
        continue;
      }
    }
    if (dstate == USX_DELTA)
      h = USX_DELTA;
    out[ol++] = c;
  }

  return ol;

}

int unishox2_decompress(const char *in, int len, char *out, const byte usx_hcodes[], const byte usx_hcode_lens[], const char *usx_freq_seq[], const char *usx_templates[]) {
  return unishox2_decompress_lines(in, len, out, usx_hcodes, usx_hcode_lens, usx_freq_seq, usx_templates, NULL);
}

int unishox2_decompress_simple(const char *in, int len, char *out) {
  return unishox2_decompress(in, len, out, USX_PSET_DFLT);
}
