/*
 * Copyright (C) 2022 Siara Logics (cc)
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
 * @file unishox3.h
 * @author Arundale Ramanathan, James Z. M. Gao
 * @brief API for Unishox3 Compression and Decompression
 *
 * This file describes each function of the Unishox3 API \n
 * For finding out how this API can be used in your program, \n
 * please see test_unishox3.c.
 */

#ifndef unishox3_def
#define unishox3_def

/**
 * Macro switch to enable/disable output buffer length parameter in low level api \n
 * Disabled by default \n
 * When this macro is defined, the all the API functions \n
 * except the simple API functions accept an additional parameter olen \n
 * that enables the developer to pass the size of the output buffer provided \n
 * so that the api function may not write beyond that length. \n
 * This can be disabled if the developer knows that the buffer provided is sufficient enough \n
 * so no additional parameter is passed and the program is faster since additional check \n
 * for output length is not performed at each step \n
 * The simple api, i.e. unishox3_(de)compress_simple will always omit the buffer length
 */
#ifndef USX3_API_WITH_OUTPUT_LEN
#  define USX3_API_WITH_OUTPUT_LEN 0
#endif

/// Upto 8 bits of initial magic bit sequence can be included. Bit count can be specified with USX3_MAGIC_BIT_LEN
#ifndef USX3_MAGIC_BITS
#  define USX3_MAGIC_BITS 0xFF
#endif

/// Desired length of Magic bits defined by USX3_MAGIC_BITS
#ifdef USX3_MAGIC_BIT_LEN
#  if USX3_MAGIC_BIT_LEN < 0 || 9 <= USX3_MAGIC_BIT_LEN
#    error "USX3_MAGIC_BIT_LEN need between [0, 8)"
#  endif
#else
#  define USX3_MAGIC_BIT_LEN 1
#endif

/**
 * This macro is for internal use, but builds upon the macro USX3_API_WITH_OUTPUT_LEN
 * When the macro USX3_API_WITH_OUTPUT_LEN is defined, the all the API functions
 * except the simple API functions accept an additional parameter olen
 * that enables the developer to pass the size of the output buffer provided
 * so that the api function may not write beyond that length.
 * This can be disabled if the developer knows that the buffer provided is sufficient enough
 * so no additional parameter is passed and the program is faster since additional check
 * for output length is not performed at each step
 */
#if defined(USX3_API_WITH_OUTPUT_LEN) && USX3_API_WITH_OUTPUT_LEN != 0
#  define USX3_API_OUT_AND_LEN(out, olen) out, olen
#else
#  define USX3_API_OUT_AND_LEN(out, olen) out
#endif

/// Commonly occuring templates (ISO Date/Time, ISO Date, US Phone number, ISO Time, Unused)
#define USX_TEMPLATES (const char *[]) {"tfff-of-tfTtf:rf:rf.fffZ", "tfff-of-tf", "(fff) fff-ffff", "tf:rf:rf", NULL}

#include <stdint.h>
#include <string.h>

extern const char **wordlist[];

/// Minimum length to consider as repeating sequence
#define NICE_LEN 7

/// Return value of function that matches repeating sequences
class usx3_longest {
  public:
    int len;
    int dist;
    usx3_longest(int l = -1, int d = -1) {
      len = l;
      dist = d;
    }
    bool is_found() {
      return (len >= 0);
    }
    int saving() {
      if (len < 0)
        return 0;
      return len + NICE_LEN;
    }
};

/// Return value of function that matches from internal dictionaries
class usx3_dict_find {
  public:
    int lvl;
    int pos;
    usx3_dict_find(int l = -1, int p = -1) {
      lvl = l;
      pos = p;
    }
    bool is_found() {
      return (pos >= 0);
    }
    int saving() {
      if (pos < 0)
        return 0;
      return strlen(wordlist[lvl][pos]);
    }
};

/** 
 * Class definition for compressing and decompressing a string
 */
class unishox3 {

  protected:

    /// Horizontal codes used by the instance
    uint8_t usx_hcodes[6];
    /// Length of each Horizontal code
    uint8_t usx_hcode_lens[6];

    /// This 2D array has the characters for the sets USX_ALPHA, USX_SYM and USX_NUM. Where a character cannot fit into a uint8_t, 0 is used and handled in code.
    uint8_t usx_sets[3][28];

    /// Stores position of letter in usx_sets.
    /// First 3 bits - position in usx_hcodes
    /// Next  5 bits - position in usx_vcodes
    uint8_t usx_code_94[94];

    const char *usx_templates[5];

    int append_code(char *out, int olen, int ol, uint8_t code, uint8_t *state);
    int append_switch_code(char *out, int olen, int ol, uint8_t state);
    int switch_to(char *out, int olen, int ol, uint8_t state, int hcode);

    /// Starts coding of nibble sets
    int append_nibble_escape(char *out, int olen, int ol, uint8_t state);

    /// Appends the terminator code depending on the state, preset and whether full terminator needs to be encoded to out or not \n
    int append_final_bits(char *const out, const int olen, int ol, const uint8_t state, const uint8_t is_all_upper);

    usx3_dict_find match_predef_dict(const char *in, int len, int l);

    /// Finds the longest matching sequence from the beginning of the string. \n
    /// If a match is found and it is longer than NICE_LEN, it is encoded as a repeating sequence to out \n
    /// This is also used for Unicode strings \n
    /// This is a crude implementation that is not optimized.  Assuming only short strings \n
    /// are encoded, this is not much of an issue.
    usx3_longest matchOccurance(const char *in, int len, int l);

    int encode_dict_matches(const char *in, int len, int l, char *out, int olen, int *ol, uint8_t *state, uint8_t *is_all_upper);

    int readVCodeIdx(const char *in, int len, int *bit_no_p);
    int readHCodeIdx(const char *in, int len, int *bit_no_p);
    int readLvlIdx(const char *in, int len, int *bit_no_p);
    int decodeRepeat(const char *in, int len, char *out, int olen, int ol, int *bit_no);

  public:

    unishox3();

    /** 
     * Simple API for compressing a string
     * @param[in] in    Input ASCII / UTF-8 string
     * @param[in] len   length in bytes
     * @param[out] out  output buffer - should be large enough to hold compressed output
     */
    int compress_simple(const char *in, int len, char *out);

    /** 
     * Simple API for decompressing a string
     * @param[in] in    Input compressed bytes (output of unishox3_compress functions)
     * @param[in] len   length of 'in' in bytes
     * @param[out] out  output buffer for ASCII / UTF-8 string - should be large enough
     */
    int decompress_simple(const char *in, int len, char *out);

    /** 
     * Comprehensive API for compressing a string
     * 
     * Presets are available for the last four parameters so they can be passed as single parameter. \n
     * See USX3_PSET_* macros. Example call: \n
     *    unishox3_compress(in, len, out, olen, USX3_PSET_ALPHA_ONLY);
     * 
     * @param[in] in             Input ASCII / UTF-8 string
     * @param[in] len            length in bytes
     * @param[out] out           output buffer - should be large enough to hold compressed output
     * @param[in] olen           length of 'out' buffer in bytes. Can be omitted if sufficient buffer is provided
     * @param[in] usx_hcodes     Horizontal codes (array of bytes). See macro section for samples.
     * @param[in] usx_templates  Templates of frequently occuring patterns. See USX3_TEMPLATES macro.
     */
    int compress(const char *in, int len, USX3_API_OUT_AND_LEN(char *out, int olen));

    /** 
     * Comprehensive API for de-compressing a string
     * 
     * Presets are available for the last four parameters so they can be passed as single parameter. \n
     * See USX3_PSET_* macros. Example call: \n
     *    unishox3_decompress(in, len, out, olen, USX3_PSET_ALPHA_ONLY);
     * 
     * @param[in] in             Input compressed bytes (output of unishox3_compress functions)
     * @param[in] len            length of 'in' in bytes
     * @param[out] out           output buffer - should be large enough to hold de-compressed output
     * @param[in] olen           length of 'out' buffer in bytes. Can be omitted if sufficient buffer is provided
     */
    int decompress(const char *in, int len, USX3_API_OUT_AND_LEN(char *out, int olen));

    void setTemplates(const char *templates[]);

    void setHCodess(uint8_t hcodes[], uint8_t hcode_lens[]);

};

#endif
