
#ifndef _WAPI_ALOGRITHM_H
#define _WAPI_ALOGRITHM_H


int x509_ecc_verify(const unsigned char *pub_s, int pub_sl, unsigned char *in ,  int in_len, unsigned char *sign,int sign_len);
int x509_ecc_sign(const unsigned char *priv_s, int priv_sl, const unsigned char * in, int in_len, unsigned char *out);
int x509_ecc_verify_key(const unsigned char *pub_s, int pub_sl, const unsigned char *priv_s, int priv_sl);

void update_gnonce(unsigned long *gnonce, int type);
int overflow(unsigned long * gnonce);
void add(unsigned long *a, unsigned long b, unsigned short len);
void get_random(unsigned char *buffer, int len);
int mhash_sha256(unsigned char *data, unsigned length, unsigned char *digest);
void KD_hmac_sha256(unsigned char *text,unsigned text_len,unsigned char *key,
					unsigned key_len, unsigned char  *output,unsigned length);
int wapi_hmac_sha256(unsigned char *text, int text_len, 
				unsigned char *key, unsigned key_len,
				unsigned char *digest, unsigned digest_length);
int sms4_encrypt(unsigned char * pofbiv_in,
				unsigned char * pbw_in,
				unsigned int plbw_in,
				unsigned char * pkey,
				unsigned char * pcw_out);

void sm4_gcm_encrypt(unsigned char *in, int in_len, unsigned char *out,
						    unsigned char *key, unsigned char *iv, int iv_len, 
							unsigned char *aad, int aad_len, unsigned char *auth_tag, int tag_len);


void sm4_gcm_decrypt(unsigned char *in, int in_len, unsigned char *out,
						    unsigned char *key, unsigned char *iv, int iv_len, 
							unsigned char *aad, int aad_len, unsigned char *auth_tag, int tag_len);

void test_sm4_gcm(void);

#endif
