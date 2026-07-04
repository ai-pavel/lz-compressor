#include "stream.h"
#include <stdio.h>
#include <string.h>

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s compress   [--lz77|--huffman|--deflate] <input> <output>\n"
        "  %s decompress <input> <output>\n"
        "\nDefault algorithm: --deflate\n",
        prog, prog);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    const char *mode = argv[1];

    if (strcmp(mode, "compress") == 0) {
        Algorithm algo = ALGO_DEFLATE;
        int file_arg_start = 2;

        if (argc >= 5) {
            const char *algo_str = argv[2];
            if (strcmp(algo_str, "--lz77") == 0) {
                algo = ALGO_LZ77;
                file_arg_start = 3;
            } else if (strcmp(algo_str, "--huffman") == 0) {
                algo = ALGO_HUFFMAN;
                file_arg_start = 3;
            } else if (strcmp(algo_str, "--deflate") == 0) {
                algo = ALGO_DEFLATE;
                file_arg_start = 3;
            }
        }

        if (file_arg_start + 1 >= argc) {
            print_usage(argv[0]);
            return 1;
        }

        const char *input_file = argv[file_arg_start];
        const char *output_file = argv[file_arg_start + 1];

        FILE *fin = fopen(input_file, "rb");
        if (!fin) {
            fprintf(stderr, "Error: cannot open input file: %s\n", input_file);
            return 1;
        }

        /* Get input size */
        fseek(fin, 0, SEEK_END);
        long input_size = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        FILE *fout = fopen(output_file, "wb");
        if (!fout) {
            fprintf(stderr, "Error: cannot open output file: %s\n", output_file);
            fclose(fin);
            return 1;
        }

        long output_size = stream_compress(fin, fout, algo);
        fclose(fin);
        fclose(fout);

        if (output_size < 0) {
            fprintf(stderr, "Compression error\n");
            return 1;
        }

        double ratio = (input_size > 0)
            ? (double)output_size / (double)input_size * 100.0
            : 0.0;

        printf("Compressed %ld -> %ld bytes (%.1f%% of original)\n",
               input_size, output_size, ratio);

    } else if (strcmp(mode, "decompress") == 0) {
        if (argc < 4) {
            print_usage(argv[0]);
            return 1;
        }

        const char *input_file = argv[2];
        const char *output_file = argv[3];

        FILE *fin = fopen(input_file, "rb");
        if (!fin) {
            fprintf(stderr, "Error: cannot open input file: %s\n", input_file);
            return 1;
        }

        fseek(fin, 0, SEEK_END);
        long input_size = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        FILE *fout = fopen(output_file, "wb");
        if (!fout) {
            fprintf(stderr, "Error: cannot open output file: %s\n", output_file);
            fclose(fin);
            return 1;
        }

        long output_size = stream_decompress(fin, fout);
        fclose(fin);
        fclose(fout);

        if (output_size < 0) {
            fprintf(stderr, "Decompression error\n");
            return 1;
        }

        printf("Decompressed %ld -> %ld bytes\n", input_size, output_size);

    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
