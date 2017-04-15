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
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/backlight.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <asm/sizes.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/regs-mem.h>
#include <asm/mach/irq.h>
#include "ili9488.h"

#define ILI94XX_PLATF_8BITONLY	(0x0001)
#define ILI94XX_PLATF_16BITONLY	(0x0002)
#define ILI94XX_PLATF_32BITONLY	(0x0004)

#define ILI94XX_MAJOR       156
#define ILI94XX_MINORS      32

#define ILI94XX_IOC_MAGIC           'r'
#define ILI94XX_IOCSQSETCONF        _IOW(ILI94XX_IOC_MAGIC,1,int)
#define ILI94XX_IOCGQSETCONF        _IOR(ILI94XX_IOC_MAGIC,2,char)
#define ILI94XX_IOCSQWRITEBYTE      _IOW(ILI94XX_IOC_MAGIC,3,char)  
#define ILI94XX_IOCTQAUNTUM         _IO(ILI94XX_IOC_MAGIC,4)
#define ILI94XX_IOCXQREAD           _IOWR(ILI94XX_IOC_MAGIC,5,int)
#define ILI94XX_IOCFULLREAD         _IO(ILI94XX_IOC_MAGIC,6)
#define ILI94XX_IOCGFULLREAD        _IOR(ILI94XX_IOC_MAGIC,7,float)
#define ILI94XX_IOCXREADTEST        _IOWR(ILI94XX_IOC_MAGIC,8,int)
#define ILI94XX_MAXNR               8

//#define NEW_UPDATA_MOD
#define res_size(_r) (((_r)->end - (_r)->start) + 1)


static DECLARE_BITMAP(minors, ILI94XX_MINORS);
static LIST_HEAD(device_list);


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

};

struct ili9488_fd_reg
{
    u16                        height;
    u16                        width;
    u16                        dir;    //0竖屏1横屏
    u8                         setxcmd;        //设置X坐标
    u8                         setycmd;        //设置Y坐标
    u8                         wramcmd;    //写gram指令
    u16                        id;

};

struct ili9481_data{
     struct list_head           device_entry;
     struct device              *dev;
     struct platform_device     *pdev;
     struct board_info          *db;
     struct fb_info             *info;
     struct ili9488_fd_reg      fb;
     struct ili94xx_page        *pages;
     unsigned int               backlight;
     unsigned int               pages_count;
     unsigned long              pseudo_palette[25];
#ifdef NEW_UPDATA_MOD
     unsigned short             *tmpbuf;
     unsigned short             *tmpbuf_be;
#endif
     dev_t                      devt;
     
};

static void ili94xx_write_cmd(struct board_info * db,int reg)
{
    writeb(reg, db->io_addr);
}

static void ili94xx_write_data(struct board_info * db,int data)
{
    writeb(data, db->io_data);
}

static void ili94xx_write_ram(struct board_info * db,int data)
{
    writeb(data, db->io_data);
}


static u16 ili94xx_read_data(struct board_info * db)
{
   return readw(db->io_data);
}

static void ili94xx_write_reg(struct board_info * db, u8 reg, u16 data)
{
	writeb(reg, db->io_addr);
	writew(data, db->io_data);//写16位数据
}

static u16 ili94xx_read_reg(struct board_info * db, u8 reg)
{
    writeb(reg, db->io_addr);
    udelay(5);
    return readw(db->io_data);
}


static int ili94xx_read_id(struct ili9481_data *pdrv)
{
    u8 buf[4];
    ili94xx_write_cmd(pdrv->db,0x04);
    buf[0] = ili94xx_read_data(pdrv->db);
    buf[1] = ili94xx_read_data(pdrv->db);
    buf[2] = ili94xx_read_data(pdrv->db);
    buf[3] = ili94xx_read_data(pdrv->db);

    return (buf[1] << 16) + (buf[2] << 8) + buf[3];
}

static void ili94xx_configure(struct board_info * db)
{
#if 1
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
	ili94xx_write_data(db,0x40);   //48

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
#else
    
#endif
}

static void ili94xx_set_cursor(struct ili9481_data *pdrv,u16 xpos,u16 ypos)
{
    ili94xx_write_cmd(pdrv->db,pdrv->fb.setxcmd);
    ili94xx_write_data(pdrv->db,xpos>>8);
    ili94xx_write_data(pdrv->db,xpos&0xFF);
    ili94xx_write_cmd(pdrv->db,pdrv->fb.setycmd);
    ili94xx_write_data(pdrv->db,ypos>>8);
    ili94xx_write_data(pdrv->db,ypos&0xFF);
}

static void ili94xx_writeram_prepare(struct ili9481_data *pdrv)
{
    ili94xx_write_cmd(pdrv->db,pdrv->fb.wramcmd);
}



static void ili94xx_clear(struct ili9481_data *pdrv,u16 color)
{
    u32 index=0;      
	u32 totalpoint=480;
	totalpoint*=320; 
    printk("flag test:%d,x:%02x,y:%02x\n",pdrv->db->flags,pdrv->fb.setxcmd,pdrv->fb.setycmd);
	if(pdrv->fb.dir == 1)
	{
        pdrv->fb.setxcmd = 0x2A;
        pdrv->fb.setycmd = 0x2B;
        ili94xx_set_cursor(pdrv,0x00,0x0000);
        pdrv->fb.setxcmd = 0x2B;
        pdrv->fb.setycmd = 0x2A;
	}
	else
	    ili94xx_set_cursor(pdrv,0x00,0x0000);
	ili94xx_writeram_prepare(pdrv);
	
	for(index=0;index<totalpoint;index++)
	{
	        writew(color, pdrv->db->io_data);//写16位数据
	}
	
}

static void ili94xx_drawpoint(struct ili9481_data *pdrv, u16 x,u16 y,u16 color)
{
    writeb(0x2a, pdrv->db->io_addr);//写8位数据
    #if 1
    writeb(x>>8, pdrv->db->io_data);
    writeb(x&0xFF, pdrv->db->io_data);
    writeb(x>>8, pdrv->db->io_data);
    writeb(x&0xFF, pdrv->db->io_data);
    #else
    writew(x, pdrv->db->io_data);
    writew(x, pdrv->db->io_data);
    #endif
    
    writeb(0x2b, pdrv->db->io_addr);//写8位数据
    #if 1
    writeb(y>>8, pdrv->db->io_data);
    writeb(y&0xFF, pdrv->db->io_data);
    writeb(y>>8, pdrv->db->io_data);
    writeb(y&0xFF, pdrv->db->io_data);
    #else 
    writew(y, pdrv->db->io_data);
    writew(y, pdrv->db->io_data);
    #endif
    writeb(0x2c, pdrv->db->io_addr);//写8位数据
    writew(color, pdrv->db->io_data);
}

static void ili94xx_DrawLine(struct ili9481_data *pdrv,u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //?????? 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //?????? 
	else if(delta_x==0)incx=0;//??? 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//??? 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //????????? 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//???? 
	{  
		ili94xx_drawpoint(pdrv,uRow,uCol,color);//?? 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   


static int ili94xx_read_id_test(struct ili9481_data *pdrv)
{
    int id;
    msleep(50);
    ili94xx_write_reg(pdrv->db,0x00,0x0001);
    msleep(50);
    id = ili94xx_read_reg(pdrv->db,0x00);

    ili94xx_write_cmd(pdrv->db,0xD3);
    ili94xx_read_data(pdrv->db);
    ili94xx_read_data(pdrv->db);
    id = ili94xx_read_data(pdrv->db);
    id<<=8;
    id|=ili94xx_read_data(pdrv->db);
    printk("test id2:%02x\n",id );
}

static int ili94xx_open(struct inode *inode,struct file *filp)
{
     int i;
     int ret = 0;
     int id;
     struct ili9481_data *pdrv;
     int color[14] = {
        WHITE,BLACK,BLUE,BRED,GRED,GBLUE,RED,MAGENTA,GREEN,CYAN,YELLOW,BROWN,BRRED,GRAY
     };

     list_for_each_entry(pdrv, &device_list, device_entry){
              if(pdrv->devt == inode->i_rdev)
              {
                 break;
              }
     }
     
     id = ili94xx_read_id(pdrv);
     printk("open ili9488 is ok flag:%d,id:%d...\n",pdrv->db->flags,id);
     filp->private_data = pdrv;
     nonseekable_open(inode, filp);
     for(i=0;i<14;i++)
     {  
        ili94xx_clear(pdrv,color[i]);
        msleep(500);
     }

     ili94xx_DrawLine(pdrv,10,10,200,200,BLACK);
     ili94xx_DrawLine(pdrv,20,20,400,200,WHITE);
     
     return ret;
}

static int ili94xx_set_window(struct ili9481_data *item,u16 x0,u16 y0,u16 x1,u16 y1)
{
    ili94xx_write_cmd(item->db, item->fb.setxcmd); // Column addr set
	ili94xx_write_data(item->db, x0 >> 8);
	ili94xx_write_data(item->db, x0 & 0xFF); // XSTART
	ili94xx_write_data(item->db, x1 >> 8);
	ili94xx_write_data(item->db, x1 & 0xFF); // XEND
	
	ili94xx_write_cmd(item->db, item->fb.setycmd); // Row addr set
	ili94xx_write_data(item->db, y0>>8);
	ili94xx_write_data(item->db, y0); // YSTART
	ili94xx_write_data(item->db, y1>>8);
	ili94xx_write_data(item->db, y1); // YEND
	
	ili94xx_write_cmd(item->db, item->fb.wramcmd); // write to RAM
	return 0;
}

static void ili94xx_copy(struct ili9481_data *item,unsigned int index)
{
    int i;
    unsigned int ystart, yend;
	int chstart, chend;
	unsigned int x;
	unsigned int y;
	unsigned short *buffer, *oldbuffer;
	unsigned int len;
	x = item->pages[index].x;
	y = item->pages[index].y;
	len = item->pages[index].len;
	ystart = y;
	yend = y+(len/(480))+1;
	dev_dbg(item->dev,"%s: page[%u]: x=%3hu y=%3hu buffer=0x%p len=%3hu\n",
		__func__, index, x, y, buffer, len);

	//Move to start of line.
	buffer = item->pages[index].buffer-x;
	oldbuffer = item->pages[index].oldbuffer-x;
	x = 0;

	//If we arrive here, we basically assume something is written at the lines
	//starting with y and lasting the page.
	for (y = ystart; y < yend; y++) {
		//Find start and end of changed data
		chstart = -1;
		for (x = 0; x < 480; x++) {
			if (buffer[x] != oldbuffer[x]) {
				oldbuffer[x] = buffer[x];
				if (chstart == -1)
					chstart = x;
				chend=x;
			}
		}
		if (chstart!=-1) {
			/* Something changed this line! chstart and chend 
			 * contain start and end x-coords. */
			ili94xx_set_window(item, chstart, y, chend, y);
			for(i=0;i<(chend - chstart);i++)
			{
			    ili94xx_write_data(item->db,buffer[chstart+i]);
			}
		}
		buffer += 480;
		oldbuffer += 480;
	}
}

static void ili94xx_touch(struct fb_info *info, int x, int y, int w, int h)
{
    struct fb_deferred_io *fbdefio = info->fbdefio;
    struct ili9481_data *item = (struct ili9481_data *)info->par;
    int i,ystart,yend;
   
    if (fbdefio) { 
          for(i=0;i<item->pages_count;i++)
          {
            ystart = item->pages[i].y;
            yend=item->pages[i].y+(item->pages[i].len/info->fix.line_length)+1;
			if (!((y+h)<ystart || y>yend)) {
				item->pages[i].must_update=1;
			}
          }
          //Schedule the deferred IO to kick in after a delay.
          schedule_delayed_work(&info->deferred_work, fbdefio->delay);
    }
}

static ssize_t ili94xx_write(struct fb_info *p, const char __user *buf,
                                size_t count, loff_t *ppos)
{
        ssize_t res;
        res = fb_sys_write(p, buf, count, ppos);
        ili94xx_touch(p, 0, 0, p->var.xres, p->var.yres);
        return res;
}
static void ili94xx_update_all(struct ili9481_data *item)
{
	unsigned short i;
	struct fb_deferred_io *fbdefio = item->info->fbdefio;
	for (i = 0; i < item->pages_count; i++) {
		item->pages[i].must_update=1;
	}
	schedule_delayed_work(&item->info->deferred_work, fbdefio->delay);
}

static void ili94xx_update(struct fb_info *info, struct list_head *pagelist)
{
#ifdef NEW_UPDATA_MOD

    struct ili9481_data *item = (struct ili9481_data *)info->par;
    struct page *page;
    int i, j;

    //Copy all pages.
    for (i=0; i<item->pages_count; i++) {
            if(i == item->pages_count-1) {
                memcpy(item->tmpbuf, item->pages[i].buffer, PAGE_SIZE/2);
                for (j=0; j<PAGE_SIZE/2; j++) {
                    item->tmpbuf_be[j] = htons(item->tmpbuf[j]);
                    ili94xx_write_data(item->db,item->tmpbuf_be[j]);
                }

            }
            else {
                memcpy(item->tmpbuf, item->pages[i].buffer, PAGE_SIZE);
                for (j=0; j<PAGE_SIZE; j++) {
                    item->tmpbuf_be[j] = htons(item->tmpbuf[j]);
                    ili94xx_write_data(item->db,item->tmpbuf_be[j]);
                }
            }
    }
#else

    int i;
    struct ili9481_data *ili94xx = (struct ili9481_data *)info->par;
    struct page *page;
    printk("lcd updata..\n");
    list_for_each_entry(page, pagelist, lru) {
        ili94xx->pages[page->index].must_update=1;
    }
    //Copy changed pages.
    for(i = 0; i < ili94xx->pages_count; i++) {
        /*ToDo: Small race here between checking and setting must_update, * maybe lock? */
        if (ili94xx->pages[i].must_update) {
            ili94xx->pages[i].must_update=0;
            ili94xx_copy(ili94xx, i);
        }
    }

#endif
}

static const struct file_operations ili94xx_fops = {
    .owner = THIS_MODULE,
    .open = ili94xx_open,
};


static void ili94xx_fillrect(struct fb_info *p, const struct fb_fillrect *rect)
{
    sys_fillrect(p, rect);
	ili94xx_touch(p, rect->dx, rect->dy, rect->width, rect->height);
}

static void ili94xx_copyarea(struct fb_info *p, const struct fb_copyarea *area) 
{
	sys_copyarea(p, area);
	ili94xx_touch(p, area->dx, area->dy, area->width, area->height);
}

static void ili94xx_imageblit(struct fb_info *p, const struct fb_image *image) 
{
	sys_imageblit(p, image);
	ili94xx_touch(p, image->dx, image->dy, image->width, image->height);
}

static int ili94xx_blank(int blank_mode, struct fb_info *info)
{
	struct ili9481_data *ili = (struct ili9481_data *)info->par;

	if (blank_mode == FB_BLANK_UNBLANK)
		ili->backlight=1;
	else
		ili->backlight=0;
	/* Item->backlight won't take effect until the LCD is written to. Force that
	 * by dirty'ing a page. */
	ili->pages[0].must_update=1;
	schedule_delayed_work(&info->deferred_work, 0);

    return 0;
}
static inline __u32 CNVT_TOHW(__u32 val, __u32 width)
{
	return ((val<<width) + 0x7FFF - val)>>16;
}

static int ili94xx_setcolreg(unsigned regno,unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{
	int ret = 1;

	/*
	 * If greyscale is true, then we convert the RGB value
	 * to greyscale no matter what visual we are using.
	 */
	if (info->var.grayscale)
		red = green = blue = (19595 * red + 38470 * green +
				      7471 * blue) >> 16;
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		if (regno < 16) {
			u32 *pal = info->pseudo_palette;
			u32 value;

			red = CNVT_TOHW(red, info->var.red.length);
			green = CNVT_TOHW(green, info->var.green.length);
			blue = CNVT_TOHW(blue, info->var.blue.length);
			transp = CNVT_TOHW(transp, info->var.transp.length);

			value = (red << info->var.red.offset) |
				(green << info->var.green.offset) |
				(blue << info->var.blue.offset) |
				(transp << info->var.transp.offset);

			pal[regno] = value;
			ret = 0;
		}
		break;
	case FB_VISUAL_STATIC_PSEUDOCOLOR:
	case FB_VISUAL_PSEUDOCOLOR:
		break;
	}
	return ret;
}


static struct fb_ops ili94xx_fbops = {
        .owner        = THIS_MODULE,
        .fb_read      = fb_sys_read,
        .fb_write     = ili94xx_write,
        .fb_fillrect  = ili94xx_fillrect,   //填充一个矩形，要在自己的函数里面实现
        .fb_copyarea  = ili94xx_copyarea,   //进行区域复制
        .fb_imageblit = ili94xx_imageblit,  //在屏幕上画一个图片
        .fb_setcolreg   = ili94xx_setcolreg,//
        .fb_blank       = ili94xx_blank,
};


static struct fb_fix_screeninfo ili94xx_fix __initdata = {
        .id =  "ili9488",
        .type        = FB_TYPE_PACKED_PIXELS,
        .visual      = FB_VISUAL_TRUECOLOR,
        .accel       = FB_ACCEL_NONE,
        .line_length = 480 * 2,
};

static struct fb_var_screeninfo ili93xx_var __initdata = {  //显卡的一般特性
        .xres           = 480,      //可见分辨率
        .yres           = 320,
        .xres_virtual   = 480,      //虚拟分辨率
        .yres_virtual   = 320,
        .width          = 480,
        .height         = 320,
        .bits_per_pixel = 16,       //每像素位数
    	.red   			= {11, 5, 0},
    	.green 			= {5, 6, 0},
    	.blue 			= {0, 5, 0},
        .activate       = FB_ACTIVATE_NOW,
        .vmode          = FB_VMODE_NONINTERLACED,
};

static struct fb_deferred_io ili9341_defio = {
        .delay          = HZ / 35,
        .deferred_io    = &ili94xx_update,
};

static int __init ili94xx_video_alloc(struct ili9481_data *item)
{
    unsigned int frame_size;
    dev_dbg(item->dev, "%s: item=0x%p\n", __func__, (void *)item);
    frame_size= item->info->fix.line_length * item->info->var.yres;
    dev_dbg(item->dev, "%s: item=0x%p frame_size=%u\n",__func__, (void *)item, frame_size);
    item->pages_count = frame_size/PAGE_SIZE;
    if ((item->pages_count * PAGE_SIZE) < frame_size) {
		item->pages_count++;
	}
	
    dev_dbg(item->dev, "%s: item=0x%p pages_count=%u\n",__func__, (void *)item, item->pages_count);

    item->info->fix.smem_len = item->pages_count * PAGE_SIZE;
	item->info->fix.smem_start =(unsigned long)vmalloc(item->info->fix.smem_len);
	if (!item->info->fix.smem_start) {
		dev_err(item->dev, "%s: unable to vmalloc\n", __func__);
		return -ENOMEM;
	}
	memset((void *)item->info->fix.smem_start, 0, item->info->fix.smem_len);
	return 0;
}

static void ili94xx_video_free(struct ili9481_data *item)
{
    dev_dbg(item->dev, "%s: item=0x%p\n", __func__, (void *)item);
	kfree((void *)item->info->fix.smem_start);
}

static int __init ili94xx_pages_alloc(struct ili9481_data *item)
{
	unsigned short pixels_per_page;
	unsigned short yoffset_per_page;
	unsigned short xoffset_per_page;
	unsigned short index;
	unsigned short x = 0;
	unsigned short y = 0;
	unsigned short *buffer;
	unsigned short *oldbuffer;
	unsigned int len;

	dev_dbg(item->dev, "%s: item=0x%p\n", __func__, (void *)item);

	item->pages = kmalloc(item->pages_count * sizeof(struct ili94xx_page),
			      GFP_KERNEL);
	if (!item->pages) {
		dev_err(item->dev, "%s: unable to kmalloc for ili9341_page\n",__func__);
		return -ENOMEM;
	}

	pixels_per_page = PAGE_SIZE / (item->info->var.bits_per_pixel / 8);
	yoffset_per_page = pixels_per_page / item->info->var.xres;
	xoffset_per_page = pixels_per_page -(yoffset_per_page * item->info->var.xres);
	dev_info(item->dev, "%s: item=0x%p pixels_per_page=%hu ""yoffset_per_page=%hu xoffset_per_page=%hu\n",
		__func__, (void *)item, pixels_per_page,yoffset_per_page, xoffset_per_page);

	oldbuffer = kmalloc(item->pages_count * pixels_per_page * 2, GFP_KERNEL);
	if (!oldbuffer) {
		dev_err(item->dev, "%s: unable to kmalloc for ili9341_page oldbuffer\n",__func__);
		return -ENOMEM;
	}
	buffer = (unsigned short *)item->info->fix.smem_start;
	for (index = 0; index < item->pages_count; index++) {
		len = (item->info->var.xres * item->info->var.yres) -(index * pixels_per_page);
		if (len > pixels_per_page){
			len = pixels_per_page;
		}
		dev_info(item->dev,"%s: page[%d]: x=%3hu y=%3hu buffer=0x%p len=%3hu\n",__func__, index, x, y, buffer, len);
		item->pages[index].x = x;
		item->pages[index].y = y;
		item->pages[index].buffer = buffer;
		item->pages[index].oldbuffer = oldbuffer;
		item->pages[index].len = len;

		x += xoffset_per_page;
		if (x >= item->info->var.xres) {
			y++;
			x -= item->info->var.xres;
		}
		y += yoffset_per_page;
		buffer += pixels_per_page;
		oldbuffer += pixels_per_page;
	}

	return 0;

}
static void ili94xx_pages_free(struct ili9481_data *item)
{
        dev_dbg(item->dev, "%s: item=0x%p\n", __func__, (void *)item);
        kfree(item->pages);
}

static int ili94xx_register_fb(struct ili9481_data *pdrv,struct platform_device *pdev)
{
    int x,y;
   int ret;
   struct fb_info *info;
   dev_dbg(&pdev->dev, "Before registering framebuffer\n");
   info = framebuffer_alloc(sizeof(struct ili9481_data),&pdev->dev);
   info->pseudo_palette = &pdrv->pseudo_palette;
   pdrv->info = info;
   info->par = pdrv;
   info->fix = ili94xx_fix;
   info->var = ili93xx_var;
   info->fbops = &ili94xx_fbops;
   info->flags = FBINFO_FLAG_DEFAULT;
   ret = ili94xx_video_alloc(pdrv);
   if(ret){
        dev_err(&pdrv->dev,"%s: unable to ili9341_video_alloc\n", __func__);
		return -1;
   }
   info->screen_base = (char __iomem *)pdrv->info->fix.smem_start;
    
   ret = ili94xx_pages_alloc(pdrv);
   if (ret < 0) {
		dev_err(&pdrv->dev,"%s: unable to ili9341_pages_init\n", __func__);
		return -1;
	}
   info->fbdefio = &ili9341_defio;
   
   fb_deferred_io_init(info);
   
   ret = register_framebuffer(info);
   if (ret < 0){
       dev_err(&pdrv->dev,"%s: unable to register_frambuffer\n", __func__);
                        return -1;
   }
   ili94xx_read_id_test(pdrv);
   ili94xx_configure(pdrv->db);
   //test
   ili94xx_set_window(pdrv, 0, 0, 320, 480);
   for(y=480; y>0; y--) {
		for(x=320; x>0; x--) {
			ili94xx_write_data(pdrv->db, 0x55);
			ili94xx_write_data(pdrv->db, 0x55);
		}
	} 
   
   return ret;
}




static void ili94xx_driver_init(struct platform_device *pdev,struct ili9481_data *pdrv)
{
    int ret;
    long minor;
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
    pdrv->devt= MKDEV(ILI94XX_MAJOR,minor);
    dev = device_create(ili94xx_class,&pdev->dev,pdrv->devt,pdrv,"ili94xx");

}

static int request_platform_resource( struct board_info **db,struct platform_device *pdev, struct ili94xx_plat_data *pdata)
{
    int ret = 0;
	int iosize;
    (*db) = (struct board_info *)kzalloc(sizeof(struct board_info),GFP_KERNEL);    //重要
	(*db)->addr_res = platform_get_resource(pdev,IORESOURCE_MEM,0);
	(*db)->data_res = platform_get_resource(pdev,IORESOURCE_MEM,1);
	(*db)->irq_res  = platform_get_resource(pdev,IORESOURCE_IRQ,0);
	iosize = res_size((*db)->addr_res);
	(*db)->addr_req = request_mem_region((*db)->addr_res->start,iosize,pdev->name);
	if((*db)->addr_req == NULL)
	{
        printk(KERN_INFO "cannot claim address reg area\n");
		ret = -EINVAL;
		return ret;
	}
    (*db)->io_addr = ioremap((*db)->addr_res->start,iosize);
    if((*db)->io_addr == NULL)
    {
        printk(KERN_INFO  "failed to ioremap address reg\n");
		ret = -EINVAL;
		return ret;
    }

    iosize = res_size((*db)->data_res);
    (*db)->data_req = request_mem_region((*db)->data_res->start,iosize,pdev->name);
    if((*db)->data_req == NULL)
    {
        printk(KERN_INFO  "cannot claim data reg area\n");
		ret = -EIO;
		return ret;
    }
    (*db)->io_data = ioremap((*db)->data_res->start,iosize);
    if ((*db)->io_data == NULL)
    {
		 printk(KERN_INFO  "failed to ioremap data reg\n");
		ret = -EINVAL;
		return ret;
	}

	if(pdata !=NULL)
    {  
		(*db)->flags = pdata->displays->height;
    }

	return ret;
}

static int __devinit ili94xx_probe(struct platform_device *pdev)
{
    struct ili9481_data         *pdrv;
    struct ili94xx_plat_data    *pdata = pdev->dev.platform_data;
    struct board_info           *db;

    int ret = 0;
	int iosize;
	
    
    
    unsigned int oldval_bwscon = *(volatile unsigned int *)S3C2410_BWSCON;  //原来的寄存器是默认32bit传输
	unsigned int oldval_bankcon3 = *(volatile unsigned int *)S3C2410_BANKCON3;//使用的CS3故此出事bank3
    *((volatile unsigned int *)S3C2410_BWSCON) = (oldval_bwscon & ~(3<<16)) | S3C2410_BWSCON_DW3_16 | S3C2410_BWSCON_WS3 | S3C2410_BWSCON_ST3;
	*((volatile unsigned int *)S3C2410_BANKCON3) = 0x1f7c;

    pdrv = ( struct ili9481_data *)devm_kzalloc(&pdev->dev,sizeof( struct ili9481_data),GFP_KERNEL);//注意这里devm_kzalloc=kzalloc+memset
    
    pdrv->fb.setxcmd = 0x2A;
    pdrv->fb.setycmd = 0x2B;
    pdrv->fb.wramcmd = 0x2C;
    pdrv->dev = &pdev->dev;
    
    if(request_platform_resource(&db,pdev,pdata)!=0)
        goto out;
        
    pdrv->db = db;

    platform_set_drvdata(pdev,pdrv);
    
    INIT_LIST_HEAD(&pdrv->device_entry);
    list_add(&pdrv->device_entry, &device_list);

    #ifdef NEW_UPDATA_MOD
    pdrv->tmpbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!pdrv->tmpbuf) {
         ret = -ENOMEM;
         dev_err(&pdrv->dev, "%s: unable to allocate memory for tmpbuf\n", __func__);
         return -1;
    }
    pdrv->tmpbuf_be = kmalloc(PAGE_SIZE, GFP_DMA);
    if (!pdrv->tmpbuf_be) {
         ret = -ENOMEM;
         dev_err(&pdrv->dev, "%s: unable to allocate memory for tmpbuf_be\n", __func__);
         return -1;
    }
   #endif
   
    
    //ili94xx_driver_init(pdev,pdrv);
    ili94xx_register_fb(pdrv,pdev);
    //ili94xx_update_all(pdrv);
    //ili94xx_clear(pdrv,RED);

    out:
    *(volatile unsigned int *)S3C2410_BWSCON   = oldval_bwscon;
	*(volatile unsigned int *)S3C2410_BANKCON3 = oldval_bankcon3;

	return ret;
}


static int __devexit ili94xx_remove(struct platform_device *pdev)
{
    struct ili9481_data         *pdrv=  platform_get_drvdata(pdev);;
    struct board_info   *db  = pdrv->db;
    
    platform_set_drvdata(pdev, NULL);

	iounmap(db->io_addr);
	iounmap(db->io_data);

	release_resource(db->data_req);
	kfree(db->data_req);

	release_resource(db->addr_req);
	kfree(db->addr_req);

    kfree(db);

    ili94xx_video_free(pdrv);
    ili94xx_pages_free(pdrv);
	
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

static int __init ili94xx_init(void)
{
    int ret ;
	ret =  platform_driver_register(&ili94xx_driver);
	return ret;
}

static void __exit ili94xx_cleanup(void)
{
	platform_driver_unregister(&ili94xx_driver);
}

module_init(ili94xx_init);
module_exit(ili94xx_cleanup);

MODULE_AUTHOR("wangxiaolei,wangwenhao");
MODULE_DESCRIPTION("ili94xx  driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ili94xx");



