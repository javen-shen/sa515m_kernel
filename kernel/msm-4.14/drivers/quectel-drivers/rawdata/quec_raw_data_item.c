/*==========================================================================
 |							QUECTEL - Build a smart world.
 |
 |							Copyright(c) 2017 QUECTEL Incorporated.
 |
 |--------------------------------------------------------------------------
 | File Description
 | ----------------
 |			quectel raw data item
 |
 |--------------------------------------------------------------------------
 |
 |	Designed by 		: 	will.shao
 |--------------------------------------------------------------------------
 | Revision History
 | ----------------
 |	2018/05/04				will.shao 		 Initialization
 |  2019/06/10              francis          add plus data item(normal (256)<14,plus(1024)<4)
 |  2019/09/12              Lucas.Tan        Add new item to implement PS common configuration.
 |	------------------------------------------------------------------------
 \=========================================================================*/
#include <quectel/quec_raw_data_item.h>

const uint32_t quec_raw_data_item_len_list[RAW_DATA_MAX_ID] =
{
	sizeof(ql_usb_cfg_type),                  //RAW_DATA_USB_CFG
	sizeof(quec_download_info_type),          //RAW_DATA_DOWNLOAD_INFO_ID
	sizeof(quec_recovery_info_type),          //RAW_DATA_RECOVERY_INFO_ID
	sizeof(quec_recovery_history_type),       //RAW_DATA_RECOVERY_HISTORY_ID
	sizeof(uint32_t),                         //RAW_DATA_MODEM_LOAD_FAILED_ID
	sizeof(ql_crash_mode_type),               //RAW_DATA_CRASH_MODE_ID
	sizeof(ql_update_pid_type),               //RAW_DATA_UPDATE_PID_ID
	sizeof(ql_ps_common_config_type),         //RAW_DATA_PS_COMMON_CONFIG_ID
};

const uint32_t quec_raw_data_plus_item_len_list[RAW_DATA_PLUS_MAX_ID] =
{
	sizeof(ql_backup_info_type),               //RAW_DATA_BACKUP_INFO_ID
};

