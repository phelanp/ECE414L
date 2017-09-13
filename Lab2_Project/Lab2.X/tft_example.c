/*
 * Update for 2.4" by Matthew Watkins
 * Update for Lab 1 - Peter Phelan & Nakul Talwar
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

// string buffer
char buffer[60];

// === thread structures ============================================
// thread control structs
// note that UART input and output are threads
static struct pt pt_led, pt_measure, pt_comp;

// === LED Thread ===================================================
// update every 0.5 seconds to draw text and blink LED
int blink = 0;
static PT_THREAD (protothread_led(struct pt *pt)) {
    PT_BEGIN(pt);
//     tft_setTextColor(ILI9341_WHITE);  tft_setTextSize(1);
//     tft_writeString("Welcome to our program!\n");
      while(1) {
        // yield time 0.5 second
        PT_YIELD_TIME_msec(500) ;
        if(blink) tft_fillCircle(20,20,15, 0xffff);
        else tft_fillCircle(20,20,15, 0);
        blink = !blink;
      } // END WHILE(1)
  PT_END(pt);
} // timer thread

// === Color Thread =================================================
// draw 4 color patches based on the switch input
int time;
static PT_THREAD (protothread_measure(struct pt *pt)) {
    PT_BEGIN(pt);
    while(1) {
        time = 0;                
        //drain capacitor
        mPORTBSetPinsDigitalOut(BIT_3);
        PT_YIELD_TIME_msec(1);
        
        //charge capacitor
        WriteTimer23(0);
        mPORTBSetPinsAnalogIn(BIT_3);
        CMP1Open(CMP_ENABLE);
        OpenCapture1(IC_ON | IC_TIMER2_SRC);
        
        //wait for charge
        PT_YIELD_TIME_msec(200);
        
        //compute capacitance
        
        
      } // END WHILE(1)
  PT_END(pt);
} // color thread

// === Comparator Polling ==========================================
// poll comparator input
static PT_THREAD (protothread_comp(struct pt *pt)) {
    PT_BEGIN(pt);
    while(1) {
        PT_YIELD_TIME_msec(5);
        if(CMP1Read()) mPORTBSetBits(BIT_5);
        else mPORTBClearBits(BIT_5);
      } // END WHILE(1)
  PT_END(pt);
} // color thread

//interrupt behavior
void __ISR(_INPUT_CAPTURE_1_VECTOR, ipl2soft) Charged(void) {
    time = mIC1ReadCapture();
    tft_setCursor(0, 100);
    tft_setTextColor(ILI9341_WHITE);  tft_setTextSize(1);
    tft_writeString("Welcome to our program!\n");
    INTClearFlag(INT_IC1);
}

// === Main  ======================================================
void main(void) {
 SYSTEMConfigPerformance(PBCLK);
  
  ANSELA = 0; ANSELB = 0; CM1CON = 0; CM2CON = 0;

  // config i/o
  mPORTBSetPinsDigitalIn(BIT_13);
  mPORTBSetPinsDigitalOut(BIT_5);
  mPORTBSetPinsAnalogIn(BIT_3);
  CNPDB = (1 << _CNPDB_CNPDB13_POSITION);
  
  //configure timer
  OpenTimer23(T2_ON | T2_PS_1_1, 0xffff);
  
  //configure comparators
  CMP1Open(CMP_ENABLE | CMP_POS_INPUT_C1IN_POS | CMP1_NEG_INPUT_IVREF);
  
  //configure input capture
  mIC1ClearIntFlag();
  OpenCapture1(IC_EVERY_RISE_EDGE | IC_INT_1CAPTURE | IC_FEDGE_RISE | IC_ON);
  
  //configure interrupts
  INTEnableSystemMultiVectoredInt();
  INTEnable(INT_IC1, INT_ENABLED);
  INTSetVectorPriority(INT_INPUT_CAPTURE_1_VECTOR, INT_PRIORITY_LEVEL_2);
  
  ConfigIntCapture1(IC_INT_OFF | IC_INT_PRIOR_1);
  IC1R = 0x3;
  
  // === config threads ==========
  // turns OFF UART support and debugger pin
  PT_setup();

  // init the threads
  PT_INIT(&pt_led);
  PT_INIT(&pt_comp);
  PT_INIT(&pt_measure);

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
      PT_SCHEDULE(protothread_comp(&pt_comp));
      PT_SCHEDULE(protothread_measure(&pt_measure));
      }
  } // main

// === end  ======================================================

