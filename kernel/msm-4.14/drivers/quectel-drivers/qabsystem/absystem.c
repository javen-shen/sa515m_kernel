/* Copyright (c) 2010-2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and * only version 2 as published by the Free Software Foundation.  *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*===============================================================
 *when           who     what, where, why
 *--------------------------------------------------------------
 *10/06/19       dawn     add for MM project AB system
 *
 ===============================================================*/ 

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/reboot.h>
#include <linux/kernel.h>

#define FOTAINFO_PART       "rawdata"
#define FOTAINFO_BLOCK_A    6
#define FOTAINFO_BLOCK_B    7
#define FOTAINFO_PAGE       0
#define FOTA_MAGIC          "FOTA#"
#define FOTAINFO_SIZE (sizeof(struct fota_info))
#define FOTAINFO_CRCSIZE (FOTAINFO_SIZE-4)
#define CRC32_POLY    0xedb88320L
#define CRC32_INIT    0xffffffff
#define CRC32_XOROUT  0xffffffff


struct absys_device_t{
    struct miscdevice misc;
};

struct absys_device_t *absys_devp;

typedef enum 
{
	STAT_SUCCEED = 0,
    STAT_UPDATE = 1,
    STAT_BACKUP = 2,
	STAT_FAILED = 3,
    STAT_DONE = 4,
	STAT_MAX,
}ESTAT;

typedef enum
{
	FOTA_NULL = 0,
    FOTA_AB = 1,
    FOTA_FULL = 2,
    FOTA_DIFF = 3,
	TYPE_MAX,
}FOTATYPE;

typedef enum
{
	BOOT_SUCCEED = 0,
	BOOT_FAILED = 1,
}BOOTSTAT;


/**
 * struct fota_info - record fota update infomation
 * magic: fota info magic number(%FOTA_MAGIC)
 * activeslot: 0 current system is A, 1 current system is B
 * padding1: reserved for expand,full zero
 * type: FOTA update type, which include A/B FOTA,full fota and diff FOTA
 * eventstat: stat of update,which include update,backup and expand stat for future
 * upcount: the count of fota update success
 * ecount: the count of the fota info update  
 * fcount: the count of fota failed,which will be cleared when one fota update success happened
 * swcount: AB switch count
 * swposition: AB system switch position
 * padding2: reserved for expand,full zero
 * info_crc: fota_info crc code
 */
struct fota_info
{
    char magic[8];
    unsigned int activeslot;
    char padding1[32];
    unsigned int type;
    unsigned int eventstat[4];
    unsigned int upcount;
    unsigned int ecount;
    unsigned int fcount;
    unsigned int swcount;
    char swposition[16];
    char padding2[36];
	char package[128];
	char padding3[64];
    unsigned int info_crc;
};


static const unsigned int crc32tab[] = {  
     0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,  
     0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,  
     0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,  
     0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,  
     0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,  
     0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,  
     0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,  
     0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,  
     0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,  
     0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,  
     0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,  
     0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,  
     0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,  
     0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,  
     0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,  
     0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,  
     0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,  
     0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,  
     0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,  
     0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,  
     0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,  
     0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,  
     0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,  
     0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,  
     0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,  
     0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,  
     0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,  
     0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,  
     0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,  
     0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,  
     0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,  
     0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,  
     0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,  
     0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,  
     0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,  
     0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,  
     0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,  
     0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,  
     0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,  
     0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,  
     0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,  
     0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,  
     0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,  
     0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,  
     0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,  
     0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,  
     0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,  
     0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,  
     0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,  
     0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,  
     0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,  
     0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,  
     0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,  
     0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,  
     0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,  
     0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,  
     0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,  
     0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,  
     0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,  
     0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,  
     0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,  
     0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,  
     0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,  
     0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL  
};  

static unsigned int crc32(const unsigned char *buf, unsigned int size)  
{  
     unsigned int i, crc;  
     crc = CRC32_INIT;
  
     for (i = 0; i < size; i++)  
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);  
  
     return crc^CRC32_XOROUT;  
}

/*
*add by dawn 20190610
*read fotainfo from rawdata
*/
static int get_fota_info(struct fota_info *info)
{
    struct fota_info info_main;
    struct fota_info info_back;
    struct mtd_info *mtd = NULL;

    unsigned int pagesize = 0;
    unsigned int blocksize = 0;
    unsigned int crc_val = 0;
    unsigned char *onepage = NULL;

    unsigned int readlen = 0;
    bool success_flag = false;
    uint64_t offset=0;

    mtd = get_mtd_device_nm(FOTAINFO_PART);
    if(IS_ERR(mtd))
    {
        printk("@AB@ get rawdata mtd fail.!\r\n");
        return -1;
    }

    blocksize = mtd->erasesize;
    pagesize = mtd->writesize;
    onepage = kmalloc(mtd->writesize, GFP_KERNEL);
    if(NULL == onepage)
    {
        printk("@AB@ memory is not enough to onepage\n");
        goto exit;
    }

    //read fotainfo main block
    offset = FOTAINFO_BLOCK_A*blocksize;

    while(offset < mtd->size){
        if(!mtd_block_isbad(mtd,offset))
            break;
        offset += blocksize;
    }

    memset(onepage, 0x00, mtd->writesize);
    mtd_read(mtd, offset, mtd->writesize, &readlen ,onepage);
    if(readlen != mtd->writesize )
    {
        printk("@AB@ system  read fotainfo failed!!\r\n\r\n");
    }

    memcpy((void *)&info_main,(void *)onepage,FOTAINFO_SIZE);
    if (!strncmp(info_main.magic, FOTA_MAGIC, strlen(FOTA_MAGIC))) {
        crc_val = crc32((char *)&info_main, FOTAINFO_CRCSIZE);
        if (crc_val == info_main.info_crc) {
            memcpy((void *)info, (void *)&info_main, FOTAINFO_SIZE);
            success_flag = true;
            goto exit;
        }
    }

    printk("@AB@ read main fotainfo failed !! fun:%s, line:%d\n", __func__, __LINE__);

    //read fotainfo backup block
    offset += blocksize;
    while(offset < mtd->size){
        if(!mtd_block_isbad(mtd,offset))
            break;
        offset += blocksize;
    }

    memset(onepage, 0x00, mtd->writesize);
    mtd_read(mtd, offset, mtd->writesize, &readlen ,onepage);
    if(readlen != mtd->writesize )
    {
        printk("@AB@ system read backup fotainfo failed!!\n");
    }

    memcpy((void *)&info_back, (void *)onepage, FOTAINFO_SIZE);
    if (!strncmp(info_back.magic, FOTA_MAGIC, strlen(FOTA_MAGIC))) {
        crc_val = crc32((char *)&info_back, FOTAINFO_CRCSIZE);
        if (crc_val == info_back.info_crc) {
            memcpy((void *)info, (void *)&info_back, FOTAINFO_SIZE);
            success_flag = true;
            goto exit;
        }
    }

    printk("@AB@ read backup fotainfo failed !! fun:%s, line:%d\n", __func__, __LINE__);

exit:
    if(onepage != NULL){
        kfree(onepage);
        onepage = NULL;
    }

    if(success_flag)
    {
        return 0;
    }else{
        printk("@AB@ get fotainfo failed\n");
        return -1;
    }

}

/*
*add by dawn 20190610
*read fotainfo from rawdata
*/
static int get_both_fota_info(struct fota_info *main_info, struct fota_info *backup_info)
{
    struct fota_info info_main;
    struct fota_info info_back;
    struct mtd_info *mtd = NULL;

    unsigned int pagesize = 0;
    unsigned int blocksize = 0;
    unsigned int crc_val = 0;
    unsigned char *onepage = NULL;

    unsigned int readlen = 0;
    bool success_flag = false;
    uint64_t offset=0;

    if(main_info == NULL || backup_info == NULL)
        return -1;

    mtd = get_mtd_device_nm(FOTAINFO_PART);
    if(IS_ERR(mtd))
    {
        printk("@AB@ get rawdata mtd fail.!\r\n");
        return -1;
    }

    blocksize = mtd->erasesize;
    pagesize = mtd->writesize;
    onepage = kmalloc(mtd->writesize, GFP_KERNEL);
    if(NULL == onepage)
    {
        printk("@AB@ memory is not enough to onepage\n");
        goto exit;
    }

    //read fotainfo main block
    offset = FOTAINFO_BLOCK_A*blocksize;

    while(offset < mtd->size){
        if(!mtd_block_isbad(mtd,offset))
            break;
        offset += blocksize;
    }

    memset(onepage, 0x00, mtd->writesize);
    mtd_read(mtd, offset, mtd->writesize, &readlen ,onepage);
    if(readlen != mtd->writesize )
    {
        printk("@AB@ system  read fotainfo failed!!\r\n\r\n");
    }

    memcpy((void *)&info_main,(void *)onepage,FOTAINFO_SIZE);
    if (!strncmp(info_main.magic, FOTA_MAGIC, strlen(FOTA_MAGIC))) {
        crc_val = crc32((char *)&info_main, FOTAINFO_CRCSIZE);
        if (crc_val == info_main.info_crc) {
            memcpy((void *)main_info, (void *)&info_main, FOTAINFO_SIZE);
            //success_flag = true;
            //goto exit;
        }
    }

    printk("@AB@ read main fotainfo failed !! fun:%s, line:%d\n", __func__, __LINE__);

    //read fotainfo backup block
    offset += blocksize;
    while(offset < mtd->size){
        if(!mtd_block_isbad(mtd,offset))
            break;
        offset += blocksize;
    }

    memset(onepage, 0x00, mtd->writesize);
    mtd_read(mtd, offset, mtd->writesize, &readlen ,onepage);
    if(readlen != mtd->writesize )
    {
        printk("@AB@ system read backup fotainfo failed!!\n");
    }

    memcpy((void *)&info_back, (void *)onepage, FOTAINFO_SIZE);
    if (!strncmp(info_back.magic, FOTA_MAGIC, strlen(FOTA_MAGIC))) {
        crc_val = crc32((char *)&info_back, FOTAINFO_CRCSIZE);
        if (crc_val == info_back.info_crc) {
            memcpy((void *)backup_info, (void *)&info_back, FOTAINFO_SIZE);
            success_flag = true;
            goto exit;
        }
    }

    printk("@AB@ read backup fotainfo failed !! fun:%s, line:%d\n", __func__, __LINE__);

exit:
    if(onepage != NULL){
        kfree(onepage);
        onepage = NULL;
    }

    if(success_flag)
    {
        return 0;
    }else{
        printk("@AB@ get fotainfo failed\n");
        return -1;
    }

}


/*
*set fotainfo to rawdata
*/
static int set_fota_info(struct fota_info info)
{
    unsigned int pagesize = 0;
    unsigned int blocksize = 0;
    unsigned char *onepage = NULL;
	struct mtd_info *mtd = NULL;
	bool success_flag = true;
	uint64_t offset=0;
    struct erase_info ei;
    size_t write = 0;
    int err,i;

	mtd = get_mtd_device_nm(FOTAINFO_PART);
	if(IS_ERR(mtd))
	{
		printk("@AB@ get rawdata mtd fail.!\r\n");
		goto exit;
	}

	blocksize = mtd->erasesize;
	pagesize = mtd->writesize;
	onepage = kmalloc(mtd->writesize, GFP_KERNEL);
	if(NULL == onepage)
	{
		printk("@AB@ memory is not enough to onepage\n");
		goto exit;
	}

	//write to main block
	offset = FOTAINFO_BLOCK_A*blocksize;

	while(offset<mtd->size){
		if(!mtd_block_isbad(mtd,offset))
			break;
		offset += blocksize;

	}

	memset(onepage, 0x00, mtd->writesize);

	info.ecount++;
	info.info_crc = crc32((char *)&info, (unsigned int)FOTAINFO_CRCSIZE);

	memset(&ei, 0, sizeof(struct erase_info));
	ei.mtd = mtd;
	ei.addr = offset;
	ei.len = mtd->erasesize;

	err = mtd_erase(mtd, &ei); //erase block before write
	
    memcpy((void *)onepage, &info, FOTAINFO_SIZE);
	err = mtd_write(mtd, offset, mtd->writesize, &write, onepage);
	if(err || write != mtd->writesize)
	{
		printk("@AB@ set fotainfo failed\n");
		success_flag = false;
		goto exit;
	}

	//write to backup block
	offset += blocksize;
	while(offset<mtd->size){
		if(!mtd_block_isbad(mtd,offset))
			break;
		offset += blocksize;

	}

	memset(&ei, 0, sizeof(struct erase_info));
	ei.mtd = mtd;
	ei.addr = offset;
	ei.len = mtd->erasesize;

	err = mtd_erase(mtd, &ei); //erase block before write
	
    memcpy((void *)onepage, &info, FOTAINFO_SIZE);
	err = mtd_write(mtd, offset, mtd->writesize, &write, onepage);
	if(err || write != mtd->writesize)
	{
		printk("@AB@ set fotainfo failed\n");
		success_flag = false;
		goto exit;
	}

exit:
	if(onepage != NULL){
        kfree(onepage);
        onepage = NULL;
	}

    if(success_flag)
        return 0;
    else
        return -1;

}

static int dump_fotainfo(void)
{
	struct fota_info info;
	int ret = -1;

	ret = get_fota_info(&info);
	if (ret == 0)
	{
		printk("@AB@ fotainfo magic=%x\n", info.magic);
		printk("@AB@ fotainfo activeslot=%u\n", info.activeslot);
		printk("@AB@ fotainfo swcount=%u\n", info.swcount);
		printk("@AB@ fotainfo crc=%u\n", info.swcount);
		
	}else{
		printk("@AB@ fotainfo read error\n");
	}

	return 0;

}

/**
 * ab_system_switch()
 * Call this function if kernel boot failed.If another system is good,
 * then will switch into another system otherwise the system will go into fastboot mode to restore.
 * 
 */
int ab_system_switch(void)
{
    struct fota_info info;
    int ret = -1;

    ret = get_fota_info(&info);
    if (ret == 0)
    {
        memset(info.swposition, 0, sizeof(info.swposition));
        if (info.activeslot == 1)
        {
            strncpy(info.swposition, "b_system", strlen("b_system") + 1);
            printk("@AB@ the system will switch into A system\n");
            info.activeslot = 0;
        }
        else
        {
            strncpy(info.swposition, "system", strlen("system") + 1);
            printk("@AB@ the system will switch into B system\n");
            info.activeslot = 1;
        }

        info.swcount += 1;

        if (set_fota_info(info))
        {
            printk("@AB@ set fotainfo failed in kernel\n");
            return -1;
        }

        machine_restart(NULL);
    }

    printk("@AB@ get fotainfo fail in kernel\n");
    return ret;
}


static struct absys_device_t absys_device = {
    .misc = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "absys",
    }
};

EXPORT_SYMBOL(ab_system_switch);

static void __exit absys_exit(void)
{
    misc_deregister(&absys_device.misc);
}

static int __init absys_init(void)
{
    int ret;
    printk("@AB@ absys_init entry !!!\r\n");
    return ret;
}

module_init(absys_init);
module_exit(absys_exit);

MODULE_DESCRIPTION("QUECTEL AB SYSTEM Driver");
MODULE_LICENSE("GPL v2");

