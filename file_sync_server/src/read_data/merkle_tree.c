//
// Created by sunlit on 2026/9/29.
//
#include "merkle_tree.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <errno.h>
#include <string.h>

#define FILE_BLOCK_SZ 32768
#define SHA256_DIGEST_LENGTH 32


void node_default(Node *self)
{
	sha256_in_stream(&data_in,&self->hash,FILE_BLOCK_SZ);
}

void node_with_args(Node *self,char hash[],char[] *nd_left,char *nd_right)
{
	strcat(nd_left,nd_right);
	sha256_in_stream(&nd_left,&self->hash,strlen(nd_left),NULL);
}

int sha256_in_stream(
	const char *data_in,
	unsigned char *hash,
	size_t data_size,
	ERR_INFO *err)
{
	if (data_size>2*FILE_BLOCK_SZ)
	{
		hld_err(&err,"The FILE_BLOCK_SZ you passed is out-limited.Normally we suggest the data package you passed within 256kb.",EINVAL);
		return -1;
	}

	unsigned int hash_len;

	EVP_MD_CTX *md_ctx=EVP_MD_CTX_new();
	EVP_DigestInit_ex(md_ctx,EVP_sha256(),NULL);
	EVP_DigestUpdate(md_ctx,data_in,data_size);
	if(1!=EVP_DigestFinal_ex(md_ctx,hash,&hash_len))
	{
		hld_err(&err,"Error occurs while handle hash.",EIO);
	}

	EVP_MD_CTX_free(md_ctx);
	return 0;
}
