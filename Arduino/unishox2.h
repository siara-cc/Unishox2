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
 *
 */
#ifndef unishox2
#define unishox2

#define UNISHOX_VERSION "2.0"

//enum {USX_ALPHA = 0, USX_SYM, USX_NUM, USX_DICT, USX_DELTA};

#define USX_HCODES_DFLT (const unsigned char[]){0x00, 0x40, 0x80, 0xC0, 0xE0}
#define USX_HCODE_LENS_DFLT (const unsigned char[]){2, 2, 2, 3, 3}

#define USX_HCODES_ALPHA_ONLY (const unsigned char[]){0x00, 0x00, 0x00, 0x00, 0x00}
#define USX_HCODE_LENS_ALPHA_ONLY (const unsigned char[]){0, 0, 0, 0, 0}

#define USX_HCODES_ALPHA_NUM_ONLY (const unsigned char[]){0x00, 0x00, 0x80, 0x00, 0x00}
#define USX_HCODE_LENS_ALPHA_NUM_ONLY (const unsigned char[]){1, 0, 1, 0, 0}

#define USX_HCODES_ALPHA_NUM_SYM_ONLY (const unsigned char[]){0x00, 0x80, 0xC0, 0x00, 0x00}
#define USX_HCODE_LENS_ALPHA_NUM_SYM_ONLY (const unsigned char[]){1, 2, 2, 0, 0}

#define USX_HCODES_FAVOR_ALPHA (const unsigned char[]){0x00, 0x80, 0xA0, 0xC0, 0xE0}
#define USX_HCODE_LENS_FAVOR_ALPHA (const unsigned char[]){1, 3, 3, 3, 3}

#define USX_HCODES_FAVOR_DICT (const unsigned char[]){0x00, 0x40, 0xC0, 0x80, 0xE0}
#define USX_HCODE_LENS_FAVOR_DICT (const unsigned char[]){2, 2, 3, 2, 3}

#define USX_HCODES_FAVOR_SYM (const unsigned char[]){0x80, 0x00, 0xA0, 0xC0, 0xE0}
#define USX_HCODE_LENS_FAVOR_SYM (const unsigned char[]){3, 1, 3, 3, 3}

//#define USX_HCODES_FAVOR_UMLAUT {0x00, 0x40, 0xE0, 0xC0, 0x80}
//#define USX_HCODE_LENS_FAVOR_UMLAUT {2, 2, 3, 3, 2}

#define USX_HCODES_FAVOR_UMLAUT (const unsigned char[]){0x80, 0xA0, 0xC0, 0xE0, 0x00}
#define USX_HCODE_LENS_FAVOR_UMLAUT (const unsigned char[]){3, 3, 3, 3, 1}

#define USX_HCODES_NO_DICT (const unsigned char[]){0x00, 0x40, 0x80, 0x00, 0xC0}
#define USX_HCODE_LENS_NO_DICT (const unsigned char[]){2, 2, 2, 0, 2}

#define USX_HCODES_NO_UNI (const unsigned char[]){0x00, 0x40, 0x80, 0xC0, 0x00}
#define USX_HCODE_LENS_NO_UNI (const unsigned char[]){2, 2, 2, 2, 0}
/*
#define USX_FREQ_SEQ_DFLT (const char *[]){"\": \"", "\": ", "</", "=\"", "\":\"", "://"}
#define USX_FREQ_SEQ_TXT (const char *[]){" the ", " and ", "tion", " with", "ing", "ment"}
#define USX_FREQ_SEQ_URL (const char *[]){"https://", "www.", ".com", "http://", ".org", ".net"}
#define USX_FREQ_SEQ_JSON (const char *[]){"\": \"", "\": ", "\",", "}}}", "\":\"", "}}"}
#define USX_FREQ_SEQ_HTML (const char *[]){"</", "=\"", "div", "href", "class", "<p>"}
#define USX_FREQ_SEQ_XML (const char *[]){"</", "=\"", "\">", "<?xml version=\"1.0\"", "xmlns:", "://"}
#define USX_TEMPLATES (const char *[]){"tfff-of-tfTtf:rf:rf.fffZ", "tfff-of-tf", "(fff) fff-ffff", "tf:rf:rf", 0}
*/
extern const char * USX_FREQ_SEQ_DFLT[];
extern const char * USX_FREQ_SEQ_TXT[];
extern const char * USX_FREQ_SEQ_URL[];
extern const char * USX_FREQ_SEQ_JSON[];
extern const char * USX_FREQ_SEQ_HTML[];
extern const char * USX_FREQ_SEQ_XML[];
extern const char * USX_TEMPLATES[];

#define USX_PSET_DFLT USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_ALPHA_ONLY USX_HCODES_ALPHA_ONLY, USX_HCODE_LENS_ALPHA_ONLY, USX_FREQ_SEQ_TXT, USX_TEMPLATES
#define USX_PSET_ALPHA_NUM_ONLY USX_HCODES_ALPHA_NUM_ONLY, USX_HCODE_LENS_ALPHA_NUM_ONLY, USX_FREQ_SEQ_TXT, USX_TEMPLATES
#define USX_PSET_ALPHA_NUM_SYM_ONLY USX_HCODES_ALPHA_NUM_SYM_ONLY, USX_HCODE_LENS_ALPHA_NUM_SYM_ONLY, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_ALPHA_NUM_SYM_ONLY_TXT USX_HCODES_ALPHA_NUM_SYM_ONLY, USX_HCODE_LENS_ALPHA_NUM_SYM_ONLY, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_FAVOR_ALPHA USX_HCODES_FAVOR_ALPHA, USX_HCODE_LENS_FAVOR_ALPHA, USX_FREQ_SEQ_TXT, USX_TEMPLATES
#define USX_PSET_FAVOR_DICT USX_HCODES_FAVOR_DICT, USX_HCODE_LENS_FAVOR_DICT, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_FAVOR_SYM USX_HCODES_FAVOR_SYM, USX_HCODE_LENS_FAVOR_SYM, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_FAVOR_UMLAUT USX_HCODES_FAVOR_UMLAUT, USX_HCODE_LENS_FAVOR_UMLAUT, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_NO_DICT USX_HCODES_NO_DICT, USX_HCODE_LENS_NO_DICT, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_NO_UNI USX_HCODES_NO_UNI, USX_HCODE_LENS_NO_UNI, USX_FREQ_SEQ_DFLT, USX_TEMPLATES
#define USX_PSET_NO_UNI_FAVOR_TEXT USX_HCODES_NO_UNI, USX_HCODE_LENS_NO_UNI, USX_FREQ_SEQ_TXT, USX_TEMPLATES
#define USX_PSET_URL USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_URL, USX_TEMPLATES
#define USX_PSET_JSON USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_JSON, USX_TEMPLATES
#define USX_PSET_JSON_NO_UNI USX_HCODES_NO_UNI, USX_HCODE_LENS_NO_UNI, USX_FREQ_SEQ_JSON, USX_TEMPLATES
#define USX_PSET_XML USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_XML, USX_TEMPLATES
#define USX_PSET_HTML USX_HCODES_DFLT, USX_HCODE_LENS_DFLT, USX_FREQ_SEQ_HTML, USX_TEMPLATES

struct us_lnk_lst {
  char *data;
  struct us_lnk_lst *previous;
};

extern int unishox2_compress_simple(const char *in, int len, char *out);
extern int unishox2_decompress_simple(const char *in, int len, char *out);
extern int unishox2_compress(const char *in, int len, char *out, 
              const unsigned char usx_hcodes[], const unsigned char usx_hcode_lens[], 
              const char *usx_freq_seq[], const char *usx_templates[]);
extern int unishox2_decompress(const char *in, int len, char *out, 
              const unsigned char usx_hcodes[], const unsigned char usx_hcode_lens[],
              const char *usx_freq_seq[], const char *usx_templates[]);
extern int unishox2_compress_lines(const char *in, int len, char *out, 
              const unsigned char usx_hcodes[], const unsigned char usx_hcode_lens[], 
              const char *usx_freq_seq[], const char *usx_templates[],
              struct us_lnk_lst *prev_lines);
extern int unishox2_decompress_lines(const char *in, int len, char *out, 
              const unsigned char usx_hcodes[], const unsigned char usx_hcode_lens[],
              const char *usx_freq_seq[], const char *usx_templates[],
              struct us_lnk_lst *prev_lines);

#endif
