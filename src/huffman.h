#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Compress data using Huffman coding. Caller must free *out. Returns 0 on success. */
int huffman_compress(const uint8_t *data, size_t data_len,
                     uint8_t **out, size_t *out_len);

/* Decompress Huffman-coded data. Caller must free *out. Returns 0 on success. */
int huffman_decompress(const uint8_t *data, size_t data_len,
                       uint8_t **out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* HUFFMAN_H */
