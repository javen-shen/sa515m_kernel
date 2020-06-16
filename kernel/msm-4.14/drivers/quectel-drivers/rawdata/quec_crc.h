/*==========================================================================
 |							QUECTEL - Build a smart world.
 |
 |							Copyright(c) 2017 QUECTEL Incorporated.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |			quectel crc 
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

#ifndef _QUEC_CRC_H_
#define _QUEC_CRC_H_

uint32_t crc_32_calc
(
	/* Pointer to data over which to compute CRC */
	uint8_t *buf_ptr,

	/* Number of bits over which to compute CRC */
	uint32_t len,

	/* Seed for CRC computation */
	uint32_t seed
);

#endif //_QUEC_CRC_H_

