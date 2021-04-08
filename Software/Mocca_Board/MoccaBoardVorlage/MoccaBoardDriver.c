/*
* ucBoardDriver.c
*
* Created: 26.10.2020 08:44:34
*  Author: Dario Dündar
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "MoccaBoardDriver.h"

#ifndef SWITCH_ACTIV_HIGH
    #define SWITCH_ACTIV_HIGH   1
#endif


void initAdc(void);
void initLcd(void);
void initRgb(void);
void lcdSid(uint8_t status);
void lcdSclk(uint8_t status);
void writeLcdF(uint16_t rs, uint16_t value);
void startSystemTimeMs(void);
uint8_t switchReadAll(void);

volatile uint64_t systemTimeMs = 0;

//--------------------------------------------------------------------------------------------
// Initialisirung Board
//--------------------------------------------------------------------------------------------

void initBoard(void)
{
    //PullUpDesable auf 0
    MCUCR = MCUCR | (0<<PUD);
    //PORTA&B LED Ports auf Ausgnag und LED off
    PORTA   = 0x00;
    DDRA    = 0xFF;
//     PORTB   = 0xFF;
//     DDRB    = 0xFF;
    //PORTC Schalter Port auf Eingnag mit Pullup
//     PORTC   = 0xFF;
//     DDRC    = 0x00;
    //PORTD (X1) auf Eingang mit PullUp
//     PORTD   = 0xFF;
//     DDRD    = 0x00;
    //PORTE
    //Taster auf Eingang
//     PORTE   = 0xFF;
    DDRE    = 0x00;
    //Taster Joystick (PE2) ohne PullUp
    PORTE   = PORTE & 0b11111011;
    //PORTF (ADC, X4) Eingang ohne PullUps
//     PORTF   = 0x00;
//     DDRF    = 0x00;
    //PORTG (LCD)
//     PORTG   = 0b00000100;//PG2 PullUp
    DDRG   |= 0b00000011;//LCD SCLK & SID
    DDRG   |= 0b00100000;//LCD-LED
    //PORTH (RGB_LED), CTS
//     PORTH   = 0b11000111;
    DDRH    = 0b01111000;
    //PORTJ (Matrix, X3), Ausgang
//     PORTJ   = 0x00;
//     DDRJ    = 0xFF;
    //PORTK (ADC-Inputs) alles auf Eingang ohne Pullup
//     PORTK   = 0x00;
//     DDRK    = 0x00;
    //PORTL (Taster, X4)
//     PORTL   = 0b00111100;//X4 mit Pullups
//     DDRL    = 0x00;//Alles Eingang
    
    //start 5ms tik
    sei();            // Global interrups aktivieren
    startSystemTimeMs();
    
    
    //init lcd
    initLcd();
    //init adc
    initAdc();
    //init rgb
    initRgb();
}


void ledWrite(uint8_t ledNr0_7, uint8_t wer0_1)
{
    uint16_t mask;
    if (wer0_1)
    {
        mask = (1 << ledNr0_7);
        PORTA = PORTA | mask;
    } 
    else
    {
        mask = ~(1 << ledNr0_7);
        PORTA = PORTA & mask;
    }
}

void ledWriteAll(uint8_t bitMuster)
{
    PORTA = bitMuster;
}

uint8_t ledReadAll(void)
{
    return PORTA;
}

uint8_t ledRead(uint8_t ledNr0_7){
    return PORTA  & (1<<ledNr0_7);
}


uint8_t switchRead(uint8_t switchNr0_7)
{
    return switchReadAll() & (1<<switchNr0_7);
}    

uint8_t switchReadAll(void)
{
    if (SWITCH_ACTIV_HIGH)
    {
        return PINK;
    }
    return ~PINK;
}

uint8_t buttonReadAllPE(void)
{
    return PINE;
}

uint8_t buttonReadPE(uint8_t buttonNr0_7)
{
    return PINE & (1<<buttonNr0_7);
}


//--------------------------------------------------------------------------------------------
// Timer 0 initialisirung
//--------------------------------------------------------------------------------------------

void startSystemTimeMs(void)
{
    TCCR0A = 0b00000011; // Timer mode einstellungen -> Fast PWM mode 10 Bit
    TCCR0B = 0b00001011; // 16Mhz / 64 = 250kHz -> 4us per step
    TIMSK0 = 0b00000001; // Timer overflow Interrupt aktivieren
    OCR0A = 249; // 0 to 249 = 250 steps -> 250steps*4us = 1.000000ms
}

//--------------------------------------------------------------------------------------------
// Timer 0 Overflow Interrupt ->  Wird alle 1ms aufgerufen
//--------------------------------------------------------------------------------------------

ISR(TIMER0_OVF_vect)
{
    systemTimeMs += 1;
}



uint64_t getSystemTimeMs(void)
{
    return systemTimeMs;
}

uint8_t waitForSystemTimeMs(uint64_t systemTimeToWaitForMS){
    if (systemTimeToWaitForMS < systemTimeMs)
    {
        return 0;
    }
    while(systemTimeToWaitForMS > systemTimeMs);
    return 1;
}



//****************************************************************** RS232 / USB -Treiber ******************************************************************//

//--------------------------------------------------------------------------------------------
// RS232 Initialisiren
//--------------------------------------------------------------------------------------------
void initRs232 (uint16_t baudratenregister)
{
    UCSR2B = 0b00011000; // RX und TX enable
    UCSR2C = 0b00000110; // Asynkrone USART, 8 Datenbits

    UBRR2H = (baudratenregister >>8);
    UBRR2L = (baudratenregister & 0x00FF);
}

//--------------------------------------------------------------------------------------------
// Ein einzelnes zeichen über RS232 Senden
//--------------------------------------------------------------------------------------------
void rs232SendeZeichen(char zeichen)
{
    UDR2 = zeichen;
    
    while( (UCSR2A & 0b01000000) == 0); // Warten bis Byte gesendet
    UCSR2A |= 0b01000000;    //Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------------------
// Einen Nullterminierten string über RS232 senden
//--------------------------------------------------------------------------------------------
void rs232SendeString(char *Text)
{
    uint8_t i = 0;
    
    while(Text[i])
    {
        rs232SendeZeichen(Text[i]);
        i++;
    }
}

//--------------------------------------------------------------------------------------------
// USB Initialisiren
//--------------------------------------------------------------------------------------------
void initUsb(void)
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
void usbSendeZeichen(char zeichen)
{
    UDR0 = zeichen;
    
    while( (UCSR0A & 0b01000000) == 0); // Warten bis Byte gesendet
    UCSR0A |= 0b01000000;                //Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------------------
// Einen Nullterminierten string über USB senden
//--------------------------------------------------------------------------------------------
void usbSendeString(char *Text)
{
    uint8_t i = 0;
    
    while(Text[i])
    {
        usbSendeZeichen(Text[i]);
        i++;
    }
}

//****************************************************************** A/D_Wandler-Treiber ******************************************************************//

void initAdc(void)
{
    //PORTF (ADC, X4) Eingang ohne PullUps
    PORTF   = 0x00;
    DDRF    = 0x00;
    //PORTK (ADC-Inputs) alles auf Eingang ohne Pullup
//     PORTK   = 0x00;
//     DDRK    = 0x00;
    
    ADMUX  = 0x40;    //AVCC Als referenz
    DIDR0  = 0x0F;    // Digitale Register an ADC pins der Potentiometer deaktivieren
//     DIDR2  = 0xFF;
    
    ADCSRA = 0b10100111; // ADC einschalten, ADC clok = 16MHz / 128 --> 8us/cycle
    ADCSRB = 0x00;    // Free runing mode
    
    ADCSRA |=  0b01000000;        // Dummy messung Starten
    while((ADCSRA&0x10) == 0);    // Warten bis Messung abgeschlossen
    
    ADCSRA &= 0xEF;                // Interruptflage löschen
}

uint16_t adcRead(adcChannel_t kanal)
{
    uint16_t messwert = 0;
    
    // Kanal definieren
    ADMUX  = 0x40;    //AVCC Als referenz
    if(kanal>=8)
    {    ADMUX  |= kanal-8;
        ADCSRB |= (3 << MUX5);
    }
    else
    {    ADMUX  |= kanal;
        ADCSRB &= ~(3 << MUX5);
    }
    
    
    ADCSRA |=  0b01000000;        // ADC Starten
    while((ADCSRA&0x10) == 0);    // Warten bis Messung abgeschllossen
    
    _delay_us(300);//25 ADC clock cycles
    
    messwert = ADCL;
    messwert |= ADCH <<8;
    
    ADCSRA &= 0xEF;                // Interruptflage löschen
    
    return messwert;
}


//****************************************************************** RGB-LED-Treiber ******************************************************************//

//--------------------------------------------------------------------------------------------
// Initialisirung des PWM der RGB-LED
//--------------------------------------------------------------------------------------------
void initRgb(void)
{
    uint16_t aufloesung = 1023; // 10bit pwm aufösung
    
    TCCR4A = 0b11111110; // Ausgänge als PWM Ausgang ,Fast PWM mode
    TCCR4B = 0b00011011; //  Clock / 64
    
    ICR4H = (aufloesung >>8);
    ICR4L = (aufloesung & 0x00FF);
    
    rgbWrite(0,0,0);
}

//--------------------------------------------------------------------------------------------
// Übergabe von Werten an die PWM Register
//--------------------------------------------------------------------------------------------
void rgbWrite(uint16_t rot,uint16_t gruen,uint16_t blau)
{
    rot        = 1023 - rot;
    gruen    = 1023 - gruen;
    blau    = 1023 - blau;
    
    OCR4AH = (rot >>8);
    OCR4AL = (rot & 0x00FF);
    
    OCR4BH = (gruen >>8);
    OCR4BL = (gruen & 0x00FF);
    
    OCR4CH = (blau >>8);
    OCR4CL = (blau & 0x00FF);
}

void rgbRot(uint16_t rot)
{
    // Wenn PWM initialisirt ist wert an PWM register übergeben
    rot        = 1023 - rot;
    OCR4AH = (rot >>8);
    OCR4AL = (rot & 0x00FF);
}

void rgbGruen(uint16_t gruen)
{
    // Wenn PWM initialisirt ist wert an PWM register übergeben
    gruen    = 1023 - gruen;
    OCR4BH = (gruen >>8);
    OCR4BL = (gruen & 0x00FF);
}

void rgbBlau(uint16_t blau)
{
    // Wenn PWM initialisirt ist wert an PWM register übergeben
    blau    = 1023 - blau;
    OCR4CH = (blau >>8);
    OCR4CL = (blau & 0x00FF);
}

//****************************************************************** LCD-Treiber ******************************************************************//

//--------------------------------------------------------------------------------------------
// Ansteuerung von einzelen Bits fur LCD
//--------------------------------------------------------------------------------------------
void lcdSid(uint8_t status)        // LCD Datenleitung
{
    if(status)    PORTG |= 0x01;
    else PORTG &= 0xFE;
}

void lcdSclk(uint8_t status)        // LCD Taktleitung
{
    if(status)    PORTG |= 0x02;
    else PORTG &= 0xFD;
}


//--------------------------------------------------------------------------------------------
// LCD Hintergundbeleuchtung mit PWM dimmen (0 bis 100%) Timer 0 muss initialisiert sein!
//--------------------------------------------------------------------------------------------
void lcdLight(uint8_t hellighkeit) // Werte von 0 bis 10 möglich -> 0 = Aus , 1= 10%, 10 = 100%
{
    if(hellighkeit == 0)
    {
        TCCR0A &= 0b11011111; //Wenn heligkeit 0 -> Hintergrundbeleuchtung ausschalten
    }
    else
    {
        TCCR0A |= 0b00100000;
        OCR0B = hellighkeit;
    }
}

//--------------------------------------------------------------------------------
// LCD ein oder aus schalten
//
// Reset: PG2 = 0V, !Reset: PG2 = Tristate, sonst 5V-Out auf 3.3V-In.
// Pullup ist im Display verbaut.
//--------------------------------------------------------------------------------
void adm_LCD_on_off(uint8_t status)		//LCD ein/Aus
{
    PORTG &= 0xFB;
    
    if(status)	DDRG &= 0xFB;
    else DDRG |= 0x04;
}
//--------------------------------------------------------------------------------------------
// Initialisierung des LCD's
//--------------------------------------------------------------------------------------------
void initLcd(void)
{
    
    
    lcdSclk(1);
    lcdSid(0);
    
    adm_LCD_on_off(0);
    _delay_ms(10);
    
    adm_LCD_on_off(1);
    _delay_ms(10);
    
    writeLcdF('C',0x34);      // set 8-Bit-Interface RE = 1
    writeLcdF('C',0x09);      // 4-Zeilen-Modus, 5-Dot Font-Breite
    
    writeLcdF('C',0x30);      // set 8-Bit-Interface RE = 0
    writeLcdF('C',0x0C);      // Display ON, Cursor OFF

    lcdClear();                // Clear Display
    
    writeLcdF('C',0x07);      // Entry Mode
    lcdLight(0);
    
}

//--------------------------------------------------------------------------------------------
// Schreibt ein Kommando oder ein Datenbyte (Zeichen) zum LCD SPI (Serial Protokoll Interface)
//--------------------------------------------------------------------------------------------
void writeLcdF(uint16_t rs, uint16_t value)
{ uint16_t i;
    // Manche Befehle müssen doppelt ausgeführt werden, damit die
    // minimale Pulslänge von 400ns eingehalten wird. (bei 20MHz)

    // Synchronisierung: 5x "1" senden
    lcdSid(1);                // Daten-Bit = 1
    for(i=0;i<5; i++)
    {
        lcdSclk(0);            // Synch-Bits senden
        lcdSclk(1);
    }
    // R/W: 1=Read, 0=Write
    lcdSid(0);                    // R/W = 0
    lcdSclk(0);                // R/W-Bit senden
    lcdSclk(1);

    // RS Register Selection: 0=Command, 1=Data
    if (rs == 'C') lcdSid(0);
    else lcdSid(1);

    lcdSclk(0);                // RS-Bit senden
    lcdSclk(1);

    // End-Marke 0
    lcdSid(0);
    
    lcdSclk(0);                // END-Bit senden
    lcdSclk(1);
    
    // Daten-Bit 0-3
    for(i=0;i<4; i++)
    {
        lcdSclk(0);
        if (value & 0x01) lcdSid(1);
        else lcdSid(0);
        value = value >> 1;
        lcdSclk(0);
        lcdSclk(1);
    }

    lcdSid(0);        // 4x "0" senden
    for(i=0;i<4; i++)
    {
        lcdSclk(0);
        lcdSclk(1);
    }
    
    // Daten-Bit 4-7
    for(i=0;i<4; i++)
    {
        lcdSclk(0);
        if (value & 0x01) lcdSid(1);
        else lcdSid(0);
        value = value >> 1;
        lcdSclk(0);
        lcdSclk(1);
    }
    
    lcdSid(0);            // 4x "0" senden
    for(i=0;i<4; i++)
    {
        lcdSclk(0);
        lcdSclk(1);
    }
    
    lcdSid(1);
    // Write-Befehl auf 50us verlängern, damit minimale Execution-Time 39us/43us eingehalten ist.
    _delay_us(50);
}

//------------------------------------------------------------
// Text an xy-Position ausgeben
//------------------------------------------------------------
void lcdWriteText(uint8_t zeile0_3, uint8_t spalte0_19, char *text)
{
    uint8_t str_p = 0;
    uint8_t pos;
    pos = spalte0_19 + (zeile0_3 * 0x20);
    writeLcdF('C',pos | 0x80);

    while(text[str_p])
    { writeLcdF('D',text[str_p++]);
    }
}

//------------------------------------------------------------
// Zahl an xy-Position ausgeben dezimal
//------------------------------------------------------------
void lcdWriteZahl(uint8_t zeile0_3, uint8_t spalte0_19, uint64_t zahl, uint8_t vorKommaStellen, uint8_t nachKommaStellen)
{
    uint8_t komma=0;
    char numberBuffer[20];//20stellen dezimal
    char send_buffer[22];//64Bit: 20Stellen dezimal + Komma + Zerotermination
    uint8_t i, posSend, posRead, stellenTotal;
    uint64_t val=zahl;
    
    stellenTotal=vorKommaStellen+nachKommaStellen;
    if(stellenTotal>20){
        lcdWriteText(zeile0_3, spalte0_19, "--------------------");
        return;
    }
    if (nachKommaStellen)
    {
        komma=1;
    }

    // Umwandlung in die einzelnen Stellen-Zahlen 1er, 10er, 100er, ...
    //  zahl = 12345;
    //  s_vk = 2;
    //  s_nk = 0;
    //  komma = 0;
    for(i=0;i<20;i++){
        numberBuffer[19-i] = (val % 10)+48;//19=einer, 18=zehner, 17=hunderter....
        val = val / 10;
    }
    //Vorkommastellen kopieren
    posSend=0;
    posRead=20-stellenTotal;
    for (i=0;i<vorKommaStellen;i++)
    {
        send_buffer[posSend] = numberBuffer[posRead];
        posSend++;
        posRead++;
    }
    //komma
    if(komma){
        send_buffer[posSend]='.';
        posSend++;
    }
    //Nachkommastellen kopieren
    for(i=0;i<nachKommaStellen;i++){
        send_buffer[posSend] = numberBuffer[posRead];
        posSend++;
        posRead++;
    }
    
    send_buffer[posSend]=0;
    // Vorangehende Nullen löschen
    i = 0;
    while ( (send_buffer[i] == 48) && (i < vorKommaStellen-1) )
    { send_buffer[i++] = 32;
    }
    lcdWriteText(zeile0_3, spalte0_19, send_buffer);
}

//------------------------------------------------------------
// Clear LCD
//------------------------------------------------------------
void lcdClear(void)
{
    writeLcdF('C',0x01);  // Clear Display
    _delay_ms(2);           // 2ms warten, bis LCD gelöscht ist
    
    lcdWriteText(0,0," ");    // Blödes Zeichen auf Disply löschen
}



//**************************** RS232 / USB -Treiber ****************************//

//--------------------------------------------------------------------------------
// RS232 Initialisiren
//--------------------------------------------------------------------------------
void adm_RS232_init (uint16_t baudratenregister)
{
    UCSR2B = 0b00011000; // RX und TX enable
    UCSR2C = 0b00000110; // Asynkrone USART, 8 Datenbits
    
    UBRR2 = baudratenregister;
}

//--------------------------------------------------------------------------------
// Ein einzelnes zeichen über RS232 Senden
//--------------------------------------------------------------------------------
void adm_RS232_send_byte(uint8_t zeichen)
{
    UDR2 = zeichen;
    
    while( (UCSR2A & 0b01000000) == 0); // Warten bis Byte gesendet
    UCSR2A |= 0b01000000;	//Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------
// Einen Nullterminierten string über RS232 senden
//--------------------------------------------------------------------------------
void adm_RS232_send_string(const char*Text)
{
    uint8_t i = 0;
    
    while(Text[i])
    {
        adm_RS232_send_byte(Text[i]);
        i++;
    }
}

//--------------------------------------------------------------------------------
// USB Initialisiren
//--------------------------------------------------------------------------------
void adm_USB_init(void)//maybe rename to init uart 0
{
    UCSR0B = 0b00011000; // RX und TX enable
    UCSR0C = 0b00000110; // Asynkrone USART, 8 Datenbits

    // 9600 Baud
    UBRR0 = 103;
}

//--------------------------------------------------------------------------------
// Ein einzelnes zeichen über USB Senden
//--------------------------------------------------------------------------------
void adm_USB_send_byte(uint8_t zeichen)
{
    UDR0 = zeichen;
    
    while( (UCSR0A & 0b01000000) == 0); // Warten bis Byte gesendet
    UCSR0A |= 0b01000000;				//Byte gesendet Flagge löschen
}

//--------------------------------------------------------------------------------
// Einen Nullterminierten string über USB senden
//--------------------------------------------------------------------------------
void adm_USB_send_string(const char*Text)
{
    uint8_t i = 0;
    
    while(Text[i])
    {
        adm_USB_send_byte(Text[i]);
        i++;
    }
}