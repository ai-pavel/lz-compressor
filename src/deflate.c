#include "deflate.h"
#include "lz77.h"
#include "huffman.h"
#include <stdlib.h>

int deflate_compress(const uint8_t *data, size_t data_len,
                     uint8_t **out, size_t *out_len) {
    /* Stage 1: LZ77 compression */
    uint8_t *lz_out = NULL;
    size_t lz_len = 0;
    if (lz77_compress(data, data_len, &lz_out, &lz_len) != 0)
        return -1;

    /* Stage 2: Huffman coding on the LZ77 output */
    int rc = huffman_compress(lz_out, lz_len, out, out_len);
    free(lz_out);
    return rc;
}

int deflate_decompress(const uint8_t *data, size_t data_len,
                       uint8_t **out, size_t *out_len) {
    /* Stage 1: Huffman decode */
    uint8_t *huff_out = NULL;
    size_t huff_len = 0;
    if (huffman_decompress(data, data_len, &huff_out, &huff_len) != 0)
        return -1;

    /* Stage 2: LZ77 decode */
    int rc = lz77_decompress(huff_out, huff_len, out, out_len);
    free(huff_out);
    return rc;
}
