# Golang binding for Unishox

## Unishox 2

Please see `main.go` under `src/unishox`, which uses `cgo` for calling the Unishox2 C Library.

To run the example, please change folder to `src/unishox` and execute:

```
go main.go "Hello World"
```

and it should show the compressed bytes and uncompressed original string.

It works with the entire Unicode character set. For example:

```
go main.go "Hello World 🙂🙂"
```
## Unishox 3 Alpha

For using Unishox3 Alpha, please see `main_usx3_alpha.go` under `src/unishox`, which uses `cgo` for calling the Unishox3 C++ Library.

At first, the C++ library needs to be compile to a shared so file using the given sh file, after chaing folder to `src/unishox`:

```
sh compile_usx3_so.sh
```

Before compiling, please ensure `marisa` library is installed using `sudo port install marisa-trie` or `sudo yum -y install marisa` depending on your OS.

To run the example, execute:

```
go run main_usx3_alpha.go "Hello World"
```

and it should show the compressed bytes and uncompressed original string.
