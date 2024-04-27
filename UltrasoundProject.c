/*
*
* Ultrasound project
* Measures the distance of an object to the ultrasound sensor in cm
* Author : Reece Gavin 17197589
*/

/*Include header files*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "Ultrasound_Header_File.h"


int main(void)
{
	char ch; /* character variable for received character*/
	
	DDRB = 0b00000000; // port B all inputs
	DDRD = 0b00010000; //port D bit 4 output
	
	/*call intilization functions*/
	init_USART();
	init_Timer0();
	init_Timer1();
	
	TCNT0 = delayFor10us;/*initilize timer0*/
	
	sei(); /*Enable global interrupts*/
	
	start_edge = 0;		/*set start edge to 0*/
	timecount = 0;       /* Initialise timecount  */
	Time_Period = 0;      /* Initialise Time_Period - not measured yet  */
	
	
	while (1)
	{
		if (UCSR0A & (1<<RXC0)) /*check for character received*/
		{
			ch = UDR0;    /*get character sent from PC*/
			switch (ch)
			{
				/*report distance*/
				case 'M':
				case 'm':
				distance = Time_Period_High/58;
				sprintf(msg1,"Distance is %lu cm",distance);
				sendmsg(msg1);
				break;
				
				/*continuously report distance*/
				case 'V':
				case 'v':
				continuous = 1;
				break;
				
				/*stop continuously reporting distance*/
				case 'W':
				case 'w':
				continuous = 0;
				break;
				
				default:
				break;
			}//end switch
		}//end if
		
		/*continuously report distance*/
		if(continuous)
		{
			if(newTime) /*check time period has changed*/
			{
				distance = Time_Period_High/58;/*distance = echo length /58 for distance in cm*/
				sprintf(msg1,"Distance is %lu cm",distance);
				if(qcntr == sndcntr)/*ensure last message is sent fully before sending next*/
				sendmsg(msg1);
				newTime = 0; /*reset new time flag*/
				
			}
		}
		
	}//end while
}//end main




/*TIMER0 OVERFLOW INTERRUPT*/
ISR(TIMER0_OVF_vect)
{
	TCNT0 = delayFor10us;/*reset timer0*/
	
	PORTD ^= 0b00010000;/*toggle port D bit 4 every 10us*/
}

//USART Initialization
void init_USART()
{
	UCSR0A = 0x00;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<TXC0);  /*enable receiver, transmitter and transmit interrupt*/
	UBRR0 = 103;  /*baud rate = 9600*/
}

//Timer0 Initialization
void init_Timer0()
{
	TCCR0B = (1<<CS00); // clk/1024 as TC0 clock source
	TIMSK0 = (1<<TOIE0);//enable overflow interrupt
}

//Timer1 Initialization
void init_Timer1()
{
	TCCR1B = (1<<ICES1)|(2<<CS10); // input capture on falling edge, clk/8
	TIMSK1 = (1<<ICIE1)|(1<<TOIE1); // capture and overflow interrupts enabled
}

/*FUNCTION TO SEND MESSAGE */
void sendmsg (char *s)
{
	qcntr = 0;    /*preset indices*/
	sndcntr = 1;  /*set to one because first character already sent*/
	queue[qcntr++] = 0x0d;   /*put CRLF into the queue first*/
	queue[qcntr++] = 0x0a;
	
	while (*s)
	queue[qcntr++] = *s++;   /*put characters into queue*/
	
	UDR0 = queue[0];  /*send first character to start process*/
}

/*USART INTERRUPT*/
ISR(USART_TX_vect)
{
	/*send next character and increment index*/
	if (qcntr != sndcntr)
	UDR0 = queue[sndcntr++];
}

/*TIMER1 OVERFLOW INTERRUPT*/
ISR(TIMER1_OVF_vect)
{
	++timecount;  // Increase overflow counter on interrupt
}


/*TIMER1 capture interrupt*/
ISR(TIMER1_CAPT_vect)
{
	static unsigned long last_time_period; /*variable used to check if time period has changed*/
	unsigned long clocks;//unsigned long clocks;     /* count of clocks in the pulse - not needed outside the ISR, so make it local */
	end_edge = ICR1;        /* The C compiler reads two 8bit regs for us  */
	clocks = ((unsigned long)timecount * 65536) + (unsigned long)end_edge - (unsigned long)start_edge;
	timecount = 0;     // Clear timecount for next time around
	start_edge = end_edge;
	
	// Save its time for next time through here
	if(TCCR1B &(1<<ICES1))/*detect falling edge*/
	{
		Time_Period_Low = clocks/2; // Microseconds
		TCCR1B ^= 1<<ICES1;/*invert ICES1 bit of TCCR1B*/
	}
	else
	{
		Time_Period_High = clocks/2; // Microseconds
		TCCR1B ^= 1<<ICES1;/*invert ICES1 bit of TCCR1B*/
	}

	/*Calculate time period*/
	Time_Period = Time_Period_High + Time_Period_Low;
	
	/*Check if time period has changed*/
	if(last_time_period != Time_Period)
	newTime = 1;
	
	/*update last time period*/
	last_time_period = Time_Period;
	
}

