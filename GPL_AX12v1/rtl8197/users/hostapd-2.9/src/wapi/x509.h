#ifndef _X509_H_
#define _X509_H_

#include "wapi_ae.h"
#include "wapi_alogrithm.h"

#define V_X509_V3	2

#define V_ASN1_UNIVERSAL		0x00
#define V_ASN1_PRIVATE			0xc0

#define V_ASN1_CONSTRUCTED		0x20
#define V_ASN1_PRIMITIVE_TAG		0x1f

#define V_ASN1_BIT_STRING		3
#define V_ASN1_OCTET_STRING 	4
#define V_ASN1_NULL 		5
#define V_ASN1_OBJECT			6
#define V_ASN1_UTF8STRING		12
#define V_ASN1_PRINTABLESTRING		19
#define V_ASN1_UTCTIME			23
#define V_ASN1_GENERALIZEDTIME		24	
#define V_ASN1_UNIVERSALSTRING		28
#define V_ASN1_BMPSTRING		30


#define WAPI_OID_NUMBER 	1

#define MAX_BYTE_DATA_LEN  256
#define COMM_DATA_LEN			2048


#define PACK_ERROR	0xffff



#define SIGN_LEN					48	
#define PUBKEY_LEN					48	
#define PUBKEY2_LEN 					49	
#define SECKEY_LEN					24	
#define DIGEST_LEN					32	
#define HMAC_LEN							20	
#define PRF_OUTKEY_LEN			48

#define X509_CERT_SIGN_LEN		  57  

#define WAPI_ECDSA_OID			"1.2.156.11235.1.1.1"
#define WAPI_ECC_CURVE_OID		"1.2.156.11235.1.1.2.1"
#define ECDSA_ECDH_OID			"1.2.840.10045.2.1"

#define  CERT_OBJ_X509 1
#define X509_TIME_LEN		15

#define PEM_STRING_X509_CERTS		"-----BEGIN CERTIFICATE-----"
#define PEM_STRING2_X509_CERTE		"-----END CERTIFICATE-----"
#define PEM_STRING_PRIKEYS			"-----BEGIN EC PRIVATE KEY-----"
#define PEM_STRING_PRIKEYE			"-----END EC PRIVATE KEY-----"

typedef struct _WOID 
{
	const char* 	pszOIDName;
	unsigned short	usOIDLen;
	unsigned short	usParLen;
	unsigned short	bOID[MAX_BYTE_DATA_LEN];		
	unsigned short	bParameter[MAX_BYTE_DATA_LEN];	
} WOID, *PWOID;

typedef  struct  __private_key
{
unsigned char				   tVersion;
unsigned char				   lVersion;
unsigned char				   vVersion;
unsigned char				   verpad;

unsigned char				   tPrivateKey; 				  
unsigned char				   lPrivateKey; 				  
unsigned char				   prikeypad[2];
unsigned char				   vPrivateKey[MAX_BYTE_DATA_LEN];	 

unsigned char				   tSPrivateKeyAlgorithm;
unsigned char				   lSPrivateKeyAlgorithm;
unsigned char				   tOID;							  
unsigned char				   lOID;						  
unsigned char				   vOID[MAX_BYTE_DATA_LEN]; 		  

unsigned char				   tSPubkey;  
unsigned char				   lSPubkey;  
unsigned char				   tPubkey;    
unsigned char				   lPubkey;    
unsigned char				   vPubkey[MAX_BYTE_DATA_LEN];
}private_key;


int rtl_wapi_cert_verify(tkey *ca_pubkey, cert_id *user_cert);
short unpack_private_key(private_key *p_private_key, const void * buffer, short bufflen);
tkey *rtl_wapi_cert_get_pubkey(cert_id *cert_st);
wapi_data *rtl_wapi_cert_get_subject_name(cert_id *cert_st);
wapi_data *rtl_wapi_cert_get_serial_number(cert_id *cert_st);
wapi_data *rtl_wapi_cert_get_issuer_name(cert_id *cert_st);
int rtl_wapi_cert_get_sign(cert_id *cert_st, unsigned char *out, int out_len);
int rtl_wapi_cert_get_sign_inlen(cert_id *cert_st);

unsigned char *rtl_wapi_cert_get_realinfo(unsigned char *des, const unsigned char *src_cert, int len,
					const  char *start_flag,
					const  char *end_flag);
int rtl_wapi_cert_get_length(unsigned char **pBuf, unsigned char *pMax, unsigned long *pLen);
int rtl_wapi_cert_get_sequence(unsigned char **pBuffer, unsigned char *pMax, int *pClass, int *pTag, unsigned long *pLength, unsigned char *pbIsConstruct);
int rtl_wapi_cert_get_OID(unsigned char **pBuffer, unsigned char *pMax, unsigned char *pszString, unsigned long *pStrLen, unsigned long *pParLen);
int rtl_wapi_cert_get_string(unsigned char **pBuffer, unsigned char *pMax, unsigned char *pszString, unsigned long *pStrLen);
int rtl_wapi_cert_get_Name(unsigned char **pBuffer, unsigned char *pMax, unsigned char *pszString, unsigned long *pStrLen);
unsigned char hlpCheckOIDAndParam(unsigned char *pOID, unsigned long dwOIDLen, unsigned char* pParam, unsigned long dwParamLen, unsigned char bIsPubKey);
int rtl_wapi_cert_get_Validity(unsigned char **pBuffer, unsigned char *pMax);

int rtl_wapi_psk_init(struct wapi_ae_st *wapi_ae, KEY_TYPE kt, u32 kl, u8 *kv);
int rtl_wapi_cert_init(struct wapi_ae_st *wapi_ae, char *ca_cert_file, char *asu_cert_file, char *ae_cert_file);


#endif  
