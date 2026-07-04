#ifndef LZ77_H
#define LZ77_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LZ77_DEFAULT_WINDOW_SIZE    4096
#define LZ77_DEFAULT_LOOKAHEAD_SIZE 18

typedef struct {
    uint16_t offset;
    uint16_t length;
    uint8_t  next_char;
} LZ77Token;

typedef struct {
    uint16_t window_size;
    uint16_t lookahead_size;
} LZ77Config;

/* Create a default config. */
LZ77Config lz77_default_config(void);

/* Encode raw data into tokens. Caller must free *out_tokens. Returns token count, or -1 on error. */
int lz77_encode(const LZ77Config *cfg, const uint8_t *data, size_t data_len,
                LZ77Token **out_tokens, size_t *out_count);

/* Decode tokens back to raw data. Caller must free *out_data. Returns 0 on success, -1 on error. */
int lz77_decode(const LZ77Config *cfg, const LZ77Token *tokens, size_t count,
                uint8_t **out_data, size_t *out_len);

/* Serialize tokens to bytes. Caller must free *out. Returns 0 on success. */
int lz77_serialize(const LZ77Token *tokens, size_t count,
                   uint8_t **out, size_t *out_len);

/* Deserialize bytes to tokens. Caller must free *out_tokens. Returns 0 on success. */
int lz77_deserialize(const uint8_t *data, size_t data_len,
                     LZ77Token **out_tokens, size_t *out_count);

/* Compress raw data. Caller must free *out. Returns 0 on success. */
int lz77_compress(const uint8_t *data, size_t data_len,
                  uint8_t **out, size_t *out_len);

/* Decompress data. Caller must free *out. Returns 0 on success. */
int lz77_decompress(const uint8_t *data, size_t data_len,
                    uint8_t **out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* LZ77_H */
