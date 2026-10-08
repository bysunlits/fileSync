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


void node_default(Node *self,char *data_in,size_t data_size,ERR_INFO *err)
{
	sha256_in_stream(data_in,self->hash,FILE_BLOCK_SZ,err);
}

void node_with_args(Node *self,char *nd_left,char *nd_right)
{
	strcat(nd_left,nd_right);
	sha256_in_stream(nd_left,self->hash,strlen(nd_left),NULL);
}

int sha256_in_stream(
	const char *data_in,
	char *hash,
	size_t data_size,
	ERR_INFO *err)
{
	if (data_size>2*FILE_BLOCK_SZ)
	{
		hld_err(&err,"The FILE_BLOCK_SZ you passed is out-limited.Normally we suggest the data package you passed within 256kb.",EINVAL);
		return -1;
	}

	unsigned int hash_len;
	unsigned char hash_digest[SHA256_DIGEST_LENGTH];

	EVP_MD_CTX *md_ctx=EVP_MD_CTX_new();
	EVP_DigestInit_ex(md_ctx,EVP_sha256(),NULL);
	EVP_DigestUpdate(md_ctx,data_in,data_size);
	if(1!=EVP_DigestFinal_ex(md_ctx,hash_digest,&hash_len))
	{
		hld_err(&err,"Error occurs while handle hash.",EIO);
	}

	bytes_to_hex(hash_digest,hash_len,hash);

	EVP_MD_CTX_free(md_ctx);


	return 0;
}

void bytes_to_hex(const unsigned char *bin, unsigned int bin_len, char *hex) {
	static const char hex_digits[] = "0123456789abcdef";

	for (unsigned int i = 0; i < bin_len; i++) {
		hex[i * 2]     = hex_digits[(bin[i] >> 4) & 0x0F];
		hex[i * 2 + 1] = hex_digits[bin[i] & 0x0F];
	}
	hex[bin_len * 2] = '\0';
}