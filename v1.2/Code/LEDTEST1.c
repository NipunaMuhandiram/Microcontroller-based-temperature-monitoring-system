/*
 * Design Project.c
 *
 * Created: 5/13/2024 11:44:05 AM
 * Author : Nipuna Muhandiram
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define RLED1 PC0
#define RLED2 PC1
#define RLED3 PC2
#define RLED4 PC3
#define ALRMLED5 PC4

#define BUTTON_01_TEMPD PD2
#define BUTTON_02_TEMPI PD3
#define BUTTON_03_MODE PD0
#define BUTTON_04_CF PD1
#define BUTTON_05_ALRM PD5

int main(void)
{
    DDRC |=(1 << RLED1)|(1 << RLED2)|(1 << RLED3)|(1 << RLED4)|(1 << ALRMLED5); // Set LED pin (PC0) as output
    DDRD &=~(1 << BUTTON_01_TEMPD); // Set Button (PD2) as input
	
	
	
    while (1) 
    {
		
		if (!(PIND &(1<< BUTTON_01_TEMPD))) { // check if the push button is pressed
			PORTC|= (1<< RLED1); //turn on led that connect to C0 pin
			_delay_ms(1000); //changing the time the bulb is on
		}
		else {
			PORTC &=~(1<< RLED1); //turn off led that connect to D0 pin
		}
		_delay_ms(10); //delay for push button
		
		
		
    }
}