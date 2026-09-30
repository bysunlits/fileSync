//
// Created by sunlit on 2026/9/29.
//
#include <string.h>
#include <uchar.h>
#include <openssl/evp.h>
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

int sha256_in_stream(const char *file_in,const char *hash_out)
//XXX:Shoudn't use strlen(), it depends on the "\0" and the stream has
{
	unsigned char buffer[FILE_BLOCK_SZ];
	size_t bytes_read;

	EVP_MD_CTX *md_ctx=EVP_MD_CTX_new();
	EVP_DigestInit_ex(md_ctx,EVP_sha256(),NULL);
	// for (int offset=0;offset<strlen(file_in);)
	// {
	// 	int block=offset+FILE_BLOCK_SZ<strlen(file_in)?FILE_BLOCK_SZ:strlen(file_in)-offset;
	// 	char *current_ptr=file_in+offset;
	// 	EVP_DigestUpdate(md_ctx,current_ptr,block);
	// 	offset+=FILE_BLOCK_SZ;
	// }
	EVP_DigestUpdate(md_ctx,file_in,);
	EVP_DigestFinal_ex(md_ctx,hash_out)
}
