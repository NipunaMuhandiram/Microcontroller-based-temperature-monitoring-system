

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL

volatile uint8_t count = 0;
volatile uint8_t temp_unit = 10; // 0 for Celsius, 1 for Fahrenheit
volatile uint8_t overflow = 0; // 0 for not overflow, 1 for overflow
volatile uint8_t mode = 1; // 1 for mode 1 (on the 7-segment display) and 0 for mode 2 (using the LEDs.)
volatile uint8_t alarmBlinkFlag = 0; // Flag to indicate LED blinking state

volatile uint8_t brightness1 = 0;
volatile uint8_t brightness2 = 0;
volatile uint8_t brightness3 = 0;
volatile uint8_t brightness4 = 0;

// Button pins
#define button1 PC4
#define button2 PC5
#define button3 PD7
#define ledx PD6

// Pins for LEDs indicating temperature ranges
#define l1 PD0
#define l2 PD1
#define l3 PD4
#define l4 PD5

#define LOAD_PIN PB0
#define CLK_PIN PB5
#define DIN_PIN PB3

ISR(INT0_vect) {
    _delay_ms(50); // Debounce delay
    if (bit_is_clear(PIND, PD2)) {
        if (count < 250) {
            count++;
        }
        if (count >= 41) {
            overflow = 1;
        }
    }
}

ISR(INT1_vect) {
    _delay_ms(50); // Debounce delay
    if (bit_is_clear(PIND, PD3)) {
        if (count > 0) {
            count--;
        } else {
            count = 0;
            overflow = 0;
        }
        if (count <= 40) {
            overflow = 0; // Reset flag to stop LED blinking
            PORTD &= ~(1 << ledx); // Turn off the LED
        }
    }
}

void softwarePWM() {
    // Calculate brightness values based on count
    if (count >= 0 && count <= 15) {
        brightness1 = count * 17;
        brightness2 = 0;
        brightness3 = 0;
        brightness4 = 0;
    } else if (count >= 16 && count <= 25) {
        
        brightness1 = 255;
        brightness2 = (count * 25) - 400;
        brightness3 = 0;
        brightness4 = 0;
    } else if (count >= 26 && count <= 35) {
        brightness1 = 255;
        brightness2 = 255;
        brightness3 = (count * 25) - 650;
        brightness4 = 0;
    } else if (count >= 35 && count <= 40) {
        brightness1 = 255;
        brightness2 = 255;
        brightness3 = 255;
        brightness4 = (count * 51) - 1785;
    }

    // Software PWM loop
    for (uint8_t i = 0; i < 255; i++) {
        // LED 1
        if (i < brightness1)
            PORTD |= (1 << l1);
        else
            PORTD &= ~(1 << l1);

        // LED 2
        if (i < brightness2)
            PORTD |= (1 << l2);
        else
            PORTD &= ~(1 << l2);

        // LED 3
        if (i < brightness3)
            PORTD |= (1 << l3);
        else
            PORTD &= ~(1 << l3);

        // LED 4
        if (i < brightness4)
            PORTD |= (1 << l4);
        else
            PORTD &= ~(1 << l4);

        _delay_us(1); // Adjust this delay as needed
    }
}

void setup() {
    // Set button pins as input with pull-up resistors
    DDRC &= ~(1 << button1) & ~(1 << button2);
    DDRD &= ~(1 << button3);
    PORTC |= (1 << button1) | (1 << button2);
    PORTD |= (1 << button3);

    // Set ledx as output
    DDRD |= (1 << ledx);
}
void customledoff(){
    PORTD &= ~(1 << l1);
    PORTD &= ~(1 << l2);
    PORTD &= ~(1 << l3);
    PORTD &= ~(1 << l4);
}

// SPI Transmit function
void SPI_Transmit(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        if (data & (1 << (7 - i))) {
            PORTB |= (1 << DIN_PIN);
        } else {
            PORTB &= ~(1 << DIN_PIN);
        }
        PORTB |= (1 << CLK_PIN);
        PORTB &= ~(1 << CLK_PIN);
    }
}

// Send data to a specific MAX7219 register
void MAX7219_send(uint8_t address, uint8_t data) {
    PORTB &= ~(1 << LOAD_PIN);  // Set LOAD low
    SPI_Transmit(address);      // Send address
    SPI_Transmit(data);         // Send data
    PORTB |= (1 << LOAD_PIN);   // Set LOAD high
}

// Initialize the MAX7219 for no decode mode
void MAX7219_init(void) {
    MAX7219_send(0x09, 0x00);  // Decode mode: No decode for digits 7-0
    MAX7219_send(0x0A, 0x08);  // Intensity (brightness)
    MAX7219_send(0x0B, 0x07);  // Scan limit (all digits)
    MAX7219_send(0x0C, 0x01);  // Shutdown register 
    MAX7219_send(0x0F, 0x00);  // Display (off)
}
void MAX7219_shutdown(void) {
    MAX7219_send(0x0C, 0x00); // Shutdown register: 0x00 turns off the display
}

void MAX7219_wakeup(void) {
    MAX7219_send(0x0C, 0x01); // Shutdown register: 0x01 turns on the display
}



const uint8_t digitPatterns[10] = {
    0b1111110,  // 0
    0b0110000,  // 1
    0b1101101,  // 2
    0b1111001,  // 3
    0b0110011,  // 4
    0b1011011,  // 5
    0b1011111,  // 6
    0b1110000,  // 7
    0b1111111,  // 8
    0b1111011   // 9
};

// Display a three-digit number (0-999) on specific digit positions (1-8)
void displayNumber(uint16_t number) {
    uint8_t units = number % 10;
    uint8_t tens = (number / 10) % 10;
    uint8_t hundreds = (number / 100) % 10;

    // Display hundreds digit (position 3)
    MAX7219_send(4, digitPatterns[hundreds]);

    // Display tens digit (position 2)
    MAX7219_send(3, digitPatterns[tens]);

    // Display units digit (position 1)
    MAX7219_send(2, digitPatterns[units]);
}

float celsius_to_fahrenheit(int celsius) {
    return (celsius * 9 / 5) + 32;
}

void displayTempUnit(uint8_t temp_unit) {
    if (temp_unit == 10) {
        MAX7219_send(1, 0b01001110);
    } else {
        MAX7219_send(1, 0b01000111);
    }
}

void setupInterrupt() {
    // Configure INT0 (PD2) for falling edge triggered external interrupt
    EICRA |= (1 << ISC01); // Falling edge triggers INT0
    EIMSK |= (1 << INT0);  // Enable INT0

    // Configure INT1 (PD3) for falling edge triggered external interrupt
    EICRA |= (1 << ISC11); // Falling edge triggers INT1
    EIMSK |= (1 << INT1);  // Enable INT1

    sei(); // Enable global interrupts
}


void timer1_init(void) {
    // Set Timer1 to CTC mode with a prescaler of 64
    TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
    // Set CTC compare value for 500ms at 16MHz AVR clock with a prescaler of 64
    // OCR1A = 12499; // 16,000,000 / (64 * 2) - 1 = 12499
    OCR1A = 2499; // 16,000,000 / (64 * 10) - 1 = 2499
    // Enable Timer1 compare interrupt
    TIMSK1 |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
    // Toggle the LED
    PORTD ^= (1 << ledx);
}

void Alarm(uint8_t state) {
    if (state) {
        // Start the timer
        timer1_init();
    } else {
        // Stop the timer
        TCCR1B &= ~((1 << CS11) | (1 << CS10)); // Clear prescaler bits to stop the timer
        PORTD &= ~(1 << ledx); // Turn off the LED
    }
}

// Minimum debounce functions
uint8_t readButton12(uint8_t button) {
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

// Minimum debounce functions
uint8_t readButton13(uint8_t button) {
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
uint8_t readButton3(uint8_t button) {
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

int main(void) {
    setupInterrupt();
    setup();
    sei();
    // Set CLK, LOAD, and DIN as output
    DDRB |= (1 << LOAD_PIN) | (1 << CLK_PIN) | (1 << DIN_PIN);
    // Set l1, l2, l3, l4 as output
    DDRD |= (1 << l1) | (1 << l2) | (1 << l3) | (1 << l4);

    // Initialize MAX7219
    MAX7219_init();

	

    while (1) {
        if (count < 41) {
            Alarm(0);
        }

        // Check if any button is pressed
        if (readButton12(button1)) {
            // Toggle temperature unit
            temp_unit = (temp_unit == 10) ? 11 : 10;
        }

        if (readButton3(button3)) {
            mode = !mode; // Toggle mode
            // Initialize LED brightness based on the current mode
            if (mode == 1) {
                customledoff();
                MAX7219_wakeup();
            } else {
               MAX7219_shutdown();
            }
        }

        if (readButton13(button2)) {
            if (overflow == 1) {
                count = 0;
            }
            overflow = 0;
            Alarm(0);
        }

        if (mode)
        {
           
            if (temp_unit == 10) {
                if (overflow == 1) {
                    Alarm(1);
                }
                displayNumber(count);
                displayTempUnit(temp_unit);
                
            } else {
                if (overflow == 1) {
                    Alarm(1);
                }
                displayNumber(celsius_to_fahrenheit(count));
                displayTempUnit(temp_unit);
              
            }
        }
        else
        {
            if (overflow == 1) {
                    Alarm(1);
                }
            softwarePWM();
            
        }
        
        


    }

    return 0;
}





