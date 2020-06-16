/* Copyright (c) 2010-2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and * only version 2 as published by the Free Software Foundation.  *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*
 * Quectel ql_lpm driver
 *
 */ 

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#include <linux/skbuff.h>
#include <net/sock.h>

#define QL_LPM_NETLINK	28   
#define MAX_MSGSIZE	24

static int wakeup_in_irq = 0;
static struct sock *nl_sk = NULL;
static int user_pid = -1;
extern struct net init_net;

/*The below number is gpio number on Qcom BaseBand.*/
static int wakeup_in = 61;
static int sleep_sys_ind = 87;
static int sleep_sys_ind_state = 0;
static bool sleep_sys_ind_enabled = false; //gpio request success flag

int lpm_set_sleep_sys_ind_suspend(void)
{
    if(sleep_sys_ind_enabled)
        return gpio_direction_output(sleep_sys_ind,(sleep_sys_ind_state==0)?0:1);

    return -1;
}

int lpm_set_sleep_sys_ind_resume(void)
{
    if(sleep_sys_ind_enabled)
        return gpio_direction_output(sleep_sys_ind,(sleep_sys_ind_state==0)?1:0);

    return -1;
}

int lpm_send_state(char* msg, int len)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;

    int ret;

    skb = nlmsg_new(len, GFP_ATOMIC);
    if(!skb)
    {
        pr_err("[ql_lpm]:netlink alloc failure\n");
        return -1;
    }

    nlh = nlmsg_put(skb, 0, 0, QL_LPM_NETLINK, len, 0);
    if(nlh == NULL)
    {
        pr_err("[ql_lpm]:nlmsg_put failaure \n");
        nlmsg_free(skb);
        return -1;
    }
 
    memcpy(nlmsg_data(nlh), msg, len);
    if(user_pid == -1)
	return -1;
    ret = netlink_unicast(nl_sk, skb, user_pid, MSG_DONTWAIT);
    pr_debug("[ql_lpm]: Send wakeupin state to user space, ret: %d\n", ret);

    return ret;
}

static irqreturn_t quectel_wakeup_irq_func(int irq, void *id)
{
    //TODO: debounce
    int value = gpio_get_value(wakeup_in);

    if(value == 0)
        lpm_send_state("falling", strlen("falling"));
    else if(value == 1)
        lpm_send_state("rising", strlen("rising"));

    return IRQ_HANDLED;
}

static int wakeup_in_init(void)
{
    int err = 0;

    if(wakeup_in == -1)
    {
        return -1;
    }

    err = gpio_request(wakeup_in, "wakeup_in");
    if (err < 0)
    {
        pr_err("[ql_lpm][%s]: request wakeup_in: %d failed, error: %d\n", __FUNCTION__, wakeup_in, err);
        goto err_gpio_request;
    }
	
    err = gpio_direction_input(wakeup_in);
    if (err < 0)
    {
        pr_err("[ql_lpm][%s]: set wakeup_in:  direction input (%d) failed: %d\n", __FUNCTION__, wakeup_in, err);
        goto err_gpio_to_irq;
    }

    err = gpio_to_irq(wakeup_in);
    if (err < 0)
    {
        pr_err("[ql_lpm][%s]: wakeup_in: %d to irq failed, err: %d\n", __FUNCTION__, wakeup_in, err);
        goto err_gpio_to_irq;
    }
	
    wakeup_in_irq = err;
    
    err = request_any_context_irq(wakeup_in_irq, quectel_wakeup_irq_func,	\
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "wakeup_in_irq", NULL);
    if (err < 0)
    {
        pr_err("[ql_lpm][%s]: Can't request %d IRQ for wakeup_in: %d\n", __FUNCTION__, wakeup_in_irq, err);
        goto err_free_irq;
    }

    return 0;

err_free_irq:
    free_irq(wakeup_in_irq, NULL);
err_gpio_to_irq:
    gpio_free(wakeup_in);
err_gpio_request:
    return err;
}

static void wakeup_in_exit(void)
{	
    free_irq(wakeup_in_irq, NULL);
    disable_irq(wakeup_in_irq);
    gpio_free(wakeup_in);
}

static void netlink_rcv_msg(struct sk_buff *__skb)
{

     struct sk_buff *skb;
     struct nlmsghdr *nlh;
     pr_debug("[ql_lpm]: user data come in\n");
     skb = skb_get (__skb);
     if(skb->len >= NLMSG_SPACE(0))
     {
         nlh = nlmsg_hdr(skb);
         user_pid = nlh->nlmsg_pid;
         pr_info("[ql_lpm]: user ID:%d, messages: %s\n",user_pid, (char *)NLMSG_DATA(nlh));
         kfree_skb(skb);
    }
}

static int quectel_low_consume_suspend(struct platform_device *pdev, pm_message_t state)
{
    pr_debug("[ql_lpm][%s]\n", __FUNCTION__);

    /* enable wakeup_in wakeup function */
    if (enable_irq_wake(wakeup_in_irq) < 0)
    {
        pr_err("[ql_lpm][%s]: enable wakeup_in wakeup function failed\n", __FUNCTION__);
        return -1;
    }
	
    return 0;
}

static int quectel_low_consume_resume(struct platform_device *pdev)
{
    pr_debug("[ql_lpm][%s]\n", __FUNCTION__);

    /* disable wakeup_in wakeup function */
    if (disable_irq_wake(wakeup_in_irq) < 0)
    {
        pr_err("[ql_lpm][%s]: disable wakeup_in wakeup function failed\n", __FUNCTION__);
        return -1;
    }
	
    return 0;
}

struct netlink_kernel_cfg cfg = { 
    .input  = netlink_rcv_msg, /* set recv callback */
};  

static int quectel_low_consume_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device_node *node = pdev->dev.of_node;
    
    wakeup_in = of_get_gpio(node,0);
    sleep_sys_ind = of_get_gpio(node,1);
    of_property_read_s32(node,"ql,ql,sleep-sys-ind-state",&sleep_sys_ind_state); 
   
     
    ret = gpio_request(sleep_sys_ind, "sleep_sys_ind");
    if (ret < 0)
    {
        pr_err("[ql_lpm][%s]: request sleep_sys_ind: %d failed, error: %d\n", __FUNCTION__, sleep_sys_ind, ret);
        return -1;
    }
    sleep_sys_ind_enabled = true;
    lpm_set_sleep_sys_ind_resume();//set default sleep_sys_ind gpio state

    if((ret = wakeup_in_init()) < 0)
    {
        pr_err("[ql_lpm][%s]: wakeup_in init failed\n", __FUNCTION__);
        goto err_wakeup_in_exit;
    }

    nl_sk = netlink_kernel_create(&init_net, QL_LPM_NETLINK, &cfg);
    if(!nl_sk){
        pr_err("[ql_lpm]netlink: create netlink socket error.\n");
        ret = -1;
        goto err_wakeup_in_exit;
    }

    pr_debug("[ql_lpm]: create netlink socket ok.\n");
    
    return 0;

err_wakeup_in_exit:
    wakeup_in_exit();
    return ret;
}

static int quectel_low_consume_remove(struct platform_device *pdev)
{
    gpio_free(sleep_sys_ind);
    sleep_sys_ind_enabled = false;
    wakeup_in_exit();

    if(nl_sk != NULL)
    {
        sock_release(nl_sk->sk_socket);
    }
    pr_info("[ql_lpm][%s]: module exit.\n", __FUNCTION__);

    return 0;
}

static const struct of_device_id quectel_low_consume_match[] =
{
    {.compatible = "quec,ql_lpm",},
    {},
};

static struct platform_driver quectel_low_consume_driver = {
    .probe    = quectel_low_consume_probe,
    .remove   = quectel_low_consume_remove,
    .suspend  = quectel_low_consume_suspend,
    .resume   = quectel_low_consume_resume,
    .driver   = {
        .name = "ql_lpm",
        .owner = THIS_MODULE,
        .of_match_table = quectel_low_consume_match,
    },
};

module_platform_driver(quectel_low_consume_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ql lpm");
MODULE_VERSION("1.0");
