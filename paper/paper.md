---
title: 'Unishox2: A hybrid encoder for Short Unicode Strings'
tags:
  - C
  - compression
  - encoding
  - string-compression
authors:
  - name: Arundale Ramanathan
    orcid: 0000-0001-6642-447X
    affiliation: 1
affiliations:
 - name: Independent Researcher
   index: 1
date: 16 October 2021
bibliography: paper.bib

---

# Summary

Unishox2 is a hybrid encoding technique with which short unicode strings could be compressed using context aware pre-mapped codes and delta coding resulting in surprisingly good ratios.

This article discusses a hybrid encoding method for compressing Short Unicode Strings of arbitrary lengths including Latin/English text and printable special characters. This has not been sufficiently addressed by lossless entropy encoding methods so far.

Although it appears inconsequential, space occupied by such strings become significant in memory constrained environments such as Arduino Uno and ESP8266. Text exchange in Chat applications is another area where cost savings could be seen using such compression. It is also possible to achieve savings in bandwidth and storage cost by storing and retrieving independent strings in Cloud databases.

# Statement of need

In information theory, `entropy encoding` is a lossless data compression scheme that is independent of the specific characteristics of the medium [[1]](#1).

One of the main types of entropy coding is about creating and assigning a unique `prefix-free code` to each unique symbol that occurs in the input. These entropy encoders then compress data by replacing each fixed-length input symbol with the corresponding variable-length prefix-free output code word.

According to Shannon's source coding theorem, the optimal code length for a symbol is $-log_bP$, where b is the number of symbols used to make output codes and P is the probability of the input symbol [[2]](#2). Therefore, the most common symbols use the shortest codes.

The most popular method for forming optimal prefix-free discrete codes is Huffman coding [[3]](#3).

A `Dictionary coder`, also sometimes known as a substitution coder, is a class of lossless data compression algorithms which operate by searching for matches between the text to be compressed and a set of strings contained in a data structure (called the \lq{dictionary}\rq) maintained by the encoder. When the encoder finds such a match, it substitutes a reference to the string's position in the data structure.

The LZ77 family of encoders use the dictionary encoding technique for compressing data. [[4]](#4)

`Delta coding` is a technique applied where encoding the difference between the previously encoded symbol or set of symbols is smaller compared to encoding the symbol or the set again. The differnce is determined by using the set minus operator or subtraction of values. [[5]](#5)

In contrast to these encoding methods, there are various other approaches to lossless coding including Run Length Encoding (RLE) and Burrows-Wheeler coding [[6]](#6).

While technologies such as GZip, Deflate, Zip, LZMA are available for file compression, they do not provide optimal compression for short strings. Eventhough these methods compress far more than what is proposed in this article, they often expand the original source for short strings because the symbol-code mapping also needs to be attached to aid decompression.

For compressing Unicode sequences, three other technologies are available: SCSU [[10]](#10) and BOCU [[11]](#11) and FAST [[12]](#12). A survey of these compression algorithms was published by Doug Ewell in 2004 [[13]](#13).

The Standard Compression Scheme for Unicode (SCSU) is defined in Unicode Technical Standard \#6 and is based on a technique originally developed by Reuters. The basic premise of SCSU is to define dynamically positioned windows into the Unicode code space, so that characters belonging to small scripts (such as the Greek alphabet or Indic abugidas) can be
encoded in a single byte, representing an index into the active window. These windows are preset to blocks expected to be in common use (e.g. Cyrillic), so the encoder doesn’t have to define them in these cases. There are also static windows that cannot be adjusted. [[10]](#10) [[13]](#13)

The Binary Ordered Compression for Unicode (BOCU) concept was developed in 2001 by Mark Davis and Markus Scherer for the ICU project. The main premise of BOCU-1 is to encode each Unicode character as the difference from the previous character, and to represent small differences in fewer bytes than large differences. By encoding differences, BOCU-1 achieves the same compression for all small alphabetic scripts, regardless of the block they reside in. [[11]](#11) [[13]](#13)

It is to be noted that SCSU is a Unicode Technical Standard (UTS\#6) and BOCU is published as a Unicode Technical Note (UTN\#6), although both have the same number assigned (6).

Fast Compression Algorithm For Unicode Text (FAST) is a compression algorithm developed based on Lempel Ziv algorithm [[4]](#4). Essentially it achieves faster compression by finding repeating unicode sequences instead of repeating bytes. There are other assumptions and variations made to LZ technique in addition to this. [[12]](#12)

Other technologies available are Smaz and shoco, which are not developed with Unicode in mind.

Smaz is a simple compression library suitable for compressing very short strings [[14]](#14). It was developed by Salvatore Sanfilippo and is released under the BSD license.

Shoco is a C library to compress short strings [[15]](#15). It was developed by Christian Schramm and is released under the MIT license.

While both are lossless encoding methods, Smaz is dictionary based and Shoco classifies as an entropy coder [[15]](#15).

In addition to providing a default frequency table as model, shoco provides an option to re-define the frequency table based on training text [[15]](#15).

A hybrid encoding method is proposed relying on the three encoding techniques `viz.` Entropy encoding, Dictionary coding and Delta encoding methods for optimal compression.

While existing techniques focus on either Unicode character sequences or only English characters, Unishox uses multiple techhniques to achieve the best compression ratio all round.

For Unicode, Delta encoding is proposed because usually the difference between subsequent symbols is quite less while encoding text of a particular language. SCSU is slightly better with switching windows, but overall it was found that plain Delta coding works well considering that usually there is only one language text to be compressed and some languages span a lot of windows.

SCSU and BOCU do have special features for Unicode that Unishox does not address such as dynamic windows, binary order maintenance, XML suitability and MIME friendliness. Unishox uses plain delta encoding to achieve the best compression.

For English letters, unlike shoco, a fixed frequency table is proposed, generated based on the characterestics of English language letter frequency. The research carried out by Oxford University [[7]](#7) and other sources [[7]](#7) [[9]](#9) have been used to arrive at a unique method that takes advantage of the conventions of the language.

A single fixed model is used because of the advantages it offers over the training models of shoco. The disadvantage with the training model, although it may appear to offer more compression, is that it does not consider the patterns that usually appear during text formation. It can be seen that this performs better than pre-trained model of shoco (See performance section).

Unlike smaz and shoco, no `a priori` knowledge about the input text is assumed. However knowledge from the research carried out on the language and common patterns of sentence formation have been used to come out with pre-assigned codes for each letter.

# Acknowledgements

The author is sincerely thankful to the following people who have notably contributed towards the development of Unishox implementation:

- Thanks to [Jonathan Greenblatt](https://github.com/leafgarden) for his [port of Unishox2 that works on Particle Photon](https://github.com/siara-cc/Unishox/tree/master/Arduino)
- Thanks to [Chris Partridge](https://github.com/tweedge) for his [port of Unishox2 to CPython](https://github.com/tweedge/unishox2-py3) and his [comprehensive tests](https://github.com/tweedge/unishox2-py3#integration-tests) using [Hypothesis](https://hypothesis.readthedocs.io/en/latest) and [extensive performance tests](https://github.com/tweedge/unishox2-py3#performance).
- Thanks to [Stephan Hadinger](https://github.com/s-hadinger) for his [port of Unishox1 to Python for Tasmota](https://github.com/arendst/Tasmota/tree/development/tools/unishox)
- Thanks to [Luis Díaz Más](https://github.com/piponazo) for his PRs to support MSVC and CMake setup
- Thanks to [James Z.M. Gao](https://github.com/gsm55) for his PRs on improving presets, safety checks, terminator codes, unit tests, bug fixes, documentation and more

# References
