/* $Id: md5ossl.c 46829 2014-07-15 15:43:03Z nobu $ */

#include "md5ossl.h"

void
MD5_Finish(MD5_CTX *pctx, unsigned char *digest)
{
    MD5_Final(digest, pctx);
}
