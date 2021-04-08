/*********************************************************************************\
*
* MMMMMMMMMMMMMMMMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW
* MMMMMMMMMMMMMMMMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW   MECHATRONIK
* MMMMMMMMMMMMMMMMMM   SSSSS                WWWW   WWWW   WWWW
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW   SCHULE
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWW   WWWW   WWWW
* MMMM   MMMM   MMMM                SSSSS   WWWWWWWWWWWWWWWWWW   WINTERTHUR
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWWWWWWWWWWWWWWWW
* MMMM   MMMM   MMMM   SSSSSSSSSSSSSSSSSS   WWWWWWWWWWWWWWWWWW   www.msw.ch
*
*
* Dateiname: main.c
*
* Projekt  : uC-Board Beispiele
* Hardware : uC-Board, ATmega2560v von Atmel
*
* Copyright: MSW, E2
*
* Beschreibung:
* =============
* Alle Hardware des uC-Board wird eingesetzt und angezeigt.
*
* Portbelegung:
* =============
* Siehe ucBoardDriver.h
*
* Verlauf:
* ========
* siehe https://github.com/msw-winterthur/uC_Board
*
\*********************************************************************************/

#include "MoccaBoardDriver.h"

#define ANIMATION_DELAY_MS  50

int main(void)
{
    uint16_t poti0, poti1, poti2, poti3;
    uint16_t tasteS1,tasteS2,tasteS3,tasteS4;
    uint8_t schalter;
    uint8_t  muster = 0;
    uint64_t nextStep=0;
    uint64_t zeit=0;
    uint16_t rot=0;
    uint16_t gruen=0;
    uint16_t blau=0;
    
    //Board initialisieren
    initBoard();
    
    //Dauertext auf LCD anzeigen
    lcdWriteText(0,0,"MoccaBoard          ");
    lcdWriteText(1,0,"P0/1:      /        ");
    lcdWriteText(2,0,"P2/3:      /        ");
    lcdWriteText(3,0,"SysTime            s");
    //LCD-Beleuchtung einschalten
    lcdLight(255);
    
    while(1)
    {
        //Eingabe-------------------------------------------------------------------------
        
        poti0       = adcRead(ADC_00_PORTF_BIT0);                   // Potentiometer 1
        poti1       = adcRead(ADC_01_PORTF_BIT1);                   // Potentiometer 1
        poti2       = adcRead(ADC_02_PORTF_BIT2);                   // Potentiometer 1
        poti3       = adcRead(ADC_03_PORTF_BIT3);                   // Potentiometer 2
        tasteS1     = buttonReadAllPE() & 0b00010000;              // Tasten 1-4
        tasteS2     = buttonReadPE(5);
        tasteS3     = buttonReadPE(6);
        tasteS4     = buttonReadPE(7);
        schalter    = switchReadAll();                             //Schalter
        zeit        = getSystemTimeMs();                        //Systemzeit
        //Verarbeitung--------------------------------------------------------------------
        if (nextStep<zeit)
        {
            nextStep=nextStep+ANIMATION_DELAY_MS;
            muster=muster>>1;
            if(!muster) muster=0x80;
        }

        
        rot=poti3;
        gruen=poti2;
        blau=poti1;
        //Ausgabe------------------------------------------------------------------
        lcdWriteZahl(1, 6,poti0,4,0);                           // Potentiometer 0
        lcdWriteZahl(1, 13,poti1,4,0);                           // Potentiometer 1
        lcdWriteZahl(2, 6,poti2,4,0);                           // Potentiometer 2
        lcdWriteZahl(2, 13,poti3,4,0);                           // Potentiometer 3
        
        
        lcdWriteZahl(0,12,tasteS1|tasteS2|tasteS3|tasteS4,3,0); // Tasten 1-4 und Joystick
        lcdWriteZahl(0,17,schalter,3,0);                        // DIP_Switch 1-8
        
        lcdWriteZahl(3,8,zeit,6,3);
        
        
        rgbWrite(rot,gruen,blau);
        
        ledWriteAll(muster);
        lcdLight(poti0>>2);
    }
}

