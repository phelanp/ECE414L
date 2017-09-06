/*
 * Update for 2.4" by Matthew Watkins
 * Author:      Bruce Land
 * Adapted from:
 *              main.c by
 * Author:      Syed Tahmid Mahbub
 * Target PIC:  PIC32MX250F128B
 */

// graphics libraries
#include "config.h"
#include "tft_master.h"
#include "tft_gfx.h"
// need for rand function
#include <stdlib.h>

// threading library
// config.h sets 40 MHz
#define	SYS_FREQ 40000000
#include "pt_cornell_1_2.h"

/* Demo code for interfacing TFT (ILI9341 controller) to PIC32
 * The library has been modified from a similar Adafruit library
 */
// Adafruit data:
/***************************************************
  This is an example sketch for the Adafruit 2.4" SPI display.
  This library works with the Adafruit 2.4" TFT Breakout w/SD card

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

// string buffer
char buffer[60];

// === thread structures ============================================
// thread control structs
// note that UART input and output are threads
static struct pt pt_led, pt_color;

// system 1 second interval tick
int sys_time_seconds ;

// === Timer Thread =================================================
// update a 1 second tick counter
static PT_THREAD (protothread_led(struct pt *pt))
{
    PT_BEGIN(pt);
     tft_setCursor(0, 0);
     tft_setTextColor(ILI9341_WHITE);  tft_setTextSize(1);
     tft_writeString("Welcome to our program!\n");
     mPORTBSetBits(BIT_5);
      while(1) {
        // yield time 1 second
        PT_YIELD_TIME_msec(500) ;
        mPORTBToggleBits(BIT_5);
//        sys_time_seconds++ ;
//        
//        // draw sys_time
//        tft_fillRoundRect(0,10, 100, 14, 1, ILI9341_BLACK);// x,y,w,h,radius,color
//        tft_setCursor(0, 10);
//        tft_setTextColor(ILI9341_YELLOW); tft_setTextSize(2);
//        sprintf(buffer,"%d", sys_time_seconds);
//        tft_writeString(buffer);
        // NEVER exit while
      } // END WHILE(1)
  PT_END(pt);
} // timer thread

// === Color Thread =================================================
// draw 3 color patches for R,G,B from a random number
static int color ;
static int red, blue, green, white, black;
static int i;
static PT_THREAD (protothread_color(struct pt *pt))
{
    PT_BEGIN(pt);
    int invert = 0;
    int pb_rst = 1;
    while(1) {
        // yield time 2 second
        PT_YIELD_TIME_msec(100) ;
        
        red = 0xf800;
        blue = 0x001f;
        green = 0x07e0;
        white = 0xffff;
        black = 0x0;
        color = 0x0;
        
        //get inputs
        int sw_red, sw_blue, sw_green, pb;
        if(mPORTBReadBits(BIT_7))   sw_red   = 1;
        else                        sw_red   = 0;
        if(mPORTBReadBits(BIT_8))   sw_blue  = 1;
        else                        sw_blue  = 0;
        if(mPORTBReadBits(BIT_9))   sw_green = 1;
        else                        sw_green = 0;
        if(mPORTBReadBits(BIT_10))  pb = 1;
        else                        pb = 0;
        
        //toggle logic
        if(pb && pb_rst) {
            invert = !invert;
            pb_rst = 0;
        } else if(!pb) {
            pb_rst = 1;
        }
        
        if(pb_rst) {
            tft_fillCircle(75,50,15, white);
        } else {
            tft_fillCircle(75,50,15, black);
        }
            
        if(sw_red ^ pb) {
            tft_fillCircle(20,85,15, red);
            color = color | red;
        } else {
            tft_fillCircle(20,85,15, black);
            tft_drawCircle(20,85,15, red);
        }
        if(sw_blue ^ pb) {
            tft_fillCircle(55,85,15, blue);
            color = color | blue;
        } else {
            tft_fillCircle(55,85,15, black);
            tft_drawCircle(55,85,15, blue);
        }
        if(sw_green ^ pb) {
            tft_fillCircle(90,85,15, green);
            color = color | green;
        } else {
            tft_fillCircle(90,85,15, black);
            tft_drawCircle(90,85,15, green);
        }
        if(color == 0x0) {
            tft_fillCircle(125,85,15, black);
            tft_drawCircle(125,85,15, white);
        } else {
            tft_fillCircle(125,85,15, color);
        }
        
        
//        // draw color string
//        tft_fillRoundRect(0,50, 150, 14, 1, ILI9341_BLACK);// x,y,w,h,radius,color
//        tft_setCursor(0, 50);
//        tft_setTextColor(ILI9341_WHITE); tft_setTextSize(1);
//        sprintf(buffer," %04x  %04x  %04x  %04x", color & 0x1f, color & 0x7e0, color & 0xf800, color);
//        tft_writeString(buffer);
//
//        // draw the actual color patches
//        tft_fillRoundRect(5,70, 30, 30, 1, color & 0x1f);// x,y,w,h,radius,blues
//        tft_fillRoundRect(40,70, 30, 30, 1, color & 0x7e0);// x,y,w,h,radius,greens
//        tft_fillRoundRect(75,70, 30, 30, 1, color & 0xf800);// x,y,w,h,radius,reds
//        // now draw the RGB mixed color
//        tft_fillRoundRect(110,70, 30, 30, 1, color);// x,y,w,h,radius,mix color
//        // NEVER exit while
        
        
      } // END WHILE(1)
  PT_END(pt);
} // color thread

//// === Animation Thread =============================================
//// move a ball across the screen
//static int xc=10, yc=150, vxc=2, vyc=0;
//static PT_THREAD (protothread_anim(struct pt *pt))
//{
//    PT_BEGIN(pt);
//      while(1) {
//        // yield time 32 msec
//        PT_YIELD_TIME_msec(32);
//
//        // erase disk
//         tft_fillCircle(xc, yc, 4, ILI9341_BLACK); //x, y, radius, color
//        // compute new position
//         xc = xc + vxc;
//         if (xc<5 || xc>315) vxc = -vxc;         
//         //  draw disk
//         tft_fillCircle(xc, yc, 4, ILI9341_GREEN); //x, y, radius, color
//        // NEVER exit while
//      } // END WHILE(1)
//  PT_END(pt);
//} // animation thread

// === Main  ======================================================
void main(void) {
 SYSTEMConfigPerformance(PBCLK);
  
  ANSELA = 0; ANSELB = 0; CM1CON = 0; CM2CON = 0;

  // config i/o
  mPORTBSetPinsDigitalIn(BIT_7 | BIT_8 | BIT_9 | BIT_10);
  mPORTBSetPinsDigitalOut(BIT_5);
  CNPDB = (1 << _CNPDB_CNPDB7_POSITION) | (1 << _CNPDB_CNPDB8_POSITION) | 
          (1 << _CNPDB_CNPDB9_POSITION) | (1 << _CNPDB_CNPDB10_POSITION);
  
  // === config threads ==========
  // turns OFF UART support and debugger pin
  PT_setup();

  // === setup system wide interrupts  ========
  INTEnableSystemMultiVectoredInt();

  // init the threads
  PT_INIT(&pt_led);
  PT_INIT(&pt_color);
//  PT_INIT(&pt_anim);

  // init the display
  tft_init_hw();
  tft_begin();
  tft_fillScreen(ILI9341_BLACK);
  //240x320 vertical display
  tft_setRotation(3); // Use tft_setRotation(1) for 320x240

  // seed random color
  srand(1);

  // round-robin scheduler for threads
  while (1){
      PT_SCHEDULE(protothread_led(&pt_led));
      PT_SCHEDULE(protothread_color(&pt_color));
//      PT_SCHEDULE(protothread_anim(&pt_anim));
      }
  } // main

// === end  ======================================================

