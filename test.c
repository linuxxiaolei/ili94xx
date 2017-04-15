#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <math.h>


#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //б┴?иж?
#define BRRED 			 0XFC07 //б┴?oимиж?
#define GRAY  			 0X8430 //?и░иж?



int color[14] = {
    WHITE,BLACK,BLUE,BRED,GRED,GBLUE,RED,MAGENTA,GREEN,CYAN,YELLOW,BROWN,BRRED,GRAY
};

int main(int argc,char **argv)
{
    int fd;
    struct fb_var_screeninfo fb_var;
    if(atoi(argv[1]) == 1)
    {
        fd = open("/dev/fb1",O_RDWR);
    }
    else
    {
        fd = open("/dev/fb0",O_RDWR);
    }
    if(fd < 0)
        printf("open failed\n");

    ioctl(fd,FBIOGET_VSCREENINFO,&fb_var);

    printf("framebuffer:w=%d,h=%d,%dbpp\n",fb_var.yres,fb_var.xres,fb_var.bits_per_pixel);

    printf("framebuffer rgb:red:%d-%d-%d,green:%d-%d-%d,blue:%d-%d-%d\n",
    fb_var.red.offset ,
	fb_var.red.length,
	fb_var.red.msb_right,
	fb_var.green.offset ,
	fb_var.green.length ,
	fb_var.green.msb_right,
	fb_var.blue.offset ,
	fb_var.blue.length,
    fb_var.blue.msb_right);
    while(1)
    { 
        
    }
}

