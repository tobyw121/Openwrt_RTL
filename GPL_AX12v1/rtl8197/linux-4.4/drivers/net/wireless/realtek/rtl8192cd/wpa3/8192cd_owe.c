/*
 *  Enhanced Open, Opportunistic Wireless Encryption
 *
 *  Copyright (c) 2019 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/delay.h>
#endif

#include "../8192cd_cfg.h"
#include "../8192cd.h"
#include "../8192cd_debug.h"
#include "../8192cd_headers.h"
#include "../sha256.h"
#include "8192cd_owe.h"

#include "./src_mbedtls/mbedtls/library/mbedtls/ecp.h"
#include "./src_mbedtls/mbedtls/library/mbedtls/ecdh.h"
#include "./src_mbedtls/mbedtls/library/mbedtls/platform.h"
#include "./src_mbedtls/mbedtls/library/mbedtls/entropy.h"
#include "./src_mbedtls/mbedtls/library/mbedtls/ctr_drbg.h"

/* parameters */
#define OWE_VAP_5G	"wlan0-va3"
#define OWE_VAP_24G	"wlan1-va3"
const unsigned char OWE_VD_OUI[3] = {0x50, 0x6f, 0x9a};
const unsigned char OWE_VD_OUI_TP = 0x1c;
#define OWE_DH_PUBKEY_OFFSET	5
#define OWE_DH_GROUP_OFFSET	3
#define OWE_PMKID_OFFSET_RSNIE	24
#define OWE_DH_PUBKEY_SIZE	32
#define OWE_SHA256_KEY_SIZE	32
#define OWE_DH_GROUP		19
#define OWE_PMKID_LEN		16
#define OWE_PMK_CACHE_NUM	16
#define OWE_RSNIE_NO_PMKID_LEN	26
#define OWE_DH_ELE_ID_EXT	32

struct owe_pmk_cache_t {
	unsigned char peer_mac[MACADDRLEN];
	unsigned char pmkid[OWE_PMKID_LEN];
	unsigned char pmk[OWE_SHA256_KEY_SIZE];
};

/* 0 for 5g; 1 for 2.4g */
static struct owe_pmk_cache_t owe_pmk_caching[2][OWE_PMK_CACHE_NUM];
static unsigned int owe_pmk_cache_5g_idx;
static unsigned int owe_pmk_cache_24g_idx;


/*
 * Initial OWE parameters
 *
 * @param priv: interface priv info
 * @return none
 */
void owe_init(struct rtl8192cd_priv *priv)
{
	priv->owe_enble = 0;
	priv->owe_pmk_rdy = 0;
	memset(priv->owe_pubkey, 0, PMK_LEN);
	memset(priv->owe_pmk, 0, PMK_LEN);
	memset(priv->owe_pmkid, 0, PMKID_LEN);
	memset(priv->owe_assoc_dh_ie, 0, PMK_LEN + 3);
	memset(priv->owe_assoc_rsn_ie, 0, OWE_MAX_RSNE_SIZE);
}

/*
 * free the buffer used by OWE
 *
 * @param priv: interface priv info
 * @return none
 */
void owe_free(struct rtl8192cd_priv *priv)
{
	log("owe free\n");
	if (priv->pshare->owe_vendor_ie[0]) {
		kfree(priv->pshare->owe_vendor_ie[0]);
	}

	if (priv->pshare->owe_vendor_ie[1]) {
		kfree(priv->pshare->owe_vendor_ie[1]);
	}
}

/*
 * Initial PMK cache buffer can control index for OWE
 *
 * @param none
 * @return none
 */
void owe_pmk_cache_init(void)
{
	owe_pmk_cache_5g_idx = 0;
	owe_pmk_cache_24g_idx = 0;
	memset(owe_pmk_caching[0], 0, OWE_PMK_CACHE_NUM * sizeof(struct owe_pmk_cache_t));
	memset(owe_pmk_caching[1], 0, OWE_PMK_CACHE_NUM * sizeof(struct owe_pmk_cache_t));
}

/*
 * Add PMK cache entry, including PMK, PMKID, and MAC of peer
 *
 * @param priv: interface priv info
 * @param pear_mac : MAC of peer
 * @return none
 */
void owe_pmk_cache_add(struct rtl8192cd_priv *priv, unsigned char *peer_mac)
{
	unsigned int *owe_pmk_cache_idx;
	unsigned int pmk_tlb;

	if (strcmp(priv->dev->name, OWE_VAP_5G) == 0) {
		owe_pmk_cache_idx = &owe_pmk_cache_5g_idx;
		pmk_tlb = 0;
	} else if (strcmp(priv->dev->name, OWE_VAP_24G) == 0) {
		owe_pmk_cache_idx = &owe_pmk_cache_24g_idx;
		pmk_tlb = 1;
	} else
		return;
	
	memcpy(owe_pmk_caching[pmk_tlb][*owe_pmk_cache_idx].peer_mac, peer_mac, MACADDRLEN);
	memcpy(owe_pmk_caching[pmk_tlb][*owe_pmk_cache_idx].pmk, priv->owe_pmk, OWE_SHA256_KEY_SIZE);
	memcpy(owe_pmk_caching[pmk_tlb][*owe_pmk_cache_idx].pmkid, priv->owe_pmkid, OWE_PMKID_LEN);

	if (*owe_pmk_cache_idx == (OWE_PMK_CACHE_NUM -1))
		*owe_pmk_cache_idx = 0;
	else
		*owe_pmk_cache_idx = *owe_pmk_cache_idx + 1;
}

/*
 * search PMK cache entry in PMK caching table
 *
 * @param mac : MAC of peer
 * @param pmkid : PMKID received from peer
 * @return PMK cache control index
 *	   if control index > OWE_PMK_CACHE_NUM, there is no chche in the table
 */
unsigned int owe_pmk_cache_search(struct rtl8192cd_priv *priv, unsigned char *mac, unsigned char *pmkid)
{
	unsigned int pmk_idx;
	unsigned int pmk_tlb;

	if (strcmp(priv->dev->name, OWE_VAP_5G) == 0) {
		pmk_tlb = 0;
	} else if (strcmp(priv->dev->name, OWE_VAP_24G) == 0) {
		pmk_tlb = 1;
	} else
		return OWE_PMK_CACHE_NUM;

	
	for (pmk_idx = 0; pmk_idx < OWE_PMK_CACHE_NUM; pmk_idx++) {
		if (!memcmp(mac, owe_pmk_caching[pmk_tlb][pmk_idx].peer_mac, MACADDRLEN)
			&& !memcmp(pmkid, owe_pmk_caching[pmk_tlb][pmk_idx].pmkid, OWE_PMKID_LEN)) {

			log("PMK Cache found !! %d\n", pmk_idx);
			
			return pmk_idx;
		}	
	}

	return OWE_PMK_CACHE_NUM;
}

/*
 * fill the OWE information, PMK and PMKID, to private
 *
 * @param priv : interface priv info
 * @param pmk_cache_idx : PMK cache control index
 * @return none
 */
void owe_pmk_cache_w2priv(struct rtl8192cd_priv *priv, unsigned int pmk_cache_idx)
{
	unsigned int pmk_tlb;
	
	if (strcmp(priv->dev->name, OWE_VAP_5G) == 0) {
		pmk_tlb = 0;
	} else if (strcmp(priv->dev->name, OWE_VAP_24G) == 0) {
		pmk_tlb = 1;
	} else
		return;

	memcpy(priv->owe_pmk, owe_pmk_caching[pmk_tlb][pmk_cache_idx].pmk, OWE_SHA256_KEY_SIZE);
	memcpy(priv->owe_pmkid, owe_pmk_caching[pmk_tlb][pmk_cache_idx].pmkid, OWE_PMKID_LEN);
	memset(priv->owe_pubkey, 0, OWE_DH_PUBKEY_SIZE);
}

/*
 * Is root inteface encrption setting of root interface is Open
 * owe_enable of mib should be enable, or return false
 *
 * @param priv: interface priv info
 * @return True or False
 */
bool owe_is_root_open(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *root_priv;
	
	if (IS_ROOT_INTERFACE(priv))
		root_priv = priv;
	else
		root_priv = GET_ROOT(priv);

	if ((root_priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 0) &&
	    (root_priv->pmib->dot1180211AuthEntry.dot11OWEEnable == 1))
		return TRUE;
	else
		return FALSE;
}

/*
 * Is OWE interface or not
 *
 * @param priv: interface priv info
 * @return True or False
 */
bool owe_is_owebss(struct rtl8192cd_priv *priv)
{
	if ((strcmp(priv->dev->name, OWE_VAP_5G) == 0) || (strcmp(priv->dev->name, OWE_VAP_24G) == 0)) {
		return TRUE;
	}
	
	return FALSE;
}

/*
 * setup a interface for OWE
 *
 * @param priv: interface priv info
 * @return none
 */
void owe_set_itf_owebss(struct rtl8192cd_priv *priv)
{
	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = PSK_OWE + PSK_WPA2;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = _CCMP_PRIVACY_;
	priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = MGMT_FRAME_PROTECTION_REQUIRED;
	priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 1;
	priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher |= BIT(3);
	priv->owe_enble = TRUE;
	priv->owe_pmk_rdy = FALSE;
}

/*
 * For OWE transition mode, construct the vendor IE of beacon
 * for OPEN BSS, including infomation of OWE BSS
 * for OWE BSS, including information of OPEN BSS
 *
 * @param priv: interface priv info
 * @return True or False
 */
bool owe_construct_vd_ie(struct rtl8192cd_priv *priv)
{
	unsigned char *p_owe_vd_bssid;
	unsigned char *p_owe_vd_ssid;
	unsigned char owe_vd_ssid_len;
	struct rtl8192cd_priv *root_priv;
	priv->pshare->owe_vendor_ie[0] = (struct owe_vendor_ie_t *) kmalloc((sizeof(struct owe_vendor_ie_t)), GFP_ATOMIC);
	memset(priv->pshare->owe_vendor_ie[0], 0, sizeof(struct owe_vendor_ie_t));
	priv->pshare->owe_vendor_ie[1] = (struct owe_vendor_ie_t *) kmalloc((sizeof(struct owe_vendor_ie_t)), GFP_ATOMIC);
	memset(priv->pshare->owe_vendor_ie[1], 0, sizeof(struct owe_vendor_ie_t));

	root_priv = GET_ROOT(priv);

	/* GDEBUG("setup beacon vendor specific\n"); */
	p_owe_vd_bssid = priv->pmib->dot11StationConfigEntry.dot11Bssid;
	p_owe_vd_ssid = priv->pmib->dot11StationConfigEntry.dot11DesiredSSID;
	owe_vd_ssid_len = priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen;
	
	/* set OWE vendor specific of root */
	memcpy(priv->pshare->owe_vendor_ie[0]->oui, OWE_VD_OUI, 3);
	priv->pshare->owe_vendor_ie[0]->oi = OWE_VD_OUI_TP; /* Value assigned by Wi-Fi Alliance */
	memcpy(priv->pshare->owe_vendor_ie[0]->bssid, p_owe_vd_bssid, MACADDRLEN);
	priv->pshare->owe_vendor_ie[0]->ssidlen = owe_vd_ssid_len;
	memcpy(priv->pshare->owe_vendor_ie[0]->ssid, p_owe_vd_ssid, owe_vd_ssid_len);
	priv->pshare->owe_vd_ie_len[0] = 4 + 1 + MACADDRLEN + owe_vd_ssid_len;

	/* set OWE vendor specific of vap0 */
	p_owe_vd_bssid = root_priv->pmib->dot11StationConfigEntry.dot11Bssid;
	p_owe_vd_ssid = root_priv->pmib->dot11StationConfigEntry.dot11DesiredSSID;
	owe_vd_ssid_len = root_priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen;
	memcpy(priv->pshare->owe_vendor_ie[1]->oui, OWE_VD_OUI, 3);
	priv->pshare->owe_vendor_ie[1]->oi = OWE_VD_OUI_TP; /* 3 Value assigned by Wi-Fi Alliance */
	memcpy(priv->pshare->owe_vendor_ie[1]->bssid, p_owe_vd_bssid, MACADDRLEN);
	priv->pshare->owe_vendor_ie[1]->ssidlen = owe_vd_ssid_len;
	memcpy(priv->pshare->owe_vendor_ie[1]->ssid, p_owe_vd_ssid, owe_vd_ssid_len);
	/* oui 3 + oui 1 + bssid 6 + ssidlen 1 + ssid n */
	priv->pshare->owe_vd_ie_len[1] = 4 + 1 + MACADDRLEN + owe_vd_ssid_len;
}

#define MBEDTLS_ERR_HKDF_BAD_INPUT_DATA  -0x5F80  /**< Bad input parameters to function. */

/*
 * Take the input keying material ikm and extract from it a fixed-length pseudorandom key prk.
 *
 * @param md: A hash function; md.size denotes the length of the hash function output in bytes.
 * @param salt:	An optional salt value (a non-secret random value); if the salt is not provided, 
 *		a string of all zeros of md.size length is used as the salt.
 * @param salt_len: The length in bytes of the optional salt.
 * @param ikm: The input keying material.
 * @param ikm_len: The length in bytes of ikm.
 * @param prk: output a pseudorandom key of at least md.size bytes.
 * @return 0 on success.
 */
static int mbedtls_hkdf_extract( const mbedtls_md_info_t *md,
                          const unsigned char *salt, size_t salt_len,
                          const unsigned char *ikm, size_t ikm_len,
                          unsigned char *prk )
{
	unsigned char null_salt[MBEDTLS_MD_MAX_SIZE] = { '\0' };

	if( salt == NULL )
	{
		size_t hash_len;

		if( salt_len != 0 )
		{
			return MBEDTLS_ERR_HKDF_BAD_INPUT_DATA;
		}

		hash_len = mbedtls_md_get_size( md );

		if( hash_len == 0 )
		{
			return MBEDTLS_ERR_HKDF_BAD_INPUT_DATA;
		}

		salt = null_salt;
		salt_len = hash_len;
	}

	return( mbedtls_md_hmac( md, salt, salt_len, ikm, ikm_len, prk ) );
}

/*
 * Expand the supplied prk into several additional pseudorandom keys, which is the output of the HKDF.
 *
 * @param md: A hash function; md.size denotes the length of the hash function output in bytes.
 * @param prk: A pseudorandom key of at least md.size bytes. prk is usually the output from the HKDF extract step
 * @param prk_len: The length in bytes of prk.
 * @param info: An optional context and application specific information string. This can be a zero-length string.
 * @param info_len: The length of info in bytes.
 * @param okm: The output keying material of okm_len bytes.
 * @param okm_len: The length of the output keying material in bytes. This must be less than or equal to 255 * md.size bytes.
 * @return 0 on success.
 */
static int mbedtls_hkdf_expand( const mbedtls_md_info_t *md, const unsigned char *prk,
                         size_t prk_len, const unsigned char *info,
                         size_t info_len, unsigned char *okm, size_t okm_len )
{
	size_t hash_len;
	size_t where = 0;
	size_t n;
	size_t t_len = 0;
	size_t i;
	int ret = 0;
	mbedtls_md_context_t ctx;
	unsigned char t[MBEDTLS_MD_MAX_SIZE];

	if( okm == NULL )
	{
		return( MBEDTLS_ERR_HKDF_BAD_INPUT_DATA );
	}

	hash_len = mbedtls_md_get_size( md );

	if( prk_len < hash_len || hash_len == 0 )
	{
		return( MBEDTLS_ERR_HKDF_BAD_INPUT_DATA );
	}

	if( info == NULL )
	{
		info = (const unsigned char *) "";
		info_len = 0;
	}

	n = okm_len / hash_len;

	if( (okm_len % hash_len) != 0 )
	{
		n++;
	}

	/*
	* Per RFC 5869 Section 2.3, okm_len must not exceed
	* 255 times the hash length
	*/
	if( n > 255 )
	{
		return( MBEDTLS_ERR_HKDF_BAD_INPUT_DATA );
	}

	mbedtls_md_init( &ctx );

	if( (ret = mbedtls_md_setup( &ctx, md, 1) ) != 0 )
	{
		goto exit;
	}

	/*
	* Compute T = T(1) | T(2) | T(3) | ... | T(N)
	* Where T(N) is defined in RFC 5869 Section 2.3
	*/
	for( i = 1; i <= n; i++ )
	{
		size_t num_to_copy;
		unsigned char c = i & 0xff;

		ret = mbedtls_md_hmac_starts( &ctx, prk, prk_len );
		if( ret != 0 )
		{
			goto exit;
		}

		ret = mbedtls_md_hmac_update( &ctx, t, t_len );
		if( ret != 0 )
		{
			goto exit;
		}

		ret = mbedtls_md_hmac_update( &ctx, info, info_len );
		if( ret != 0 )
		{
			goto exit;
		}

		/* The constant concatenated to the end of each T(n) is a single octet.
		 * */
		ret = mbedtls_md_hmac_update( &ctx, &c, 1 );
		if( ret != 0 )
		{
			goto exit;
		}

		ret = mbedtls_md_hmac_finish( &ctx, t );
		if( ret != 0 )
		{
			goto exit;
		}

		num_to_copy = i != n ? hash_len : okm_len - where;
		memcpy( okm + where, t, num_to_copy );
		where += hash_len;
		t_len = hash_len;
	}

exit:
	mbedtls_md_free( &ctx );

	return( ret );
}

/*
 * construct the element of OWE key (DH element)
 * | elememt ID | Length | ID ext | data               |
 * | 255        | 35     | 32     | 19 00 + public key |
 *
 * @param priv: interface priv info
 * @param pubkey: public key generated by ECDH
 * @param len: key length
 * @return none
 */
static void owe_create_assoc_ie(struct rtl8192cd_priv *priv, unsigned char *pubkey, size_t len)
{
	/* element id extension */
	priv->owe_assoc_dh_ie[0] = OWE_DH_ELE_ID_EXT;
	/* group 2 bytes */
	priv->owe_assoc_dh_ie[1] = OWE_DH_GROUP;
	priv->owe_assoc_dh_ie[2] = 0;
	/* public key */
	memcpy(priv->owe_assoc_dh_ie + 3, pubkey, len);
}

/*
 * Decompresses an EC Public Key
 *
 * @param grp: mbedtls ecp group
 * @param input: input compressed key
 * @param ilen: input compressed key length
 * @param output: output uncompressed key
 * @param olen: output key length pointer
 * @param osize: output key size
 * @return success (0) or not
 */
static int mbedtls_ecp_decompress_pubkey(const mbedtls_ecp_group *grp, const unsigned char *input,
				size_t ilen, unsigned char *output, size_t *olen, size_t osize )
{
	int ret;
	size_t plen;
	mbedtls_mpi r;
	mbedtls_mpi x;
	mbedtls_mpi n;

	plen = mbedtls_mpi_size(&grp->P);

	*olen = 2 * plen + 1;

	if (osize < *olen)
		return(MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL);

	if (ilen != plen + 1)
		return(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);

	if (input[0] != 0x02 && input[0] != 0x03)
		return(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);

	/* output will consist of 0x04|X|Y */
	memcpy(output, input, ilen);
	output[0] = 0x04;

	mbedtls_mpi_init(&r);
	mbedtls_mpi_init(&x);
	mbedtls_mpi_init(&n);

	/* x <= input */
	MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&x, input + 1, plen));

	/* r = x^2 */
	MBEDTLS_MPI_CHK(mbedtls_mpi_mul_mpi(&r, &x, &x));

	/* r = x^2 + a */
	if (grp->A.p == NULL) {
		// Special case where a is -3
		MBEDTLS_MPI_CHK(mbedtls_mpi_sub_int(&r, &r, 3));
	} else {
		MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(&r, &r, &grp->A));
	}

	/* r = x^3 + ax */
	MBEDTLS_MPI_CHK(mbedtls_mpi_mul_mpi(&r, &r, &x));

	/* r = x^3 + ax + b */
	MBEDTLS_MPI_CHK(mbedtls_mpi_add_mpi(&r, &r, &grp->B));

	/* Calculate square root of r over finite field P:
		 r = sqrt(x^3 + ax + b) = (x^3 + ax + b) ^ ((P + 1) / 4) (mod P) */

	/* n = P + 1 */
	MBEDTLS_MPI_CHK(mbedtls_mpi_add_int(&n, &grp->P, 1));

	/* n = (P + 1) / 4 */
	MBEDTLS_MPI_CHK(mbedtls_mpi_shift_r(&n, 2));

	/* r ^ ((P + 1) / 4) (mod p) */
	MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod(&r, &r, &n, &grp->P, NULL));

	/* Select solution that has the correct "sign" (equals odd/even solution in finite group) */
	if ((input[0] == 0x03) != mbedtls_mpi_get_bit(&r, 0)) {
		// r = p - r
		MBEDTLS_MPI_CHK(mbedtls_mpi_sub_mpi(&r, &grp->P, &r));
	}

	/* y => output */
	ret = mbedtls_mpi_write_binary(&r, output + 1 + plen, plen);

cleanup:
	mbedtls_mpi_free(&r);
	mbedtls_mpi_free(&x);
	mbedtls_mpi_free(&n);

	return(ret);

}

/*
 *  For OWE, generate public key and save it in priv 
 *
 *  @param priv: interface priv info
 *  @return fail code, 0 is success
 */
static int owe_gen_pub_key(struct rtl8192cd_priv *priv, mbedtls_ecdh_context *owe_ctx, unsigned char *mac)
{
	int ret = 0;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	/* todo: pers set as MAC addr ? */
	const char pers[] = "ecdh";
	unsigned char tmp_key[OWE_DH_PUBKEY_SIZE + OWE_DH_PUBKEY_OFFSET];
	unsigned int olen;
	
	mbedtls_ecdh_init(owe_ctx);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);

	if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
				   mac, MACADDRLEN)) != 0 ) {
		printk( " failed\n	! mbedtls_ctr_drbg_seed returned %d\n", ret );
		return (ret);
	}

	owe_ctx->point_format = MBEDTLS_ECP_PF_COMPRESSED;

	ret = mbedtls_ecp_group_load(&owe_ctx->grp, MBEDTLS_ECP_DP_SECP256R1);
	if (ret != 0) {
		printk( " failed\n	! mbedtls_ecp_group_load returned %d\n", ret );
		goto exit;
	}

	
	int i;
	ret = mbedtls_ecdh_make_params(owe_ctx, &olen, tmp_key, OWE_DH_PUBKEY_SIZE + OWE_DH_PUBKEY_OFFSET,
				       mbedtls_ctr_drbg_random, &ctr_drbg);
	if (ret != 0) {
		printk( " failed\n	! mbedtls_ecdh_make_params returned %d\n", ret );
		goto exit;
	}

	memcpy(priv->owe_pubkey, tmp_key + OWE_DH_PUBKEY_OFFSET, OWE_DH_PUBKEY_SIZE);
exit:
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	return ret;
}

/*
 *  compute the secret (32 bytes) from public key of pear and our private key
 *  z = F(DH(x, Y))
 *  
 *  @param in  c: public key of client
 *  @param out z: secret
 *  @return fail code, 0 is success
 */
static int owe_compute_dh_secret(mbedtls_ecdh_context *owe_ctx, unsigned char *c, unsigned char *z)
{
	int ret = 0;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	const char pers[] = "ecdh";

	unsigned char tmp_key[OWE_DH_PUBKEY_SIZE + 1]; /* add sign bit of Y */
	unsigned char uncompress_p_pub_key[2*OWE_DH_PUBKEY_SIZE + 1]; /* sign bit + X + Y */
	unsigned char pear_key[2*OWE_DH_PUBKEY_SIZE + 2]; /* len + sign bit + X + Y */
	unsigned int uncompress_p_pub_key_olen;

	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
				   (const unsigned char *) pers,
				   sizeof pers)) != 0 ) {
		printk( " failed\n	! mbedtls_ctr_drbg_seed returned %d\n", ret );
		return (ret);
	}
	
	tmp_key[0] = 0x2; /* set sign bit 0 */
	memcpy(tmp_key + 1, c, OWE_DH_PUBKEY_SIZE);

	ret = mbedtls_mpi_lset(&owe_ctx->Qp.Z, 1);
	if (ret != 0) {
		printk( " failed\n	! mbedtls_mpi_lset returned %d\n", ret );
		goto exit;
	}

	ret = mbedtls_ecp_decompress_pubkey(&owe_ctx->grp, tmp_key, OWE_DH_PUBKEY_SIZE + 1,
					uncompress_p_pub_key, &uncompress_p_pub_key_olen, 2*OWE_DH_PUBKEY_SIZE + 1);
	if ((ret != 0) || (uncompress_p_pub_key_olen != 2*OWE_DH_PUBKEY_SIZE + 1)) {
		printk( " failed\n	! mbedtls_ecp_decompress_pubkey returned %x\n", ret );
	}

	memcpy(pear_key + 1, uncompress_p_pub_key, 2*OWE_DH_PUBKEY_SIZE + 1);
	pear_key[0] = 2*OWE_DH_PUBKEY_SIZE + 1;
	
	ret = mbedtls_ecdh_read_public(owe_ctx, pear_key, 66);
	if (ret != 0) {
		printk( " failed\n	! mbedtls_ecdh_read_public returned %x\n", -(ret) );
	}

	ret = mbedtls_ecdh_compute_shared(&owe_ctx->grp, &owe_ctx->z,
					  &owe_ctx->Qp, &owe_ctx->d,
					  mbedtls_ctr_drbg_random, &ctr_drbg);
	if (ret != 0) {
		printk( " failed\n	! mbedtls_ecdh_compute_shared returned %x\n", -(ret) );
		goto exit;
	}

	ret = mbedtls_mpi_write_binary(&owe_ctx->z, z, OWE_DH_PUBKEY_SIZE);
	if (ret != 0) {
		printk( " failed\n	! mbedtls_ecdh_compute_shared returned %x\n", -(ret) );
		goto exit;
	}

exit:
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	return ret;
}

/*
 *  generate PMKID
 *  
 *  @param priv: wlan interface
 *  @param c : public key of pear
 *  @return fail code, 0 is success
 */
static int owe_compute_pmkid(struct rtl8192cd_priv *priv, unsigned char *c)
{
	int ret = 0;
	const unsigned char *addr[2];
	size_t len[2];

	/* PMKID = Truncate-128(Hash(C | A)) */
	addr[0] = c;
	len[0] = OWE_SHA256_KEY_SIZE;
	addr[1] = priv->owe_pubkey;
	len[1] = OWE_SHA256_KEY_SIZE;
	ret = sha256_vector(2, addr, len, priv->owe_pmkid);

	return ret;
}

/*
 *  generate PMK
 *  
 *  @param priv: wlan interface
 *  @param c : public key of pear
 *  @param z : secret
 *  @return fail code, 0 is success
 */
static int owe_compute_pmk(struct rtl8192cd_priv *priv, unsigned char *c, unsigned char *z)
{
	int ret = 0;
	unsigned char prk[OWE_SHA256_KEY_SIZE];
	unsigned char salt[2*OWE_DH_PUBKEY_SIZE + 2];
	unsigned int salt_len, z_len;
	const unsigned char *owe_info = "OWE Key Generation";
	salt_len = 2*OWE_DH_PUBKEY_SIZE + 2;
	z_len = OWE_SHA256_KEY_SIZE;

	memset(prk, 0, OWE_SHA256_KEY_SIZE);
	memset(salt, 0, salt_len);
	memcpy(salt, c, OWE_DH_PUBKEY_SIZE);
	memcpy(salt + OWE_DH_PUBKEY_SIZE, priv->owe_pubkey, OWE_DH_PUBKEY_SIZE);
	salt[2*OWE_DH_PUBKEY_SIZE] = OWE_DH_GROUP;
	salt[2*OWE_DH_PUBKEY_SIZE + 1] = 0;

	/* prk = HKDF-extract(C | A | group, z) */
	ret = mbedtls_hkdf_extract(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
				salt, salt_len, z, z_len, prk);
	if (ret != 0) {
		GDEBUG( " failed\n	! mbedtls_hkdf_extract returned %d\n", ret );
		goto exit;
	}
	
	/* PMK = HKDF-expand(prk, "OWE Key Generation", n) */
	ret = mbedtls_hkdf_expand(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
		prk, OWE_SHA256_KEY_SIZE, owe_info, strlen(owe_info),
		priv->owe_pmk, OWE_SHA256_KEY_SIZE);
	if (ret != 0) {
		GDEBUG( " failed\n	! mbedtls_hkdf_expand returned %d\n", ret );
		goto exit;
	}

	dump_hex("prk", prk, OWE_SHA256_KEY_SIZE);

exit:
	return ret;
}

/*
 * check if the public key of peer is computed or not
 *
 * @param pear_key : public key of pear
 * @return TRUE : computed
 *         FALSE: not computed
 */
static int owe_chk_pear_pubkey(char *pear_key)
{
	static char last_pear_key[OWE_DH_PUBKEY_SIZE];
	unsigned int i;

	if (memcmp(last_pear_key, pear_key, OWE_DH_PUBKEY_SIZE)) {
		memcpy(last_pear_key, pear_key, OWE_DH_PUBKEY_SIZE);
		return FALSE;
	} else {
		return TRUE;
	}
}

/*
 * set PMKID not appeared in RSNIE in association respond
 *
 * @param priv: interface priv info
 * @param rsnie: the rsnie received in association request
 * @return none
 */
static void owe_set_pmkid_null(struct rtl8192cd_priv *priv, char *rsnie)
{
	/* set PMKID no show in RSNE */
	memset(priv->owe_assoc_rsn_ie + OWE_PMKID_OFFSET_RSNIE - 2, 0, OWE_PMKID_LEN + 2);
	memcpy(priv->owe_assoc_rsn_ie + OWE_PMKID_OFFSET_RSNIE,
		priv->owe_assoc_rsn_ie + OWE_PMKID_OFFSET_RSNIE + OWE_PMKID_LEN, 4);
}

/*
 * set PMKID of PMK cache into RSNIE
 *
 * @param rsnie: the rsnie of the interface
 * @param pmkid_peer: the PMKID found in PMK cache
 * @return none
 */
static void owe_set_rsnie_pmkid(unsigned char* rsnie, unsigned char* pmkid_peer)
{
	unsigned int ori_len = rsnie[1];
	unsigned char grp_man_cipher[4];
		
	rsnie[1] = rsnie[1] + OWE_PMKID_LEN; /* rsnie len */
	rsnie[OWE_PMKID_OFFSET_RSNIE - 2] = 1; /* pmkid count b0 */
	rsnie[OWE_PMKID_OFFSET_RSNIE - 1] = 0; /* pmkid count b1 */
	/* padding for group cipher suite */
	memcpy(grp_man_cipher, rsnie + ori_len - 2, 4);
	/* set pmkid */
	memcpy(rsnie + OWE_PMKID_OFFSET_RSNIE, pmkid_peer, OWE_PMKID_LEN);
	memcpy(rsnie + OWE_PMKID_OFFSET_RSNIE + OWE_PMKID_LEN, grp_man_cipher, 4);
}

/*
 * For OWE association
 *
 * @param priv: interface priv info
 * @param cli_pub: ie 0f public key of pear
 * @return fail code, 0 is success
 */
int owe_assoc_req_process(struct rtl8192cd_priv *priv, unsigned char *cli_pub, 
	unsigned char *pear_mac, unsigned char *rsnie)
{
	int ret = 0;
	unsigned char pub_key_pear[OWE_DH_PUBKEY_SIZE];
	unsigned char z[OWE_SHA256_KEY_SIZE]; /* secret */
	unsigned char mac[MACADDRLEN];
	unsigned char pmkid_peer[OWE_PMKID_LEN];
	unsigned int pmk_cache_idx;
	unsigned char peer_grp;
	mbedtls_ecdh_context owe_ctx;

	peer_grp = cli_pub[OWE_DH_GROUP_OFFSET];
	if (peer_grp != OWE_DH_GROUP) {
		GDEBUG(" OWE group %d not supported\n", peer_grp);
		return _STATS_OWE_GP_ERR_;
	}
	
	memcpy(mac, (GET_MIB(priv))->dot11OperationEntry.hwaddr, MACADDRLEN);
	memcpy(pub_key_pear, cli_pub + OWE_DH_PUBKEY_OFFSET, OWE_DH_PUBKEY_SIZE);

	/* pmk cache count in RSNIE not 0 */
	if ((rsnie[OWE_PMKID_OFFSET_RSNIE-2] != 0) || (rsnie[OWE_PMKID_OFFSET_RSNIE-1] != 0)) {
		memcpy(pmkid_peer, rsnie + OWE_PMKID_OFFSET_RSNIE, OWE_PMKID_LEN);
		
		pmk_cache_idx = owe_pmk_cache_search(priv, pear_mac, pmkid_peer);
		if ((0 <= pmk_cache_idx) && (pmk_cache_idx < OWE_PMK_CACHE_NUM)) {
			log(" Using PMK Caching !!!\n");
			owe_pmk_cache_w2priv(priv, pmk_cache_idx);
			priv->owe_pmk_rdy = TRUE;
			priv->owe_pmk_cache_apply = TRUE;
			owe_create_assoc_ie(priv, priv->owe_pubkey, OWE_DH_PUBKEY_SIZE);
			owe_set_rsnie_pmkid(priv->owe_assoc_rsn_ie, pmkid_peer);
			return 0;
		} else {
			log(" not found in PMK caching \n");
			/* owe_set_pmkid_null(priv, rsnie); */
		}
	}

	/* if not use PMK cache */
	priv->owe_pmk_cache_apply = FALSE;
	
	ret = owe_chk_pear_pubkey(pub_key_pear);
	if (ret) {
		GDEBUG(" pear's key has been computed\n");
		return 0;
	}

	ret = owe_gen_pub_key(priv, &owe_ctx, mac);
	if (ret) {
		GDEBUG("owe_gen_pub_key fail\n");
		goto exit;
	}
		
	ret = owe_compute_dh_secret(&owe_ctx, pub_key_pear, z);
	if (ret) {
		GDEBUG("owe_compute_dh_secret fail\n");
		goto exit;
	}
		
	ret = owe_compute_pmkid(priv, pub_key_pear);
	if (ret) {
		GDEBUG("owe_compute_dh_secret fail\n");
		goto exit;
	}

	ret = owe_compute_pmk(priv, pub_key_pear, z);
	if (ret) {
		GDEBUG("owe_compute_dh_secret fail\n");
		goto exit;
	}

	priv->owe_pmk_rdy = TRUE;

	dump_hex("owe z", z, OWE_SHA256_KEY_SIZE);
	dump_hex("owe srv", priv->owe_pubkey, OWE_DH_PUBKEY_SIZE);
	dump_hex("owe cli", pub_key_pear, OWE_DH_PUBKEY_SIZE);
	dump_hex("owe PMK", priv->owe_pmk, OWE_SHA256_KEY_SIZE);
	dump_hex("owe PMKID", priv->owe_pmkid, OWE_PMKID_LEN);

	owe_pmk_cache_add(priv, pear_mac);

	owe_create_assoc_ie(priv, priv->owe_pubkey, OWE_DH_PUBKEY_SIZE);
exit:
	mbedtls_ecdh_free(&owe_ctx);

	return ret;
}

/*
 * calculate MIC of 4-way handshake for OWE (use sha256)
 *
 * @param EAPOLMsgSend : EAPOL buffer for 4-way handshake
 * @param algo : key descriptor version
 * @param key : key for computed
 * @param keylen : key length
 * @return none
 */
void owe_calc_mic(OCTET_STRING EAPOLMsgSend, int algo, unsigned char *key, int keylen)
{
	struct lib1x_eapol *eapol = (struct lib1x_eapol *)(EAPOLMsgSend.Octet + ETHER_HDRLEN);
	lib1x_eapol_key *eapolkey = (lib1x_eapol_key *)(EAPOLMsgSend.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	unsigned char sha256digest[20];
	
	memset(eapolkey->key_mic, 0, KEY_MIC_LEN);

#if defined(CONFIG_IEEE80211W) || defined(CONFIG_IEEE80211R)
	if (algo == key_desc_ver3 || algo == key_desc_ver0) {
		hmac_sha256(key, keylen, (unsigned char*)eapol, EAPOLMsgSend.Length - ETHER_HDRLEN, sha256digest);
		memcpy(eapolkey->key_mic, sha256digest, KEY_MIC_LEN);
	}
#endif
}

/*
 * check MIC of pear in 4-way handshake for OWE (use sha256)
 *
 * @param priv: interface priv info
 * @param EAPOLMsgRecvd : received EAPOL buffer for 4-way handshake
 * @param key : key for computed
 * @param keylen : key length
 * @return 1 : pass, the MIC is the same between server and client
 *	   0 : fail, the MIC is not the same between server and client
 */
int owe_check_mic(struct rtl8192cd_priv *priv, OCTET_STRING EAPOLMsgRecvd, unsigned char *key, int keylen)
{
	int retVal = 0;
	OCTET_STRING EapolKeyMsgRecvd;
	unsigned char ucAlgo;
	OCTET_STRING tmp; //copy of overall 802.1x message
	unsigned char tmpbuf[512];
	struct lib1x_eapol *tmpeapol;
	lib1x_eapol_key *tmpeapolkey;
	unsigned char sha1digest[20];

	if (priv->owe_enble == 0) {
		/* not OWE */
		return 0;
	}

	EapolKeyMsgRecvd.Octet = EAPOLMsgRecvd.Octet +
							 ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN;
	EapolKeyMsgRecvd.Length = EAPOLMsgRecvd.Length -
							  (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	ucAlgo = Message_KeyDescVer(EapolKeyMsgRecvd);

	tmp.Length = EAPOLMsgRecvd.Length;
	tmp.Octet = tmpbuf;
	memcpy(tmp.Octet, EAPOLMsgRecvd.Octet, EAPOLMsgRecvd.Length);
	tmpeapol = (struct lib1x_eapol *)(tmp.Octet + ETHER_HDRLEN);
	tmpeapolkey = (lib1x_eapol_key *)(tmp.Octet + ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN);
	memset(tmpeapolkey->key_mic, 0, KEY_MIC_LEN);

#if defined(CONFIG_IEEE80211W) || defined(CONFIG_IEEE80211R)
	if (ucAlgo == key_desc_ver0) {
		hmac_sha256(key, keylen, (unsigned char*)tmpeapol, LIB1X_EAPOL_HDRLEN + ntohs(tmpeapol->packet_body_length), sha1digest);
		if (!memcmp(sha1digest, EapolKeyMsgRecvd.Octet + KeyMICPos, KEY_MIC_LEN))
			retVal = 1;
	}
#endif
	return retVal;
}

/*
 * show PMK cache information
 *
 * @param nonw
 * @return none
 */
void owe_dbg_pmk_cache(struct rtl8192cd_priv *priv)
{
	unsigned int idx, j;
	unsigned int *owe_pmk_cache_idx;
	unsigned int pmk_tlb;

	if (strcmp(priv->dev->name, OWE_VAP_5G) == 0) {
		owe_pmk_cache_idx = &owe_pmk_cache_5g_idx;
		pmk_tlb = 0;
	} else if (strcmp(priv->dev->name, OWE_VAP_24G) == 0) {
		owe_pmk_cache_idx = &owe_pmk_cache_24g_idx;
		pmk_tlb = 1;
	}


	if (strcmp(priv->dev->name, OWE_VAP_5G) == 0) {
		pmk_tlb = 0;
	} else if (strcmp(priv->dev->name, OWE_VAP_24G) == 0) {
		pmk_tlb = 1;
	}

	printk("OWE PMK cache:\n");
	printk(" pmkid_caching_idx: %d\n", *owe_pmk_cache_idx);
	for(idx = 0; idx < OWE_PMK_CACHE_NUM; idx++) {
		printk(" [idx] %d\n", idx);
		printk("  pmk: ");
		for(j = 0; j < OWE_SHA256_KEY_SIZE; j++) {
			printk("%02x", owe_pmk_caching[pmk_tlb][idx].pmk[j]);
		}
		printk("\n  pmkid: ");
		for(j = 0; j < OWE_PMKID_LEN; j++) {
			printk("%02x", owe_pmk_caching[pmk_tlb][idx].pmkid[j]);
		}
		printk("\n  peermac: ");
		for(j = 0; j < MACADDRLEN; j++) {
			printk("%02x", owe_pmk_caching[pmk_tlb][idx].peer_mac[j]);
		}
		printk("\n");
	}
}

