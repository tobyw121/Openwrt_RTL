#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include "wapi_alogrithm.h"
#include "wapi_ae.h"
#include "wapi_sms4.h"

#define SHA256_DIGEST_SIZE 32
#define SHA256_DATA_SIZE 64
#define _SHA256_DIGEST_LENGTH 8


extern int ecc192_verify(const unsigned char *pub_key, const unsigned char *in,
			  int in_len, const unsigned char *sign, int sign_len);
extern int ecc192_sign(const unsigned char *priv_key, const unsigned char *in,
			int in_len, unsigned char *out);
int overflow(unsigned long *gnonce)
{
	unsigned char flow[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

	if (memcmp(gnonce, flow, 8) == 0)
		return 0;
	else
		return -1;
}

void add(unsigned long *a, unsigned long b, unsigned short len)
{
	int i = 0;
	unsigned long carry = 0;
	unsigned long *a1 = NULL;
	unsigned long ca1 = 0;
	unsigned long ca2 = 0;
	unsigned long ca3 = 0;
	unsigned long bb = b;

	for(i=len - 1; i>=0; i--)                         //·Ö¶Î32bits¼Ó                      
	{	
		a1=a+i; 
		ca1 = (*a1)&0x80000000;	
		ca2 = bb&0x80000000;
		*a1 += bb + carry;
		bb = 0;
		ca3 = (*a1)&0x80000000;	
		if(ca1==0x80000000 && ca2==0x80000000)  carry=1; 
		else if(ca1!=ca2 && ca3==0)  carry=1; 
        	else carry=0;
		a1++;
	}
	
}

void update_gnonce(unsigned long *gnonce, int type)
{
	add(gnonce, type+1, 4);
}

static void smash_random(unsigned char *buffer, int len )
{
	 unsigned char smash_key[32] = {  0x09, 0x1A, 0x09, 0x1A, 0xFF, 0x90, 0x67, 0x90,
									0x7F, 0x48, 0x1B, 0xAF, 0x89, 0x72, 0x52, 0x8B,
									0x35, 0x43, 0x10, 0x13, 0x75, 0x67, 0x95, 0x4E,
									0x77, 0x40, 0xC5, 0x28, 0x63, 0x62, 0x8F, 0x75};
	KD_hmac_sha256(buffer, len, smash_key, 32, buffer, len);
}


void get_random(unsigned char *buffer, int len)
{
	RAND_bytes(buffer, len);
	smash_random(buffer, len);
}


int mhash_sha256(unsigned char *data, unsigned length, unsigned char *digest)
{

	SHA256((const unsigned char *)data, length, digest);
        return 0;
}

int wapi_hmac_sha256(unsigned char *text, int text_len, 
	unsigned char *key, unsigned key_len, unsigned char *digest,
	unsigned digest_length)
{
	unsigned char out[SHA256_DIGEST_SIZE] = {0,};
	HMAC(EVP_sha256(),
			key,
			key_len,
			text,
			text_len,
			out,
			NULL);
	memcpy(digest, out, digest_length);
	return 0;
}

/*KD-HMAC-SHA256Ëã·¨*/
void KD_hmac_sha256(unsigned char *text,unsigned text_len,unsigned char *key,
					unsigned key_len, unsigned char  *output, unsigned length)
{
	unsigned i;
	
	for(i=0;length/SHA256_DIGEST_SIZE;i++,length-=SHA256_DIGEST_SIZE){
		wapi_hmac_sha256(
			text,
			text_len,
			key,
			key_len, 
			&output[i*SHA256_DIGEST_SIZE],
			SHA256_DIGEST_SIZE);
		text=&output[i*SHA256_DIGEST_SIZE];
		text_len=SHA256_DIGEST_SIZE;
	}

	if(length>0)
		wapi_hmac_sha256(
			text,
			text_len,
			key,
			key_len, 
			&output[i*SHA256_DIGEST_SIZE],
			length);

}


int   x509_ecc_verify(const unsigned char *pub_s, int pub_sl, unsigned char *in ,  int in_len, unsigned char *sign,int sign_len)
{
	int ret = 0;
	
	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);
	
	if (pub_s == NULL || pub_sl <= 0 || in == NULL || in_len <= 0 || sign == NULL || sign_len <= 0)
	{
		if(pub_s == NULL)
			rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify: pub_s == NULL\n");
		if(pub_sl <= 0)
			rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify: pub_sl <= 0\n");
		if( in == NULL)
			rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify:	in == NULL\n");
		if( in_len <= 0 )
			rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify:	in_len <= 0 \n");
		if( sign == NULL )
			rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify:	sign == NULL \n");
		if( sign_len <= 0 )
			rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify:	sign_len <= 0 \n");

		return ret;
	}
	else
	{
		rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify:	call  ecc192_verify case \n");
		ret = ecc192_verify(pub_s, in, in_len, sign, sign_len);
		rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify:	after ecc192_verify = %d  \n",ret);

	}

	if (ret <= 0)
		ret = 0;
	else
		ret = 1;

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}


int x509_ecc_sign(const unsigned char *priv_s, int priv_sl, const unsigned char * in, int in_len, unsigned char *out)
{
	priv_sl = priv_sl;/*disable warnning*/
	if (priv_s == NULL || in == NULL || in_len <= 0 || out == NULL)
	{
		return 0;
	}
	else
	{
		return ecc192_sign(priv_s, in, in_len, out);
	}
}

int x509_ecc_verify_key(const unsigned char *pub_s, int pub_sl, const unsigned char *priv_s, int priv_sl)
{
#define EC962_SIGN_LEN		48
	unsigned char data[] = "123456abcd";
	unsigned char sign[EC962_SIGN_LEN+1];
	int ret = 0;


	if (priv_s == NULL || pub_sl <= 0  || priv_s == NULL || priv_sl <= 0)
	{
		rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify_key return exceptionly\n");
		return 0;
	}

	memset(sign, 0, sizeof(sign));
	ret = ecc192_sign(priv_s, data, strlen((char*)data), sign);
	if (ret != EC962_SIGN_LEN)
	{
		rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify_key ecc192_sign  error = %d \n",ret);
		ret = 0;
		return ret;
	}

	rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify_key:after  ecc192_sign %d \n",ret);

	ret = ecc192_verify(pub_s, data, strlen((char*)data), sign, EC962_SIGN_LEN);
	if (ret <= 0)
	{
		rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify_key  ecc192_verify error = %d \n",ret);
		ret = 0;
	}

	rtl_wapi_trace(MSG_DEBUG, "x509_ecc_verify_key:after  ecc192_verify %d \n",ret);

	return ret;

}

int sms4_encrypt(unsigned char * pofbiv_in,unsigned char * pbw_in,unsigned int plbw_in,unsigned char * pkey,unsigned char * pcw_out)
{
	unsigned int ofbtmp[4];
	unsigned int * pint0, * pint1;
	unsigned char * pchar0, * pchar1,* pchar2;
	unsigned int counter,comp,i;
	unsigned int prkey_in[32];


	if(plbw_in<1)	return 1;

	SMS4KeyExt(pkey,  prkey_in, 0);

	counter=plbw_in / 16;
	comp=plbw_in % 16;

	SMS4Crypt(pofbiv_in,(unsigned char *)ofbtmp, prkey_in);
	pint0=(unsigned int *)pbw_in;
	pint1=(unsigned int *)pcw_out;
	for(i=0;i<counter;i++) {
		pint1[0]=pint0[0]^ofbtmp[0];
		pint1[1]=pint0[1]^ofbtmp[1];
		pint1[2]=pint0[2]^ofbtmp[2];
		pint1[3]=pint0[3]^ofbtmp[3];
		SMS4Crypt((unsigned char *)ofbtmp,(unsigned char *)ofbtmp, prkey_in);
		pint0+=4;
		pint1+=4;
	}
	pchar0=(unsigned char *)pint0;
	pchar1=(unsigned char *)pint1;
	pchar2=(unsigned char *)ofbtmp;
	for(i=0;i<comp;i++) {
		pchar1[i]=pchar0[i]^pchar2[i];
	}
	
	return 0;	
}


