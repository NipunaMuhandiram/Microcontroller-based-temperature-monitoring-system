#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// celcuis faranheit indicate
#define F_CPU 16000000UL

#define DEC_PORT PORTB
#define DEC_DDR DDRB

#define CTRL_PORT PORTC
#define CTRL_DDR DDRC

//CONTROLL SSD
#define s0 PC0
#define s1 PC1
#define s2 PC2
#define s3 PC3
#define s4 PC4

#define ssd1 PB0
#define ssd2 PB1
#define ssd3 PB2
#define ssd4 PB3

#define button1 PD0 // Button connected to PD0
#define button2 PD1 // Button connected to PD1
#define button3 PD4 // Button connected to PD

#define ledx PD6


// Constants for temperature range thresholds
#define COLD_THRESHOLD 15
#define NORMAL_THRESHOLD_LOW 16
#define NORMAL_THRESHOLD_HIGH 25
#define WARMTH_THRESHOLD_LOW 26
#define WARMTH_THRESHOLD_HIGH 35
#define HOT_THRESHOLD 36

// Pins for LEDs indicating temperature ranges
#define LED_COLD PB4
#define LED_NORMAL PB5
#define LED_WARMTH PB6
#define LED_HOT PB7



const uint8_t ssdnums[10] = {
	0b00000000, // 0
	0b00000001, // 1
	0b00000010, // 2
	0b00000011, // 3
	0b00000100, // 4
	0b00000101, // 5
	0b00000110, // 6
	0b00000111, // 7
	0b00001000, // 8
	0b00001001,  // 9

};

const uint8_t cf[2] = {
	0b00001100,  // C
	0b00001111  // F
};

volatile uint8_t count = 0;
volatile uint8_t temp_unit = 0; // 0 for Celsius, 1 for Fahrenheit
volatile uint8_t overflow = 0; //0 for not overflow 1 for overflow


ISR(INT0_vect)
{
	_delay_ms(50); // Debounce delay
	if (bit_is_clear(PIND, PD2))
	{
		count++;
		if (count>40)
		{
			overflow=1;
		}
		if (count >= 250)
		count = 0;
	}
}

ISR(INT1_vect)
{
	_delay_ms(50); // Debounce delay
	if (bit_is_clear(PIND, PD3))
	{

		if (count > 0){		
		count--;	
		}		
		else{
			count = 250;
			overflow=1;
		}
		
	}
}

void setupInterrupt()
{
	// Configure INT0 (PD2) for falling edge triggered external interrupt
	EICRA |= (1 << ISC01); // Falling edge triggers INT0
	EIMSK |= (1 << INT0);  // Enable INT0

	// Configure INT1 (PD3) for falling edge triggered external interrupt
	EICRA |= (1 << ISC11); // Falling edge triggers INT1
	EIMSK |= (1 << INT1);  // Enable INT1

	sei(); // Enable global interrupts
}
// Function to read button state
uint8_t readButton(uint8_t button)
{
	if (!(PIND & (1 << button)))
	{
		_delay_ms(200); // Debounce delay
		if (!(PIND & (1 << button)))
		{
			return 1; // Button pressed
		}
	}
	return 0; // Button not pressed
}

void setup()
{
	// Set button pins as input with pull-up resistors
	DDRD &= ~(1 << button1) & ~(1 << button2) & ~(1 << button3);
	PORTD |= (1 << button1) | (1 << button2) | (1 << button3);

	// Set control and decoder pins as output
	DEC_DDR |= (1 << ssd1) | (1 << ssd2) | (1 << ssd3) | (1 << ssd4);
	CTRL_DDR |= (1 << s0) | (1 << s1) | (1 << s2)| (1 << s3);
	
	//set ledx as output
	DDRD |= (1 << ledx);
	
	//set range led as output
	DDRB |= (1 << LED_COLD) | (1 << LED_NORMAL) | (1 << LED_WARMTH) | (1 << LED_HOT);

	
}


void displayCount(uint8_t count)
{
	// Display thousands digit (if count >= 100)
	CTRL_PORT = (1 << s0);
	DEC_PORT = (count >= 100) ? ssdnums[count / 100] : 0; // Display thousands digit if count is greater than or equal to 100
	_delay_ms(5);

	// Display hundreds digit (if count >= 10)
	CTRL_PORT = (1 << s1);
	DEC_PORT = ((count >= 10) || (count >= 100)) ? ssdnums[(count / 10) % 10] : 0; // Display hundreds digit if count is greater than or equal to 10
	_delay_ms(5);

	// Display tens digit
	CTRL_PORT = (1 << s2);
	DEC_PORT = ssdnums[count % 10]; // Display ones digit
	_delay_ms(5);
}

void displayTempUnit(uint8_t temp_unit)
{
	CTRL_PORT = (1 << s3);
	DEC_PORT = cf[temp_unit];
	_delay_ms(5);
}
float celsius_to_fahrenheit(int celsius) {
	return (celsius * 9 / 5) + 32;
}
/*
void alarmReset(){
	PORTC ^= (1 << ledx); // Toggle the LED state
	//PORTC|= (1<< ledx);
	_delay_ms(1000);
}*/


volatile uint8_t alarmBlinkFlag = 0; // Flag to indicate LED blinking state

/*
void setupTimer() {
	// Configure Timer1 for 1 Hz (500 ms) blinking
	TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
	OCR1A = 15624; // Timer1 count for 500 ms (16000000 / 1024 / 2 - 1)
	TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare match A interrupt
}*/
/*
void setupTimer() {
	// Configure Timer1 for 50 ms interval
	TCCR1B |= (1 << WGM12) | (1 << CS12); // CTC mode, Prescaler 256
	OCR1A = 625; // Timer1 count for 50 ms (16000000 / 256 / 20 - 1)
	TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare match A interrupt
}
*/

/*
void setupTimer() {
	// Configure Timer1 for 100 ms interval
	TCCR1B |= (1 << WGM12) | (1 << CS12); // CTC mode, Prescaler 256
	OCR1A = 1250; // Timer1 count for 100 ms (16000000 / 256 / 10 - 1)
	TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare match A interrupt
}*/

void setupTimer() {
	// Configure Timer1 for 300 ms interval
	TCCR1B |= (1 << WGM12) | (1 << CS12); // CTC mode, Prescaler 256
	OCR1A = 3750; // Timer1 count for 300 ms (16000000 / 256 / 3 - 1)
	TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare match A interrupt
	
	TCCR1B |= (1 << WGM12) | (1 << CS12); // CTC mode, Prescaler 256
	OCR1A = 3750;                          // Timer1 count for 300 ms (16000000 / 256 / 3 - 1)
	TIMSK1 |= (1 << OCIE1A);               // Enable Timer1 compare match A interrupt

}


ISR(TIMER1_COMPA_vect) {

	alarmBlinkFlag = !alarmBlinkFlag; // Toggle the blink flag
	updateTemperatureIndication(count);
}

void alarmReset() {
	if (alarmBlinkFlag) {
		PORTD |= (1 << ledx); // Turn on the LED
		} else {
		PORTD &= ~(1 << ledx); // Turn off the LED
	}
}

void updateTemperatureIndication(uint8_t temperature)
{
	PORTB &= ~((1 << LED_COLD) | (1 << LED_NORMAL) | (1 << LED_WARMTH) | (1 << LED_HOT));

	if (temperature < COLD_THRESHOLD)
	{
		PORTB |= (1 << LED_COLD);
		// Increase brightness of LED 1
		for (int i = 0; i < 10; i++)
		{
			PORTB ^= (1 << LED_COLD);
			_delay_ms(10); // Adjust delay for desired brightness level
		}
	}
	else if (temperature >= NORMAL_THRESHOLD_LOW && temperature <= NORMAL_THRESHOLD_HIGH)
	{
		PORTB |= (1 << LED_NORMAL);
		// Increase brightness of LED 2
		for (int i = 0; i < 10; i++)
		{
			PORTB ^= (1 << LED_NORMAL);
			_delay_ms(10); // Adjust delay for desired brightness level
		}
	}
	else if (temperature >= WARMTH_THRESHOLD_LOW && temperature <= WARMTH_THRESHOLD_HIGH)
	{
		PORTB |= (1 << LED_WARMTH);
		// Increase brightness of LED 3
		for (int i = 0; i < 10; i++)
		{
			PORTB ^= (1 << LED_WARMTH);
			_delay_ms(1); // Adjust delay for desired brightness level
		}
	}
	else if (temperature >= HOT_THRESHOLD && temperature < 40)
	{
		PORTB |= (1 << LED_HOT);
		// Increase brightness of LED 4
		for (int i = 0; i < 10; i++)
		{
			PORTB ^= (1 << LED_HOT);
			_delay_ms(1); // Adjust delay for desired brightness level
		}
	}
	else if (temperature >= 40)
	{
		// Blink alarm LED
		alarmReset();
	}
}








int main(void)
{


	setup();
	setupInterrupt();
	setupTimer();
	// Enable global interrupts
	sei();



	while (1)
	{
		
		// Check if any button is pressed
		if (readButton(button1))
		{
			temp_unit = !temp_unit; // Toggle temperature unit
		}
		if (readButton(button2))
		{
			if (overflow==1)
			{
				count=0;
			}
			overflow = 0; // Toggle temperature unit
			alarmBlinkFlag =0;
			PORTD &= ~(1 << ledx); // Turn off the LED
		}
		
	
		if (temp_unit==0){
			// Display count and temperature unit celcius
			if (overflow==1)
			{
				alarmReset();
				displayCount(count);
				displayTempUnit(temp_unit);
			}

			displayCount(count);
			displayTempUnit(temp_unit);
			
		}		
		else{
			if (overflow==1)
			{
				alarmReset();
				displayCount(celsius_to_fahrenheit(count));
				displayTempUnit(temp_unit);
			}
			displayCount(celsius_to_fahrenheit(count));
			displayTempUnit(temp_unit);
		}

			// Update temperature indication LEDs
			//updateTemperatureIndication(count);
		

	}
	return 0;
}

