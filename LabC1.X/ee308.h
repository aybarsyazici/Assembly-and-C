#pragma config WDT = OFF
#pragma config PBADEN = OFF

#define _XTAL_FREQ 16000000

#define LCD_CMD 0
#define LCD_DATA 1

void initLCD(void);
void sendLCD(unsigned char data, unsigned char type);
void initTimer0(void);
void changeDuty(int setduty1, int setduty2);
void displayOnLowerLCD(int pwm1, int pwm2);

void __interrupt(high_priority) tcInt(void);