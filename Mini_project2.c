#include <avr/interrupt.h>
#include<avr/io.h>
#include<util/delay.h>

unsigned int sec = 0;
unsigned int min = 0;
unsigned int hours = 0;
unsigned char tick = 0;
char stp_watch[6];

void ext_int0(void) {
	MCUCR |= (1 << ISC01);
	DDRD &= ~(1 << 2);
	PORTD |= (1 << 2);
	GICR |= (1 << INT0);
	SREG |= (1 << 7);

}
void ext_int1(void) {
	MCUCR |= (1 << ISC10) | (1 << ISC11);
	DDRD &= ~(1 << 3);
	PORTD &= ~(1 << 3);
	GICR |= (1 << INT1);
	SREG |= (1 << 7);

}
void ext2_int2(void) {
	MCUCSR &= ~(1 << ISC2);
	DDRB &= ~(1 << 2);
	PORTB = (1 << 2);
	GICR |= (1 << INT2);
	SREG |= (1 << 7);

}

void Timer1_cmp_init(void) {
	TCCR1A = (1 << FOC1A); // to activate normal or compare mode
	TCCR1B = (1 << CS12) | (1 << CS10); // set clk to 1024 
	TCCR1B |= (1 << WGM12); // to activate ctc mode
	OCR1A = 1000; // compare value to reach
	TIMSK |= (1 << OCIE1A); // ctc interrupt enable
	SREG |= (1 << 7);
	TCNT1 = 0; // set intial value to timer 

}
ISR( INT2_vect) {
	SREG |= (1 << 7);
	TCCR1B |= (1 << CS12) | (1 << CS10); // start again

}
ISR( INT1_vect) {
	SREG |= (1 << 7);
	TCCR1B &= ~(1 << CS12) & ~(1 << CS10) & ~(1 << CS12); // stop clock 
}

ISR( INT0_vect) {
	if (!(PIND & (1 << 2))) {
		for (int i = 0; i < 6; i++) {
			stp_watch[i] = 0; //reset counter timer
		}
		TCNT1 = 0; // reset clock timer
	}
}
ISR( TIMER1_COMPA_vect) {
	//
	stp_watch[0]++;
	if (stp_watch[0] == 10) { //first 7-segment
		stp_watch[1]++; // second 7-segment
		stp_watch[0] = 0;  // set first 7-segment equal to 0 
		if (stp_watch[1] == 6 && stp_watch[0] == 0) { // 60 seconds means one minutes so increment 3rd 7-segment and reset seconds
			stp_watch[1] = 0;
			stp_watch[2]++; // one minute incremented
		}
	}
	if (stp_watch[2] == 10) { //if minutes decimal reach 10 start count in 4th 7-segment
		stp_watch[3]++;
		stp_watch[2] = 0; //reset decimal minutes
		if (stp_watch[3] == 6 && stp_watch[2] == 0) {  // reach 60 minutes 
			stp_watch[3] = 0;
			stp_watch[4]++; // increment hours
		}
	}
	if (stp_watch[4] == 10) { //decimal hours reach 9 increase tens hours
		stp_watch[5]++;
		stp_watch[4] = 0; //reset decimal hours
		if (stp_watch[5] == 10 && stp_watch[4] == 0) { // reach 99 hours 59 minutes 59 seconds 
			for (int i = 0; i < 6; i++) {
				stp_watch[i] = 0; // reach limit of stopwatch so start count from zero
			}
			TCNT1 = 0; // reset timer counter
		}
	}

}

int main() {

	DDRC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	PORTC &= ~(1 << 0) & ~(1 << 1) & ~(1 << 2) & ~(1 << 3);

	DDRA |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
	PORTA &= ~(1 << 0) & ~(1 << 1) & ~(1 << 2) & ~(1 << 3) & ~(1 << 4)
			& ~(1 << 5);

	Timer1_cmp_init();
	ext_int0();
	ext_int1();
	ext2_int2();

	while (1) {
		for (int j = 0; j < 6; j++) {
			PORTA = 1 << j; //looping over 6 7-segments to turn on
			PORTC = stp_watch[j]; //showing value on each 7-segment
			_delay_ms(3);
		}

	}
}
