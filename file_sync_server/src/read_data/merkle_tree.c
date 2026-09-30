//
// Created by sunlit on 2026/9/29.
//
#include <string.h>
#include <uchar.h>
#include <openssl/evp.h>
#include  <openssl/sha.h>
#include "base/hld_err.h"
#include <errno.h>

#define FILE_BLOCK_SZ 32768

typedef struct MerkleTree
{
	char32_t root_node;

	void (*left_node)(struct MerkleTree *self);
	void (*right_node)(struct MerkleTree *self);
}MerkleTree;

typedef struct Node
{
	char32_t hash;
	char32_t left;
	char32_t right;
	void (*node_default)();
	void (*node_with_args)(char32_t nd_parent,char32_t nd_left,char32_t nd_right);
}Node;

int sha256_in_stream(const char *data_in,size_t data_size,ERR_INFO *err)
{
	if (data_size>2*FILE_BLOCK_SZ)
	{
		hld_err(err,&"The FILE_BLOCK_SZ you passed is out-limited.",EINVAL);
		return -1;
	}

	unsigned char hash[SHA256_DIGEST_LENGTH];
	unsigned int hash_len;

	EVP_MD_CTX *md_ctx=EVP_MD_CTX_new();
	EVP_DigestInit_ex(md_ctx,EVP_sha256(),NULL);
	EVP_DigestUpdate(md_ctx,data_in,data_size);
	if(1!=EVP_DigestFinal_ex(md_ctx,hash,&hash_len)
	{
		goto cleanup;
	}


cleanup:

}
