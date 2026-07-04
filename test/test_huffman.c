#include "huffman.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_roundtrip_simple(void) {
    const char *str = "hello world, this is a huffman test!";
    const uint8_t *input = (const uint8_t *)str;
    size_t input_len = strlen(str);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, input_len, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);

    assert(decomp_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_simple\n");
}

static void test_roundtrip_empty(void) {
    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(NULL, 0, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_empty\n");
}

static void test_roundtrip_single_byte(void) {
    uint8_t input[] = {'A'};

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 1, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 1);
    assert(decompressed[0] == 'A');

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_single_byte\n");
}

static void test_roundtrip_repeated_char(void) {
    uint8_t input[200];
    memset(input, 'Q', 200);

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 200, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 200);
    assert(memcmp(decompressed, input, 200) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_repeated_char\n");
}

static void test_roundtrip_binary(void) {
    uint8_t input[256];
    for (int i = 0; i < 256; i++) input[i] = (uint8_t)i;

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 256, &compressed, &comp_len) == 0);

    uint8_t *decompressed = NULL;
    size_t decomp_len = 0;
    assert(huffman_decompress(compressed, comp_len, &decompressed, &decomp_len) == 0);
    assert(decomp_len == 256);
    assert(memcmp(decompressed, input, 256) == 0);

    free(compressed);
    free(decompressed);
    printf("  PASS: roundtrip_binary\n");
}

static void test_compresses_skewed(void) {
    uint8_t input[1000];
    memset(input, 'a', 1000);
    input[100] = 'b';
    input[500] = 'c';

    uint8_t *compressed = NULL;
    size_t comp_len = 0;
    assert(huffman_compress(input, 1000, &compressed, &comp_len) == 0);
    assert(comp_len < 1000);

    free(compressed);
    printf("  PASS: compresses_skewed\n");
}

int main(void) {
    printf("Huffman tests:\n");
    test_roundtrip_simple();
    test_roundtrip_empty();
    test_roundtrip_single_byte();
    test_roundtrip_repeated_char();
    test_roundtrip_binary();
    test_compresses_skewed();
    printf("All Huffman tests passed.\n\n");
    return 0;
}
