/*
* Ultrasound header file
* Contains all function declarations and global variables for UltrasoundProject.c
* Author : Reece Gavin 17197589
*/
#ifndef ULTRASOUND_HEADER_H
#define ULTRASOUND_HEADER_H

/*Declare functions*/
void sendmsg (char *s);
void init_USART();
void init_Timer0();
void init_Timer1();

/*USART send message variables*/
unsigned char qcntr = 0,sndcntr = 0;   /*indexes into the que*/
unsigned char queue[50];       /*character queue*/
char msg1[50]; /*char array for storing messages to print*/

/*timer variables and time period variables*/
unsigned char timecount;    // Extends TCNT of Timer1
unsigned int start_edge, end_edge;  // globals for times.
unsigned long Time_Period;
unsigned long Time_Period_High;
unsigned long Time_Period_Low;


/*Flag variables*/
unsigned int continuous;
unsigned int newTime = 1;

/*variable for storing distance value*/
unsigned long distance;

/*TIMER0*/
#define delayFor10us 96;/*256-96*160 = 10us*/

#endif