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

#define TABLE_SIZE 256
int16_t sineTable[TABLE_SIZE] = {0,50,100,151,201,251,300,350,399,449,497,546,594,642,690,737,783,830,875,920,965,1009,1052,1095,1137,1179,1219,1259,1299,1337,1375,1411,1447,1483,1517,1550,1582,1614,1644,1674,1702,1729,1756,1781,1805,1828,1850,1871,1891,1910,1927,1944,1959,1973,1986,1997,2008,2017,2025,2032,2037,2041,2045,2046,2047,2046,2045,2041,2037,2032,2025,2017,2008,1997,1986,1973,1959,1944,1927,1910,1891,1871,1850,1828,1805,1781,1756,1729,1702,1674,1644,1614,1582,1550,1517,1483,1447,1411,1375,1337,1299,1259,1219,1179,1137,1095,1052,1009,965,920,875,830,783,737,690,642,594,546,497,449,399,350,300,251,201,151,100,50,0,-50,-100,-151,-201,-251,-300,-350,-399,-449,-497,-546,-594,-642,-690,-737,-783,-830,-875,-920,-965,-1009,-1052,-1095,-1137,-1179,-1219,-1259,-1299,-1337,-1375,-1411,-1447,-1483,-1517,-1550,-1582,-1614,-1644,-1674,-1702,-1729,-1756,-1781,-1805,-1828,-1850,-1871,-1891,-1910,-1927,-1944,-1959,-1973,-1986,-1997,-2008,-2017,-2025,-2032,-2037,-2041,-2045,-2046,-2047,-2046,-2045,-2041,-2037,-2032,-2025,-2017,-2008,-1997,-1986,-1973,-1959,-1944,-1927,-1910,-1891,-1871,-1850,-1828,-1805,-1781,-1756,-1729,-1702,-1674,-1644,-1614,-1582,-1550,-1517,-1483,-1447,-1411,-1375,-1337,-1299,-1259,-1219,-1179,-1137,-1095,-1052,-1009,-965,-920,-875,-830,-783,-737,-690,-642,-594,-546,-497,-449,-399,-350,-300,-251,-201,-151,-100,-50};
int16_t triangleTable[TABLE_SIZE];

#define F_CPU 40000000
#define F_dac 550000
const short PR = F_CPU/F_dac - 1;

volatile uint16_t counter = 0;

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
    OpenTimer2(T2_ON | T2_PS_1_1, PR);
    // Configure T2 for DAC update frequency
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
    
}

uint16_t note_incr = 0x100;
uint16_t table_index;
void __ISR(_TIMER_2_VECTOR, ipl2) T2Int(void){
    table_index = counter;
    table_index = table_index>>8;
    int16_t sine_val = 2048+sineTable[table_index];
    writeDAC(0x3000 | sine_val); // write to channel A, gain = 1
    //writeDAC(0xB000 | triangleTable[counter]); // write to channel B, gain = 1
    counter += note_incr;
    if (table_index == TABLE_SIZE) counter = 0;
    LATAINV = 1;
    mT2ClearIntFlag();
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
