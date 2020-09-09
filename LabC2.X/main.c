/*
 * File:   main.c
 *
 * Serial Port & LCD & 7-segment template 
 * Also enables TIMER0 interrupt
 * 
 */

#include <xc.h>
#include "ee308.h"

volatile const unsigned char LCDcmd[2] = {0xC0,0x80};
volatile const unsigned char seven[16] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x58,0x5E,0x79,0x71};
unsigned char c;
short int count = 0;
unsigned char inputAvailable;
unsigned int counter2 = 0;
unsigned char serialRead;
int oneCount = 0;

void main(void) {
    
    
    initLCD();
    initSerial();
    initSeven();
    initTimer0();
    initTimer2();
    ADCON1 |= 0x0F;
    
    TRISB = 0x00;   // to show TIMER0 int
    TRISCbits.TRISC1 = 0;  // Buzzer
    LATBbits.LATB3 = 0;
    LATBbits.LATB0 = 1;
    LATCbits.LATC1 = 0;
    

    
    PORTD = seven[0];
            
    while(1){
        if(PIR1bits.RC1IF) { //I will be displaying the bit banging we need to do on the LCD as well, just for debugging purposes.
            PIR1bits.RC1IF = 0;
            c = RCREG;
            inputAvailable = 1;
            unsigned char cCopy = c;
            sendLCD(0x80, LCD_CMD);
            sendLCD(c,LCD_DATA);
            int i;
            sendLCD(0xC0, LCD_CMD);
            sendLCD('0', LCD_DATA); //Start bit
            sendLCD(' ', LCD_DATA);
            for (i = 0; i < 8; i++)
            {
                unsigned char lsb = cCopy&1;
                sendLCD(lsb+'0', LCD_DATA);
                if(lsb == 1)
                    count++;
                cCopy >>=1;
            }
            sendLCD(' ',LCD_DATA);
            if(count%2==0)
            {
                sendLCD('1', LCD_DATA);
            }
            else
            {
                sendLCD('0', LCD_DATA);
            }
            sendLCD(' ', LCD_DATA);
            sendLCD('1', LCD_DATA); //Stop bit
            count = 0;
        }
    }
    return;
}