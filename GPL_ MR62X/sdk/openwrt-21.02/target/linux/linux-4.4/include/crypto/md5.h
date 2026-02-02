#ifndef _CRYPTO_MD5_H
#define _CRYPTO_MD5_H

#include <linux/types.h>
#ifdef CONFIG_CRYPTO_DEV_REALTEK
#include "../../drivers/crypto/realtek/rtl_crypto_helper.h"
#endif // CONFIG_CRYPTO_DEV_REALTEK

#define MD5_DIGEST_SIZE		16
#define MD5_HMAC_BLOCK_SIZE	64
#define MD5_BLOCK_WORDS		16
#define MD5_HASH_WORDS		4

#define MD5_H0	0x67452301UL
#define MD5_H1	0xefcdab89UL
#define MD5_H2	0x98badcfeUL
#define MD5_H3	0x10325476UL

struct md5_state {
	u32 hash[MD5_HASH_WORDS];
	u32 block[MD5_BLOCK_WORDS];
	u64 byte_count;
#ifdef CONFIG_CRYPTO_DEV_REALTEK
	struct rtl_hash_ctx rtl_ctx;
#endif
};

#endif
