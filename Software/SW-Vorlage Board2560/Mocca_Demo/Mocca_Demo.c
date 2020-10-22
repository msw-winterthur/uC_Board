/***********************************************************************
 *                                                                     
 *  DATE        : 11.11.2014										   
 *  DESCRIPTION : Testsoftware für Mocca			                                                                       
 *  Author:		:                 
 *                                                                     
 ***********************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Mocca_Treiber.h"

u16  Helligkeit;		// Variable eröffnen (16Bit ohne Vorzeichen, Bereich 0 - 65535)
u16  zaehler = 0;



// Hauptprogramm
void main(void)
{
	// Initialisierung: Board und Peripherie
	init_mocca();
	init_lcd();		
	init_ADC();
    wait_5ms(100);
	init_RGB();
	
	// LCD mit Grundmaske beschreiben (4 Zeilen x 20 Zeichen)
	write_text(1,0,"P1/2:         L:    ");
	write_text(2,0,"M1/2:         C:    ");
	write_text(3,0,"Jx/y:         T:    ");
	 
	PWM_RGB(0,0,0);									// Rot, Grün, Blau: Bereich: 0 - 1023
	zaehler = 0;
	 
	// Hauptprogramm
	while(1)
    {
		write_zahl(1, 5,read_ADC( 9),4,0,0);		// A/D-Wandler Kanal 5 messen = Potentiometer R1
		write_zahl(1, 9,read_ADC( 8),4,0,0);
		Helligkeit  = read_ADC(12);					// Helligkeit auf Kanal 12 lesen und in Variable Helligkeit schreiben
		write_zahl(1,16,Helligkeit,4,0,0);			// Helligkeit auf LCD ausgeben
	
		write_zahl(2, 5,read_ADC(14),4,0,0);
		write_zahl(2, 9,read_ADC(15),4,0,0);
		write_zahl(2,16,read_ADC(13),4,0,0);
		
		write_zahl(3, 5,read_ADC(11),4,0,0);
		write_zahl(3, 9,read_ADC(10),4,0,0);
		
		write_zahl(3,16,PINE>>4,4,0,0);
		
		PWM_RGB(read_ADC(10)/2,read_ADC( 9)/2,read_ADC(11)/2);
		
		if(Taste_1) LED_0_ON;
		
		PORTA = ~zaehler;
		zaehler = zaehler + 1;
    }
}