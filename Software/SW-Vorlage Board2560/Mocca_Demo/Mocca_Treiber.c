/***********************************************************************
 *                                                                     
 *  DATE        : 28.11.2014
 *  DESCRIPTION : Treiber für Mocca
 *  Author		: 
 *	Version		: 1.1                               
 *                                                                     
 ***********************************************************************/


#include <avr/io.h>
#include "Mocca_Treiber.h"

void lcd_sid(uint8_t status);
void lcd_sclk(uint8_t status);
void write_lcd_f(uint16_t rs, uint16_t value);

volatile uint16_t takt_5ms_zaehler, time_cnt=0, cnt = 0;


//--------------------------------------------------------------------------------------------
// Initialisirung SiriusBlack
//--------------------------------------------------------------------------------------------

void init_sirius(void)
{
	init_mocca();
}

void init_mocca(void)
{
	DDRA = 0xFF;	// LED Port als Ausgang
	DDRK = 0x00;	// Schalter Port als Eingang
	DDRE = 0x00;	// Taster als Eingang
	DDRG = 0x27;	// LCD port als Ausgang
	DDRH = 0x78;	// RGB, CTS als Ausgang
	
	DDRJ = 0xFF;	// LED-Matrix
	
	init_5ms_timer();
	
	sei();			// Global interrups aktivieren
	
	init_lcd();
	lcd_light(10);
}

//--------------------------------------------------------------------------------------------
// Timer 0 initialisirung
//--------------------------------------------------------------------------------------------

void init_5ms_timer(void)
{
	TCCR0A = 0b00000011; // Timer mode einstellungen -> Fast PWM mode
	TCCR0B = 0b00001101; // 16Mhz / 1024 = 15,635kHz
	TIMSK0 = 0b00000001; // Timer overflow Interrupt aktivieren
	OCR0A  = 77;
}

//--------------------------------------------------------------------------------------------
// Timer 0 Overflow Interrupt ->  Wird alle 5ms aufgerufen
//--------------------------------------------------------------------------------------------

ISR(TIMER0_OVF_vect)
{
	takt_5ms_zaehler --;
	time_cnt++;
	if(time_cnt % 20 == 0) cnt++;

}

//--------------------------------------------------------------------------------------------
// 5ms Wartefunktion
//--------------------------------------------------------------------------------------------

void wait_5ms(uint16_t faktor)
{
	takt_5ms_zaehler = faktor;
	if(faktor != 0) while(takt_5ms_zaehler);
}

//--------------------------------------------------------------------------------------------
// 1us Wartefunktion
//--------------------------------------------------------------------------------------------
void wait_1us(uint16_t volatile faktor)
{
	while (faktor)
	{	
		// 11 Taktzüklen keine operation , retliche 5 taktzyklen für while und faktor increment
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		asm  ("nop");
		faktor --;
	}
}
//****************************************************************** RS232 / USB -Treiber ******************************************************************//

//--------------------------------------------------------------------------------------------
// RS232 Initialisiren
//--------------------------------------------------------------------------------------------
void init_rs232 (uint16_t baudratenregister)
{
	UCSR2B = 0b00011000; // RX und TX enable
	UCSR2C = 0b00000110; // Asynkrone USART, 8 Datenbits

	UBRR2H = (baudratenregister >>8);
	UBRR2L = (baudratenregister & 0x00FF);
}

//--------------------------------------------------------------------------------------------
// Ein einzelnes zeichen über RS232 Senden
//--------------------------------------------------------------------------------------------
void rs232_sende_zeichen(char zeichen)
{
	UDR2 = zeichen;
	
	while( (UCSR2A & 0b01000000) == 0); // Warten bis Byte gesendet
	UCSR2A |= 0b01000000;	//Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------------------
// Einen Nullterminierten string über RS232 senden
//--------------------------------------------------------------------------------------------
void rs232_sende_string(char *Text)
{	
	uint8_t i = 0;
	
	while(Text[i])
	{
		rs232_sende_zeichen(Text[i]);
		i++;	
	}	
}

//--------------------------------------------------------------------------------------------
// USB Initialisiren
//--------------------------------------------------------------------------------------------
void init_usb(void)
{
	UCSR0B = 0b00011000; // RX und TX enable
	UCSR0C = 0b00000110; // Asynkrone USART, 8 Datenbits

	// 9600 Baud
	UBRR0H = 0;
	UBRR0L = 103; 
}

//--------------------------------------------------------------------------------------------
// Ein einzelnes zeichen über USB Senden
//--------------------------------------------------------------------------------------------
void USB_sende_zeichen(char zeichen)
{
	UDR0 = zeichen;
	
	while( (UCSR0A & 0b01000000) == 0); // Warten bis Byte gesendet
	UCSR0A |= 0b01000000;				//Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------------------
// Einen Nullterminierten string über USB senden
//--------------------------------------------------------------------------------------------
void USB_sende_string(char *Text)
{
	uint8_t i = 0;
	
	while(Text[i])
	{
		USB_sende_zeichen(Text[i]);
		i++;
	}
}

//****************************************************************** A/D_Wandler-Treiber ******************************************************************//

void init_ADC(void)
{
	ADMUX  = 0x40;	//AVCC Als referenz
	DIDR0  = 0x0F;	// Digitale Register an ADC pins der Potentiometer deaktivieren
	ADCSRA = 0b10100111; // ADC einschalten, ADC clok = 16MHz / 128
	ADCSRB = 0x00;	// Free runing mode
	
	ADCSRA |=  0b01000000;		// Dummy messung Starten
	while((ADCSRA&0x10) == 0);	// Warten bis Messung abgeschllossen
	
	ADCSRA &= 0xEF;				// Interruptflage löschen	
}

uint16_t read_ADC(uint8_t kanal)
{
	uint16_t messwert = 0;
	
	// Kanal definieren
	ADMUX  = 0x40;	//AVCC Als referenz
	if(kanal>=8) 
	{	ADMUX  |= kanal-8;
		ADCSRB |= (3 << MUX5);
	}
	else
	{	ADMUX  |= kanal;
		ADCSRB &= ~(3 << MUX5);
	}
	
	
	ADCSRA |=  0b01000000;		// ADC Starten
	while((ADCSRA&0x10) == 0);	// Warten bis Messung abgeschllossen
	
	wait_5ms(1);
	
	messwert = ADCL;
	messwert |= ADCH <<8;
	
	ADCSRA &= 0xEF;				// Interruptflage löschen	
	
	return messwert;
}


//****************************************************************** RGB-LED-Treiber ******************************************************************//

//--------------------------------------------------------------------------------------------
// Initialisirung des PWM der RGB-LED
//--------------------------------------------------------------------------------------------
void init_RGB(void)
{
	uint16_t aufloesung = 1023; // 10bit pwm aufösung
	
	TCCR4A = 0b11111110; // Ausgänge als PWM Ausgang ,Fast PWM mode
	TCCR4B = 0b00011011; //  Clock / 64
	
	ICR4H = (aufloesung >>8);
	ICR4L = (aufloesung & 0x00FF);
}

//--------------------------------------------------------------------------------------------
// Übergabe von Werten an die PWM Register
//--------------------------------------------------------------------------------------------
void PWM_RGB(uint16_t Rot,uint16_t Gruen,uint16_t Blau)
{
	Rot		= 1023 - Rot;
	Gruen	= 1023 - Gruen;
	Blau	= 1023 - Blau;
	
	OCR4AH = (Rot >>8);
	OCR4AL = (Rot & 0x00FF);
	
	OCR4BH = (Gruen >>8);
	OCR4BL = (Gruen & 0x00FF);
	
	OCR4CH = (Blau >>8);
	OCR4CL = (Blau & 0x00FF);
}

void RGB_Rot(uint16_t Rot)
{
	// Wenn PWM nicht initialisiert ist Pin mit LED High oder Low setzen
	if(Rot)	PORTH |= 0x08;
	   else PORTH &= 0xF7;
	
	// Wenn PWM initialisirt ist wert an PWM register übergeben
	Rot		= 1023 - Rot;
	OCR4AH = (Rot >>8);
	OCR4AL = (Rot & 0x00FF);
}

void RGB_Gruen(uint16_t Gruen)
{
	// Wenn PWM nicht initialisiert ist Pin mit LED High oder Low setzen
	if(Gruen)	PORTH |= 0x10;
		   else PORTH &= 0xEF;
	
	// Wenn PWM initialisirt ist wert an PWM register übergeben
	Gruen	= 1023 - Gruen;
	OCR4BH = (Gruen >>8);
	OCR4BL = (Gruen & 0x00FF);
}

void RGB_Blau(uint16_t Blau)
{
	// Wenn PWM nicht initialisiert ist Pin mit LED High oder Low setzen
	if(Blau)	PORTH |= 0x20;
		   else PORTH &= 0xDF;
	
	// Wenn PWM initialisirt ist wert an PWM register übergeben
	Blau	= 1023 - Blau;
	OCR4CH = (Blau >>8);
	OCR4CL = (Blau & 0x00FF);
}

//****************************************************************** LCD-Treiber ******************************************************************//

//--------------------------------------------------------------------------------------------
// Ansteuerung von einzelen Bits fur LCD
//--------------------------------------------------------------------------------------------
void lcd_sid(uint8_t status)		// LCD Datenleitung
{
	if(status)	PORTG |= 0x01;
	else PORTG &= 0xFE; 
}

void lcd_sclk(uint8_t status)		// LCD Taktleitung
{
	if(status)	PORTG |= 0x02;
	else PORTG &= 0xFD;
}

void lcd_on_off(uint8_t status)		//LCD ein/Aus
{
	if(status)	PORTG |= 0x20;
	else PORTG &= ~0x20;
}

//--------------------------------------------------------------------------------------------
// LCD Hintergundbeleuchtung mit PWM dimmen (0 bis 100%) Timer 0 muss initialisiert sein! 
//--------------------------------------------------------------------------------------------
void lcd_light(uint8_t hellighkeit) // Werte von 0 bis 10 möglich -> 0 = Aus , 1= 10%, 10 = 100%
{
	if(hellighkeit == 0)
	{
		TCCR0A &= 0b11011111; //Wenn heligkeit 0 -> Hintergrundbeleuchtung ausschalten
	}
	else
	{
		TCCR0A |= 0b00100000;
		if(hellighkeit >10)hellighkeit = 10;
		OCR0B = (hellighkeit*8);// + 178;
	}
}

//--------------------------------------------------------------------------------------------
// Initialisierung des LCD's
//--------------------------------------------------------------------------------------------
void init_lcd(void)
{
	lcd_sclk(1);
	lcd_sid(0);
	
	lcd_on_off(0);
	wait_5ms(2);
	 
	lcd_on_off(1); 
	wait_5ms(10);
	
	write_lcd_f('C',0x34);      // set 8-Bit-Interface RE = 1
	write_lcd_f('C',0x09);      // 4-Zeilen-Modus, 5-Dot Font-Breite
	
	write_lcd_f('C',0x30);      // set 8-Bit-Interface RE = 0
	write_lcd_f('C',0x0C);      // Display ON, Cursor OFF

	clear_lcd_f();				// Clear Display
	
	write_lcd_f('C',0x07);      // Entry Mode
	
	write_text(0,0,"My first Board       ");
	write_text(1,0, "-------------       ");
	write_text(2,0, "                    ");
	write_text(3,0, "                    ");
}

//--------------------------------------------------------------------------------------------
// Schreibt ein Kommando oder ein Datenbyte (Zeichen) zum LCD SPI (Serial Protokoll Interface)
//--------------------------------------------------------------------------------------------
void write_lcd_f(uint16_t rs, uint16_t value)
{ uint16_t i;
	// Manche Befehle müssen doppelt ausgeführt werden, damit die
	// minimale Pulslänge von 400ns eingehalten wird. (bei 20MHz)

	// Synchronisierung: 5x "1" senden
	lcd_sid(1);				// Daten-Bit = 1
	for(i=0;i<5; i++)
	{	
		lcd_sclk(0);			// Synch-Bits senden
		lcd_sclk(1);
	}
	// R/W: 1=Read, 0=Write
	lcd_sid(0);				    // R/W = 0
	lcd_sclk(0);				// R/W-Bit senden
	lcd_sclk(1);

	// RS Register Selection: 0=Command, 1=Data
	if (rs == 'C') lcd_sid(0);
	else lcd_sid(1);

	lcd_sclk(0);				// RS-Bit senden
	lcd_sclk(1);

	// End-Marke 0
	lcd_sid(0);
	
	lcd_sclk(0);				// END-Bit senden
	lcd_sclk(1);
	
	// Daten-Bit 0-3
	for(i=0;i<4; i++)
	{	
		lcd_sclk(0);
		if (value & 0x01) lcd_sid(1);
		else lcd_sid(0);
		value = value >> 1;
		lcd_sclk(0);
		lcd_sclk(1);
	}

	lcd_sid(0);		// 4x "0" senden
	for(i=0;i<4; i++)
	{	
		lcd_sclk(0);
		lcd_sclk(1);
	}
	
	// Daten-Bit 4-7
	for(i=0;i<4; i++)
	{	
		lcd_sclk(0);
		if (value & 0x01) lcd_sid(1);
		else lcd_sid(0);
		value = value >> 1;
		lcd_sclk(0);
		lcd_sclk(1);
	}
	
	lcd_sid(0);			// 4x "0" senden
	for(i=0;i<4; i++)
	{	
		lcd_sclk(0);
		lcd_sclk(1);
	}
	
	lcd_sid(1);
	// Write-Befehl auf 50us verlängern, damit minimale Execution-Time 39us/43us eingehalten ist.
	wait_1us(50);
}

//------------------------------------------------------------
// Text an xy-Position ausgeben
//------------------------------------------------------------
void write_text(uint8_t y_pos, uint8_t x_pos, char *str_ptr)
{
	uint8_t str_p = 0;
	uint8_t pos;
	pos = x_pos + (y_pos * 0x20);
	write_lcd_f('C',pos | 0x80);

	while(str_ptr[str_p])
	{ write_lcd_f('D',str_ptr[str_p++]);
	}
}

//------------------------------------------------------------
// Zahl an xy-Position ausgeben dezimal
//------------------------------------------------------------
void write_zahl(uint8_t x_pos, uint8_t y_pos, uint16_t zahl_v, uint8_t s_vk, uint8_t s_nk, uint8_t komma)
{ 
	unsigned long zehner = 10;
	unsigned char send_buffer[12];
	unsigned char i, pos, pos_t;

	// Umwandlung in die einzelnen Stellen-Zahlen 1er, 10er, 100er, ...
	//  zahl = 12345;
	//  s_vk = 2;
	//  s_nk = 0;
	//  komma = 0;
	send_buffer[11] = (zahl_v % 10) + 48;
	i = 10;
	do
	{ send_buffer[i] = (zahl_v / zehner % 10) + 48;
		zehner *= 10;
	}while(i--);
	
	// Vor-Kommastellen kopieren
	pos = 0;
	pos_t = 12-komma-s_vk;
	for (i=0; i<s_vk; i++)
	{ send_buffer[pos++] = send_buffer[pos_t++];
	}
	if (s_nk > 0)
	{ send_buffer[pos++] = '.';

		// Nach-Kommastellen kopieren
		pos_t = 12-komma;
		for (i=0; i<s_nk; i++)
		{ send_buffer[pos++] = send_buffer[pos_t++];
		}
	}
	send_buffer[pos] = 0;     // Endmarke des Strings setzen
	
	// Vorangehende Nullen löschen
	i = 0;
	while ( (send_buffer[i] == 48) && (i < s_vk-1) )
	{ send_buffer[i++] = 32;
	}

	write_text(x_pos, y_pos, send_buffer);
}

//------------------------------------------------------------
// Clear LCD
//------------------------------------------------------------
void clear_lcd_f(void)
{
	write_lcd_f('C',0x01);  // Clear Display
	wait_5ms(1);			// 2ms warten, bis LCD gelöscht ist
	
	write_text(0,0," ");	// Blödes Zeichen auf Disply löschen
}

