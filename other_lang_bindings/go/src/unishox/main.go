// main.go
package main

// #cgo CFLAGS: -g -Wall
// #include "../../../../unishox2.c"
import "C"
import (
        "os"
	"fmt"
	"unsafe"
)

func compress(str_to_compress string) []byte {

	cstr := C.CString(str_to_compress)
	defer C.free(unsafe.Pointer(cstr))
        clen := C.int(len(str_to_compress))
        //defer C.free(unsafe.Pointer(&clen)) // Not sure if clen to be freed

	// Allocate sufficient buffer for holding compressed data
	// should be a little larger than the input string for safety
	ptr := C.malloc(C.sizeof_char * 1000)
	defer C.free(unsafe.Pointer(ptr))

	size := C.unishox2_compress_simple(cstr, clen, (*C.char)(ptr))

	compressed_bytes := C.GoBytes(ptr, size)
	return compressed_bytes

}

func decompress(compressed_bytes []byte) string {

        cbytes := C.CBytes(compressed_bytes)
        defer C.free(unsafe.Pointer(cbytes))

        // Allocate sufficient buffer for receiving decompressed string
        ptr := C.malloc(C.sizeof_char * 1000)
        defer C.free(unsafe.Pointer(ptr))
        dlen := C.int(len(compressed_bytes))
        // defer C.free(unsafe.Pointer(&dlen)) // Not sure if dlen to be freed

        str_size := C.unishox2_decompress_simple((*C.char)(cbytes), dlen, (*C.char) (ptr))

        orig_string := C.GoStringN((*C.char)(ptr), str_size)

	return orig_string

}

func main() {

	if (len(os.Args) < 2) {
		fmt.Println("Usage: go run main.go <string to compress>")
                return
        }

	compressed_bytes := compress(os.Args[1])
        fmt.Print("Compressed bytes: ")
        fmt.Println(compressed_bytes)
        fmt.Print("Length: ")
        fmt.Println(len(compressed_bytes))
        println()

        orig_string := decompress(compressed_bytes)
        fmt.Print("Original String: ")
        fmt.Println(orig_string)
        fmt.Print("Length (utf-8 bytes): ")
        fmt.Println(len(orig_string))

}

