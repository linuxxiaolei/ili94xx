#include <stdio.h>  
#include <stdlib.h>   
#include <fcntl.h>   
#include <sys/mman.h>  
#include <linux/fb.h>  
#include <unistd.h>  
int main(int argc, char* argv[])  
{  
    int fb;  
    int w, h;  
    int *fbmem;  
    struct fb_var_screeninfo fb_var;  
    int bits;  
    int i;  
      
    //打开设备文件fb0  
    fb = open("/dev/fb1", O_RDWR);  
    if(fb < 0)  
    {  
        printf("open fb1 error!/n");   
        exit(0);  
    }  
    //获取设备信息  
    ioctl(fb, FBIOGET_VSCREENINFO, &fb_var);  
    w = fb_var.xres;  
    h = fb_var.yres;  
    bits = fb_var.bits_per_pixel;  
    printf("Framebuffer:%d * %d/n", w, h);   
    printf("Bits:%d/n", bits);  
    //映射空间  
    fbmem = mmap(0, w*h*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);  
    *(fbmem + w * 300 + 400) = 0xffffff;  
    return 0;   
}  

