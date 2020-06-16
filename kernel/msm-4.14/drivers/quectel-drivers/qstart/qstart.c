/*!	@file
	qstart.c
	@brief
	This file provides apis to set restore flag to mtd nand
*/
/*===========================================================================
	Copyright(c) 2018 Quectel Wierless Solution,Co.,Ltd. All Rights Reserved.
	Quectel Wireless Solution Proprietary and Confidential.
============================================================================*/
/*===========================================================================
							EDIT HISTORY FOR MODULE
This section contains comments describing changees made to the module.
Notice that changes are listed in reverse chronological order.
WHEN		 WHO		WHAT,WHERE,WHY
----------	-----		----------------------------------------------------
3/25/2019	Francis		Init
============================================================================*/
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
#include <quectel/quec_raw_data_item.h>
#include <quectel/quec_raw_flash_data.h>


/*
typedef struct 
{
  uint32_t magic1;  
  uint32_t magic2;
  uint32_t page_count;
  uint32_t data_crc;
  
  uint32_t reserve1;  
  uint32_t reserve2;
  uint32_t reserve3;
  uint32_t reserve4;
} quec_cefs_file_header_type;
*/
static  ql_backup Flag_msg_init[10] = {
        {"efs2","sys_rev",0,0,0,0,0,0},
        {"usrdata","",0,0,0,0,0,0}
};


struct qstart_device_t{
	struct miscdevice misc;
};

static loff_t qfirst_goodblock_addr = 0;

struct qstart_device_t *qstart_devp;
struct timer_list qstart_poll_timer;
struct work_struct qstart_timer_work;

static DEFINE_MUTEX(qstart_timer_lock);

void Quectel_Erase_Partition(const char * partition_name);

bool boot_mode = false;
bool debug_mode = false;
static int __init quectel_set_bootmode(char *str)
{
    if (str) {
        boot_mode = true;
    }
    else {
        boot_mode = false;
    }
    return 0;
}
static int __init quectel_set_debugmode(char *str)
{
    if (str) {
        debug_mode = true;
    }
    else {
        debug_mode = false;
    }
    return 0;
}
early_param("recovery", quectel_set_bootmode);
early_param("debug", quectel_set_debugmode);

/**
 * Author : Darren
 * Date : 2017/6/30
 * get_bootmode - will return current start mode
 * 0 -- normode, system mount linuxfs as rootfs
 * 1 -- recoverymode, system mount recoveryfs as rootfs
 */
//modify by len 2018-1-18 for boot mode
bool get_bootmode(unsigned int *mode)
{
    return boot_mode;
}
//modify end
EXPORT_SYMBOL(get_bootmode);

bool get_debugmode(unsigned int *mode)
{
    return debug_mode;
}
EXPORT_SYMBOL(get_debugmode);

/******************************************************************************************
unsigned int Quectel_Is_EFS_Backup_Valid(void)
{
	size_t readlen = 0;
	quec_cefs_file_header_type BackupCefs_Info;
	uint32_t crc = 0;
	uint32_t i =0;
	uint64_t mtd_size;
	unsigned char *onepage = NULL;
	struct mtd_info *mtd = NULL;

	mtd = get_mtd_device_nm("sys_rev");
	if(IS_ERR(mtd))
	{
		printk("@Ramos get sys_rev mtd fail.!\r\n");
		goto efsBackupInvalid;
	}
	else
	{
		mtd_size = mtd->size;
		printk("@Ramos mtd->writesize =%d, mtd->erasesize:%d  blockcount\n",  mtd->writesize,mtd->erasesize);
		for(i=0; qfirst_goodblock_addr < mtd_size; i++)
		{
			qfirst_goodblock_addr = i * mtd->erasesize; 
			if(!mtd_block_isbad(mtd,qfirst_goodblock_addr)) 
				break;
		}

		onepage = kmalloc(mtd->writesize, GFP_KERNEL);
		if(NULL == onepage)
		{
			printk("@Ramos memory is not enough to onepage, line=%d\n", __LINE__);
			goto  efsBackupInvalid;
		}
		
		memset(onepage, 0x00, mtd->writesize);
		mtd_read(mtd, qfirst_goodblock_addr, mtd->writesize, &readlen ,onepage);
		if(readlen != mtd->writesize )
		{
			printk("@Ramos read Flag Failed!!,line=%d\r\n\r\n",__LINE__);
			goto  efsBackupInvalid;
		}
		memset((void *)&BackupCefs_Info, 0x00, sizeof(quec_cefs_file_header_type));
		memcpy((void *)&BackupCefs_Info, onepage, sizeof(quec_cefs_file_header_type));

		if((CEFS_FILE_MAGIC1 != BackupCefs_Info.magic1) || (CEFS_FILE_MAGIC2 != BackupCefs_Info.magic2))
		{
			printk("@Ramos efs2 restore file magic1 error !!!\r\n\r\n");
			goto  efsBackupInvalid;
		}
	}

	return 1;
efsBackupInvalid:
	if(onepage != NULL)
	{
		kfree(onepage);
		onepage = NULL;
	}
	return 0;
}

*/


static void qstart_poll_timer_cb(void)
{
}

static struct qstart_device_t qstart_device = {
	.misc = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "qstart",
	}
};

unsigned int Quectel_Is_BackupPartition_Exist(const char * sourc_partition)
{
	struct mtd_info *mtd = NULL;

	mtd = get_mtd_device_nm(sourc_partition);
	if(IS_ERR(mtd))
	{
		printk("@Ramos can't get modem backup partition !!!\r\n");
		return 0;
	}
	else
	{
		if(!strcmp(sourc_partition,"efs2"))
		{
/*			if(Quectel_Is_EFS_Backup_Valid)
			{
				return 1;
			}
			else
			{
				return 0;
			}
*/
		}
	}
}

unsigned int Quectel_Set_Partition_RestoreFlag(const char * partition_name,int mtd_nub, int where)
{
	ql_backup_info_type Flag_msg;
	int isExistFlag = 0;
	int i,ret=0;
	bool get_raw_ret;	

	memset(&Flag_msg,0x00,sizeof(Flag_msg));
	get_raw_ret = quec_rawdata_get_item(RAW_DATA_RESTORE_ID,&Flag_msg);

	if(false == get_raw_ret || ((true == get_raw_ret) && (Flag_msg.ql_backup_info[0].ql_mtd_nub == 0)))
	{
		for ( i = 0 ;i < 10;i++)
		{
			if(Flag_msg_init[i].ql_mtd_name[0] != 0)
			{
				if(!IS_ERR(get_mtd_device_nm(Flag_msg_init[i].ql_mtd_name))){
					Flag_msg_init[i].ql_mtd_nub=get_mtd_device_nm(Flag_msg_init[i].ql_mtd_name)->index;
					Flag_msg_init[i].total_size=get_mtd_device_nm(Flag_msg_init[i].ql_mtd_name)->size;
				}
				if(true) //always get mtd info , will error if partition update 
				{
					strcpy(Flag_msg.ql_backup_info[i].ql_mtd_name,Flag_msg_init[i].ql_mtd_name);
					strcpy(Flag_msg.ql_backup_info[i].ql_mtd_restore_name,Flag_msg_init[i].ql_mtd_restore_name);
					Flag_msg.ql_backup_info[i].ql_mtd_nub =  Flag_msg_init[i].ql_mtd_nub;
					Flag_msg.ql_backup_info[i].total_size =  Flag_msg_init[i].total_size;					
				}
			}else {	
				strcpy(Flag_msg.ql_backup_info[i].ql_mtd_name,"");
			}
		}

	}
	for ( i = 0 ;i < 10;i++)
	{
		if(Flag_msg.ql_backup_info[i].ql_mtd_name[0] != 0)
		{
			if( partition_name[0] == 0 )
			{
				if(Flag_msg.ql_backup_info[i].ql_mtd_nub == mtd_nub)
				{
					if(strlen(Flag_msg.ql_backup_info[i].ql_mtd_restore_name) == 0)
					{
						printk("the [%s] parition restore no exist,erase !!!\n",Flag_msg.ql_backup_info[i].ql_mtd_name);
						Flag_msg.ql_backup_info[i].crash[where] +=1;
						Flag_msg.ql_backup_info[i].restore_times +=1;
						if (!quec_rawdata_set_item(RAW_DATA_RESTORE_ID,&Flag_msg)) {
							printk("@quectel Waring:Write rawdata failed\n");
							return 0;
						}


						Quectel_Erase_Partition(Flag_msg.ql_backup_info[i].ql_mtd_name);

					}	
					Flag_msg.ql_backup_info[i].restore_flag = 1;
					Flag_msg.ql_backup_info[i].crash[where] +=1;
					isExistFlag = 1;
				}
			}else
			{

				if(!strcmp(Flag_msg.ql_backup_info[i].ql_mtd_name,partition_name))
				{
					if(strlen(Flag_msg.ql_backup_info[i].ql_mtd_restore_name) == 0)
					{
						printk("the [%s] parition restore no exist,erase !!!\n",Flag_msg.ql_backup_info[i].ql_mtd_name);
						Flag_msg.ql_backup_info[i].crash[where] +=1;
						Flag_msg.ql_backup_info[i].restore_times +=1;
						if (!quec_rawdata_set_item(RAW_DATA_RESTORE_ID,&Flag_msg)) {
							printk("@quectel Waring:Write rawdata failed\n");
							return 0;
						}

						Quectel_Erase_Partition(Flag_msg.ql_backup_info[i].ql_mtd_name);

					}	
					Flag_msg.ql_backup_info[i].restore_flag = 1;
					Flag_msg.ql_backup_info[i].crash[where] +=1;
					isExistFlag = 1;
				}

			}
		}	

	}

	if(isExistFlag == 0)
	{
		printk("Waring:Restore partition is not exist\n");
			return 0;
	}	


	if (!quec_rawdata_set_item(RAW_DATA_RESTORE_ID,&Flag_msg)) {
		printk("@quectel Waring:Write rawdata failed\n");
		return 0;
	}



	ret = 1;
	machine_restart(NULL);
exit:
	return ret;
}

void Quectel_Partition_Restore(const char * partition_name,int mtd_nub, int where)
{

    Quectel_Set_Partition_RestoreFlag(partition_name, mtd_nub ,where);
    return;

}

void Quectel_Erase_Partition(const char * partition_name)
{
	struct mtd_info *mtd = NULL;
	struct erase_info ei;
	int err,i;

#ifdef QUECTEL_FLASH_SUPPORT_1G                                             
    return;
#endif
	
	printk("@Ramos there are fatal errror on the %s partition , we must erase it !!!\r\n", partition_name);
	mtd = get_mtd_device_nm(partition_name);
	if(IS_ERR(mtd))
	{
		printk("ERROR!!!!!  @Ramos get  %s mtd fail.!\r\n", partition_name);
		return;
	}
	else
	{
		memset(&ei, 0, sizeof(struct erase_info));
		ei.mtd = mtd;
		ei.len = mtd->erasesize;
		for(i=0 ;  ; i++)
		{
			ei.addr = i*(mtd->erasesize);
			if(ei.addr  > mtd->size)
			{
				break;
			}
			err = mtd_erase(mtd, &ei); //
		}
	}
		
	machine_restart(NULL);

return;
}

EXPORT_SYMBOL(Quectel_Set_Partition_RestoreFlag);
EXPORT_SYMBOL(Quectel_Partition_Restore);

static void qstart_timer_handle(struct work_struct *work)
{

}

static void __exit qstart_exit(void)
{
	misc_deregister(&qstart_device.misc);
}

static int __init qstart_init(void)
{
	int ret;

	printk("@Ramos Qstart_init entry !!!\r\n");
	return ret;
}

late_initcall(qstart_init);

MODULE_DESCRIPTION("QUECTEL Start Driver");
MODULE_LICENSE("GPL v2");



