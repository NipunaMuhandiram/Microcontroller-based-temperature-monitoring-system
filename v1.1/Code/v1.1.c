//by nipuna Sandaruwan
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// celcuis faranheit indicate
#define F_CPU 16000000UL

volatile uint8_t count = 0;
volatile uint8_t temp_unit = 0; // 0 for Celsius, 1 for Fahrenheit
volatile uint8_t overflow = 0; //0 for not overflow 1 for overflow
volatile uint8_t mode = 1; //1 for mode 1( on the 7-segment display) and 0 for mode 2 (using the LEDs.)
int tmpval;


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
#define l1 PB1
#define l2 PB2
#define l3 PB3
#define l4 PD5



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

ISR(INT0_vect)
{
	_delay_ms(50); // Debounce delay
	if (bit_is_clear(PIND, PD2))
	{
		count++;
		if (count>=41)
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
		if (count <= 40)
		{
			overflow = 0; // Reset flag to stop LED blinking
			PORTD &= ~(1 << ledx); // Turn off the LED
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
	DDRD |= (1 << ledx);
	


	
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

// ============================================================================

void PWM_init() {
	// Set OC1A, OC1B (PB1, PB2) as output pins
	DDRB |= (1 << l1) | (1 << l2) | (1 << l3);
	// Set OC0B (PD5) as output pin
	DDRD |= (1 << l4);

	// Set Timer/Counter2 in Fast PWM mode with non-inverted output
	TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
	// Set Timer/Counter2 prescaler to 64
	TCCR2B |= (1 << CS22);

	// Timer1 Fast PWM Mode, 8-bit
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
	TCCR1B |= (1 << CS10) | (1 << WGM12); // Prescaler 1, Mode 7 (Fast PWM, 8-bit)

	// Timer0 Fast PWM Mode, 8-bit
	TCCR0A |= (1 << COM0B1) | (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS00); // No prescaling
}

void setBrightness(uint8_t brightness1, uint8_t brightness2, uint8_t brightness3, uint8_t brightness4) {
	OCR1A = brightness1; // Set brightness for l1
	OCR1B = brightness2; // Set brightness for l2
	OCR0B = brightness4; // Set brightness for l4
	OCR2A = brightness3; // Set brightness for l3
}

void brightness(){
	if (count >= 0 && count <= 15)
	{
		
		setBrightness(count*17, 0, 0, 0); // Set brightness for LED 1 to maximum
	}
	else if (count >= 16 && count <= 25)
	{
		// 255/(26°C-16°C) =25.5 // 25.5*16°C = 408 //408-25.5=382.5 //(count°C*25.5)-383
		tmpval = (count*25)-383;
		setBrightness(255, tmpval, 0, 0); // Set brightness for LED 2 to maximum
	}
	else if (count >= 26 && count <= 35)
	{
		tmpval=(count*25)-638;
		setBrightness(255, 255, tmpval, 0); // Set brightness for LED 3 to maximum
	}
	else if (count > 35 && count <= 40)
	{
		tmpval=(count*42)-1445;
		setBrightness(255, 255, 255, tmpval); // Set brightness for LED 4 to maximum
	}
}

// ===========

int main(void)
{

	PWM_init();
	setup();
	setupInterrupt();
	setupTimer();
	// Enable global interrupts
	sei();


	// Initialize LED brightness based on the current mode
	if (mode == 1) {
		// Mode 1: Turn off LED l1
		setBrightness(0, 0, 0, 0);
		} else {
		// Mode 2: Clear the 7-segment display
		CTRL_PORT = 0x00; // Clear all control lines
		DEC_PORT = 0x00; // Clear all decoder lines
		// Mode 2: Adjust brightness according to temperature
		brightness();
	}




	while (1)
	{
		if (count < 41)
		{
			alarmBlinkFlag = 0; // Reset flag to stop LED blinking
			PORTD &= ~(1 << ledx); // Turn off the LED
		}
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
		if (readButton(button3))
		{
			mode = !mode; // Toggle temperature unit
			// Initialize LED brightness based on the current mode
			if (mode == 1) {
				// Mode 1: Turn off LED l1
				setBrightness(0, 0, 0, 0);
				} else {
				// Mode 2: Clear the 7-segment display
				CTRL_PORT = 0x00; // Clear all control lines
				DEC_PORT = 0x00; // Clear all decoder lines
				// Mode 2: Adjust brightness according to temperature
				brightness();
			}
		}
		
		
		if (mode)
		{
			
			if (temp_unit==0){
				// Display count and temperature unit celcius
				if (overflow==1)
				{
					alarmReset();
					displayCount(count);
					displayTempUnit(temp_unit);
					brightness();
				}

				displayCount(count);
				displayTempUnit(temp_unit);
				brightness();
				
				
				
			}
			else{
				if (overflow==1)
				{
					alarmReset();
					displayCount(celsius_to_fahrenheit(count));
					displayTempUnit(temp_unit);
					brightness();
				}
				displayCount(celsius_to_fahrenheit(count));
				displayTempUnit(temp_unit);
				brightness();
			}
			
		}
		else{
			if (overflow==1)
			{
				alarmReset();
				brightness();
			}
			brightness();


		}
		



	}
	return 0;
}



