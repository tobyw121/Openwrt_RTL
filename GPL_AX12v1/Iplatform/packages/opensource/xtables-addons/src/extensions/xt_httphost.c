/*!Copyright(c) 2013-2014 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     xt_httphost.c
 *\brief    kernel/netfilter part for http host filter. 
 *
 *\author   Hu Luyao
 *\version  1.0.0
 *\date     23Dec13
 *
 *\history  \arg 1.0.0, creat this based on "multiurl" mod from soho.  
 *                  
 */

/***************************************************************************/
/*                      CONFIGURATIONS                   */
/***************************************************************************/


/***************************************************************************/
/*                      INCLUDE_FILES                    */
/***************************************************************************/
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/x_tables.h>
#include <linux/version.h>

#include "xt_httphost.h"
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,34))
#include "compat_xtnu.h"
#endif

/***************************************************************************/
/*                      DEFINES                      */
/***************************************************************************/
#define HOST_STR    "\r\nHost: "
#define GET_STR     "GET "
#define HTTP_STR    " HTTP"
#define DEBUG       0

#define DNS_PORT 53
#define HTTP_PORT 80
#define HTTPS_PORT 443

#define HANDSHAKE 22 /*ssl: content type.SSL_TYPE*/
#define CLIENT_HELLO 1 /*handshake: content type.HANDSHAKE_TYPE*/
#define SERVER_NAME 0 /*extension type in client hello(can only appear once in client hello).EXTENSION_TYPE*/
#define HOST_NAME 0 /*content type in SNI(in server_name extension).SERVER_NAME_TYPE*/
/***************************************************************************/
/*                      TYPES                            */
/***************************************************************************/
 enum FILTER_TYPE{
    HTTP,
    DNS,
    TYPE_SUM
 };

typedef struct _PROTOCOL_VERSION
{
	uint8_t majorVersion;
	uint8_t minorVersion;
}PROTOCOL_VERSION;

typedef struct _SSL_MSG{
	uint8_t type; /*len:1 byte*/
	PROTOCOL_VERSION version; /*len:2 bytes*/
	uint16_t length; /* The length (in bytes) of the following TLSPlaintext.fragment.*/
	uint8_t *pContent; /*  The application data,type is specified by the type field.*/
}SSL_MSG;

typedef uint32_t uint24_t;

typedef struct{
	uint16_t length;
	uint8_t *pData;
}CIPHER_SUITE,CH_EXTENSIONS;

typedef struct{
	uint8_t length;
	uint8_t *pData;
}SESSION_ID,COMPRESSION_METHOD;

typedef struct _TLS_EXTENSION{
	uint16_t type;
	uint16_t length;
	uint8_t *pData;
}TLS_EXTENSION;/*TLS(client hello) extension*/

typedef struct _HANDSHAKE_CLIENT_HELLO{
	uint8_t type; /*len:1 byte*/
	uint24_t length;
	PROTOCOL_VERSION clientVersion;
    uint8_t *random;/*the length is 32,but we don't need this field.So only give pointer to start position*/
    SESSION_ID sessionID;
    CIPHER_SUITE cipherSuites;
    COMPRESSION_METHOD compression_methods;
    uint8_t *pExtensions /*pointer to extensions length field*/;
}HANDSHAKE_CLIENT_HELLO;

/***************************************************************************/
/*                      EXTERN_PROTOTYPES                    */
/***************************************************************************/


/***************************************************************************/
/*                      LOCAL_PROTOTYPES                     */
/***************************************************************************/
/*!
 *\fn           static void _transDomain2Buf(unsigned char *dns, unsigned char *buf)
 *\brief        Trans the dns domain in packet to buf with '.'.
 *\param[in]    dns      Domain name in packet.
 *\param[out]   buf      Buffer to store the domain.
 *\return       N/A
 */
static unsigned int _transDomain2Buf(unsigned char *dns, unsigned char *buf, signed int bufLen);

/*!
 *\fn           unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, 
                                        const unsigned char* strCharSet)
 *\brief        find the url in str zone
 *\param[in]    start           start ptr of str zone.
 *\param[in]    end             end ptr of str zone.
 *\param[in]    strCharSet      the url you want to find
 *\return       url postion
 */
static unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, const unsigned char* strCharSet);

/*!
 *\fn           static bool match(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns)
 *\return       found or not
 */
static bool match(const struct sk_buff *skb, struct xt_action_param *param);

/*!
 *\fn           static int __init httphost_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init httphost_init(void);

/*!
 *\fn           static void __exit httphost_exit(void)
 *\brief        mod exit
 *\return       none
 */

static void __exit httphost_exit(void);
/* 
 * fn		static bool extractHandshakeFromSSL(const uint8_t *pSSLBuff, uint8_t **ppHandshake) 
 * brief	extract the handshake From SSL packet.
 * param[in]	pSSL - pointer to the start of SSL packet in skb_buff.
 * param[out]	ppHandshake - address of pointer to the start of handshake message wrapped with SSLv3/TLS.
 * return	BOOL
 * retval	true  succeed to extract handshake 
 *		false fail to extract handshake  	
 */
static bool extractHandshakeFromSSL(const uint8_t *pSSL, uint8_t **ppHandshake);

/* 
 * fn		static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t *ppSNIExt) 
 * brief	extract SNI extension form extensions.
 * param[in]	pExtensions - pointer to start of extensionList.
 * param[out]	ppSNIExt      - address of pointer to SNI extension.
 * return	bool
 * retval	true - extract SNI extension successfully.
 *       	false - extract SNI extension unsuccessfully.	
 */
static bool extractSNIFromExtensions(const uint8_t *pExtensions,uint8_t **ppSNIExt);

/* 
 * fn		static  bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt) 
 * brief	extract SNI extension(Server_name)represents host_name from client_hello.
 * param[in]	pClientHello - pointer to start position of client_hello message.
 * param[out]	ppSNIExt - address of pointer to the start position of SNI extension in client_hello.
 * return	bool
 * retval	true -get the SNI represents host_name.
 *		false - doesn't get the right SNI.
 */
static bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt);

/***************************************************************************/
/*                      VARIABLES                        */
/***************************************************************************/
static struct xt_match httphost_match = { 
    .name           = "httphost",
    .family         = NFPROTO_UNSPEC,
    .match          = match,
    .matchsize      = XT_ALIGN(sizeof(struct _xt_httphost_info)),
    .me             = THIS_MODULE,
};
 
/***************************************************************************/
/*                      LOCAL_FUNCTIONS                  */
/***************************************************************************/

/* 
 * fn		static bool extractHandshakeFromSSL(const uint8_t *pSSLBuff, uint8_t **ppHandshake) 
 * brief	extract the handshake From SSL packet.
 * details	only get address of the pointer to handshake.
 *
 * param[in]	pSSL - pointer to the start of SSL packet in skb_buff.
 * param[out]	ppHandshake - address of pointer to the start of handshake message wrapped with SSLv3/TLS.
 *
 * return	BOOL
 * retval	true  succeed to extract handshake 
 *		false fail to extract handshake  
 * note		
 */
static bool extractHandshakeFromSSL(const uint8_t *pSSL, uint8_t **ppHandshake)
{
	SSL_MSG ssl;
	
	if ((ssl.type = *pSSL++) != HANDSHAKE)
	{
		return false;
	}
	/*
	ssl.version.majorVersion = *pSSL++;
	ssl.version.minorVersion = *pSSL++;
	*/
	pSSL += 2;
	
	ssl.length = ntohs(*((uint16_t *)pSSL));
	pSSL += 2;
	
	if(0 == ssl.length)
	{
		return false;
	}
	/*ssl.pContent = pSSL;*/
	*ppHandshake = (uint8_t *)pSSL;

	
	return true;
}
/* 
 * fn		static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t *ppSNIExt) 
 * brief	extract SNI extension form extensions.
 * details	get pointer to start position of SNI extension that exists in server name extension.
 *
 * param[in]	pExtensions - pointer to start of extensionList.
 * param[out]	ppSNIExt      - address of pointer to SNI extension.
 *
 * return	bool
 * retval	true - extract SNI extension successfully.
 *          false - extract SNI extension unsuccessfully.
 * note		
 */
static bool extractSNIFromExtensions(const uint8_t *pExtensions, uint8_t **ppSNIExt)
{
	int extensionsLen; /*length of all extensions.*/
	int handledExtLen;/*length of handled extensions.*/
	TLS_EXTENSION ext;

	extensionsLen = ntohs(*((uint16_t *)pExtensions));
	pExtensions += 2;
	
	for (handledExtLen = 0; handledExtLen < extensionsLen; )
	{
		ext.type = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.length = ntohs(*((uint16_t *)pExtensions));
		pExtensions += 2;
		ext.pData = (ext.length ? (uint8_t *)pExtensions : NULL);
		if (SERVER_NAME == ext.type)
		{
			*ppSNIExt = ext.pData;
			if (ext.length)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		pExtensions += ext.length;
		handledExtLen += (2 + 2 + ext.length);
	}

	return false;
}
/* 
 * fn		static  bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt) 
 * brief	extract SNI extension(Server_name)represents host_name from client_hello.
 * details	get pointer to start position of SNI extension from client_hello message.
 *
 * param[in]	pClientHello - pointer to start position of client_hello message.
 * param[out]	ppSNIExt - address of pointer to the start position of SNI extension in client_hello.
 *
 * return	bool
 * retval	true -get the SNI represents host_name.
 *			false - doesn't get the right SNI.
 * note		
 */
static bool extractSNIFromClientHello(const uint8_t *pClientHello, uint8_t **ppSNIExt)
{
	HANDSHAKE_CLIENT_HELLO clientHello;
	/*
	clientHello.type = *pClientHello++;
	clientHello.length = NET_3BYTES_TO_HOST_UINT32(pClientHello);
	pClientHello += 3;
	Ignore type and length of client_hello.
	*/
	pClientHello += 4;
	
	clientHello.clientVersion.majorVersion = *pClientHello++;
	clientHello.clientVersion.minorVersion = *pClientHello++;
	/*SNI extension is not supported until TLS 1.0(version 0x0301)*/
	if (clientHello.clientVersion.majorVersion < 3
	 || (3 == clientHello.clientVersion.majorVersion && 0 == clientHello.clientVersion.minorVersion))
	{
		return false;
	}
	/*clientHello.random = pClientHello;*/
	pClientHello += 32;/*length of random is fixed.*/
	clientHello.sessionID.length = *pClientHello++;
	/*clientHello.sessionID.pData = pClientHello;*/
	pClientHello += clientHello.sessionID.length;
	clientHello.cipherSuites.length = ntohs(*((uint16_t *)pClientHello));
	pClientHello += 2;
	/*clientHello.cipherSuites.pData = pClientHello;*/
	pClientHello += clientHello.cipherSuites.length;
	clientHello.compression_methods.length = *pClientHello++;
	/*clientHello.compression_methods.pData = pClientHello;*/
	
	pClientHello += clientHello.compression_methods.length;
	clientHello.pExtensions = (uint8_t *)pClientHello;

	return extractSNIFromExtensions(clientHello.pExtensions, ppSNIExt);
}

/*!
 *\fn           static void _transDomain2Buf(unsigned char *dns, unsigned char *buf)
 *\brief        Trans the dns domain in packet to buf with '.'.
 *
 *\param[in]    dns      Domain name in packet.
 *\param[out]   buf      Buffer to store the domain.
 *
 *\return       N/A
 */
static unsigned int _transDomain2Buf(unsigned char *dns, 
                                     unsigned char *buf, signed int bufLen)
{
    signed int index;
    signed int orig_bufLen = bufLen;
    while(('\0' != *dns) && (bufLen > 0))
    {
        for (index = *dns; (index > 0) && (bufLen > 0); index --, bufLen --)
        {
            *(buf++) = *(++dns);
        }
        *(buf ++) = '.';
        dns ++;
        bufLen --;
    }

    if (bufLen < orig_bufLen)
    {
        bufLen ++;
        buf --;
    }
    
    *buf = '\0';
    return (orig_bufLen - bufLen);
}

/*!
 *\fn           unsigned char *_url_strstr(const unsigned char* start, const unsigned char* end, 
                                        const unsigned char* strCharSet)
 *\brief        find the url in str zone
 *\param[in]    start           start ptr of str zone.
 *\param[in]    end             end ptr of str zone.
 *\param[in]    strCharSet      the url you want to find
 *\return       url postion
 */
static unsigned char *_url_strstr(const unsigned char* start, 
                                  const unsigned char* end, const unsigned char* strCharSet)
{
    const unsigned char *s_temp = start;        /*the s_temp point to the s*/

    int l1, l2;

    l2 = strlen(strCharSet);
    
    if (!l2)
    {
        return (unsigned char *)start;
    }

    l1 = end - s_temp + 1;

    while (l1 >= l2)
    {
        l1--;

        if (!memcmp(s_temp, strCharSet, l2))
        {
            return (unsigned char *)s_temp;
        }

        s_temp++;
    }

    return NULL;
}

/*!
 *\fn           static bool match(const struct sk_buff *skb, struct xt_action_param *param)
 *\brief        find the url in skb (host in http or querys in dns or servername in https(Clienthello) )
 *\return       found or not
 */
static bool match(const struct sk_buff *skb, struct xt_action_param *param)
{   
   
    const struct _xt_httphost_info *info = param->matchinfo;
    const struct iphdr *iph = ip_hdr(skb);
    /* handle http request */

    if (info->type == HTTP)
    {
        unsigned char*   http_payload_start = NULL;
        unsigned char*   http_payload_end = NULL;
  
        unsigned char*   host_str_start = NULL;
        unsigned char*   HOST_start = NULL;
        unsigned char*   HOST_end = NULL;

        unsigned char*   URI_start = NULL;
        unsigned char*   URI_end = NULL;

	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
        
        int i;
	
        
        if(HTTP_PORT == ntohs(tcph->dest))
        {
                http_payload_start = (unsigned char *)tcph + tcph->doff*4;
                http_payload_end = http_payload_start + (iph->tot_len - iph->ihl*4 - tcph->doff*4) - 1;

            if (http_payload_start < http_payload_end)
            {
                host_str_start = _url_strstr(http_payload_start, http_payload_end, HOST_STR);
                URI_start = _url_strstr(http_payload_start, http_payload_end, GET_STR);
            }



            if (host_str_start != NULL && URI_start != NULL)
            {
                HOST_start = host_str_start + 8;
                URI_start  = URI_start + 4;

                HOST_end = _url_strstr(HOST_start, http_payload_end, "\r\n");
                URI_end = _url_strstr(URI_start, http_payload_end, HTTP_STR);

                if (HOST_end == NULL || URI_end == NULL)
                {
#if DEBUG
                    printk("can not find end of host or GET\n");
#endif
                    return info->mode;   
                }

#if DEBUG
                printk("HOST: ");
                unsigned char* pHOST;
                for (pHOST = HOST_start; pHOST != HOST_end; ++pHOST)
                {
                    printk("%c", *pHOST);
                }
                printk("\n");

                printk("URI: ");
                unsigned char* pURI;
                for (pURI = URI_start; pURI != URI_end; ++pURI)
                {
                    printk("%c", *pURI);
                }
                printk("\n");        
#endif
           
                for (i = 0; i < info->count; ++i)
                {    
#if DEBUG
                    printk("host%d: %s\n", i, info->host_names[i]);
#endif

                    if ( _url_strstr(HOST_start, HOST_end, info->host_names[i]) || 
                    _url_strstr(URI_start, URI_end, info->host_names[i]) )
                    {
#if DEBUG
                        printk("==== matched %s ====\n", info->host_names[i]);
#endif     
                        return TRUE;
                    }
                }
#if DEBUG
               printk("Host OK, not matched\n");
#endif 
               return FALSE;
            }

            /* 
                if you can not find Host
                black list: do not match
                white list: match
                anyway, let it pass
            */
            else
            {
#if DEBUG
                printk("no Host\n");
#endif 
                return info->mode;
            }
        }
        else if (HTTPS_PORT == ntohs(tcph->dest))
	{
            unsigned char *sslStart;
            unsigned char *sslEnd;
            uint8_t *pHandshake;
	    uint8_t * pSNIExt;
		
	    sslStart = (unsigned char *)tcph + tcph->doff * 4;
	    sslEnd = sslStart + (ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4);
            if (sslStart >= sslEnd)
            {
                /*UNIDENTIFY*/
                return info->mode;
            }
            if ((!extractHandshakeFromSSL(sslStart, &pHandshake))
	    || (*pHandshake != CLIENT_HELLO)
	    || (!extractSNIFromClientHello(pHandshake, &pSNIExt)))
	    {
		/*UNIDENTIFY*/
		return info->mode;
	    }
            TLS_EXTENSION SNIExt;/*format is similar with server name extension*/
	    int SNIListLen;
	    int handledSNILen; 

	    SNIListLen = ntohs(*((uint16_t *)pSNIExt));
	    pSNIExt += 2;

	    for (handledSNILen = 0; handledSNILen < SNIListLen; )
	    {
                SNIExt.type = *pSNIExt++;
                SNIExt.length = ntohs(*((uint16_t *)pSNIExt));
                pSNIExt += 2;
                SNIExt.pData = (uint8_t *)pSNIExt;
                pSNIExt += SNIExt.length;
                /*Does CLENT HELLO  fragment have impact on SNI?*/
                if (pSNIExt > sslEnd)
                {
                    /*UNIDENTIFY*/
                    return info->mode;
                }
                handledSNILen += (1 + 2 + SNIExt.length);
#if DEBUG
                printk("servername: ");
                for (i=0;i<SNIExt.length;i++)
                printk("%c",*(SNIExt.pData+i));
                printk("\n");
#endif
                if (HOST_NAME == SNIExt.type)
                {
                    for (i = 0; i < info->count; ++i)
                    {
#if DEBUG
                         printk("host%d: %s\n", i, info->host_names[i]);
#endif
                         if(_url_strstr(SNIExt.pData,pSNIExt,info->host_names[i]))
                         {
#if DEBUG
                             printk("==== matched %s ====\n", info->host_names[i]);
#endif     
                             return true;
                         }
		    }
                }
            }
            return false;	
        }
    }
    /* handle dns request */
    else if (info->type == DNS)
    {
        
        // I can not use "index" because of a warning. I do not mean it!
        signed int i;
        signed int i_count;
        signed int dns_len;

        void  *dns_pkt;
        unsigned char *pTmp;
        unsigned char domain[URLFILTER_MAX_DNS_SIZE];

        unsigned int pkt_len = 0;
        unsigned int domain_len = 0;

        urlfilter_dns_header *pDnsHdr;

        struct udphdr *udph = (void *)iph + iph->ihl*4;
         
        dns_len = (unsigned int) ntohs(udph->len) - sizeof(struct udphdr) - sizeof(urlfilter_dns_header);


        if(dns_len < 0)
        {
#if DEBUG
            printk("dns_len < 0!\n");
#endif
            return info->mode;
        }

        if (dns_len >= URLFILTER_MAX_DNS_SIZE)
        {
#if DEBUG
            printk("dns_len >= URLFILTER_MAX_DNS_SIZE!\n");
#endif
            return info->mode;
        }

        dns_pkt = (void *) udph + sizeof(struct udphdr);
        pDnsHdr = (urlfilter_dns_header *)dns_pkt;

        if (0 != (ntohs(pDnsHdr->flags) & 0x8000)) /* If not request */
        {
#if DEBUG
            printk("not request!\n");
#endif
            return info->mode;
        }

        pTmp = (unsigned char *)pDnsHdr + sizeof(urlfilter_dns_header);

        /* See the packet header, most time it only has one request url, but to do more, we check
         * all about it. */
#if DEBUG
            printk("pDnsHdr->nQDCount ==%d \n", ntohs(pDnsHdr->nQDCount));
#endif
        for (i_count = 0; i_count < ntohs(pDnsHdr->nQDCount) && pkt_len < dns_len; i_count ++)
        {
#if DEBUG
            printk("dns%d \n", i_count);
#endif
            domain_len = _transDomain2Buf(pTmp, domain, URLFILTER_MAX_DNS_SIZE - 1);

#if DEBUG
            printk("domain:%s \n", domain);
#endif
            for (i = 0; i < info->count; ++i)
            {    
#if DEBUG
                printk("host%d: %s\n", i, info->host_names[i]);
#endif

                if (_url_strstr(domain, domain + domain_len, info->host_names[i]))
                {
#if DEBUG
                    printk("==== matched %s ====\n", info->host_names[i]);
#endif     
                    return TRUE;
                }
            }
            
            pkt_len += domain_len + 4 + 1;
            pTmp    += domain_len + 4 + 1;
        }

        return FALSE;
    }
    return FALSE;
}

/*!
 *\fn           static int __init httphost_init(void)
 *\brief        mod init
 *\return       SUCCESS or not
 */
static int __init httphost_init(void)
{
    return xt_register_match(&httphost_match);
}

/*!
 *\fn           static void __exit httphost_exit(void)
 *\brief        mod exit
 *\return       none
 */
static void __exit httphost_exit(void)
{
    xt_unregister_match(&httphost_match);
}

/***************************************************************************/
/*                      PUBLIC_FUNCTIONS                     */
/***************************************************************************/


/***************************************************************************/
/*                      GLOBAL_FUNCTIONS                     */
/***************************************************************************/
module_init(httphost_init);
module_exit(httphost_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hu Luyao <huluyao@tp-link.net>");
MODULE_DESCRIPTION("Xtables: http host name match");
MODULE_ALIAS("ipt_httphost");
MODULE_ALIAS("ip6t_httphost");
