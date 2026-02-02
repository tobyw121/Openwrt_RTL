/* 
 * DES & Triple DES EDE Cipher Algorithms.
 */

#ifndef __CRYPTO_DES_H
#define __CRYPTO_DES_H

#define DES_KEY_SIZE		8
#define DES_EXPKEY_WORDS	32
#define DES_BLOCK_SIZE		8

#define DES3_EDE_KEY_SIZE	(3 * DES_KEY_SIZE)
#define DES3_EDE_EXPKEY_WORDS	(3 * DES_EXPKEY_WORDS)
#define DES3_EDE_BLOCK_SIZE	DES_BLOCK_SIZE


extern unsigned long des_ekey(u32 *pe, const u8 *k);

#if defined(CONFIG_CRYPTO_DEV_REALTEK)
extern int __des3_ede_setkey(struct crypto_tfm *tfm, u32 *expkey, u32 *flags, const u8 *key,
					 unsigned int keylen);
#else
extern int __des3_ede_setkey(u32 *expkey, u32 *flags, const u8 *key,
			     unsigned int keylen);
#endif

#endif /* __CRYPTO_DES_H */
