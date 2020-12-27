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
#ifndef unishox2
#define unishox2

#define UNISHOX_VERSION "2.0"

#define USX_PSET_ALPHA_ONLY 1
#define USX_PSET_ALPHA_NUM_ONLY 2
#define USX_PSET_ALPHA_NUM_SYM_ONLY 3
#define USX_PSET_FAVOUR_ALPHA 4
#define USX_PSET_FAVOUR_NUM 5
#define USX_PSET_FAVOUR_SYM 6
#define USX_PSET_FAVOUR_UMLAUT 7
#define USX_PSET_NO_DICT 8
#define USX_PSET_NO_UNI 9
#define USX_PSET_URL 10

//enum {USX_ALPHA = 0, USX_SYM, USX_NUM, USX_DICT, USX_DELTA};

#define USX_HCODES_DFLT {0x00, 0x40, 0xE0, 0x80, 0xC0}
#define USX_HCODE_LENS_DFLT {2, 2, 3, 2, 3}

#define USX_HCODES_ALPHA_ONLY {0x00, 0x00, 0x00, 0x00, 0x00}
#define USX_HCODE_LENS_ALPHA_ONLY {0, 0, 0, 0, 0}

#define USX_HCODES_ALPHA_NUM_ONLY {0x00, 0x00, 0x80, 0x00, 0x00}
#define USX_HCODE_LENS_ALPHA_NUM_ONLY {1, 0, 1, 0, 0}

#define USX_HCODES_ALPHA_NUM_SYM_ONLY {0x00, 0x80, 0xC0, 0x00, 0x00}
#define USX_HCODE_LENS_ALPHA_NUM_SYM_ONLY {1, 2, 2, 0, 0}

#define USX_HCODES_FAVOUR_ALPHA {0x00, 0x80, 0xA0, 0xC0, 0xE0}
#define USX_HCODE_LENS_FAVOUR_ALPHA {1, 3, 3, 3, 3}

#define USX_HCODES_FAVOUR_NUM {0x80, 0xA0, 0xC0, 0xE0, 0x00}
#define USX_HCODE_LENS_FAVOUR_NUM {3, 3, 3, 3, 1}

#define USX_HCODES_FAVOUR_SYM {0x80, 0x00, 0xA0, 0xC0, 0xE0}
#define USX_HCODE_LENS_FAVOUR_SYM {3, 1, 3, 3, 3}

//#define USX_HCODES_FAVOUR_UMLAUT {0x00, 0x40, 0xE0, 0xC0, 0x80}
//#define USX_HCODE_LENS_FAVOUR_UMLAUT {2, 2, 3, 3, 2}

#define USX_HCODES_FAVOUR_UMLAUT {0x80,  0xA0, 0xC0, 0xE0, 0x00}
#define USX_HCODE_LENS_FAVOUR_UMLAUT {3, 3, 3, 3, 1}

#define USX_HCODES_NO_DICT {0x00, 0x40, 0x00, 0x80, 0xC0}
#define USX_HCODE_LENS_NO_DICT {2, 2, 0, 2, 2}

#define USX_HCODES_NO_UNI {0x00, 0x40, 0x80, 0x00, 0xC0}
#define USX_HCODE_LENS_NO_UNI {2, 2, 2, 0, 2}

#define USX_FREQ_SEQ_DFLT {"\": \"", "\": ", "</", "=\"", "\":\"", "://"}
#define USX_FREQ_SEQ_TXT {" the ", " and ", "tion", " with", "ing", "ment"}
#define USX_FREQ_SEQ_URL {"https://", "www.", ".com", "http://", ".org", ".net"}

struct us_lnk_lst {
  char *data;
  struct us_lnk_lst *previous;
};

extern int unishox2_compress_simple(const char *in, int len, char *out);
extern int unishox2_decompress_simple(const char *in, int len, char *out);
extern int unishox2_compress_preset(const char *in, int len, char *out, int preset, struct us_lnk_lst *prev_lines);
extern int unishox2_decompress_preset(const char *in, int len, char *out, int preset, struct us_lnk_lst *prev_lines);
extern int unishox2_compress(const char *in, int len, char *out, 
              const unsigned char usx_hcodes[], const unsigned char usx_hcode_lens[], 
              const char *usx_freq_seq[], struct us_lnk_lst *prev_lines);
extern int unishox2_decompress(const char *in, int len, char *out, 
              const unsigned char usx_hcodes[], const unsigned char usx_hcode_lens[],
              const char *usx_freq_seq[], struct us_lnk_lst *prev_lines);

#endif
