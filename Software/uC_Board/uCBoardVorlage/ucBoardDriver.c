/*
* ucBoardDriver.c
*
* Created: 26.10.2020 08:44:34
*  Author: Dario Dündar
*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "ucBoardDriver.h"

#ifndef SWITCH_ACTIV_HIGH
    #define SWITCH_ACTIV_HIGH   1
#endif


void initAdc(void);
void initLcd(void);
void initRgb(void);
void lcdSid(uint8_t status);
void lcdSclk(uint8_t status);
void writeLcdF(uint16_t rs, uint16_t value);
void matrixWriteNextLine(void);
void startSystemTimeMs(void);
uint8_t switchReadAll(void);

static uint8_t matrixRunning = 0;
volatile uint64_t systemTimeMs = 0;

//--------------------------------------------------------------------------------------------
// Initialisirung Board
//--------------------------------------------------------------------------------------------

void initBoard(uint8_t startAnimation)
{
    //PullUpDesable auf 0
    MCUCR = MCUCR | (0<<PUD);
    //PORTA&B LED Ports auf Ausgnag und LED off
    PORTA   = 0x00;
    DDRA    = 0xFF;
    PORTB   = 0x00;
    DDRB    = 0xFF;
    //PORTC Schalter Port auf Eingnag mit Pullup
    PORTC   = 0xFF;
    DDRC    = 0x00;
    //PORTD (X1) auf Eingang mit PullUp
    PORTD   = 0xFF;
    DDRD    = 0x00;
    //PORTE
    //Eingang mit Pullups
    PORTE   = 0xFF;
    DDRE    = 0x00;
    //Taster Joystick (PE2) ohne PullUp
    PORTE   = PORTE & 0b11111011;
    //PORTF (ADC, X4) Eingang ohne PullUps
    PORTF   = 0x00;
    DDRF    = 0x00;
    //PORTG (LCD)
    PORTG   = 0b00000100;//PG2 PullUp
    DDRG   |= 0b00000011;//LCD SCLK & SID
    DDRG   |= 0b00100000;//LCD-LED
    //PORTH (RGB_LED), sonst Eingang mit PullUp
    PORTH   = 0b11000111;
    DDRH    = 0b00111000;
    //PORTJ (Matrix, X3), Ausgang
    PORTJ   = 0x00;
    DDRJ    = 0xFF;
    //PORTK (ADC-Inputs) alles auf Eingang ohne Pullup
    PORTK   = 0x00;
    DDRK    = 0x00;
    //PORTL (Taster, X4)
    PORTL   = 0b00111100;//X4 mit Pullups
    DDRL    = 0x00;//Alles Eingang
    
    //start 5ms tik
    sei();            // Global interrups aktivieren
    startSystemTimeMs();
    
    
    //init lcd
    initLcd();
    //init adc
    initAdc();
    //init rgb
    initRgb();
    
    if (startAnimation)
    {
        lcdLight(255);
        rgbWrite(0,1023,0);
        lcdWriteText(0,0,"      uC-Board      ");
        lcdWriteText(1,0,"ATmega2560 at 16 MHz");
        lcdWriteText(2,0,"--------------------");
        lcdWriteText(3,0,"Press any key to run");
        uint8_t i=0;
        uint8_t goRight=1;
        uint8_t btnPressed=0;
        uint8_t light=0;
        uint8_t goBrighter=1;
        while (1)
        {
            if (goRight)
            {
                ledWriteAll(0x8000>>i);
            } 
            else
            {
                ledWriteAll(0x0001<<i);
            }
            i++;
            if (i>16)
            {
                i=0;
                goRight=!goRight;
            }
            
            if (goBrighter)
            {
                light+=2;
            }
            else
            {
                light-=2;
            }
            lcdLight(light);
            if (light==254)
            {
                goBrighter=0;
            }
            if (light==120)
            {
                goBrighter=1;
            }
            
            if (!btnPressed && (buttonReadAllPL()&0b11000011))
            {
                rgbWrite(1023,550,0);
                lcdWriteText(3,0,"  release all keys  ");
                btnPressed=1;
            }
            if (btnPressed && !(buttonReadAllPL()&0b11000011))
            {
                break;
            }
            _delay_ms(8);
        }
        lcdClear();
        lcdLight(0);
        ledWriteAll(0);
        rgbWrite(0,0,0);
        systemTimeMs=0;
    }
    
}

void pinInitX1PortD(uint8_t bitNr0_7, ioType_t type)
{
    switch (type)
    {
        case OUTPUT:
        DDRD |= (1<<bitNr0_7);//Output
        break;
        
        case INPUT:
        DDRD &= ~(1<<bitNr0_7);//Input
        PORTD &= ~(1<<bitNr0_7);//Pullup off
        break;
        
        case INPUT_PULLUP:
        DDRD &= ~(1<<bitNr0_7);//Input
        PORTD |= (1<<bitNr0_7);//Pullup on
        break;
        
        case INPUT_ADC://No adc on portD
        default:
        break;
    }
}

void pinInitX4PortL(uint8_t bitNr2_5, ioType_t type)
{
    //Andere bits für taster...
    if ((bitNr2_5<2) || (bitNr2_5>5))
    {
        return;
    }
    switch (type)
    {
        case OUTPUT:
        DDRL |= (1<<bitNr2_5);//Output
        break;
        
        case INPUT:
        DDRL &= ~(1<<bitNr2_5);//Input
        PORTL &= ~(1<<bitNr2_5);//Pullup off
        break;
        
        case INPUT_PULLUP:
        DDRL &= ~(1<<bitNr2_5);//Input
        PORTL |= (1<<bitNr2_5);//Pullup on
        break;
        
        case INPUT_ADC://No adc on portL
        default:
        break;
    }
}

void pinInitX4PortF(uint8_t bitNr0_3, ioType_t type)
{
    if (bitNr0_3 > 3)
    {
        return;
    }
    switch (type)
    {
        case OUTPUT:
        DIDR0  &= ~(1<<bitNr0_3);    // Digitale In Reg an ADC pins aktivieren
        DDRF |= (1<<bitNr0_3);//Output
        break;
        
        case INPUT:
        DIDR0  &= ~(1<<bitNr0_3);    // Digitale In Reg an ADC pins aktivieren
        DDRF &= ~(1<<bitNr0_3);//Input
        PORTF &= ~(1<<bitNr0_3);//Pullup off
        break;
        
        case INPUT_ADC:
        DDRF &= ~(1<<bitNr0_3);//Input
        PORTF &= ~(1<<bitNr0_3);//Pullup off
        DIDR0  |= (1<<bitNr0_3);    // Digitale In Reg an ADC pins deaktivieren
        break;
        
        case INPUT_PULLUP:
        DIDR0  &= ~(1<<bitNr0_3);    // Digitale In Reg an ADC pins aktivieren
        DDRF &= ~(1<<bitNr0_3);//Input
        PORTF |= (1<<bitNr0_3);//Pullup on
        break;
        
        default:
        break;
    }
}

void pinWriteX1PortD(uint8_t bitNr0_7, uint8_t val0_1)
{
    if (val0_1)
    {
        PORTD |= (1<<bitNr0_7);
    }
    else
    {
        PORTD &= ~(1<<bitNr0_7);
    }
}

void pinWriteX4PortL(uint8_t bitNr2_5, uint8_t val0_1)
{
    //Andere bits für taster...
    if ((bitNr2_5<2) || (bitNr2_5>5))
    {
        return;
    }
    if (val0_1)
    {
        PORTL |= (1<<bitNr2_5);
    }
    else
    {
        PORTL &= ~(1<<bitNr2_5);
    }
}

void pinWriteX4PortF(uint8_t bitNr0_3, uint8_t val0_1)
{
    if (bitNr0_3 > 3)
    {
        return;
    }
    if (val0_1)
    {
        PORTF |= (1<<bitNr0_3);
    }
    else
    {
        PORTF &= ~(1<<bitNr0_3);
    }
}

void ledWrite(uint8_t ledNr0_15, uint8_t wer0_1)
{
    uint16_t mask;
    if (wer0_1)
    {
        mask = (1 << ledNr0_15);
        PORTA = PORTA | mask;
        PORTB = PORTB | (mask >> 8);
    } 
    else
    {
        mask = ~(1 << ledNr0_15);
        PORTA = PORTA & mask;
        PORTB = PORTB & (mask >> 8);
    }
}

void ledWriteAll(uint16_t bitMuster)
{
    PORTA = bitMuster;
    PORTB = (bitMuster>>8);
}

uint16_t ledReadAll(void)
{
    return (PORTA | (PORTB<<8));
}

uint8_t ledRead(uint8_t ledNr0_15){
    return (PORTA | (PORTB<<8)) & (1<<ledNr0_15);
}

uint8_t pinReadX1PortD(uint8_t bitNr0_7)
{
    return PIND & (1<<bitNr0_7);
}

uint8_t pinReadX4PortL(uint8_t bitNr2_5)
{
    return PINL & (1<<bitNr2_5);
}

uint8_t pinReadX4PortF(uint8_t bitNr0_3)
{
    return PINF & (1<<bitNr0_3);
}

uint8_t switchRead(uint8_t switchNr0_7)
{
    return switchReadAll() & (1<<switchNr0_7);
}    

uint8_t switchReadAll(void)
{
    if (SWITCH_ACTIV_HIGH)
    {
        return PINC;
    }
    return ~PINC;
}

uint8_t buttonReadAllPL(void)
{
    return PINL;
}

uint8_t buttonReadPL(uint8_t buttonNr0_7)
{
    return PINL & (1<<buttonNr0_7);
}

uint8_t buttonReadJoyStickPE2(void)
{
    return (PINE&0b00000100);
}



// void init_mocca(void)
// {
//     DDRA = 0xFF;    // LED Port als Ausgang
//     DDRB = 0xFF;    // LED Port als Ausgang
//     DDRK = 0x00;    // Schalter Port als Eingang
//     DDRE = 0x00;    // Taster als Eingang
//     DDRG = 0x27;    // LCD port als Ausgang
//     DDRH = 0x78;    // RGB, CTS als Ausgang
//
//     DDRJ = 0xFF;    // LED-Matrix
//
//     init_5ms_timer();
//
//     sei();            // Global interrups aktivieren
//
//     init_lcd();
//     lcd_light(10);
// }

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

    if(matrixRunning)matrixWriteNextLine();
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
    PORTK   = 0x00;
    DDRK    = 0x00;
    
    ADMUX  = 0x40;    //AVCC Als referenz
    DIDR0  = 0x0F;    // Digitale Register an ADC pins der Potentiometer deaktivieren
    DIDR2  = 0xFF;
    
    ADCSRA = 0b10100111; // ADC einschalten, ADC clok = 16MHz / 128 --> 8us/cycle
    ADCSRB = 0x00;    // Free runing mode
    
    ADCSRA |=  0b01000000;        // Dummy messung Starten
    while((ADCSRA&0x10) == 0);    // Warten bis Messung abgeschllossen
    
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

//--------------------------------------------------------------------------------------------
// Initialisierung des LCD's
//--------------------------------------------------------------------------------------------
void initLcd(void)
{
    lcdSclk(1);
    lcdSid(0);
    
    
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
void lcdWriteText(uint8_t zeile0_3, uint8_t spalte0_19, char const *formattedText, ...)
{
    #define MAX_STRING_SIZE 21
    char text[MAX_STRING_SIZE];
    va_list arglist;
    va_start(arglist, formattedText);
    vsnprintf(text, MAX_STRING_SIZE, formattedText, arglist);
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

#define ANZAHL_ZEILEN 4
#define ANZAHL_SPALTEN 16
void lcdLog(char const *formattedText, ...)
{
    static char lcdPrintText[ANZAHL_ZEILEN][ANZAHL_SPALTEN];
    char newText[ANZAHL_SPALTEN];
    static uint16_t lcdPrintNr[ANZAHL_ZEILEN];
    static uint8_t firstCall=1;
    static uint16_t nummer;
    
    if (firstCall)
    {
        lcdClear();
        firstCall=0;
        nummer = 0;
        //Array komplett auf 0 setzen
        for (uint8_t i=0; i<ANZAHL_ZEILEN; i++)
        {
            lcdPrintNr[i]=0;
            for (uint8_t j=0; j<ANZAHL_SPALTEN; j++)
            {
                lcdPrintText[i][j] = ' ';
                if (j==ANZAHL_SPALTEN-1)
                {
                    lcdPrintText[i][j] = 0;
                }
            }
        }
    }
    nummer++;
    
    //prepare new text
    va_list arglist;
    va_start(arglist, formattedText);
    vsnprintf(newText, ANZAHL_SPALTEN, formattedText, arglist);
    //mit Leerzeichen auffüllen
    uint8_t finish=0;
    for (uint8_t i=0;i<ANZAHL_SPALTEN;i++)
    {
        if (newText[i] == 0)
        {
            finish=1;
        }
        if (finish)
        {
            newText[i]=' ';
        }
    }
    newText[ANZAHL_SPALTEN-1]=0;
    //compare new against line 0
    uint8_t textUpdate=strcmp(newText,lcdPrintText[0]);
    
    if (textUpdate)
    {
        //array text 1 zeile nach oben schieben
        for (uint8_t i=ANZAHL_ZEILEN-1; i>0; i--)
        {
            lcdPrintNr[i]=lcdPrintNr[i-1];
            for (uint8_t j=0; j<ANZAHL_SPALTEN; j++)
            {
                lcdPrintText[i][j] = lcdPrintText[i-1][j];
            }
        }
        //neuer text in array
        for (uint8_t i=0; i<ANZAHL_SPALTEN; i++)
        {  
            lcdPrintText[0][i] = newText[i];
        }
        lcdPrintNr[0]=nummer;
        //komplettes array ausgeben
        for (uint8_t i=0; i<ANZAHL_ZEILEN; i++)
        {
            uint8_t j = 3-i;
            lcdWriteText(i,5,lcdPrintText[j]);
            lcdWriteText(i,4,":");
            lcdWriteZahl(i,0,lcdPrintNr[j],4,0);
        }
    }else{
        //if same text, update only number
        lcdPrintNr[0]=nummer;
        lcdWriteZahl(ANZAHL_ZEILEN-1,0,lcdPrintNr[0],4,0);
    }
}

void lcdLogWaitBtn(uint8_t waitForBtn, char const *formattedText, ...){
    char newText[ANZAHL_SPALTEN];
    //prepare new text
    va_list arglist;
    va_start(arglist, formattedText);
    vsnprintf(newText, ANZAHL_SPALTEN, formattedText, arglist);
    lcdLog(newText);
    if (waitForBtn)
    {
        uint64_t oldSysTime=getSystemTimeMs();
        while (!(buttonReadAllPL()&0b11000011))
        {
            ;
        }
        while(buttonReadAllPL()&0b11000011)
        {
            ;
        }
        systemTimeMs = oldSysTime;
    }
}

void lcdLogWaitMs(uint64_t waitTimeMs, char const *formattedText, ...){
    char newText[ANZAHL_SPALTEN];
    //prepare new text
    va_list arglist;
    va_start(arglist, formattedText);
    vsnprintf(newText, ANZAHL_SPALTEN, formattedText, arglist);
    lcdLog(newText);
    if (waitTimeMs)
    {
        uint64_t oldSysTime=getSystemTimeMs();
        waitForSystemTimeMs(oldSysTime+waitTimeMs);
        systemTimeMs = oldSysTime;
    }
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

//****************************************************************** MATRIX-Treiber ******************************************************************//


#define ENABLE_0          PORTJ &= ~0x08; // PJ3 Zeilen/Mux ausschalten
#define ENABLE_1          PORTJ |=  0x08; // PJ3 Zeilen/Mux einschalten
#define CLOCK_0           PORTJ &= ~0x10; // PJ4 Clock = 0
#define CLOCK_1           PORTJ |=  0x10; // PJ4 Clock = 1
#define STROBE_0          PORTJ &= ~0x40; // PJ6 Strobe = 0
#define STROBE_1          PORTJ |=  0x40; // PJ6 Strobe = 1

#define DATA_Modul_1_ON   PORTJ |=  0x20; // PJ5 Data = 1
#define DATA_Modul_1_OFF  PORTJ &= ~0x20; // PJ5 Data = 0



//------------------------------------------------------------------------------------

//                                    Länge,Spalten ...                              Nr.
const uint8_t ASCII_Tab[64][10]= {
    {4,  0,  0,  0,  0,  0,  0,  0,    0,  0},        //  0    " "
    {5,  0,  0, 95,  0,  0,  0,  0,    0,  0},        //  1    !
    {5,  0,  7,  0,  7,  0,  0,  0,    0,  0},        //  2    "
    {5, 20,127, 20,127, 20,  0,  0,    0,  0},        //  3    #
    {5,  4, 42,107, 42, 16,  0,  0,    0,  0},        //  4    $
    {5, 35, 19,  8,100, 98,  0,  0,    0,  0},        //  5    %
    {5, 54, 73, 85, 34, 80,  0,  0,    0,  0},        //  6    &
    {4,  0,  5,  3,  0,  0,  0,  0,    0,  0},        //  7    '
    {5,  0, 28, 34, 65,  0,  0,  0,    0,  0},        //  8    (
    {5,  0, 65, 34, 28,  0,  0,  0,    0,  0},        //  9    )
    {5, 20,  8, 62,  8, 20,  0,  0,    0,  0},        // 10    *
    {5,  8,  8, 62,  8,  8,  0,  0,    0,  0},        // 11    +
    {4,  0, 80, 48,  0,  0,  0,  0,    0,  0},        // 12    ,
    {5,  8,  8,  8,  8,  8,  0,  0,    0,  0},        // 13    -
    {4,  0, 96, 96,  0,  0,  0,  0,    0,  0},        // 14    .
    {5, 16,  8,  4,  2,  1,  0,  0,    0,  0},        // 15    /
    {5, 62, 65, 65, 65, 62,  0,  0,    0,  0},        // 16    0
    {3, 66,127, 64,  0,  0,  0,  0,    0,  0},        // 17    1
    {5, 66, 97, 81, 73, 70,  0,  0,    0,  0},        // 18    2
    {5, 34, 65, 73, 73, 54,  0,  0,    0,  0},        // 19    3
    {5, 24, 20, 18,127, 16,  0,  0,    0,  0},        // 20    4
    {5, 39, 69, 69, 69, 57,  0,  0,    0,  0},        // 21    5
    {5, 60, 74, 73, 73, 48,  0,  0,    0,  0},        // 22    6
    {5,  1,113,  9,  5,  3,  0,  0,    0,  0},        // 23    7
    {5, 54, 73, 73, 73, 54,  0,  0,    0,  0},        // 24    8
    {5,  6, 73, 73, 41, 30,  0,  0,    0,  0},        // 25    9
    {5, 54, 54,  0,  0,  0,  0,  0,    0,  0},        // 26    :
    {5, 86, 54,  0,  0,  0,  0,  0,    0,  0},        // 27    ;
    {5,  8, 20, 34, 65,  0,  0,  0,    0,  0},        // 28    <
    {5, 20, 20, 20, 20, 20,  0,  0,    0,  0},        // 29    =
    {5, 65, 34, 20,  8,  0,  0,  0,    0,  0},        // 30    >
    {5,  2,  1, 81,  9,  6,  0,  0,    0,  0},        // 31    ?
    {5, 62, 65, 93, 85, 30,  0,  0,    0,  0},        // 32    @
    {5,126, 17, 17, 17,126,  0,  0,    0,  0},        // 33    A
    {5,127, 73, 73, 73, 54,  0,  0,    0,  0},        // 34    B
    {5, 62, 65, 65, 65, 34,  0,  0,    0,  0},        // 35    C
    {5,127, 65, 65, 34, 28,  0,  0,    0,  0},        // 36    D
    {5,127, 73, 73, 73, 65,  0,  0,    0,  0},        // 37    E
    {5,127,  9,  9,  9,  1,  0,  0,    0,  0},        // 38    F
    {5, 62, 65, 73, 73,122,  0,  0,    0,  0},        // 39    G
    {5,127,  8,  8,  8,127,  0,  0,    0,  0},        // 40    H
    {3, 65,127, 65,  0,  0,  0,  0,    0,  0},         // 41    I
    {5, 32, 64, 65, 63,  1,  0,  0,    0,  0},        // 42    J
    {5,127,  8, 20, 34, 65,  0,  0,    0,  0},        // 43    K
    {5,127, 64, 64, 64, 64,  0,  0,    0,  0},        // 44    L
    {5,127,  2, 12,  2,127,  0,  0,    0,  0},        // 45    M
    {5,127,  4,  8, 16,127,  0,  0,    0,  0},         // 46    N
    {5, 62, 65, 65, 65, 62,  0,  0,    0,  0},        // 47    O
    {5,127,  9,  9,  9,  6,  0,  0,    0,  0},        // 48    P
    {5, 62, 65, 81, 33, 94,  0,  0,    0,  0},        // 49    Q
    {5,127,  9, 25, 41, 70,  0,  0,    0,  0},        // 50    R
    {5, 70, 73, 73, 73, 49,  0,  0,    0,  0},        // 51    S
    {5,  1,  1,127,  1,  1,  0,  0,    0,  0},        // 52    T
    {5, 63, 64, 64, 64, 63,  0,  0,    0,  0},        // 53    U
    {5, 31, 32, 64, 32, 31,  0,  0,    0,  0},        // 54    V
    {5, 63, 64, 48, 64, 63,  0,  0,    0,  0},        // 55    W
    {5, 99, 20,  8, 20, 99,  0,  0,    0,  0},        // 56    X
    {5,  7,  8,112,  8,  7,  0,  0,    0,  0},        // 57    Y
    {5, 97, 81, 73, 69, 67,  0,  0,    0,  0},        // 58    Z
    {2,127,127,  0,  0,  0,  0,  0,    0,  0},         // 59
    {6,124,126, 27, 27,126,124,  0,    0,  0},         // 60
    {6,124,126, 27, 27,126,124,  0,    0,  0},         // 61
    {7,127,127,  6, 12, 24, 63,127,    0,  0},      // 62
    {7, 62,73, 85,    81,    85,    73,    62, 0,  0}
};        // 63    Smile

uint16_t matrix[Anzahl_Spalten];         // Diese Tabelle representiert die einzelnen Pixel der Matrix

volatile uint8_t writeTextFinished = 0;

void matrixStart(void)
{
    matrixRunning = 1;
}

void matrixStop(void)
{
    matrixRunning = 0;
}

void matrixFill(uint8_t wert)
{
    uint8_t i;
    if (!matrixRunning)
    {
        return;
    }
    writeTextFinished = 0;
    while(writeTextFinished == 0);                          // Warten, bis alle 8 Zeilen geschrieben wurden.
    for(i=0; i<Anzahl_Spalten; i++) matrix[i] = wert;
}

// write_Zeichen_Matrix --------------------------------------------------------------------------------------
// =============
// Beschreibung:  Kopiert die Pixelinformationen eines Zeichens in die gewünschte Position der Matrix
//
// Input:         Matrix_Spalten_Nr:  Spalte der Matrix, wo das Zeichen hinkopiert werden soll.
//                Zeichen_Nr:         Zeichen-Nummer 0 .. max
// Output:        Pixelinformationen dieses Zeichens stehen in der Matrix und werden vom Hintergrundtreiber
//                fortlaufend herausgeschrieben.
//-------------------------------------------------------------------------------------------------------------
void matrixWriteZeichen(uint16_t spalte, uint16_t zeichenNr)
{
    uint8_t i, breite;
    if (!matrixRunning)
    {
        return;
    }
    writeTextFinished = 0;
    while(writeTextFinished == 0);                          // Warten, bis alle 8 Zeilen geschrieben wurden.
    breite = ASCII_Tab[zeichenNr][0];                          // Breite eines Zeichens aus Tabelle lesen
    for (i=0; i<breite; i++)                                    // Je nach Zeichenbreite alle Spalten kopieren
    { if(spalte+i < Anzahl_Spalten)              // Nicht über die Matrix hinaus schreiben !!!
        { matrix[spalte+i]= ASCII_Tab[zeichenNr][i+1];
        }
    }
}

// write_Text_Matrix ---------------------------------------------------------------------------------------
// =============
// Beschreibung:  Kopiert die Pixelinformationen eines Zeichens in die gewünschte Position der Matrix
//
// Input:         Zeichen_Nr:         Zeichen-Nummer 0 .. max
//                Matrix_Spalten_Nr:  Spalte der Matrix, wo das Zeichen hinkopiert werden soll.
// Output:        Pixelinformationen dieses Zeichens stehen in der Matrix und werden vom Hintergrundtreiber
//                fortlaufend herausgeschrieben.
// Input:         Matrix_Spalten_Nr : horizontale Position in der Matrix
//                *str_ptr          : Text
//                logic             : 3 =  (Bitmuster der Ziffern) übernehmen 1 und 0 schreiben
//                logic             : 2 =  (Bitmuster der Ziffern) XOR (altem Wert in der Matrix)
//                logic             : 1 =  (Bitmuster der Ziffern)  OR (altem Wert in der Matrix) --> Pixel setzen
//                logic             : 0 = !(Bitmuster der Ziffern) AND (altem Wert in der Matrix) --> Pixel löschen
//-------------------------------------------------------------------------------------------------------------
void matrixWriteText(int16_t spalte, char *text, uint16_t logic)
{ int16_t matrix_buffer_pos;
    uint8_t breite, i, z_nr;
    uint8_t str_p = 0;                                      // Dieser Zeiger zeigt auf das zu schreibende Zeichen im Text
    if (!matrixRunning)
    {
        return;
    }
    writeTextFinished = 0;
    while(writeTextFinished == 0);                          // Warten, bis alle 8 Zeilen geschrieben wurden.
    matrix_buffer_pos = spalte;
    while (text[str_p] != 0)
    {
        breite = ASCII_Tab[text[str_p]-32][0];               // Breite eines Zeichens aus Tabelle lesen
        z_nr = text[str_p]-32;                               // Zeichen-Nr aus Text lesen
        for (i=0; i<breite; i++)                                // Je nach Zeichenbreite alle Spalten kopieren
        { if( (matrix_buffer_pos+i >= 0) && (matrix_buffer_pos+i < Anzahl_Spalten) )    // Nicht über die Matrix hinaus schreiben !!!
            { switch(logic)
                { case 0: matrix[matrix_buffer_pos+i] &= ~ASCII_Tab[z_nr][i+1]; break;  // Matrix AND !Pixel --> löschen
                    case 1: matrix[matrix_buffer_pos+i] |=  ASCII_Tab[z_nr][i+1]; break;  // Matrix  OR  Pixel --> setzen
                    case 2: matrix[matrix_buffer_pos+i] ^=  ASCII_Tab[z_nr][i+1]; break;  // Matrix XOR  Pixel --> vertauschen
                    case 3: matrix[matrix_buffer_pos+i]  =  ASCII_Tab[z_nr][i+1]; break;  // Matrix  =   Pixel --> setzen & löschen
                }
            }
        }
        matrix_buffer_pos += breite + Zeichenabstand;           // Position für das nächste Zeichen berechnen
        str_p++;                                                // Zeichen-Zeiger auf das nächste Zeichen richten
    }
}




void matrixWriteNextLine(void)
{
    const  uint8_t Bit_Muster_Tab[8] = {1,2,4,8,16,32,64,128};
    static uint16_t Zeilen_Nr, Bit_Muster;
    uint8_t i;
    
    STROBE_0;                             // Daten des Scheiberegisters nicht ins Latch schreiben

    Bit_Muster = Bit_Muster_Tab[Zeilen_Nr]; // Bitmuster aus Tabelle holen, um mit Daten in matrix[] zu vergleichen.
    
    // Alle Bits seriell ins Schieberegister hineinschreiben
    for(i=0; i<Anzahl_Spalten; i++)         // Alle Spalten durchgehen und überpüfen, ob Bit gesetzt ist
    {                                       // Vergleicht Bit-Muster der aktuellen Zeile mit Daten in matrix[]
        if((matrix[Anzahl_Spalten-1- i] & Bit_Muster) != 0)     // Ist in matrix[] das entsprechende Bit gesetzt?
        { DATA_Modul_1_ON;                    //   JA:   ==> Datenbit "1" ins Schieberegister schreiben
        }
        else
        { DATA_Modul_1_OFF;                   //   NEIN: ==> Datenbit "0" ins Schieberegister schreiben
        }
        CLOCK_1;
        CLOCK_0;                            // Datenbit wird bei der negativen Flanke des Clock-Signals übernommen
    }
    
    ENABLE_0;                                // +5V aller Zeilen abschalten
    PORTJ &= ~0x07;
    PORTJ |= Zeilen_Nr;                     // gewünschte Zeile einstellen
    STROBE_1;                             // Daten ins Latch schreiben
    ENABLE_1;                             // +5V Speisung der eingestellten Zeile einschalten

    // Die Zeilen werden nacheinander eingeschaltet --> zyklisch 0,1,2,3,4,5,6,7, 0,1,2,3, ...
    if (Zeilen_Nr < 7)
    {
        Zeilen_Nr++;
    }
    else
    {
        Zeilen_Nr = 0;
        writeTextFinished = 1;
    }
    
}