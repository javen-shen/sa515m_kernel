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
 |	2019/01/19				Mike			 Modify data type
 |	------------------------------------------------------------------------
 \=========================================================================*/


#ifndef _QUEC_RAW_FLASH_DATA_H
#define _QUEC_RAW_FLASH_DATA_H

#include <quectel/quec_raw_data_item.h>

bool quec_rawdata_set_item(quec_raw_data_id data_id, void* data);
bool quec_rawdata_get_item(quec_raw_data_id data_id, void* data);

#endif //_QUEC_RAW_FLASH_DATA_H


