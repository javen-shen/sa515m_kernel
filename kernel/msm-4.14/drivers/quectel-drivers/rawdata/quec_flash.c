/*==========================================================================
 |							QUECTEL - Build a smart world.
 |
 |							Copyright(c) 2017 QUECTEL Incorporated.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |			quectel flash driver api 
 |
 |--------------------------------------------------------------------------
 |
 |	Designed by 		: 	will.shao
 |--------------------------------------------------------------------------
 | Revision History
 | ----------------
 |	2018/05/04				will.shao 		 Initialization
 |	------------------------------------------------------------------------
 \=========================================================================*/

#include <quectel/quec_flash.h>

flash_handle_t quec_flash_open(unsigned char* partition)
{
	return get_mtd_device_nm((const char *)partition);
}

int quec_flash_close(flash_handle_t handle)
{  
	put_mtd_device(handle);
	
	return 0;
}

int quec_flash_read_page (flash_handle_t handle, page_id page, void *data)
{
	size_t read_len;
	uint32_t page_size = quec_flash_page_size(handle);
	
	return mtd_read(handle,  page_size*page, page_size, &read_len, data);
}

int quec_flash_write_page (flash_handle_t handle, page_id page, void *data)
{
	size_t write_len;
	uint32_t page_size = quec_flash_page_size(handle);

	return mtd_write(handle, page_size*page, page_size, &write_len, data);
}

int quec_flash_erase_block(flash_handle_t handle, int block_id)
{
	struct erase_info ei;
	
	memset(&ei, 0, sizeof(struct erase_info));
	ei.mtd = handle;
	ei.addr = block_id * handle->erasesize;
	ei.len = handle->erasesize;
	
	//ei.callback  ----------- callback!!!!
 
	return mtd_erase(handle, &ei); 
}

bool quec_flash_is_block_good(flash_handle_t handle, int block_id)
{  
	return (mtd_block_isbad(handle,  block_id * handle->erasesize) == 0);
}

page_id quec_flash_get_page_id(flash_handle_t handle,int block_id, int page_index_in_block)
{
	uint32_t pages_per_block = handle->erasesize / handle->writesize;

	return block_id * pages_per_block + page_index_in_block;
}

uint32_t quec_flash_get_block_count(flash_handle_t handle)
{
	return mtd_div_by_eb(handle->size, handle);
}

int quec_flash_set_block_state(flash_handle_t handle, int block_id, enum flash_block_state block_state)
{
	if (block_state == FLASH_BLOCK_BAD) {
		return mtd_block_markbad(handle, block_id);
	}

	return -1;
}

uint32_t quec_flash_pages_per_block (flash_handle_t handle)
{
	if (handle == NULL)
		return 0;

	return handle->erasesize / handle->writesize;
}

uint32_t quec_flash_page_size (flash_handle_t handle)
{
	if (handle == NULL)
		return 0;
	
	return handle->writesize;
}


