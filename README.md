# Unishox - Guaranteed Compression for Unicode Short Strings

This is a C library for compressing short strings.  It was developed to individually compress and decompress small strings. In general compression utilities such as `zip`, `gzip` do not compress short strings well and often expand them. They also use lots of memory which makes them unusable in constrained environments like Arduino.

# Applications

- Compression for low memory devices such as Arduino and ESP8266
- Compression of Chat application text exchange include Emojis
- Storing compressed text in database
- Faster retrieval speed when used as join keys

![Promo video](Banner1.png?raw=true)

# How it works

Unishox is an hybrid encoder (entropy, dictionary and delta coding).  It works by assigning fixed prefix-free codes for each letter in the above Character Set (entropy coding).  It also encodes repeating letter sets separately (dictionary coding).  For Unicode characters, delta coding is used. More information is available in [this article](Unishox_Article_1.pdf?raw=true).

# Compiling

To compile, just use `make` or use gcc as follows:

```sh
gcc -o unishox1 unishox1.c
```

# API

```C
int unishox1_compress(const char *in, int len, char *out, struct lnk_lst *prev_lines);
int unishox1_decompress(const char *in, int len, char *out, struct lnk_lst *prev_lines);
```

The `lnk_list` is used only when a bunch of strings are compressed for use with Arduino Flash Memory.  Just pass `NULL` if you have only one String to compress or decompress.

# Usage

To see Unishox in action, simply try to compress a string:

```
./unishox1 "Hello World"
```

To compress and decompress a file, use:

```
./unishox1 -c <input_file> <compressed_file>
./unishox1 -d <compressed_file> <decompressed_file>
```

Unishox does not give good ratios compressing files for compressing binary files.

# Character Set

Unishox supports the entire Unicode character set.  As of now it supports UTF-8 as input and output encoding.

For Binary symbols (ASCII 0 to 31 and 128 to 255), it actually expands the input size.  This is expected to be addressed in future versions.

# Projects that use Unishox

- [Unishox Compression Library for Arduino Progmem](https://github.com/siara-cc/Unishox_Arduino_Progmem_lib)
- [Sqlite3 User Defined Function as loadable extension](https://github.com/siara-cc/Unishox_Sqlite_UDF)
- [Sqlite3 Library for ESP32](https://github.com/siara-cc/esp32_arduino_sqlite3_lib)
- [Sqlite3 Library for ESP8266](https://github.com/siara-cc/esp_arduino_sqlite3_lib)
- [Port of this library to Python and C++ by Stephan Hadinger for Tasmota](https://github.com/arendst/Tasmota/tree/development/lib/Unishox-1.0-shadinger)

# Issues

In case of any issues, please email the Author (Arundale Ramanathan) at arun@siara.cc or create GitHub issue.
