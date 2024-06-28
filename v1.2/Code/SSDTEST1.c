#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

#define DEC_PORT PORTB
#define DEC_DDR DDRB

#define CTRL_PORT PORTD // Change to PORTC or any other port
#define CTRL_DDR DDRD    // Change to DDRC or any other port

#define s0 PD5 // Change to corresponding pins in the new control port
#define s1 PD6 // Change to corresponding pins in the new control port
#define s2 PD7 // Change to corresponding pins in the new control port

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
	0b00001001 // 9
};

int digits[3] = {0, 0, 0};

int main(void)
{
	DEC_DDR |= (1 << ssd1) | (1 << ssd2) | (1 << ssd3) | (1 << ssd4); // Set PORTB as output
	CTRL_DDR |= (1 << s0) | (1 << s1) | (1 << s2);                    // Set corresponding pins in PORTC as output

	while (1)
	{
		CTRL_PORT = (1 << s0);
		DEC_PORT = ssdnums[6];
		_delay_ms(5);
		
		CTRL_PORT = (1 << s1);
		DEC_PORT = ssdnums[7];
		_delay_ms(5);
		
		CTRL_PORT = (1 << s2);
		DEC_PORT = ssdnums[8];
		_delay_ms(5);
	}
	return 0;
}
