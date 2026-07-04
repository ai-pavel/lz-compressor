#include "stream.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper: compress a string through FILE streams and decompress, verify roundtrip. */
static void test_stream_roundtrip(const char *label, const char *text, Algorithm algo) {
    size_t text_len = strlen(text);

    /* Write text to a temp file */
    FILE *tmp_in = tmpfile();
    assert(tmp_in);
    fwrite(text, 1, text_len, tmp_in);
    rewind(tmp_in);

    /* Compress to another temp file */
    FILE *tmp_comp = tmpfile();
    assert(tmp_comp);
    long comp_size = stream_compress(tmp_in, tmp_comp, algo);
    assert(comp_size > 0);
    fclose(tmp_in);

    /* Decompress */
    rewind(tmp_comp);
    FILE *tmp_out = tmpfile();
    assert(tmp_out);
    long decomp_size = stream_decompress(tmp_comp, tmp_out);
    fclose(tmp_comp);

    assert(decomp_size == (long)text_len);

    /* Read back and compare */
    rewind(tmp_out);
    char *result = malloc(text_len + 1);
    assert(result);
    size_t n = fread(result, 1, text_len, tmp_out);
    assert(n == text_len);
    result[text_len] = '\0';
    fclose(tmp_out);

    assert(memcmp(result, text, text_len) == 0);
    free(result);

    printf("  PASS: %s\n", label);
}

static void test_read_all_helper(void) {
    const char *text = "some binary data";
    size_t text_len = strlen(text);

    FILE *f = tmpfile();
    assert(f);
    fwrite(text, 1, text_len, f);
    rewind(f);

    uint8_t *data = NULL;
    size_t data_len = 0;
    assert(read_all(f, &data, &data_len) == 0);
    fclose(f);

    assert(data_len == text_len);
    assert(memcmp(data, text, text_len) == 0);
    free(data);

    printf("  PASS: read_all_helper\n");
}

int main(void) {
    printf("Stream tests:\n");
    test_stream_roundtrip("stream_lz77",
        "Hello hello hello world world world!", ALGO_LZ77);
    test_stream_roundtrip("stream_huffman",
        "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ", ALGO_HUFFMAN);
    test_stream_roundtrip("stream_deflate",
        "DEFLATE combines LZ77 and Huffman. DEFLATE combines LZ77 and Huffman.",
        ALGO_DEFLATE);
    test_read_all_helper();
    printf("All Stream tests passed.\n\n");
    return 0;
}
