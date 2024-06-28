#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL
// increment using interrupts
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

volatile uint8_t count = 0;

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

void setupInterrupt()
{
	// Configure INT0 (PD2) for falling edge triggered external interrupt
	EICRA |= (1 << ISC01); // Falling edge triggers INT0
	EIMSK |= (1 << INT0);  // Enable INT0
	sei();                 // Enable global interrupts
}

int main(void)
{
	DEC_DDR |= (1 << ssd1) | (1 << ssd2) | (1 << ssd3) | (1 << ssd4);
	CTRL_DDR |= (1 << s0) | (1 << s1) | (1 << s2);

	setupInterrupt();

	while (1)
	{
		CTRL_PORT = (1 << s0);
		DEC_PORT = ssdnums[count / 10];
		_delay_ms(5);

		CTRL_PORT = (1 << s1);
		DEC_PORT = ssdnums[count % 10];
		_delay_ms(5);
	}
	return 0;
}
