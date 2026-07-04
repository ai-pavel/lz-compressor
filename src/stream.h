#ifndef STREAM_H
#define STREAM_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ALGO_LZ77    = 1,
    ALGO_HUFFMAN = 2,
    ALGO_DEFLATE = 3
} Algorithm;

/* Streaming compress: reads all of fin, compresses, writes to fout.
   Returns number of bytes written to fout, or -1 on error. */
long stream_compress(FILE *fin, FILE *fout, Algorithm algo);

/* Streaming decompress: reads all of fin, decompresses, writes to fout.
   Algorithm is auto-detected from file header.
   Returns number of bytes written to fout, or -1 on error. */
long stream_decompress(FILE *fin, FILE *fout);

/* Read all bytes from a FILE*. Caller must free *out. Returns 0 on success. */
int read_all(FILE *f, uint8_t **out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* STREAM_H */
