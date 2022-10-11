# Unishox: A hybrid encoder for Short Unicode Strings

[![C/C++ CI](https://github.com/siara-cc/Unishox/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/siara-cc/Unishox/actions/workflows/c-cpp.yml) [![DOI](https://joss.theoj.org/papers/10.21105/joss.03919/status.svg)](https://doi.org/10.21105/joss.03919)
[![npm ver](https://img.shields.io/npm/v/unishox2.siara.cc)](https://www.npmjs.com/package/unishox2.siara.cc)
[![afl](https://img.shields.io/badge/afl%20crashes%2Fhangs-0-green)](https://github.com/siara-cc/Unishox2/tree/master/afl_fuzz)

In general compression utilities such as `zip`, `gzip` do not compress short strings well and often expand them. They also use lots of memory which makes them unusable in constrained environments like Arduino.  So Unishox algorithm was developed for individually compressing (and decompressing) short strings.

This is a C/C++ library.  See [here for CPython version](https://github.com/tweedge/unishox2-py3) and [here for Javascript version](https://github.com/siara-cc/Unishox_JS) which is interoperable with this library.

# Applications

- Faster transfer of text over low-speed networks such as LORA or BLE
- Compression for low memory devices such as Arduino and ESP8266
- Compression of Chat application text exchange including Emojis
- Storing compressed text in database
- Bandwidth and storage cost reduction for Cloud

![Promo picture](https://github.com/siara-cc/Unishox2/blob/master/promo/Banner1.png?raw=true)

# How it works

Unishox is an hybrid encoder (entropy, dictionary and delta coding).  It works by assigning fixed prefix-free codes for each letter in the above Character Set (entropy coding).  It also encodes repeating letter sets separately (dictionary coding).  For Unicode characters, delta coding is used.

The model used for arriving at the prefix-free code is shown below:

![Promo picture](https://github.com/siara-cc/Unishox2/blob/master/promo/model.png?raw=true)

The complete specification can be found in this article: [A hybrid encoder for compressing Short Unicode Strings](https://github.com/siara-cc/Unishox2/blob/master/Unishox_Article_2.pdf?raw=true). This can also be found at `figshare` [here](https://figshare.com/articles/preprint/Unishox_A_hybrid_encoder_for_Short_Unicode_Strings/17056334) with DOI `10.6084/m9.figshare.17056334.v2`.

# Compiling

To compile, just use `make` or use gcc as follows:

```sh
gcc -std=c99 -o unishox2 test_unishox2.c unishox2.c
```

# Unit tests (automated)

For testing the compiled program, use:

```sh
./test_unishox2 -t
```

This invokes `run_unit_tests()` function of `test_unishox2.c`, which tests all the features of Unishox2, including edge cases, using 159 strings covering several languages, emojis and binary data.

Further, the CI pipeline at `.github/workflows/c-cpp.yml` runs these tests for all presets and also tests file compression for the different types of files in `sample_texts` folder.  This happens whenever a commit is made to the repository.

# API

```C
int unishox2_compress_simple(const char *in, int len, char *out);
int unishox2_decompress_simple(const char *in, int len, char *out);
```

# Usage

To see Unishox in action, simply try to compress a string:

```
./test_unishox2 "Hello World"
```

To compress and decompress a file, use:

```
./test_unishox2 -c <input_file> <compressed_file>
./test_unishox2 -d <compressed_file> <decompressed_file>
```

Note: Unishox is good for text content upto few kilobytes. Unishox does not give good ratios compressing large files or compressing binary files.

# Character Set

Unishox supports the entire Unicode character set.  As of now it supports UTF-8 as input and output encoding.

# Interoperability with the JS Library

Strings that were compressed with this library can be decompressed with the [JS Library](https://github.com/siara-cc/Unishox_JS) and vice-versa.  However please see [this section in the documentation](https://github.com/siara-cc/Unishox_JS#interoperability-with-the-c-library) for usage.

# Projects that use Unishox

- [Unishox Compression Library for Arduino Progmem](https://github.com/siara-cc/Unishox_Arduino_Progmem_lib)
- [Sqlite3 User Defined Function as loadable extension](https://github.com/siara-cc/Unishox_Sqlite_UDF)
- [Sqlite3 Library for ESP32](https://github.com/siara-cc/esp32_arduino_sqlite3_lib)
- [Sqlite3 Library for ESP8266](https://github.com/siara-cc/esp_arduino_sqlite3_lib)
- [Port of Unishox 1 to Python and C++ by Stephan Hadinger for Tasmota](https://github.com/arendst/Tasmota/tree/development/tools/unishox)
- [Python bindings for Unishox2](https://github.com/tweedge/unishox2-py3)
- [Unishox2 Javascript library](https://github.com/siara-cc/unishox_js)
- [Unishox2 proposed to be used in Meshtastic project](https://github.com/meshtastic/Meshtastic-device/tree/master/src/mesh/compression)

# Credits

- Thanks to [Jonathan Greenblatt](https://github.com/leafgarden) for his [port of Unishox2 that works on Particle Photon](https://github.com/siara-cc/Unishox/tree/master/Arduino)
- Thanks to [Chris Partridge](https://github.com/tweedge) for his [port of Unishox2 to CPython](https://github.com/tweedge/unishox2-py3) and his [comprehensive tests](https://github.com/tweedge/unishox2-py3#integration-tests) using [Hypothesis](https://hypothesis.readthedocs.io/en/latest) and [extensive performance tests](https://github.com/tweedge/unishox2-py3#performance)
- Thanks to [Stephan Hadinger](https://github.com/s-hadinger) for his [port of Unishox1 to Python for Tasmota](https://github.com/arendst/Tasmota/tree/development/tools/unishox)
- Thanks to [Luis Díaz Más](https://github.com/piponazo) for his PRs to support MSVC and CMake setup
- Thanks to [James Z.M. Gao](https://github.com/gsm55) for his PRs on improving presets, unit tests, bug fixes and more
- Thanks to [Jm Casler](https://github.com/mc-hamster) and [Shiv Kokroo](https://github.com/kokroo) for choosing and integrating Unishox into [Meshtastic](https://github.com/meshtastic/Meshtastic-device) project

# Versions

The present byte-code version is 2 and it replaces [Unishox 1](https://github.com/siara-cc/Unishox2/blob/master/Unishox1/Unishox_Article_1.pdf?raw=true).  Unishox 1 is still available as unishox1.c, but it will have to be compiled manually if it is needed.

The next version would be Unishox3 and it would include a multi-level static dictionaries residing in RAM or Flash memory that would greatly improve compression ratios compared to Unishox2.  However Unishox2 will still be supported for cases where space for storing static dictionaries is an issue.

# Issues

In case of any issues, please email the Author (Arundale Ramanathan) at arun@siara.cc or create GitHub issue.
