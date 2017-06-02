
#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1; 
unsigned long _avr_timer_cntcurr = 0; 
unsigned char column_val = 0x80;
unsigned char column_sel = 0x7F;

//Code Taken From (Driver for Joystick):
//http://www.embedds.com/interfacing-analog-joystick-with-avr/
void InitADC(void)
{
	ADMUX|=(1<<REFS0);
	ADCSRA|=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); 
}
uint16_t readadc(uint8_t ch)
{
	ch&=0b00000111;         
	ADMUX = (ADMUX & 0xf8)|ch;  
	ADCSRA|=(1<<ADSC);        
	while((ADCSRA)&(1<<ADSC));    
	return(ADC);    
}
//Driver for Joystick Ends Here

//Shift Register Transmit Data Functions Taken from Supplemental Lab (Modified)
void transmit_data_red(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		PORTD = 0x08;
		PORTD |= ((data >> i) & 0x01);
		PORTD |= 0x02;
	}
	PORTD |= 0x04;
	PORTD = 0x00;
}

void transmit_data_blue(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		PORTA = 0x08;

		PORTA |= ((data >> i) & 0x01);

		PORTA |= 0x02;
	}

	PORTA |= 0x04;
	PORTA = 0x00;
}

void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		PORTB = 0x08;
	
		PORTB |= ((data >> i) & 0x01);

		PORTB |= 0x02;
	}

	PORTB |= 0x04;
	
	PORTB = 0x00;
}
//Shift register transmit data functions end here

//Buzzer Drivers From Lab 8
void set_PWM(double frequency) {

	static double current_frequency;
	if (frequency != current_frequency) {

		if (!frequency) TCCR3B &= 0x08; 
		else TCCR3B |= 0x03; 
		
		if (frequency < 0.954) OCR3A = 0xFFFF;
		
		else if (frequency > 31250) OCR3A = 0x0000;
		
		else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}
//Buzzer Driver Ends here

void TimerOn() {
	TCCR1B = 0x0B;
	
	OCR1A = 125;	

	TIMSK1 = 0x02;

	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80; 
}

void TimerOff() {
	TCCR1B = 0x00; 
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--; 
	if (_avr_timer_cntcurr == 0) { 
		TimerISR(); 
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void LightLED(unsigned char x, unsigned char y)
{
	unsigned char temp_y = 0x80; 
	unsigned char temp_x = 0x7F; 
	
	for(int i = 1; i < x; i++){
		temp_x = (temp_x >> 1) | 0x80;
	}
	for(int i = 1; i < y; i++){
		temp_y = temp_y >> 1;
	}
	
	column_sel = column_sel & temp_x;
	column_val = column_val | temp_y;
	transmit_data(column_val); 
	transmit_data_blue(column_sel); 
	return;
}

void UnlightLED(unsigned char x, unsigned char y)
{
	unsigned char temp_y = 0x7F; 
	unsigned char temp_x = 0x80; 
	
	for(int i = 1; i < x; i++){
		temp_x = (temp_x >> 1);
	}
	for(int i = 1; i < y; i++){
		temp_y = (temp_y >> 1) | 0x80;
	}
	
	column_sel = column_sel | temp_x;
	column_val = column_val & temp_y;
	transmit_data(column_val); // PORTB selects column to display pattern
	transmit_data_blue(column_sel); // PORTA displays column pattern
	return;
}

unsigned char ButtonA1;
unsigned char ButtonA0;
unsigned char cnt;
unsigned char period = 1;
unsigned char score;
uint16_t  x = 0;
uint16_t y = 0;

enum States{Init, ReadYAnalogInput, ReadXAnalogInput} state;

void ButtonTick()
{
	const unsigned char* string = reinterpret_cast<const unsigned char *>("Score: ");
	const unsigned char* string1 = reinterpret_cast<const unsigned char *>("Right");
	const unsigned char* string2 = reinterpret_cast<const unsigned char *>("Left");
	const unsigned char* string3 = reinterpret_cast<const unsigned char *>("Up");
	const unsigned char* string4 = reinterpret_cast<const unsigned char *>("Down");

	static unsigned char score = 0;
	static unsigned char x_pos = 1;
	static unsigned char y_pos = 1;
	
	switch(state) //Transition 
	{
		case Init:
			state = Init;
			break;
		
		default:
			break;
	}

	switch(state)
	{
		case Init:
			//LightLED(8,8);
			x = readadc(4);
			y = readadc(5);
			//LightLED(x_pos,y_pos);
			//LightLED(7,7);
			if(x > 800) {
				LCD_DisplayString(1, string1);
				UnlightLED(x_pos, y_pos);
				x_pos = x_pos + 1;
				LightLED(x_pos, y_pos);
				//column_sel = (column_sel >> 1) | 0x80;
			}
			else if (x < 200){
				LCD_DisplayString(1, string2);
				UnlightLED(x_pos, y_pos);
				x_pos = x_pos - 1;
				LightLED(x_pos, y_pos);
				//column_sel = (column_sel << 1) | 0x01;
			}
			else{
				LCD_ClearScreen();
			}
			
			if(y > 800) {
				LCD_DisplayString(1, string4);
				UnlightLED(x_pos, y_pos);
				y_pos = y_pos - 1;
				LightLED(x_pos, y_pos);
				//column_val = column_val << 1;
			}
			else if (y < 200){
				LCD_DisplayString(1, string3);
				UnlightLED(x_pos, y_pos);
				y_pos = y_pos + 1;
				LightLED(x_pos, y_pos);
				//column_val = column_val >> 1;
			}

			//LCD_Cursor(1);
		    //LCD_DisplayString(1, string);
			//LCD_WriteData('0' + ((score % 1000) / 100));
			//LCD_WriteData('0' + ((score % 100) / 10));
			//LCD_WriteData('0' + ((score % 10)));
			set_PWM(329.63);
			break;
	}
	
	LightLED(x_pos, y_pos);
	//LightLED(8, 8);
	//transmit_data(column_val); // PORTB selects column to display pattern
	//transmit_data_blue(column_sel); // PORTA displays column pattern
}


int main(void)
{
	DDRB = 0xFF; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	DDRA = 0x0F; PORTA = 0xF0; //Input Analog
	
	InitADC();
	
	TimerSet(150);
	TimerOn();
	PWM_on();


	LCD_init();

	state = Init;
	while(1)
	{
		//ButtonA0 = ~PINA & 0x01;
		//ButtonA1 = ~PINA & 0x02;
		ButtonTick();
		
		while (!TimerFlag);
		TimerFlag = 0;
	}
}
