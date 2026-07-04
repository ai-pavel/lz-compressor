#include "stream.h"
#include "lz77.h"
#include "huffman.h"
#include "deflate.h"
#include <stdlib.h>
#include <string.h>

static const uint8_t MAGIC[4] = {'L', 'Z', 'C', 'M'};

int read_all(FILE *f, uint8_t **out, size_t *out_len) {
    if (!out || !out_len) return -1;

    size_t capacity = 8192;
    size_t len = 0;
    uint8_t *buf = malloc(capacity);
    if (!buf) return -1;

    for (;;) {
        size_t n = fread(buf + len, 1, capacity - len, f);
        len += n;
        if (n == 0) break;
        if (len == capacity) {
            capacity *= 2;
            uint8_t *tmp = realloc(buf, capacity);
            if (!tmp) { free(buf); return -1; }
            buf = tmp;
        }
    }

    *out = buf;
    *out_len = len;
    return 0;
}

long stream_compress(FILE *fin, FILE *fout, Algorithm algo) {
    uint8_t *input = NULL;
    size_t input_len = 0;
    if (read_all(fin, &input, &input_len) != 0)
        return -1;

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    int rc = -1;

    switch (algo) {
        case ALGO_LZ77:
            rc = lz77_compress(input, input_len, &compressed, &comp_len);
            break;
        case ALGO_HUFFMAN:
            rc = huffman_compress(input, input_len, &compressed, &comp_len);
            break;
        case ALGO_DEFLATE:
            rc = deflate_compress(input, input_len, &compressed, &comp_len);
            break;
        default:
            free(input);
            return -1;
    }

    if (rc != 0) {
        free(input);
        return -1;
    }

    /* Write header: magic(4) + algorithm(1) + original_size(4) */
    fwrite(MAGIC, 1, 4, fout);
    uint8_t algo_byte = (uint8_t)algo;
    fwrite(&algo_byte, 1, 1, fout);
    uint32_t orig_size = (uint32_t)input_len;
    fwrite(&orig_size, 1, 4, fout);

    /* Write compressed data */
    fwrite(compressed, 1, comp_len, fout);

    long total = (long)(9 + comp_len);

    free(input);
    free(compressed);
    return total;
}

long stream_decompress(FILE *fin, FILE *fout) {
    uint8_t *data = NULL;
    size_t data_len = 0;
    if (read_all(fin, &data, &data_len) != 0)
        return -1;

    if (data_len < 9) {
        free(data);
        return -1;
    }

    if (memcmp(data, MAGIC, 4) != 0) {
        free(data);
        return -1;
    }

    Algorithm algo = (Algorithm)data[4];
    uint32_t orig_size;
    memcpy(&orig_size, data + 5, 4);

    const uint8_t *compressed = data + 9;
    size_t comp_len = data_len - 9;

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    int rc = -1;

    switch (algo) {
        case ALGO_LZ77:
            rc = lz77_decompress(compressed, comp_len, &decompressed, &decomp_len);
            break;
        case ALGO_HUFFMAN:
            rc = huffman_decompress(compressed, comp_len, &decompressed, &decomp_len);
            break;
        case ALGO_DEFLATE:
            rc = deflate_decompress(compressed, comp_len, &decompressed, &decomp_len);
            break;
        default:
            free(data);
            return -1;
    }

    free(data);

    if (rc != 0) return -1;

    fwrite(decompressed, 1, decomp_len, fout);
    long result = (long)decomp_len;
    free(decompressed);
    return result;
}
