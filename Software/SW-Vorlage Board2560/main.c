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


int main(void)
{
	
	u16 Poti_1, Poti_2;
	u16 Licht, Magnet_1, Magnet_2, Temp;
	u16 JS_x, JS_y;
	u16 Tasten, DIP_Switch;

	init_mocca();
	init_lcd();		
	init_ADC();

    wait_5ms(100);
	init_RGB();
	



	 
	 write_text(1,0,"P1/2:         L:    ");
	 write_text(2,0,"M1/2:         C:    ");
	 write_text(3,0,"Jx/y:         T:    ");
	 
	 PWM_RGB(0,0,0);
	 
	while(1)
    {
		
		
		
		Poti_1     = read_ADC( 8);									// Potentiometer 1
		Poti_2     = read_ADC( 9);									// Potentiometer 2
		Licht      = read_ADC(12);									// Lichtsensor
		Magnet_1   = read_ADC(14);									// Magnet 1 
		Magnet_2   = read_ADC(15);									// Magnet 2
		Temp       = read_ADC(13);									// Temperatur
		JS_x	   = read_ADC(10);									// Joystick x-Achse
		JS_y	   = read_ADC(11);									// Joystick y-Achse
		Tasten     = (PINL&0b11000011) | (PINE&0b00000100);			// Tasten 1-4
		DIP_Switch = PINC;
		
		write_zahl(1, 9,Poti_1,4,0,0);								// Potentiometer 1
		write_zahl(1, 5,Poti_2,4,0,0);								// Potentiometer 2
		write_zahl(1,16,Licht,4,0,0);								// Lichtsensor
		
		write_zahl(2, 5,Magnet_1,4,0,0);							// Magnet 1 
		write_zahl(2, 9,Magnet_2,4,0,0);							// Magnet 2
		write_zahl(2,16,Temp,4,0,0);								// Temperatur
		
		write_zahl(3, 5,JS_x,4,0,0);								// Joystick x-Achse
		write_zahl(3, 9,JS_y,4,0,0);								// Joystick y-Achse
		
		write_zahl(3,16,Tasten,4,0,0);								// Tasten 1-4 und Joystick
		write_zahl(0,16,DIP_Switch,4,0,0);							// DIP_Switch 1-8
		
		PWM_RGB(JS_x/2,Poti_2/2,JS_y/2);							// RGB-LED: rot = JS_x, grün = Poti_2, blau = JS_y
		
		
	
    }
}