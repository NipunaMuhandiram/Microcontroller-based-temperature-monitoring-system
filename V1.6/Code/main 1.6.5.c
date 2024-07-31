#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 8000000UL  // Internal 8 MHz oscillator

volatile uint8_t count = 0;
volatile uint8_t temp_unit = 10; // 0 for Celsius, 1 for Fahrenheit
volatile uint8_t overflow = 0; // 0 for not overflow, 1 for overflow
volatile uint8_t mode = 1; // 1 for mode 1 (on the 7-segment display) and 0 for mode 2 (using the LEDs.)
int tmpval;


// Segment mappings for digits 0-9 and characters 'C' and 'F'
const uint8_t segment_map[12][7] = {
	{1, 1, 1, 1, 1, 1, 0}, // 0
	{0, 1, 1, 0, 0, 0, 0}, // 1
	{1, 1, 0, 1, 1, 0, 1}, // 2
	{1, 1, 1, 1, 0, 0, 1}, // 3
	{0, 1, 1, 0, 0, 1, 1}, // 4
	{1, 0, 1, 1, 0, 1, 1}, // 5
	{1, 0, 1, 1, 1, 1, 1}, // 6
	{1, 1, 1, 0, 0, 0, 0}, // 7
	{1, 1, 1, 1, 1, 1, 1}, // 8
	{1, 1, 1, 1, 0, 1, 1}, // 9
	{1, 0, 0, 1, 1, 1, 0}, // C
	{1, 0, 0, 0, 1, 1, 1}  // F
};

// const uint8_t segment_map[12][7] = {
// 	{0, 0, 0, 0, 0, 0, 1}, // 0
// 	{1, 0, 0, 1, 1, 1, 1}, // 1
// 	{0, 0, 1, 0, 0, 1, 0}, // 2
// 	{0, 0, 0, 0, 1, 1, 0}, // 3
// 	{1, 0, 0, 1, 1, 0, 0}, // 4
// 	{0, 1, 0, 0, 1, 0, 0}, // 5
// 	{0, 1, 0, 0, 0, 0, 0}, // 6
// 	{0, 0, 0, 1, 1, 1, 1}, // 7
// 	{0, 0, 0, 0, 0, 0, 0}, // 8
// 	{0, 0, 0, 0, 1, 0, 0}, // 9
// 	{0, 1, 1, 0, 0, 0, 1}, // C
// 	{0, 1, 1, 1, 0, 0, 0}  // F
// };

// //Segment mappings for digits 0-9 and characters 'C' and 'F'
// const uint8_t segment_map[12][7] = {
// 	{1, 1, 1, 1, 1, 1, 0}, // 0
// 	{0, 0, 0, 0, 0, 1, 0}, // 1
// 	{0, 0, 0, 0, 0, 1, 1}, // 2
// 	{0, 0, 0, 0, 1, 0, 0}, // 3
// 	{0, 0, 0, 0, 1, 0, 1}, // 4
// 	{0, 1, 0, 0, 1, 0, 0}, // 5
// 	{0, 1, 0, 0, 0, 0, 0}, // 6
// 	{0, 0, 0, 1, 1, 1, 1}, // 7
// 	{0, 0, 0, 0, 0, 0, 0}, // 8
// 	{0, 0, 0, 0, 1, 0, 0}, // 9
// 	{0, 1, 1, 0, 0, 0, 1}, // C
// 	{0, 1, 1, 1, 0, 0, 0}  // F
// };
// const uint8_t segment_map[10][7] = {
// 	{0, 0, 0, 0, 0, 0, 1}, // 0
// 	{1, 0, 0, 1, 1, 1, 1}, // 1
// 	{0, 0, 1, 0, 0, 1, 0}, // 2
// 	{0, 0, 0, 0, 1, 1, 0}, // 3
// 	{1, 0, 0, 1, 1, 0, 0}, // 4
// 	{0, 1, 0, 0, 1, 0, 0}, // 5
// 	{0, 1, 0, 0, 0, 0, 0}, // 6
// 	{0, 0, 0, 1, 1, 1, 1}, // 7
// 	{0, 0, 0, 0, 0, 0, 0}, // 8
// 	{0, 0, 0, 0, 1, 0, 0}  // 9
// };

// Pin definitions for segments
#define SEG_A PD0
#define SEG_B PD1
#define SEG_C PD4
#define SEG_D PC0
#define SEG_E PC1
#define SEG_F PC2
#define SEG_G PC3

// Control pins for switching SSDs
#define CTRL_PORT PORTB
#define CTRL_DDR DDRB
#define s0 PB4
#define s1 PB5
#define s2 PB6
#define s3 PB7

// Button pins
#define button1 PC4
#define button2 PC5
// #define button3 PC6
#define button3 PD7

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

ISR(INT0_vect)
{
	_delay_ms(50); // Debounce delay
	if (bit_is_clear(PIND, PD2))
	{
		count++;
		if (count >= 41)
		{
			overflow = 1;
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
		if (count > 0)
		{
			count--;
		}
		else
		{
			count = 100;
			overflow = 1;
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




void setup()
{
	// Set button pins as input with pull-up resistors
	DDRC &= ~(1 << button1) & ~(1 << button2);
	DDRD &= ~(1 << button3);
	PORTC |= (1 << button1) | (1 << button2);
	PORTD |= (1 << button3);

	// Set segment pins as output
	DDRD |= (1 << SEG_A) | (1 << SEG_B) | (1 << SEG_C);
	DDRC |= (1 << SEG_D) | (1 << SEG_E) | (1 << SEG_F) | (1 << SEG_G);

	// Set control pins as output
	CTRL_DDR |= (1 << s0) | (1 << s1) | (1 << s2) | (1 << s3);

	// Set ledx as output
	DDRD |= (1 << ledx);
}

void setSegments(uint8_t character_index)
{
	// Set segments based on the character index
	PORTD = (PORTD & ~((1 << SEG_A) | (1 << SEG_B) | (1 << SEG_C))) |
	(segment_map[character_index][0] << SEG_A) |
	(segment_map[character_index][1] << SEG_B) |
	(segment_map[character_index][2] << SEG_C);

	PORTC = (PORTC & ~((1 << SEG_D) | (1 << SEG_E) | (1 << SEG_F) | (1 << SEG_G))) |
	(segment_map[character_index][3] << SEG_D) |
	(segment_map[character_index][4] << SEG_E) |
	(segment_map[character_index][5] << SEG_F) |
	(segment_map[character_index][6] << SEG_G);
}

void displayCount(uint8_t count)
{
	// Display hundreds digit (if count >= 100)
	if (count >= 100)
	{
		setSegments((count / 100) % 10);
		CTRL_PORT |= (1 << s0);
		_delay_ms(1);
		CTRL_PORT &= ~(1 << s0);
	}

	// Display tens digit
	if (count >= 10)
	{
		setSegments((count / 10) % 10);
		CTRL_PORT |= (1 << s1);
		_delay_ms(1);
		CTRL_PORT &= ~(1 << s1);
	}

	// Display ones digit
	setSegments(count % 10);
	CTRL_PORT |= (1 << s2);
	_delay_ms(1);
	CTRL_PORT &= ~(1 << s2);

	
}
volatile uint8_t alarmBlinkFlag = 0; // Flag to indicate LED blinking state
void displayTempUnit(uint8_t temp_unit)
{
	// Display 'C' or 'F' for Celsius or Fahrenheit
	setSegments(temp_unit); // 10 for 'C', 11 for 'F'
	CTRL_PORT |= (1 << s3);
	_delay_ms(2);
	CTRL_PORT &= ~(1 << s3);
}
float celsius_to_fahrenheit(int celsius) {
	return (celsius * 9 / 5) + 32;
}

void testDigits()
{
	for (uint8_t i = 0; i < 12; i++)
	{
		setSegments(i);
		_delay_ms(1000); // Wait for 1 second to observe the display
	}
}

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


// minimum debounce functions
uint8_t readButton12(uint8_t button)
{
	static uint8_t previousState = 1;
	uint8_t currentState = PINC & (1 << button);

	// Check if the button state has changed from released to pressed
	if (previousState && !currentState) {
		_delay_ms(20); // Debounce delay
		currentState = PINC & (1 << button);
		if (!currentState) {
			previousState = currentState;
			return 1;
		}
	}

	previousState = currentState;
	return 0;
}

// Function to read button state
uint8_t readButton3(uint8_t button)
{
	static uint8_t previousState = 1;
	uint8_t currentState = PIND & (1 << button);

	
	if (previousState && !currentState) {
		_delay_ms(20); // Debounce delay
		currentState = PIND & (1 << button);
		if (!currentState) {
			previousState = currentState;
			return 1;
		}
	}

	previousState = currentState;
	return 0;
}


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
		if (readButton12(button1))
		{
			// temp_unit = !temp_unit; // Toggle temperature unit
			if (temp_unit==10)
			{
				temp_unit=11;
			}
			else{
				temp_unit=10;
			}
			

		}
		if (readButton12(button2))
		{
			if (overflow==1)
			{
				count=0;
			}
			overflow = 0; // Toggle temperature unit
			alarmBlinkFlag =0;
			PORTD &= ~(1 << ledx); // Turn off the LED
		}
		if (readButton3(button3))
		{
			mode = !mode; // Toggle temperature unit
			// Initialize LED brightness based on the current mode
			if (mode == 1) {
				// Mode 1: Turn off LED l1
				setBrightness(0, 0, 0, 0);
				} else {
				brightness();
			}
		}
		
		
		if (mode)
		{
			
			if (temp_unit==10){
				// Display count and temperature unit celcius
				if (overflow==1)
				{
					alarmReset();
					displayCount(count);
					displayTempUnit(temp_unit);
					// for both to LED and SSD to turn on
					// brightness();
				}

				displayCount(count);
				displayTempUnit(temp_unit);
				// for both to LED and SSD to turn on
				// brightness();
				
				
				
			}
			else{
				if (overflow==1)
				{
					alarmReset();
					displayCount(celsius_to_fahrenheit(count));
					displayTempUnit(temp_unit);
					// for both to LED and SSD to turn on
					// brightness();
				}
				displayCount(celsius_to_fahrenheit(count));
				displayTempUnit(temp_unit);
				// for both to LED and SSD to turn on
				// brightness();
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