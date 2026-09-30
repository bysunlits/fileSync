#pragma once
#include <stddef.h>
#include <uchar.h>
#include "base/hld_err.h"

int sha256_in_stream(const char *data_in,unsigned char *hash,size_t data_size,ERR_INFO *err);

typedef struct MerkleTree
{
	char32_t root_node;

	void (*left_node)(struct MerkleTree *self);
	void (*right_node)(struct MerkleTree *self);
}MerkleTree;

typedef struct Node
{
	char hash[16];
	char left[16];
	char right[16];
	void (*node_default)(struct Node *self);
	void (*node_with_args)(struct Node *self,char32_t nd_parent,char32_t nd_left,char32_t nd_right);
}Node;