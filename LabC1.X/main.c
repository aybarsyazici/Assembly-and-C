/*
 * File:   main.c
 *
 * TIMER0 interrupt for PWM generation
 * 
 */

#include <xc.h>
#include "ee308.h"

unsigned char duty1;
unsigned char duty2;
int oldtenbitDuty1 = 0;
int oldtenbitDuty2 = 0;
void main(void) {

    ADCON1 &= 0x0D;     // 1101, AN0 and AN1 Analog.
   
    initTimer0();
    initLCD();
    
    TRISD &= 0xF0;

    while(1) {
        
        ADCON0 = 0x01; // 0000 0001; Turn the ADC on, and set select bits to 0000, thus select AN0
        ADCON0bits.GO = 1; //Start the conversion
        while(ADCON0bits.GO != 0) {
            //Do nothing and wait for the conversion to finish 
        }
        duty1 = ADRESH; // We now have the upper 8 bits of the 10bit output.
        int tenbitDuty1 = duty1 << 2;
        ADRESL >>= 6;
        unsigned char lowerbits = ADRESL&(0b11);
        tenbitDuty1 |= lowerbits; //Now we have the whole 10bits of the output.
        ADCON0 = 0x5; // 0000 00101; Turn the ADC on, set the select bits to 0001, thus select AN1
        ADCON0bits.GO = 1; //Start the conversion
        while(ADCON0bits.GO != 0) {
            //Do nothing and wait for the conversion to finish 
        }
        duty2 = ADRESH;
        int tenbitDuty2 = duty2 << 2;
        ADRESL >>=6;
        lowerbits = ADRESL&(0b11);
        tenbitDuty2 |= lowerbits;
        if((tenbitDuty1 == oldtenbitDuty1) && (tenbitDuty2 == oldtenbitDuty2))
        {
            //Do nothing.
        }
        else if((tenbitDuty1 != oldtenbitDuty1) || (tenbitDuty2 != oldtenbitDuty2)) //One of the values of the potentiometer has changed.
        {
            displayOnLowerLCD(tenbitDuty1, tenbitDuty2); //Redisplay the new PWM value.
            oldtenbitDuty1 = tenbitDuty1;   //Save it
            oldtenbitDuty2 = tenbitDuty2;
            changeDuty(tenbitDuty1, tenbitDuty2); //Change the duty cycle of the LED.
        }
    }
    return;
}