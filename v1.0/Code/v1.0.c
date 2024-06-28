// Nipuna Muhamdiram 
// Github ver 1.10 - beta

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL

volatile uint8_t count = 0;
volatile uint8_t overflow = 0;
int tmpval;

#define DEC_PORT PORTC
#define DEC_DDR DDRC
#define CTRL_PORT PORTB
#define CTRL_DDR DDRB

#define s0 PB4
#define s1 PB5
#define s2 PB6
#define s3 PB7
#define ssd1 PC0
#define ssd2 PC1
#define ssd3 PC2
#define ssd4 PC3
#define button1 PC4
#define button2 PC5
#define button3 PC6
#define ledx PD6
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
	0b00001001  // 9
};

ISR(INT0_vect) {
	_delay_ms(50);
	if (bit_is_clear(PIND, PD2)) {
		count++;
		if (count >= 41) {
            //TODO
			overflow = 1;
		}
		if (count >= 250)
        //TODO
		count = 0;
	}
}

ISR(INT1_vect) {
	_delay_ms(50);
	if (bit_is_clear(PIND, PD3)) {
		if (count > 0) {
			count--;
			} else {
			count = 250;
			overflow = 1;
            //TODO
		}
		if (count <= 40) {
			overflow = 0;
			PORTD &= ~(1 << ledx);
            //TODO
		}
	}
}

void setupInterrupt() {
	EICRA |= (1 << ISC01);
    //TODO
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC11);
	EIMSK |= (1 << INT1);
    //TODO
	sei();
}

uint8_t readButton(uint8_t button) {
	if (!(PINC & (1 << button))) {
		_delay_ms(200);
        //TODO
		if (!(PINC & (1 << button))) {
			return 1;
            //TODO
		}
	}
	return 0;
}

void setup() {
	DDRC &= ~(1 << button1) & ~(1 << button2) & ~(1 << button3);
	PORTC |= (1 << button1) | (1 << button2) | (1 << button3);
    //TODO
	DEC_DDR |= (1 << ssd1) | (1 << ssd2) | (1 << ssd3) | (1 << ssd4);
	CTRL_DDR |= (1 << s0) | (1 << s1) | (1 << s2) | (1 << s3);
    //TODO
	DDRD |= (1 << ledx);
}

void displayCount(uint8_t count) {
	CTRL_PORT = (1 << s0);
	DEC_PORT = (count >= 100) ? ssdnums[count / 100] : 0;
	_delay_ms(5);
    //TODO
	CTRL_PORT = (1 << s1);
	DEC_PORT = ((count >= 10) || (count >= 100)) ? ssdnums[(count / 10) % 10] : 0;
	_delay_ms(5);
	CTRL_PORT = (1 << s2);
	DEC_PORT = ssdnums[count % 10];
	_delay_ms(5);
    //TODO
}

void PWM_init() {
	DDRB |= (1 << l1) | (1 << l2) | (1 << l3);
	DDRD |= (1 << l4);
	TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
	TCCR2B |= (1 << CS22);
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
	TCCR1B |= (1 << CS10) | (1 << WGM12);
	TCCR0A |= (1 << COM0B1) | (1 << WGM00) | (1 << WGM01);
	TCCR0B |= (1 << CS00);
    //todo
}

void setBrightness(uint8_t brightness1, uint8_t brightness2, uint8_t brightness3, uint8_t brightness4) {
	OCR1A = brightness1;
	OCR1B = brightness2;
	OCR0B = brightness4;
	OCR2A = brightness3;
    //TODO
}

void brightness() {
	//TODO
}


//TODO mode func

//TODO value changer range func

//TODO convertion func

//TODO custom debouncer func

//TODO led brightness controller effectlees func

//TODO  interupt handler with debouncer func

//TODO  tester func


int main(void) {
	PWM_init();
	setup();
	setupInterrupt();
	sei();
	
	while (1) {
		if (count < 41) {
			PORTD &= ~(1 << ledx);
		}
		if (readButton(button1)) {
		}
		if (readButton(button2)) {
			if (overflow == 1) {
				count = 0;
				//count = 0;
			}
			overflow = 0;
			PORTD &= ~(1 << ledx);
			//PORTB &= ~(1 << ledx);
		}
		if (readButton(button3)) {
		}
		if (overflow == 1) {
		}
		displayCount(count);
		brightness();
		/*displayCount(count);
		displayCount(count);
		brightness();
		brightness();*/
	}
	return 0;
}
