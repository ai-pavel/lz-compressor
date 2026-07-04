#include "huffman.h"
#include <stdlib.h>
#include <string.h>

/* ---- Internal Huffman tree node ---- */

typedef struct HuffNode {
    uint8_t byte;
    uint32_t freq;
    struct HuffNode *left;
    struct HuffNode *right;
} HuffNode;

static HuffNode *huff_new_node(uint8_t byte, uint32_t freq) {
    HuffNode *n = calloc(1, sizeof(HuffNode));
    if (n) {
        n->byte = byte;
        n->freq = freq;
    }
    return n;
}

static void huff_free_tree(HuffNode *node) {
    if (!node) return;
    huff_free_tree(node->left);
    huff_free_tree(node->right);
    free(node);
}

static int huff_is_leaf(const HuffNode *n) {
    return n && !n->left && !n->right;
}

/* ---- Min-heap (priority queue) for building tree ---- */

typedef struct {
    HuffNode **nodes;
    size_t size;
    size_t capacity;
} MinHeap;

static MinHeap *heap_create(size_t cap) {
    MinHeap *h = malloc(sizeof(MinHeap));
    if (!h) return NULL;
    h->nodes = malloc(cap * sizeof(HuffNode *));
    if (!h->nodes) { free(h); return NULL; }
    h->size = 0;
    h->capacity = cap;
    return h;
}

static void heap_free(MinHeap *h) {
    if (h) { free(h->nodes); free(h); }
}

static void heap_swap(HuffNode **a, HuffNode **b) {
    HuffNode *t = *a; *a = *b; *b = t;
}

static void heap_sift_up(MinHeap *h, size_t i) {
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (h->nodes[parent]->freq > h->nodes[i]->freq) {
            heap_swap(&h->nodes[parent], &h->nodes[i]);
            i = parent;
        } else break;
    }
}

static void heap_sift_down(MinHeap *h, size_t i) {
    for (;;) {
        size_t smallest = i;
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        if (left < h->size && h->nodes[left]->freq < h->nodes[smallest]->freq)
            smallest = left;
        if (right < h->size && h->nodes[right]->freq < h->nodes[smallest]->freq)
            smallest = right;
        if (smallest == i) break;
        heap_swap(&h->nodes[i], &h->nodes[smallest]);
        i = smallest;
    }
}

static void heap_push(MinHeap *h, HuffNode *node) {
    h->nodes[h->size] = node;
    heap_sift_up(h, h->size);
    h->size++;
}

static HuffNode *heap_pop(MinHeap *h) {
    HuffNode *top = h->nodes[0];
    h->size--;
    h->nodes[0] = h->nodes[h->size];
    heap_sift_down(h, 0);
    return top;
}

/* ---- Build Huffman tree from frequency table ---- */

static HuffNode *build_tree(const uint32_t freq[256], int *num_symbols) {
    int nsym = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) nsym++;
    }
    if (num_symbols) *num_symbols = nsym;
    if (nsym == 0) return NULL;

    MinHeap *heap = heap_create(nsym + 1);
    if (!heap) return NULL;

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HuffNode *n = huff_new_node((uint8_t)i, freq[i]);
            heap_push(heap, n);
        }
    }

    /* Handle single-symbol case */
    if (heap->size == 1) {
        HuffNode *child = heap_pop(heap);
        HuffNode *parent = huff_new_node(0, child->freq);
        parent->left = child;
        heap_free(heap);
        return parent;
    }

    while (heap->size > 1) {
        HuffNode *left = heap_pop(heap);
        HuffNode *right = heap_pop(heap);
        HuffNode *parent = huff_new_node(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        heap_push(heap, parent);
    }

    HuffNode *root = heap_pop(heap);
    heap_free(heap);
    return root;
}

/* ---- Build code table ---- */

typedef struct {
    uint8_t bits[32]; /* bit pattern stored as 0/1 bytes */
    uint8_t len;
} HuffCode;

static void build_codes_recurse(const HuffNode *node, uint8_t *path, int depth,
                                HuffCode codes[256]) {
    if (!node) return;
    if (huff_is_leaf(node)) {
        codes[node->byte].len = (depth == 0) ? 1 : (uint8_t)depth;
        if (depth == 0) {
            codes[node->byte].bits[0] = 0;
        } else {
            memcpy(codes[node->byte].bits, path, depth);
        }
        return;
    }
    path[depth] = 0;
    build_codes_recurse(node->left, path, depth + 1, codes);
    path[depth] = 1;
    build_codes_recurse(node->right, path, depth + 1, codes);
}

/* ---- Serialize / deserialize tree ---- */

typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} ByteBuf;

static int buf_init(ByteBuf *b, size_t cap) {
    b->data = malloc(cap);
    if (!b->data) return -1;
    b->len = 0;
    b->cap = cap;
    return 0;
}

static int buf_push(ByteBuf *b, uint8_t byte) {
    if (b->len >= b->cap) {
        size_t nc = b->cap * 2;
        uint8_t *tmp = realloc(b->data, nc);
        if (!tmp) return -1;
        b->data = tmp;
        b->cap = nc;
    }
    b->data[b->len++] = byte;
    return 0;
}

static int buf_append(ByteBuf *b, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (buf_push(b, data[i]) != 0) return -1;
    }
    return 0;
}

static void serialize_tree(const HuffNode *node, ByteBuf *buf) {
    if (!node) return;
    if (huff_is_leaf(node)) {
        buf_push(buf, 1); /* leaf marker */
        buf_push(buf, node->byte);
    } else {
        buf_push(buf, 0); /* internal marker */
        serialize_tree(node->left, buf);
        serialize_tree(node->right, buf);
    }
}

static HuffNode *deserialize_tree(const uint8_t *data, size_t data_len, size_t *pos) {
    if (*pos >= data_len) return NULL;

    HuffNode *node = huff_new_node(0, 0);
    if (!node) return NULL;

    uint8_t marker = data[(*pos)++];
    if (marker == 1) {
        /* Leaf */
        if (*pos >= data_len) { free(node); return NULL; }
        node->byte = data[(*pos)++];
    } else {
        node->left = deserialize_tree(data, data_len, pos);
        node->right = deserialize_tree(data, data_len, pos);
    }
    return node;
}

/* ---- Public API ---- */

int huffman_compress(const uint8_t *data, size_t data_len,
                     uint8_t **out, size_t *out_len) {
    if (!out || !out_len) return -1;

    if (data_len == 0) {
        /* Return minimal header: 4 zero bytes for original size */
        *out = calloc(4, 1);
        if (!*out) return -1;
        *out_len = 4;
        return 0;
    }

    /* Build frequency table */
    uint32_t freq[256] = {0};
    for (size_t i = 0; i < data_len; i++) freq[data[i]]++;

    HuffNode *root = build_tree(freq, NULL);
    if (!root) return -1;

    /* Build code table */
    HuffCode codes[256];
    memset(codes, 0, sizeof(codes));
    uint8_t path[32];
    build_codes_recurse(root, path, 0, codes);

    /* Encode data: count total bits first */
    size_t total_bits = 0;
    for (size_t i = 0; i < data_len; i++) {
        total_bits += codes[data[i]].len;
    }

    uint8_t padding = (uint8_t)((8 - (total_bits % 8)) % 8);
    size_t encoded_bytes_count = (total_bits + padding) / 8;

    /* Pack bits into bytes */
    uint8_t *encoded_bytes = calloc(encoded_bytes_count, 1);
    if (!encoded_bytes) { huff_free_tree(root); return -1; }

    size_t bit_pos = 0;
    for (size_t i = 0; i < data_len; i++) {
        const HuffCode *c = &codes[data[i]];
        for (uint8_t j = 0; j < c->len; j++) {
            if (c->bits[j]) {
                encoded_bytes[bit_pos / 8] |= (uint8_t)(1 << (7 - (bit_pos % 8)));
            }
            bit_pos++;
        }
    }

    /* Serialize tree */
    ByteBuf tree_buf;
    if (buf_init(&tree_buf, 256) != 0) {
        free(encoded_bytes);
        huff_free_tree(root);
        return -1;
    }
    serialize_tree(root, &tree_buf);
    huff_free_tree(root);

    /* Output format:
       [4 bytes: original size]
       [1 byte: padding bits]
       [2 bytes: tree data size]
       [tree data]
       [encoded bytes] */
    uint32_t orig_size = (uint32_t)data_len;
    uint16_t tree_size = (uint16_t)tree_buf.len;
    size_t result_len = 4 + 1 + 2 + tree_buf.len + encoded_bytes_count;
    uint8_t *result = malloc(result_len);
    if (!result) {
        free(tree_buf.data);
        free(encoded_bytes);
        return -1;
    }

    size_t pos = 0;
    memcpy(result + pos, &orig_size, 4); pos += 4;
    result[pos++] = padding;
    memcpy(result + pos, &tree_size, 2); pos += 2;
    memcpy(result + pos, tree_buf.data, tree_buf.len); pos += tree_buf.len;
    memcpy(result + pos, encoded_bytes, encoded_bytes_count);

    free(tree_buf.data);
    free(encoded_bytes);

    *out = result;
    *out_len = result_len;
    return 0;
}

int huffman_decompress(const uint8_t *data, size_t data_len,
                       uint8_t **out, size_t *out_len) {
    if (!out || !out_len) return -1;
    if (data_len < 4) return -1;

    uint32_t orig_size;
    memcpy(&orig_size, data, 4);

    if (orig_size == 0) {
        *out = NULL;
        *out_len = 0;
        return 0;
    }

    if (data_len < 7) return -1;

    uint8_t padding = data[4];
    uint16_t tree_size;
    memcpy(&tree_size, data + 5, 2);

    size_t pos = 7;
    HuffNode *root = deserialize_tree(data, data_len, &pos);
    if (!root) return -1;

    size_t encoded_start = 7 + tree_size;
    if (encoded_start > data_len) {
        huff_free_tree(root);
        return -1;
    }

    /* Decode bits */
    uint8_t *result = malloc(orig_size);
    if (!result) { huff_free_tree(root); return -1; }

    size_t result_pos = 0;
    HuffNode *node = root;

    /* Total bits in encoded data, minus padding */
    size_t total_encoded_bytes = data_len - encoded_start;
    size_t total_bits = total_encoded_bytes * 8;
    if (padding <= total_bits) total_bits -= padding;

    size_t bit_idx = 0;
    for (size_t i = encoded_start; i < data_len && result_pos < orig_size; i++) {
        for (int j = 7; j >= 0 && result_pos < orig_size; j--) {
            if (bit_idx >= total_bits) goto done;
            bit_idx++;

            if ((data[i] >> j) & 1) {
                node = node->right;
            } else {
                node = node->left;
            }

            if (node && huff_is_leaf(node)) {
                result[result_pos++] = node->byte;
                node = root;
            }
        }
    }

done:
    huff_free_tree(root);

    *out = result;
    *out_len = (size_t)orig_size;
    return 0;
}
