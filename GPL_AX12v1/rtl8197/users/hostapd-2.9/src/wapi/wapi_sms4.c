#include "sms4_const.h"


#include <openssl/modes.h>
#include <modes/modes_lcl.h>

#define ENCRYPT  0     
#define DECRYPT  1 


void SMS4Crypt(unsigned char *Input, unsigned char *Output, unsigned int *rk)
{
	 unsigned int r, mid, x0, x1, x2, x3, *p;
	 p = (unsigned int *)Input;
	 x0 = p[0];
	 x1 = p[1];
	 x2 = p[2];
	 x3 = p[3];
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0x00FF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0x00FF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0x00FF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0x00FF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);

	 for (r = 0; r < 32; r += 4)
	 {
		  mid = x1 ^ x2 ^ x3 ^ rk[r + 0];
		  mid = ByteSub(mid);
		  x0 ^= L1(mid);
		  mid = x2 ^ x3 ^ x0 ^ rk[r + 1];
		  mid = ByteSub(mid);
		  x1 ^= L1(mid);
		  mid = x3 ^ x0 ^ x1 ^ rk[r + 2];
		  mid = ByteSub(mid);
		  x2 ^= L1(mid);
		  mid = x0 ^ x1 ^ x2 ^ rk[r + 3];
		  mid = ByteSub(mid);
		  x3 ^= L1(mid);
	 }
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0x00FF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0x00FF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0x00FF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0x00FF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
	 p = (unsigned int *)Output;
	 p[0] = x3;
	 p[1] = x2;
	 p[2] = x1;
	 p[3] = x0;
}

void SMS4KeyExt(unsigned char *Key, unsigned int *rk, unsigned int CryptFlag)
{
	 unsigned int r, mid, x0, x1, x2, x3, *p;

	CryptFlag = CryptFlag;
	 
	 p = (unsigned int *)Key;
	 x0 = p[0];
	 x1 = p[1];
	 x2 = p[2];
	 x3 = p[3];
	 x0 = Rotl(x0, 16); x0 = ((x0 & 0xFF00FF) << 8) ^ ((x0 & 0xFF00FF00) >> 8);
	 x1 = Rotl(x1, 16); x1 = ((x1 & 0xFF00FF) << 8) ^ ((x1 & 0xFF00FF00) >> 8);
	 x2 = Rotl(x2, 16); x2 = ((x2 & 0xFF00FF) << 8) ^ ((x2 & 0xFF00FF00) >> 8);
	 x3 = Rotl(x3, 16); x3 = ((x3 & 0xFF00FF) << 8) ^ ((x3 & 0xFF00FF00) >> 8);
	 x0 ^= 0xa3b1bac6;
	 x1 ^= 0x56aa3350;
	 x2 ^= 0x677d9197;
	 x3 ^= 0xb27022dc;
	 for (r = 0; r < 32; r += 4)
	 {
		  mid = x1 ^ x2 ^ x3 ^ CK[r + 0];
		#if 0  
		  printf("mid1[%d]=%04x\n", r,mid);
		  printf("ByteSub(%d) (Sbox[(%d) >> 24 & 0xFF] << 24 ^ Sbox[(%d) >> 16 & 0xFF] << 16 ^ Sbox[(%d) >>  8 & 0xFF] <<  8 ^ Sbox[(%d) & 0xFF]\n",ByteSub(mid), mid, mid, mid, mid);
		#endif
		  mid = ByteSub(mid);
		  rk[r + 0] = x0 ^= L2(mid);
		  mid = x2 ^ x3 ^ x0 ^ CK[r + 1];
		  mid = ByteSub(mid);
		  rk[r + 1] = x1 ^= L2(mid);
		  mid = x3 ^ x0 ^ x1 ^ CK[r + 2];
		  mid = ByteSub(mid);
		  rk[r + 2] = x2 ^= L2(mid);
		  mid = x0 ^ x1 ^ x2 ^ CK[r + 3];
		  mid = ByteSub(mid);
		  rk[r + 3] = x3 ^= L2(mid);
	 }
}



static uint32_t load_u32_be(const uint8_t *b, uint32_t n)
{
    return ((uint32_t)b[4 * n] << 24) |
           ((uint32_t)b[4 * n + 1] << 16) |
           ((uint32_t)b[4 * n + 2] << 8) |
           ((uint32_t)b[4 * n + 3]);
}

static void store_u32_be(uint32_t v, uint8_t *b)
{
    b[0] = (uint8_t)(v >> 24);
    b[1] = (uint8_t)(v >> 16);
    b[2] = (uint8_t)(v >> 8);
    b[3] = (uint8_t)(v);
}


static uint32_t SM4_T_slow(uint32_t X)
{
    uint32_t t = 0;

    t |= ((uint32_t)Sbox[(uint8_t)(X >> 24)]) << 24;
    t |= ((uint32_t)Sbox[(uint8_t)(X >> 16)]) << 16;
    t |= ((uint32_t)Sbox[(uint8_t)(X >> 8)]) << 8;
    t |= Sbox[(uint8_t)X];

    /*
     * L linear transform
     */
    return t ^ Rotl(t, 2) ^ Rotl(t, 10) ^ Rotl(t, 18) ^ Rotl(t, 24);
}

static uint32_t SM4_T(uint32_t X)
{
    return SM4_SBOX_T[(uint8_t)(X >> 24)] ^
           Rotl(SM4_SBOX_T[(uint8_t)(X >> 16)], 24) ^
           Rotl(SM4_SBOX_T[(uint8_t)(X >> 8)], 16) ^
           Rotl(SM4_SBOX_T[(uint8_t)X], 8);
}



int sm4_set_key(const uint8_t *key, SM4_KEY *ks)
{
    /*
     * Family Key
     */
    static const uint32_t FK[4] =
        { 0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc };

    /*
     * Constant Key
     */
    static const uint32_t CK[32] = {
        0x00070E15, 0x1C232A31, 0x383F464D, 0x545B6269,
        0x70777E85, 0x8C939AA1, 0xA8AFB6BD, 0xC4CBD2D9,
        0xE0E7EEF5, 0xFC030A11, 0x181F262D, 0x343B4249,
        0x50575E65, 0x6C737A81, 0x888F969D, 0xA4ABB2B9,
        0xC0C7CED5, 0xDCE3EAF1, 0xF8FF060D, 0x141B2229,
        0x30373E45, 0x4C535A61, 0x686F767D, 0x848B9299,
        0xA0A7AEB5, 0xBCC3CAD1, 0xD8DFE6ED, 0xF4FB0209,
        0x10171E25, 0x2C333A41, 0x484F565D, 0x646B7279
    };

    uint32_t K[4];
    int i;

    K[0] = load_u32_be(key, 0) ^ FK[0];
    K[1] = load_u32_be(key, 1) ^ FK[1];
    K[2] = load_u32_be(key, 2) ^ FK[2];
    K[3] = load_u32_be(key, 3) ^ FK[3];

    for (i = 0; i != SM4_KEY_SCHEDULE; ++i) {
        uint32_t X = K[(i + 1) % 4] ^ K[(i + 2) % 4] ^ K[(i + 3) % 4] ^ CK[i];
        uint32_t t = 0;

        t |= ((uint32_t)Sbox[(uint8_t)(X >> 24)]) << 24;
        t |= ((uint32_t)Sbox[(uint8_t)(X >> 16)]) << 16;
        t |= ((uint32_t)Sbox[(uint8_t)(X >> 8)]) << 8;
        t |= Sbox[(uint8_t)X];

        t = t ^ Rotl(t, 13) ^ Rotl(t, 23);
        K[i % 4] ^= t;
        ks->rk[i] = K[i % 4];
    }

    return 1;
}

#define SM4_RNDS(k0, k1, k2, k3, F)          \
      do {                                   \
         B0 ^= F(B1 ^ B2 ^ B3 ^ ks->rk[k0]); \
         B1 ^= F(B0 ^ B2 ^ B3 ^ ks->rk[k1]); \
         B2 ^= F(B0 ^ B1 ^ B3 ^ ks->rk[k2]); \
         B3 ^= F(B0 ^ B1 ^ B2 ^ ks->rk[k3]); \
      } while(0)


void sm4_encrypt(const uint8_t *in, uint8_t *out, const SM4_KEY *ks)
{
    uint32_t B0 = load_u32_be(in, 0);
    uint32_t B1 = load_u32_be(in, 1);
    uint32_t B2 = load_u32_be(in, 2);
    uint32_t B3 = load_u32_be(in, 3);

    /*
     * Uses byte-wise sbox in the first and last rounds to provide some
     * protection from cache based side channels.
     */
    SM4_RNDS( 0,  1,  2,  3, SM4_T_slow);
    SM4_RNDS( 4,  5,  6,  7, SM4_T);
    SM4_RNDS( 8,  9, 10, 11, SM4_T);
    SM4_RNDS(12, 13, 14, 15, SM4_T);
    SM4_RNDS(16, 17, 18, 19, SM4_T);
    SM4_RNDS(20, 21, 22, 23, SM4_T);
    SM4_RNDS(24, 25, 26, 27, SM4_T);
    SM4_RNDS(28, 29, 30, 31, SM4_T_slow);

    store_u32_be(B3, out);
    store_u32_be(B2, out + 4);
    store_u32_be(B1, out + 8);
    store_u32_be(B0, out + 12);
}

void sm4_gcm_encrypt(unsigned char *in, int in_len, unsigned char *out,
						    unsigned char *key, unsigned char *iv, int iv_len, 
							unsigned char *aad, int aad_len, unsigned char *auth_tag, int tag_len)
{
	GCM128_CONTEXT ctx;
	SM4_KEY ks;

	sm4_set_key((const unsigned char *)key, &ks);

	CRYPTO_gcm128_init(&ctx, &ks, (block128_f)sm4_encrypt);
	CRYPTO_gcm128_setiv(&ctx, iv, iv_len);

	if (aad)
		CRYPTO_gcm128_aad(&ctx, aad, aad_len);

	CRYPTO_gcm128_encrypt(&ctx, in, out, in_len);
	rtl_dump_buffer(out, in_len, "encrypt result:");

	if (auth_tag) {
		CRYPTO_gcm128_tag(&ctx, auth_tag, tag_len);
		rtl_dump_buffer(auth_tag, tag_len, "encrypt auth tag:");
	}
}

							

void sm4_gcm_decrypt(unsigned char *in, int in_len, unsigned char *out,
						    unsigned char *key, unsigned char *iv, int iv_len, 
							unsigned char *aad, int aad_len, unsigned char *auth_tag, int tag_len)
{
	GCM128_CONTEXT ctx;
	SM4_KEY ks;

	sm4_set_key((const unsigned char *)key, &ks);

	CRYPTO_gcm128_init(&ctx, &ks, (block128_f)sm4_encrypt);
	CRYPTO_gcm128_setiv(&ctx, iv, iv_len);

	if (aad)
		CRYPTO_gcm128_aad(&ctx, aad, aad_len);

	CRYPTO_gcm128_decrypt(&ctx, in, out, in_len);
	rtl_dump_buffer(out, in_len, "decrypt result:");

	if (auth_tag) {
		CRYPTO_gcm128_tag(&ctx, auth_tag, tag_len);
		rtl_dump_buffer(auth_tag, tag_len, "decrypt auth tag:");
	}
}


//test sm4_gcm_encrypt() and sm4_gcm_decrypt()
//test case from GCM-SM4 SPEC.
void test_sm4_gcm(void)
{	
#if 0
	//case 3
	unsigned char data[36] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
							  0x00, 0x0C, 0x43, 0x35, 0xA1, 0xDD, 0x0A, 0x0A, 0x0A, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							  0x0A, 0x0A, 0x0A, 0xFE};	
	unsigned char key[16] = {0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08};
	unsigned char iv[12] = {0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x36};	
	unsigned char aad[32] = {0x08, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x0C, 0x43, 0x35, 0xA1, 0xDD, 0x00, 0x00,
						     0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x24};

	unsigned char out[36] = {0};
	unsigned char auth_tag[16] = {0};
#endif
		
	//case 8
	unsigned char data[36] = {0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x02,
							  0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x0A, 0x0A, 0x0A, 0xFE, 0x00, 0x0C, 0x43, 0x35, 0xA1, 0xDD,
							  0x0A, 0x0A, 0x0A, 0x67};
	unsigned char key[16] = {0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5, 0x5A, 0xA5};
	unsigned char iv[12] = {0x5C, 0x36, 0x5C, 0x36, 0x5C, 0x66, 0x5C, 0x36, 0x50, 0x41, 0x52, 0x32};
	unsigned char aad[34] = {0x88, 0x43, 0x00, 0x0C, 0x43, 0x35, 0xA1, 0xDD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00,
							 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x22, 0x33, 0x44, 0x55, 0x66, 0x05, 0x00, 0x00, 0x00,
							 0x00, 0x24};

	unsigned char out[36] = {0};
	unsigned char auth_tag[16] = {0};	

	sm4_gcm_encrypt(data, sizeof(data), out,
					key, iv, sizeof(iv), 
					aad, sizeof(aad), auth_tag, sizeof(auth_tag));

	sm4_gcm_decrypt(out, sizeof(out), data,
					key, iv, sizeof(iv),
					aad, sizeof(aad), auth_tag, sizeof(auth_tag));
}

