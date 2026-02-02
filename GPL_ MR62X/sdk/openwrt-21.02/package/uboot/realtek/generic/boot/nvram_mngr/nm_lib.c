
/*! Copyright(c) 1996-2009 Shenzhen TP-LINK Technologies Co. Ltd.
 * \file    nm_lib.c
 * \brief   library functions for NVRAM manager.
 * \author  Meng Qing
 * \version 1.0
 * \date    24/04/2009
 */

/**************************************************************************************************/
/*                                      CONFIGURATIONS                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      INCLUDE_FILES                                             */
/**************************************************************************************************/
//#include <common.h>
#include "nm_api.h"
#include "nm_lib.h"
#include "nm_fwup.h"
#include <linux/types.h>
#include "string.h"
#include "nm_utils.h"
#include "linux/delay.h"
/**************************************************************************************************/
/*                                      DEFINES                                                   */
/**************************************************************************************************/
#define FLASH_SECTOR_SIZE      (64*1024)
#define copy_from_user(kernel, user, len) memcpy(kernel, user, len)
#define copy_to_user(user, kernel, len)   memcpy(user, kernel, len)
extern unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
#define strtoul(a,b,c)                    simple_strtoul(a, b, c)
/**************************************************************************************************/
/*                                      TYPES                                                     */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      EXTERN_PROTOTYPES                                         */
/**************************************************************************************************/
//extern unsigned long bcm_strtoul(const char *cp, char **endp, unsigned int base);
//extern int nvrammngr_flashOpPortErase(unsigned int off, unsigned int len);
//extern int nvrammngr_flashOpPortWrite(unsigned int off, unsigned int len, unsigned char *in);


/**************************************************************************************************/
/*                                      LOCAL_PROTOTYPES                                          */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      VARIABLES                                                 */
/**************************************************************************************************/
static int  l_nminited = FALSE;
NM_PTN_STRUCT *g_nmPtnStruct;
NM_PTN_STRUCT g_nmPtnStructEntity;

#if USE_LOCK
SEM_ID g_nmReadWriteLock;
#endif

NM_STR_MAP nm_ptnIndexFileParaStrMap[] =
{
    {NM_PTN_INDEX_PARA_ID_NAME,         "partition"},
    {NM_PTN_INDEX_PARA_ID_BASE,         "base"},
    {NM_PTN_INDEX_PARA_ID_TAIL,         "tail"},
    {NM_PTN_INDEX_PARA_ID_SIZE,         "size"},

    {-1,                                NULL}
};

#define TP_FLASH_READ              0x01
#define TP_FLASH_WRITE             0x02
#define TP_FLASH_ERASE             0x03			/* 64 * 1024 */


typedef struct 
{
	u_int32_t addr;		/* flash r/w addr	*/
	u_int32_t len;		/* r/w length		*/
	u_int8_t* buf;		/* user-space buffer*/
	u_int32_t buflen;	/* buffer length	*/
	u_int32_t hasHead;	/* hasHead flag 		*/
}ARG;
/*
 * static int spi_nor_read(struct mtd_info *mtd, loff_t from, size_t len,
 *	size_t *retlen, u_char *buf)
 */
int spiflash_ioctl_read(u_char *buf, u32 from, u32 len)
{
	int ret = 0;
	ret = flashread((unsigned long)buf, from, len);
	return ret;
}

int spiflash_ioctl_write(u_char *buf, u32 to, u32 len)
{
	int ret = 0;
	ret = spi_flw_image(0, to, buf, len);
	return ret;
}

int spiflash_ioctl_erase(u32 to, u32 len)
{
	return 0;
}



int tp_flash_read(u_int8_t *rwBuf, u_int32_t addr, u_int8_t *usrBuf, u_int32_t usrBufLen)
{
    u_int32_t read_len = usrBufLen;
	
	if(NULL == rwBuf || NULL == usrBuf || read_len <= 0)
	{
		dprintf("Invalid args!\n");
		return -1;
	}
	
	if(spiflash_ioctl_read(usrBuf, addr, read_len) < 0)
	{
		dprintf("spiflash_ioctl_read out of scope!\n");
        return -1;
	}
	
	
    return 0;
}

int tp_flash_write(u_int8_t *tempData, 
                      u_int32_t hasHead, u_int32_t offset, u_int8_t *data, u_int32_t len)
{
    u_int32_t address = 0;
    u_int32_t headLen = 0;
    u_int32_t endAddr = 0, startAddr = 0;
    u_int8_t *orignData = NULL;
    u_int32_t headData[2] = {len, 0};
    u_int32_t frontLen = 0, tailLen = 0;
    u_int32_t retLen = 0;

    headData[0] = htonl(len);   

    if (hasHead)
    {
        headLen = 2 * sizeof(u_int32_t);
        len += headLen;
    }

    frontLen = offset % FLASH_SECTOR_SIZE;
    tailLen  = (offset + len) % FLASH_SECTOR_SIZE;
    /* first block address */
    address = offset - frontLen;
    /* last uncomplete block address. if none, next block address instead. */
    endAddr = offset + len - tailLen;

    orignData = tempData + FLASH_SECTOR_SIZE;

    if (frontLen > 0 || headLen > 0)/*first block*/
    {   
        dprintf("H#__line__%d", __LINE__);
        spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
        memcpy(tempData, orignData, frontLen);
        
        if (FLASH_SECTOR_SIZE < frontLen + headLen) /* header is in different block */
        {
            dprintf("H#__line__%d", __LINE__);
            headLen = FLASH_SECTOR_SIZE - frontLen;
            /* partition header, first part. */
            memcpy(tempData + frontLen, headData, headLen);

            /***************************************************/
            if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE)) 
            {   
                dprintf("H#__line__%d", __LINE__);
                spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
				udelay(20);
                spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
				dprintf("H#");
            }
            address += FLASH_SECTOR_SIZE;
            /***************************************************/
            spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
            /* partition header, second part. */
            memcpy(tempData, (u_int8_t*)(headData) + headLen, 8 - headLen);

            if (len - headLen < FLASH_SECTOR_SIZE) /* writen length less than one block */
            {
                headLen = 8 - headLen;
                copy_from_user(tempData + headLen, data, tailLen - headLen); /* data to be writen */
                memcpy(tempData + tailLen, orignData + tailLen, FLASH_SECTOR_SIZE - tailLen);
                data += tailLen - headLen;
            }
            else
            {
                headLen = 8 - headLen;
                copy_from_user(tempData + headLen, data, FLASH_SECTOR_SIZE - headLen);
                data += FLASH_SECTOR_SIZE - headLen;
            }
        }
        else /* normal */
        {
            
            memcpy(tempData + frontLen, headData, headLen); /* header (if exist) */
            
            if (len + frontLen < FLASH_SECTOR_SIZE) /* write less then a block */
            {
                copy_from_user(tempData + frontLen + headLen, data, len - headLen);
                data += len - headLen;
                /* orginal data */
                memcpy(tempData + frontLen + len,
                         orignData + frontLen + len,
                         FLASH_SECTOR_SIZE - (frontLen + len));
            }
            else
            {
                copy_from_user(tempData + frontLen + headLen, data, FLASH_SECTOR_SIZE - frontLen - headLen);
                /* data to be writen */
                data += FLASH_SECTOR_SIZE - frontLen - headLen;
            }
        }
        dprintf("H#__line__%d", __LINE__);
        /***************************************************/
        if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE))/* context changed */
        {
            dprintf("H#__line__%d", __LINE__);  
			spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
			udelay(20);
            spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
			dprintf("H#");
        }
        address += FLASH_SECTOR_SIZE;
        /***************************************************/
    }

    if (address < endAddr)/* complete blocks in middle */
    {
        dprintf("H#__line__%d", __LINE__);
        startAddr = address;
        while (address < endAddr)
        {
            spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
            copy_from_user(tempData, data, FLASH_SECTOR_SIZE);
            /***************************************************/
            if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE)) /* context changed */
            {
                dprintf("H#__line__%d", __LINE__);
                spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
				udelay(20);
            	spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
				dprintf("#");
            }
            address += FLASH_SECTOR_SIZE;
            /***************************************************/
            data += FLASH_SECTOR_SIZE;
			udelay(20);
        }
    }

    if (address < offset + len) /* last uncomplete block */
    {
        /*dprintf("[asuka] block at last start %p\n", address);*/
        spiflash_ioctl_read(orignData, address, FLASH_SECTOR_SIZE);
        copy_from_user(tempData, data, tailLen); /* firstly, data to be writen */
        memcpy(tempData + tailLen, orignData + tailLen, FLASH_SECTOR_SIZE - tailLen);
        /* secondly, recover orginal data */
        /***************************************************/
        dprintf("H#__line__%d", __LINE__);
        if (memcmp(orignData, tempData, FLASH_SECTOR_SIZE)) /* context changed */
        {   
            dprintf("H#__line__%d", __LINE__);
			spiflash_ioctl_erase(address, FLASH_SECTOR_SIZE);
			udelay(20);
            spiflash_ioctl_write(tempData, address, FLASH_SECTOR_SIZE);
			dprintf("T#");
        }
        address += FLASH_SECTOR_SIZE;
        /***************************************************/
    }
	
    return 0;
}
static int applyed = 0;
unsigned char *rwBuf = NULL;
static long tp_flash_ioctl(unsigned int cmd, ARG* arg)
{
     /* temp buffer for r/w */
     if (applyed==0)
     {
        rwBuf = (unsigned char *)malloc(FLASH_SECTOR_SIZE * 2);
        applyed = 1;
     }
    
    ARG *pArg = (ARG*)arg;
    
    if (rwBuf == NULL)
    {
        dprintf("rw_buf error!\n");
        goto wrong;
    }
   
    switch(cmd)
    {
        case TP_FLASH_READ:
        {
            tp_flash_read(rwBuf, pArg->addr, pArg->buf, pArg->buflen);
            goto good;
            break; 
        }

        case TP_FLASH_WRITE:
        {
            dprintf("T#");
            tp_flash_write(rwBuf, pArg->hasHead, pArg->addr, pArg->buf, pArg->buflen);
            goto good;
            break;
        }
        
        case  TP_FLASH_ERASE:
        {
            goto good;
            break;
        }
		default:
		{
            dprintf("rw_buf error!\n");
			goto wrong;
			break;
		}
    }
    
good:
    
    return 0;
wrong:
    if (rwBuf)
    {
        free(rwBuf);
        applyed = 0;
    }
    return -1;
}

// static unsigned char nvrammngr_rwBuf[FLASH_SECTOR_SIZE];
// static unsigned char orignData[FLASH_SECTOR_SIZE];
//static unsigned char test_buf[FLASH_SECTOR_SIZE];

/**************************************************************************************************/
/*                                      LOCAL_FUNCTIONS                                           */
/**************************************************************************************************/

int nvrammngr_flashOpPortRead(unsigned char *in, unsigned int off, unsigned int len)
{
    ARG arg;
    arg.addr = off;
    arg.len = len;
    arg.buf = in;
    arg.buflen = len;
    arg.hasHead = FALSE;
    return tp_flash_ioctl(TP_FLASH_READ, &arg);
}

int nvrammngr_flashOpPortErase(unsigned int off, unsigned int len)
{
    ARG arg;
    arg.addr = off;
    arg.len = len;
    arg.hasHead = FALSE;
    return tp_flash_ioctl(TP_FLASH_ERASE, &arg);
    
}

int nvrammngr_flashOpPortWrite(int hashead, unsigned int off, unsigned int len, unsigned char *in)
{
    ARG arg;
    arg.addr = off;
    arg.len = len;
    arg.buf = in;
    arg.buflen = len;
    arg.hasHead = hashead;
    return tp_flash_ioctl(TP_FLASH_WRITE, &arg);
}


static int dump_buf(uint32_t line,unsigned char * buf)
{
    // NM_DEBUG("[line:%d]buf:%02x|%02x|%02x|%02x",line,*buf,*(buf+1),*(buf+2),*(buf+3));
    return 0;
}

int nm_lib_writePtntoNvram_unify(uint32_t hasHead, uint32_t offset, uint8_t *data, uint32_t len)
{
	nvrammngr_flashOpPortWrite(hasHead, offset, len, data);
	return 0;
}



/**************************************************************************************************/
/*                                      PUBLIC_FUNCTIONS                                          */
/**************************************************************************************************/

/*******************************************************************
 * Name		: nm_lib_parseU32
 * Abstract	: Converts the string in arg to numeric value.
 * Input	: 
 * Output	: 
 * Return	: success:    0.
 *            fail:       -1
 */
int nm_lib_parseU32(NM_UINT32 *val, const char *arg)
{
    unsigned long res;
    char *ptr = NULL;

    if (!arg || !*arg)
    {
        return -1;
    }

    res = strtoul(arg, &ptr, 0);
    if (!ptr || ptr == arg || *ptr || res > 0xFFFFFFFFUL)
    {
        return -1;
    }
    *val = res;

    
    return 0;
}



/*******************************************************************
 * Name		: nm_lib_makeArgs
 * Abstract	: parse the string of partition-table.
 * Input	: 
 * Output	: 
 * Return	: 
 */
int nm_lib_makeArgs(char *string, char *argv[], int maxArgs)
{
    static const char ws[] = " \t\r\n";
    char *cp;
    int argc = 0;
    char *p_last = NULL;
    
    if (string == NULL)
    {
        return -1;
    }

    for (cp = strtok_r(string, ws, &p_last); cp; cp = strtok_r(NULL, ws, &p_last)) 
    {
        if (argc >= (maxArgs - 1)) 
        {
            NM_ERROR("Too many arguments.");
            return -1;
        }
        argv[argc++] = cp;
    }
    argv[argc] = NULL;

    return argc;
}



/*******************************************************************
 * Name		: nm_lib_strToKey
 * Abstract	: Converts the string in map to numeric value.
 * Input	: map:    The int to string map to use.
 *			  str:    The string representation.
 * Output	: 
 * Return	: success:    The key representation.
 *            fail:       -1
 */
int nm_lib_strToKey(NM_STR_MAP *map, char *str)
{
    int index;

    if (str)
    {
        for (index=0; map[index].str != NULL; index++)
        {
            if (strcmp(str, map[index].str) == 0)
            {
                return map[index].key;
            }
        }
    }
    
    return -1;
}


/*******************************************************************
 * Name		: nm_lib_ptnNameToEntry
 * Abstract	: get partition-entry match the input name.
 * Input	: 
 * Output	: 
 * Return	: point to the partition-entry if match successful.
 *            NULL if match failed.
 */
NM_PTN_ENTRY *nm_lib_ptnNameToEntry(NM_PTN_STRUCT *ptnStruct, char *name)
{
    int index;

    if ((ptnStruct == NULL) || (name == NULL))
    {
        NM_ERROR("invalid input param.");
        return NULL;
    }

    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {       
        if (strcmp(ptnStruct->entries[index].name, name) == 0)
            return &(ptnStruct->entries[index]);
    }
    
    return NULL;
}


/*******************************************************************
 * Name		: nm_lib_fetchUnusedPtnEntry
 * Abstract	: get an unused partition-entry from partition-struct.
 * Input	: 
 * Output	: 
 * Return	: point to the partition-entry if match successful.
 *            NULL if match failed.
 */
NM_PTN_ENTRY *nm_lib_fetchUnusedPtnEntry(NM_PTN_STRUCT *ptnStruct)
{
    int index;

    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {       
        if (ptnStruct->entries[index].usedFlag != TRUE)
        {
            ptnStruct->entries[index].usedFlag = TRUE;
            return &(ptnStruct->entries[index]);
        }
    }
    
    return NULL;
}


/*******************************************************************
 * Name		: nm_lib_writeHeadlessPtnToNvram
 * Abstract	: write the value of a partition in NVRAM.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_lib_writeHeadlessPtnToNvram(char *base, char *buf, int len)
{
    NM_DEBUG("ptnEntry->base = %08x, buf = %08x, len = %d", base + NM_NVRAM_BASE, buf, len);

    if (nm_lib_writePtntoNvram_unify(0, (uint32_t)base + NM_NVRAM_BASE, (uint8_t*)buf, len) < 0)
    {
        return -1;
    }
    
    return len;
}


/*******************************************************************
 * Name		: nm_lib_writePtnToNvram
 * Abstract	: write the value of a partition in NVRAM.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_lib_writePtnToNvram(char *base, char *buf, int len)
{
    NM_DEBUG("ptnEntry->base = %08x, buf = %08x, len = %d", base + NM_NVRAM_BASE, buf, len);
	
    if (nm_lib_writePtntoNvram_unify(1, (uint32_t)base + NM_NVRAM_BASE, (uint8_t*)buf, len) < 0)
    {
		NM_DEBUG("Write partition error!");
        return -1;
    }

    return len;
}


/*******************************************************************
 * Name		: nm_lib_readHeadlessPtnFromNvram
 * Abstract	: read the value of a partition in NVRAM.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_lib_readHeadlessPtnFromNvram(char *base, char *buf, int len)
{
    
    if (nvrammngr_flashOpPortRead((uint8_t*)buf, (uint32_t)base + NM_NVRAM_BASE, len) < 0)
    {
        return -1;
    }
    
    return len;
}

int nvram_flash_read_tp_partition(char *address, char *data, int len)
{
	uint32_t partition_used_len;
    uint32_t read_len;
    nvrammngr_flashOpPortRead((unsigned char*)&partition_used_len, (uint32_t)address, sizeof(uint32_t));
	/*printf("partition_used_len = %d\r\n", partition_used_len);*/
	partition_used_len = ntohl(partition_used_len);
	read_len = (len > partition_used_len) ? partition_used_len : len;
	/* jump over partition length and checksum */
	if (nvrammngr_flashOpPortRead(data, (uint32_t)(address + sizeof(int) + sizeof(int)), read_len) < 0)
	{
		return -1;
	}

	return read_len;
}


/*******************************************************************
 * Name		: nm_lib_readPtnFromNvram
 * Abstract	: read the value of a partition in NVRAM.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_lib_readPtnFromNvram(char *base, char *buf, int len)
{
    int ret = OK;
    ret = nvram_flash_read_tp_partition(base + NM_NVRAM_BASE, buf, len);
    return ret;
}


/*******************************************************************
 * Name		: nm_lib_initPtnStruct
 * Abstract	: parse partition-index-file to runtime-partition-struct.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_lib_initPtnStruct(void)
{
    prom_printf("%s %x\n", __FUNCTION__, &g_nmPtnStructEntity);
    memset(&g_nmPtnStructEntity, 0, sizeof(g_nmPtnStructEntity));
    g_nmPtnStruct = &g_nmPtnStructEntity;
    
    return OK;
}




/*******************************************************************
 * Name		: nm_lib_parsePtnIndexFile
 * Abstract	: parse partition-index-file to runtime-partition-struct.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_lib_parsePtnIndexFile(NM_PTN_STRUCT *ptnStruct, char *ptr)
{   
    int index = 0;
    int paraId = -1;
	
    int argc;
    char *argv[NM_PTN_INDEX_ARG_NUM_MAX];
	
    char buf[NM_PTN_INDEX_SIZE+1] = {0};
    NM_PTN_ENTRY *currPtnEntry = NULL;

    /* reset partition-table param */
    memset(ptnStruct, 0, sizeof(NM_PTN_STRUCT));
    for (index=0; index<NM_PTN_NUM_MAX; index++)
    {
        ptnStruct->entries[index].usedFlag = FALSE;
    }

    strncpy((char *)buf, (char *)ptr, NM_PTN_INDEX_SIZE+1);

    argc = nm_lib_makeArgs(buf, argv, NM_PTN_INDEX_ARG_NUM_MAX);
    
    index = 0;

    while (index < argc)
    {
        if ((paraId = nm_lib_strToKey(nm_ptnIndexFileParaStrMap, argv[index])) < 0)
        {
            NM_ERROR("invalid partition-index-file para id.");
            goto error;
        }

        index++;

        switch (paraId)
        {
        case NM_PTN_INDEX_PARA_ID_NAME:
            /* check if this partition-name already be used */
            if (nm_lib_ptnNameToEntry(ptnStruct, argv[index]) != NULL)
            {
                NM_ERROR("duplicate partition name found.");
                goto error;
            }
            
            /* get a new partition-entry */
            if ((currPtnEntry = nm_lib_fetchUnusedPtnEntry(ptnStruct)) == NULL)
            {
                NM_ERROR("too many partitions.");
                goto error;
            }
            strncpy(currPtnEntry->name, argv[index], NM_PTN_NAME_LEN);
            // NM_ERROR("idx:%d get parttion name:%s\n", index, currPtnEntry->name);
            currPtnEntry->usedFlag = TRUE;
            index++;
            break;
            
        case NM_PTN_INDEX_PARA_ID_BASE:
            if (nm_lib_parseU32((NM_UINT32 *)&currPtnEntry->base, argv[index]) < 0)
            {
                NM_ERROR("parse base-addr value failed.");
                goto error;
            }
            // NM_ERROR("idx:%d parttion name:%s base:%x\n", index, currPtnEntry->name, currPtnEntry->base);
            index++;
            break;

        case NM_PTN_INDEX_PARA_ID_TAIL:
            if (nm_lib_parseU32((NM_UINT32 *)&currPtnEntry->tail, argv[index]) < 0)
            {
                NM_ERROR("parse tail-addr value failed.");
                goto error;
            }
            // NM_ERROR("idx:%d parttion name:%s tail:%x\n", index, currPtnEntry->name, currPtnEntry->tail);
            index++;
            break;

        case NM_PTN_INDEX_PARA_ID_SIZE:
            if (nm_lib_parseU32((NM_UINT32 *)&currPtnEntry->size, argv[index]) < 0)
            {
                NM_ERROR("parse size value failed.");
                goto error;
            }
            // NM_ERROR("idx:%d parttion name:%s size:%x\n", index, currPtnEntry->name, currPtnEntry->size);
            index++;
            break;

        default:
            NM_ERROR("invalid para id.");
            goto error;
            break;
        }
        
    }
    
    return OK;
error:
    return ERROR;
}



/*******************************************************************
 * Name		: nm_lib_parsePtnTable
 * Abstract	: parse partition-table.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_lib_parsePtnTable(char *ptr)
{
    /* jump over "probe to os-image" */
    ptr += sizeof(int);

    return nm_lib_parsePtnIndexFile(g_nmPtnStruct, ptr);
}



/*******************************************************************
 * Name		: nm_lib_parsePtnTable
 * Abstract	: read and parse partition-table from NVRAM
 * Input	: 
 * Output	: 
 * Return	: 
 */
int nm_lib_readPtnTable(void)
{
    char ptnTable[NM_PTN_TABLE_SIZE];
    int ret = OK;

    prom_printf("%s:%d, addr of pantable:%x\n",__FUNCTION__,__LINE__ ,ptnTable);	
    nm_lib_initPtnStruct();
    memset(&ptnTable, 0, sizeof(ptnTable));

    NM_DEBUG("NM_PTN_TABLE_BASE = 0x%x",  NM_PTN_TABLE_BASE);
    
    ret = nm_lib_readPtnFromNvram((char *)NM_PTN_TABLE_BASE, (char *)&ptnTable, NM_PTN_TABLE_SIZE);
    if (ret < 0)
    {
        NM_ERROR("Reading Partition Table from NVRAM ... FAILED\r\n");	
        goto failed;
    }
    ret = nm_lib_parsePtnTable((char *)&ptnTable);
    if (OK != ret)
    {
        NM_ERROR("Parsing Partition Table ... FAILED\r\n");
		goto failed;
    }
    NM_DEBUG("Parsing Partition Table ... OK\r\n");

    return OK;

failed:
    nm_lib_initPtnStruct();
    return ERROR;
}

/*******************************************************************
 * Name		: nm_init
 * Abstract	: Initialize partition-struct and fwup PTN-struct.
 * Input	: 
 * Output	: 
 * Return	: OK/ERROR
 */
int nm_init()
{
    int ret = OK;

    if (l_nminited)
        return OK;

#if USE_LOCK	
	g_nmReadWriteLock = semBCreate(0, 1);

    if (g_nmReadWriteLock == (SEM_ID)NULL)
    {
        NM_ERROR("creating binary semaphore failed.");
        return ERROR;
    }
#endif
    prom_printf("%s:%d\n",__FUNCTION__,__LINE__);	
    nm_initFwupPtnStruct();

    ret = nm_lib_readPtnTable();
    if (OK != ret)   
    {
        return ERROR;
    }

    l_nminited = TRUE;

    return OK;
}

/**************************************************************************************************/
/*                                      GLOBAL_FUNCTIONS                                          */
/**************************************************************************************************/

