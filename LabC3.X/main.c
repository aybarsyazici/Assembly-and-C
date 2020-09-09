/*
 * File:   main.c
 *
 * Template for a DS1307 
 * 
 */

#include <xc.h>
#include "ee308.h"
#include "i2c.h"

unsigned char  zz = 0;

unsigned char LCD_line1[16] = {"Time       Alarm"};
unsigned char LCD_line2[16] = {"00:00:00   07:30"};
unsigned char tempNOTACK[16] = "NOT ACK";
unsigned char tempACK[16] = "ACK";
unsigned char seconds_ones_digit;
unsigned char seconds_tens_digit;
unsigned char minutes_ones_digit;
unsigned char minutes_tens_digit;
unsigned char hours_ones_digit;
unsigned char hours_tens_digit;


//Please make sure to run the PicSIMLab at 40MHz Clock, otherwise if you 
//click the alarm change buttons really fast, because I change the LCD display a lot, the LCD display can get broken,
//It seems to be fine at 40MHz, but when I tried it at 16MHz clock sometimes, though rarely, I have broken the LCD display...


//RB3 increases the Alarm time by 5 minutes while RB4 decreases it by 5 minutes.

//RB5 increases the Alarm time by 1 hour while RA5 decreases it by an hour.

unsigned int alarmMinute = 25; //Change these values to change the default/start value of the alarm
unsigned int alarmHour = 18;

void main(void) {

    ADCON1 |= 0x0F;     // all DIGITAL
   
    TRISB &= 0xFF;     
    TRISD = 0x00;   // all output
    
    TRISCbits.TRISC1 = 0;   // buzzer
    LATCbits.LATC1 = 1;
    
    
    initLCD();
    refreshLCD(LCD_line1,0);
    refreshLCD(LCD_line2,1);
    sendLCD(0xC0,LCD_CMD); 
    
    i2c_init();
    /*Below I tried to modify the oscillator, but then decided not to bother with it, so ignore the below code.
    i2c_start();
    unsigned char ACK2 = i2c_wb(address_plus_write);
    unsigned char pointerToConfig = 0x07;
    ACK2 = i2c_wb(pointerToConfig);
    unsigned char configToBeApplied = 0x10; // 0001 0000 RS1 = 0 RS0 = 0, thus the oscillator is at 1Hz.
    ACK2 = i2c_wb(configToBeApplied);
    i2c_stop();
    */
    __delay_ms(1);
    i2c_start();
    
    unsigned char ACK = i2c_wb(address_plus_write); //slave address + write bit
        /*if(ACK == 1)
    {
        writeToLCD(tempNOTACK, 0, 0, 7);
    }
    else if(ACK==0)
    {
        writeToLCD(tempACK, 0, 0, 7);
    }*/
    unsigned char pointerValue = 0x00; //Set the pointer value to 0x00
    ACK = i2c_wb(pointerValue);
    /*if(ACK == 1)
    {
        writeToLCD(tempNOTACK, 0, 5, 7);
    }
    else if(ACK==0)
    {
        writeToLCD(tempACK, 0, 5, 3);
    }*/
    
    //We have written the pointer address...
    //Now onto the read operations...
    i2c_start(); //Restart 
    
    ACK = i2c_wb(address_plus_read); //Slave address + read operation
    /*if(ACK == 1)
    {
        writeToLCD(tempNOTACK, 1, 0, 7);
    }
    else if(ACK==0)
    {
        writeToLCD(tempACK, 1, 0, 7);
    }*/
    unsigned char seconds = i2c_rb(1);
    seconds_ones_digit = ((seconds & 0x0F) + '0');
    seconds_tens_digit = (((seconds & 0xF0)>>4)+'0');
    
    //Now lets get the minutes info.
    unsigned char minutes = i2c_rb(1);
    minutes_ones_digit = ((minutes & 0x0F) + '0');
    minutes_tens_digit = (((minutes & 0xF0)>>4)+'0');
    
    //And now the hours info.
    unsigned char hours = i2c_rb(0);
    hours_ones_digit = ((hours & 0x0F) + '0');
    hours_tens_digit = (((hours & 0xF0)>>4)+'0');
    
    writeTimeToLCD(seconds_ones_digit,seconds_tens_digit,minutes_ones_digit,minutes_tens_digit,hours_ones_digit,hours_tens_digit);
    i2c_stop();

    initTimer0();
    
    while(1) {

        if(PORTBbits.RB3 == 0) //If RB3 is pressed, increment the Alarm clock by 5mins.
        {
            LATCbits.LATC1 ^= 1;
            int carry = (alarmMinute + 5)/60; //See if we get a carry when we ad 5 to minute variable.
            alarmHour = (alarmHour+carry)%24;   //alarmHour will increase 1 if we have a carry.
            alarmMinute = (alarmMinute+5)%60;   //increase alarmMinute by 5
            writeSingleToLCD(alarmHour/10+'0',1,11);    //Display it on the LCD
            writeSingleToLCD(alarmHour%10+'0',1,12);
            writeSingleToLCD(':',1,13);
            writeSingleToLCD(alarmMinute/10+'0',1,14);
            writeSingleToLCD(alarmMinute%10+'0',1,15);
            while(PORTBbits.RB3 == 0) {} //Wait release.
        }
        if(PORTBbits.RB4 == 0) //If RB4 is preseed decrement the alarm clock by 5mins.
        {
            if(alarmMinute < 5){
                if(alarmHour > 0)
                    alarmHour -= 1;
                else
                    alarmHour = 23;
                alarmMinute += 55;
            }
            else
            {
                alarmMinute -= 5;
            }
            writeSingleToLCD(alarmHour/10+'0',1,11);
            writeSingleToLCD(alarmHour%10+'0',1,12);
            writeSingleToLCD(':',1,13);
            writeSingleToLCD(alarmMinute/10+'0',1,14);
            writeSingleToLCD(alarmMinute%10+'0',1,15);    
            while(PORTBbits.RB4 == 0) {} //Wait release.
        }
        if(PORTBbits.RB5 == 0) //If RB5 is pressed increment the alarm clock by 1 hour.
        {
            alarmHour = (alarmHour+1)%24;
            writeSingleToLCD(alarmHour/10+'0',1,11);
            writeSingleToLCD(alarmHour%10+'0',1,12);
            while(PORTBbits.RB5 == 0) {} //Wait release.
        }
        if(PORTAbits.RA5 == 0) //If RA5 is pressed decrement the alarm clock by 1 hour.
        {
            if(alarmHour > 0)
                alarmHour -= 1;
            else
                alarmHour = 23;
            writeSingleToLCD(alarmHour/10+'0',1,11);
            writeSingleToLCD(alarmHour%10+'0',1,12);
            while(PORTAbits.RA5 == 0) {} //Wait release.
        }
    }
    return;
    
}