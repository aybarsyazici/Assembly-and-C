#include <xc.h>
#include "ee308.h"

int counter, Realduty2, tmpduty2, Realduty1, tmpduty1;
double tmp1, tmp2;

void initLCD(void)
{
    TRISD &= 0x0F;           // D<7:4> OUTPUT
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

void initTimer0(void) {
    RCONbits.IPEN = 1;
    
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    
    INTCON2bits.TMR0IP = 1;
    
    T0CONbits.T08BIT = 1;
    T0CONbits.T0PS2 = 0;
    T0CONbits.T0PS1 = 0;
    T0CONbits.T0PS0 = 1;
    T0CONbits.T0CS = 0;
    T0CONbits.PSA = 0;
    T0CONbits.TMR0ON = 1;
    TMR0L = 0x00;
    INTCONbits.TMR0IF = 0;
}

void changeDuty(int setduty1, int setduty2) {
    tmpduty1 = setduty1;
    tmp1 = setduty1/10.23; //tmp1 is a double value
    sendLCD(0x80,LCD_CMD);
    if(tmp1>=100) { //If tmp1 is == 100, that means setduty1 was 1023, then we will display 100.00%
        sendLCD('1',LCD_DATA);
        sendLCD('0',LCD_DATA);
        sendLCD('0',LCD_DATA);
        sendLCD('.',LCD_DATA);
        sendLCD('0',LCD_DATA);
        sendLCD('0',LCD_DATA);
    }
    else{       //If it was not 1023, then we will have to display the whole number            
        int integertmp1 = (int) tmp1; //First we parse the tmp1 to integer, so we get only the ones digit and tens digit, and discard the ones after the '.'
        sendLCD(' ', LCD_DATA); //We need an emptyspace, because we will only be displaying a 2 digit number instead of a 3 digit one.
        if(integertmp1 > 9)
        {
            sendLCD(integertmp1/10+'0',LCD_DATA);
            integertmp1 = integertmp1%10; //we have displayed the tens digit, now we only have the ones digit remaining.
        }
        else
            sendLCD(' ', LCD_DATA); //If integertmp1 was not larger than 9, then we don't have a tens digit to display, so we display an empty space.
        sendLCD(integertmp1 + '0', LCD_DATA); //Now we display the ones digit.
        sendLCD('.', LCD_DATA); //Now we have to display the numbers after the decimal point.
        integertmp1 = (int) tmp1; //We get the double number again and parse it to int
        tmp1 = tmp1 - integertmp1; //When we substract the double from the integer, we only have the numbers after the decimal point.
        tmp1 = tmp1*100; //Get the two numbers after the decimal point
        integertmp1 = (int) tmp1; //Parse it to int. And now we are ready to display the numbers after the decimal point.
        sendLCD(integertmp1/10+'0', LCD_DATA); 
        integertmp1 = integertmp1 % 10;
        sendLCD(integertmp1+'0', LCD_DATA);
    }
    sendLCD('%',LCD_DATA);
    sendLCD(' ',LCD_DATA);
    tmpduty2 = setduty2; //We repeat the same procedures for the displaying the second percentage.
    tmp2 = setduty2/10.23;
    if(tmp2>=100) {
        sendLCD('1',LCD_DATA);
        sendLCD('0',LCD_DATA);
        sendLCD('0',LCD_DATA);
        sendLCD('.',LCD_DATA);
        sendLCD('0',LCD_DATA);
        sendLCD('0',LCD_DATA);
        tmp2 = 0;
    }
    else
    {
        int integertmp2 = (int) tmp2;
        sendLCD(' ', LCD_DATA);
        if(integertmp2 > 9)
        {
            sendLCD(integertmp2/10+'0',LCD_DATA);
            integertmp2 = integertmp2%10;
        }
        else
            sendLCD(' ', LCD_DATA);
        sendLCD(integertmp2+'0',LCD_DATA);
        sendLCD('.',LCD_DATA);
        integertmp2 = (int) tmp2;
        tmp2 = tmp2 - integertmp2;
        tmp2 = tmp2 * 100;
        integertmp2 = (int) tmp2;
        sendLCD(integertmp2/10+'0',LCD_DATA);
        integertmp2 = integertmp2 % 10;
        sendLCD(integertmp2+'0',LCD_DATA);
    }
    sendLCD('%',LCD_DATA);
}

void displayOnLowerLCD(int pwm1, int pwm2) { //This is a function I have written to facilitate the process of displaying.
    int temp_pwm1 = pwm1; //we create a temporary variable as a copy of our original variable, to not change our original variable.
    sendLCD(0xC0,LCD_CMD);
    sendLCD(' ', LCD_DATA);
    if(temp_pwm1 > 999) { 
        sendLCD('1',LCD_DATA); //As pwm value can be at max 1023, if it is larger than 999 then, for sure we need to display a 1.
        temp_pwm1 = temp_pwm1%1000; //Then get the remaining 3 digits.
    }
    else
        sendLCD(' ',LCD_DATA); //If it is not larger than 999, then we just display an empty space.
    if(temp_pwm1 > 99)
    {
        sendLCD(temp_pwm1/100+'0', LCD_DATA); //If the PWM value is between 100-999 inclusive, if we divide it by 100 we get the hundreds digit.
        temp_pwm1 = temp_pwm1%100; //Get the last two digits.
    }
    else if(pwm1 > 999)     //Our TEMP_PWM value was less than 100, so we skipped the first if. And our original PWM was larger than 999, meaning We need to display a 0 not an empty space.
        sendLCD('0', LCD_DATA);
    else
        sendLCD(' ', LCD_DATA); //Our original pwm value was less than 1000, and our current temp_pwm value was less than 100, thus we display an empty space.
    if(temp_pwm1 > 9)
    {
        sendLCD(temp_pwm1/10+'0', LCD_DATA);  //Display the tens digit.
        temp_pwm1 = temp_pwm1%10; //Get the last digit.
    }
    else if(pwm1 > 99)      //Our original pwm value was/is higher than 99, this means we have another digit in front of us right now, thus we display a 0 instead of an empty space.
        sendLCD('0', LCD_DATA);
    else
        sendLCD(' ', LCD_DATA);
    sendLCD(temp_pwm1+'0',LCD_DATA); //Display the last digit.
    sendLCD(' ',LCD_DATA);  //Leave some empty space for the next pwm value.
    sendLCD(' ',LCD_DATA);
    sendLCD(' ',LCD_DATA);
    sendLCD(' ',LCD_DATA);
    /***********************************/
    int temp_pwm2 = pwm2;   //We repeat the same prodecures for the second pwm value.
    if(temp_pwm2 > 999) {
        sendLCD('1',LCD_DATA);
        temp_pwm2 = temp_pwm2%1000;
    }
    else
        sendLCD(' ',LCD_DATA);
    if(temp_pwm2 > 99)
    {
        sendLCD(temp_pwm2/100+'0', LCD_DATA);
        temp_pwm2 = temp_pwm2%100;
    }
    else if(pwm2 > 999)
        sendLCD('0', LCD_DATA);
    else
        sendLCD(' ', LCD_DATA);
    if(temp_pwm2 > 9)
    {
        sendLCD(temp_pwm2/10+'0', LCD_DATA);
        temp_pwm2 = temp_pwm2%10;
    }
    else if(pwm2 > 99)
        sendLCD('0', LCD_DATA);
    else
        sendLCD(' ', LCD_DATA);
    sendLCD(temp_pwm2+'0',LCD_DATA);
}

void __interrupt(high_priority) tcInt(void)
{
    if (TMR0IE && TMR0IF)     // any timer 0 interrupts?
    {
        TMR0IF=0;
        counter++;
        if(counter==1022) //As our PWM value can get 1023 at max, we set this value to 1022. So when PWM Value is = 1023, the duty cycle is 100%, and the LED is always on.
        {
            counter = 0;
            Realduty1 = tmpduty1;
            Realduty2 = tmpduty2;
            LATDbits.LATD0 = 1;
            LATDbits.LATD1 = 1;
        }
        if(counter==Realduty1)
            LATDbits.LATD0 = 0;
        if(counter==Realduty2)
            LATDbits.LATD1 = 0;
    }
    
    return;
}