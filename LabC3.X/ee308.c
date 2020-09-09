#include <xc.h>
#include <pic18f4620.h>
#include "ee308.h"
#include "i2c.h"
unsigned char seconds_ones_digit;
unsigned char seconds_tens_digit;
unsigned char minutes_ones_digit;
unsigned char minutes_tens_digit;
unsigned char hours_ones_digit;
unsigned char hours_tens_digit;

unsigned char old_seconds_ones_digit;
unsigned char old_seconds_tens_digit;
unsigned char old_minutes_ones_digit;
unsigned char old_minutes_tens_digit;
unsigned char old_hours_ones_digit;
unsigned char old_hours_tens_digit;

unsigned char LCD_line1[16] = {"Time       Alarm"};
unsigned char LCD_line2[16] = {"00:00:00   07:30"};

unsigned int alarmMinute = 30;
unsigned int alarmHour = 7;

int counter = 0;

void initLCD(void)
{
    TRISEbits.TRISE1 = 0;   // EN
    TRISEbits.TRISE2 = 0;   // RS

    sendLCD(0x28,LCD_CMD);
    __delay_ms(5);
    sendLCD(0x28,LCD_CMD);
    __delay_ms(1);
    sendLCD(0x28,LCD_CMD);
    __delay_ms(1);
    
	sendLCD(0x28,LCD_CMD);			// init LCD
	__delay_ms(1);
	sendLCD(0x0E,LCD_CMD);			// LCD on, cursor on
	__delay_ms(1);
	sendLCD(0x01,LCD_CMD);			// clear LCD
	__delay_ms(1);
    sendLCD(0x06,LCD_CMD);
	__delay_ms(1);
}

void initTimer0(void) {
    RCONbits.IPEN = 1;
    
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    
    INTCON2bits.TMR0IP = 1;
    
    T0CONbits.T08BIT = 1;
    T0CONbits.T0PS2 = 1;
    T0CONbits.T0PS1 = 1;
    T0CONbits.T0PS0 = 1;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.TMR0ON = 1;
    TMR0L = 0xFF;
    INTCONbits.TMR0IF = 0;
}

void sendLCD(unsigned char data, unsigned char type)
{
	LATEbits.LATE2 = type;		// select LCD command register 
	PORTD = data; 			// output command
	LATEbits.LATE1 = 1;
	__delay_ms(1);
	LATEbits.LATE1 = 0;
	__delay_ms(1);
    data = data << 4;
    PORTD = data;
    LATEbits.LATE1 = 1;
	__delay_ms(1);
	LATEbits.LATE1 = 0;
	__delay_ms(1);
}
void refreshLCD(unsigned char* text, unsigned char line) {
    unsigned short int i;
    unsigned char temp;
    temp = 0x80 + (line<<6);
    sendLCD(temp,LCD_CMD);
    for(i=0; i<16; i++) {
        sendLCD(*text,LCD_DATA);
        text++;
    }
}

void writeToLCD(unsigned char* text, unsigned char line, unsigned char position, unsigned char size){
    unsigned short int i;
    unsigned char temp2 = 0b0010000000;
    if(line== 1)
    {
        temp2 = 0b0011000000;
    }
    temp2 += position;
    sendLCD(temp2,LCD_CMD);
    for(i=0; i<size; i++) {
        sendLCD(*text,LCD_DATA);
        text++;
    }
    for(i=size; i<16; i++)
    {
        sendLCD(' ', LCD_DATA);
    }
}

//Below function is for writing a single character to a certain position on the LCD.
void writeSingleToLCD(unsigned char text, unsigned char line, unsigned char position){
    unsigned char temp2 = 0b0010000000;
    if(line== 1)
    {
        temp2 = 0b0011000000;
    }
    temp2 += position;
    sendLCD(temp2,LCD_CMD);
    __delay_ms(0.1);
    LATEbits.LATE2 = 1;	
	PORTD = text; 			
	LATEbits.LATE1 = 1;
	__delay_ms(0.1);
	LATEbits.LATE1 = 0;
	__delay_ms(0.1);
    text = text << 4;
    PORTD = text;
    LATEbits.LATE1 = 1;
	__delay_ms(0.1);
	LATEbits.LATE1 = 0;
	__delay_ms(0.1);
}


//Below function will be used to write the time to the LCD
void writeTimeToLCD(unsigned char secondsOne, unsigned char secondsTen, unsigned char minutesOne, unsigned char minutesTen, unsigned char hoursOne, unsigned char hoursTen)
{
    writeSingleToLCD(secondsOne, 1, 7);
    writeSingleToLCD(secondsTen, 1, 6);
    writeSingleToLCD(':', 1, 5);
    writeSingleToLCD(minutesOne, 1, 4);
    writeSingleToLCD(minutesTen, 1, 3);
    writeSingleToLCD(':', 1, 2);
    writeSingleToLCD(hoursOne, 1, 1);
    writeSingleToLCD(hoursTen, 1, 0);
    //The time has changed update the old values.
    old_seconds_ones_digit = secondsOne;    
    old_seconds_tens_digit = secondsTen;
    
    old_minutes_ones_digit = minutesOne;
    old_minutes_tens_digit = minutesTen;
    
    old_hours_ones_digit = hoursOne;
    old_hours_tens_digit = hoursTen;
}

void refreshAlarmClock() 
{
    writeSingleToLCD(' ', 1, 8);
    writeSingleToLCD(' ', 1, 9);
    writeSingleToLCD(' ', 1, 10);
    writeSingleToLCD(alarmHour/10+'0',1,11);
    writeSingleToLCD(alarmHour%10+'0',1,12);
    writeSingleToLCD(':',1,13);
    writeSingleToLCD(alarmMinute/10+'0',1,14);
    writeSingleToLCD(alarmMinute%10+'0',1,15);
}

void __interrupt(high_priority) tcInt(void)
{
    if (TMR0IE && TMR0IF)     // any timer 0 interrupts?
    {
        if(counter==70){
        /*
        i2c_start();
        unsigned char ACKtmr0 = i2c_wb(address_plus_write);
        unsigned char pointerValue = 0x07;
        ACKtmr0 = i2c_wb(pointerValue);
        i2c_start();
        ACKtmr0 = i2c_wb(address_plus_read);
        unsigned char controlRegister = i2c_rb(0);
        i2c_stop();
        unsigned char toBeAnded = 1 << 7;
        unsigned char Out = (controlRegister & toBeAnded);
        if(Out == 0x80)
        {
            LATCbits.LATC1 ^= 1;
        }*/
        i2c_start(); //Start the communication
        unsigned char ACK = i2c_wb(address_plus_write); //DS1307's address consisting of 7 bits plus the another 0 bit which indicates that we are going to be writing.
        /*if(ACK == 1)
        {
        writeToLCD(tempNOTACK, 0, 0, 7);
        }
        else if(ACK==0)
        {
        writeToLCD(tempACK, 0, 0, 7);
        }*/
        unsigned char pointerValue = 0x00; //Set the pointer value.
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
        ACK = i2c_wb(address_plus_read); // Now we are going to do a read operation.
        /*if(ACK == 1)
        {
        writeToLCD(tempNOTACK, 1, 0, 7);
        }
        else if(ACK==0)
        {
        writeToLCD(tempACK, 1, 0, 7);
        }*/
        unsigned char seconds = i2c_rb(1); //First we get the seconds 
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
        i2c_stop();
        //Below I compare it with our old values, so that I don't continously refresh the info on the LCD. This way I'll only refresh when a value has been changed.
        if(seconds_ones_digit != old_seconds_ones_digit)
        {
            old_seconds_ones_digit = seconds_ones_digit;
            writeSingleToLCD(seconds_ones_digit, 1, 7);
            //PORTD |= 0x07; //0000 0111 = 0xF7 so RD0, RD1, RD2 are made 1 rest is not touched.
            //Move the motor each time the second variable changes(Meaning each time a second elapses...)
            TRISD = 0x00;
        
            LATDbits.LATD0 = 0;
            LATDbits.LATD1 = 0;
            LATDbits.LATD2 = 0;
            LATDbits.LATD3 = 1;
        
            LATDbits.LATD0 = 0;
            LATDbits.LATD1 = 0;
            LATDbits.LATD2 = 1;
            LATDbits.LATD3 = 0;
        
            LATDbits.LATD0 = 0;
            LATDbits.LATD1 = 1;
            LATDbits.LATD2 = 0;
            LATDbits.LATD3 = 0;
        
            LATDbits.LATD0 = 1;
            LATDbits.LATD1 = 0;
            LATDbits.LATD2 = 0;
            LATDbits.LATD3 = 0;
            TRISD = 0x0F;
        }
        //Below code is pretty self explanatory, if any of the variables have changed, update them and also the according position on the LCD.
        if(seconds_tens_digit != old_seconds_tens_digit)
        {
            old_seconds_tens_digit = seconds_tens_digit;
            writeSingleToLCD(seconds_tens_digit, 1, 6);
        }
        
        if(minutes_ones_digit != old_minutes_ones_digit)
        {
            old_minutes_ones_digit = minutes_ones_digit;
            writeSingleToLCD(minutes_ones_digit, 1, 4);
        }
        
        if(minutes_tens_digit != old_minutes_tens_digit)
        {
            old_minutes_tens_digit = minutes_tens_digit;
            writeSingleToLCD(minutes_tens_digit, 1, 3);
        }
        
        if(hours_ones_digit != old_hours_ones_digit)
        {
            old_hours_ones_digit = hours_ones_digit;
            writeSingleToLCD(hours_ones_digit, 1, 1);
        }
        
        if(hours_tens_digit != old_hours_tens_digit)
        {
            old_hours_tens_digit = hours_tens_digit;
            writeSingleToLCD(hours_tens_digit, 1, 0);
        }
        //refreshLCD(LCD_line1,0);
        //writeTimeToLCD(seconds_ones_digit,seconds_tens_digit,minutes_ones_digit,minutes_tens_digit,hours_ones_digit,hours_tens_digit);
        refreshAlarmClock(); //I've written this function, as weirdly enough, sometimes the LCD screen got broken from writing too much to the LCD. (I am guessing this is caused because my program refreshes LCD a lot and writing to LCD functions have __delay_ms() calls inside, when I've decreased the __delay_ms() times, the LCD screen stopped getting broken, but I've left this here just in case.)
        counter = 0;
        }
        else
            counter++;
        //Now we have to check for our buzzer. Whenever the alarm variables are equal to our clock variables, we enable the buzzer, when they are not equal we disable the buzzer.
        if(alarmHour/10 == hours_tens_digit-'0' && alarmHour%10 == hours_ones_digit-'0' && alarmMinute/10 == minutes_tens_digit-'0'  && alarmMinute%10 == minutes_ones_digit-'0' )
        {
            LATCbits.LATC1 = 0;
        }
        else
            LATCbits.LATC1 = 1;
        TMR0IF=0; //Don't forget to clear the interrupt flag.
    }
    /*writeTimeToLCD(seconds_ones_digit,seconds_tens_digit,minutes_ones_digit,minutes_tens_digit,hours_ones_digit,hours_tens_digit);
        i2c_stop();
    }*/
}