#pragma once
#include "base/hld_err.h"
#include <stddef.h>
#include <uchar.h>
#define LOG_MODULE "read_data"
#include "logger.h"

typedef struct MerkleTree {
	unsigned char root_node[65];
} MerkleTree;

int node_default(unsigned char *block, size_t data_size);

struct Node* node_with_args(unsigned char *nd_left,unsigned char *nd_right);

int sha256(const unsigned char *block,size_t data_size, unsigned char *hash,int hash_length);

void bytes_to_hex(const unsigned char *bin, unsigned int bin_len, char *hex);
