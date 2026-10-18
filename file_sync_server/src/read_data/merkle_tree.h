#pragma once
#include "base/hld_err.h"
#include <stddef.h>
#include <uchar.h>
#define LOG_MODULE "read_data"
#include "logger.h"
#include <openssl/evp.h>
#include <openssl/sha.h>

#define SHA256_DIGEST_LENGTH 32

typedef struct MerkleTree {
	unsigned char root_node[65];
} MerkleTree;

struct Node* build_merkle_tree(unsigned char leaf_hashes[][SHA256_DIGEST_LENGTH], int n);

int node_default(unsigned char *block, size_t data_size,unsigned char *node);

int node_with_args(unsigned char *parents,unsigned char *left,unsigned char *right);

int sha256(const unsigned char *block,size_t data_size, unsigned char *hash,int hash_length);

void bytes_to_hex(const unsigned char *bin, unsigned int bin_len, char *hex);
