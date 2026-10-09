//
// Created by sunlit on 2026/9/29.
//
#include "merkle_tree.h"
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string.h>

#define BLOCK_SZ 32768U
#define SHA256_DIGEST_LENGTH 32

typedef struct Node {
	unsigned char hash[32];
	struct Node *left;
	struct Node *right;
} Node;

void build_tree(MerkleTree *self, unsigned char *data, uint32_t data_size)
{
	int current_level=0;
	int member=0;
	unsigned char *offset=data;
	uint32_t leaf_node_sum=data_size/BLOCK_SZ;
	if (data_size%BLOCK_SZ>0)leaf_node_sum++;
	Node *nodes=(Node *)malloc(sizeof(Node)*2*leaf_node_sum);

	while (member<2) {
		if (member==0) {
			for (uint32_t i = 0; i < data_size; i +=BLOCK_SZ) {
				int block_sz =
					BLOCK_SZ < (data_size - 1) ? BLOCK_SZ : (data_size - i);
				node_default(offset, block_sz);
				offset+=block_sz;
				member++;
				current_level++;
			}
		}
		for ()


	}


}

// Node* deal_hash(Node *hash1,Node *hash2)
// {
// 	Node *node;
// 	if (hash1->right!=NULL) {
// 		node=deal_hash(hash1->right,hash2);
// 	}else {
// 		node=node_with_args(hash1->hash,hash2->hash);
// 		return node;
// 	}
// 	Node *new_node=node_with_args(hash1->hash,node->hash);
// 	return new_node;
// }

int node_default(unsigned char *block, size_t data_size)
{
	if (!block) return EINVAL;

	Node *node = malloc(sizeof(Node));
	if (!node)return EINVAL;

	node->right=NULL;
	node->left=NULL;

	if (sha256(block, data_size,node->hash,SHA256_DIGEST_LENGTH) < 0) {
		LOG_ERROR("Error occurred while doing sha256 hash.");
	}
	return 0;
}

Node* node_with_args(unsigned char *nd_left,unsigned char *nd_right)
{
	if (!nd_left||!nd_right)return NULL;

	Node *node = malloc(sizeof(Node));
	if (!node)return NULL;

	node->right=NULL;
	node->left=NULL;

	unsigned char binded[SHA256_DIGEST_LENGTH*2];
	memcpy(binded,nd_left,SHA256_DIGEST_LENGTH);
	memcpy(binded+SHA256_DIGEST_LENGTH,nd_right,SHA256_DIGEST_LENGTH);

	if (sha256(binded,sizeof(binded),node->hash,SHA256_DIGEST_LENGTH) < 0) {
		LOG_ERROR("Error occurred while doing sha256 hash.");
	}
	return node;
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