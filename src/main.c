/*
 * File:   main.c
 * Author: nate
 *
 * Created on October 26, 2021, 7:29 PM
 */

// CONFIG
#pragma config FOSC = INTRCIO   // Oscillator Selection bits (INTOSC oscillator: I/O function on GP4/OSC2/CLKOUT pin, I/O function on GP5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-Up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // GP3/MCLR pin function select (GP3/MCLR pin function is MCLR)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config CP = OFF         // Code Protection bit (Program Memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)

#include <xc.h>

#define _XTAL_FREQ 32000

int time;

void checkVdd() {
    int result;
    //Configure ADCON0
    ADCON0bits.VCFG = 0;
    ADCON0bits.CHS = 1;
    ADCON0bits.ADON = 1;

    __delay_us(20); //Wait the acquisition time (about 20us)
    ADCON0bits.GO = 1; //Start the conversion

    while (ADCON0bits.GO == 1) {
        //wait for the conversion to end
    } 
    result = (ADRESH << 8) + ADRESL; //combine the 10 bits of the conversion
    if (result > 548) {
        GP2 = 1; //Turn on the LED if the battery voltage is below 2.8V
    } else {
        GP2 = 0; //Otherwise turn off the LED
    }
}

void checkTemp() {
    int result;
    //Configure ADCON0
    ADCON0bits.VCFG = 1;
    ADCON0bits.CHS = 0;
    ADCON0bits.ADON = 1;
    __delay_us(20); //Wait the acquisition time (about 20us)
    ADCON0bits.GO = 1; //Start the conversion
    while (ADCON0bits.GO == 1) {
        //wait for the conversion to end
    } 

    result = (ADRESH << 8) + ADRESL; //combine the 10 bits of the conversion

    if (result > 602) { //0F is approx 2.553V
        TMR1ON = 1; //Enable Timer
        time = getTime();
        if (time > 60) {
            GP4 = 1;//Turn on the LED if the battery voltage is below 2.8V
            GP5 = 1;//Turn on the buzzer if the temp goes above 
        }
    } else {
        time = 0;   //Reset time counter
        GP4 = 0;    //Turn off the LED
        GP5 = 0;    //Turn Off the buzzer
    }
}

void initPorts() {
    //GP0 & GP1 are inputs
    TRISIO = 0 | (1 << GP0) | (1 << GP1); // 0 - op, 1 - ip
    ANSEL = (1 << GP0) | (1 << GP1);
    ; // Ana. ip on GP0 GP1
}

void initTimer() {
    //Clear Timer Registers before enabling
    TMR1H = 0;
    TMR1L = 0;
    T1CON = 0;
}

int getTime() {
    return ((int) ((TMR1H << 8) + TMR1L) / _XTAL_FREQ);
}

void main(void) {
    int i;
    initPorts();
    initTimer();
    for (i = 0; i < 3; i++) {
        GP4 = 1;
        __delay_ms(500);
        GP4 = 0;
        __delay_ms(500);
    }
    while (1) {
        checkVdd();
        checkTemp();
    }
}