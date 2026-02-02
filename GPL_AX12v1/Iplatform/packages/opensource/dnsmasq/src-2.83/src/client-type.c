/*
 *\file		client-type.c
 *\brief	check client type base on DHCP Options.
 *
 */

/***************************************************************************/
/*                    INCLUDE FILES                     */
/***************************************************************************/
#include <string.h>
#include "dnsmasq.h"

/***************************************************************************/
/*                    GLOBAL_FUNCTIONS                  */
/***************************************************************************/
/*
 *\fn           device_type_check
 *\brief        check client type base on DHCP Options [60,12,55,57]
 *
 *\param[in]    p_hostname            hostname(option 12)
 *\param[in]    p_vendor              vendor(option 60)
 *\param[in]    p_req_options         request_options(option 55)
 *\param[in]    max_dhcp_message_size max_dhcp_message_size(option 57)
 *
 *\return       DEVICE_TYPE
 */
DEVICE_TYPE device_type_check(const char *p_hostname, const char *p_vendor, const unsigned char *p_req_options, size_t max_dhcp_message_size)
{
	size_t i = 0;
	size_t name_len = 0;
	size_t req_options_len = 0;
	unsigned char ios_options[5] = {0x1, 0x79, 0x3, 0x6, 0xf};
	unsigned char android_options[9] ={0x1, 0x3, 0x6, 0xf, 0x1a, 0x1c, 0x33, 0x3a, 0x3b};
	unsigned char windows_options[14] ={0x1, 0x3, 0x6, 0xf, 0x1f, 0x21, 0x2b, 0x2c, 0x2e, 0x2f, 0x77, 0x79, 0xf9, 0xfc};
	char buf[32] = {0};
	int flag = 0;

	/* 
	 * device type check base on Vendor(option 60)
	 */
	if(NULL != p_vendor)
	{
		if(strstr(p_vendor, "ndroid")) /* "android" or "Android" */
		{
			return DEVICE_TYPE_PHONE;
		}
		else if(strstr(p_vendor, "MSFT")) 
		{
			return DEVICE_TYPE_PC;
		}
		else if((strstr(p_vendor, "SoC") || strstr(p_vendor, "soc"))) /* system on chip */
		{
			return DEVICE_TYPE_IOT;
		}
	}

	/* 
	 * device type check base on Request options(option 55)
	 */
	if(NULL == p_req_options)
	{
		return DEVICE_TYPE_OTHER;
	}
	req_options_len = strlen(p_req_options);
	for(i = 0; i < req_options_len; i++)
	{
		/* PC usually request NetBIOS options  */
		if(p_req_options[i] == OPTION_NBNS_OPT || p_req_options[i] == OPTION_NBNT_OPT)
		{	
			return DEVICE_TYPE_PC;
		}
	}

	/* match IOS, windows, android request options  */
	if((!memcmp(p_req_options, ios_options, sizeof(ios_options)) && NULL == p_vendor ) || 
		!memcmp(p_req_options, android_options, sizeof(android_options)))
	{
		return DEVICE_TYPE_PHONE;
	}

	if(!memcmp(p_req_options, windows_options, sizeof(windows_options)))
	{
		return DEVICE_TYPE_PC;
	}

	if (NULL != p_vendor && (strstr(p_vendor, "linux") || strstr(p_vendor, "Linux")))
	{
		return DEVICE_TYPE_TABLET;
	}

	/*
	 * device type check base on Max dhcp massage size(option 57)
	 */
	if(max_dhcp_message_size == 576)
	{
		return DEVICE_TYPE_IOT;
	}

	/* some IoT devices dont have a device_name and tend to request hostname form DHCP server */
	for(i = 0; i < req_options_len; i++)
	{
		if(p_req_options[i] == OPTION_HOSTNAME)
		{			
			return DEVICE_TYPE_IOT;
		}
		else if(p_req_options[i] == OPTION_DOMAINNAME)
		{
			flag = 1;
		}

		if(i == (req_options_len -1) && 0 == flag)
		{
			return DEVICE_TYPE_IOT;
		}
	}

	/*
	 * device type check base on Hostname(option 12)
	 */
	if(NULL != p_hostname)
	{
		memset(buf, 0, 32);

		name_len = strlen(p_hostname);
		if(name_len > 31)
		{
			name_len = 31;
		}

		strncpy(buf, p_hostname, name_len);
		for(i=0; i < sizeof(buf) ; i++)
		{
			if(buf[i] >= 'A' && buf[i] <= 'Z')
			{
				buf[i] = 'a' + (buf[i] - 'A');
			}
		}

		if(strstr(buf, "iphone") || strstr(buf, "galaxy"))
		{
			return DEVICE_TYPE_PHONE;
		}
		else if(strstr(buf, "pad"))
		{
			return DEVICE_TYPE_TABLET;
		}
		else if(strstr(buf, "desktop")  || strstr(buf, "imac"))
		{
			return DEVICE_TYPE_PC;
		}
		else if(strstr(buf, "macbook"))
		{
			return DEVICE_TYPE_LABTOP;
		}
	}

	return DEVICE_TYPE_OTHER;
}

