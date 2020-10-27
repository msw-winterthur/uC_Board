/*
* uCBoardBeispiele.c
*
* Created: 26.10.2020 08:43:59
* Author : Dario Dündar
*/

#include <avr/io.h>
#include "ucBoardDriver.h"


int main(void)
{
    int16_t x_pos;
    uint16_t poti1, poti2;
    uint16_t licht, magnet1, magnet2, temperatur;
    uint16_t joyStickX, JoyStickY;
    uint16_t tasten;
    uint8_t schalter;
    uint16_t  counter = 0;
    uint8_t btnJoystick;
    uint64_t zeit=0;
    
    //Board initialisieren
    initBoard();
    //Matrix starten und startanimation zeigen
    startMatrix();
    
    fillMatrix(0);   // Matrix löschen
    writeZeichenMatrix( 0, 63);
    writeZeichenMatrix(50, 30);
    writeTextMatrix(20,"MSW",1);

    
    //Dauertext auf LCD anzeigen
    writeText(0,0,"uC-Board     s S:   ");
    writeText(1,0,"P1/2:         L:    ");
    writeText(2,0,"M1/2:         C:    ");
    writeText(3,0,"Jx/y:         T:    ");
    //LCD-Beleuchtung einschalten
    lcdLight(10);
    
    
    
    while(1)
    {
        //Eingabe-------------------------------------------------------------------------
        btnJoystick = (PINE&0b00000100);

        poti1       = readAdc(ADC_08_POTI_1);									// Potentiometer 1
        poti2       = readAdc(ADC_09_POTI_2);									// Potentiometer 2
        licht       = readAdc(ADC_12_LICHT);									// Lichtsensor
        magnet1     = readAdc(ADC_14_MAGNET_1);									// Magnet 1
        magnet2     = readAdc(ADC_15_MAGNET_2);									// Magnet 2
        temperatur  = readAdc(ADC_13_TEMPERATUR);									// Temperatur
        joyStickX	= readAdc(ADC_10_JOYSTICK_X);									// Joystick x-Achse
        JoyStickY	= readAdc(ADC_11_JOYSTICK_Y);									// Joystick y-Achse
        tasten      = (PINL&0b11000011);			// Tasten 1-4
        schalter    = PINC;
        zeit        = getSystemTimeMs();
        //Verarbeitung--------------------------------------------------------------------
        
        counter=counter>>1;
        if(!counter) counter=0x8000;
        
        
        //Ausgabe------------------------------------------------------------------
        writeZahl(1, 9,poti1,4,0);								// Potentiometer 1
        writeZahl(1, 5,poti2,4,0);								// Potentiometer 2
        writeZahl(1,16,licht,4,0);								// Lichtsensor
        
        writeZahl(2, 5,magnet1,4,0);							// Magnet 1
        writeZahl(2, 9,magnet2,4,0);							// Magnet 2
        writeZahl(2,16,temperatur,4,0);								// Temperatur
        
        writeZahl(3, 5,joyStickX,4,0);								// Joystick x-Achse
        writeZahl(3, 9,JoyStickY,4,0);								// Joystick y-Achse
        
        writeZahl(3,16,tasten,4,0);								// Tasten 1-4 und Joystick
        writeZahl(0,17,schalter,3,0);							// DIP_Switch 1-8
        if (zeit <= 9999)
        {
            writeZahl(0,8,zeit,1,3);
        } 
        else
        {
            writeZahl(0,0,zeit,9,3);
        }
        
        pwmRgb(joyStickX/2,poti2/2,JoyStickY/2);							// RGB-LED: rot = JS_x, grün = Poti_2, blau = JS_y
        
        PORTA = ~counter;
        PORTB = ~counter>>8;
        
        //Matrixanimation bei Tastendruck auf joystick
        if(btnJoystick)
        {
            //Displayausgabe stoppen
            writeText(0,0,"Matrix Animation....");
            writeText(1,0,"                    ");
            writeText(2,0,"                    ");
            writeText(3,0,"                    ");
            for(x_pos=-18; x_pos<Anzahl_Spalten; x_pos++)     // 3x6 = 36 entspricht der Länge des Textes in Pixel.
            {
                writeTextMatrix(x_pos,"_-->",1);    // Schrift schreiben
                writeTextMatrix(x_pos,"_-->",2);    // Schrift löschen
            }
            //Dauertext wieder anzeigen
            writeText(0,0,"uC-Board     s S:   ");
            writeText(1,0,"P1/2:         L:    ");
            writeText(2,0,"M1/2:         C:    ");
            writeText(3,0,"Jx/y:         T:    ");
        }
    }
}

