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

void transmit_data_red(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTD = 0x08;
		// set SER = next bit of data to be sent.
		PORTD |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTD |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTD |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTD = 0x00;
}

void transmit_data_blue(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTA = 0x08;
		// set SER = next bit of data to be sent.
		PORTA |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTA |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTA |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTA = 0x00;
}

void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x08;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTB |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}

enum SM1_States {sm1_display, display2} state ;
void SM1_Tick() {
	static unsigned char column_val = 0x01; // sets the pattern displayed on columns
	static unsigned char column_sel = 0x7F; // grounds column to display pattern
	

	switch (state) {
		case sm1_display:
		state = sm1_display;
		break;
		case display2:
		state = sm1_display;
		break;
		default:
		state = sm1_display;
		break;
	}
	
	switch (state) {
		case sm1_display: // If illuminated LED in bottom right corner
		if ((column_sel == 0xFE && column_val == 0x80)/* || (column_sel == 0xFF)*/) {
			column_sel = 0x7F; // display far left column
			column_val = 0x01; // pattern illuminates top row
		}
		// else if far right column was last to display (grounded)
		else if (column_sel == 0xFE) {
			column_sel = 0x7F; // resets display column to far left column
			column_val = column_val << 1; // shift down illuminated LED one row
		}
		// else Shift displayed column one to the right
		else {
			column_sel = (column_sel >> 1) | 0x80;
		}
		//PORTB = column_sel;
		transmit_data(column_val); // PORTB selects column to display pattern
		transmit_data_blue(column_sel); // PORTA displays column pattern
		break;
		
		// 		case display2:
		// 		if ((column_sel == 0xFE && column_val == 0x80) || (column_sel == 0xFF)) {
		// 			column_sel = 0x7F; // display far left column
		// 			column_val = 0x01; // pattern illuminates top row
		// 		}
		// 		else{
		// 			column_val = column_val << 1;
		// 		}
		// 		PORTA = column_val; // PORTA displays column pattern
		// 		transmit_data(column_sel); // PORTB selects column to display pattern
		// 		break;
		default:
		break;
	}
	
	
};



int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	DDRA = 0xFF; PORTA = 0x00; //Input Analog
	
	//InitADC();
	
	TimerSet(100);
	TimerOn();
	//PWM_on();


	//LCD_init();
	state = sm1_display;
	
	while(1)
	{
		SM1_Tick();

		while (!TimerFlag);
		TimerFlag = 0;
	}
}
