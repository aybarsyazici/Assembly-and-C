#include <xc.h>
#include "ee308.h"
unsigned char counter=0;
unsigned int counter2 = 0;
unsigned char serialRead;
unsigned char c = 0b11111111;
int oneCount = 0;
int timer1Index = 0;
int timer2Index = 0;
unsigned int displayingCounter = 0;
unsigned char inputAvailable = 0;
void initLCD(void)
{
    TRISD = 0x00;           // all bits OUTPUT
    TRISEbits.TRISE1 = 0;   // EN
    TRISEbits.TRISE2 = 0;   // RS

    sendLCD(0x30,LCD_CMD);
    __delay_ms(5);
    sendLCD(0x30,LCD_CMD);
    __delay_ms(1);
    sendLCD(0x30,LCD_CMD);
    __delay_ms(1);
    
	sendLCD(0x38,LCD_CMD);			// init LCD
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
    TMR0L = 0x00;
    INTCONbits.TMR0IF = 0;
}

void initTimer2(void)
{
    INTCONbits.GIE = 1;  //Global interrupts.
    INTCONbits.PEIE = 1; //Peripheral interrupts.
    PIE1bits.TMR2IE = 1; //Enable timer 2 interrupt.
    PIR1bits.TMR2IF = 0; //Clear the timer 2 interrupt flag.
    IPR1bits.TMR2IP = 1; //Make timer2 high priority.
    
    T2CONbits.T2OUTPS0 = 0; //Set the post scaler to 1:1
    T2CONbits.T2OUTPS1 = 0;
    T2CONbits.T2OUTPS2 = 0;
    T2CONbits.T2OUTPS3 = 0;
    
    T2CONbits.T2CKPS1 = 0;
    T2CONbits.T2CKPS0 = 1; //Prescaler is 4. Thus TMR2 Freq is now = 16MHz/16 = 1Mhz, Timer2 will increment itself with a freq of 1MHz.
    //Timer2 interrupt occurs when Timer2 register equals to PR2 register, thus by setting the PR2 register, we can adjust our Timer2 interrupt frequency.
    PR2 = 0x67; //Now timer2 interrupt will occur every 1MHz/(103+1) = 9615 times a second, (9615Hz, which is the desired BAUD value)
    T2CONbits.TMR2ON = 1; //TMR2 ON.
}

void initSerial(void) {
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;
    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 1;
    
    BAUDCONbits.BRG16 = 1;  // 16-bit/Asynchronous
    TXSTAbits.BRGH = 0;     // BAUD=F_OSC/[16(SPBRGH:SPBRG + 1)]
    TXSTAbits.SYNC = 0;     // asynchronous
    
    SPBRGH = 0;
    SPBRG = 51;     // 19200 for FOSC = 16 MHz
}

void initSeven(void) {
    //TRISD = 0x00;
    TRISA &= 0xC3;      // PORT bits 2,3,4,5 output
    LATA  &= 0xC3;      // all segments deselected
    LATAbits.LATA5 = 1; // turn on DIS4
}

void __interrupt(high_priority) tcInt(void)
{
    if (TMR0IE && TMR0IF)     // any timer 0 interrupts?
    {
        TMR0IF=0;
        counter++;
        if(counter==19) //Was 76
        {
            counter = 0;
        }
    }
    if (TMR2IE && TMR2IF) //TMR2 interrupts
    {
        if(inputAvailable == 1) //Are there any serial inputs received?
        {
            if(counter2 == 0)
            {
                __delay_ms(2);      //This delay is to be able to better see the results on the oscilloscope, when we remove this the signal in the oscilloscope moves a lot
                LATBbits.LATB0 = 0; //So we wait in the idle stage a bit before sending the start bit so we can observe the signal better in the oscilloscope.
                //__delay_ms(0.045);
                //PIR1bits.RC1IF = 0;
                serialRead = c; //We get the serial input.
                //PIR1bits.RC1IF will get cleared when we read the RCREG register.
            }
            else if(counter2 < 9) //Now we need to go through the LSB till the MSB and set the bits accordingly. We will do this 8 times. So we get 8 bits.
            {
                unsigned char lsb = serialRead & 1;     //Get the LSB.
                if(lsb == 1){   //If the LSB is 1, 
                    oneCount++; //Count the number of 1's as we will require it for the ODD parity.
                    LATBbits.LATB0 = 1;
                }
                else    //If the LSB is 0
                {
                    LATBbits.LATB0 = 0;
                }
                serialRead >>=1;
                //sendLCD(lsb+'0', LCD_DATA);  
            }
            else if (counter2 == 9) //Now we need the parity bit.
            {
                //sendLCD(' ', LCD_DATA);
                int temp = oneCount % 2; //As we have already counted the number of 1's we can use this to get which bit we need to display.
                if(temp == 0)   //If it is an even number, we need to add another 1 for the ODD parity.
                {
                    //sendLCD('1', LCD_DATA); //For ODD parity.
                    LATBbits.LATB0 = 1;
                }
                else    //If it is already odd, then just display a 0, to keep the odd parity.
                {
                    LATBbits.LATB0 = 0;
                    //sendLCD('0', LCD_DATA);
                }
            }
            else if(counter2 == 10) //Finally the stop bit.
            {
                //sendLCD(' ', LCD_DATA);
                //sendLCD('1', LCD_DATA); //STOP bit.
                LATBbits.LATB0 = 1;
            }
            else    //Reset everything and start again.
            {
                counter2 = -1;
                oneCount = 0;
            }
            counter2++;
        }
        else //We have not received a serial input so we are in idle.
        {
            LATBbits.LATB0 = 1; //IDLE
        }
        TMR2IF = 0;
        /* Each second incrementation for testing. 1/9615 *10000 = 1second at clock = 16MHz
        if(counter2 == 10000)
        {
            sendLCD(displayingCounter + '0', LCD_DATA);
            displayingCounter++;
            counter2 = 0;
        }*/
    }
    return;
}
