#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/console.h>
#include <asm/uaccess.h>

#include <asm/sizes.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/regs-mem.h>
#include <asm/mach/irq.h>

#include "ili9488.h"
//CS使用CS3  ,RS使用ADDR2
static struct resource s3c_ili93xx_resource[] = 
{
    [0] = {
		.start	= S3C2410_CS3,
		.end	= S3C2410_CS3 + 15,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S3C2410_CS3 + 16,
		.end	= S3C2410_CS3 + 16 + 3,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT8,
		.end	= IRQ_EINT8,
		.flags	= IORESOURCE_IRQ ,
	}
};


#if 0
static struct platform_device  s3c_device_ili93xx = {
    .name       = "ili9418",
    .id         = 0,
    .num_resources = ARRAY_SIZE(s3c_ili93xx_resource),
    .resource   = s3c_ili93xx_resource,
    .dev        = {
        .platform_data = &s3c_ili93xx_platdata,
    },
};
#else
static struct platform_device  s3c_device_ili93xx = {
    .name       = "ili9418",
    .id         = 0,
};
#endif
static struct ili9488_display   ili94xx_lcd_cfg[] = {
    [0] = {
        .width          =   320,
        .xres           =   320,
        .height         =   480,
        .yres           =   480,
        .left_margin    =   21,
        .right_margin   =   38,
       	.upper_margin	=   4,	
	    .lower_margin	=   4,	
	    .hsync_len	    =   6,	
	    .vsync_len	    =   2,	
        .bpp		    =   16,
	    //.type		    =   (S3C2410_LCDCON1_TFT16BPP|S3C2410_LCDCON1_TFT),
    },

    [1] = {
        .width          =   480,
        .xres           =   480,
        .height         =   800,
        .yres           =   800,
        .left_margin    =   40,
        .right_margin   =   40,
       	.upper_margin	=   29,	
	    .lower_margin	=   3,	
	    .hsync_len	    =   48,	
	    .vsync_len	    =   3,	
        .bpp		    =   16,
	    //.type		    =   (S3C2410_LCDCON1_TFT16BPP|S3C2410_LCDCON1_TFT),
    },
};

static struct ili94xx_plat_data ili94xx_fb_info={
       .displays        =   &ili94xx_lcd_cfg[0],
       .num_displays    =   12,
       .default_display =   0,
};

void *ili94xx_set_platdata(struct ili94xx_plat_data *pd,struct platform_device *pdev)
{
    void *npd;
    if(!pd){
        printk(KERN_ERR "%s: no platform data supplied\n", pdev->name);
        return NULL;
    }
    npd = kmemdup(pd,sizeof(struct ili94xx_plat_data),GFP_KERNEL);
    if(!npd){
       printk(KERN_ERR "%s: cannot clone platform data\n", pdev->name);
	   return NULL; 
    }
    pdev->dev.platform_data = npd;
}

static int __init ili94xx_dev_init(void)
{
    int ret = 0 ;
    ili94xx_set_platdata(&ili94xx_fb_info,&s3c_device_ili93xx);
    platform_device_add_resources(&s3c_device_ili93xx,&s3c_ili93xx_resource,3);
    platform_device_register(&s3c_device_ili93xx);
	return ret;
}

static void __exit ili94xx_dev_cleanup(void)
{
    platform_device_unregister(&s3c_device_ili93xx);
}

module_init(ili94xx_dev_init);
module_exit(ili94xx_dev_cleanup);
MODULE_AUTHOR("wangxiaolei,wangwenhao");
MODULE_DESCRIPTION("ili94xx  device");
MODULE_LICENSE("GPL");

