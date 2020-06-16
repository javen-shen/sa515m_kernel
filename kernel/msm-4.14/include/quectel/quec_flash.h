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
 |	2019/01/19				Mike			 Adds usb feature
 |	------------------------------------------------------------------------
 \=========================================================================*/

#ifndef _QUEC_FLASH_H_
#define _QUEC_FLASH_H_

#include <linux/mtd/mtd.h>

enum flash_block_state
{
	FLASH_BLOCK_OK	= 0x1,			 /**< Flash Block is Good */
	FLASH_BLOCK_BAD = 0x2,			 /**< Flash Block is Bad */
	FLASH_BLOCK_RESERVED = 0x4,  /**< Reserved State */
	FLASH_BLOCK_STATE_UNKNOWN = 0x7FFFFFFF /* Force 32-bit enum */
};

#define FS_DEVICE_OK          (0)
#define FLASH_DEVICE_DONE     (0)

typedef uint32_t page_id;
typedef struct mtd_info* flash_handle_t;

flash_handle_t quec_flash_open(unsigned char* partition);
int quec_flash_close(flash_handle_t handle);

int quec_flash_read_page (flash_handle_t handle, page_id page, void *data);
int quec_flash_write_page (flash_handle_t handle, page_id page, void *data);

int quec_flash_erase_block(flash_handle_t handle, int block_id);
bool quec_flash_is_block_good(flash_handle_t handle, int block_id);
int quec_flash_set_block_state(flash_handle_t handle, int block_id, enum flash_block_state block_state);

page_id quec_flash_get_page_id(flash_handle_t handle,int block_id, int page_index_in_block);
uint32_t quec_flash_get_block_count(flash_handle_t handle);

uint32_t quec_flash_pages_per_block (flash_handle_t handle);
uint32_t quec_flash_page_size (flash_handle_t handle);

#endif //_QUEC_FLASH_H_


