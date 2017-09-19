/* Modified by Matthew Watkins to used precomputed table
 * Basic example of using MCP 4822 DAC to generate sine wave
 * File:   main.c
 * Author: Syed Tahmid Mahbub
 *
 * Created on October 10, 2014
 */

/******************************************************************************
 * Software License Agreement
 *
 * Copyright � 2011 Microchip Technology Inc.  All rights reserved.
 * Microchip licenses to you the right to use, modify, copy and distribute
 * Software only when embedded on a Microchip microcontroller or digital
 * signal controller, which is integrated into your product or third party
 * product (pursuant to the sublicense terms in the accompanying license
 * agreement).
 *
 * You should refer to the license agreement accompanying this Software
 * for additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED ìAS ISî WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 * BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY
 * CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
 * OR OTHER SIMILAR COSTS.
 *
 *****************************************************************************/


#include "config.h"
#include "math.h"

#define SS      LATBbits.LATB13
#define dirSS   TRISBbits.TRISB13
#define anSS    ANSELBbits.ANSB13

#define TABLE_SIZE 32
int16_t sineTable[TABLE_SIZE] = {0,400,784,1138,1448,1703,1892,2009,2047,2009,1892,1703,1448,1138,784,400,0,-400,-784,-1138,-1448,-1703,-1892,-2009,-2048,-2009,-1892,-1703,-1448,-1138,-784,-400};
int16_t triangleTable[TABLE_SIZE];

#define F_CPU 40000000
#define F_dac 20000000
const short PR = F_CPU/F_dac - 1;

volatile uint8_t counter = 0;

void initDAC(void){
    anSS = 0;
    dirSS = 0;
    SS = 1;
   // RPB11R = 3; // SDO
    SpiChnOpen(2, SPI_OPEN_MSTEN | SPI_OPEN_MODE16 | SPI_OPEN_ON |
            SPI_OPEN_DISSDI | SPI_OPEN_CKE_REV , 2);
    PPSOutput(2, RPB5, SDO2);
    // Clock at 20MHz
}

inline void writeDAC(uint16_t data){
    SS = 0;
    while (TxBufFullSPI2());
    WriteSPI2(data);
    while (SPI2STATbits.SPIBUSY); // wait for it to end of transaction
    SS = 1;
}

void initTimers(void){
    OpenTimer1(T1_ON | T1_PS_1_1, PR);
    // Configure T1 for DAC update frequency
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
    
}

void __ISR(_TIMER_1_VECTOR, ipl2) T1Int(void){
    int16_t sine_val = 2048+sineTable[counter];
    writeDAC(0x3000 | sine_val); // write to channel A, gain = 1
    //writeDAC(0xB000 | triangleTable[counter]); // write to channel B, gain = 1
    counter++;
    if (counter == TABLE_SIZE) counter = 0;
    LATAINV = 1;
    mT1ClearIntFlag();
}

int main(void) {
    int i,j;
    
    SYSTEMConfigPerformance(40000000);
    
    CM1CON = 0; CM2CON = 0; ANSELA = 0; ANSELB = 0;
    initDAC();
    TRISACLR = 1;

    initTimers();
    INTEnableSystemMultiVectoredInt();
    
    while (1){
        
    }
}
