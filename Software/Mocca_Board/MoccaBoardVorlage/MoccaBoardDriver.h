/*
 * ucBoardDriver.h
 *
 * Created: 26.10.2020 08:44:55
 *  Author: Dario Dündar
 */ 


#ifndef MOCCABOARDDRIVER_H_
#define MOCCABOARDDRIVER_H_

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
   - Alle Hardware auf dem Board wird initialisiert
   - Timer 0: Systemzeit, 1ms Tick, lcdLight
   - Timer 4: rgbLED
 */
void initBoard (void);


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
 * \brief Wartet auf eine bestimte Systemzeit, eignet sich um getaktete Programme zu schreiben.
 * 
 * \param systemTimeToWaitForMS Auf diese Systemzeit wird gewartet.
 * 
 * \return uint8_t Ist gleich 0 falls die abzuwartende Zeit bereits vorbei ist.
 */
uint8_t waitForSystemTimeMs(uint64_t systemTimeToWaitForMS);


/*************************************************************************************************/
/* LEDs                                                                                          */
/*************************************************************************************************/

/**
 * \brief Zeige ein Bitmuster an den LEDs an (PortA0 LSB bis PortB7 MSB)
 * 
 * \param bitMuster     Das Bitmuster, das angezeigt werden soll
 */
void ledWriteAll(uint8_t bitMuster);

/**
 * \brief Gibt zurück, was im Moment an den LEDs angezeigt wird
 *
 * \return uint8_t Bitmuster, das an den LEDs angezeigt wird
 */
uint8_t ledReadALL(void);

/**
 * \brief Setzt eine einzelne LED
 * 
 * \param ledNr0_7 Welche LED soll gesetzt werden? LED 0 = PORTA Bit 0
 * \param wer0_1 Auf welchen Wert soll die LED gesetzt werden? 1 = LED leuchtet
 */
void ledWrite(uint8_t ledNr0_7, uint8_t wer0_1);

/**
 * \brief Liest den Zustand einer enzelnen LED ein
 * 
 * \param ledNr0_15 Welche LED soll eingelesen werden?
 * 
 * \return uint8_t Wahr wenn die entsprechende LED leuchtet
 */
uint8_t ledRead(uint8_t ledNr0_7);

/*************************************************************************************************/
/* Schalter und Buttons                                                                          */
/*************************************************************************************************/

/**
 * \brief Gibt an ob die Schalter (PortK) aktiv High oder Low sind (default: 1)
 */
#define SWITCH_ACTIV_HIGH   1

/**
 * \brief Liest die Schalter (PortK) ein
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
 * \brief Liest die Buttons an PortE ein (S1=PE4, S2=PE5, S3=PE6, S4=PE7)
 * 
 * \return uint8_t Den gesamten PortE
 */
uint8_t buttonReadAllPE(void);

/**
 * \brief Liest einen Button an PortE ein (S1=PE4, S2=PE5, S3=PE6, S4=PE7)
 * 
 * \param buttonNr0_7 4=S1, 5=S2, 6=S3, 7=S4
 * 
 * \return uint8_t Wahr wenn der entsprechende Button gedrückt ist
 */
uint8_t buttonReadPE(uint8_t buttonNr0_7);

/*************************************************************************************************/
/* Analog-Digital-Wandler (ADC)                                                                  */
/*************************************************************************************************/

// Alle Analog-Eingänge und ihre Funktionen
typedef enum adcChannel_t {
    ADC_00_PORTF_BIT0 = 0,   
    ADC_01_PORTF_BIT1,       
    ADC_02_PORTF_BIT2,       
    ADC_03_PORTF_BIT3
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
 *          https://de.wikipedia.org/wiki/Printf
 * 
 * \param zeile0_3      Auf welcher Zeile von 0...3 soll geschrieben werden?
 * \param spalte0_19    Ab welcher Spalte von 0...19 soll geschirebene werden?
 * \param formattedText Formatierter Text, z.B: "Hallo %d", 5
 */
void lcdWriteText(uint8_t zeile0_3, uint8_t spalte0_19, char const *formattedText, ...);

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
 * \brief Löscht alles auf dem LCD
 */
void lcdClear(void);


/*************************************************************************************************/
/* Mocca-Board Kummunikation                                                                     */
/*************************************************************************************************/
void adm_RS232_init(uint16_t baudratenregister);
void adm_RS232_send_byte(uint8_t zeichen);
void adm_RS232_send_string(const char*Text);

void adm_USB_init(void);
void adm_USB_send_byte(uint8_t zeichen);
void adm_USB_send_string(const char*Text);



#endif /* MOCCABOARDDRIVER_H_ */