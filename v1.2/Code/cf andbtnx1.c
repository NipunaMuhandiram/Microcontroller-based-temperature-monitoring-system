#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// celcuis faranheit indicate
#define F_CPU 16000000UL

#define DEC_PORT PORTB
#define DEC_DDR DDRB

#define CTRL_PORT PORTD
#define CTRL_DDR DDRD

#define s0 PD5
#define s1 PD6
#define s2 PD7

#define ssd1 PB0
#define ssd2 PB1
#define ssd3 PB2
#define ssd4 PB3

#define button1 PD0 // Button connected to PD0
#define button2 PD1 // Button connected to PD1
#define button3 PD4 // Button connected to PD

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
	0b00001001  // 9
};

const uint8_t cf[2] = {
	0b00001100,  // C
	0b00001111  // F
};

volatile uint8_t count = 0;
volatile uint8_t temp_unit = 0; // 0 for Celsius, 1 for Fahrenheit

ISR(INT0_vect)
{
	_delay_ms(50); // Debounce delay
	if (bit_is_clear(PIND, PD2))
	{
		count++;
		if (count >= 100)
		count = 0;
	}
}

ISR(INT1_vect)
{
	_delay_ms(50); // Debounce delay
	if (bit_is_clear(PIND, PD3))
	{
		if (count > 0)
		count--;
		else
		count = 99;
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
	CTRL_DDR |= (1 << s0) | (1 << s1) | (1 << s2);
}









int main(void)
{

	setup();
	setupInterrupt();

	while (1)
	{
		
		// Check if any button is pressed
		if (readButton(button1))
		{
			temp_unit = !temp_unit; // Toggle temperature unit
		}
		else if (readButton(button2))
		{
			// Your code for button connected to PD1
		}
		else if (readButton(button3))
		{
			// Your code for button connected to PD4
		}
		
		CTRL_PORT = (1 << s0);
		DEC_PORT = ssdnums[count / 10];
		_delay_ms(5);

		CTRL_PORT = (1 << s1);
		DEC_PORT = ssdnums[count % 10];
		_delay_ms(5);
		
		CTRL_PORT = (1 << s2);
		DEC_PORT = cf[temp_unit];
		_delay_ms(5);
	}
	return 0;
}
