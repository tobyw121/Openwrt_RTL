/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 30425 $
 * $Date: 2012-06-29 11:48:48 +0800 (Fri, 29 Jun 2012) $
 *
 * Purpose : Define those public diag shell utility APIs.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Diag shell utility
 */

/*
 * Include Files
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#if defined(__linux__) /* Add the line for eCos, 2010-05-07 Fixed Me!!! */
#include <termio.h>
#endif /* Add the line for eCos, 2010-05-07 Fixed Me!!! */
#include <common/debug/rt_log.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <diag_util.h>
#include <diag_om.h>
/*
 * Symbol Definition
 */
#define MAX_MORE_LINES  20

#define UTIL_STRING_BUFFER_LENGTH       (128)
#define UTIL_PORT_MASK_BUFFER_LENGTH    (16)
#define UTIL_IP_TMP_BUFFER_LENGTH       (4)
#define UTIL_IPV6_TMP_BUFFER_LENGTH     (8)

/*
 * Data Declaration
 */
#if defined(__linux__) /* Add the line for eCos, 2010-05-07 Fixed Me!!! */
static struct termios stored_settings;
#endif /* Add the line for eCos, 2010-05-07 Fixed Me!!! */
static int lines = 0;
static int stopped = 0;

#define _parse_err_return()  \
    do { \
        _diag_util_lPortMask_clear(mask); \
        return RT_ERR_FAILED; \
    } while (0)

#define _s2m_atoi(NUM, PTR, ENDCHAR) \
    do { \
        (NUM) = 0; \
        do { \
            if (!isdigit((int) *(PTR))) \
                return RT_ERR_FAILED; \
            (NUM) = (NUM) * 10 + (int) *(PTR) - (int)'0'; \
        } while (*(++(PTR)) != (ENDCHAR)); \
    } while (0)

/*
 * Function Declaration
 */
static int32 _diag_util_lPortMask_clear(rtk_portmask_t *pstLPortMask);
static int32 _diag_util_port2LPortMask_set(rtk_portmask_t *pstLPortMask, unsigned char ucPortId);
static int32 _diag_util_port2LPortMask_get(rtk_portmask_t *pstLPortMask, unsigned char ucPortId);
static int32 _diag_util_getnext(uint8 *src, int32 separator, uint8 *dest);

static int32
_diag_util_lPortMask_clear(rtk_portmask_t *pstLPortMask)
{
    uint32  i = 0;

    for (i = 0; i < RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST; ++i)
    {
        pstLPortMask->bits[i] = 0;
    }

    return RT_ERR_OK;
} /* end of _diag_util_lPortMask_clear */

static int32
_diag_util_port2LPortMask_set(rtk_portmask_t *pstLPortMask, unsigned char ucPortId)
{
    if (ucPortId > RTK_MAX_NUM_OF_PORTS - 1)
    {
        return RT_ERR_FAILED;
    }

    pstLPortMask->bits[ucPortId / MASK_BIT_LEN] |= (1 << (ucPortId % MASK_BIT_LEN));

    return RT_ERR_OK;
} /* end of _diag_util_port2LPortMask_set */

static int32
_diag_util_port2LPortMask_get(rtk_portmask_t *pstLPortMask, unsigned char ucPortId)
{
    if (ucPortId > RTK_MAX_NUM_OF_PORTS - 1)
    {
        return RT_ERR_FAILED;
    }

    if (pstLPortMask->bits[ucPortId / MASK_BIT_LEN] & (1 << (ucPortId % MASK_BIT_LEN)))
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of _diag_util_port2LPortMask_get */

int32
diag_util_str2LPortMask(uint8 *str, rtk_portmask_t *mask)
{
    uint32   i = 0;
    uint32   num = 0;
    uint32   num_end = 0;
    uint8    *ptr = NULL, *p = NULL;

    if ((NULL == str) || (NULL == mask))
    {
        return RT_ERR_FAILED;
    }

    _diag_util_lPortMask_clear(mask);

    ptr = (uint8 *)strtok((char *)str, ",");
    while (NULL != ptr)
    {
        if (isdigit((int)*ptr))
        {
            p = (uint8 *)strchr((char *)ptr, '-');
            if (NULL == p)
            {   /* number only */
                _s2m_atoi(num, ptr, '\0');
                if (num > MAX_PHY_PORT)
                    _parse_err_return();

                _diag_util_port2LPortMask_set(mask, num);
            }
            else
            {   /* number-number */
                _s2m_atoi(num, ptr, '-');
                ++p;
                _s2m_atoi(num_end, p, '\0');
                if (num > MAX_PHY_PORT || num_end > MAX_PHY_PORT)
                    _parse_err_return();
                if (num_end > num)
                {
                    for (i = num; i <= num_end; i++)
                    {
                        _diag_util_port2LPortMask_set(mask, i);
                    }
                }
                else
                {
                    for (i = num_end; i <= num; i++)
                    {
                        _diag_util_port2LPortMask_set(mask, i);
                    }
                }
            }
        }
        else if (!strncasecmp((char *)ptr, "trunk", 5))
        {
            ptr += 5;
            p = (uint8 *)strchr((char *)ptr, '-');
            if (NULL == p)
            {
                _s2m_atoi(num, ptr, '\0');
                if (num > MAX_TRK_PORT)
                    _parse_err_return ();
                _diag_util_port2LPortMask_set(mask, num + MAX_PHY_N_CPU_PORT);
            }
            else
            {
                _s2m_atoi(num, ptr, '-');
                ++p;
                _s2m_atoi(num_end, p, '\0');
                if (num > MAX_TRK_PORT || num_end > MAX_TRK_PORT)
                    _parse_err_return();
                if (num_end > num)
                {
                    for (i = num; i <= num_end; i++)
                    {
                        _diag_util_port2LPortMask_set(mask, i + MAX_PHY_N_CPU_PORT);
                    }
                }
                else
                {
                    for (i = num_end; i <= num; i++)
                    {
                        _diag_util_port2LPortMask_set(mask, i + MAX_PHY_N_CPU_PORT);
                    }
                }
            }
        }
        else
            _parse_err_return();

        ptr = (uint8 *)strtok(NULL, ",");
    }
    return RT_ERR_OK;
} /* end of diag_util_str2LPortMask */

/* convert logical port mask to string, separated by ","s, string length of comma is DIAG_UTIL_PORT_MASK_STRING_LEN */
int32 diag_util_lPortMask2str (uint8 *comma, rtk_portmask_t *pstLPortMask)
{
    int32   first = 0;
    int32   begin = 0;
    int32   end = 0;
    uint32  i = 0;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];

    if ((NULL == comma) || (NULL == pstLPortMask))
    {
        return RT_ERR_FAILED;
    }

    memset(buf, 0, UTIL_PORT_MASK_BUFFER_LENGTH);

    comma[0] = '\0';

    first = 1;
    begin = -1;
    end = -1;

    for (i = 0; i <= MAX_PHY_PORT; ++i)
    {
        if (RT_ERR_OK == _diag_util_port2LPortMask_get(pstLPortMask, i))
        {

            if (1 == first)
            {
                first = 0;
            }

            if (-1 == begin)
            {
                begin = end = i;
            }
            else
            {
                end = i;
            }

        }
        else
        {
            if ((0 == first) && (begin != -1))
            {
                first = -1;
            }
            else if ((-1 == first) && (begin != -1))
            {
                sprintf((char *)buf, ",");
                if ((strlen((char *)comma) + strlen((char *)buf)) > DIAG_UTIL_PORT_MASK_STRING_LEN)
                {
                    return RT_ERR_FAILED;
                }
                strcat((char *)comma, (char *)buf);
            }

            if ((begin != -1) && (begin == end))
            {
                sprintf((char *)buf, "%d", begin);
                if ((strlen((char *)comma) + strlen((char *)buf)) > DIAG_UTIL_PORT_MASK_STRING_LEN)
                {
                    return RT_ERR_FAILED;
                }
                strcat((char *)comma, (char *)buf);
            }
            else if (begin != -1)
            {
                sprintf((char *)buf, "%d-%d", begin, end);
                if ((strlen((char *)comma) + strlen((char *)buf)) > DIAG_UTIL_PORT_MASK_STRING_LEN)
                {
                    return RT_ERR_FAILED;
                }
                strcat((char *)comma, (char *)buf);
            }

            begin = -1;
            end = -1;
        }
    }

    if ((begin != -1) || (end != -1))
    {
        if (-1 == first)
        {
            sprintf((char *)buf, ",");
            if ((strlen((char *)comma) + strlen((char *)buf)) > DIAG_UTIL_PORT_MASK_STRING_LEN)
            {
                return RT_ERR_FAILED;
            }
            strcat((char *)comma, (char *)buf);
        }
        if (begin == end)
        {
            sprintf((char *)buf, "%d", begin);
            if ((strlen((char *)comma) + strlen((char *)buf)) > DIAG_UTIL_PORT_MASK_STRING_LEN)
            {
                return RT_ERR_FAILED;
            }
            strcat((char *)comma, (char *)buf);
        }
        else
        {
            sprintf((char *)buf, "%d-%d", begin, end);
            if ((strlen((char *)comma) + strlen((char *)buf)) > DIAG_UTIL_PORT_MASK_STRING_LEN)
            {
                return RT_ERR_FAILED;
            }
            strcat((char *)comma, (char *)buf);
        }
    }
#if 0
    for (i = MAX_PHY_N_CPU_PORT; i < MAX_LOGIC_PORT; ++i)
    {
        if (RT_ERR_OK == _diag_util_port2LPortMask_get(pstLPortMask, i))
        {
            if (1 == first)
            {
                first = 0;
                sprintf(buf, "Trunk%d", i - MAX_PORT - 1 + 1);

            }
            else
            {
                sprintf(buf, ",Trunk%d", i - MAX_PORT - 1 + 1);
            }
            strcat(comma, buf);
        }
    }
#endif
    return RT_ERR_OK;
} /* end of diag_util_lPortMask2str */

int32 diag_util_extract_portlist(uint32 unit, uint8 *pStr, uint32 type, diag_portlist_t *pPortlist)
{
    int32 ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t devInfo;

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));

    if ((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return ret;
    }

    if('a' == pStr[0])
    {
        pPortlist->min = devInfo.ether.min;
        pPortlist->max = devInfo.ether.max;
        memcpy(&pPortlist->portmask, &(devInfo.ether.portmask), sizeof(rtk_portmask_t));
    }
    else
    {
        switch(type)
        {
            case DIAG_PORTTYPE_FE:
                pPortlist->min = devInfo.fe.min;
                pPortlist->max = devInfo.fe.max;
                break;

            case DIAG_PORTTYPE_GE:
                pPortlist->min = devInfo.ge.min;
                pPortlist->max = devInfo.ge.max;
                break;

            case DIAG_PORTTYPE_GE_COMBO:
                pPortlist->min = devInfo.ge_combo.min;
                pPortlist->max = devInfo.ge_combo.max;
                break;

            case DIAG_PORTTYPE_SERDES:
                pPortlist->min = devInfo.serdes.min;
                pPortlist->max = devInfo.serdes.max;
                break;

            case DIAG_PORTTYPE_10GE:
                pPortlist->min = devInfo.ge_10ge.min;
                pPortlist->max = devInfo.ge_10ge.max;
                break;

            case DIAG_PORTTYPE_ETHER:
                pPortlist->min = devInfo.ether.min;
                pPortlist->max = devInfo.ether.max;
                break;

            case DIAG_PORTTYPE_ALL:
                pPortlist->min = devInfo.all.min;
                pPortlist->max = devInfo.all.max;
                break;

            default:
                diag_util_printf("port type input ERROR!\n");
                return RT_ERR_FAILED;
        }
        if ((ret = diag_util_str2LPortMask(pStr, &pPortlist->portmask)) != RT_ERR_OK)
        {
            diag_util_printf("port list ERROR!\n");
            RT_ERR(ret, (MOD_DIAGSHELL), "port list=%s", pStr);
            return ret;
        }
    }

    return ret;
}

static int32
_diag_util_mask_clear(diag_bmp_t *pBmp)
{
    uint32  i = 0;

    for (i = 0; i < DIGA_BMP_WIDTH(DIAG_MASK_MAX_LEN); ++i)
    {
        pBmp->bits[i] = 0;
    }

    return RT_ERR_OK;
} /* end of _diag_util_mask_clear */

static int32
_diag_util_index2Mask_set(diag_bmp_t *pBmp, unsigned char index)
{
    if (index > DIAG_MASK_MAX_LEN - 1)
    {
        return RT_ERR_FAILED;
    }

    pBmp->bits[index / DIAG_BMP_BIT_LEN] |= (1 << (index % DIAG_BMP_BIT_LEN));

    return RT_ERR_OK;
} /* end of _diag_util_index2Mask_set */

int32 diag_util_str2Mask(uint8 *str, diag_mask_t *pMask)
{
    uint32      i = 0;
    uint32      num = 0;
    uint32      num_end = 0;
    uint8       *ptr = NULL, *p = NULL;
    diag_bmp_t  *mask;

    if ((NULL == str) || (NULL == pMask))
    {
        return RT_ERR_FAILED;
    }

    mask = &pMask->mask;

    _diag_util_mask_clear(mask);

    ptr = (uint8 *)strtok((char *)str, (char *)",");
    while (NULL != ptr)
    {
        if (isdigit((int)*ptr))
        {
            p = (uint8 *)strchr((char *)ptr, '-');
            if (NULL == p)
            {   /* number only */
                _s2m_atoi(num, ptr, '\0');
                if (num < pMask->min || num > pMask->max)
                    return RT_ERR_FAILED;

                _diag_util_index2Mask_set(mask, num);
            }
            else
            {   /* number-number */
                _s2m_atoi(num, ptr, '-');
                ++p;
                _s2m_atoi(num_end, p, '\0');

                if (num < pMask->min || num > pMask->max)
                    return RT_ERR_FAILED;

                if (num_end < pMask->min || num_end > pMask->max)
                    return RT_ERR_FAILED;

                if (num_end > num)
                {
                    for (i = num; i <= num_end; i++)
                    {
                        _diag_util_index2Mask_set(mask, i);
                    }
                }
                else
                {
                    for (i = num_end; i <= num; i++)
                    {
                        _diag_util_index2Mask_set(mask, i);
                    }
                }
            }
        }
        else
            return RT_ERR_FAILED;

        ptr = (uint8 *)strtok((char *)NULL, ",");
    }
    return RT_ERR_OK;
} /* end of diag_util_str2Mask */

int32 diag_util_extract_mask(uint32 unit, uint8 *pStr, uint32 type,
    diag_mask_t *pMask)
{
    uint32                  i;
    int32                   ret = RT_ERR_FAILED;
    rtk_switch_devInfo_t    devInfo;

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));

    if ((ret = diag_om_get_deviceInfo(unit, &devInfo)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return ret;
    }

    switch (type)
    {
        case DIAG_MASKTYPE_QUEUE:
            pMask->min = 0;
            pMask->max = devInfo.capacityInfo.max_num_of_queue - 1;
            break;
        case DIAG_MASKTYPE_DSCP:
            pMask->min = 0;
            pMask->max = RTK_VALUE_OF_DSCP_MAX;
            break;
        default:
            diag_util_printf("mask type input ERROR!\n");
            return RT_ERR_FAILED;
    }

    if('a' == pStr[0])
    {
        for (i = pMask->min; i <= pMask->max; ++i)
        {
            _diag_util_index2Mask_set(&pMask->mask, i);
        }
    }
    else
    {
        if ((ret = diag_util_str2Mask(pStr, pMask)) != RT_ERR_OK)
        {
            diag_util_printf("mask ERROR!\n");
            RT_ERR(ret, (MOD_DIAGSHELL), "mask=%s", pStr);
            return ret;
        }
    }

    return ret;
}

/*
 * On success, the function returns the converted integral number as a unsigned long int value.
 * If no valid conversion could be performed, a zero value is returned.
 */
int32
diag_util_str2ul(uint32 *ul, const char *str)
{
    uint32 value, base= 10;

    if ((NULL == ul) || (NULL == str))
    {
        return RT_ERR_FAILED;
    }

    if(('0' == str[0]) && ('X' == toupper(str[1])))
    {
        str += 2;
        base = 16;
    }

    while(isxdigit(*str) && (value = isdigit(*str) ? (*str - '0') : (toupper(*str) - 'A' + 10)) < base)
    {
        *ul = (*ul * base) + value;
        str++;
    }

    return RT_ERR_OK;
}

/*
 * getnext -- get the next token
 *
 * Parameters:
 *   src: pointer to the start of the source string
 *   separater: the symbol used to separate the token
 *   dest: destination of the next token to be placed
 *
 * Returns:
 *   length of token (-1 when failed)
 */
static int32
_diag_util_getnext (uint8 *src, int32 separator, uint8 *dest)
{
    int32   len = 0;
    uint8   *c = NULL;

    if ((NULL == src) || (NULL == dest))
    {
        return -1;
    }

    c = (uint8 *)strchr((char *)src, separator);
    if (NULL == c)
    {
        strcpy((char *)dest, (char *)src);
        return -1;
    }
    len = c - src;
    strncpy((char *)dest, (char *)src, len);
    dest[len] = '\0';

    return len + 1;
} /* end of _diag_util_getnext */

/* Convert MAC address from string to unsigned char array */
int32
diag_util_str2mac (uint8 *mac, uint8 *str)
{
    int32    len = 0;
    uint32   i = 0;
    uint8    *ptr = str;
    uint8    buf[UTIL_STRING_BUFFER_LENGTH];

    if ((NULL == mac) || (NULL == str))
    {
        return RT_ERR_FAILED;
    }

    memset(buf, 0, UTIL_STRING_BUFFER_LENGTH);

    for (i = 0; i < 5; ++i)
    {
        if ((len = _diag_util_getnext(ptr, ':', buf)) == -1 &&
            (len = _diag_util_getnext(ptr, '-', buf)) == -1)
        {
            return RT_ERR_FAILED; /* parse error */
        }
        mac[i] = strtol((char *)buf, NULL, 16);
        ptr += len;
    }
    mac[5] = strtol((char *)ptr, NULL, 16);

    return RT_ERR_OK;
} /* end of diag_util_str2mac */

int32
diag_util_mac2str (uint8 *str, const uint8 *mac)
{
    if ((NULL == mac) || (NULL == str))
    {
        return RT_ERR_FAILED;
    }

    sprintf((char *)str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return RT_ERR_OK;
} /* end of diag_util_mac2str */

/* convert IP address from number to string. Length of ip_str must more than 15 characters*/
int32
diag_util_ip2str(uint8 *str, uint32 ip)
{
    if (NULL == str)
    {
        return RT_ERR_FAILED;
    }

    sprintf((char *)str, "%d.%d.%d.%d", ((ip>>24)&0xff), ((ip>>16)&0xff), ((ip>>8)&0xff), (ip&0xff));

    return RT_ERR_OK;
}

/* convert IP address from string to number */
int32
diag_util_str2ip (uint32 *ip, uint8 *str)
{
    int32   len = 0;
    uint32  i = 0;
    uint32  ip_tmp_buf[UTIL_IP_TMP_BUFFER_LENGTH];
    uint8   *ptr = str;
    uint8   buf[UTIL_STRING_BUFFER_LENGTH];

    if ((NULL == ip) || (NULL == str))
    {
        return RT_ERR_FAILED;
    }

    memset(ip_tmp_buf, 0, sizeof(uint32) * UTIL_IP_TMP_BUFFER_LENGTH);
    memset(buf, 0, UTIL_STRING_BUFFER_LENGTH);

    for (i = 0; i < 3; ++i)
    {
        if ((len = _diag_util_getnext(ptr, '.', buf)) == -1)
        {
            return RT_ERR_FAILED; /* parse error */
        }

        ip_tmp_buf[i] = atoi((char *)buf);
        if ((ip_tmp_buf[i] < 0) || (ip_tmp_buf[i] > 255))
        {
            return RT_ERR_FAILED; /* parse error */
        }

        ptr += len;
    }
    ip_tmp_buf[3] = atoi((char *)ptr);
    if ((ip_tmp_buf[3] < 0) || (ip_tmp_buf[3] > 255))
    {
        return RT_ERR_FAILED; /* parse error */
    }

    *ip = (ip_tmp_buf[0] << 24) + (ip_tmp_buf[1] << 16) + (ip_tmp_buf[2] << 8) + ip_tmp_buf[3];
    return RT_ERR_OK;
} /* end of diag_util_str2Ip */

/* convert IPv6 address from number to string. Length of ipv6_str must more than 39 characters*/
int32
diag_util_ipv62str(uint8 *str, const uint8 *ipv6)
{
    uint32  i;
    uint16  ipv6_ptr[UTIL_IPV6_TMP_BUFFER_LENGTH] = {0};

    if ((NULL == str) || (NULL == ipv6))
    {
        return RT_ERR_FAILED;
    }

    for (i = 0; i < UTIL_IPV6_TMP_BUFFER_LENGTH ;i++)
    {
        ipv6_ptr[i] = ipv6[i*2+1];
        ipv6_ptr[i] |=  ipv6[i*2] << 8;
    }
    sprintf((char *)str, "%x:%x:%x:%x:%x:%x:%x:%x", ipv6_ptr[0], ipv6_ptr[1], ipv6_ptr[2], ipv6_ptr[3]
    , ipv6_ptr[4], ipv6_ptr[5], ipv6_ptr[6], ipv6_ptr[7]);

    return RT_ERR_OK;
}

/* convert IPv6 address from string to number. Length of ipv6_addr must more than 16 characters */
int32
diag_util_str2ipv6(uint8 *ipv6, const uint8 *str)
{
#define IN6ADDRSZ 16
#define INT16SZ     2
    static const uint8 xdigits_l[] = "0123456789abcdef",
              xdigits_u[] = "0123456789ABCDEF";
    uint8 tmp[IN6ADDRSZ], *tp, *endp, *colonp;
    const uint8 *xdigits, *curtok;
    int ch, saw_xdigit;
    int val;

    if ((NULL == str) || (NULL == ipv6))
    {
        return RT_ERR_FAILED;
    }

    memset((tp = tmp), '\0', IN6ADDRSZ);
    endp = tp + IN6ADDRSZ;
    colonp = NULL;
    /* Leading :: requires some special handling. */
    if (*str == ':')
        if (*++str != ':')
            return (RT_ERR_FAILED);
    curtok = str;
    saw_xdigit = 0;
    val = 0;
    while ((ch = *str++) != '\0') {
        const uint8 *pch;

        if ((pch = (uint8 *)strchr((char *)(xdigits = xdigits_l), ch)) == NULL)
            pch = (uint8 *)strchr((char *)(xdigits = xdigits_u), ch);
        if (pch != NULL) {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return (RT_ERR_FAILED);
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':') {
            curtok = str;
            if (!saw_xdigit) {
                if (colonp)
                    return (RT_ERR_FAILED);
                colonp = tp;
                continue;
            }
            if (tp + INT16SZ > endp)
                return (RT_ERR_FAILED);
            *tp++ = (uint8) (val >> 8) & 0xff;
            *tp++ = (uint8) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
#if 0
        if (ch == '.' && ((tp + INADDRSZ) <= endp) &&
            inet_pton4(curtok, tp) > 0) {
            tp += INADDRSZ;
            saw_xdigit = 0;
            break;  /* '\0' was seen by inet_pton4(). */
        }
#endif
        return (RT_ERR_FAILED);
    }
    if (saw_xdigit) {
        if (tp + INT16SZ > endp)
            return (RT_ERR_FAILED);
        *tp++ = (uint8) (val >> 8) & 0xff;
        *tp++ = (uint8) val & 0xff;
    }
    if (colonp != NULL) {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;
        int i;

        for (i = 1; i <= n; i++) {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return (RT_ERR_FAILED);
    memcpy(ipv6, tmp, IN6ADDRSZ);
    return (RT_ERR_OK);
}/* end of diag_util_str2Ipv6 */

int32
diag_util_ip2str_format(uint8 *str, uint32 ip, uint32 blankWidth)
{
    uint8   i;
    uint8   len;
    
    if (NULL == str)
    {
        return RT_ERR_FAILED;
    }

    sprintf((char *)str, "%d.%d.%d.%d", ((ip>>24)&0xff), ((ip>>16)&0xff), ((ip>>8)&0xff), (ip&0xff));
    len = strlen((char *)str);

    if (blankWidth > len)
    {
        for (i = 0; i < (blankWidth - len);i++)
        {
            strcat((char *)str, " ");
        }
    }

    return RT_ERR_OK;
}

/* Check if the MAC address is a broadcast address or not */
int32
diag_util_isBcastMacAddr(uint8 *mac)
{
    uint32 i = 0;

    if (NULL == mac)
    {
        return FALSE;
    }

    for (i = 0; i < 6; i++)
    {
        if (0xFF == *(mac + i))
        {
            continue;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
} /* end of diag_util_isBcastMacAddr */

/* Check if the MAC address is a multicast address or not */
int32
diag_util_isMcastMacAddr(uint8 *mac)
{
    if (NULL == mac)
    {
        return FALSE;
    }

    if (0x1 == (mac[0] & 0x1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
} /* end of diag_util_isMcastMacAddr */

static void
diag_util_set_keypress (void)
{
#if defined(__linux__) /* Add the line for eCos, 2010-05-07 Fixed Me!!! */
    struct termios  new_settings;

#if defined(__linux__)
    tcgetattr(0, &stored_settings);
#endif
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_lflag &= (~ECHO);
    new_settings.c_cc[VTIME] = 0;
#if defined(__linux__)
    tcgetattr(0, &stored_settings);
#endif
    new_settings.c_cc[VMIN] = 1;
#if defined(__linux__)
    tcsetattr(0, TCSANOW, &new_settings);
#endif
#endif /* Add the line for eCos, 2010-05-07 Fixed Me!!! */
    return;
} /* end of diag_util_set_keypress */

static void
diag_util_reset_keypress(void)
{
#if defined(__linux__)
    tcsetattr(0, TCSANOW, &stored_settings);
#endif
    return;
} /* end of diag_util_reset_keypress */

void
diag_util_mprintf(char *fmt, ...)
{
    va_list     args;

    if (stopped)
    {
        return;
    }

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    ++lines;
    if (lines > MAX_MORE_LINES)
    {
        char    ch;

        lines = 1;
        printf("\n--More--");
        diag_util_set_keypress();
        ch = getchar() & 0xFF;
        putchar(8);
        diag_util_reset_keypress();
        printf("\n");
        if (('Q' == ch) || ('q' == ch))
        {
            stopped = 1;
            return;
        }
    }
    return;
} /* end of diag_util_mprintf */

void
diag_util_mprintf_init (void)
{
    stopped = 0;
    lines = 0;
    return;
} /* end of diag_util_mprintf_init */


