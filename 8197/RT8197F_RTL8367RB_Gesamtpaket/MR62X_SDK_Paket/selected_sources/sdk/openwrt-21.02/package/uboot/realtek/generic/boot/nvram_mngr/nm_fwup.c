/*! Copyright(c) 1996-2009 Shenzhen TP-LINK Technologies Co. Ltd.
 * \file    nm_fwup.c
 * \brief   Implements for upgrade firmware to NVRAM.
 * \author  Meng Qing
 * \version 1.0
 * \date    21/05/2009
 */


/**************************************************************************************************/
/*                                      CONFIGURATIONS                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      INCLUDE_FILES                                             */
/**************************************************************************************************/
//#include <common.h>
#include "nm_utils.h"
#include "nm_api.h"
#include "nm_lib.h"
#include "nm_fwup.h"
#include "sysProductInfo.h"
#include "string.h"
#include "md5.h"
#include "linux/delay.h"


// #define CONFIG_FIRMWARE_NOCHECK
/**************************************************************************************************/
/*                                      DEFINES                                                   */
/**************************************************************************************************/
/* Porting memory managing utils. */
#define fflush(stdout) 

/**************************************************************************************************/
/*                                      TYPES                                                     */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      EXTERN_PROTOTYPES                                         */
/**************************************************************************************************/
STATUS nm_initFwupPtnStruct(void);
STATUS nm_getDataFromFwupFile(NM_PTN_STRUCT *ptnStruct, char *fwupPtnIndex, char *fwupFileBase);
STATUS nm_getDataFromNvram(NM_PTN_STRUCT *ptnStruct, NM_PTN_STRUCT *runtimePtnStruct);
STATUS nm_updateDataToNvram(NM_PTN_STRUCT *ptnStruct);
STATUS nm_updateRuntimePtnTable(NM_PTN_STRUCT *ptnStruct, NM_PTN_STRUCT *runtimePtnStruct);
static int nm_checkSupportList(char *support_list, int len);
STATUS nm_checkUpdateContent(NM_PTN_STRUCT *ptnStruct, char *pAppBuf, int nFileBytes, int *errorCode);
STATUS nm_cleanupPtnContentCache(void);
int nm_buildUpgradeStruct(char *pAppBuf, int nFileBytes);
STATUS nm_upgradeFwupFile(char *pAppBuf, int nFileBytes);

/**************************************************************************************************/
/*                                      LOCAL_PROTOTYPES                                          */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      VARIABLES                                                 */
/**************************************************************************************************/
int g_fwupPercent = 0 ;
NM_STR_MAP nm_fwupPtnIndexFileParaStrMap[] =
{
    {NM_FWUP_PTN_INDEX_PARA_ID_NAME,    "fwup-ptn"},
    {NM_FWUP_PTN_INDEX_PARA_ID_BASE,    "base"},
    {NM_FWUP_PTN_INDEX_PARA_ID_SIZE,    "size"},

    {-1,                                NULL}
};

static unsigned char md5Key[IMAGE_SIZE_MD5] = 
{
	0x7a, 0x2b, 0x15, 0xed,  0x9b, 0x98, 0x59, 0x6d,
	0xe5, 0x04, 0xab, 0x44,  0xac, 0x2a, 0x9f, 0x4e
};

#ifndef CONFIG_FIRMWARE_NOCHECK
static unsigned char rsaPubKey[] = "BgIAAACkAABSU0ExAAQAAAEAAQD9lxDCQ5DFNSYJBriTmTmZlEMYVgGcZTO+AIwm" \
				"dVjhaeJI6wWtN7DqCaHQlOqJ2xvKNrLB+wA1NxUh7VDViymotq/+9QDf7qEtJHmesji" \
				"rvPN6Hfrf+FO4/hmjbVXgytHORxGta5KW4QHVIwyMSVPOvMC4A5lFIh+D1kJW5GXWtA==";
#endif

int handle_fw_cloud(unsigned char *buf, int buf_len);

static struct fw_type_option fw_type_array[] =
{
	{	"Cloud",	FW_TYPE_CLOUD,		handle_fw_cloud}, /*don't use ->func to call in uboot!!!*/
	{	NULL,	FW_TYPE_INVALID,	NULL}	/* end entry of  fw_type_array */
};

NM_PTN_STRUCT *g_nmFwupPtnStruct;
NM_PTN_STRUCT g_nmFwupPtnStructEntity;
int g_nmCountFwupCurrWriteBytes;
int g_nmCountFwupAllWriteBytes;

STATUS g_nmUpgradeResult;

char *ptnContentCache[NM_PTN_NUM_MAX];

char *need_erase_ptn[] = {"NULL"};

/**************************************************************************************************/
/*                                      LOCAL_FUNCTIONS                                           */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      PUBLIC_FUNCTIONS                                          */
/**************************************************************************************************/


enum fw_type gset_fw_type(enum fw_type type)
{
	static enum fw_type curr_fw_type = FW_TYPE_COMMON;

	if (FW_TYPE_INVALID < type && type < FW_TYPE_MAX)
		curr_fw_type = type;

	return curr_fw_type;
}

STATUS nm_tpFirmwareMd5Check(unsigned char *ptr,int bufsize)
{
	unsigned char fileMd5Checksum[IMAGE_SIZE_MD5];
	unsigned char digst[IMAGE_SIZE_MD5];
	
	memcpy(fileMd5Checksum, ptr + IMAGE_SIZE_LEN, IMAGE_SIZE_MD5);
	memcpy(ptr + IMAGE_SIZE_LEN, md5Key, IMAGE_SIZE_MD5);

	md5_make_digest(digst, ptr + IMAGE_SIZE_LEN, bufsize - IMAGE_SIZE_LEN);

	if (0 != memcmp(digst, fileMd5Checksum, IMAGE_SIZE_MD5))
	{
		NM_ERROR("Check md5 error.\n");
		return -1;
	}

	memcpy(ptr + IMAGE_SIZE_LEN, fileMd5Checksum, IMAGE_SIZE_MD5);

	return 0;
}

int handle_fw_cloud(unsigned char *buf, int buf_len)
{	
#ifdef CONFIG_FIRMWARE_NOCHECK
	NM_INFO("%s %d\n",__FUNCTION__,__LINE__);
	return nm_tpFirmwareMd5Check(buf, buf_len);
#else	
	unsigned char md5_dig[IMAGE_SIZE_MD5];
	unsigned char sig_buf[IMAGE_LEN_RSA_SIG];
	unsigned char tmp_rsa_sig[IMAGE_LEN_RSA_SIG];
	int ret = 0;

	NM_INFO("%s %d\n",__FUNCTION__,__LINE__);

	/*backup data*/
	memcpy(tmp_rsa_sig,buf + IMAGE_SIZE_RSA_SIG,IMAGE_LEN_RSA_SIG);
	memcpy(sig_buf, buf + IMAGE_SIZE_RSA_SIG, IMAGE_LEN_RSA_SIG);
	
	/* fill with 0x0 */
	memset(buf + IMAGE_SIZE_RSA_SIG, 0x0, IMAGE_LEN_RSA_SIG);

	md5_make_digest(md5_dig, buf + IMAGE_SIZE_FWTYPE, buf_len - IMAGE_SIZE_FWTYPE);
    hex_dump(md5_dig, IMAGE_SIZE_MD5);
	ret = rsaVerifySignByBase64EncodePublicKeyBlob(rsaPubKey, strlen((char *)rsaPubKey),
                md5_dig, IMAGE_SIZE_MD5, sig_buf, IMAGE_LEN_RSA_SIG);

	memcpy(buf + IMAGE_SIZE_RSA_SIG,tmp_rsa_sig,IMAGE_LEN_RSA_SIG);

	if (0 == ret)
	{
		NM_ERROR("Check rsa error.\n");
		return -1;
	}
	else
	{
        NM_ERROR("Check rsa OK.\n");
		return 0;
	}
#endif
}

STATUS nm_tpFirmwareFindType(char *ptr, int len, char *buf, int buf_len)
{
	int end = 0;
	int begin = 0;
	char *pBuf = NULL;
	char *type = "fw-type:"; //the fw type stored as "fw-type:cloud\n"

	if (buf_len < IMAGE_CLOUD_HEAD_OFFSET || 
		len < (IMAGE_SIZE_FWTYPE + IMAGE_CLOUD_HEAD_OFFSET))
	{
		return -1;
	}
	
	pBuf = ptr + IMAGE_SIZE_FWTYPE;

	//find the fw type name begin and end
	while (*(pBuf + end) != '\n' && end < IMAGE_CLOUD_HEAD_OFFSET)
	{
		if (begin < strlen(type))
		{			
			if (*(pBuf + end) != type[begin])
			{
				return -1;
			}

			begin++;
		}

		end++;
	}

	if (end >= IMAGE_CLOUD_HEAD_OFFSET || begin != strlen(type) || end <= begin) 
		return -1;

	//copy to the fw type name buffer
	memcpy(buf, pBuf + begin, end - begin);

	return 0;
}

STATUS nm_tpFirmwareVerify(unsigned char *ptr,int len)
{
	int ret;
	char fw_type_name[FW_TYPE_NAME_LEN_MAX];
	struct fw_type_option *ptr_fw_type = NULL;
	
	memset(fw_type_name, 0x0, FW_TYPE_NAME_LEN_MAX);
	nm_tpFirmwareFindType((char *)ptr, len, fw_type_name, FW_TYPE_NAME_LEN_MAX);
	NM_INFO("fw type name : %s.\n", fw_type_name);

	//get firmware type
	for (ptr_fw_type = fw_type_array; ; ++ptr_fw_type)
	{
		if (!ptr_fw_type || !ptr_fw_type->name)
		{
			gset_fw_type(FW_TYPE_COMMON);
			break;
		}
		
		if (!strcmp(ptr_fw_type->name, fw_type_name))
		{
			gset_fw_type(ptr_fw_type->type);
			break;
		}
	}

	if (gset_fw_type(FW_TYPE_INVALID) == FW_TYPE_COMMON)
	{
		//common firmware MD5 check
		NM_INFO("Firmware process common.\r\n");
		ret = nm_tpFirmwareMd5Check(ptr, len);
	}
	else
	{
		NM_INFO("Firmware process id %d.\r\n", gset_fw_type(FW_TYPE_INVALID));
		// don't use ptr_fw_type->func,because uboot is relocated!!!
		ret = handle_fw_cloud(ptr,len);		
	}

	if ( ret < 0 )
	{
		return NM_FWUP_ERROR_INVALID_FILE;
	}

	NM_INFO("Image verify OK!\r\n");

	return OK;
}

/*******************************************************************
 * Name		: nm_initFwupPtnStruct
 * Abstract	: Initialize partition-struct.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
STATUS nm_initFwupPtnStruct()
{
    int index = 0;

    prom_printf("%s:%d\n",__FUNCTION__,__LINE__);	
    
    memset(&g_nmFwupPtnStructEntity, 0, sizeof(g_nmFwupPtnStructEntity));
    g_nmFwupPtnStruct = &g_nmFwupPtnStructEntity;


    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {           
        if (ptnContentCache[index] != NULL)
        {
            ptnContentCache[index] = NULL;
        }   
    }
    
    return OK;
}


/*******************************************************************
 * Name		: nm_getDataFromFwupFile
 * Abstract	: 
 * Input	: fwupFileBase: start addr of FwupPtnTable
 * Output	: 
 * Return	: OK/ERROR.
 */
STATUS nm_getDataFromFwupFile(NM_PTN_STRUCT *ptnStruct, char *fwupPtnIndex, char *fwupFileBase)
{   
    int index = 0;
    int paraId = -1;
    int argc;
    char *argv[NM_FWUP_PTN_INDEX_ARG_NUM_MAX];
    NM_PTN_ENTRY *currPtnEntry = NULL;

    argc = nm_lib_makeArgs(fwupPtnIndex, argv, NM_FWUP_PTN_INDEX_ARG_NUM_MAX);
    
    while (index < argc)
    {
        if ((paraId = nm_lib_strToKey(nm_fwupPtnIndexFileParaStrMap, argv[index])) < 0)
        {
            //NM_ERROR("invalid partition-index-file para id.\r\n");
			NM_ERROR("[%d]%s 0x%08x 0x%08x\n",index,argv[index],(int)fwupPtnIndex,(int)ptnStruct);
			goto error;
        }

        index++;

        switch (paraId)
        {
        case NM_FWUP_PTN_INDEX_PARA_ID_NAME:
            /* we only update upgrade-info to partitions exist in partition-table */
            currPtnEntry = nm_lib_ptnNameToEntry(ptnStruct, argv[index]);

            if (currPtnEntry == NULL)
            {
                NM_INFO("partition name not found.");
                continue;           
            }

            if (currPtnEntry->upgradeInfo.dataType == NM_FWUP_UPGRADE_DATA_TYPE_BLANK)
            {
                currPtnEntry->upgradeInfo.dataType = NM_FWUP_UPGRADE_DATA_FROM_FWUP_FILE;
            }
            index++;
            break;
            
        case NM_FWUP_PTN_INDEX_PARA_ID_BASE:
            /* get data-offset in fwupFile */
            if (nm_lib_parseU32((NM_UINT32 *)&currPtnEntry->upgradeInfo.dataStart, argv[index]) < 0)
            {
                NM_ERROR("parse upgradeInfo start value failed.");
                goto error;
            }
            
            currPtnEntry->upgradeInfo.dataStart += (unsigned int)fwupFileBase;
            index++;
            break;

        case NM_FWUP_PTN_INDEX_PARA_ID_SIZE:
            if (nm_lib_parseU32((NM_UINT32 *)&currPtnEntry->upgradeInfo.dataLen, argv[index]) < 0)
            {
                NM_ERROR("parse upgradeInfo len value failed.");
                goto error;
            }
            index++;
            break;

        default:
            NM_ERROR("invalid para id.");
            goto error;
            break;
        }
        
    }

    /* force get partition-table from fwup-file */
    currPtnEntry = nm_lib_ptnNameToEntry(ptnStruct, NM_PTN_NAME_PTN_TABLE); 
    if (currPtnEntry == NULL)
    {
        NM_ERROR("no partition-table in fwup-file.\r\n");
        goto error; 
    }

    currPtnEntry->upgradeInfo.dataType = NM_FWUP_UPGRADE_DATA_FROM_FWUP_FILE;
    currPtnEntry->upgradeInfo.dataStart = (unsigned int)fwupFileBase + NM_FWUP_PTN_INDEX_SIZE;
    /* length of partition-table is "probe to os-image"(4 bytes) and ptn-index-file(string) */
    currPtnEntry->upgradeInfo.dataLen = sizeof(int) + strlen((char*)(currPtnEntry->upgradeInfo.dataStart + sizeof(int)));
    
    return OK;
error:
    return ERROR;
}



/*******************************************************************
 * Name		: nm_getDataFromNvram
 * Abstract	: 
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR.
 */
STATUS nm_getDataFromNvram(NM_PTN_STRUCT *ptnStruct, NM_PTN_STRUCT *runtimePtnStruct)
{   
    int index = 0;
    NM_PTN_ENTRY *currPtnEntry = NULL;
    NM_PTN_ENTRY *tmpPtnEntry = NULL;
    NM_UINT32 readSize = 0;
    
	if (ptnStruct == NULL)
	{
        NM_ERROR("invalid input ptnStruct.");
        goto error;		
	}

    nm_cleanupPtnContentCache();   

    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {       
#if 0		
        currPtnEntry = nm_lib_ptnNameToEntry(ptnStruct, runtimePtnStruct->entries[index].name);

        if (currPtnEntry == NULL)
        {
            continue;           
        }

        if (currPtnEntry->upgradeInfo.dataType == NM_FWUP_UPGRADE_DATA_TYPE_BLANK)
        {
			/* if base not changed, do nothing */
			if (currPtnEntry->base == runtimePtnStruct->entries[index].base)
			{
				currPtnEntry->upgradeInfo.dataType = NM_FWUP_UPGRADE_DATA_TYPE_NO_CHANGE;
				continue;
			}
            /* read content from NVRAM to a memory cache */
            readSize = 0;
            if(currPtnEntry->size <= runtimePtnStruct->entries[index].size)
            {
                readSize = currPtnEntry->size;
            }
            else
            {
                readSize = runtimePtnStruct->entries[index].size;
            }
            //ptnContentCache[index] = malloc(runtimePtnStruct->entries[index].size);
            ptnContentCache[index] = malloc(readSize);

            if (ptnContentCache[index] == NULL)
            {
                NM_ERROR("memory malloc failed.");
                goto error;
            }
            
            //memset(ptnContentCache[index], 0, runtimePtnStruct->entries[index].size);
            memset(ptnContentCache[index], 0, readSize);

            if (nm_lib_readHeadlessPtnFromNvram((char *)runtimePtnStruct->entries[index].base, 
                                                ptnContentCache[index], readSize) < 0)
            {               
                NM_ERROR("get data from NVRAM failed.");
                goto error;
            }

            currPtnEntry->upgradeInfo.dataStart = (unsigned int)ptnContentCache[index];
            currPtnEntry->upgradeInfo.dataLen = readSize;
            currPtnEntry->upgradeInfo.dataType = NM_FWUP_UPGRADE_DATA_FROM_NVRAM;
        }
#else
		tmpPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
		if (tmpPtnEntry->upgradeInfo.dataType == NM_FWUP_UPGRADE_DATA_TYPE_BLANK)
		{
			/* if not in nvram */
			currPtnEntry = nm_lib_ptnNameToEntry(runtimePtnStruct, tmpPtnEntry->name);
			if (currPtnEntry == NULL)
			{
				continue;			
			}
		
			/* if base not changed, do nothing */
			if (currPtnEntry->base == tmpPtnEntry->base)
			{
				tmpPtnEntry->upgradeInfo.dataType = NM_FWUP_UPGRADE_DATA_TYPE_NO_CHANGE;
				continue;
			}
			/* read content from NVRAM to a memory cache */
			readSize = 0;
			if(currPtnEntry->size <= tmpPtnEntry->size)
			{
				readSize = currPtnEntry->size;
			}
			else
			{
				readSize = tmpPtnEntry->size;
			}

			ptnContentCache[index] = (char*)malloc(readSize);
		
			if (ptnContentCache[index] == NULL)
			{
				NM_ERROR("memory malloc failed.ptn=%s,readSize=%x",currPtnEntry->name,readSize);
				goto error;
			}
			
			memset(ptnContentCache[index], 0, readSize);
		
			if (nm_lib_readHeadlessPtnFromNvram((char *)currPtnEntry->base, 
												ptnContentCache[index], readSize) < 0)
			{				
				NM_ERROR("get data from NVRAM failed.");
				goto error;
			}
		
			tmpPtnEntry->upgradeInfo.dataStart = (unsigned int)ptnContentCache[index];
			tmpPtnEntry->upgradeInfo.dataLen = readSize;
			tmpPtnEntry->upgradeInfo.dataType = NM_FWUP_UPGRADE_DATA_FROM_NVRAM;
		}
#endif
    }

    return OK;
error:
    return ERROR;
}
    


/*******************************************************************
 * Name		: nm_updateDataToNvram
 * Abstract	: write to NARAM
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR.
 */
STATUS nm_updateDataToNvram(NM_PTN_STRUCT *ptnStruct)
{   
    int index = 0;
	int numBlookUpdate = 0;
    NM_PTN_ENTRY *currPtnEntry = NULL;
    int  firstFragmentSize = 0;
    int firstFragment = TRUE;
    unsigned long int fragmentBase = 0;
    int  fragmentDataStart = 0;
    int  fwupDataLen = 0;
    g_fwupPercent = 0;

    /* clear write bytes counter first */
    g_nmCountFwupAllWriteBytes = 0;

    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);

        switch (currPtnEntry->upgradeInfo.dataType)
        {
        case NM_FWUP_UPGRADE_DATA_TYPE_BLANK:
            /* if a partition is "blank", means it's a new partition
             * without content, we set content of this partition to all zero */
            if (ptnContentCache[index] != NULL)
            {
                free(ptnContentCache[index]);
                ptnContentCache[index] = NULL;
            }

            ptnContentCache[index] = (char*)malloc(currPtnEntry->size);            

            if (ptnContentCache[index] == NULL)
            {
                NM_ERROR("memory malloc failed.");
                goto error;
            }
            
            memset(ptnContentCache[index], 0, currPtnEntry->size);

            currPtnEntry->upgradeInfo.dataStart = (unsigned int)ptnContentCache[index];
            currPtnEntry->upgradeInfo.dataLen = currPtnEntry->size;
            break;
		case NM_FWUP_UPGRADE_DATA_TYPE_NO_CHANGE:
			NM_DEBUG("PTN %s no need to update.", currPtnEntry->name);
			break;

        case NM_FWUP_UPGRADE_DATA_FROM_FWUP_FILE:
        case NM_FWUP_UPGRADE_DATA_FROM_NVRAM:
            /* Do Nothing */
            break;

        default:
            NM_ERROR("invalid upgradeInfo dataType found.");
            goto error;
            break;  
        }
        
    }


    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);

        if (currPtnEntry->usedFlag != TRUE)
        {
            continue;
        }

        g_nmCountFwupAllWriteBytes += currPtnEntry->upgradeInfo.dataLen;
        
        NM_DEBUG("PTN %02d: dataLen = %08x, g_nmCountFwupAllWriteBytes = %08x", 
                        index+1, currPtnEntry->upgradeInfo.dataLen, g_nmCountFwupAllWriteBytes);
    }

    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);

        switch (currPtnEntry->upgradeInfo.dataType)
        {
		case NM_FWUP_UPGRADE_DATA_TYPE_NO_CHANGE:
			NM_DEBUG("PTN %s no need to update.\r\n", currPtnEntry->name);
			break;
        case NM_FWUP_UPGRADE_DATA_TYPE_BLANK:       
        case NM_FWUP_UPGRADE_DATA_FROM_FWUP_FILE:   
        case NM_FWUP_UPGRADE_DATA_FROM_NVRAM:
            if (currPtnEntry->usedFlag != TRUE)
            {
                NM_DEBUG("PTN %02d: usedFlag = FALSE", index+1);
                continue;
            }

            NM_DEBUG("PTN %02d: name = %s, base = 0x%08x, size = 0x%08x Bytes, upDataType = %d, upDataStart = 0x%08x, upDataLen = 0x%08x",
                index+1, 
                currPtnEntry->name, 
                currPtnEntry->base,
                currPtnEntry->size,
                currPtnEntry->upgradeInfo.dataType,
                currPtnEntry->upgradeInfo.dataStart,
                currPtnEntry->upgradeInfo.dataLen);

			/* 升级过程中将升级文件分成多个分片进行升级, 这样做是为了在升级完
			 * 一个分片后可以统计一次当前已经升级文件长度, 从而使上层模块知道
			 * 当前的升级进度.切分分片时需要注意升级过程中flash的block对齐问题 */
            if (currPtnEntry->upgradeInfo.dataLen > NM_FWUP_FRAGMENT_SIZE)
            {
                fwupDataLen = currPtnEntry->upgradeInfo.dataLen;
                firstFragment = TRUE;
                
                firstFragmentSize = NM_FWUP_FRAGMENT_SIZE - (currPtnEntry->base % NM_FWUP_FRAGMENT_SIZE);
                fragmentBase = 0;
                fragmentDataStart = 0;
                
                while (fwupDataLen > 0)
                {
                    if (firstFragment)
                    {
                        fragmentBase = currPtnEntry->base;
                        fragmentDataStart = currPtnEntry->upgradeInfo.dataStart;
                        
                        NM_DEBUG("PTN f %02d: fragmentBase = %08x, FragmentStart = %08x, FragmentLen = %08x, datalen = %08x", 
                            index+1, fragmentBase, fragmentDataStart, firstFragmentSize, fwupDataLen);

                        if (nm_lib_writeHeadlessPtnToNvram((char *)fragmentBase, 
                                                            (char *)fragmentDataStart,
                                                            firstFragmentSize) < 0)
                        {
                            NM_ERROR("WRITE TO NVRAM FAILED!!!!!!!!.");
                            goto error;
                        }

                        fragmentBase += firstFragmentSize;
                        fragmentDataStart += firstFragmentSize;
                        g_nmCountFwupCurrWriteBytes += firstFragmentSize;
                        fwupDataLen -= firstFragmentSize;
                        NM_DEBUG("PTN f %02d: write bytes = %08x", index+1, g_nmCountFwupCurrWriteBytes);
                        firstFragment = FALSE;
                    }
                    /* last block */
                    else if (fwupDataLen < NM_FWUP_FRAGMENT_SIZE)
                    {
                        NM_DEBUG("PTN l %02d: fragmentBase = 0x%08x, FragmentStart = 0x%08x, FragmentLen = 0x%08x, datalen = 0x%08x", 
                            index+1, fragmentBase, fragmentDataStart, fwupDataLen, fwupDataLen);

                        if (nm_lib_writeHeadlessPtnToNvram((char *)fragmentBase, 
                                                            (char *)fragmentDataStart,
                                                            fwupDataLen) < 0)
                        {
                            NM_ERROR("WRITE TO NVRAM FAILED!!!!!!!!.");
                            goto error;
                        }
						
                        fragmentBase += fwupDataLen;
                        fragmentDataStart += fwupDataLen;
                        g_nmCountFwupCurrWriteBytes += fwupDataLen;
                        fwupDataLen -= fwupDataLen;
                        NM_DEBUG("PTN l %02d: write bytes = %08x", index+1, g_nmCountFwupCurrWriteBytes);
                    }
                    else
                    {
                       NM_DEBUG("PTN n %02d: fragmentBase = %08x, FragmentStart = %08x, FragmentLen = %08x, datalen = %08x", 
                           index+1, fragmentBase, fragmentDataStart, NM_FWUP_FRAGMENT_SIZE, fwupDataLen);
                        
                        if (nm_lib_writeHeadlessPtnToNvram((char *)fragmentBase, 
                                                            (char *)fragmentDataStart,
                                                            NM_FWUP_FRAGMENT_SIZE) < 0)
                        {
                            NM_ERROR("WRITE TO NVRAM FAILED!!!!!!!!.");
                            goto error;
                        }
                   
                        fragmentBase += NM_FWUP_FRAGMENT_SIZE;
                        fragmentDataStart += NM_FWUP_FRAGMENT_SIZE;
                        g_nmCountFwupCurrWriteBytes += NM_FWUP_FRAGMENT_SIZE;
                        fwupDataLen -= NM_FWUP_FRAGMENT_SIZE;
                       NM_DEBUG("PTN n %02d: write bytes = %08x", index+1, g_nmCountFwupCurrWriteBytes);
                    }
					/* add by mengqing, 18Sep09, 由于对单个的FLASH读写操作使用tasklock加锁，升级软件时
					 * 需要在各个文件块之间增加taskdelay人为释放CPU，否则CPU可能被升级模块长期占据导致
					 * web页面无法获得当前的升级进度 */
                    NM_DEBUG("g_nmCountFwupAllWriteBytes =%d   g_nmCountFwupCurrWriteBytes =%d  g_fwupPercent =%d\n",g_nmCountFwupAllWriteBytes,g_nmCountFwupCurrWriteBytes,g_fwupPercent);
					if(g_nmUpgradeResult == TRUE)
                    {
                       g_fwupPercent = 100;
                    }
                    else
                    {
                       g_fwupPercent = g_nmCountFwupCurrWriteBytes*100/g_nmCountFwupAllWriteBytes;
                    }
                    if(numBlookUpdate >= 70)
					{
						numBlookUpdate = 0;
						prom_printf("\r\n");
					}
					numBlookUpdate ++;
                    // printf("#");
					fflush(stdout);
					mdelay (2);
                }
            }
            else
            {           
                /* we should add head to ptn-table partition */
                if (memcmp(currPtnEntry->name, NM_PTN_NAME_PTN_TABLE, NM_PTN_NAME_LEN) == 0)
                {
                    if (nm_lib_writePtnToNvram((char *)currPtnEntry->base, 
                                                    (char *)currPtnEntry->upgradeInfo.dataStart,
                                                    currPtnEntry->upgradeInfo.dataLen) < 0)

                    {
                        NM_ERROR("WRITE TO NVRAM FAILED!!!!!!!!.");
                        goto error;
                    }
                }
                /* head of other partitions can be found in fwup-file or NVRAM */
                else
                {
                	if (nm_lib_writeHeadlessPtnToNvram((char *)currPtnEntry->base, 
                                                    (char *)currPtnEntry->upgradeInfo.dataStart,
                                                    currPtnEntry->upgradeInfo.dataLen) < 0)                                 
                	{
                   	 	NM_ERROR("WRITE TO NVRAM FAILED!!!!!!!!.");
                    	goto error;
                	}
				}
             
                g_nmCountFwupCurrWriteBytes += currPtnEntry->upgradeInfo.dataLen;
                NM_DEBUG("PTN %02d: write bytes = %08x", index+1, g_nmCountFwupCurrWriteBytes);

				if(numBlookUpdate >= 70)
				{
					numBlookUpdate = 0;
					//printf("\r\n");
				}
				numBlookUpdate ++;
				//printf("#");
				fflush(stdout);
            }
            break;

        default:
            NM_ERROR("invalid upgradeInfo dataType found.");
            goto error;
            break;  
        }       
    }
	//printf("\r\nDone.\r\n");
    return OK;
error:
    return ERROR;
}


/*******************************************************************
 * Name		: nm_updateRuntimePtnTable
 * Abstract	: update the runtimePtnTable.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
STATUS nm_updateRuntimePtnTable(NM_PTN_STRUCT *ptnStruct, NM_PTN_STRUCT *runtimePtnStruct)
{   
    int index = 0;
    NM_PTN_ENTRY *currPtnEntry = NULL;
    NM_PTN_ENTRY *currRuntimePtnEntry = NULL;

    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
        currRuntimePtnEntry = (NM_PTN_ENTRY *)&(runtimePtnStruct->entries[index]);

        memcpy(currRuntimePtnEntry->name, currPtnEntry->name, NM_PTN_NAME_LEN);
        currRuntimePtnEntry->base = currPtnEntry->base;
        currRuntimePtnEntry->tail = currPtnEntry->tail;
        currRuntimePtnEntry->size = currPtnEntry->size;
        currRuntimePtnEntry->usedFlag = currPtnEntry->usedFlag;
    }   

    return OK;
}

/*******************************************************************
 * Name		: nm_checkSupportList
 * Abstract	: check the supportlist.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
STATUS nm_checkSupportList(char *support_list, int len)
{
    int ret = 0;
    
    PRODUCT_INFO_STRUCT *pProductInfo = NULL;

    /* skip partition header */
    len -= 8;
    support_list += 8;
 
    /* check list prefix string */
    if (len < 12 || strncmp(support_list, "SupportList:", 12) != 0)
        return ERROR;

    len -= 12;
    support_list += 12;
#ifdef REFINE_FACTORY_UBOOT
	pProductInfo = get_fixed_product_info();
#else
    pProductInfo = sysmgr_getProductInfo();
#endif
    ret = sysmgr_cfg_checkSupportList(pProductInfo, support_list, len);
    if (0 == ret)
    {
        NM_INFO("Firmware supports, check OK.\r\n");
        return OK;
    }
    
    NM_INFO("Firmware not supports, check failed.\r\n");
    return ERROR;
}



/*******************************************************************
 * Name		: nm_checkSoftVer
 * Abstract	: check the software version.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
STATUS nm_checkSoftVer(char *sw_ver, int len)
{
	int ret = 0;
	int cnt = 0;

	char tmpbuff[NM_FWUP_SOFTWARE_VERSION_LEN] = {0};
	int buflen = NM_FWUP_SOFTWARE_VERSION_LEN;
	int curFw_x = 0, curFw_y = 0;
	int newFw_x = 0, newFw_y = 0;

	/* skip partition header */
	len -= 8;
	sw_ver += 8;

	/* check list prefix string */
	if (strncmp(sw_ver, "soft_ver:", 9) != 0)
		return ERROR;

	len -= 9;
	sw_ver += 9;

	cnt = sscanf(sw_ver, "%d.%d.%*s", &newFw_x, &newFw_y);
	if (cnt != 2)
	{
		NM_ERROR("Input get soft version digit failed!");		
		return ERROR;
	}

	/* read partition from nvram */
	ret = nm_api_readPtnFromNvram(NM_PTN_NAME_SOFT_VERSION, tmpbuff, buflen);
	if (ret < 0)
	{
		NM_ERROR("failed to read partition from flash!");
		return ERROR;
	}
	
	if (strncmp(tmpbuff, "soft_ver:", 9) != 0)
		return ERROR;
			
	cnt = sscanf(tmpbuff+9, "%d.%d.%*s", &curFw_x, &curFw_y);
	if (cnt != 2)
	{
		NM_ERROR("Flash get soft version digit failed!");		
		return ERROR;
	}

	NM_INFO(" (curFw_ver, newFw_ver) == (%d.%d, %d.%d) \r\n", curFw_x,curFw_y, newFw_x,newFw_y);

	if ((curFw_x > newFw_x) || 
	    ((curFw_x == newFw_x) && (curFw_y > newFw_y)))
	{
		NM_ERROR("Firmwave not supports, check failed.");
		return ERROR;
	}

	return OK;
}

/*******************************************************************
 * Name		: nm_checkUpgradeMode
 * Abstract	: check the boot mode
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
STATUS nm_checkUpgradeMode(NM_PTN_STRUCT *ptnStruct)
{
    int len = -1;
    NM_PTN_ENTRY *ptnEntry = NULL;
    int upDbootFound = FALSE;
    int flashDbootFound = FALSE;
	NM_PTN_EXTRA_PARA *ptnSysParaData = NULL;
    NM_PTN_EXTRA_PARA realPtnSysParaData;

	memset(&realPtnSysParaData, 0, sizeof(NM_PTN_EXTRA_PARA));
	
	/* not allowed to upgrade between double boot version 
	 * and single boot version, "extra-para" all exist or 
	 * not and double boot flag must equal
	 */

	/* check partition-table partition base address is equal or not*/
#if defined(CFG_DOUBLE_BOOT_FACTORY) || defined(CFG_DOUBLE_BOOT_SECOND)
	if ((ptnEntry = nm_lib_ptnNameToEntry(ptnStruct, NM_PTN_NAME_PTN_TABLE)) == NULL)
	{
		NM_ERROR("Up file partition-table not found!\n");
		return ERROR;
		
	}

	if (ptnEntry->base != NM_PTN_TABLE_BASE)
	{
		NM_ERROR("Up file partition-table base address changed up(0x%08x) flash(0x%08x)!\n",
			ptnEntry->base, NM_PTN_TABLE_BASE);
		return ERROR;		
	}
#endif

	/* do not allow double boot upgrade to single boot or single to double */
	if ((ptnEntry = nm_lib_ptnNameToEntry(ptnStruct, NM_PTN_NAME_EXTRA_PARA)) != NULL)
	{
		/* double boot up file must have extra-para partition for boot mode check */
		if (ptnEntry->upgradeInfo.dataType == NM_FWUP_UPGRADE_DATA_FROM_FWUP_FILE)
		{
			ptnSysParaData = (NM_PTN_EXTRA_PARA *)((char*)ptnEntry->upgradeInfo.dataStart + 8);

			if (ptnSysParaData->dbootFlag == NM_FWUP_IS_DOUBLE_BOOT)
			{
				upDbootFound = TRUE;
			}
		}
	}

    len = nm_api_readPtnFromNvram(NM_PTN_NAME_EXTRA_PARA, 
					(char *)&realPtnSysParaData, sizeof(NM_PTN_EXTRA_PARA));
	if (len > 0 && realPtnSysParaData.dbootFlag == NM_FWUP_IS_DOUBLE_BOOT)   
	{
		flashDbootFound = TRUE;
	}

	if (upDbootFound != flashDbootFound)
	{
		NM_ERROR("Double boot flag up %d flash %d not ok!",
			upDbootFound, flashDbootFound);
		return ERROR;
	}
	
	return OK;
}
/**************************************************************
* Name 	   : nm_getSoftVer()
* Abstract : Get soft version.
* Input    : None.
* Output   : Software version, host order.
* Return   : STATUS, OK or ERROR, check this ret val first.
*/
STATUS nm_getSoftVer(unsigned int* softVer)
{
    int ret = -1;
	int a=0, b=0, c=0;
	int cnt = 0;
	unsigned int tempInt = 0;
    char tempStr[NM_FWUP_SOFTWARE_VERSION_LEN];
	
	ret = nm_api_readPtnFromNvram(NM_PTN_NAME_SOFT_VERSION, tempStr, NM_FWUP_SOFTWARE_VERSION_LEN);
	if (ret < 0)
	{
		NM_ERROR("failed to read soft version from flash!");
		return ERROR;
	}

	NM_DEBUG("soft version %s ret %d", tempStr, ret);

	if (strncmp(tempStr, "soft_ver:", 9) != 0)
	{
		NM_ERROR("failed to read soft version from flash!");
		return ERROR;
	}

	cnt = sscanf(tempStr, "soft_ver:%d.%d.%d Build", &a, &b, &c);
	if (cnt != 3)
	{
		NM_ERROR("Flash get soft version digit failed!");		
		return ERROR;
	}
	
	tempInt = (a<<16) + (b<<8) + c;
	*softVer = tempInt;

    return OK;
}

/*******************************************************************
 * Name		: nm_checkUpdateContent
 * Abstract	: check the updata content.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
STATUS nm_checkUpdateContent(NM_PTN_STRUCT *ptnStruct, char *pAppBuf, int nFileBytes, int *errorCode)
{   
    int index = 0;
    NM_PTN_ENTRY *currPtnEntry = NULL;
    int ptnFound = FALSE;
	int suppportListIndex = 0;
	int softVersionIndex = 0;

    /* check update content */
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);

        if (currPtnEntry->upgradeInfo.dataType == NM_FWUP_UPGRADE_DATA_FROM_FWUP_FILE)
        {       
            if ((currPtnEntry->upgradeInfo.dataStart + currPtnEntry->upgradeInfo.dataLen)
                > (unsigned int)(pAppBuf + nFileBytes))
            {
                NM_ERROR("ptn \"%s\": update data end out of fwup-file.", currPtnEntry->name);
                *errorCode = NM_FWUP_ERROR_BAD_FILE;
                goto error;
            }
        }
    }

    /* check important partitions */
    ptnFound = FALSE;
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
		currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);

       	if (strncmp(currPtnEntry->name, NM_PTN_NAME_FS_UBOOT, NM_PTN_NAME_LEN) == 0)
        {
            ptnFound = TRUE;
            break;
        }
    }
    if (ptnFound == FALSE)
    {               
        NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_FS_UBOOT);
        *errorCode = NM_FWUP_ERROR_BAD_FILE;
        goto error;
    }


	ptnFound = FALSE;
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
        
        if (strncmp(currPtnEntry->name, NM_PTN_NAME_PTN_TABLE, NM_PTN_NAME_LEN) == 0)
        {
            ptnFound = TRUE;
            break;
        }
    }
    if (ptnFound == FALSE)
    {               
        NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_PTN_TABLE);
        *errorCode = NM_FWUP_ERROR_BAD_FILE;
        goto error;
    }

    ptnFound = FALSE;
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
        
        if (strncmp(currPtnEntry->name, NM_PTN_NAME_DEFAULT_MAC, NM_PTN_NAME_LEN) == 0)
        {
            ptnFound = TRUE;
            break;
        }
    }
    if (ptnFound == FALSE)
    {               
        NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_DEFAULT_MAC);
        *errorCode = NM_FWUP_ERROR_BAD_FILE;
        goto error;
    }


	ptnFound = FALSE;
	for (index=0; index<NM_PTN_NUM_MAX; index++)
	{
		currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
		
		if (strncmp(currPtnEntry->name, NM_PTN_NAME_PRODUCT_INFO, NM_PTN_NAME_LEN) == 0)
		{
			ptnFound = TRUE;
			break;
		}
	}
	if (ptnFound == FALSE)
	{				
		NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_PRODUCT_INFO);
		*errorCode = NM_FWUP_ERROR_BAD_FILE;
		goto error;
	}

	ptnFound = FALSE;
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
        
        if (strncmp(currPtnEntry->name, NM_PTN_NAME_SUPPORT_LIST, NM_PTN_NAME_LEN) == 0)
        {
			suppportListIndex = index;
            ptnFound = TRUE;
            break;
        }
    }
    if (ptnFound == FALSE)
    {               
        NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_SUPPORT_LIST);
        *errorCode = NM_FWUP_ERROR_BAD_FILE;
        goto error;
    }

	
	ptnFound = FALSE;
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
        
        if (strncmp(currPtnEntry->name, NM_PTN_NAME_SOFT_VERSION, NM_PTN_NAME_LEN) == 0)
        {
			softVersionIndex = index;
            ptnFound = TRUE;
            break;
        }
    }
    if (ptnFound == FALSE)
    {               
        NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_SOFT_VERSION);
        *errorCode = NM_FWUP_ERROR_BAD_FILE;
        goto error;
    }
	

    ptnFound = FALSE;
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
        
        if (strncmp(currPtnEntry->name, NM_PTN_NAME_OS_IMAGE, NM_PTN_NAME_LEN) == 0)
        {
            ptnFound = TRUE;
            break;
        }
    }
    if (ptnFound == FALSE)
    {               
        NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_OS_IMAGE);
        *errorCode = NM_FWUP_ERROR_BAD_FILE;
        goto error;
    }


	ptnFound = FALSE;
	for (index=0; index<NM_PTN_NUM_MAX; index++)
	{
		currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[index]);
		
		if (strncmp(currPtnEntry->name, NM_PTN_NAME_FILE_SYSTEM, NM_PTN_NAME_LEN) == 0)
		{
			ptnFound = TRUE;
			break;
		}
	}
	if (ptnFound == FALSE)
	{				
		NM_ERROR("ptn \"%s\" not found whether in fwup-file or NVRAM.", NM_PTN_NAME_FILE_SYSTEM);
		*errorCode = NM_FWUP_ERROR_BAD_FILE;
		goto error;
	}

	if (OK != nm_checkUpgradeMode(ptnStruct))
	{
		NM_ERROR("upgrade boot mode check fail.");
		*errorCode = NM_FWUP_ERROR_UNSUPPORT_BOOT_MOD;
		goto error;
		
	}

	//check the hardware version support list
	currPtnEntry = (NM_PTN_ENTRY *)&(ptnStruct->entries[suppportListIndex]);
	if (NM_FWUP_UPGRADE_DATA_FROM_FWUP_FILE != currPtnEntry->upgradeInfo.dataType ||
		OK != nm_checkSupportList((char*)currPtnEntry->upgradeInfo.dataStart, currPtnEntry->upgradeInfo.dataLen))
	{
		NM_ERROR("the firmware is not for this model");
		*errorCode = NM_FWUP_ERROR_INCORRECT_MODEL;
		goto error;
	}

#ifndef	REFINE_FACTORY_UBOOT
	currPtnEntry = (NM_PTN_ENTRY *) & (ptnStruct->entries[softVersionIndex]);
	if (gset_fw_type(FW_TYPE_INVALID) == FW_TYPE_CLOUD &&
	        OK != nm_checkSoftVer((char *)currPtnEntry->upgradeInfo.dataStart, currPtnEntry->upgradeInfo.dataLen))
	{
		NM_ERROR("the firmware software version dismatched");
		*errorCode = NM_FWUP_ERROR_UNSUPPORT_VER;
		goto error;
	}
#endif	
    return OK;
error:
    return ERROR;
}



/*******************************************************************
 * Name		: nm_cleanupPtnContentCache
 * Abstract	: free the memmory of ptnContentCache.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR.
 */

STATUS nm_cleanupPtnContentCache()
{   
    int index = 0;


    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {           
        if (ptnContentCache[index] != NULL)
        {
            free(ptnContentCache[index]);
            ptnContentCache[index] = NULL;
        }   
    }
    
    return OK;
}


/*******************************************************************
 * Name		: nm_buildUpgradeStruct
 * Abstract	: Generate an upgrade file from NVRAM and firmware file.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR.
 */
int nm_buildUpgradeStruct(char *pAppBuf, int nFileBytes)
{
    char fwupPtnIndex[NM_FWUP_PTN_INDEX_SIZE+1] = {0};
    char *fwupFileBase = NULL;
    int index;
    int ret = 0;

	NM_DEBUG("%s %d %x\n",__FUNCTION__,__LINE__,g_nmFwupPtnStruct);

    memset(g_nmFwupPtnStruct, 0, sizeof(NM_PTN_STRUCT));
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        g_nmFwupPtnStruct->entries[index].usedFlag = FALSE;
    }
    g_nmCountFwupAllWriteBytes = 0;
    g_nmCountFwupCurrWriteBytes = 0;

    nm_cleanupPtnContentCache();
	
    /* backup "fwup-partition-index" */
    fwupFileBase = pAppBuf;
    strncpy(fwupPtnIndex, pAppBuf, NM_FWUP_PTN_INDEX_SIZE+1);
    pAppBuf += NM_FWUP_PTN_INDEX_SIZE;
    pAppBuf += sizeof(int);

    NM_DEBUG("nFileBytes = %d",  nFileBytes);
	
    if (nm_lib_parsePtnIndexFile(g_nmFwupPtnStruct, pAppBuf) != OK)
    {
        NM_ERROR("parse new ptn-index failed.");
        ret = NM_FWUP_ERROR_BAD_FILE;
        goto cleanup;
    }

    if (nm_getDataFromFwupFile(g_nmFwupPtnStruct, (char *)&fwupPtnIndex, fwupFileBase) != OK)
    {
        NM_ERROR("getDataFromFwupFile failed.");
        ret = NM_FWUP_ERROR_BAD_FILE;
        goto cleanup;
    }
	
    if (nm_getDataFromNvram(g_nmFwupPtnStruct, g_nmPtnStruct) != OK)
    {
        NM_ERROR("getDataFromNvram failed.");
        ret = NM_FWUP_ERROR_BAD_FILE;
        goto cleanup;
    }

    if (nm_checkUpdateContent(g_nmFwupPtnStruct, fwupFileBase, nFileBytes, &ret) != OK)
    {
        NM_ERROR("checkUpdateContent failed.");
        goto cleanup;
    }

    return 0;
    
cleanup:
    memset(g_nmFwupPtnStruct, 0, sizeof(NM_PTN_STRUCT));
    g_nmCountFwupAllWriteBytes = 0;
    g_nmCountFwupCurrWriteBytes = 0;
    nm_cleanupPtnContentCache();
    g_nmUpgradeResult = FALSE;
    return ret;
}

/*  *********************************************************************
    *  nm_endProcess()
    *  
    *  end process afger upgrade
    *  
    *  Input parameters: 
    *  	   none
    *  	   
    *  Return value:
    *  	   0 if set ok
    *  	   -1 is error
    ********************************************************************* */
STATUS nm_endProcess(void)
{
	int cnt = 0;

	//count any partition need to erase static
	cnt = 0;
	while (strcmp(need_erase_ptn[cnt], "NULL") != 0)	
	{
		if (OK != nm_api_erasePtn(need_erase_ptn[cnt]))
		{
			NM_ERROR("Erase %s failed.", need_erase_ptn[cnt]);
			return ERROR;
		}
	
		cnt++;
	}

	return OK;
}
#ifdef REFINE_FACTORY_UBOOT
STATUS nm_new_copy_ptn_from_up_to_runtime()
{
   g_nmPtnStruct = g_nmFwupPtnStruct;
}
#endif

/*  *********************************************************************
    *  nm_tpFirmwareCheck()
    *  
    *  firmware check
    *  
    *  Input parameters: 
    *  	   ptr     : buffer pointer
    *	   bufsize : buffer size
    *  	   
    *  Return value:
    *  	   0 if set ok
    *  	   other is error
    ********************************************************************* */
STATUS nm_tpFirmwareCheck(unsigned char *ptr,int bufsize)
{
    int ret = 0;
    int fileLen = 0;
    unsigned char *pBuf = NULL;

#ifndef REFINE_FACTORY_UBOOT
    ret = nm_init();
    if (OK != ret)
    {
        NM_ERROR("Init failed.");
        return NM_FWUP_ERROR_NORMAL;
    }
#else
	nm_initFwupPtnStruct();
#endif	
    
    memcpy(&fileLen, ptr, sizeof(int));
	fileLen = ntohl(fileLen);

	if(fileLen < IMAGE_SIZE_MIN || fileLen > IMAGE_SIZE_MAX || bufsize < fileLen)
	{
		NM_ERROR("The file's length is bad(buf:%d fileLen%d)", bufsize, fileLen);
		return NM_FWUP_ERROR_INVALID_FILE;
	}

	NM_INFO("Firmware Recovery file length : %d\r\n", fileLen);

	ret = nm_tpFirmwareVerify(ptr, fileLen);
	if (0 != ret)
	{
		return ret;
	}
	
	NM_INFO("Firmware file Verify ok!\r\n");

#ifdef REFINE_FACTORY_UBOOT
	nm_new_copy_ptn_from_up_to_runtime();
#endif

	pBuf = ptr + IMAGE_SIZE_BASE;
    NM_INFO("buf size:%d, image_size_base:%d\r\n", fileLen, IMAGE_SIZE_BASE);
	ret = nm_buildUpgradeStruct((char *)pBuf, fileLen - IMAGE_SIZE_BASE);
	if (0 != ret)
	{
		return ret;
	}

	NM_INFO("Firmware Recovery check ok!\r\n");
	
    return OK;
}

/*  *********************************************************************
    *  nm_tpFirmwareRecovery()
    *  
    *  firmware recovery process
    *  
    *  Input parameters: 
    *  	   ptr     : buffer pointer
    *	   bufsize : buffer size
    *  	   
    *  Return value:
    *  	   0 if set ok
    *  	   other is error
    ********************************************************************* */
STATUS nm_tpFirmwareRecovery(unsigned char *ptr,int bufsize)
{
    int ret = 0;

	NM_INFO("nm_tpFirmwareRecovery---\n");
	ret = nm_tpFirmwareCheck(ptr, bufsize);
    if (OK != ret)
    {   
        NM_INFO("FirmwareCheck filed ret:%d\n", ret);
		return ret;
	}
	
	ret = nm_upgradeFwupFile((char *)ptr + IMAGE_SIZE_BASE, bufsize - IMAGE_SIZE_BASE);
    if (OK != ret)
    {
        NM_ERROR("upgrade firmware failed!");
		return NM_FWUP_ERROR_NORMAL;
    }

	NM_INFO("Firmware Recovery Success!\r\n");
    return OK;
}

/*******************************************************************
 * Name		: nm_upgradeFwupFile
 * Abstract	: upgrade the FwupFile to NVRAM
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR.
 */
STATUS nm_upgradeFwupFile(char *pAppBuf, int nFileBytes)
{
	unsigned int preSoftVer  = 0;
	unsigned int postSoftVer = 0;
	
    g_nmUpgradeResult = FALSE;

#if defined(CFG_DOUBLE_BOOT_FACTORY) || defined(CFG_DOUBLE_BOOT_SECOND) || defined(CONFIG_PRODUCT_C6V2)
	if (OK != nm_api_setIntegerFlag(NM_FWUP_NOT_INTEGER))
	{
		NM_ERROR("set not integer failed!");
		goto cleanup;
	}
#endif

#ifndef REFINE_FACTORY_UBOOT
	//get soft-version before upgrade
	if (OK != nm_getSoftVer(&preSoftVer))
	{
		NM_ERROR("pre get no soft version failed!");
		goto cleanup;
	}	
#endif	

    if (nm_updateDataToNvram(g_nmFwupPtnStruct) != OK)
    {
        NM_ERROR("updateDataToNvram failed.");
        goto cleanup;
    }

    /* update run-time partition-table, active new partition-table without restart */
    if (nm_updateRuntimePtnTable(g_nmFwupPtnStruct, g_nmPtnStruct) != OK)
    {
        NM_ERROR("updateDataToNvram failed.");
        goto cleanup;
    }

	//get soft-version after upgrade
	if (OK != nm_getSoftVer(&postSoftVer))
	{
		NM_ERROR("post get no soft version failed!");
		goto cleanup;
	}

	//check the soft version, if the first two digit not equal
	//just erase the user-config
#ifndef REFINE_FACTORY_UBOOT // always reset user-config in recovery mode...
	if ((postSoftVer & 0xffff00) != (preSoftVer & 0xffff00))
#endif			
	{
		NM_DEBUG("erase user config to default failed!");
		
		if (OK != nm_api_erasePtn(NM_PTN_NAME_USER_CFG))
		{
			NM_ERROR("erase user config to default failed!");
			goto cleanup;
		}
	}

	//end process
	if (OK != nm_endProcess())
	{
		goto cleanup;
	}

#if defined(CFG_DOUBLE_BOOT_FACTORY) || defined(CFG_DOUBLE_BOOT_SECOND)|| defined(CONFIG_PRODUCT_C6V2)
	if (OK != nm_api_setIntegerFlag(NM_FWUP_IS_INTEGER))
	{
		NM_ERROR("set integer failed!");
		goto cleanup;
	}
#endif

    memset(g_nmFwupPtnStruct, 0, sizeof(NM_PTN_STRUCT));
    g_nmCountFwupAllWriteBytes = 0;
    g_nmCountFwupCurrWriteBytes = 0;
    nm_cleanupPtnContentCache();
    g_nmUpgradeResult = TRUE;
    return OK;
    
cleanup:
    memset(g_nmFwupPtnStruct, 0, sizeof(NM_PTN_STRUCT));
    g_nmCountFwupAllWriteBytes = 0;
    g_nmCountFwupCurrWriteBytes = 0;
    nm_cleanupPtnContentCache();
    g_nmUpgradeResult = FALSE;
    return ERROR;
}

/**************************************************************************************************/
/*                                      GLOBAL_FUNCTIONS                                          */
/**************************************************************************************************/

