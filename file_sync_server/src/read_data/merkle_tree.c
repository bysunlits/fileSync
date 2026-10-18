//
// Created by sunlit on 2026/9/29.
//
#include "merkle_tree.h"
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdlib.h>


#define BLOCK_SZ 32768U
#define SHA256_DIGEST_LENGTH 32

typedef struct Node {
	unsigned char hash[SHA256_DIGEST_LENGTH];
	struct Node *left;
	struct Node *right;
} Node;

void build_tree(MerkleTree *self, unsigned char *data, uint32_t data_size)
{
	unsigned char *offset=data;
	int leaves_sum=(int)data_size/BLOCK_SZ;
	if (data_size%BLOCK_SZ>0)leaves_sum++;
	unsigned char leaf_hashes[leaves_sum][SHA256_DIGEST_LENGTH];
	for (uint32_t i = 0; i < leaves_sum; i +=1) {
			int block_sz =
				BLOCK_SZ < (data_size - 1) ? BLOCK_SZ : (data_size - i);
			node_default(offset, block_sz,&leaf_hashes[i][0]);
			offset+=block_sz;
	}
	build_merkle_tree(leaf_hashes,leaves_sum);
}


Node* build_merkle_tree(unsigned char leaf_hashes[][SHA256_DIGEST_LENGTH], int n) {
	if (n <= 0) return NULL;

	Node *nodes = (Node *)malloc(sizeof(Node) * (2 * n));

	for (int i = 0; i < n; i++) {
		memcpy(nodes[i].hash, leaf_hashes[i], SHA256_DIGEST_LENGTH);
		nodes[i].left = NULL;
		nodes[i].right = NULL;
	}

	int current_layer_start = 0; // 当前层在数组里的起始下标
	int current_count = n;       // 当前层的节点数
	int next_free_idx = n;       // 下一个新父节点在数组里存放的位置

	while (current_count > 1) {
		int next_count = 0;

		for (int i = 0; i < current_count; i += 2) {
			Node *left = &nodes[current_layer_start + i];
			Node *right = (i + 1 < current_count) ? &nodes[current_layer_start + i + 1] : left;

			Node *parent = &nodes[next_free_idx++];
			parent->left = left;
			parent->right = right;
			if(node_with_args( parent->hash, left->hash, right->hash)<0){
				LOG_ERROR("Error while processing bind hash.");
			}

			next_count++;
		}

		current_layer_start += current_count;
		current_count = next_count;
	}

	return &nodes[next_free_idx - 1];//root节点
}

int node_default(unsigned char *block, size_t data_size,unsigned char *node)
{
	if (!block) return EINVAL;
	if (sha256(block, data_size,node,SHA256_DIGEST_LENGTH) < 0) {
		return -1;
	}
	return 0;
}

int node_with_args(unsigned char *parents,unsigned char *left,unsigned char *right){
	unsigned char binded[SHA256_DIGEST_LENGTH*2];
	memcpy(binded,left,SHA256_DIGEST_LENGTH);
	memcpy(binded+SHA256_DIGEST_LENGTH,right,SHA256_DIGEST_LENGTH);
	if (sha256(binded,sizeof(binded),parents,SHA256_DIGEST_LENGTH) < 0) {
		return -1;
	}
	return 0;
}

int sha256(const unsigned char *block,size_t data_size, unsigned char *hash,int hash_length)
{
	if (data_size > 2 * BLOCK_SZ) {
		LOG_ERROR("Data block provided is too large.");
		return EINVAL;
	}
	if (hash_length!=SHA256_DIGEST_LENGTH||block==NULL||hash==NULL)return EINVAL;

	EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(md_ctx, EVP_sha256(), NULL);
	EVP_DigestUpdate(md_ctx, block, data_size);
	if (1 != EVP_DigestFinal_ex(md_ctx, hash, NULL)) {
		LOG_ERROR("Error occurred while doing sha256 hash.Usually due "
			  "to the wrong input");
		return EINVAL;
	}

	EVP_MD_CTX_free(md_ctx);

	return 0;
}

void bytes_to_hex(const unsigned char *bin, unsigned int bin_len, char *hex)
{
	static const char hex_digits[] = "0123456789abcdef";

	for (unsigned int i = 0; i < bin_len; i++) {
		hex[i * 2] = hex_digits[(bin[i] >> 4) & 0x0F];
		hex[i * 2 + 1] = hex_digits[bin[i] & 0x0F];
	}
	hex[bin_len * 2] = '\0';
}