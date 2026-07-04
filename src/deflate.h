#ifndef DEFLATE_H
#define DEFLATE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DEFLATE-style compression: LZ77 followed by Huffman coding. */

/* Compress data. Caller must free *out. Returns 0 on success. */
int deflate_compress(const uint8_t *data, size_t data_len,
                     uint8_t **out, size_t *out_len);

/* Decompress data. Caller must free *out. Returns 0 on success. */
int deflate_decompress(const uint8_t *data, size_t data_len,
                       uint8_t **out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* DEFLATE_H */
