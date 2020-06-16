#include <linux/types.h>
#include <linux/device.h>
#include <linux/io.h>
#include <quectel/quec_raw_data_item.h>
#include <quectel/ql_data_interface.h>

static int net_interface = QL_DATA_INTERFACE_USB;
static int diag_interface = QL_DATA_INTERFACE_USB;

static int __init init_data_interface(char *s)
{
	int net = QL_DATA_INTERFACE_USB;
	int diag = QL_DATA_INTERFACE_USB;
	int ret;

	ret = sscanf(s, "[%d,%d]", &net, &diag);

	if (ret == 2) {
		if (net == QL_DATA_INTERFACE_USB || net == QL_DATA_INTERFACE_PCIE) {
			net_interface = net;
		}
		if (diag == QL_DATA_INTERFACE_USB || diag == QL_DATA_INTERFACE_PCIE) {
			diag_interface = diag;
		}
	}

	return 0;
}
__setup("data_interface=", init_data_interface);

static int get_boot_config(void) {
	unsigned int *boot_config;
	static unsigned int boot_config_value = -1;

	if (boot_config_value == -1) {
		boot_config = ioremap(0x00786070, 4);
		if (boot_config) {
			boot_config_value = *boot_config;
			iounmap(boot_config);
		} else {
			return 0;
		}
	}

	return boot_config_value;
}

int ql_get_net_interface(void) {
	if (get_boot_config() & 0x00000010) {
		return QL_DATA_INTERFACE_PCIE;
	}

	return net_interface;
}

int ql_get_diag_interface(void) {
	if (get_boot_config() & 0x00000010) {
		return QL_DATA_INTERFACE_PCIE;
	}

	return diag_interface;
}



