
//#resource "c64-sid.cfg"
#define CFGFILE c64-sid.cfg

const unsigned int VIC_BASE = 0xc000;

#include "sidmacros.h"
//#link "musicplayer.c"

#include "common.h"
//#link "common.c"

#include "multisprite.h"
//#link "multisprite.ca65"

#include "mcbitmap.h"
//#link "mcbitmap.c"

#include "rasterirq.h"
//#link "rasterirq.ca65"

//#incbin "holiday2023-c64.multi.bin"

// TODO: use #embed in later versions of 8bitworkshop
/*
const char const holiday2023_c64_multi_bin[] = {
  #embed "holiday2023-c64.multi.bin"
};
*/

#define NUM_TEST_SPRITES 20

#define SPRITE_SHAPE_BASE 60
#define NUM_SHAPES 3

/*{w:24,h:21,bpp:1,brev:1,count:3}*/
const char SPRITES[NUM_SHAPES][3*21] = {
  {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,
  0x00,0x54,0x00,0x00,0x38,0x00,0x04,0x10,0x40,
  0x08,0x10,0x20,0x16,0xBA,0xD0,0x01,0x11,0x00,
  0x02,0xD6,0x80,0x00,0x28,0x00,0x02,0xD6,0x80,
  0x01,0x11,0x00,0x16,0xBA,0xD0,0x08,0x10,0x20,
  0x04,0x10,0x40,0x00,0x38,0x00,0x00,0x54,0x00,
  0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  },{
  0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x38,0x00,
  0x00,0x7C,0x00,0x38,0x38,0x38,0x3C,0x10,0x78,
  0x38,0x7C,0x38,0x16,0xBA,0xD0,0x01,0xD7,0x00,
  0x03,0x93,0x80,0x01,0xFF,0x00,0x03,0x93,0x80,
  0x01,0xD5,0x00,0x16,0xBA,0xD0,0x38,0x7C,0x38,
  0x3C,0x10,0x78,0x38,0x38,0x38,0x00,0x7C,0x00,
  0x00,0x38,0x00,0x00,0x10,0x00,0x00,0x00,0x00
  },{
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,
  0x01,0xD7,0x00,0x02,0x38,0x80,0x0C,0x6C,0x60,
  0x3B,0x6D,0xB8,0x35,0xBB,0x58,0x27,0x11,0xC8,
  0x22,0xD6,0x88,0x20,0x28,0x08,0x22,0xD6,0x88,
  0x27,0x11,0xC8,0x35,0xBB,0x58,0x0B,0x6D,0xA0,
  0x04,0x6C,0x40,0x03,0x39,0x80,0x00,0xD6,0x00,
  0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  }
};

void sprite_shape(byte index, const char* sprite_data) {
  memmove((char*)VIC_BASE + index*64, sprite_data, 64);
}

void setup_sprites() {
  byte i;
  
  sprite_shape(SPRITE_SHAPE_BASE+0, SPRITES[0]);
  sprite_shape(SPRITE_SHAPE_BASE+1, SPRITES[1]);
  sprite_shape(SPRITE_SHAPE_BASE+2, SPRITES[2]);
  
  for (i=0; i<MAX_MSPRITES; i++) {
    msprite_order[i] = i;
    msprite_y[i] = 255;
  }
  
  for (i=0; i<NUM_TEST_SPRITES; i++) {
    int x = rand() % 280;
    msprite_x_lo[i] = x;
    msprite_x_hi[i] = x>>8;
    msprite_y[i] = i*12;
//    msprite_flags[i] = 0;
    msprite_shape[i] = SPRITE_SHAPE_BASE + (i % NUM_SHAPES);
    msprite_color[i] = 1;
  }
}

// constants for display list

#define Y0 21
#define Y1 35
#define YS 42

void display_list() {
  msprite_render_init();
  msprite_render_section();
  DLIST_NEXT(Y1+YS*1);
  msprite_render_section();
  DLIST_NEXT(Y1+YS*2);
  msprite_render_section();
  DLIST_NEXT(Y1+YS*3);
  msprite_render_section();
  DLIST_NEXT(Y1+YS*4);
  msprite_render_section();
//  VIC.bordercolor = 3;
  msprite_sort();
//  VIC.bordercolor = 4;
  msprite_add_velocity(NUM_TEST_SPRITES);
  VIC.bordercolor = 0;
  DLIST_RESTART(Y0);
}

void msprite_add_force(byte index, int dx, int dy) {
  int x,y;
  x = msprite_xvel_lo[index] | msprite_xvel_hi[index]*256;
  x += dx;
  if (x > 0) x--;
  else if (x < 0) x++;
  y = msprite_yvel_lo[index] | msprite_yvel_hi[index]*256;
  y += dy;
  if (y > 0) y--;
  else if (x < 0) y++;
  asm("sei");
  msprite_xvel_lo[index] = x;
  msprite_xvel_hi[index] = x >> 8;
  msprite_yvel_lo[index] = y;
  msprite_yvel_hi[index] = y >> 8;
  asm("cli");
}

char spridx = 0;
signed char wind_x = 0;
signed char wind_y = 0;
signed char gravity = 64;

void wind_gust() {
  if (wind_x == 0 && wind_y == 0) {
    wind_x = (rand() & 255) - 127;
    wind_y = -(rand() & 31);
  } else {
    if (wind_x > 0) wind_x--; else if (wind_x < 0) wind_x++;
    if (wind_y > 0) wind_y--; else if (wind_y < 0) wind_y++;
  }
}

void move_snow() {
  char i = spridx;
  if (msprite_yvel_hi[i] == 0) {
    msprite_add_force(i, wind_x, wind_y+gravity+(rand()&31));
  } else {
    msprite_add_force(i, wind_x, 0);
  }
  if (++spridx >= NUM_TEST_SPRITES) {
    spridx = 0;
  }
}

void show_background() {
  const char* uncomp = holiday2023_c64_multi_bin;
  char bgcolor;
  // setup VIC for multicolor bitmap
  // colormap = $c000-$c7ff
  // bitmap = $e000-$ffff
  setup_bitmap_multi();
  // read background color
  bgcolor = uncomp[10000];
  // copy data to destination areas
  memcpy((void*)MCB_BITMAP, uncomp, 8000);
  memcpy(COLOR_RAM, uncomp+9000, 1000);
  memcpy((void*)MCB_COLORS, uncomp+8000, 1000);
  // set background color
  VIC.bgcolor0 = bgcolor;
}


const byte music1[] = {
0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x90,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x88,0x39,0x39,0x2d,0x89,0x37,0x37,0x2d,0x88,0x35,0x35,0x89,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x2d,0x88,0x30,0x30,0x89,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x88,0x32,0x32,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x88,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x88,0x28,0x28,0x28,0x89,0x2a,0x2a,0x88,0x2c,0x2c,0x89,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x89,0x2f,0x2f,0x88,0x30,0x30,0x89,0x32,0x32,0x2a,0x88,0x34,0x34,0x89,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x90,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x90,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x90,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x89,0x39,0x39,0x2d,0x88,0x37,0x37,0x2d,0x89,0x35,0x35,0x88,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x2d,0x88,0x30,0x30,0x89,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x88,0x32,0x32,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x89,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x89,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x89,0x2a,0x2a,0x88,0x2c,0x2c,0x89,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x90,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x90,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x91,0x2f,0x2f,0x2d,0x88,0x30,0x30,0x2d,0x89,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x2d,0x90,0x2f,0x2f,0x2d,0x89,0x30,0x30,0x2d,0x88,0x2d,0x2d,0x2d,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x89,0x30,0x30,0x34,0x88,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x30,0x30,0x34,0x91,0x2f,0x2f,0x32,0x88,0x30,0x30,0x34,0x89,0x2d,0x2d,0x30,0x91,0x39,0x39,0x2d,0x91,0x39,0x39,0x2d,0x88,0x39,0x39,0x2d,0x89,0x37,0x37,0x2d,0x88,0x35,0x35,0x88,0x34,0x34,0x2d,0x91,0x34,0x34,0x2d,0x89,0x34,0x34,0x2d,0x88,0x32,0x32,0x2d,0x89,0x30,0x30,0x88,0x32,0x32,0x2d,0x91,0x32,0x32,0x2d,0x89,0x32,0x32,0x2d,0x88,0x34,0x34,0x2d,0x89,0x32,0x32,0x88,0x2d,0x2d,0x2d,0x91,0x2d,0x2d,0x2d,0x89,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x2d,0x88,0x2d,0x2d,0x89,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x89,0x2f,0x2f,0x88,0x30,0x30,0x89,0x32,0x32,0x2a,0x88,0x34,0x34,0x89,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x28,0x28,0x28,0x88,0x2a,0x2a,0x89,0x2c,0x2c,0x88,0x2d,0x2d,0x88,0x2f,0x2f,0x89,0x30,0x30,0x88,0x32,0x32,0x2a,0x89,0x34,0x34,0x88,0x32,0x32,0x2c,0x91,0x30,0x30,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x89,0x30,0x30,0x88,0x2d,0x2d,0x91,0x30,0x30,0x28,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x2b,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x30,0x30,0x29,0x91,0x2f,0x2f,0x88,0x30,0x30,0x89,0x2d,0x2d,0x91,0x2d,0x2d,0x28,0xb2,0x2d,0x2d,0x28,0xb3,0x2d,0x2d,0x28,0xb3,0x34,0x34,0x24,0x91,0x32,0x32,0x23,0x88,0x34,0x34,0x24,0x89,0x2d,0x2d,0x21,0xff
//0x0d,0x0d,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x90,0x19,0x19,0x91,0x27,0x2f,0x90,0x2a,0x19,0x89,0x2a,0x2f,0x90,0x2e,0x19,0x91,0x25,0x2a,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x0d,0x2a,0x91,0x27,0x2f,0x88,0x27,0x2a,0x88,0x0d,0x0d,0x89,0x2e,0x25,0x90,0x25,0x2a,0x89,0x0d,0x0d,0x88,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x91,0x0d,0x0d,0x90,0x27,0x19,0x91,0x27,0x2a,0x88,0x27,0x0d,0x91,0x18,0x27,0x90,0x2e,0x89,0x2e,0x88,0x2c,0x17,0x88,0x2c,0x91,0x16,0x25,0xa1,0x2a,0x24,0xa2,0x27,0x88,0x14,0x23,0x91,0x2a,0x88,0x27,0x14,0x88,0x2a,0x23,0x89,0x2e,0x90,0x29,0x19,0xa2,0x2f,0x27,0x90,0x2f,0x27,0x89,0x27,0x2a,0x90,0x19,0x25,0x91,0x2e,0x2a,0x90,0x19,0x19,0x89,0x0d,0x0d,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x0d,0x2f,0x90,0x27,0x2f,0x89,0x2f,0x2a,0x88,0x0d,0x0d,0x88,0x0d,0x2e,0x91,0x2e,0x2a,0x88,0x0d,0x0d,0x89,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x90,0x0d,0x0d,0x91,0x19,0x27,0x91,0x27,0x2a,0x88,0x2a,0x27,0x91,0x27,0x2e,0x90,0x2e,0x88,0x2e,0x89,0x17,0x26,0x88,0x2c,0x91,0x16,0x2a,0xa1,0x24,0x2a,0xa1,0x27,0x89,0x14,0x23,0x90,0x2a,0x89,0x14,0x27,0x88,0x19,0x23,0x88,0x2e,0x91,0x2c,0x29,0xa1,0x19,0x19,0x89,0x19,0x19,0x90,0x12,0x12,0x91,0x2b,0x0f,0x99,0x16,0x16,0x88,0x16,0x16,0x88,0x2e,0x91,0x33,0x88,0x21,0x21,0x89,0x33,0x90,0x27,0x2a,0x91,0x21,0x21,0x99,0x0f,0x2b,0x91,0x2e,0x88,0x16,0x16,0x88,0x33,0x16,0x91,0x35,0x88,0x33,0x89,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x2b,0x0f,0x91,0x2e,0x88,0x16,0x16,0x89,0x16,0x33,0x90,0x35,0x89,0x33,0x88,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x23,0x14,0x91,0x27,0x17,0x91,0x1b,0x2a,0x90,0x2e,0x1e,0x91,0x19,0x25,0x99,0x27,0x2a,0x91,0x27,0x2a,0x88,0x2f,0x27,0x88,0x19,0x19,0x89,0x2a,0x25,0x90,0x25,0x2e,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x90,0x27,0x0d,0x91,0x2f,0x27,0x88,0x2a,0x2f,0x88,0x0d,0x0d,0x89,0x0d,0x2e,0x90,0x25,0x2e,0x89,0x0d,0x0d,0x88,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x91,0x0d,0x0d,0x90,0x2f,0x0d,0x91,0x2f,0x27,0x88,0x2a,0x27,0x91,0x27,0x2e,0x91,0x2e,0x88,0x2e,0x88,0x26,0x17,0x89,0x2c,0x90,0x25,0x2a,0xa2,0x24,0x2a,0xb4,0x2e,0x27,0x8b,0x27,0x18,0x8b,0x17,0x2c,0x89,0x26,0x2c,0x90,0x2a,0x25,0xa2,0x15,0x24,0xa9,0x27,0x2e,0x91,0x27,0x2e,0x91,0x2c,0x17,0x88,0x26,0x17,0x91,0x25,0x2a,0xa1,0x15,0x2a,0xa1,0x27,0x88,0x23,0x14,0x91,0x2a,0x88,0x14,0x27,0x89,0x2a,0x19,0x88,0x2f,0x91,0x19,0x25,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x91,0x2a,0x23,0x90,0x20,0x0d,0x89,0x20,0x2a,0x88,0x20,0x2a,0x88,0x22,0x2c,0x91,0x25,0x27,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x88,0x22,0x88,0x19,0x19,0x91,0x2a,0x0d,0x90,0x2a,0x23,0x89,0x20,0x23,0x88,0x0d,0x0d,0x88,0x22,0x25,0x91,0x29,0x27,0x88,0x0d,0x0d,0x91,0x19,0x19,0x91,0x0d,0x0d,0x90,0x19,0x19,0x91,0x2a,0x20,0x91,0x0d,0x23,0x88,0x2a,0x20,0x88,0x23,0x27,0x89,0x2c,0x0d,0x90,0x19,0x19,0x89,0x0d,0x0d,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0f,0x2b,0x99,0x16,0x16,0x88,0x16,0x16,0x88,0x2e,0x91,0x33,0x88,0x21,0x21,0x89,0x33,0x90,0x27,0x2a,0x91,0x21,0x21,0x99,0x0f,0x2b,0x91,0x2e,0x88,0x16,0x16,0x88,0x16,0x33,0x91,0x35,0x88,0x33,0x89,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x0f,0x2b,0x91,0x2e,0x88,0x16,0x16,0x88,0x33,0x16,0x91,0x35,0x88,0x33,0x89,0x21,0x21,0x88,0x33,0x91,0x27,0x90,0x2a,0x99,0x23,0x14,0x91,0x27,0x17,0x91,0x2a,0x1b,0x90,0x2e,0x1e,0x91,0x25,0x31,0x99,0x2f,0x27,0x91,0x27,0x2a,0x88,0x2a,0x27,0x88,0x19,0x19,0x89,0x2e,0x25,0x90,0x2a,0x25,0x91,0x19,0x19,0x88,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x0d,0x27,0x91,0x27,0x2a,0x88,0x27,0x2f,0x88,0x0d,0x0d,0x89,0x0d,0x2e,0x90,0x25,0x2a,0x89,0x0d,0x0d,0x88,0x19,0x19,0x88,0x0d,0x0d,0x91,0x0d,0x19,0x91,0x0d,0x0d,0x90,0x19,0x0d,0x91,0x2f,0x27,0x88,0x27,0x2a,0x91,0x27,0x2e,0x91,0x2e,0x88,0x2e,0x88,0x26,0x17,0x88,0x2c,0x91,0x25,0x2a,0xa1,0x2a,0x24,0xb5,0x27,0x18,0x8b,0x27,0x18,0x8b,0x2c,0x26,0x89,0x26,0x17,0x90,0x16,0x25,0xa2,0x24,0x15,0xa9,0x2e,0x27,0x91,0x2e,0x27,0x91,0x26,0x17,0x88,0x17,0x26,0x90,0x2a,0x16,0xa2,0x15,0x24,0xa1,0x27,0x88,0x2a,0x23,0x91,0x2a,0x88,0x27,0x14,0x89,0x19,0x2a,0x88,0x2f,0x91,0x0d,0x27,0x90,0x19,0x19,0x91,0x0d,0x0d,0x91,0x19,0x19,0x90,0x0d,0x0d,0x91,0x19,0x19,0x91,0x27,0x20,0x90,0x20,0x23,0x89,0x27,0x19,0x88,0x0d,0x20,0x88,0x2a,0x25,0x91,0x19,0x19,0x88,0x31,0x0d,0x84,0x2d,0x84,0x2e,0x89,0x19,0x31,0x88,0x31,0x88,0x0d,0x2e,0x89,0x33,0x88,0x19,0x19,0x88,0x31,0x89,0x0d,0x0d,0x88,0x27,0x23,0x91,0x2a,0x27,0x88,0x19,0x19,0x88,0x2a,0x0d,0x89,0x19,0x19,0x90,0x2c,0x0d,0x84,0x2d,0x85,0x2e,0x88,0x31,0x19,0x88,0x31,0x89,0x0d,0x2e,0x88,0x31,0x88,0x19,0x19,0x89,0x2e,0x88,0x0d,0x0d,0x88,0x23,0x0d,0x89,0x2a,0x0d,0x90,0x2a,0x25,0x89,0x0d,0x0d,0x88,0x19,0x19,0x91,0x31,0x0d,0x84,0x2d,0x84,0x2e,0x88,0x31,0x19,0x88,0x31,0x89,0x0d,0x2e,0x88,0x2e,0x33,0x88,0x19,0x19,0x89,0x31,0x88,0x0d,0x0d,0x88,0x27,0x23,0x89,0x2a,0x27,0x90,0x19,0x2a,0xa2,0x31,0x88,0x0d,0x0d,0x88,0x19,0x19,0x89,0x31,0x88,0x0d,0x0d,0x88,0x0d,0x2c,0x82,0x2d,0x97,0x19,0x19,0x89,0x27,0x23,0x88,0x27,0x0d,0x91,0x2a,0x0d,0x99,0x2f,0x34,0x88,0x31,0x19,0x88,0x34,0x89,0x2f,0x0d,0x88,0x31,0x88,0x34,0x19,0x88,0x2f,0x89,0x0d,0x31,0x88,0x34,0x88,0x2f,0x19,0x81,0x31,0x90,0x2a,0x27,0x88,0x19,0x23,0x89,0x23,0x19,0x88,0x0d,0x25,0x99,0x34,0x19,0x88,0x31,0x89,0x34,0x0d,0x88,0x2f,0x88,0x19,0x31,0x89,0x34,0x88,0x2f,0x0d,0x88,0x33,0x91,0x2a,0x19,0x88,0x19,0x27,0x89,0x19,0x2a,0x90,0x22,0x0d,0x89,0x19,0x19,0x90,0x34,0x2f,0x89,0x31,0x88,0x34,0x19,0x88,0x2f,0x89,0x31,0x0d,0x88,0x34,0x88,0x2f,0x19,0x88,0x31,0x89,0x0d,0x0d,0x88,0x2a,0x27,0x88,0x27,0x2a,0x91,0x25,0x0d,0xa1,0x2f,0x34,0x89,0x31,0x88,0x34,0x19,0x88,0x2f,0x89,0x31,0x0d,0x88,0x34,0x0d,0x88,0x19,0x2f,0x89,0x31,0x88,0x0d,0x0d,0x88,0x27,0x23,0x89,0x23,0x2a,0x90,0x25,0x22,0x91,0x19,0x19,0x91,0x0d,0x0d,0x88,0x0d,0x27,0x88,0x19,0x19,0x89,0x23,0x2a,0x88,0x19,0x19,0x88,0x2a,0x22,0x89,0x19,0x19,0x90,0x0d,0x23,0x88,0x27,0x23,0x89,0x19,0x27,0x88,0x0d,0x0d,0x88,0x19,0x25,0x89,0x0d,0x0d,0x88,0x19,0x19,0x91,0x27,0x23,0x88,0x0d,0x2a,0x88,0x19,0x19,0x91,0x19,0x27,0x88,0x22,0x2a,0x89,0x19,0x19,0x90,0x23,0x2a,0x89,0x2a,0x27,0x88,0x23,0x19,0x88,0x0d,0x0d,0x89,0x22,0x19,0x88,0x0d,0x0d,0x88,0x19,0x19,0x91,0x23,0x0d,0x88,0x27,0x23,0x89,0x19,0x19,0x88,0x23,0x0d,0x88,0x19,0x19,0x89,0x22,0x25,0x88,0x19,0x19,0x91,0x2a,0x27,0x88,0x2a,0x27,0x88,0x2a,0x19,0x88,0x0d,0x0d,0x89,0x19,0x25,0x88,0x0d,0x0d,0x88,0x19,0x19,0x91,0x27,0x23,0x88,0x27,0x2a,0x89,0x19,0x19,0x88,0x0d,0x23,0x88,0x19,0x19,0x89,0x25,0x22,0x99,0x2a,0x23,0x90,0x0d,0x0d,0x99,0x2c,0x25,0xa1,0x2c,0x19,0xff
//0x33,0x99,0x33,0x24,0x8c,0x35,0x8c,0x33,0x24,0x98,0x30,0x24,0x97,0x2c,0x99,0x30,0x24,0x8c,0x31,0x8c,0x33,0x24,0x98,0x24,0x27,0x97,0x35,0x99,0x35,0x25,0x98,0x33,0x8c,0x31,0x8c,0x30,0x24,0x97,0x2e,0x8d,0x31,0x8c,0x33,0x25,0x98,0x33,0x25,0x98,0x25,0x27,0x97,0x33,0x8d,0x27,0x8c,0x33,0x2c,0x8c,0x35,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x30,0x2c,0x8b,0x27,0x8c,0x2c,0x8d,0x27,0x8c,0x30,0x2c,0x8c,0x2e,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x2c,0x8b,0x27,0x8c,0x35,0x8d,0x25,0x8c,0x31,0x29,0x8c,0x33,0x2c,0x8c,0x35,0x27,0x8c,0x33,0x2c,0x8c,0x30,0x27,0x8b,0x24,0x8c,0x2e,0x8d,0x25,0x8c,0x33,0x27,0x8c,0x2b,0x8c,0x2c,0x24,0x8c,0x30,0x86,0x2e,0x86,0x2c,0x8b,0x27,0x8c,0x31,0x29,0x99,0x31,0x29,0x98,0x35,0x29,0xaf,0x33,0x27,0x99,0x33,0x27,0x8c,0x33,0x8c,0x30,0x27,0xaf,0x2e,0x25,0x99,0x2e,0x22,0x98,0x31,0x22,0x94,0x27,0x83,0x2b,0x82,0x2e,0x83,0x31,0x93,0x30,0x27,0x99,0x33,0x27,0x8c,0x33,0x88,0x27,0x83,0x2c,0x81,0x2c,0x82,0x30,0x82,0x33,0xab,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x27,0x8c,0x33,0x27,0x8c,0x27,0x2c,0x8c,0x30,0x24,0x8b,0x24,0x2a,0x8c,0x31,0x8d,0x20,0x25,0x8c,0x33,0x20,0x8c,0x20,0x25,0x8c,0x35,0x25,0x8c,0x25,0x2b,0x8c,0x29,0x2c,0x8b,0x27,0x2e,0x8c,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x29,0x8c,0x33,0x27,0x8c,0x33,0x27,0x8c,0x30,0x27,0x8b,0x27,0x2c,0x8c,0x2e,0x8d,0x25,0x2b,0x8c,0x33,0x25,0x8c,0x25,0x2b,0x8c,0x2c,0x24,0xaf,0x33,0x99,0x33,0x24,0x8c,0x35,0x8c,0x33,0x24,0x98,0x30,0x24,0x97,0x2c,0x99,0x30,0x24,0x8c,0x31,0x8c,0x33,0x24,0x98,0x24,0x27,0x97,0x35,0x99,0x35,0x25,0x98,0x33,0x8c,0x31,0x8c,0x30,0x24,0x97,0x2e,0x8d,0x31,0x8c,0x33,0x25,0x98,0x33,0x25,0x98,0x25,0x27,0x97,0x33,0x8d,0x27,0x8c,0x33,0x2c,0x8c,0x35,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x30,0x2c,0x8b,0x27,0x8c,0x2c,0x8d,0x27,0x8c,0x30,0x2c,0x8c,0x2e,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x2c,0x8b,0x27,0x8c,0x35,0x8d,0x25,0x8c,0x31,0x29,0x8c,0x33,0x2c,0x8c,0x35,0x27,0x8c,0x33,0x2c,0x8c,0x30,0x27,0x8b,0x24,0x8c,0x2e,0x8d,0x25,0x8c,0x33,0x27,0x8c,0x2b,0x8c,0x2c,0x24,0x8c,0x30,0x86,0x2e,0x86,0x2c,0x8b,0x27,0x8c,0x31,0x29,0x99,0x31,0x29,0x98,0x35,0x29,0xaf,0x33,0x27,0x99,0x33,0x27,0x8c,0x33,0x8c,0x30,0x27,0xaf,0x2e,0x25,0x99,0x2e,0x22,0x98,0x31,0x22,0x94,0x27,0x83,0x2b,0x82,0x2e,0x83,0x31,0x93,0x30,0x27,0x99,0x33,0x27,0x8c,0x33,0x88,0x27,0x83,0x2c,0x81,0x2c,0x82,0x30,0x82,0x33,0xab,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x27,0x8c,0x33,0x27,0x8c,0x27,0x2c,0x8c,0x30,0x24,0x8b,0x24,0x2a,0x8c,0x31,0x8d,0x20,0x25,0x8c,0x33,0x20,0x8c,0x20,0x25,0x8c,0x35,0x25,0x8c,0x25,0x2b,0x8c,0x29,0x2c,0x8b,0x27,0x2e,0x8c,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x29,0x8c,0x33,0x27,0x8c,0x33,0x27,0x8c,0x30,0x27,0x8b,0x27,0x2c,0x8c,0x2e,0x8d,0x25,0x2b,0x8c,0x33,0x25,0x8c,0x25,0x2b,0x8c,0x2c,0x24,0xaf,0x33,0x99,0x33,0x24,0x8c,0x35,0x8c,0x33,0x24,0x98,0x30,0x24,0x97,0x2c,0x99,0x30,0x24,0x8c,0x31,0x8c,0x33,0x24,0x98,0x24,0x27,0x97,0x35,0x99,0x35,0x25,0x98,0x33,0x8c,0x31,0x8c,0x30,0x24,0x97,0x2e,0x8d,0x31,0x8c,0x33,0x25,0x98,0x33,0x25,0x98,0x25,0x27,0x97,0x33,0x8d,0x27,0x8c,0x33,0x2c,0x8c,0x35,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x30,0x2c,0x8b,0x27,0x8c,0x2c,0x8d,0x27,0x8c,0x30,0x2c,0x8c,0x2e,0x30,0x8c,0x33,0x2c,0x8c,0x30,0x8c,0x2c,0x8b,0x27,0x8c,0x35,0x8d,0x25,0x8c,0x31,0x29,0x8c,0x33,0x2c,0x8c,0x35,0x27,0x8c,0x33,0x2c,0x8c,0x30,0x27,0x8b,0x24,0x8c,0x2e,0x8d,0x25,0x8c,0x33,0x27,0x8c,0x2b,0x8c,0x2c,0x24,0x8c,0x30,0x86,0x2e,0x86,0x2c,0x8b,0x27,0x8c,0x31,0x29,0x99,0x31,0x29,0x98,0x35,0x29,0xaf,0x33,0x27,0x99,0x33,0x27,0x8c,0x33,0x8c,0x30,0x27,0xaf,0x2e,0x25,0x99,0x2e,0x22,0x98,0x31,0x22,0x94,0x27,0x83,0x2b,0x82,0x2e,0x83,0x31,0x93,0x30,0x27,0x99,0x33,0x27,0x8c,0x33,0x88,0x27,0x83,0x2c,0x81,0x2c,0x82,0x30,0x82,0x33,0xab,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x27,0x8c,0x33,0x27,0x8c,0x27,0x2c,0x8c,0x30,0x24,0x8b,0x24,0x2a,0x8c,0x31,0x8d,0x20,0x25,0x8c,0x33,0x20,0x8c,0x20,0x25,0x8c,0x35,0x25,0x8c,0x25,0x2b,0x8c,0x29,0x2c,0x8b,0x27,0x2e,0x8c,0x33,0x8d,0x27,0x2c,0x8c,0x33,0x27,0x8c,0x35,0x29,0x8c,0x33,0x27,0x8c,0x33,0x27,0x8c,0x30,0x27,0x8b,0x27,0x2c,0x8c,0x2e,0x8d,0x25,0x2b,0x8c,0x33,0x25,0x8c,0x25,0x2b,0x8c,0x2c,0x24,0xff
};

extern const byte* music_ptr;

void start_music(const byte* music);
void play_music();

#define SETUP_VOICE(v, pw) \
  SID_PULSEWIDTH(v, pw);\
  SID_ADSR(v, 10, 10, 2, 4);

void main() {
  
  clrscr();
  
  show_background();

  setup_sprites();
  
  VIC.spr_mcolor = 0x0;
  
  DLIST_SETUP(display_list);

  SID_INIT(15, 0b010);
  SID.flt_ctrl = 0x07;
  SID.flt_freq = 0x400;
  SETUP_VOICE(v1, 0x80);
  SETUP_VOICE(v2, 0x100);
  SETUP_VOICE(v3, 0x200);

  while (1) {
    raster_wait(200);
    move_snow();
    wind_gust();
    if (!music_ptr) start_music(music1);
    play_music();
  }
}
