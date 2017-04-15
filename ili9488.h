#ifndef __ILI9488_H
#define __ILI9488_H
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


struct ili9488_display{ 
     unsigned int	            type;
     unsigned short             height;
     unsigned short             width;

    	/* Screen info */
	 unsigned short             xres;
	 unsigned short             yres;
	 unsigned short             bpp;
 
	 unsigned                   pixclock;		/* pixclock in picoseconds */
	 unsigned short             left_margin;  /* value in pixels (TFT) or HCLKs (STN) */
	 unsigned short             right_margin; /* value in pixels (TFT) or HCLKs (STN) */
	 unsigned short             hsync_len;    /* value in pixels (TFT) or HCLKs (STN) */
	 unsigned short             upper_margin;	/* value in lines (TFT) or 0 (STN) */
	 unsigned short             lower_margin;	/* value in lines (TFT) or 0 (STN) */
	 unsigned short             vsync_len;	/* value in lines (TFT) or 0 (STN) */
     

};

struct ili94xx_page {
        unsigned short x;
        unsigned short y;
        unsigned short *buffer;
        unsigned short *oldbuffer;
        unsigned short len;
        int must_update;
};


struct ili94xx_plat_data{
    struct ili9488_display     *displays;
	unsigned                    num_displays;			/* number of defined displays */
	unsigned                    default_display;
};

#endif
