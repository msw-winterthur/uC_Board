﻿/*
 * ucBoardDriver.h
 *
 * Created: 26.10.2020 08:44:55
 *  Author: Dario Dündar
 */ 


#ifndef UCBOARDDRIVER_H_
#define UCBOARDDRIVER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*************************************************************************************************/
/* Initialisierung                                                                               */
/*************************************************************************************************/

/**
 * \brief Initialisiert das Board.
 * - Die Pins an X1 und X4 werden auf ADC oder Eingang mit PullUp gesetzt.
 * - Alle Hardware auf dem Board wird initialisiert
 * - Timer 0: Systemzeit, 5ms Tick, lcdLight, Matrix
 * - Timer 4: rgbLED
 *  
 * \param startAnimation Zeigt eine kleine Startanimation wenn wahr
 */
void initBoard(uint8_t startAnimation);


/*************************************************************************************************/
/* Zeit (zusätlich zu _delay_ms und _delay_us)                                                   */
/*************************************************************************************************/

/**
 * \brief Gibt die Zeit seit dem Aufstarten in Millisekunden zurück.
 * 
 * \return uint64_t Zeit seit dem Start in ms
 */
uint64_t getSystemTimeMs(void);


/**
 * \brief Wartet auf eine bestimmte Systemzeit, eignet sich um getaktete Programme zu schreiben.
 * 
 * \param systemTimeToWaitForMS Auf diese Systemzeit wird gewartet.
 * 
 * \return uint64_t Gibt 0 zurück wenn alles ok ist. Falls die abzuwartende Zeit bereits vorbei
 *                  war, wird angegeben wie lange (ms) dieser Zeitpunkt bereits vergangen war.
 */
uint64_t waitForSystemTimeMs(uint64_t systemTimeToWaitForMS);


/*************************************************************************************************/
/* LEDs                                                                                          */
/*************************************************************************************************/

/**
 * \brief Zeige ein Bitmuster an den LEDs an (PortA0 LSB bis PortB7 MSB)
 * 
 * \param bitMuster     Das Bitmuster, das angezeigt werden soll
 */
void ledWriteAll(uint16_t bitMuster);

/**
 * \brief Gibt zurück, was im Moment an den LEDs angezeigt wird  (PortA0 LSB bis PortB7 MSB)
 *
 * \return uint16_t Bitmuster, das an den LEDs angezeigt wird
 */
uint16_t ledReadALL(void);

/**
 * \brief Setzt eine einzelne LED
 * 
 * \param ledNr0_15 Welche LED soll gesetzt werden? LED 0 = PORTA Bit 0 und LED 15 = PORTB Bit 7
 * \param wer0_1 Auf welchen Wert soll die LED gesetzt werden? 1 = LED leuchtet
 */
void ledWrite(uint8_t ledNr0_15, uint8_t wer0_1);

/**
 * \brief Liest den Zustand einer enzelnen LED ein
 * 
 * \param ledNr0_15 Welche LED soll eingelesen werden?
 * 
 * \return uint8_t Wahr wenn die entsprechende LED leuchtet
 */
uint8_t ledRead(uint8_t ledNr0_15);

/*************************************************************************************************/
/* Schalter und Buttons                                                                          */
/*************************************************************************************************/

/**
 * \brief Gibt an ob die Schalter (PortC) aktiv High oder Low sind (default: 1)
 */
#define SWITCH_ACTIV_HIGH   1

/**
 * \brief Liest die Schalter (PortC) ein
 *
 * \return uint8_t Eingestellter Schalterwert
 */
uint8_t switchReadAll(void);

/**
 * \brief Liest einen einzelnen Schalter ein.
 * 
 * \param switchNr0_7 Welcher Schalter soll eingelesen werden?
 *                    Schalter 0 = PORTC Bit 0 / Schalter 7 = PORTC Bit 7
 * 
 * \return uint8_t Wahr wenn der Schalter ein ist, falsch wenn der Schalter aus ist.
 */
uint8_t switchRead(uint8_t switchNr0_7);

/**
 * \brief Liest die Buttons an PortL ein (S1=PL0, S2=PL1, S3=PL6, S4=PL7)
 * 
 * \return uint8_t Den gesamten PortL, ACHTUNG Bit 3-5 sind auf Anschluss X4
 */
uint8_t buttonReadAllPL(void);

/**
 * \brief Liest einen Button an PortL ein (S1=PL0, S2=PL1, S3=PL6, S4=PL7)
 * 
 * \param buttonNr0_7 0=S1, 1=S2, 6=S3, 7=S4
 * 
 * \return uint8_t Wahr wenn der entsprechende Button gedrückt ist
 */
uint8_t buttonReadPL(uint8_t buttonNr0_7);

/**
 * \brief Lese den JoyStick-Button ein (PortE Bit 2)
 * 
 * \return uint8_t Wahr wenn der Button gedrückt wird
 */
uint8_t buttonReadJoyStickPE2(void);


/*************************************************************************************************/
/* Analog-Digital-Wandler (ADC)                                                                  */
/*************************************************************************************************/

// Alle Analog-Eingänge und ihre Funktionen
typedef enum adcChannel_t {
    ADC_00_X4_PORTF_BIT0 = 0,   //Anschluss X4
    ADC_01_X4_PORTF_BIT1,       //Anschluss X4
    ADC_02_X4_PORTF_BIT2,       //Anschluss X4
    ADC_03_X4_PORTF_BIT3,       //Anschluss X4
    ADC_04_NOT_USED,            //Nicht in Gebrauch
    ADC_05_NOT_USED,            //Nicht in Gebrauch
    ADC_06_NOT_USED,            //Nicht in Gebrauch
    ADC_07_NOT_USED,            //Nicht in Gebrauch
    ADC_08_POTI_1,              //Board: Potentiometer 1 (PK0, R1)
    ADC_09_POTI_2,              //Board: Potentiometer 2 (PK1, R6)
    ADC_10_JOYSTICK_X,          //Board: Joystick X (PK2)
    ADC_11_JOYSTICK_Y,          //Board: Joystick Y (PK3)
    ADC_12_LICHT,               //Board: Sichtsensor (PK4)
    ADC_13_TEMPERATUR,          //Board: Temperatursensor (PK5)
    ADC_14_MAGNET_1,            //Board: Magnetsensor 1 (PK6, IC3)
    ADC_15_MAGNET_2             //Board: Magnetsensor 2 (PK6, IC4)
    } adcChannel_t;

/**
 * \brief Liest einen Analogen Wert von einem ADC ein. Auflösung 10 Bit (0..1023)
 * 
 * \param kanal Einen ADC-Kanal, siehe adcChannel_t
 * 
 * \return uint16_t Eingelesener 10 Bit Wert zwischen 0...1023
 */
uint16_t adcRead(adcChannel_t kanal);


/*************************************************************************************************/
/* RGB-LED                                                                                       */
/*************************************************************************************************/

/**
 * \brief Setzt die Farbe der RGB-LED
 * 
 * \param rot       10-Bit Rotanteil (0...1023)
 * \param gruen     10-Bit Grünanteil (0...1023)
 * \param blau      10-Bit Blauanteil (0...1023)
 */
void rgbWrite(uint16_t rot,uint16_t gruen,uint16_t blau);

/**
 * \brief  Setzt den Rotanteil
 * 
 * \param rot       10-Bit Rotanteil (0...1023)
 */
void rgbRot(uint16_t rot);

/**
 * \brief  Setzt den Grünanteil
 * 
 * \param guren     10-Bit Grünanteil (0...1023)
 */
void rgbGruen(uint16_t gruen);

/**
 * \brief  Setzt den Blauanteil
 * 
 * \param blau      10-Bit Blauanteil (0...1023)
 */
void rgbBlau(uint16_t blau);


/*************************************************************************************************/
/* LCD-Anzeige                                                                                   */
/*************************************************************************************************/

/**
 * \brief Setzt die Helligkeit der Hintergrundbeleuchtung des LCDs
 * 
 * \param helligkeit Helligkeit von 0...255
 */
void lcdLight(uint8_t helligkeit);

/**
 * \brief   Schreibt einen Text auf das LCD. Der Text darf formatiert sein:
 *          
 *          variable=15;
 *          lcdWriteText(0,0,"Hallo %u, %u", variable, 7);
 *          schreibt auf dem LCD Zeile 0, Spalte 0 den Text:
 *          Hallo 15, 7
 *          
 *          anstatt %u können auch andere Platzhalter eingefügt werden, das
 *          Format ist immer:
 *          
 *          %[0][n][d] wobei
 *          %   immer gebraucht wird
 *          [d] den Datentypen angibt (immer gebraucht, siehe Tabelle), z.B.: %u,    ergibt: "15"
 *          [n] minimale Anzahl Stellen (kann weggelassen werden), z.B.: %3u         ergibt: " 15"
 *          [0] vordere Stellen mit 0 auffüllen (kann weggelassen werden, z.B.: %03u ergibt: "015"
 *
 *          Datentyp:   Platzhalter:        Bedeutung:          Beispiele:
 *          uint8_t     %u                  unsigned            %u, %2u, %03u
 *          int8_t      %i                  integer             %i, %3i, %02i
 *          uint16_t    %u                  unsigned            %u, %2u, %03u
 *          int16_t     %i                  integer             %i, %3i, %02i
 *          uint32_t    %lu                 long unsigned       %lu, %2lu, %03lu    
 *          int32_t     %li                 long integer        %li, %3li, %02li
 *          uint64_t    NICHT UNTERTÜTZT -> uint64_t Variablen mit %lu (nur 32Bit!!!) ausgeben
 *          int64_t     NICHT UNTERTÜTZT -> int64_t Variablen mit %li (nur 32Bit!!!) ausgeben
 *          float       NICHT UNTERTÜTZT -> float Variablen mit %i, %li (nur Ganzzahl!!!) ausgeben
 *          char        %c                  char                %c
 *          string      %s                  string              %s
 * 
 * \param zeile0_3      Auf welcher Zeile von 0...3 soll geschrieben werden?
 * \param spalte0_19    Ab welcher Spalte von 0...19 soll geschirebene werden?
 * \param formattedText Formatierter Text, z.B: "Hallo %d", 5
 */
void lcdWriteText(uint8_t zeile0_3, uint8_t spalte0_19, char const *formattedText, ...);

/**
 * \brief Speichert ein eigenes Symbol auf dem LCD. Das Symbol kann nach dem Speichern mit
 * lcdWriteText(0,0,"%c",symbolAddr1_7); angezeigt werden.
 * 
 * \param symbolAddr1_7 Adresse an welcher das Symbol gespeichert wird.
 * \param BITMAP Bitmap des Symbols, siehe https://maxpromer.github.io/LCD-Character-Creator/ 
 * 
 * \return void
 */
void lcdCreateCustomChar(uint8_t symbolAddr1_7, const uint8_t BITMAP[]);

/**
 * \brief   Schreibt einen Text auf das LCD. Der Text startet oben links, Zeilenumbrüche
 *          werden automatisch eingefügt.
 * 
 * \param formattedText     Formatierter Text, z.B: "Hallo %d", 5
 *
 */
void lcdShowText(char const *formattedText, ...);

/**
 * \brief Schreibt eine Zahl mit maximal 20 Stellen auf das Display
 * 
 * \param zeile0_3          Auf welcher Zeile von 0...3 soll geschrieben werden?
 * \param spalte0_19        Ab welcher Spalte von 0...19 soll geschirebene werden?
 * \param zahl              Die Zahl die geschrieben werden soll
 * \param vorKommaStellen   Wieviele Stellen sollen vor dem Komma stehen?
 * \param nachKommaStellen  Wieviele Stellen sollen nach dem Komma stehen?
                            z.B: zahl=1234; nachKommaStellen=2 ergibt: 12.34
 */
void lcdWriteZahl(  uint8_t zeile0_3, uint8_t spalte0_19, uint64_t zahl,
                    uint8_t vorKommaStellen, uint8_t nachKommaStellen);





/**
 * \brief   Setze die Anzahl Zeilen, die von den Log-Funktionen verwendet werden.
 *          Es werden immer die untersten Zeilen verwendet, die freien oberen Zeilen
 *          können dann mit den lcdWrite Funktionen beschrieben werden.
 * 
 * \param linesToUse1_4 Anzahl Zeilen die von den lcdLog-Funktionen verwendet werden.
 */
void lcdLogSetLinesToUse(uint8_t linesToUse1_4);

/**
 * \brief   Ein Log wird auf das LCD geschrieben. Der Log erhält automatisch eine Zeilennummer.
 *          Falls mehrmals die selbe Nachricht hintereinander gelogt wird, wird nur die
 *          Zeilennummer aktualisiert. Die Idee ist, dass das Display nur für den Log und für
 *          nichts anderes verwendet wird. Der Text darf formatiert sein:
 *          https://de.wikipedia.org/wiki/Printf
 * 
 * \param formatedText  Formatierter Text, z.B: "Hallo %d", 5
 * 
 * \return void
 */
void lcdLog(char const *formatedText, ...);

/**
 * \brief   Ein Log wird auf das LCD geschrieben. Der Log erhält automatisch eine Zeilennummer.
 *          Falls mehrmals die selbe Nachricht hintereinander gelogt wird, wird nur die
 *          Zeilennummer aktualisiert. Die Idee ist, dass das Display nur für den Log und für
 *          nichts anderes verwendet wird. Der Text darf formatiert sein:
 *          https://de.wikipedia.org/wiki/Printf
 * 
 * \param formatedText  Formatierter Text, z.B: "Hallo %d", 5
 * \param waitForBtn    Pausiert das Programm nach dem Log bis ein Button gedrückt wird
 * 
 * \return void
 */
void lcdLogWaitBtn(uint8_t waitForBtn, char const *formattedText, ...);

/**
 * \brief   Ein Log wird auf das LCD geschrieben. Der Log erhält automatisch eine Zeilennummer.
 *          Falls mehrmals die selbe Nachricht hintereinander gelogt wird, wird nur die
 *          Zeilennummer aktualisiert. Die Idee ist, dass das Display nur für den Log und für
 *          nichts anderes verwendet wird. Der Text darf formatiert sein:
 *          https://de.wikipedia.org/wiki/Printf
 * 
 * \param formatedText  Formatierter Text, z.B: "Hallo %d", 5
 * \param waitTimeMs    Pausiert das Programm nach dem Log für diese Anzahl ms
 * 
 * \return void
 */
void lcdLogWaitMs(uint64_t waitTimeMs, char const *formattedText, ...);


/**
 * \brief Löscht alles auf dem LCD
 */
void lcdClear(void);


/*************************************************************************************************/
/* Matrix (extern an X3)                                                                         */
/*************************************************************************************************/

// Vom Benutzer vorgegebene Parameter, die sich je nach Matrix-Grösse unterscheiden
#define Anzahl_Spalten     56       // Wieviele Spalten hat die Matrix?
#define Zeichenabstand      1       // Zeichenabstand zwischen zwei Zeichen bei der Textausgabe

/**
 * \brief Startet die Matrixausgabe mit Hilfe von Timer 0 (alle 1ms eine Zeile->alle 8ms ein Bild)
 */
void matrixStart(void);

/**
 * \brief Stopt die Matrix Ausgabe
 */
void matrixStop(void);

/**
 * \brief Ganze Matrix auffüllen
 * 
 * \param wert Wert mit dem die Matrix aufgefüllt wird (0 oder 1)
 * 
 * \return void
 */
void matrixFill(uint8_t wert);

/**
 * \brief Schreibt einen Text auf die Matrix
 * 
 * \param spalte    Ab welcher Spalte soll der Text geschrieben werden
 * \param text      Der Text, z.B: "Hallo"
 * \param logic     3 =  (Bitmuster der Ziffern) übernehmen 1 und 0 schreiben
 *                  2 =  (Bitmuster der Ziffern) XOR (altem Wert in der Matrix)
 *                  1 =  (Bitmuster der Ziffern)  OR (altem Wert in der Matrix) --> Pixel setzen
 *                  0 = !(Bitmuster der Ziffern) AND (altem Wert in der Matrix) --> Pixel löschen
 */
void matrixWriteText(int16_t spalte, char *text, uint16_t logic);

/**
 * \brief Schreibe ein Zeichen auf die Matrix
 * 
 * \param spalte        Ab welcher Spalte soll das Zeichen geschrieben werden
 * \param zeichenNr     Ein Zeichen aus der ASCII-Tabelle z.B: 'A'
 */
void matrixWriteZeichen(uint16_t spalte, uint16_t zeichenNr);


/*************************************************************************************************/
/* Anschlüsse X4 und X1                                                                          */
/*************************************************************************************************/

// Input/Output Möglichkeiten
typedef enum ioType_t{
    OUTPUT,             //Ausgang
    INPUT,              //Normaler Eingang ohne PullUp-Wiederstand
    INPUT_PULLUP,       //Eingang mit PullUp-Widerstand
    INPUT_ADC           //ADC-Eingang, nur X4 PortF (Digitale Register werden deaktiviert)
    } ioType_t;
    
/**
 * \brief Setze ein Bit an X1, PortD als Ein- oder Ausgang
 * 
 * \param bitNr0_7  Welches Bit an PortD (0...7)
 * \param type      Ein- oder Ausgang (siehe ioType_t)
 */
void pinInitX1PortD(uint8_t bitNr0_7, ioType_t type);

/**
 * \brief Setze ein Bit an X4, PortL als Ein- oder Ausgang
 * 
 * \param bitNr2_5  Welches Bit an PortL (2...5)
 * \param type      Ein- oder Ausgang (siehe ioType_t)
 */
void pinInitX4PortL(uint8_t bitNr2_5, ioType_t type);

/**
 * \brief Setze ein Bit an X4, PortF als Ein- oder Ausgang
 * 
 * \param bitNr0_3  Welches Bit an PortF (0...3)
 * \param type      Ein- oder Ausgang (siehe ioType_t)
 */
void pinInitX4PortF(uint8_t bitNr0_3, ioType_t type);

/**
 * \brief Setzt oder löscht das entsprechende Bit. Bit mit initX... auf Ausgang setzen
 * 
 * \param bitNr0_7  Welches Bit an PortD (0...7)
 * \param val0_1    0 = 0V, 1 = 5V
 */
void pinWriteX1PortD(uint8_t bitNr0_7, uint8_t val0_1);

/**
 * \brief Setzt oder löscht das entsprechende Bit. Bit mit initX... auf Ausgang setzen
 * 
 * \param bitNr2_5  Welches Bit an PortL (2...5)
 * \param val0_1    0 = 0V, 1 = 5V
 */
void pinWriteX4PortL(uint8_t bitNr2_5, uint8_t val0_1);

/**
 * \brief Setzt oder löscht das entsprechende Bit. Bit mit initX... auf Ausgang setzen
 * 
 * \param bitNr0_3  Welches Bit an PortF (0...3)
 * \param val0_1    0 = 0V, 1 = 5V
 */
void pinWriteX4PortF(uint8_t bitNr0_3, uint8_t val0_1);

/**
 * \brief Lese das entspechende Bit ein. Bit mit initX... auf Eingang setzen
 * 
 * \param bitNr0_7  Welches Bit an PortD (0...7)
 * 
 * \return uint8_t Wahr wenn High eingelesen wird
 */
uint8_t pinReadX1PortD(uint8_t bitNr0_7);

/**
 * \brief Lese das entspechende Bit ein. Bit mit initX... auf Eingang setzen
 * 
 * \param bitNr2_5  Welches Bit an PortL (2...5)
 * 
 * \return uint8_t Wahr wenn High eingelesen wird
 */
uint8_t pinReadX4PortL(uint8_t bitNr2_5);

/**
 * \brief Lese das entspechende Bit ein. Bit mit initX... auf Eingang setzen
 * 
 * \param bitNr0_3  Welches Bit an PortF (0...3)
 * 
 * \return uint8_t Wahr wenn High eingelesen wird
 */
uint8_t pinReadX4PortF(uint8_t bitNr0_3);




#endif /* UCBOARDDRIVER_H_ */