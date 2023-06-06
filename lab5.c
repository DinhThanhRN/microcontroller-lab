// 1. Thực hiện chương trình cấu hình UART để mỗi khi nhận được chuỗi “Hello” từ PC 
// thì sẽ gửi lại chuỗi “My name is A”, với A là tên sinh viên. Khi nhận được chuỗi 
// khác chuỗi “Hello”, thì gửi lại chuổi “I do not know”. Lưu ý có sử dụng ngắt UART. 

#include <xc.h> 
#include <string.h>

#define BRATE 68 // 14400 Bd 
#define U_ENABLE 0x8008 // enable UART, 8-bit data, no parity 
#define U_TX 0x0400 // enable transmission, clear all flags
#define BACKSPACE 0x8 // ASCII backspace character code
#define BUF_SIZE 128 

char name[] = “A”; // change this to your name 
char buffer[BUF_SIZE]; // buffer to store received string 
int index; // index of buffer

void initU1(void) { 
	U1BRG = BRATE; // set baud rate 
	U1MODE = U_ENABLE; // set UART mode 
	U1STA = U_TX; // set UART status 

	U1STAbits.URXISEL = 0; // interrupt when a character is received 
	_U1RXIP = 4; // set interrupt priority to 4
	_U1RXIF = 0; // clear interrupt flag 
	_U1RXIE = 1; // enable interrupt 

	// Thiết lập các chân RX và TX
 	RPINR18bits.U1RXR = 10;
  	RPOR8bits.RP17R = 3;
}

int putU1(int c) { 
	while(U1STAbits.UTXBF); // wait while Tx buffer full 
	U1TXREG = c; // send character 
	return c;
}

void putsU1(char *s) { 
	while(*s) { 
		putU1(*s++); // send character and increment pointer 
	} 
}

char getU1(void) { 
    while (!U1STAbits.URXDA); // wait for a new character to arrive    
    return U1RXREG; // read the character from the receive buffer 
} 

char *getsnU1(char *s, int len) {
  	// Nhận một chuỗi qua UART với độ dài tối đa cho trước
  	char *p = s; // Sao chép con trỏ chuỗi
  	int cc = 0; // Đếm số ký tự đã nhận
  	do {
    	*s = getU1(); // Nhận một ký tự mới
    	if ((*s == BACKSPACE)&&(s > p)) {
      		putU1(' '); // Ghi đè lên ký tự cuối cùng
      		putU1(BACKSPACE);
      		len++;
      		s--; // Giảm con trỏ chuỗi xuống một đơn vị
      		continue;
    	}
    	if (*s == '\n') // Ký tự xuống dòng, bỏ qua
      		continue;
    	if (*s == '\r') // Ký tự kết thúc dòng, kết thúc vòng lặp
      		break;
    	s++; // Tăng con trỏ chuỗi lên một đơn vị
    		len--;
  	} while (len > 1); // Cho đến khi đầy bộ đệm
  	*s = '\0'; // Thêm ký tự kết thúc chuỗi
  	return p; // Trả về con trỏ chuỗi
}

void _ISR _U1RXInterrupt(void) {
  	// Hàm ngắt nhận UART
  	//getsnU1(s, BUF_SIZE); // Nhận chuỗi từ UART vào biến s
	buffer = getsnU1(buffer, BUF_SIZE);

  	if (strcmp(buffer, "Hello") == 0) { // So sánh chuỗi với "Hello"
   		putsU1("My name is TRINH DINH THANH"); // Gửi tên sinh viên
    	//putsU1(name); 
    	putU1('\r'); // Gửi ký tự kết thúc dòng
  	} else { // Nếu khác, gửi lại chuỗi "I do not know"
    	putsU1("I do not know");
    	putU1('\r'); // Gửi ký tự kết thúc dòng
  	}

  	IFS0bits.U1RXIF = 0; // Xóa cờ ngắt
}

int main(void) { 
	initU1(); // initialize UART1 
	while(1); // infinite loop 
}

// 2. Thực hiện chương trình cấu hình UART và ADC để mỗi khi nhận được chuỗi 
// “temperature?” thì sẽ gửi lại giá trị nhiệt độ đọc được từ cảm biến nhiệt độ. Ví dụ 
// nhiều độ là 28ºC thì sẽ gửi đến PC chuỗi kí tự là “28 doC”. 

#include <xc.h>
#include <p24fj1024gb610.h>
#include <string.h>

#define BRATE 68 // 14400 Bd 
#define U_ENABLE 0x8008 // enable UART, 8-bit data, no parity 
#define U_TX 0x0400 // enable transmission, clear all flags
#define BACKSPACE 0x8 // ASCII backspace character code
#define BUF_SIZE 128 

char name[] = “A”; // change this to your name 
char buffer[BUF_SIZE]; // buffer to store received string 
int index; // index of buffer

void Init_T1Interrupt(void){
	T1CON = 0;
	TMR1 = 0;
	PR1 = 31250;
	T1CONbits.TCS = 0;
	T1CONbits.TCKPS = 2;

	_T1IF = 0;
	_T1IE = 1;
	_T1IP = 1;

	T1CONbits.TON = 1;
}

void _ISR _T1Interrupt(void) {
	_SAMP = 1;
	IFS0bits.T1IF = 0;
	TMR1 = 0;
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

void _ISR _ADCInterrupt(void) {
	int adcValue = ADC1BUF0;
	float temp, vol;
	vol = (adcValue * 3.3) / 1024;
	temp = (vol - 0.5) * 0.01;
	
	const int i = (int)temp;
	char* ch = (char*) i;
	putsU1(ch);

	_AD1IF = 0;
}




void initU1(void) { 
	U1BRG = BRATE; // set baud rate 
	U1MODE = U_ENABLE; // set UART mode 
	U1STA = U_TX; // set UART status 

	U1STAbits.URXISEL = 0; // interrupt when a character is received 
	_U1RXIP = 4; // set interrupt priority to 4
	_U1RXIF = 0; // clear interrupt flag 
	_U1RXIE = 1; // enable interrupt 

	// Thiết lập các chân RX và TX
 	RPINR18bits.U1RXR = 10;
  	RPOR8bits.RP17R = 3;
}

int putU1(int c) { 
	while(U1STAbits.UTXBF); // wait while Tx buffer full 
	U1TXREG = c; // send character 
	return c;
}

void putsU1(char *s) { 
	while(*s) { 
		putU1(*s++); // send character and increment pointer 
	} 
}

char *getsnU1(char *s, int len) {
  	// Nhận một chuỗi qua UART với độ dài tối đa cho trước
  	char *p = s; // Sao chép con trỏ chuỗi
  	int cc = 0; // Đếm số ký tự đã nhận
  	do {
    		*s = getU1(); // Nhận một ký tự mới
    		if ((*s == BACKSPACE)&&(s > p)) {
      			putU1(' '); // Ghi đè lên ký tự cuối cùng
      			putU1(BACKSPACE);
      			len++;
      			s--; // Giảm con trỏ chuỗi xuống một đơn vị
      			continue;
    		}
    		if (*s == '\n') // Ký tự xuống dòng, bỏ qua
      		continue;
    		if (*s == '\r') // Ký tự kết thúc dòng, kết thúc vòng lặp
      		break;
    		s++; // Tăng con trỏ chuỗi lên một đơn vị
    		len--;
  	} while (len > 1); // Cho đến khi đầy bộ đệm
  	*s = '\0'; // Thêm ký tự kết thúc chuỗi
  	return p; // Trả về con trỏ chuỗi
}

void _ISR _U1RXInterrupt(void) {
  	// Hàm ngắt nhận UART
  	getsnU1(s, BUF_SIZE); // Nhận chuỗi từ UART vào biến s

  	if (strcmp(s, "temperature") == 0) { // So sánh chuỗi với "Hello"
   		putsU1("My name is "); // Nếu giống, gửi lại chuỗi "My name is "
    	putU1('\r'); // Gửi ký tự kết thúc dòng

		_SAMP = 1;
  	} else { 
    	putsU1("I do not know");
    	putU1('\r'); // Gửi ký tự kết thúc dòng
  	}

  	IFS0bits.U1RXIF = 0; // Xóa cờ ngắt
}
void main() {
	
	Init_ADCInterrupt();
	initU1();
	// Init_T1Interrupt();

	while(1);
}