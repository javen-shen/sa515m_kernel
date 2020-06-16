
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
 |	2019/01/19				Mike			 Adds usb feature
 |      2019/06/10       francis        add plus data item(normal (256)<14,plus(1024)<4)
 |	------------------------------------------------------------------------
 \=========================================================================*/

#ifndef _QUEC_RAW_DATA_ITEM_H_
#define _QUEC_RAW_DATA_ITEM_H_
//#include "quectel-features-config.h"  //quectel add

#include <linux/types.h>

#define QUEC_PACKED
#define QUEC_PACKED_POST __attribute__((__packed__))

#define RAW_FLASH_DATA_BLOCK_NUMS 			(5)
#define QUECTEL_RAW_DATA_PARTITION_NAME ("rawdata")

// RAW DATA ID
typedef enum
{
	RAW_DATA_USB_CFG = 0,
	RAW_DATA_DOWNLOAD_INFO_ID,
	RAW_DATA_RECOVERY_INFO_ID,
	RAW_DATA_RECOVERY_HISTORY_ID,
	RAW_DATA_MODEM_LOAD_FAILED_ID,
	RAW_DATA_CRASH_MODE_ID,
	RAW_DATA_UPDATE_PID_ID,
	RAW_DATA_PS_COMMON_CONFIG_ID,
	RAW_DATA_MAX_ID = 13 ,
	RAW_DATA_RESTORE_ID,
	RAW_DATA_PLUS_MAX_ID
} quec_raw_data_id;

typedef QUEC_PACKED struct
{
	unsigned short vid;
	unsigned short pid;
	char usb_manuf_info[32];
	char usb_product_info[32];
	unsigned char bootup;
	unsigned char usbnet;
	unsigned int mask;
	char resv[48];
	unsigned char wifi_mac[6];
} QUEC_PACKED_POST ql_system_cfg_type;


#define QUEC_RAW_USB_MANUF_DATA_LEN       32
#define QUEC_RAW_USB_PRODUCT_DATA_LEN     32
typedef enum
{
  USB_FUNC_DIAG,
  USB_FUNC_NMEA,
  USB_FUNC_AT,
  USB_FUNC_MODEM,
  USB_FUNC_RMNET,
  USB_FUNC_ADB,
  USB_FUNC_MAX
} ql_usb_func_enum;

typedef enum
{
  USB_NET_RMNET,
  USB_NET_ECM,
  USB_NET_MBIM,
  USB_NET_RNDIS,
  USB_NET_MAX,
} ql_usb_net_enum;

#define QL_DATA_INTERFACE_USB  (0)
#define QL_DATA_INTERFACE_PCIE (1)

// RAW DATA TYPE
typedef QUEC_PACKED struct
{
  int vid;
  int pid;
  uint8_t func[USB_FUNC_MAX];
  uint32_t net;
  unsigned char net_interface;
  unsigned char diag_interface;
  char usb_manuf_info[QUEC_RAW_USB_MANUF_DATA_LEN];
  char usb_product_info[QUEC_RAW_USB_PRODUCT_DATA_LEN];
} QUEC_PACKED_POST ql_usb_cfg_type;


typedef enum
{
	BOOT_MODE_DOWNLOAD = 0,
	BOOT_MODE_BACKUP_SBL,
	BOOT_MODE_NORMAL,
} boot_mode_type;

typedef QUEC_PACKED struct 
{
	uint32_t boot_mode;
	uint32_t port;
	uint32_t baudrate;
	uint32_t fota;
	
} QUEC_PACKED_POST quec_download_info_type;

#define PARTITION_NAME_LEN_MAX              (40)
#define PARTITION_RECOVERY_MAX              (10)
#define PARTITION_RECOVERY_WHERE_MAX        (12)

typedef QUEC_PACKED struct
{
	char partition[PARTITION_NAME_LEN_MAX];
	int where;
	int recovery_retry;
} QUEC_PACKED_POST quec_recovery_info_type;

typedef QUEC_PACKED struct
{
	QUEC_PACKED struct 
	{
		uint16_t restore_times[PARTITION_RECOVERY_WHERE_MAX];
	} QUEC_PACKED_POST partition[PARTITION_RECOVERY_MAX];
	
} QUEC_PACKED_POST quec_recovery_history_type;

#define CRASH_MODE_DOWNLOAD     (0)
#define CRASH_MODE_REBOOT       (1)

typedef QUEC_PACKED struct 
{
	uint32_t modem;
	uint32_t ap;
} QUEC_PACKED_POST ql_crash_mode_type;

typedef QUEC_PACKED struct 
{
	uint32_t pid;
	uint32_t reserved;
} QUEC_PACKED_POST ql_update_pid_type;


typedef QUEC_PACKED struct
{       
	char ql_mtd_name[12];
	char ql_mtd_restore_name[12];
	uint64_t total_size;
	uint32_t ql_mtd_nub;
	uint32_t restore_flag;
	uint32_t restore_times;
	uint32_t backup_times;
	uint32_t crash[12];
} QUEC_PACKED_POST  ql_backup;

typedef QUEC_PACKED struct
{
	ql_backup  ql_backup_info[10];
} QUEC_PACKED_POST  ql_backup_info_type;

typedef QUEC_PACKED struct
{
    uint32_t ps_ethernet_auto_boot;
    uint32_t ps_ethernet_speed;
    uint32_t ps_ethernet_an;
    uint32_t ps_ethernet_dm;
    char resv[240];
} QUEC_PACKED_POST ql_ps_common_config_type;

#endif //_QUEC_RAW_DATA_ITEM_H_

