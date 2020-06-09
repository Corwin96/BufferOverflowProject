/*
 * Bufferoverflow.c
 *
 * Created: 05/06/2020 14:08:35
 * Author : Stijn, Corwin
 */ 

#define F_CPU 16000000
#define RESOLUTION 65536 //max in 16bit counter
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

char buff[64];
char instruction[64];
int i = 0;
int loggedIn = 0;

void initUsart();
void writeInt(int i);
void writeChar(char x);
void writeString(char st[]);
ISR(USART_RX_vect);
int checkpwd( char * pwd, int len);
void controller();
void login();

int main(void)
{
    initUsart();
	
	writeString("\n\tstart\r\n");
	writeString("Enter Password:\r\n");
	DDRB |= (1 << PORTB4);
    while (1) 
    {
    }
}

void initUsart(){
	UCSR0A = 0;
	UCSR0B = (1 << RXEN0)|(1 << TXEN0)|(1 << RXCIE0); // Enable de USART Transmitter
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); /* 8 data bits, 1 stop bit, No parity */
	UBRR0H=00;
	UBRR0L=103; //baudrate 9600
	sei();
}

void writeInt(int i) {
	char buffer [7];
	itoa ( i, buffer, 10 );
	writeString(buffer);
}

void writeString(char st[]){
	for(uint8_t i = 0; st[i] != 0; i++){
		writeChar(st[i]);
	}
}

void writeChar(char x) {
	while(~UCSR0A & (1 << UDRE0));
	UDR0 = x;
}

ISR(USART_RX_vect){
	if (!loggedIn){
		login();
	} else {
		controller();
	}
}

void controller(){
	instruction[i] = UDR0;
	if ((0x1F < instruction[i] && instruction[i] < 0x7F) || instruction[i] == 0x0D){
		writeChar(instruction[i]);
		if (instruction[i] == 0x0D || i > 4) {
			if (instruction[0] == 'o' && instruction[1] == 'n') {
					PORTB |= 1 << PORTB4;
					writeString("\r\nTurned light on\r\n");
				} else if (instruction[0] == 'o' && instruction[1] == 'f' && instruction[2] == 'f') {
					PORTB &= ~(1 << PORTB4);
					writeString("\r\nturned light off\r\n");
				} else if (instruction[0] == 's' && instruction[1] == 't' && instruction[2] == 'o' && instruction[3] == 'p') { 
					loggedIn = 0;
					writeString("\r\nLogged out\r\n");
					writeString("\r\nEnter password to log back in:\r\n");
				} else {
				writeString("\r\nInvalid response\r\n");
			}
			i=0;
			} else {
			i++;
		}
		} else if (buff[i] == 0x08 || buff[i] == 0x7F) {
		i--;
	}	
}

void login(){
	// Get data from the USART in register
	buff[i] = UDR0;
	if ((0x1F < buff[i] && buff[i] < 0x7F) || buff[i] == 0x0D){
		writeChar(buff[i]);
		if (buff[i] == 0x0D) {
			int result = checkpwd(buff, i);
			if (result == 0) {
				writeString("\r\nSuccess\r\n");
				loggedIn = 1;
				writeString("\r\nDo you want to turn on the light? (on/off/stop)\r\n");
			} else {
				writeString("\r\nIncorrect\r\n");
				writeString("Enter Password:\r\n");
				loggedIn = 0;
			}
			i=0;
		} else {
			i++;
		}
	} else if (buff[i] == 0x08 || buff[i] == 0x7F) {
		i--;
	}
}

int checkpwd( char * pwd, int len) {
	char buff2[6] = {0};
	//password is 3nTeR
	memcpy(buff2, pwd, len);
	buff2[3] -= (10*10);
	buff2[3]--;
	buff2[0] ^= 51;
	buff2[1] ^= 110;
	buff2[4] -= ((int)buff2[2] -2);
	buff2[2] ^= 84;
	int j=0;
	int result = 0;
	for (j=0; j<5; j++){
		result += (int)buff2[j];
	}
	return result;
}
