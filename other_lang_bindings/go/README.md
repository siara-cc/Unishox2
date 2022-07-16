# Golang binding for Unishox

Please see `main.go` under `src/unishox`, which uses `cgo` for calling the Unishox C Library.

To run the example, please change folder to `src/unishox` and execute:

```
go main.go "Hello World"
```

and it should show the compressed bytes and uncompressed original string.

It works with the entire Unicode character set. For example:

```
go main.go "Hello World 🙂🙂"
```

