#include "xc.h" 
#include <p24FJ1024GB610.h> 

 
#define LCD_MAX_COLUMN      16 
 
#define CS1_BASE_ADDRESS    0x00020000 
#define CS2_BASE_ADDRESS    0x000A0000 
 
static __eds__ unsigned int __attribute__((noload, section("epmp_cs1"), 
address(CS1_BASE_ADDRESS))) ADDR0 __attribute__((space(eds))); 
static __eds__ unsigned int __attribute__((noload, section("epmp_cs1"), 
address(CS1_BASE_ADDRESS))) ADDR1 __attribute__((space(eds))); 
 
#define LCD_COMMAND_CLEAR_SCREEN        0x01 
#define LCD_COMMAND_RETURN_HOME         0x02 
#define LCD_COMMAND_ENTER_DATA_MODE     0x06 
#define LCD_COMMAND_CURSOR_OFF          0x0C 
#define LCD_COMMAND_CURSOR_ON           0x0F 
#define LCD_COMMAND_MOVE_CURSOR_LEFT    0x10 
#define LCD_COMMAND_MOVE_CURSOR_RIGHT   0x14 
#define LCD_COMMAND_SET_MODE_8_BIT      0x38 
#define LCD_COMMAND_ROW_0_HOME          0x80 
#define LCD_COMMAND_ROW_1_HOME          0xC0 
#define LCD_START_UP_COMMAND_1          0x33     
#define LCD_START_UP_COMMAND_2          0x32     
 
#define LCD_COMMAND_CLEAR_SCREEN        0x01 
#define LCD_COMMAND_RETURN_HOME         0x02 
#define LCD_COMMAND_ENTER_DATA_MODE     0x06 
#define LCD_COMMAND_CURSOR_OFF          0x0C 
#define LCD_COMMAND_CURSOR_ON           0x0F 
#define LCD_COMMAND_MOVE_CURSOR_LEFT    0x10 
#define LCD_COMMAND_MOVE_CURSOR_RIGHT   0x14 
#define LCD_COMMAND_SET_MODE_8_BIT      0x38 
#define LCD_COMMAND_ROW_0_HOME          0x80 
#define LCD_COMMAND_ROW_1_HOME          0xC0 
#define LCD_START_UP_COMMAND_1          0x33     
#define LCD_START_UP_COMMAND_2          0x32   
 
#define LCD_SET_ACG(d) LCD_CMD((d & 0x3f) | 0x40); 
#define LCD_SET_ADD(d) LCD_CMD((d & 0x7f) | 0x80); 
 
#define delay_32ms()  TMR1 = 0; while(TMR1 < 2000); 
#define delay_162us() TMR1 = 0; while(TMR1 < 100); 
#define delay_48us()  TMR1 = 0; while(TMR1 < 3); 
#define DELAY() TMR1=0; while( TMR1<9000) 
 
#define POT 5 // 10k potentiometer connected to AN5 input 
#define AINPUTS 0xffef // Analog inputs for Explorer16 POT 
 
void LCD_CMD(char cmd){ 
    ADDR0 = cmd; 
}
void LCD_DATA(char data){ 
    ADDR1 = data; 
} 
void LCD_ClearScreen(void) { 
    LCD_CMD(LCD_COMMAND_CLEAR_SCREEN); 
    delay_32ms(); 
 
    LCD_CMD(LCD_COMMAND_RETURN_HOME); 
    delay_32ms(); 
} 
 
void LCDinit(void) { 
 
    PMCON1bits.PMPEN = 1; 
    PMCON1bits.MODE = 3; 
    PMCON1bits.CSF = 0; 
    PMCON1bits.ALP = 0; 
    PMCON1bits.ALMODE = 0; 
    PMCON1bits.BUSKEEP = 0; 
    PMCON1bits.ADRMUX = 0; 
    PMCON1bits.IRQM = 1; 
 
    PMCS1BS = (CS1_BASE_ADDRESS >> 8); 
 
    PMCS1CFbits.CSDIS = 0; // enable CS 
    PMCS1CFbits.CSP = 1; // CS1 polarity 
    PMCS1CFbits.BEP = 1; // byte enable polarity 
    PMCS1CFbits.WRSP = 1; // write strobe polarity 
    PMCS1CFbits.RDSP = 1; // read strobe polarity 
    PMCS1CFbits.CSPTEN = 1; // enable CS port 
    PMCS1CFbits.SM = 0; // read and write strobes on separate lines 
    PMCS1CFbits.PTSZ = 0; // data bus width is 8-bit 
    PMCS1MDbits.ACKM = 0; // PMACK is not used 
 
    PMCS1MDbits.DWAITB = 3; 
    PMCS1MDbits.DWAITM = 0xf; 
    PMCS1MDbits.DWAITE = 3; 
 
    PMCON2bits.RADDR = 0; // don't care since CS2 is not be used 
    PMCON4 = 0x0001; // PMA0 - PMA15 address lines are enabled 
 
    PMCON3bits.PTWREN = 1; // enable write strobe port 
    PMCON3bits.PTRDEN = 1; // enable read strobe port 
    PMCON3bits.PTBE0EN = 1; // enable byte enable port 
    PMCON3bits.PTBE1EN = 0; // enable byte enable port 
    PMCON3bits.AWAITM = 0b11; // set address latch pulses width to 3 1/2 Tcy 
    PMCON3bits.AWAITE = 1; // set address hold time to 1 1/4 Tcy 
 
    PMCON1bits.PMPEN = 1; // enable the module 
 
    T1CON = 0x8030; // Fosc/2, prescaled 1:256, 16us/tick 
    delay_32ms();
     LCD_CMD(LCD_START_UP_COMMAND_1); 
    delay_48us(); 
 
    LCD_CMD(LCD_START_UP_COMMAND_2); 
    delay_48us(); 
 
    LCD_CMD(LCD_COMMAND_SET_MODE_8_BIT); 
    delay_48us(); 
 
    LCD_CMD(LCD_COMMAND_CURSOR_OFF); 
    delay_48us(); 
 
    LCD_CMD(LCD_COMMAND_ENTER_DATA_MODE); 
    delay_48us(); 
 
    LCD_CMD(LCD_COMMAND_CLEAR_SCREEN); 
    delay_162us(); 
 
    LCD_CMD(LCD_COMMAND_RETURN_HOME); 
    delay_162us(); 
} 
void LCD_PutChar(char inputChar){ 
    LCD_DATA(inputChar); 
    delay_48us(); 
} 
 
void LCD_PutString(char* inputString, uint16_t length) { 
    while (length--) { 
        switch (*inputString) { 
            case 0x00: 
                return; 
            default: 
                LCD_PutChar(*inputString++);                 
                break; 
        } 
    } 
} 

void initADC(int pinNumber) { 
    ANSB = pinNumber; 
    _SSRC = 0x7; 
    AD1CSSL = 0;  
    AD1CON2 = 0;  
    AD1CON3 = 0x1F02;  
    _ADON = 1;  
} 

void Init_ADCInterrupt(void) {
	ANSB = 1;
	AD1CHS = 4;
	_SSRC = 0x7;
	AD1CSSL = 0;
	AD1CON2 = 0;
	AD1CON3 = 0x1f02;
	ADON = 1;

	_AD1IF = 0;
	AD1CON5bits.ASINT = 3;
	_AD1IE = 1;
	_AD1IP = 1;
} 

int readADC(int ch) {
    AD1CHS = ch;
    _SAMP = 1;
    while(_DONE == 1);
    return ADC1BUF0;
}
 
void _ISR _ADCInterrupt(void) {
	int a;
    a = readADC(POT);
    a >>= 7;
    a = 0x80 >> a;
    LCD_PutString("ADC la: ", 8);
    char* strValue = (char*)a;
    LCD_PutString(strValue, strlen(strValue)); 
    LCD_PutString(" V", 2);
    
	_AD1IF = 0;
}
 
int main(void) { 
    LCDinit(); 
    //initADC(AINPUTS); 
    Init_ADCInterrupt(); 
    
    while (1);           
    return 0;
}