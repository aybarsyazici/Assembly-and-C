#pragma config WDT = OFF
#pragma config PBADEN = OFF

#define _XTAL_FREQ 16000000

#define LCD_CMD 0
#define LCD_DATA 1

void initLCD(void);
void sendLCD(unsigned char data, unsigned char type);
void initTimer0(void);
void initTimer2(void);
void initSerial(void);
void initSeven(void);

void __interrupt(high_priority) tcInt(void);