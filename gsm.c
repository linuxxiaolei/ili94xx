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

#include <asm/sizes.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/regs-mem.h>
#include <asm/mach/irq.h>


#define ILI94XX_PLATF_8BITONLY	(0x0001)
#define ILI94XX_PLATF_16BITONLY	(0x0002)
#define ILI94XX_PLATF_32BITONLY	(0x0004)

#define ILI94XX_MAJOR       156
#define ILI94XX_MINORS      32

#define res_size(_r) (((_r)->end - (_r)->start) + 1)


static DECLARE_BITMAP(minors, ILI94XX_MINORS);


struct board_info{
    //实际映射的IO
    void __iomem        *io_addr;
    void __iomem        *io_data;
    //获取deivce 资源
    struct resource     *addr_res;
    struct resource     *data_res;
    //用于申请内存
    struct resource	    *addr_req;   /* resources requested */
	struct resource     *data_req;
	
	struct resource     *irq_res;

	unsigned int	    flags;

    void (*inblk)(void __iomem *port, void *data, int length);
	void (*outblk)(void __iomem *port, void *data, int length);
	void (*dumpblk)(void __iomem *port, int length);
};

struct ili93xx_plat_data {
	unsigned int	flags;
	unsigned char	dev_addr[6];
	/* allow replacement IO routines */
	void	(*inblk)(void __iomem *reg, void *data, int len);
	void	(*outblk)(void __iomem *reg, void *data, int len);
	void	(*dumpblk)(void __iomem *reg, int len);
};




/*
 *   Read a byte from I/O port
 */
static u8 ior(struct board_info * db, int reg)
{
	writeb(reg, db->io_addr);
	return readb(db->io_data);
}
/*
 *   Write a byte to I/O port
 */
static void iow(struct board_info * db, int reg, int value)
{
	writeb(reg, db->io_addr);
	writeb(value, db->io_data);
}

static void ili94xx_write_cmd(struct board_info * db,int reg)
{
    writeb(reg, db->io_addr);
}

static void ili94xx_write_data(struct board_info * db,int data)
{
    writeb(data, db->io_data);
}

static u8 ili94xx_read_data(struct board_info * db)
{
   return readb(db->io_data);
}


static void ili94xx_outblk_8bit(void __iomem *reg, void *data, int count)
{
	writesb(reg, data, count);
}

static void ili94xx_outblk_16bit(void __iomem *reg, void *data, int count)
{
	writesw(reg, data, (count+1) >> 1);
}

static void ili94xx_inblk_8bit(void __iomem *reg, void *data, int count)
{
	readsb(reg, data, count);
}

static void ili94xx_inblk_16bit(void __iomem *reg, void *data, int count)
{
	readsw(reg, data, (count+1) >> 1);
}

static void  ili94xx_dumpblk_8bit(void __iomem *reg, int count)
{
	int i;
	int tmp;

	for (i = 0; i < count; i++)
		tmp = readb(reg);
}

static void ili94xx_dumpblk_16bit(void __iomem *reg, int count)
{
	int i;
	int tmp;

	count = (count + 1) >> 1;

	for (i = 0; i < count; i++)
		tmp = readw(reg);
}


static int ili94xx_read_id(struct board_info *db)
{
    u8 buf[4];
    writeb(0x04, db->io_addr);
    buf[0] = readb(db->io_data);
    buf[1] = readb(db->io_data);
    buf[2] = readb(db->io_data);
    buf[3] = readb(db->io_data);

    return (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

static void ili94xx_configure(struct board_info * db)
{
    ili94xx_write_cmd(db,0xE0); //P-Gamma
	ili94xx_write_data(db,0x00);
	ili94xx_write_data(db,0x13);
	ili94xx_write_data(db,0x18);
	ili94xx_write_data(db,0x04);
	ili94xx_write_data(db,0x0F);
	ili94xx_write_data(db,0x06);
	ili94xx_write_data(db,0x3A);
	ili94xx_write_data(db,0x56);
	ili94xx_write_data(db,0x4D);
	ili94xx_write_data(db,0x03);
	ili94xx_write_data(db,0x0A);
	ili94xx_write_data(db,0x06);
	ili94xx_write_data(db,0x30);
	ili94xx_write_data(db,0x3E);
	ili94xx_write_data(db,0x0F);

	ili94xx_write_cmd(db,0XE1); //N-Gamma
	ili94xx_write_data(db,0x00);
	ili94xx_write_data(db,0x13);
	ili94xx_write_data(db,0x18);
	ili94xx_write_data(db,0x01);
	ili94xx_write_data(db,0x11);
	ili94xx_write_data(db,0x06);
	ili94xx_write_data(db,0x38);
	ili94xx_write_data(db,0x34);
	ili94xx_write_data(db,0x4D);
	ili94xx_write_data(db,0x06);
	ili94xx_write_data(db,0x0D);
	ili94xx_write_data(db,0x0B);
	ili94xx_write_data(db,0x31);
	ili94xx_write_data(db,0x37);
	ili94xx_write_data(db,0x0F); 

	ili94xx_write_cmd(db,0XC0);   //Power Control 1
	ili94xx_write_data(db,0x18); //Vreg1out
	ili94xx_write_data(db,0x17); //Verg2out

	ili94xx_write_cmd(db,0xC1);   //Power Control 2
	ili94xx_write_data(db,0x41); //VGH,VGL

	ili94xx_write_cmd(db,0xC5);   //Power Control 3
	ili94xx_write_data(db,0x00);
	ili94xx_write_data(db,0x1A); //Vcom
	ili94xx_write_data(db,0x80);

	ili94xx_write_cmd(db,0x36);   //Memory Access
	ili94xx_write_data(db,0x48);   //48

	ili94xx_write_cmd(db,0x3A);   // Interface Pixel Format
	ili94xx_write_data(db,0x55); //16bit

	ili94xx_write_cmd( db,0XB0);   // Interface Mode Control
	ili94xx_write_data(db,0x00);

	ili94xx_write_cmd( db,0xB1);   //Frame rate
	ili94xx_write_data(db,0xA0); //60Hz

	ili94xx_write_cmd( db,0xB4);   //Display Inversion Control
	ili94xx_write_data(db,0x02); //2-dot

	ili94xx_write_cmd( db,0xB6);   //RGB/MCU Interface Control
	ili94xx_write_data(db,0x02); //MCU RGB
	ili94xx_write_data(db,0x22); //Source,Gate scan dieection

	ili94xx_write_cmd( db,0XE9);    // Set Image Function
	ili94xx_write_data(db,0x00);  //disable 24 bit data input

	ili94xx_write_cmd( db,0xF7);    // Adjust Control
	ili94xx_write_data(db,0xA9);
	ili94xx_write_data(db,0x51);
	ili94xx_write_data(db,0x2C);
	ili94xx_write_data(db,0x82);  // D7 stream, loose

	ili94xx_write_cmd( db,0x21);    //Normal Black
	ili94xx_write_cmd( db,0x11);    //Sleep out
	msleep(120);
	ili94xx_write_cmd( db,0x29);  
	ili94xx_write_cmd( db,0x2C);
}


static void ili94xx_set_io( struct board_info *db,int byte_width)
{
    switch(byte_width)
    {
        case 1:
            db->dumpblk = ili94xx_dumpblk_8bit;
            db->outblk  = ili94xx_outblk_8bit;
            db->inblk   = ili94xx_inblk_8bit;
            break;

        case 2:
            db->dumpblk = ili94xx_dumpblk_16bit;
            db->outblk  = ili94xx_outblk_16bit;
            db->inblk   = ili94xx_inblk_16bit;
        break;
        
    }
}


static int ili94xx_open(struct inode *inode,struct file *filp)
{
     int ret = 0;
     printk("open ili9433 is ok...\n");

     return ret;
}
static int ili94xx_release(struct inode *inode,struct file *filp)
{
     int ret = 0;
     printk("release ili9433 is ok...\n");

     return ret;
}

static const struct file_operations ili94xx_fops = {
    .owner = THIS_MODULE,
    .open = ili94xx_open,
    .release = ili94xx_release,
};


static void ili94xx_driver_init(struct platform_device *pdev)
{
    int ret;
    long minor;
    dev_t            devt;
    struct device    *dev;
    struct class     *ili94xx_class;
    ret = register_chrdev(ILI94XX_MAJOR,"ili94xx",&ili94xx_fops);
    if(ret < 0)
    {
        printk("====== %s register chrdev failed, status %d\n", __func__, ret);
        return ;
    }
    ili94xx_class = class_create(THIS_MODULE,"ili94xx");
    if(IS_ERR(ili94xx_class )){
        printk("====== %s class_create failed \n", __func__);
        unregister_chrdev(ILI94XX_MAJOR ,"ili94xx");
        return ;
    }
    minor = find_first_zero_bit(minors,ILI94XX_MINORS);
    devt = MKDEV(ILI94XX_MAJOR,minor);
    dev = device_create(ili94xx_class,&pdev->dev,devt,NULL,"ili94xx");

}

static int __devinit    ili94xx_probe(struct platform_device *pdev)
{
    struct ili93xx_plat_data    *pdata = pdev->dev.platform_data;
    struct board_info           *db;
    int ret = 0;
	int iosize;

    unsigned int oldval_bwscon = *(volatile unsigned int *)S3C2410_BWSCON;  //原来的寄存器是默认32bit传输
	unsigned int oldval_bankcon3 = *(volatile unsigned int *)S3C2410_BANKCON3;//使用的CS3故此出事bank3
    *((volatile unsigned int *)S3C2410_BWSCON) = (oldval_bwscon & ~(3<<16)) | S3C2410_BWSCON_DW3_16 | S3C2410_BWSCON_WS3 | S3C2410_BWSCON_ST3;
	*((volatile unsigned int *)S3C2410_BANKCON3) = 0x1f7c;
    db = (struct board_info *)kzalloc(sizeof(struct board_info),GFP_KERNEL);
	db->addr_res = platform_get_resource(pdev,IORESOURCE_MEM,0);
	db->data_res = platform_get_resource(pdev,IORESOURCE_MEM,1);
	db->irq_res  = platform_get_resource(pdev,IORESOURCE_IRQ,0);
    printk(KERN_INFO "test1111\n");
	iosize = res_size(db->addr_res);
	printk(KERN_INFO "test2222\n");
	db->addr_req = request_mem_region(db->addr_res->start,iosize,pdev->name);
	if(db->addr_req == NULL)
	{
        printk(KERN_INFO "cannot claim address reg area\n");
		ret = -EINVAL;
		goto out;
	}
    db->io_addr = ioremap(db->addr_res->start,iosize);
    if(db->io_addr == NULL)
    {
        printk(KERN_INFO  "failed to ioremap address reg\n");
		ret = -EINVAL;
		goto out;
    }

    iosize = res_size(db->data_res);
    db->data_req = request_mem_region(db->data_res->start,iosize,pdev->name);
    if(db->data_req == NULL)
    {
        printk(KERN_INFO  "cannot claim data reg area\n");
		ret = -EIO;
		goto out;
    }
    db->io_data = ioremap(db->data_res->start,iosize);
    if (db->io_data == NULL)
    {
		 printk(KERN_INFO  "failed to ioremap data reg\n");
		ret = -EINVAL;
		goto out;
	}

    if(pdata !=NULL)
    {
        if(pdata->flags == ILI94XX_PLATF_8BITONLY)
            ili94xx_set_io(db, 1);
        if(pdata->flags == ILI94XX_PLATF_16BITONLY)
            ili94xx_set_io(db, 2);    
        if (pdata->inblk != NULL)
			db->inblk = pdata->inblk;
		if (pdata->outblk != NULL)
			db->outblk = pdata->outblk;
		if (pdata->dumpblk != NULL)
			db->dumpblk = pdata->dumpblk;
		db->flags = pdata->flags;
    }

    platform_set_drvdata(pdev,db);

    printk("tft id:%02x\n", ili94xx_read_id(db));

    ili94xx_driver_init(pdev);
    
    out:
    *(volatile unsigned int *)S3C2410_BWSCON   = oldval_bwscon;
	*(volatile unsigned int *)S3C2410_BANKCON3 = oldval_bankcon3;

	return ret;
}


static int __devexit ili94xx_remove(struct platform_device *pdev)
{
    struct board_info   *db =  platform_get_drvdata(pdev);
    platform_set_drvdata(pdev, NULL);

	iounmap(db->io_addr);
	iounmap(db->io_data);

	release_resource(db->data_req);
	kfree(db->data_req);

	release_resource(db->addr_req);
	kfree(db->addr_req);

    kfree(db);
	
	return 0;
}

static struct platform_driver ili94xx_driver = {
	.driver	= {
		.name    = "ili9418",
		.owner	 = THIS_MODULE,
	},
	.probe   = ili94xx_probe,
	.remove  = __devexit_p(ili94xx_remove),
};
//CS使用CS3  ,RS使用ADDR2
static struct resource s3c_ili93xx_resource[] = 
{
    [0] = {
		.start	= S3C2410_CS3,
		.end	= S3C2410_CS3 + 3,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S3C2410_CS3 + 4,
		.end	= S3C2410_CS3 + 4 + 3,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT8,
		.end	= IRQ_EINT8,
		.flags	= IORESOURCE_IRQ ,
	}
};

static struct ili93xx_plat_data s3c_ili93xx_platdata = {
	.flags	= ILI94XX_PLATF_16BITONLY,
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
#endif
static struct platform_device  s3c_device_ili93xx = {
    .name       = "ili9418",
    .id         = 0,
    .dev        = {
        .platform_data = &s3c_ili93xx_platdata,
    },
};

static int __init ili94xx_init(void)
{
    int ret ;
    platform_device_add_resources(&s3c_device_ili93xx,&s3c_ili93xx_resource,3);
    platform_device_register(&s3c_device_ili93xx);
	ret =  platform_driver_register(&ili94xx_driver);
	return ret;
}

static void __exit ili94xx_cleanup(void)
{
	platform_driver_unregister(&ili94xx_driver);
    platform_device_unregister(&s3c_device_ili93xx);
}

module_init(ili94xx_init);
module_exit(ili94xx_cleanup);

MODULE_AUTHOR("wangxiaolei,wangwenhao");
MODULE_DESCRIPTION("ili94xx  driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ili94xx");



