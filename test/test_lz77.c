#include "lz77.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_roundtrip_simple(void) {
    const char *str = "AABABCABCABC";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_simple\n");
}

static void test_roundtrip_repeated(void) {
    uint8_t input[500];
    memset(input, 'X', 500);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 500, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == 500);
    assert(memcmp(decompressed, input, 500) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_repeated\n");
}

static void test_roundtrip_empty(void) {
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(NULL, 0, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_empty\n");
}

static void test_roundtrip_single_byte(void) {
    uint8_t input[] = {'A'};
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 1, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(lz77_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 1);
    assert(decompressed[0] == 'A');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_single_byte\n");
}

static void test_compression_reduces_size(void) {
    uint8_t input[1000];
    memset(input, 'Z', 1000);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(lz77_compress(input, 1000, &compressed, &comp_len) == 0);
    assert(comp_len < 1000);

    free(compressed);
    printf("  PASS: compression_reduces_size\n");
}

static void test_encode_decode_tokens(void) {
    const char *str = "ABCABC";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    LZ77Config cfg = lz77_default_config();
    LZ77Token *tokens = NULL;
    size_t count = 0;
    assert(lz77_encode(&cfg, input, input_len, &tokens, &count) == 0);

    uint8_t *decoded = NULL;
    size_t decoded_len = 0;
    assert(lz77_decode(&cfg, tokens, count, &decoded, &decoded_len) == 0);
    assert(decoded_len == input_len);
    assert(memcmp(decoded, input, input_len) == 0);

    free(tokens);
    free(decoded);
    printf("  PASS: encode_decode_tokens\n");
}

int main(void) {
    printf("LZ77 tests:\n");
    test_roundtrip_simple();
    test_roundtrip_repeated();
    test_roundtrip_empty();
    test_roundtrip_single_byte();
    test_compression_reduces_size();
    test_encode_decode_tokens();
    printf("All LZ77 tests passed.\n\n");
    return 0;
}
