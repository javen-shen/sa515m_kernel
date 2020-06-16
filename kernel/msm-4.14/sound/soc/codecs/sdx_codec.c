#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/tlv.h>
/*==========================================================================
  WHEN          WHO                          WHY
-----------  -----------  --------------------------------------------------
 2019/10/19   augus.xiong          support audio bring up
===========================================================================*/

struct codec_info{
	char* codec_name;
	char* dai_name;
};
struct codec_info *codec;
void* get_codec_info(void)
{

	if(!codec)
	{
		printk("%s : not found codec info \n",__func__);
	}
	return codec;
	
}
EXPORT_SYMBOL_GPL(get_codec_info);
void set_codec_info(char * codec_name ,char * dai_name)
{
	codec = kzalloc(sizeof(struct codec_info), GFP_KERNEL);
	codec->codec_name = codec_name;
	codec->dai_name = dai_name;

}
EXPORT_SYMBOL_GPL(set_codec_info);
static int __init codec_init(void)
{
	printk("enter into codec_init\n");	
	return 0;
}
static void __exit codec_exit(void)
{
	printk("codec exit \n");
}
module_init(codec_init);
module_exit(codec_exit);
MODULE_DESCRIPTION("get codec info for sdx55");
MODULE_AUTHOR("barry.chen@quectel.com");
MODULE_LICENSE("GPL");

