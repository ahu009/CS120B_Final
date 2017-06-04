#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

unsigned char HangGame = 0;
unsigned char ButtonState;
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char bladeX = 1, bladeY = 1;
unsigned char score;
uint16_t  x = 0;
uint16_t y = 0;

typedef struct fruit {
	unsigned char x1, y1, x2, y2;
	unsigned char pattern;
	unsigned char speed;	
} fruit;

typedef struct bomb {
	unsigned char x1, y1, x2, y2;
	unsigned char pattern;
	unsigned char speed;
} bomb;

fruit fruits[7];
bomb bombs[7];

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

void UnlightLED()
{
	transmit_data(0x00);
	transmit_data_blue(0xFF);
	return;
}

void LightLED(unsigned char x, unsigned char y)
{
	unsigned char column_val = 0x80;
	unsigned char column_sel = 0x7F;
	
	for(int i = 1; i < x; i++){
		column_sel = (column_sel >> 1) | 0x80;
	}
	for(int i = 1; i < y; i++){
		column_val = column_val >> 1;
	}
	
	transmit_data(column_val);
	transmit_data_blue(column_sel);
	return;
}


void LightBlock(unsigned char x, unsigned char y){
	unsigned char column_val = 0xC0;
	unsigned char column_sel = 0x3F;
	
	for(int i = 1; i < x; i++){
		column_sel = (column_sel >> 1) | 0x80;
	}
	for(int i = 1; i < y; i++){
		column_val = column_val >> 1;
	}
	
	transmit_data(column_val);
	transmit_data_blue(column_sel);
	return;
}

enum FruitStates{NoFruit, CreateFruit, Wait, Lose} FruitState;
void FruitTick(){
	const unsigned char* string1 = reinterpret_cast<const unsigned char *>("FruitCreated");
	const unsigned char* string2 = reinterpret_cast<const unsigned char *>("???");
	
	switch(FruitState){
		case NoFruit:
			FruitState = CreateFruit;
			break;
		case CreateFruit:
			
			break;
		case Wait:
			break;
		case Lose:
			break;
	}
	
	switch(FruitState){
		case NoFruit:
			break;
		case CreateFruit:
			break;
		case Wait:
			break;
		case Lose:
			break;
	}
};

enum GameStates{WaitStart, ButtonDown1, ButtonDown2, Gameon} gamestate;
void StartTick(){
	const unsigned char* string1 = reinterpret_cast<const unsigned char *>("WaitStart");
	const unsigned char* string2 = reinterpret_cast<const unsigned char *>("GameOn");
	
	ButtonState = ~PIND & 0x10;
	
	switch(gamestate){
		case WaitStart:
			if(ButtonState)
				gamestate = ButtonDown1;
			else
				gamestate = WaitStart;
			break;
		case ButtonDown1:
			if(ButtonState)
				gamestate = ButtonDown1;
			else
				gamestate = Gameon;
			break;
		case Gameon:
			if(ButtonState)
				gamestate = ButtonDown2;
			else
				gamestate = Gameon;
			break;
		case ButtonDown2:
			if(ButtonState)
				gamestate = ButtonDown2;
			else
				gamestate = WaitStart;
			break;
	}
	
	switch(gamestate){
		case WaitStart:
			//Add Reset Shit as you go
			LCD_ClearScreen();
			LCD_DisplayString(1, string1);
			HangGame = 1;
			UnlightLED();
			bladeX = 1;
			bladeY = 1;
			break;
		case Gameon:
			LCD_ClearScreen();
			HangGame = 0;
			LCD_DisplayString(1, string2);
			break;
		case ButtonDown1:
			break;
		case ButtonDown2:
			break;
	}
}

enum States{Init} state;
void JoystickTick()
{
	const unsigned char* string = reinterpret_cast<const unsigned char *>("Score: ");
	const unsigned char* string1 = reinterpret_cast<const unsigned char *>("Right");
	const unsigned char* string2 = reinterpret_cast<const unsigned char *>("Left");
	const unsigned char* string3 = reinterpret_cast<const unsigned char *>("Up");
	const unsigned char* string4 = reinterpret_cast<const unsigned char *>("Down");

	static unsigned char score = 0;
	
	switch(state)
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
		x = readadc(4);
		y = readadc(5);
		if(x > 800) {
			//LCD_DisplayString(1, string1);
			if(bladeX < 8) bladeX += 1;
		}
		else if (x < 200){
			//LCD_DisplayString(1, string2);
			if(bladeX > 1) bladeX -= 1;
		}
		else{
			//LCD_ClearScreen();
		}
		
		if(y > 800) {
			//LCD_DisplayString(1, string4);
			if(bladeY > 1 )bladeY -= 1;
		}
		else if (y < 200){
			//LCD_DisplayString(1, string3);
			if(bladeY < 8) bladeY += 1;
		}
		
		//LCD_Cursor(1);
		//LCD_DisplayString(1, string);
		//LCD_WriteData('0' + ((score % 1000) / 100));
		//LCD_WriteData('0' + ((score % 100) / 10));
		//LCD_WriteData('0' + ((score % 10)));
		set_PWM(329.63);
		
		break;
	}
}

enum displayStates{Init1} displayState;
void DisplayTick(){
	unsigned const char numDisplay = 3;
	static unsigned char i = 0;
	
	switch(displayState){
		case Init1:
		displayState = Init1;
		break;
	}
	
	switch(displayState){
		case Init1:
		
		if(i == 0){
			UnlightLED();
			LightLED(bladeX, bladeY);
		}
		else if(i == 1){
			UnlightLED();
			LightLED(8,8);
		}
		else if(i == 2){
			//UnlightLED();
			//LightBlock(2,8);
		}
		i++;
		if(i >= numDisplay) i = 0;
		break;
	}
}


int main(void)
{
	DDRB = 0xFF; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xEF; PORTD = 0x10;
	DDRA = 0x0F; PORTA = 0xF0;
	
	unsigned char systemPeriod = 1;
	
	InitADC();
	TimerSet(systemPeriod);
	TimerOn();
	PWM_on();
	LCD_init();
	
	state = Init;
	displayState = Init1;
	gamestate = WaitStart;
	FruitState = NoFruit;
	
	unsigned long elapsedTime1 = 1;
	unsigned long elapsedTime2 = 100;
	unsigned long elapsedTime3 = 150;
	unsigned long elapsedTime4 = 150;
	
	while(1)
	{
		if(elapsedTime3 >= 150){
			StartTick();
			elapsedTime3 = 0;
		}
		
		if(!HangGame){
			if(elapsedTime1 >= 1){
				DisplayTick();
				elapsedTime1 = 0;
			}
			if(elapsedTime2 >= 100){
				JoystickTick();
				elapsedTime2 = 0;
			}
			if(elapsedTime4 >= 150){
				FruitTick();
				elapsedTime4 = 0;
			}
		}
		
		while (!TimerFlag);
		TimerFlag = 0;
		
		elapsedTime1 = elapsedTime1 + systemPeriod;
		elapsedTime2 = elapsedTime2 + systemPeriod;
		elapsedTime3 = elapsedTime3 + systemPeriod;
		elapsedTime4 = elapsedTime4 + systemPeriod;
	}
}
