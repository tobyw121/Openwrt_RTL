#ifndef	_WAPI_CRYPTO_H_
#define	_WAPI_CRYPTO_H_




typedef struct
{
    unsigned long total[2];     /*!< number of bytes processed  */
    unsigned long state[8];     /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
    int is224;                  /*!< 0 => SHA-256, else SHA-224 */
}
sha2_context;





void sha2_starts( sha2_context *ctx, int is224 );

void sha2_update( sha2_context *ctx, unsigned char *input, int ilen );

void sha2_finish( sha2_context *ctx, unsigned char output[32] );

void sha2( unsigned char *input, int ilen, unsigned char output[32], int is224 );

void sha2_hmac_starts( sha2_context *ctx, unsigned char *key, int keylen, int is224 );

void sha2_hmac_update( sha2_context *ctx, unsigned char *input, int ilen );

void sha2_hmac_finish( sha2_context *ctx, unsigned char output[32] );

void sha2_hmac( unsigned char *key, int keylen,
                unsigned char *input, int ilen,
                unsigned char output[32], int is224 );

void sha256_hmac( unsigned char *key, int keylen,
				unsigned char *input, int ilen,
				unsigned char *output, int hlen);

void KD_hmac_sha256(unsigned char *input, int ilen,
					unsigned char *key, int keylen,
					unsigned char *output, int hlen);






































#endif





















