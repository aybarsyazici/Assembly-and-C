#pragma config WDT = OFF
#pragma config PBADEN = OFF

#define _XTAL_FREQ 16000000

#define LCD_CMD 0
#define LCD_DATA 1


unsigned char address_plus_read = 0xD1;
unsigned char address_plus_write = 0xD0;

void initLCD(void);
void sendLCD(unsigned char data, unsigned char type);
void refreshLCD(unsigned char*, unsigned char);
void writeToLCD(unsigned char* text, unsigned char line, unsigned char position, unsigned char size);
void writeSingleToLCD(unsigned char text, unsigned char line, unsigned char position);
void initTimer0(void);
void writeTimeToLCD(unsigned char secondsOne, unsigned char secondsTen, unsigned char minutesOne, unsigned char minutesTen, unsigned char hoursOne, unsigned char hoursTen);
void refreshAlarmClock();

void __interrupt(high_priority) tcInt(void);