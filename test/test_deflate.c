#include "deflate.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_roundtrip_simple(void) {
    const char *str = "ABCABCABCABCABC";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_simple\n");
}

static void test_roundtrip_empty(void) {
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(NULL, 0, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_empty\n");
}

static void test_roundtrip_longer_text(void) {
    const char *str =
        "The quick brown fox jumps over the lazy dog. "
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "Pack my box with five dozen liquor jugs.";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(deflate_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_longer_text\n");
}

static void test_compresses_repeated(void) {
    uint8_t input[2000];
    memset(input, 'X', 2000);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(deflate_compress(input, 2000, &compressed, &comp_len) == 0);
    assert(comp_len < 2000);

    free(compressed);
    printf("  PASS: compresses_repeated\n");
}

int main(void) {
    printf("Deflate tests:\n");
    test_roundtrip_simple();
    test_roundtrip_empty();
    test_roundtrip_longer_text();
    test_compresses_repeated();
    printf("All Deflate tests passed.\n\n");
    return 0;
}
