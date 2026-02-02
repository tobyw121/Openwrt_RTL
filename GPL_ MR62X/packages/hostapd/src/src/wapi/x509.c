#include "wapi_ae.h"
#include "x509.h"
#include "wapi_alogrithm.h"

static const WOID ECDSAOID[WAPI_OID_NUMBER] = 
{
	/* OIDName			OIDLen		ParLen		OID 	Parameter*/
	{WAPI_ECDSA_OID, 8, 11, {0x2A, 0x81, 0x1C, 0xD7, 0x63, 0x01, 0x01, 0x01}, {0x06, 0x09, 0x2A, 0x81, 0x1C, 0xD7, 0x63, 0x01, 0x01, 0x02, 0x01}}
};

static const WOID ECDHOID[WAPI_OID_NUMBER] = 
{
	{ECDSA_ECDH_OID, 7, 11, {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01}, {0x06, 0x09, 0x2A, 0x81, 0x1C, 0xD7, 0x63, 0x01, 0x01, 0x02, 0x01}}
};


int rtl_wapi_cert_get_length(unsigned char ** pBuffer, unsigned char* pMax, unsigned long* pLen)
{
	unsigned char *p = *pBuffer;
	unsigned long l = 0, n = 0;
	if (*p & 0x80)
	{
		n = *p++ & 0x7f;
		while (n-- && p < pMax)
		{
			l <<= 8;
			l |= *p++;
		}
	}
	else
	{
		l = *p++;
	}
	*pLen = l;
	*pBuffer = p;
	return 0;
}

int rtl_wapi_cert_get_sequence(unsigned char **pBuffer, unsigned char *pMax, int *pClass, int *pTag, unsigned long *pLength, unsigned char *pbIsConstruct)
{
	unsigned char *p = *pBuffer;
	int c, t;
	unsigned char bCt;
	unsigned long tl = 0;
	c = *p & V_ASN1_PRIVATE;
	t = *p & V_ASN1_PRIMITIVE_TAG;
	bCt = *p & V_ASN1_CONSTRUCTED;
	p++;
	if (t == V_ASN1_PRIMITIVE_TAG) 
	{
		t = 0;
		do
		{
			t <<= 7;
			t |= *p & 0x7f;
		} while( (*(p++) & 0x80) && p < pMax);
	}
	if (rtl_wapi_cert_get_length(&p, pMax, &tl))
	{
		return 1;
	}
	*pBuffer = p;
	if (pClass)
	{
		*pClass = c;
	}
	if (pTag)
	{
		*pTag = t;
	}
	if (pbIsConstruct)
	{
		*pbIsConstruct = bCt;
	}
	if (pLength)
	{
		*pLength = tl;
	}
	return 0;
}

int rtl_wapi_cert_get_OID(unsigned char **pBuffer, unsigned char *pMax, unsigned char *pszString, unsigned long *pStrLen, unsigned long *pParLen)
{
	unsigned char *p = *pBuffer, *pbak;
	unsigned long len;
	if (*p != V_ASN1_OBJECT)
	{
		return 1;
	}
	p++;
	if (rtl_wapi_cert_get_length(&p, pMax, &len))
	{
		return 1;
	}
	*pStrLen = len;
	pbak = p;
	if (pParLen)
	{
		p += len;
		if (*p == V_ASN1_NULL)
		{
			*pParLen = 2;
		}
		else
		{
			p++;
			if (rtl_wapi_cert_get_length(&p, pMax, pParLen))
			{
				return 1;
			}
			*pParLen += (unsigned long)(p - pbak - len);
		}
	}
	if (pszString)
	{
		memcpy(pszString, pbak, len);
		*pBuffer = pbak + len;
	}
	return 0;
}

int rtl_wapi_cert_get_string(unsigned char **pBuffer, unsigned char *pMax, unsigned char *pszString, unsigned long *pStrLen)
{
	unsigned char* p = *pBuffer;
	unsigned char type = *p++;
	unsigned long len;
	if (rtl_wapi_cert_get_length(&p, pMax, &len))
	{
		return 1;
	}
	if (pszString == NULL || *pStrLen < len)
	{
		*pStrLen = len;
		return 0;
	}
	switch(type)
	{
	case V_ASN1_UTF8STRING:
		break;
	case V_ASN1_BMPSTRING:
		break;
	case V_ASN1_UNIVERSALSTRING:
		break;
	case V_ASN1_PRINTABLESTRING:
		break;
	case V_ASN1_BIT_STRING:
		break;
	case V_ASN1_OCTET_STRING:
		break;
	default:
		return 1;
		break;
	}
	memcpy(pszString, p, len);
	*pStrLen = len;
	*pBuffer = p+len;
	return 0;
}

int rtl_wapi_cert_get_Name(unsigned char **pBuffer, unsigned char *pMax, unsigned char *pszString, unsigned long *pStrLen)
{
	unsigned char* p = *pBuffer;
	unsigned long allLen;
	if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &allLen, NULL))
	{
		return 1;
	}
	p += allLen;
	allLen = (unsigned long)(p - *pBuffer);
	if (pszString)
	{
		memcpy(pszString, *pBuffer, allLen);
	}
	*pBuffer = p;
	*pStrLen = allLen;
	return 0;
}

int rtl_wapi_cert_get_Validity(unsigned char **pBuffer, unsigned char *pMax)
{
	unsigned char *p = *pBuffer;
	unsigned long len;
	if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
	{
		return 1;
	}
	if (*p != V_ASN1_UTCTIME && *p != V_ASN1_GENERALIZEDTIME)
	{
		return 1;
	}
	if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &len, NULL))
	{
		return 1;
	}

	p += len;
	if (*p != V_ASN1_UTCTIME &&
		*p != V_ASN1_GENERALIZEDTIME)
	{
		return 1;
	}
	if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &len, NULL))
	{
		return 1;
	}
	*pBuffer = p + len;
	return 0;
}

unsigned char hlpCheckOIDAndParam(unsigned char *pOID, unsigned long dwOIDLen, unsigned char* pParam, unsigned long dwParamLen, unsigned char bIsPubKey)
{
	int i;
	const WOID* pWD = bIsPubKey ? &ECDHOID[0] : &ECDSAOID[0];
	if (!pOID && !pParam)
	{
		return 1;
	}

	for(i = 0; i < WAPI_OID_NUMBER; i++)
	{
		if ( (pOID == NULL || (pWD[i].usOIDLen == dwOIDLen &&
						memcmp(pWD[i].bOID, pOID, dwOIDLen) == 0) )
			  &&
			  (pParam == NULL || (pWD[i].usParLen == dwParamLen &&
						 memcmp(pWD[i].bParameter, pParam, dwParamLen) == 0) )
			)
		{
			return 1;
		}
	}
	return 0;
}

short unpack_private_key(private_key *p_private_key, const void * buffer, short bufflen)
{
	short  offset = 0;
	unsigned char  tTotal;
	unsigned char  lTotal;

	memcpy(&tTotal, (unsigned char *)buffer + offset, 1);
	offset ++;

	memcpy(&lTotal, (unsigned char *)buffer + offset, 1);
	offset ++;

	memcpy(&p_private_key->tVersion, (unsigned char *)buffer + offset, 1);
	offset ++;
	memcpy(&p_private_key->lVersion, (unsigned char *)buffer + offset, 1);
	offset ++;
	if (offset + p_private_key->lVersion > bufflen)
		return (short)PACK_ERROR;
	memcpy(&p_private_key->vVersion, (unsigned char *)buffer + offset, p_private_key->lVersion);
	offset += p_private_key->lVersion;

	memcpy(&p_private_key->tPrivateKey, (unsigned char *)buffer + offset, 1);
	offset ++;
	memcpy(&p_private_key->lPrivateKey, (unsigned char *)buffer + offset, 1);
	offset ++;
	if (offset + p_private_key->lPrivateKey > bufflen)
		return (short)PACK_ERROR;
	memcpy(&p_private_key->vPrivateKey, (unsigned char *)buffer + offset, p_private_key->lPrivateKey);
	offset += p_private_key->lPrivateKey;

	memcpy(&p_private_key->tSPrivateKeyAlgorithm, (unsigned char *)buffer + offset, 1);
	offset ++;
	memcpy(&p_private_key->lSPrivateKeyAlgorithm, (unsigned char *)buffer + offset, 1);
	offset ++;

	memcpy(&p_private_key->tOID, (unsigned char *)buffer + offset, 1);
	offset ++;
	memcpy(&p_private_key->lOID, (unsigned char *)buffer + offset, 1);
	offset ++;
	if (offset + p_private_key->lOID > bufflen)
		return (short)PACK_ERROR;
	memcpy(&p_private_key->vOID, (unsigned char *)buffer + offset, p_private_key->lOID);
	offset += p_private_key->lOID;

	memcpy(&p_private_key->tSPubkey, (unsigned char *)buffer + offset, 1);
	offset ++;
	memcpy(&p_private_key->lSPubkey, (unsigned char *)buffer + offset, 1);
	offset ++;

	memcpy(&p_private_key->tPubkey, (unsigned char *)buffer + offset, 1);
	offset ++;
	memcpy(&p_private_key->lPubkey, (unsigned char *)buffer + offset, 1);
	offset ++;
	if (offset + p_private_key->lPubkey > bufflen)
		return (short)PACK_ERROR;
	memcpy(&p_private_key->vPubkey, (unsigned char *)buffer + offset, p_private_key->lPubkey);
	offset += p_private_key->lPubkey;

	return offset;
}

int ParsePubKey(unsigned char **pBuffer, unsigned char *pMax, unsigned char *pPubKey, unsigned long *pLen)
{
#define TMP_BUF 100
	unsigned char* p = *pBuffer;
	unsigned long len = 0, ParLen;
	unsigned char pTmp[TMP_BUF] = {0};
	if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
	{
		return 1;
	}
	if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &len, NULL))
	{
		return 1;
	}
	len = TMP_BUF;
	if (rtl_wapi_cert_get_OID(&p, pMax, pTmp, &len, &ParLen))
	{
		return 1;
	}
	if (hlpCheckOIDAndParam(pTmp, len, p, ParLen, 1))
	{
		return 1;
	}
	p += ParLen;   
	if ((*p & V_ASN1_PRIMITIVE_TAG) != V_ASN1_BIT_STRING)
	{
		return 1;
	}
	if (rtl_wapi_cert_get_string(&p, pMax, NULL, &len) || len != PUBKEY_LEN + 1 + 1)
	{
		return 1;
	}
	if (pPubKey == NULL || *pLen < len - 1)
	{
		*pLen = len;
		return 0;
	}
	if (rtl_wapi_cert_get_string(&p, pMax, pTmp, &len))
	{
		return 1;
	}
	if (pTmp[1] != 0x04 || pTmp[0] != 0)
	{
		return 1;
	}
	/* copy unusedbits */
	memcpy(&pPubKey[0], &pTmp[1], PUBKEY2_LEN);
	*pBuffer = p;
	*pLen = len - 1;
	return 0;
}

unsigned char GetBase64Value(unsigned char ch)
{
	if ((ch >= 'A') && (ch <= 'Z')) 
		return ch - 'A'; 
	if ((ch >= 'a') && (ch <= 'z')) 
		return ch - 'a' + 26; 
	if ((ch >= '0') && (ch <= '9')) 
		return ch - '0' + 52; 
	switch (ch) 
	{ 
	case '+': 
		return 62; 
	case '/': 
		return 63; 
	case '=': /* base64 padding */ 
		return 0; 
	default: 
		return 0; 
	} 
}


int Base64Dec(unsigned char *buf,const unsigned char*text,int size)
{
	unsigned char chunk[4];
	int parsenum=0;
	unsigned char* p = buf;

	if(size%4)
		return -1;

	while(size>0)
	{
		chunk[0] = GetBase64Value(text[0]); 
		chunk[1] = GetBase64Value(text[1]); 
		chunk[2] = GetBase64Value(text[2]); 
		chunk[3] = GetBase64Value(text[3]); 
		
		*buf++ = (chunk[0] << 2) | (chunk[1] >> 4); 
		*buf++ = (chunk[1] << 4) | (chunk[2] >> 2); 
		*buf++ = (chunk[2] << 6) | (chunk[3]);
		
		text+=4;
		size-=4;
		parsenum+=3;
	}

	if (0x30 == p[0])
	{
		if (0x82 == p[1])
		{
			parsenum = (p[2]<<8) + p[3] + 4;
		}
		else
		{
			parsenum = p[1] + 2;
		}
	}

	return parsenum;
}


static int getchartype_base64(unsigned char b)
{
	if (	(b>='A'&&b<='Z')
		||	(b>='a'&&b<='z')
		||	(b>='0'&&b<='9')
		||	'+'==b || '/'==b || '='==b)
	{
		return 0;
	}
	else if ('\r'==b || '\n'==b)
	{
		return 1;
	}
	return -1;
}

/*find mark in src*/
static const unsigned char* findmark_mem(const unsigned char* src, int lsrc, const	char* mark, int lmark)
{
	const unsigned char* p = src;
	const unsigned char* pe = src+lsrc;
	if (NULL==src || NULL==mark || lsrc<0 || lmark<0 || lsrc<lmark)
	{
		return NULL;
	}
	pe -= lmark;
	for (; p<=pe; p++)
	{
		if (0 == memcmp(p, mark, lmark))
		{
			return p;
		}
	}
	return NULL;
}

unsigned char *rtl_wapi_cert_get_realinfo(unsigned char *des, const unsigned char *src_cert, int len, const char *start_flag, const char *end_flag)
{
	const u8 *p = src_cert;
	const u8 *ps  = NULL;
	const u8 *pe  = NULL;
	int l0 = strlen((const char*)start_flag);
	int l1 = strlen((const char*)end_flag);
	int c = 0;
	
	if (src_cert == NULL || start_flag == NULL || end_flag == NULL)
	{
		return NULL;
	}
	
	ps = findmark_mem(p, len, start_flag, l0);
	pe = findmark_mem(p, len, end_flag, l1);
	if (NULL==ps || NULL==pe || ps>=pe)
	{
		return NULL;
	}
	
	for (p=ps+l0; p<pe; p++)
	{
		int t = getchartype_base64(*p);
		if (0 == t)
		{
			des[c++] = *p;
		}
	}
	return des;
}

tkey *rtl_wapi_cert_get_pubkey(cert_id *cert_st)
{
	unsigned long tmp;
	unsigned long parLen = 0;
	unsigned char tmpOID[100];
	unsigned char *p = NULL;
	unsigned char *pMax = NULL;
	unsigned char *pBAK = NULL;
	tkey *ret = NULL;
	int fail_flag = 0;

	if (!cert_st)
		return ret;

	p = (unsigned char *)cert_st->data;
	if (!p)
		return ret;
	
	pMax = p + cert_st->length;

	ret = (tkey*)rtl_allocate_buffer(sizeof(tkey));
	if (!ret)
		return ret;

	do
	{
		/* tbsCertificate */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL)) {
			fail_flag = 1;
			break;
		}

		/* version */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL)) {
			fail_flag = 1;
			break;
		}

		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL)) {
			fail_flag = 1;
			break;
		}

		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL)) {
			fail_flag = 1;
			break;
		}

		if (*p != V_X509_V3) {	/* only support V3 */
			fail_flag = 1;
			break;
		}
		p += tmp;
		/* sn */
		{

			pBAK = p;
			if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL)) {
				fail_flag = 1;
				break;
			}

			p += tmp;
			if ((unsigned long)(p - pBAK) > 0xff) {
				fail_flag = 1;
				break;
			}
		}

		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL)) {
			fail_flag = 1;
			break;
		}

		tmp = sizeof(tmpOID);
		if (rtl_wapi_cert_get_OID(&p, pMax, tmpOID, &tmp, &parLen)) {
			fail_flag = 1;
			break;
		}

		if (p + parLen > pMax) {
			fail_flag = 1;
			break;
		}

		if (hlpCheckOIDAndParam(tmpOID, tmp, NULL, 0, 0)) {
			fail_flag = 1;
			break;
		}

		p += parLen;
		/* Issuer */
		if (rtl_wapi_cert_get_Name(&p, pMax, NULL, &tmp)) {
			fail_flag = 1;
			break;
		}

		/* validity */
		if (rtl_wapi_cert_get_Validity(&p, pMax)) {
			fail_flag = 1;
			break;
		}

		/* subject */
		if (rtl_wapi_cert_get_Name(&p, pMax, NULL, &tmp)) {
			fail_flag = 1;
			break;
		}

		/* pubkey info */
		tmp = sizeof(ret->data);
		if (ParsePubKey(&p, pMax, ret->data, &tmp)) {
			rtl_wapi_trace(MSG_DEBUG,  "rtl_wapi_cert_get_pubkey: '%s', '%d' ", __FILE__, __LINE__);
			fail_flag = 1;
			break;
		}
		ret->length = (unsigned short)tmp;

		//rtl_wapi_trace(MSG_DEBUG, "rtl_wapi_cert_get_pubkey: tmp = '%d', '%s', '%d' ", tmp, __FILE__, __LINE__);
	} while(0);

	if (fail_flag)
		ret = (tkey *)rtl_free_buffer(ret, sizeof(tkey));
	
	return ret;
}

wapi_data *rtl_wapi_cert_get_subject_name(cert_id *cert_st)
{
	unsigned long tmp;
	unsigned long parLen = 0;
	unsigned char tmpOID[100];
	unsigned char *p = NULL;
	unsigned char *pMax = NULL;
	unsigned char *pBAK = NULL;
	wapi_data *ret = NULL;
	int fail_flag  = 0;

	if (cert_st == NULL)
	{
		return ret;
	}

	p = cert_st->data;
	if (p == NULL)
		return ret;
	pMax = p + cert_st->length;

	ret =(wapi_data *) rtl_allocate_buffer(sizeof(wapi_data));
	if (ret == NULL)
	{
		return ret;
	}
  
	do
	{
		/* tbsCertificate */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		/* version */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (*p != V_X509_V3)	/* only support V3 */
		{
			fail_flag = 1;
			break;
		}
		p += tmp;
		/* SN */
		{
			pBAK = p;
			if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
			{
				fail_flag = 1;
				break;
			}
			p += tmp;
			if ((unsigned long)(p - pBAK) > 0xff)
			{
				fail_flag = 1;
				break;
			}
		}

		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		tmp = sizeof(tmpOID);
		if (rtl_wapi_cert_get_OID(&p, pMax, tmpOID, &tmp, &parLen))
		{
			fail_flag = 1;
			break;
		}
		if (p + parLen > pMax)
		{
			fail_flag = 1;
			break;
		}
		if (hlpCheckOIDAndParam(tmpOID, tmp, NULL, 0, 0))
		{
			fail_flag = 1;
			break;
		}

		p += parLen;
		/* Issuer */
		tmp = sizeof(ret->data);
		if (rtl_wapi_cert_get_Name(&p, pMax, NULL, &tmp))
		{
			fail_flag = 1;
			break;
		}
		/* validity */
		if (rtl_wapi_cert_get_Validity(&p, pMax))
		{
			fail_flag = 1;
			break;
		}
		/* subject */
		tmp = sizeof(ret->data);
		if (rtl_wapi_cert_get_Name(&p, pMax, ret->data, &tmp))
		{
			fail_flag = 1;
			break;
		}
		ret->length = (unsigned char)tmp;
		
	} while(0);

	if (fail_flag)
	{
		ret = (wapi_data *)rtl_free_buffer(ret, sizeof(wapi_data));
	}
	return ret;

}

wapi_data *rtl_wapi_cert_get_serial_number(cert_id *cert_st)
{
	unsigned long tmp;
	unsigned char *p = NULL;
	unsigned char *pMax = NULL;
	unsigned char *pBAK = NULL;
	wapi_data *ret = NULL;
	int fail_flag  = 0;

	if (cert_st == NULL)
	{
		return (void *)ret;
	}

	p = cert_st->data;
	if (p == NULL)
	return ret;
	pMax = p + cert_st->length;

	ret = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (ret == NULL)
	{
		return ret;
	}
  
	do
	{
		/* tbsCertificate */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		/* version */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (*p != V_X509_V3)	/* only support V3 */
		{
			fail_flag = 1;
			break;
		}
		p += tmp;
		{
			pBAK = p;
			if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
			{
				fail_flag = 1;
				break;
			}
			p += tmp;
			if ((unsigned long)(p - pBAK) > 0xff)
			{
				fail_flag = 1;
				break;
			}
			memcpy(ret->data, pBAK, p - pBAK);
			ret->length = (unsigned char)(p - pBAK);
		}
	} while(0);

	if (fail_flag)
	{
		ret =(wapi_data *) rtl_free_buffer(ret, sizeof(wapi_data));
	}
	return ret;
}

wapi_data *rtl_wapi_cert_get_issuer_name(cert_id *cert_st)
{
	unsigned long tmp;
	unsigned long parLen = 0;
	unsigned char tmpOID[100];
	unsigned char *p	   = NULL;
	unsigned char *pMax = NULL;
	unsigned char *pBAK = NULL;
	wapi_data *ret = NULL;
	int fail_flag  = 0;

	if (cert_st == NULL)
	{
		return ret;
	}

	p = cert_st->data;
	if (p == NULL)
	return ret;
	pMax = p + cert_st->length;
	
	ret = (wapi_data *)rtl_allocate_buffer(sizeof(wapi_data));
	if (ret == NULL)
	{
		return ret;
	}
	do
	{
		/* tbsCertificate */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		/* version */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		{
			fail_flag = 1;
			break;
		}
		if (*p != V_X509_V3)	/* only support V3 */
		{
			fail_flag = 1;
			break;
		}
		p += tmp;
		/* SN */
		{
			pBAK = p;
			if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
			{
				fail_flag = 1;
				break;
			}
			p += tmp;
			if ((unsigned long)(p - pBAK) > 0xff)
			{
				fail_flag = 1;
				break;
			}
		}

		/* signature algorithm */
		if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		{
			fail_flag = 1;
			break;
		}
		tmp = sizeof(tmpOID);
		if (rtl_wapi_cert_get_OID(&p, pMax, tmpOID, &tmp, &parLen))
		{
			fail_flag = 1;
			break;
		}
		if (p + parLen > pMax)
		{
			fail_flag = 1;
			break;
		}
		if (hlpCheckOIDAndParam(tmpOID, tmp, NULL, 0, 0))
		{
			fail_flag = 1;
			break;
		}

		p += parLen;
		/* Issuer */
		tmp = sizeof(ret->data);
		if (rtl_wapi_cert_get_Name(&p, pMax, ret->data, &tmp))
		{
			fail_flag = 1;
			break;
		}
		ret->length = (unsigned char)tmp;
		
	} while(0);

	if (fail_flag)
	{
		ret = (wapi_data*)rtl_free_buffer(ret, sizeof(wapi_data));
	}
	return ret;
}



int rtl_wapi_cert_get_sign(cert_id *cert_st, unsigned char *out, int out_len)
{
	unsigned char *p = NULL;
	unsigned char *pMax = NULL;
	unsigned long tmp = 0;
	int tmp_len = 0;
	int ret = -1;	

	if (cert_st == NULL || out == NULL || out_len < SIGN_LEN)
	{
		return ret;
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	p = cert_st->data;
	if (p == NULL)
		return ret;
	pMax = p + cert_st->length;
 
	do
	{
		/* tbsCertificate */
		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		   {
				break;
		   }
		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		   {
				break;
		   }
		/* skip the cert main informations */
		p += tmp;
		tmp = 0;

		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		   {
				break;
		   }
		/* skip the sign arithmetic */
		p += tmp;
		tmp = 0;

		/* parse sign value -------start--------*/
		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		   {
				break;
		   }

		/* skip the compress flag */
		p++;

		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		   {
				break;
		   }

		/* first parts */
		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		   {
				break;
		   }

		if (tmp > 0x18 && *p == 0x00)
		{
			/* skip 0x00 */
			p++;
			tmp_len = (int)tmp - 1;
			memcpy(out, p, tmp_len);
		}
		else
		{
			if (tmp == 0x17)
			{	
				p--;
				tmp_len = (int)tmp + 1;
				memcpy(out, p, tmp_len);
				out[0] = 0x00;
			}
			else
			{
				tmp_len = (int)tmp;
				memcpy(out, p, tmp_len);
			}
		}
		p += tmp_len;
		tmp = 0;

		/* second parts */
		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		   {
				break;
		   }

		if (tmp > 0x18 && *p == 0x00)
		{
			/* skip 0x00 */
			p++;
			memcpy(out+tmp_len, p, tmp - 1);
		}
		else
		{
			if (tmp == 0x17)
			{	
				p--;
				memcpy(out+tmp_len, p, tmp + 1);
				out[tmp_len] = 0x00;
			}
			else
			{
				memcpy(out+tmp_len, p, tmp_len);
			}
		}
		/* parse sign value -------end--------*/

		/* sccess return */
		ret = 0;

	}while(0);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit (ret = %d)<<<<<<<<<<<<<<<<<", __FUNCTION__, ret);

	return ret;
}

int rtl_wapi_cert_get_sign_inlen(cert_id *cert_st)
{
	   unsigned char *p 	  = NULL;
	unsigned char *pMax = NULL;
	unsigned char *pBAK = NULL;
	   unsigned long tmp;
	int ret_len = 0;	

	   if (cert_st == NULL)
	   {
		return ret_len;
	   }

	p = cert_st->data;
	if (p == NULL)
		return ret_len;
	pMax = p + cert_st->length;

	do
	{
		/* tbsCertificate */
		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, NULL, NULL))
		   {
				break;
		   }
			pBAK = p;	
		   if (rtl_wapi_cert_get_sequence(&p, pMax, NULL, NULL, &tmp, NULL))
		   {
				break;
		   }
		/* sign(input) availability length */
		ret_len = ((int)tmp + (p - pBAK));
	}while(0);

	rtl_wapi_trace(MSG_DEBUG,"rtl_wapi_cert_get_sign_inlen: '%d'", ret_len);
	return ret_len;
}

//verify certification signature
int rtl_wapi_cert_verify(tkey *ca_pubkey, cert_id *user_cert)
{
	int sign_len;
	u8 sign_value[SIGN_LEN+1];

	if (!ca_pubkey || !user_cert)
		return -1;

	memset(sign_value, 0, sizeof(sign_value));
	if (rtl_wapi_cert_get_sign(user_cert, sign_value, sizeof(sign_value)) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_wapi_cert_get_sign() fail.", __FUNCTION__, __LINE__);
		return -1;
	}

	//rtl_dump_buffer(sign_value, SIGN_LEN, "signature from ASUE cert");
	
	sign_len = rtl_wapi_cert_get_sign_inlen(user_cert);

	if (sign_len <= 0) {
		rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_wapi_cert_get_sign_inlen() fail.", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if (x509_ecc_verify(ca_pubkey->data, ca_pubkey->length, user_cert->data+4, sign_len, sign_value, SIGN_LEN) <= 0) {
		rtl_wapi_trace(MSG_ERROR, "%s:%d Call x509_ecc_verify() fail.", __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

int rtl_load_cert_from_file(char *cert_file, u8 *buff, u16 buff_len)
{	
	int len = 0;
	FILE* fp = fopen((const char*)cert_file, "rb");
	if (fp == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: Open %s fail.", __FUNCTION__, cert_file);
		return -1;
	}
	
	len = fread(buff, 1, buff_len, fp);
	fclose(fp);
	
	return len;
}

tkey *rtl_parse_private_key(u8 *cert_data, u16 cert_len)
{
	u8 *p  = NULL;
	u8 tmp[MAX_CERT_LEN] = {0};
	u8 buffer[MAX_CERT_LEN] = {0};
	int len = 0;
	private_key prikey;
	tkey *private_key = NULL;
	
	memset(tmp, 0, sizeof(tmp));
	p = rtl_wapi_cert_get_realinfo(tmp, cert_data, cert_len, PEM_STRING_PRIKEYS,	PEM_STRING_PRIKEYE);
	if (p == NULL ) {
		rtl_wapi_trace(MSG_ERROR, "%s: Call rtl_wapi_cert_get_realinfo() fail.", __FUNCTION__);
		return NULL;
	}

	len = strlen((const char *)tmp);
	memset(buffer, 0, sizeof(buffer));
	/* decode base64 */
	if ((len = Base64Dec(buffer, tmp, len)) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: Call Base64Dec() fail.", __FUNCTION__);
		return NULL;
	}

	/* parse der */
	unpack_private_key(&prikey, buffer, (short)len);

	private_key = (tkey*)rtl_allocate_buffer(sizeof(tkey));
	if (!private_key) {
		rtl_wapi_trace(MSG_ERROR, "%s: Call rtl_allocate_buffer() fail.", __FUNCTION__);
		return NULL;
	}
	
	memcpy(private_key->data, prikey.vPrivateKey, prikey.lPrivateKey);
	private_key->length = prikey.lPrivateKey;

	return private_key;
}

int rtl_decode_base64_cert(u8 *cert_data, u16 *cert_len)
{
	u8 *p  = NULL;
	u8 tmp[MAX_CERT_LEN] = {0};
	u8 buffer[MAX_CERT_LEN] = {0};
	int len = 0;

	memset(tmp, 0, sizeof(tmp));
	p = rtl_wapi_cert_get_realinfo(tmp, cert_data, *cert_len, PEM_STRING_X509_CERTS, PEM_STRING2_X509_CERTE);
	if (p == NULL) {
		rtl_wapi_trace(MSG_ERROR, "%s: Call rtl_wapi_cert_get_realinfo() fail.", __FUNCTION__);
		return -1;
	}

	len = strlen((const char *)tmp);
	memset(buffer, 0, sizeof(buffer));	
	/* decode base64 */
	if ((len = Base64Dec(buffer, tmp, len)) < 0) {
		rtl_wapi_trace(MSG_ERROR, "%s: Call Base64Dec() fail.", __FUNCTION__);
		return -1;
	}

	memcpy(cert_data, buffer, len);
	*cert_len = len;
	return 0;	
}

void rtl_wapi_cert_free(cert_info **ppcert_infos)
{
	cert_info *cert_infos = *ppcert_infos;
	
	if (!cert_infos) {
		rtl_wapi_trace(MSG_DEBUG,"%s:%d certs_info is NULL\n", __FUNCTION__, __LINE__);
		return;
	}
	if (cert_infos->ca_cert) {
		rtl_free_buffer(cert_infos->ca_cert, sizeof(cert_id));
		cert_infos->ca_cert = NULL;
	}
	if (cert_infos->ca_pubkey) {
		rtl_free_buffer(cert_infos->ca_pubkey, sizeof(tkey));
		cert_infos->ca_pubkey = NULL;
	}
	if (cert_infos->asu_cert) {
		rtl_free_buffer(cert_infos->asu_cert, sizeof(cert_id));
		cert_infos->asu_cert = NULL;
	}
	if (cert_infos->asu_pubkey) {
		rtl_free_buffer(cert_infos->asu_pubkey, sizeof(tkey));
		cert_infos->asu_pubkey = NULL;
	}
	if (cert_infos->asu_prikey) {
		rtl_free_buffer(cert_infos->asu_prikey, sizeof(tkey));
		cert_infos->asu_prikey = NULL;
	}
	if (cert_infos->asu_id) {
		rtl_free_buffer(cert_infos->asu_id, sizeof(wai_fixdata_id));
		cert_infos->asu_id = NULL;
	}
	if (cert_infos->ae_cert) {
		rtl_free_buffer(cert_infos->ae_cert, sizeof(cert_id));
		cert_infos->ae_cert = NULL;
	}
	if (cert_infos->ae_pubkey) {
		rtl_free_buffer(cert_infos->ae_pubkey, sizeof(tkey));
		cert_infos->ae_pubkey = NULL;
	}
	if (cert_infos->ae_prikey) {
		rtl_free_buffer(cert_infos->ae_prikey, sizeof(tkey));
		cert_infos->ae_prikey = NULL;
	}
	if (cert_infos->ae_id) {
		rtl_free_buffer(cert_infos->ae_id, sizeof(wai_fixdata_id));
		cert_infos->ae_id = NULL;
	}

	rtl_free_buffer(cert_infos, sizeof(cert_info));
	cert_infos = NULL;
}


int rtl_wapi_psk_init(struct wapi_ae_st *wapi_ae, KEY_TYPE kt, u32 kl, u8 *kv)
{	
	if (!wapi_ae)
		return -1;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	wapi_ae->psk_infos = (psk_info *)rtl_allocate_buffer(sizeof(psk_info));
	if (!wapi_ae->psk_infos) {
		rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_allocate_buffer() fail.", __FUNCTION__, __LINE__);
		return -1;
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: kt = %d kl = %d", __FUNCTION__, kt, kl);

	psk_info *psk_infos = wapi_ae->psk_infos;
	psk_infos->kt = kt;
	psk_infos->kl = kl;
	memcpy(psk_infos->kv, kv, kl);

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);
	
	return 0;	
}

int rtl_wapi_cert_init(struct wapi_ae_st *wapi_ae, char *ca_cert_file, char *asu_cert_file, char *ae_cert_file)
{
	int len;
	cert_id *pcert = NULL;
	
	if (!wapi_ae)
		return -1;
	
	if (!ca_cert_file && !asu_cert_file && !ae_cert_file)
		return -1;

	rtl_wapi_trace(MSG_DEBUG, "%s: Enter >>>>>>>>>>>>>>>>>", __FUNCTION__);

	if (wapi_ae->cert_infos)
		rtl_wapi_cert_free(&wapi_ae->cert_infos);

	wapi_ae->cert_infos = (cert_info *)rtl_allocate_buffer(sizeof(cert_info));

	if (!wapi_ae->cert_infos) {
		rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_allocate_buffer() fail.", __FUNCTION__, __LINE__);
		return -1;
	}

	if (ca_cert_file) {	
		snprintf(wapi_ae->cert_infos->ca_cert_file, CERT_FILE_NAME_LEN, "%s", ca_cert_file);
		wapi_ae->cert_infos->ca_cert = (cert_id *)rtl_allocate_buffer(sizeof(cert_id));
		pcert = wapi_ae->cert_infos->ca_cert;		
		if (!pcert) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_allocate_buffer() fail.", __FUNCTION__, __LINE__);
			return -1;
		}

		len = rtl_load_cert_from_file(ca_cert_file, pcert->data, MAX_CERT_LEN);
		if (len <= 0) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_load_cert_from_file() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
		pcert->cert_flag = CERT_OBJ_X509;
		pcert->length = len;

		if (rtl_decode_base64_cert(pcert->data, &pcert->length) < 0) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_decode_base64_cert() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
		
		wapi_ae->cert_infos->ca_pubkey = rtl_wapi_cert_get_pubkey(pcert);
		if (!wapi_ae->cert_infos->ca_pubkey) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_wapi_cert_get_pubkey() fail.", __FUNCTION__, __LINE__);
			return -1;
		}

		//rtl_dump_buffer(wapi_ae->cert_infos->ca_pubkey->data, wapi_ae->cert_infos->ca_pubkey->length, "CA public key");
	}

	if (asu_cert_file) {	
		snprintf(wapi_ae->cert_infos->asu_cert_file, CERT_FILE_NAME_LEN, "%s", asu_cert_file);
		wapi_ae->cert_infos->asu_cert = (cert_id *)rtl_allocate_buffer(sizeof(cert_id));
		pcert = wapi_ae->cert_infos->asu_cert;	
		if (!pcert) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_allocate_buffer() fail.", __FUNCTION__, __LINE__);
			return -1;
		}

		len = rtl_load_cert_from_file(asu_cert_file, pcert->data, MAX_CERT_LEN);
		if (len <= 0) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_load_cert_from_file() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
		pcert->cert_flag = CERT_OBJ_X509;
		pcert->length = len;

		if (strcmp(asu_cert_file, ca_cert_file)) {
			wapi_ae->cert_infos->asu_prikey = rtl_parse_private_key(pcert->data, pcert->length);
			if (!wapi_ae->cert_infos->asu_prikey) {
				rtl_wapi_trace(MSG_ERROR, "%s:%d There is no private key in %s.", __FUNCTION__, __LINE__, asu_cert_file);
				return -1;
			}
		}

		if (rtl_decode_base64_cert(pcert->data, &pcert->length) < 0) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_decode_base64_cert() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
		
		wapi_ae->cert_infos->asu_pubkey = rtl_wapi_cert_get_pubkey(pcert);
		if (!wapi_ae->cert_infos->asu_pubkey) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_wapi_cert_get_pubkey() fail.", __FUNCTION__, __LINE__);
			return -1;
		}

		wapi_ae->cert_infos->asu_id = (wai_fixdata_id *)rtl_allocate_buffer(sizeof(wai_fixdata_id));
		if (!wapi_ae->cert_infos->asu_id) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_allocate_buffer() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
		rtl_wapi_fixdata_id(pcert, wapi_ae->cert_infos->asu_id);

		if (strcmp(asu_cert_file, ca_cert_file)) {
			if (rtl_wapi_cert_verify(wapi_ae->cert_infos->ca_pubkey, pcert) < 0) {
				rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_wapi_cert_verify() fail.", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	if (ae_cert_file) {
		snprintf(wapi_ae->cert_infos->ae_cert_file, CERT_FILE_NAME_LEN, "%s", ae_cert_file);
		wapi_ae->cert_infos->ae_cert = (cert_id *)rtl_allocate_buffer(sizeof(cert_id));
		pcert = wapi_ae->cert_infos->ae_cert;
		if (!pcert) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_allocate_buffer() fail.", __FUNCTION__, __LINE__);
			return -1;
		}

		len = rtl_load_cert_from_file(ae_cert_file, pcert->data, MAX_CERT_LEN);
		if (len <= 0) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_load_cert_from_file() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
		pcert->cert_flag = CERT_OBJ_X509;
		pcert->length = len;

		wapi_ae->cert_infos->ae_prikey = rtl_parse_private_key(pcert->data, pcert->length);
		if (!wapi_ae->cert_infos->ae_prikey) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_parse_private_key() fail.", __FUNCTION__, __LINE__);
			return -1;
		}		

		if (rtl_decode_base64_cert(pcert->data, &pcert->length) < 0) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_decode_base64_cert() fail.", __FUNCTION__, __LINE__);
			return -1;
		}

		wapi_ae->cert_infos->ae_id = (wai_fixdata_id *)rtl_allocate_buffer(sizeof(wai_fixdata_id));
		if (!wapi_ae->cert_infos->ae_id) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_allocate_buffer() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
		rtl_wapi_fixdata_id(pcert, wapi_ae->cert_infos->ae_id);

		if (rtl_wapi_cert_verify(wapi_ae->cert_infos->ca_pubkey, pcert) < 0) {
			rtl_wapi_trace(MSG_ERROR, "%s:%d Call rtl_wapi_cert_verify() fail.", __FUNCTION__, __LINE__);
			return -1;
		}
	}

	rtl_wapi_trace(MSG_DEBUG, "%s: Exit <<<<<<<<<<<<<<<<<", __FUNCTION__);

	return 0;
}

