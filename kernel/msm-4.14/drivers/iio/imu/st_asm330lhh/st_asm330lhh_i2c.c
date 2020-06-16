/*
 * STMicroelectronics st_asm330lhh i2c driver
 *
 * Copyright 2018 STMicroelectronics Inc.
 *
 * Lorenzo Bianconi <lorenzo.bianconi@st.com>
 *
 * Licensed under the GPL-2.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/of.h>

#include "st_asm330lhh.h"

/* added by bowen.gu:2019/12/11:add gpio control power on sensor */
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#define QUECTEL_SENSOR_POWER 1
/* end */       

static int st_asm330lhh_i2c_read(struct device *dev, u8 addr, int len, u8 *data)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct i2c_msg msg[2];
	uint8_t *buf;
	int ret = 0;

	buf = kmalloc(len + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].len = 1;
	msg[0].buf = buf;

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = buf+1;

	buf[0] = addr;
	ret = i2c_transfer(client->adapter, msg, 2);
	memcpy(data, buf + 1, len);
	kfree(buf);

	return ret;
}

static int st_asm330lhh_i2c_write(struct device *dev, u8 addr, int len, u8 *data)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct i2c_msg msg;
	uint8_t *send;
	int ret = 0;

	send = kmalloc(len + 1, GFP_KERNEL);
	if (!send)
		return -ENOMEM;

	send[0] = addr;
	memcpy(&send[1], data, len * sizeof(u8));

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.len = len + 1;
	msg.buf = send;

	ret = i2c_transfer(client->adapter, &msg, 1);
	kfree(send);

	return ret;
}

static const struct st_asm330lhh_transfer_function st_asm330lhh_transfer_fn = {
	.read = st_asm330lhh_i2c_read,
	.write = st_asm330lhh_i2c_write,
};

/* aded by bowen.gu 2019/12/11 AG550 use gpio pin to control sensor's power*/
static int quectel_sensor_poweron(struct i2c_client *client)
{
    int gpio = 0;
    int err = 0;

    gpio = of_get_named_gpio_flags(client->dev.of_node, "qcom,power-gpio", 0, NULL);
    if (gpio > 0) {
        printk("asm330lhh sensor power gpio =%d \n", gpio);
        err = gpio_request(gpio, "qcom,power-gpio");
        if (err) {
            printk("asm330lhh gpio_requet failed\n");
        }

        err = gpio_direction_output(gpio, 1);
        if (err) {
            printk("asm330lhh gpio_direction failed\n");
            gpio_free(gpio);
        }
        msleep(100);

    } else {
        printk(" asm330lhh failed to get of node qcom,power-gpio gpio=%d \n", gpio);
    }

    return err;
}
/* end */


static int st_asm330lhh_i2c_probe(struct i2c_client *client,
        const struct i2c_device_id *id)
{
    //add by bowen.gu:2019/12/11:add gpio control power on sensor
    quectel_sensor_poweron(client);

    return st_asm330lhh_probe(&client->dev, client->irq,
            &st_asm330lhh_transfer_fn);
}

static const struct of_device_id st_asm330lhh_i2c_of_match[] = {
	{
		.compatible = "st,asm330lhh",
	},
	{},
};
MODULE_DEVICE_TABLE(of, st_asm330lhh_i2c_of_match);

static const struct i2c_device_id st_asm330lhh_i2c_id_table[] = {
	{ ST_ASM330LHH_DEV_NAME },
	{},
};
MODULE_DEVICE_TABLE(i2c, st_asm330lhh_i2c_id_table);

static struct i2c_driver st_asm330lhh_driver = {
	.driver = {
		.name = "st_asm330lhh_i2c",
		.pm = &st_asm330lhh_pm_ops,
		.of_match_table = of_match_ptr(st_asm330lhh_i2c_of_match),
	},
	.probe = st_asm330lhh_i2c_probe,
	.id_table = st_asm330lhh_i2c_id_table,
};
module_i2c_driver(st_asm330lhh_driver);

MODULE_AUTHOR("Lorenzo Bianconi <lorenzo.bianconi@st.com>");
MODULE_DESCRIPTION("STMicroelectronics st_asm330lhh i2c driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(ST_ASM330LHH_VERSION);
