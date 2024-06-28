#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// celcuis faranheit indicate
#define F_CPU 16000000UL

#define DEC_PORT PORTC
#define DEC_DDR DDRC

#define CTRL_PORT PORTB
#define CTRL_DDR DDRB

//CONTROLL SSD
#define s0 PB4
#define s1 PB5
#define s2 PB6
#define s3 PB7

//DECODER

#define ssd1 PC0
#define ssd2 PC1
#define ssd3 PC2
#define ssd4 PC3


#define button1 PC4     //PD0 // Button connected to PD0
#define button2 PC5    //PD1 // Button connected to PD1
#define button3 PC6 // Button connected to PD

#define ledx PD6


// Constants for temperature range thresholds
#define COLD_THRESHOLD 15
#define NORMAL_THRESHOLD_LOW 16
#define NORMAL_THRESHOLD_HIGH 25
#define WARMTH_THRESHOLD_LOW 26
#define WARMTH_THRESHOLD_HIGH 35
#define HOT_THRESHOLD 36

// Pins for LEDs indicating temperature ranges
#define LED_COLD PB1
#define LED_NORMAL PB2
#define LED_WARMTH PB3
#define LED_HOT PD5



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
	if (!(PINC & (1 << button)))
	{
		_delay_ms(200); // Debounce delay
		if (!(PINC & (1 << button)))
		{
			return 1; // Button pressed
		}
	}
	return 0; // Button not pressed
}

void setup()
{
	// Set button pins as input with pull-up resistors
	DDRC &= ~(1 << button1) & ~(1 << button2) & ~(1 << button3);
	PORTC |= (1 << button1) | (1 << button2) | (1 << button3);

	// Set control and decoder pins as output
	DEC_DDR |= (1 << ssd1) | (1 << ssd2) | (1 << ssd3) | (1 << ssd4);
	CTRL_DDR |= (1 << s0) | (1 << s1) | (1 << s2)| (1 << s3);
	
	//set ledx as output
	DDRD |= (1 << ledx)| (1 << LED_HOT);
	
	//set range led as output
	DDRB |= (1 << LED_COLD) | (1 << LED_NORMAL) | (1 << LED_WARMTH);

	
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



volatile uint8_t alarmBlinkFlag = 0; // Flag to indicate LED blinking state



void setupTimer() {
	// Configure Timer1 for 300 ms interval
	TCCR1B |= (1 << WGM12) | (1 << CS12); // CTC mode, Prescaler 256
	OCR1A = 3750; // Timer1 count for 300 ms (16000000 / 256 / 3 - 1)
	TIMSK1 |= (1 << OCIE1A); // Enable Timer1 compare match A interrupt

}


ISR(TIMER1_COMPA_vect) {

	alarmBlinkFlag = !alarmBlinkFlag; // Toggle the blink flag
	
}

void alarmReset() {
	if (alarmBlinkFlag) {
		PORTD |= (1 << ledx); // Turn on the LED
		} else {
		PORTD &= ~(1 << ledx); // Turn off the LED
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

		

	}
	return 0;
}


