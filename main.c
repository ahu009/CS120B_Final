/*
 * GccApplication1.cp/*	Partner(s) Name & E-mail: Dillon Sio dsio001@ucr.edu
 *	Lab Section: 022
 *	Assignment: Lab #6  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b)
{
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

unsigned char GetBit(unsigned char x, unsigned char k)
{
	
	return ((x & (0x01 << k)) != 0);
}

unsigned char ButtonA1;
unsigned char ButtonA0;
unsigned char cnt;
unsigned char period = 1;
unsigned char score;

enum States{Init} state;

void ButtonTick()
{
	static unsigned char score = 5;
	const unsigned char* string = reinterpret_cast<const unsigned char *>("Score: ");
	
	switch(state) //Transition 
	{
		case Init:
			state = Init;
			break;
	}

	switch(state)//Actions
	{
		case Init:
			LCD_Cursor(1);
		    LCD_DisplayString(1, string);
			LCD_Cursor(8);
			LCD_WriteData('0' + ((score % 1000) / 100));
			LCD_WriteData('0' + ((score % 100) / 10));
			LCD_WriteData('0' + ((score % 10)));
			score++;
			break;
	}
}


int main(void)
{
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	DDRA = 0x00; PORTA = 0xFF;
	
	TimerSet(250);
	TimerOn();

	// Initializes the LCD display
	LCD_init();
	// Starting at position 1 on the LCD screen, writes Hello World
	state = Init;
	while(1)
	{
		ButtonA0 = ~PINA & 0x01;
		ButtonA1 = ~PINA & 0x02;
		ButtonTick();

		while (!TimerFlag);	// Wait 1 sec
		TimerFlag = 0;
	}
}
