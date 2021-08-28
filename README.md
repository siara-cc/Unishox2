# Unishox - A hybrid encoder for compressing Short Unicode Strings

In general compression utilities such as `zip`, `gzip` do not compress short strings well and often expand them. They also use lots of memory which makes them unusable in constrained environments like Arduino.  So Unishox algorithm was developed for individually compressing (and decompressing) short strings.

Note: The present byte-code version is 2 and it replaces [Unishox 1](Unishox_Article_1.pdf?raw=true).  Unishox 1 is still available as unishox1.c, but it will have to be compiled manually if it is needed.

# Applications

- Compression for low memory devices such as Arduino and ESP8266
- Compression of Chat application text exchange include Emojis
- Storing compressed text in database
- Faster retrieval speed when used as join keys
- Bandwidth and storage cost reduction for Cloud

![Promo video](demo/Banner1.png?raw=true)

# How it works

Unishox is an hybrid encoder (entropy, dictionary and delta coding).  It works by assigning fixed prefix-free codes for each letter in the above Character Set (entropy coding).  It also encodes repeating letter sets separately (dictionary coding).  For Unicode characters, delta coding is used.

The model used for arriving at the prefix-free code is shown below:

![Promo video](demo/model.png?raw=true)

The complete specification can be found in this article: [A hybrid encoder for compressing Short Unicode Strings](Unishox_Article_2.pdf?raw=true).

# Compiling

To compile, just use `make` or use gcc as follows:

```sh
gcc -o unishox2 test_unishox2.c unishox2.c
```

For testing the compiled program, use:

```sh
./test_unishox2 -t
```

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

Unishox does not give good ratios compressing large files or compressing binary files.

# Character Set

Unishox supports the entire Unicode character set.  As of now it supports UTF-8 as input and output encoding.

# Projects that use Unishox

- [Unishox Compression Library for Arduino Progmem](https://github.com/siara-cc/Unishox_Arduino_Progmem_lib)
- [Sqlite3 User Defined Function as loadable extension](https://github.com/siara-cc/Unishox_Sqlite_UDF)
- [Sqlite3 Library for ESP32](https://github.com/siara-cc/esp32_arduino_sqlite3_lib)
- [Sqlite3 Library for ESP8266](https://github.com/siara-cc/esp_arduino_sqlite3_lib)
- [Port of Unishox 1 to Python and C++ by Stephan Hadinger for Tasmota](https://github.com/arendst/Tasmota/tree/development/tools/unishox)
- [Python bindings for Unishox2](https://github.com/tweedge/unishox2-py3)

# Credits

- Thanks to [Jonathan Greenblatt](https://github.com/leafgarden) for his [port of Unishox2 that works on Particle Photon](https://github.com/siara-cc/Unishox/tree/master/Arduino)
- Thanks to [Chris Partridge](https://github.com/tweedge) for his [port of Unishox2 to CPython](https://github.com/tweedge/unishox2-py3) and his [comprehensive tests](https://github.com/tweedge/unishox2-py3#integration-tests) using [Hypothesis](https://hypothesis.readthedocs.io/en/latest) and [extensive performance tests](https://github.com/tweedge/unishox2-py3#performance).
- Thanks to [Stephan Hadinger](https://github.com/s-hadinger) for his [port of Unishox1 to Python for Tasmota](https://github.com/arendst/Tasmota/tree/development/tools/unishox)

# Issues

In case of any issues, please email the Author (Arundale Ramanathan) at arun@siara.cc or create GitHub issue.
