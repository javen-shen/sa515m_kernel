/*==========================================================================
 |							QUECTEL - Build a smart world.
 |
 |							Copyright(c) 2017 QUECTEL Incorporated.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |			quectel raw data 
 |
 |--------------------------------------------------------------------------
 |
 |	Designed by 		: 	will.shao
 |--------------------------------------------------------------------------
 | Revision History
 | ----------------
 |	2018/05/04				will.shao 		 Initialization
 |      2019/06/10       francis        add plus data item(normal (256)<14,plus(1024)<4)
 |--------------------------------------------------------------------------
 \=========================================================================*/

#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/capability.h>
#include <linux/export.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <quectel/quec_flash.h>
#include <quectel/quec_raw_flash_data.h>
#include "quec_crc.h"


#define QUEC_IOCTL_RAWDATA_GET_ITEM 		(0x00050001)
#define QUEC_IOCTL_RAWDATA_SET_ITEM 		(0x00050002)

#define RAW_DATA_ITEM_MAX_LEN   (256)
#define RAW_DATA_PLUS_ITEM_MAX_LEN   (1024)
#define RAW_DATA_ITEM_NUMS      (14)
#define RAW_DATA_PLUS_ITEM_NUMS      (4)
#define RAW_DATA_CACHE_SIZE     (1024*8)


typedef QUEC_PACKED struct
{
	int data_len;
	uint8_t data[RAW_DATA_ITEM_MAX_LEN];
} QUEC_PACKED_POST raw_data_type;

typedef QUEC_PACKED struct
{
	int data_len;
	uint8_t data[RAW_DATA_PLUS_ITEM_MAX_LEN];
} QUEC_PACKED_POST raw_data_plus_type;

typedef QUEC_PACKED struct {
	int id;
	raw_data_type value;
} QUEC_PACKED_POST rawdata_item;

typedef QUEC_PACKED struct
{
	raw_data_type items[RAW_DATA_ITEM_NUMS];
	raw_data_plus_type plus_items[RAW_DATA_PLUS_ITEM_NUMS];
	uint32_t crc;
} QUEC_PACKED_POST raw_data_list_type;

typedef struct
{
	char data_cache[RAW_DATA_CACHE_SIZE];
	bool data_dirty;
} raw_data_cache_type;

extern const uint32_t quec_raw_data_item_len_list[];
extern const uint32_t quec_raw_data_plus_item_len_list[];

static raw_data_cache_type* raw_data_cache = NULL;

static flash_handle_t raw_flash_handle = NULL;

static DEFINE_MUTEX(rawdata_lock);

atomic_t client_count = {0};

static int get_start_block_id(flash_handle_t handle)
{
	return 0;
}

static int get_next_block_id(int block_id)
{
	return (block_id + 1);
}

static bool verify_raw_config_data(raw_data_list_type* raw_data_list)
{
	uint8_t* data = (uint8_t*) raw_data_list;

	if (crc_32_calc(data, (sizeof(raw_data_list)-sizeof(raw_data_list->crc))*8, 0) 
			!= raw_data_list->crc)
	{
		return false;
	}

	return true;
}

static bool load_raw_data_list(void)
{
	int block_id;
	int i, j;
	char* cache_ptr = NULL;
	page_id page;
	raw_data_list_type* raw_data_list;
	uint32_t page_nums;
	uint32_t page_size;
	int data_len = sizeof(raw_data_list_type);
	char* page_buf;
	
	if (raw_flash_handle == NULL)
	{
		return false;
	}

	if (raw_data_cache == NULL)
	{
		raw_data_cache = (raw_data_cache_type*)kmalloc(sizeof(raw_data_cache_type), GFP_KERNEL);
		if (raw_data_cache == NULL)
		{
			return false;
		}
		memset(raw_data_cache, 0, sizeof(raw_data_list_type));
	}

	page_nums = quec_flash_pages_per_block(raw_flash_handle);
	page_size = quec_flash_page_size(raw_flash_handle);
	data_len = sizeof(raw_data_list_type);
	raw_data_list = (raw_data_list_type*) raw_data_cache->data_cache;
	
	memset(raw_data_list, 0, sizeof(raw_data_list_type));
	
	block_id = get_start_block_id(raw_flash_handle);
	page_buf = (char*) kmalloc(page_size, GFP_KERNEL);
	if (page_buf == NULL)
	{
		return false;
	}
	
	for (i=0; i<RAW_FLASH_DATA_BLOCK_NUMS; i++)
	{ 	
		if (quec_flash_is_block_good(raw_flash_handle, block_id) == false)
		{
			block_id = get_next_block_id(block_id);
			continue;
		}

		page = quec_flash_get_page_id(raw_flash_handle, block_id, 0);
		
		memset(raw_data_cache->data_cache, 0, sizeof(raw_data_cache->data_cache));
		cache_ptr = raw_data_cache->data_cache;

		data_len = sizeof(raw_data_list_type);
		
		for (j=0; j<page_nums; j++)
		{
			int copy_len = page_size > data_len ? data_len : page_size;
			
			if (quec_flash_read_page(raw_flash_handle, page+j, page_buf) != FLASH_DEVICE_DONE)
			{
				break;
			}

			memcpy(cache_ptr, page_buf, copy_len);
			cache_ptr += copy_len;
			data_len -= copy_len;

			if (data_len <= 0)
				break;
		}

		if ((data_len <= 0) && verify_raw_config_data(raw_data_list))
		{
			kfree(page_buf);
			return true;
		}

		block_id = get_next_block_id(block_id);
	}

	kfree(page_buf);
	
	return false;;
}

static bool quec_raw_flash_data_open(void)
{
	bool ret = false;

	mutex_lock(&rawdata_lock);
	
	if (sizeof(raw_data_list_type) > sizeof(raw_data_cache->data_cache))
	{
		goto out;
	}

	if (raw_flash_handle)
	{
		ret = true;
		atomic_inc(&client_count);
		goto out;
	}

	raw_flash_handle = quec_flash_open((unsigned char*)QUECTEL_RAW_DATA_PARTITION_NAME);
	if (raw_flash_handle == NULL)
	{
		goto out;
	}
	
	if (load_raw_data_list() == false)
	{
		raw_data_list_type* raw_data_list = (raw_data_list_type*) raw_data_cache->data_cache;
		memset(raw_data_list, 0, sizeof(raw_data_list_type));
	}

	raw_data_cache->data_dirty = false;
	ret = true;
	atomic_inc(&client_count);

out:
	mutex_unlock(&rawdata_lock);
	
	return ret;
}

static bool raw_flash_data_sync(bool lock_mutex)
{
	int block_id;
	int i, j;
	page_id page;
	raw_data_list_type* raw_data_list;
	bool ret = false;
	char* cache_ptr = NULL;
	uint32_t page_nums;
	uint32_t page_size;
	int data_len;
	int err;
	
	if (lock_mutex)
	{
		mutex_lock(&rawdata_lock);
	}

	if (raw_flash_handle == NULL)
	{
		goto out;
	}

	page_nums = quec_flash_pages_per_block(raw_flash_handle);
	page_size = quec_flash_page_size(raw_flash_handle);
	data_len = sizeof(raw_data_list_type);
	raw_data_list = (raw_data_list_type*) raw_data_cache->data_cache;
	
	if (raw_data_cache->data_dirty == false)
	{
		ret = true;
		goto out;
	}

	raw_data_list->crc = crc_32_calc((uint8_t*)raw_data_list, (sizeof(raw_data_list)-sizeof(raw_data_list->crc))*8, 0);

	block_id =	get_start_block_id(raw_flash_handle);

	for (i=0; i<RAW_FLASH_DATA_BLOCK_NUMS; i++)
	{
		if (quec_flash_is_block_good(raw_flash_handle, block_id) == false)
		{
			block_id = get_next_block_id(block_id);
			
			continue;
		}

		err = quec_flash_erase_block(raw_flash_handle, block_id);
		if (err != FS_DEVICE_OK)
		{
			if (err == -EIO)
			{
				quec_flash_set_block_state(raw_flash_handle, block_id, FLASH_BLOCK_BAD);
			}
			block_id = get_next_block_id(block_id);

			continue;
		}

		page = quec_flash_get_page_id(raw_flash_handle, block_id, 0);
		cache_ptr = raw_data_cache->data_cache;

		data_len = sizeof(raw_data_list_type);
		for (j=0; j<page_nums; j++)
		{
			int copy_len = page_size > data_len ? data_len : page_size;

			if (quec_flash_write_page(raw_flash_handle, page+j, (void*)cache_ptr) != FLASH_DEVICE_DONE)
			{
				break;
			}

			cache_ptr += copy_len;
			data_len -= copy_len;

			if (data_len <= 0)
			{
				ret = true;
				raw_data_cache->data_dirty = false;
				break;
			}
		}
		
		block_id = get_next_block_id(block_id);
	}

out:
	if (lock_mutex)
	{
		mutex_unlock(&rawdata_lock);
	}
	return ret;
}

bool quec_raw_flash_data_sync(void)
{
	return raw_flash_data_sync(true);
}

static bool quec_raw_flash_data_close(void)
{
	bool ret = false;
	
	mutex_lock(&rawdata_lock);

	if (raw_flash_handle == NULL)
	{
		goto out;
	}

	if (atomic_read(&client_count) > 0)
		atomic_dec(&client_count);
	
	if (atomic_read(&client_count) > 0)
	{
		ret = true;
		raw_flash_data_sync(false);
		goto out;
	}

	raw_flash_data_sync(false);

	if (quec_flash_close(raw_flash_handle) == FLASH_DEVICE_DONE)
	{
		raw_flash_handle = NULL;
		kfree(raw_data_cache);
		raw_data_cache = NULL;
		ret = true;
		goto out;
	}

out:
	mutex_unlock(&rawdata_lock);

	return ret;
}

static bool quec_raw_flash_data_write_item(quec_raw_data_id data_id, void* data)
{
	raw_data_list_type* raw_data_list;
	bool ret = false;

	mutex_lock(&rawdata_lock);

	if (raw_data_cache == NULL || raw_flash_handle == NULL)
	{
		goto out;
	}

	raw_data_list = (raw_data_list_type*) raw_data_cache->data_cache;


	if (data_id < RAW_DATA_MAX_ID )
	{

		if (quec_raw_data_item_len_list[data_id] == 0)
		{
			goto out;
		}
		if (((uint32_t)raw_data_list->items[data_id].data_len) == quec_raw_data_item_len_list[data_id]
				&& memcmp(raw_data_list->items[data_id].data, data, quec_raw_data_item_len_list[data_id]) == 0)
		{
			ret = true;
			goto out;
		}

		load_raw_data_list();	
		memset(raw_data_list->items[data_id].data, 0, sizeof(raw_data_list->items[data_id].data));
		memcpy(raw_data_list->items[data_id].data, data, quec_raw_data_item_len_list[data_id]);
		raw_data_list->items[data_id].data_len = quec_raw_data_item_len_list[data_id];
	}else if ( data_id < RAW_DATA_PLUS_MAX_ID ) {
        data_id = data_id - RAW_DATA_ITEM_NUMS;
		if (quec_raw_data_plus_item_len_list[data_id] == 0)
		{
			goto out;
		}

		if (((uint32_t)raw_data_list->plus_items[data_id].data_len) == quec_raw_data_plus_item_len_list[data_id]
				&& memcmp(raw_data_list->plus_items[data_id].data, data, quec_raw_data_plus_item_len_list[data_id]) == 0)
		{
			ret = true;
			goto out;
		}

		load_raw_data_list();	
		memset(raw_data_list->plus_items[data_id].data, 0, sizeof(raw_data_list->plus_items[data_id].data));
		memcpy(raw_data_list->plus_items[data_id].data, data, quec_raw_data_plus_item_len_list[data_id]);
		raw_data_list->plus_items[data_id].data_len = quec_raw_data_plus_item_len_list[data_id];  
	}else{
		goto out;
	}
	raw_data_cache->data_dirty = true;

	ret = true;

out:
	mutex_unlock(&rawdata_lock);

	return ret;
}

static bool quec_raw_flash_data_read_item(quec_raw_data_id data_id, void* data)
{
	raw_data_list_type* raw_data_list;
	bool ret = false;

	mutex_lock(&rawdata_lock);

	if (raw_data_cache == NULL || raw_flash_handle == NULL)
	{
		goto out;
	}

	raw_data_list = (raw_data_list_type*) raw_data_cache->data_cache;


	if (data_id < RAW_DATA_MAX_ID )
	{
		if (quec_raw_data_item_len_list[data_id] == 0)
		{
			goto out;
		}

		if (raw_data_list->items[data_id].data_len != quec_raw_data_item_len_list[data_id])
		{
			goto out;
		}

		memcpy(data, raw_data_list->items[data_id].data, quec_raw_data_item_len_list[data_id]);

	}else if ( data_id < RAW_DATA_PLUS_MAX_ID ) {
        data_id = data_id - RAW_DATA_ITEM_NUMS;
		if (quec_raw_data_plus_item_len_list[data_id] == 0)
		{
			goto out;
		}

		if (raw_data_list->plus_items[data_id].data_len != quec_raw_data_plus_item_len_list[data_id])
		{
			goto out;
		}

		memcpy(data, raw_data_list->plus_items[data_id].data, quec_raw_data_plus_item_len_list[data_id]);
	} else {
		goto out;
	}


		ret = true;
out:
	
	mutex_unlock(&rawdata_lock);
	return ret;
}

bool quec_rawdata_get_item(quec_raw_data_id data_id, void* data) {

	if(data == NULL)
	{
		return false;
	}

	if (!quec_raw_flash_data_open()) {
		printk("@quectel Waring:rawdata open failed \n");
		return false;
	}

	return quec_raw_flash_data_read_item(data_id,data);

}

bool quec_rawdata_set_item(quec_raw_data_id data_id, void* data) {

	if(data == NULL)
	{
		return false;
	}
	if (!quec_raw_flash_data_open()) {
		printk("@quectel Waring:rawdata open failed \n");
		return false;
	}

	if(false == quec_raw_flash_data_write_item(data_id,data))
	{
		return false;
	}

	quec_raw_flash_data_close();

	return true;
}


static int rawdata_open(struct inode *inode, struct file *file) {
	quec_raw_flash_data_open();
	return 0;
}

static int rawdata_release(struct inode *inode, struct file *file) {
	quec_raw_flash_data_close();
	return 0;
}

static long rawdata_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	rawdata_item item;
	int error = 0;
	
	switch (cmd) {
	case QUEC_IOCTL_RAWDATA_GET_ITEM:
		if (copy_from_user(&item, (rawdata_item __user *) arg,
				 sizeof(rawdata_item))) {
			error = -EFAULT;
			break;
		}

		if ((item.id < 0) || (item.id >= RAW_DATA_MAX_ID)) {
			error = -EFAULT;
			break;
		}

		if (!quec_raw_flash_data_open()) {
			error = -EFAULT;
			break;
		}
		item.value.data_len = quec_raw_data_item_len_list[item.id];

		if (quec_raw_flash_data_read_item(item.id, item.value.data)) {
			if (copy_to_user((void __user *)arg, &item, sizeof(rawdata_item))) {
				error = -EFAULT;
			}
		} else {
			error = -EFAULT;
		}
		quec_raw_flash_data_close();
		break;
		
	case QUEC_IOCTL_RAWDATA_SET_ITEM:
		if (copy_from_user(&item, (rawdata_item __user *) arg,
				 sizeof(rawdata_item))) {
			error = -EFAULT;
			break;
		}

		if ((item.id < 0) || (item.id >= RAW_DATA_MAX_ID)) {
			error = -EFAULT;
			break;
		}
		if (item.value.data_len != quec_raw_data_item_len_list[item.id]) {
			error = -EFAULT;
			break;
		}
			
		if (!quec_raw_flash_data_open()) {
			error = -EFAULT;
			break;
		}

		if (!quec_raw_flash_data_write_item(item.id, item.value.data)) {
			error = -EFAULT;
		}
		
		quec_raw_flash_data_close();
		break;
	}

	return error;
}

static const struct file_operations rawdata_fops = {
	.owner = THIS_MODULE,
	.open = rawdata_open,
	.unlocked_ioctl = rawdata_ioctl,
	.llseek = NULL,
	.release = rawdata_release,
};

static int __init quec_rawdata_proc_init(void)
{
	proc_create("quec_rawdata", 0, NULL, &rawdata_fops);
	return 0;
}

__initcall(quec_rawdata_proc_init);


