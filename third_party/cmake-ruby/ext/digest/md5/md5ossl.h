/* $Id: md5ossl.h 46829 2014-07-15 15:43:03Z nobu $ */

#ifndef MD5OSSL_H_INCLUDED
#define MD5OSSL_H_INCLUDED

#include <stddef.h>
#include <openssl/md5.h>

#define MD5_BLOCK_LENGTH	MD5_CBLOCK

void MD5_Finish(MD5_CTX *pctx, unsigned char *digest);

#endif
