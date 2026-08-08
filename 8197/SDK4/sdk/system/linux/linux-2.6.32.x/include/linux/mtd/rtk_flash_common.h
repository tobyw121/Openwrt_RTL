/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 9021 $
 * $Date: 2010-04-13 15:45:15 +0800 (Tue, 13 Apr 2010) $
 *
 */

#ifndef _RTK_FLASH_COMMON_H
#define _RTK_FLASH_COMMON_H
#ifndef __UBOOT__
#include <common/rt_autoconf.h>
#endif /*__UBOOT__*/

#ifndef FLASH_BASE
#define FLASH_BASE 0xBD000000
#endif

#define UNIT_SIZE 65536  //only for this file

#if defined(CONFIG_FLASH_LAYOUT_TYPE1)

#if defined(CONFIG_FLASH_SIZE_4MB)
/* 4MB flash layout */
	#define LOADER_START        (0x00000000U)                            	//0x00000000
	#define LOADER_SIZE         (UNIT_SIZE*3)                            	//0x00030000
	#define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)               	//0x00030000
	#define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                               //0x00010000
	#define JFFS2_START         (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)    //0x00040000
	#define JFFS2_SIZE          (UNIT_SIZE*5)                               //0x00050000

	//#if defined(CONFIG_SQUASHFS_LZMA) | defined(CONFIG_CRAMFS) | defined(CONFIG_SQUASHFS)
	//    #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x00090000
	//    #define KERNEL_SIZE         (UNIT_SIZE*22)                          //0x00160000
	//    #define ROOTFS_START        (KERNEL_START+KERNEL_SIZE)              //0x001F0000
	//    #define ROOTFS_SIZE         (UNIT_SIZE*33)                          //0x00210000
	//#else /*for initramfs*/
	    #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x00090000
	    #define KERNEL_SIZE         (UNIT_SIZE*55)                          //0x00370000
	//#endif /*Readonly Filesystems*/
#elif defined(CONFIG_FLASH_SIZE_8MB)
/* 8MB flash layout */
	#define LOADER_START        (0x00000000U)                            	//0x00000000
	#define LOADER_SIZE         (UNIT_SIZE*4)                            	//0x00040000
	#define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)               	//0x00040000
	#define LOADER_BDINFO_SIZE  (UNIT_SIZE*2)                               //0x00020000
	#define JFFS2_START         (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)    //0x00060000
	#define JFFS2_SIZE          (UNIT_SIZE*10)                              //0x000A0000

	//#if defined(CONFIG_SQUASHFS_LZMA) | defined(CONFIG_CRAMFS) | defined(CONFIG_SQUASHFS)
	//    #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x00100000
	//    #define KERNEL_SIZE         (UNIT_SIZE*40)                          //0x00280000
	//    #define ROOTFS_START        (KERNEL_START+KERNEL_SIZE)              //0x00380000
	//    #define ROOTFS_SIZE         (UNIT_SIZE*72)                          //0x00480000
	//#else /*for initramfs*/
	    #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x00100000
	    #define KERNEL_SIZE         (UNIT_SIZE*112)                         //0x00700000
	//#endif /*Readonly Filesystems*/
#elif defined(CONFIG_FLASH_SIZE_16MB)
/* 16MB flash layout */
    #ifdef CONFIG_DUAL_IMAGE
        #define LOADER_START        (0x00000000U)                            	    //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*4)                            	    //0x00040000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)               	    //0x00040000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*2)                                   //0x00020000
        #define JFFS2_START         (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)        //0x00060000
        #define JFFS2_SIZE          (UNIT_SIZE*10)                                  //0x000A0000
        #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                        //0x00100000
        #define KERNEL_SIZE         CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x00700000 (default)
        #define KERNEL2_START       (KERNEL_START+CONFIG_DUAL_IMAGE_PARTITION_SIZE) //0x00800000
        #define KERNEL2_SIZE        CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x00700000 (default)
        #define SYSINFO_START       (0x1000000-UNIT_SIZE)                           //0x00FF0000
        #define SYSINFO_SIZE        (UNIT_SIZE)                                     //0x00010000
    #else
        #define LOADER_START        (0x00000000U)                            	 //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*4)                            	 //0x00040000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)               	 //0x00040000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*2)                                //0x00020000
        #define JFFS2_START         (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)     //0x00060000
        #define JFFS2_SIZE          (UNIT_SIZE*10)                               //0x000A0000
        #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                     //0x00100000
        #define KERNEL_SIZE         (UNIT_SIZE*112)                              //0x00700000
        /* 16MB without dual-image should be able to use more flash space as below setting.
           But our image is far less than it, so we use 0x700000 for saving upgrade image time now.
           If the image is larger than 0x700000 in the future, we can use below setting */
        //#define KERNEL_SIZE         (UNIT_SIZE*224)                              //0x00E00000
    #endif
#endif


#elif defined(CONFIG_FLASH_LAYOUT_TYPE2)
/* Turnkey 2.x flash layout */
#if defined(CONFIG_FLASH_SIZE_4MB)
/* 4MB flash layout */
    #define LOADER_START        (0x00000000U)                               //0x00000000
    #define LOADER_SIZE         (UNIT_SIZE*3)                               //0x00030000
    #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                  //0x00030000
    #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                               //0x00010000
    #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)    //0x00040000
    #define SYSINFO_SIZE        (UNIT_SIZE*1)                               //0x00010000
    #define JFFS2_START         (SYSINFO_START+SYSINFO_SIZE)                //0x00050000
    #define JFFS2_SIZE          (UNIT_SIZE*5)                               //0x00050000

    //#if defined(CONFIG_SQUASHFS_LZMA) | defined(CONFIG_CRAMFS) | defined(CONFIG_SQUASHFS)
    //    #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x000A0000
    //    #define KERNEL_SIZE         (UNIT_SIZE*22)                          //0x00160000
    //    #define ROOTFS_START        (KERNEL_START+KERNEL_SIZE)              //0x00200000
    //    #define ROOTFS_SIZE         (UNIT_SIZE*32)                          //0x00200000
    //#else /*for initramfs*/
        #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x000A0000
        #define KERNEL_SIZE         (UNIT_SIZE*54)                          //0x00360000
    //#endif /*Readonly Filesystems*/
#elif defined(CONFIG_FLASH_SIZE_8MB)
/* 8MB flash layout */
    #define LOADER_START        (0x00000000U)                               //0x00000000
    #define LOADER_SIZE         (UNIT_SIZE*4)                               //0x00040000
    #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                  //0x00040000
    #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                               //0x00010000
    #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)    //0x00050000
    #define SYSINFO_SIZE        (UNIT_SIZE*1)                               //0x00010000
    #define JFFS2_START         (SYSINFO_START+SYSINFO_SIZE)                //0x00060000
    #define JFFS2_SIZE          (UNIT_SIZE*16)                              //0x00100000

    //#if defined(CONFIG_SQUASHFS_LZMA) | defined(CONFIG_CRAMFS) | defined(CONFIG_SQUASHFS)
    //    #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x00160000
    //    #define KERNEL_SIZE         (UNIT_SIZE*40)                          //0x002E0000
    //    #define ROOTFS_START        (KERNEL_START+KERNEL_SIZE)              //0x003E0000
    //    #define ROOTFS_SIZE         (UNIT_SIZE*72)                          //0x004E0000
    //#else /*for initramfs*/
        #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x00160000
        #define KERNEL_SIZE         (UNIT_SIZE*106)                         //0x006A0000
    //#endif /*Readonly Filesystems*/
#elif defined(CONFIG_FLASH_SIZE_16MB)
/* 16MB flash layout */
    #ifdef CONFIG_DUAL_IMAGE
        #define LOADER_START        (0x00000000U)                                   //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*4)                                   //0x00040000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                      //0x00040000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                   //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)        //0x00050000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                   //0x00010000
        #define JFFS2_START         (SYSINFO_START+SYSINFO_SIZE)                    //0x00060000
        #define JFFS2_SIZE          (UNIT_SIZE*32)                                  //0x00200000
        #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                        //0x00260000
        #define KERNEL_SIZE         CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x006D0000 (default)
        #define KERNEL2_START       (KERNEL_START+CONFIG_DUAL_IMAGE_PARTITION_SIZE) //0x00930000
        #define KERNEL2_SIZE        CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x006D0000 (default)
    #else
        #define LOADER_START        (0x00000000U)                                //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*4)                                //0x00040000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                   //0x00040000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)     //0x00050000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                //0x00010000
        #define JFFS2_START         (SYSINFO_START+SYSINFO_SIZE)                 //0x00060000
        #define JFFS2_SIZE          (UNIT_SIZE*32)                               //0x00200000
        #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                     //0x00260000
        #define KERNEL_SIZE         (UNIT_SIZE*112)                              //0x00700000
        /* 16MB without dual-image should be able to use more flash space as below setting.
           But our image is far less than it, so we use 0x700000 for saving upgrade image time now.
           If the image is larger than 0x700000 in the future, we can use below setting */
        //#define KERNEL_SIZE         (UNIT_SIZE*224)                              //0x00E00000
    #endif
#endif

#elif defined(CONFIG_FLASH_LAYOUT_TYPE3)
/* Turnkey 2.x flash layout */
#if defined(CONFIG_FLASH_SIZE_4MB)
/* 4MB flash layout */
    #define LOADER_START        (0x00000000U)                               //0x00000000
    #define LOADER_SIZE         (UNIT_SIZE*3)                               //0x00030000
    #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                  //0x00030000
    #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                               //0x00010000
    #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)    //0x00040000
    #define SYSINFO_SIZE        (UNIT_SIZE*1)                               //0x00010000
    #define JFFS2_START         (SYSINFO_START+SYSINFO_SIZE)                //0x00050000
    #define JFFS2_SIZE          (UNIT_SIZE*5)                               //0x00050000

    //#if defined(CONFIG_SQUASHFS_LZMA) | defined(CONFIG_CRAMFS) | defined(CONFIG_SQUASHFS)
    //    #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x000A0000
    //    #define KERNEL_SIZE         (UNIT_SIZE*22)                          //0x00160000
    //    #define ROOTFS_START        (KERNEL_START+KERNEL_SIZE)              //0x00200000
    //    #define ROOTFS_SIZE         (UNIT_SIZE*32)                          //0x00200000
    //#else /*for initramfs*/
        #define KERNEL_START        (JFFS2_START+JFFS2_SIZE)                //0x000A0000
        #define KERNEL_SIZE         (UNIT_SIZE*54)                          //0x00360000
    //#endif /*Readonly Filesystems*/
#elif defined(CONFIG_FLASH_SIZE_8MB)
/* 8MB flash layout */
    #define LOADER_START        (0x00000000U)                               //0x00000000
    #define LOADER_SIZE         (UNIT_SIZE*4)                               //0x00040000
    #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                  //0x00040000
    #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                               //0x00010000
    #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)    //0x00050000
    #define SYSINFO_SIZE        (UNIT_SIZE*1)                               //0x00010000
    #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                //0x00060000
    #define JFFS2_CFG_SIZE      (UNIT_SIZE*8)                               //0x00080000
    #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)            //0x000E0000
    #define JFFS2_LOG_SIZE      (UNIT_SIZE*8)                               //0x00080000
    #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)            //0x00160000
    #define KERNEL_SIZE         (UNIT_SIZE*106)                             //0x006A0000
#elif defined(CONFIG_FLASH_SIZE_16MB)
/* 16MB flash layout */
    #ifdef CONFIG_DUAL_IMAGE
        #define LOADER_START        (0x00000000U)                                   //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*4)                                   //0x00040000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                      //0x00040000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                   //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)        //0x00050000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                   //0x00010000
        #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                    //0x00060000
        #define JFFS2_CFG_SIZE      (UNIT_SIZE*16)                                  //0x00100000
        #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)                //0x00160000
        #define JFFS2_LOG_SIZE      (UNIT_SIZE*16)                                  //0x00100000
        #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)                //0x00260000
        #define KERNEL_SIZE         CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x006D0000 (default)
        #define KERNEL2_START       (KERNEL_START+CONFIG_DUAL_IMAGE_PARTITION_SIZE) //0x00930000
        #define KERNEL2_SIZE        CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x006D0000 (default)
    #else
        #define LOADER_START        (0x00000000U)                                //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*4)                                //0x00040000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                   //0x00040000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)     //0x00050000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                //0x00010000
        #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                 //0x00060000
        #define JFFS2_CFG_SIZE      (UNIT_SIZE*16)                               //0x00100000
        #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)             //0x00160000
        #define JFFS2_LOG_SIZE      (UNIT_SIZE*16)                               //0x00100000
        #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)             //0x00260000
        #define KERNEL_SIZE         (UNIT_SIZE*112)                              //0x00700000
        /* 16MB without dual-image should be able to use more flash space as below setting.
           But our image is far less than it, so we use 0x700000 for saving upgrade image time now.
           If the image is larger than 0x700000 in the future, we can use below setting */
        //#define KERNEL_SIZE         (UNIT_SIZE*224)                              //0x00E00000
    #endif
#endif

#elif defined(CONFIG_FLASH_LAYOUT_TYPE4)
/* Turnkey 3.x flash layout */
#if defined(CONFIG_FLASH_SIZE_8MB)
/* 8MB flash layout */
    #define LOADER_START        (0x00000000U)                               //0x00000000
    #define LOADER_SIZE         (UNIT_SIZE*8)                               //0x00080000
    #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                  //0x00080000
    #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                               //0x00010000
    #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)    //0x00090000
    #define SYSINFO_SIZE        (UNIT_SIZE*1)                               //0x00010000
    #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                //0x000A0000
    #define JFFS2_CFG_SIZE      (UNIT_SIZE*8)                               //0x00080000
    #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)            //0x00120000
    #define JFFS2_LOG_SIZE      (UNIT_SIZE*8)                               //0x00080000
    #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)            //0x001A0000
    #define KERNEL_SIZE         (UNIT_SIZE*102)                             //0x00660000
#elif defined(CONFIG_FLASH_SIZE_16MB)
/* 16MB flash layout */
    #ifdef CONFIG_DUAL_IMAGE
        #define LOADER_START        (0x00000000U)                                   //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*8)                                   //0x00080000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                      //0x00080000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                   //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)        //0x00090000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                   //0x00010000
        #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                    //0x000A0000
        #define JFFS2_CFG_SIZE      (UNIT_SIZE*16)                                  //0x00100000
        #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)                //0x001A0000
        #define JFFS2_LOG_SIZE      (UNIT_SIZE*16)                                  //0x00100000
        #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)                //0x002A0000
        #define KERNEL_SIZE         CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x006B0000 (default)
        #define KERNEL2_START       (KERNEL_START+CONFIG_DUAL_IMAGE_PARTITION_SIZE) //0x00950000
        #define KERNEL2_SIZE        CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x006B0000 (default)
    #else
        #define LOADER_START        (0x00000000U)                                //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*8)                                //0x00080000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                   //0x00080000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)     //0x00090000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                //0x00010000
        #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                 //0x000A0000
        #define JFFS2_CFG_SIZE      (UNIT_SIZE*16)                               //0x00100000
        #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)             //0x001A0000
        #define JFFS2_LOG_SIZE      (UNIT_SIZE*16)                               //0x00100000
        #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)             //0x002A0000
        #define KERNEL_SIZE         (UNIT_SIZE*112)                              //0x00700000
        /* 16MB without dual-image should be able to use more flash space as below setting.
           But our image is far less than it, so we use 0x700000 for saving upgrade image time now.
           If the image is larger than 0x700000 in the future, we can use below setting */
        //#define KERNEL_SIZE         (UNIT_SIZE*224)                              //0x00E00000
    #endif
#elif defined(CONFIG_FLASH_SIZE_32MB)
    /* 32MB flash layout */
    #ifdef CONFIG_DUAL_IMAGE
        #define LOADER_START        (0x00000000U)                                   //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*8)                                   //0x00080000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                      //0x00080000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                   //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)        //0x00090000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                   //0x00010000
        #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                    //0x000A0000
        #define JFFS2_CFG_SIZE      (UNIT_SIZE*64)                                  //0x00400000
        #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)                //0x004A0000
        #define JFFS2_LOG_SIZE      (UNIT_SIZE*16)                                  //0x00100000
        #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)                //0x005A0000
        #define KERNEL_SIZE         CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x00D30000 (default)
        #define KERNEL2_START       (KERNEL_START+CONFIG_DUAL_IMAGE_PARTITION_SIZE) //0x012D0000
        #define KERNEL2_SIZE        CONFIG_DUAL_IMAGE_PARTITION_SIZE                //0x00D30000 (default)
    #else
        #define LOADER_START        (0x00000000U)                                //0x00000000
        #define LOADER_SIZE         (UNIT_SIZE*8)                                //0x00080000
        #define LOADER_BDINFO_START (LOADER_START+LOADER_SIZE)                   //0x00080000
        #define LOADER_BDINFO_SIZE  (UNIT_SIZE*1)                                //0x00010000
        #define SYSINFO_START       (LOADER_BDINFO_START+LOADER_BDINFO_SIZE)     //0x00090000
        #define SYSINFO_SIZE        (UNIT_SIZE*1)                                //0x00010000
        #define JFFS2_CFG_START     (SYSINFO_START+SYSINFO_SIZE)                 //0x000A0000
        #define JFFS2_CFG_SIZE      (UNIT_SIZE*64)                               //0x00400000
        #define JFFS2_LOG_START     (JFFS2_CFG_START+JFFS2_CFG_SIZE)             //0x004A0000
        #define JFFS2_LOG_SIZE      (UNIT_SIZE*16)                               //0x00100000
        #define KERNEL_START        (JFFS2_LOG_START+JFFS2_LOG_SIZE)             //0x005A0000
        #define KERNEL_SIZE         (UNIT_SIZE*256)                              //0x01000000
            /* 32MB without dual-image should be able to use more flash space as below setting.
               But our image is far less than it, so we use 0x1000000 for saving upgrade image time now.
               If the image is larger than 0x1000000 in the future, we can use below setting */
            //#define KERNEL_SIZE         (UNIT_SIZE*443)                        //0x00E00000
    #endif
#endif

#endif /* CONFIG_FLASH_LAYOUT_TYPE4 */

#endif /*_RTK_FLASH_COMMON_H*/

