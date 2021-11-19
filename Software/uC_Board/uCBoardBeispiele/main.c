/*********************************************************************************\
*
* MMMMMMMMMMMM   SSSSSSSSSSSS   WW   WW   WW   MECHATRONIK
* MM   MM   MM   SS             WW   WW   WW   SCHULE
* MM   MM   MM   SSSSSSSSSSSS   WW   WW   WW   WINTERTHUR
* MM   MM   MM             SS   WW   WW   WW   
* MM   MM   MM   SSSSSSSSSSSS   WWWWWWWWWWWW   www.msw.ch
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

#define SWITCH_ACTIV_HIGH   1 //Sind die Schalter an PortC active Low oder High?
#include "ucBoardDriver.h"


int main(void)
{
    int16_t x_pos;
    uint16_t poti1, poti2;
    uint16_t licht, magnet1, magnet2, temperatur;
    uint16_t joyStickX, JoyStickY;
    uint16_t tasteS1,tasteS2,tasteS3,tasteS4;
    uint8_t schalter;
    uint16_t  ledMuster = 0;
    uint16_t ledInvert = 0x00FF;
    uint8_t ledDirRight = 1;
    uint8_t btnJoystick;
    uint64_t zeit=0;
    uint16_t rot=0;
    uint16_t gruen=0;
    uint16_t blau=0;
    uint8_t inputX1PortD1 = 0;
    
    //Board initialisieren
    initBoard(1);
    
    //Matrix starten und startanimation zeigen
    matrixStart();
    
    matrixFill(0);   // Matrix löschen
    matrixWriteZeichen( 0, 63);
    matrixWriteZeichen(50, 30);
    matrixWriteText(20,"MSW",1);

    
    //Dauertext auf LCD anzeigen
    lcdWriteText(0,0,"uC-Board     s S:   ");
    lcdWriteText(1,0,"P1/2:         L:    ");
    lcdWriteText(2,0,"M1/2:         C:    ");
    lcdWriteText(3,0,"Jx/y:         T:    ");
    //LCD-Beleuchtung einschalten
    lcdLight(255);
    
    //Pin PD0 an X1 auf Ausgang setzen
    pinInitX1PortD(0, OUTPUT);
    //Pin PD1 an X1 auf Eingang mit internem PullUp setzen
    pinInitX1PortD(1, INPUT_PULLUP);
    while(1)
    {
        //Eingabe-------------------------------------------------------------------------
        btnJoystick = buttonReadJoyStickPE2();
        
        inputX1PortD1 = pinReadX1PortD(1);

        poti1       = adcRead(ADC_08_POTI_1);                   // Potentiometer 1
        poti2       = adcRead(ADC_09_POTI_2);                   // Potentiometer 2
        licht       = adcRead(ADC_12_LICHT);                    // Lichtsensor
        magnet1     = adcRead(ADC_14_MAGNET_1);                 // Magnet 1
        magnet2     = adcRead(ADC_15_MAGNET_2);                 // Magnet 2
        temperatur  = adcRead(ADC_13_TEMPERATUR);               // Temperatur
        joyStickX   = adcRead(ADC_10_JOYSTICK_X);               // Joystick x-Achse
        JoyStickY   = adcRead(ADC_11_JOYSTICK_Y);               // Joystick y-Achse
        tasteS1     = buttonReadAllPL() & 0b00000001;              // Tasten 1-4
        tasteS2     = buttonReadPL(1);
        tasteS3     = buttonReadPL(6);
        tasteS4     = buttonReadPL(7);
        schalter    = switchReadAll();                             //Schalter
        zeit        = getSystemTimeMs();                        //Systemzeit
        //Verarbeitung--------------------------------------------------------------------
        
        if (ledDirRight)
        {
            ledMuster=ledMuster>>1;
            if(!ledMuster){
                ledMuster=0x0001;
                ledDirRight=0;   
            }                
        } 
        else
        {
            ledMuster=ledMuster<<1;
            if(!ledMuster){
                ledMuster=0x8000;
                ledDirRight=1;
                ledInvert=~ledInvert;
            }                
        }      
        
        if (tasteS1)
        {
            rot=1023;
        }
        if (tasteS2)
        {
            gruen=1023;
        }
        if (tasteS3)
        {
            blau=1023;
        }
        if (tasteS4)
        {
            rot=0;
            gruen=0;
            blau=0;
        }
        
        //Ausgabe------------------------------------------------------------------
        lcdWriteZahl(1, 5,poti1,4,0);                           // Potentiometer 1
        lcdWriteZahl(1, 9,poti2,4,0);                           // Potentiometer 2
        lcdWriteZahl(1,16,licht,4,0);                           // Lichtsensor
        
        lcdWriteZahl(2, 5,magnet1,4,0);                         // Magnet 1
        lcdWriteZahl(2, 9,magnet2,4,0);                         // Magnet 2
        lcdWriteZahl(2,16,temperatur,4,0);                      // Temperatur
        
        lcdWriteZahl(3, 5,joyStickX,4,0);                       // Joystick x-Achse
        lcdWriteZahl(3, 9,JoyStickY,4,0);                       // Joystick y-Achse
        
        lcdWriteZahl(3,16,tasteS1|tasteS2|tasteS3|tasteS4,4,0); // Tasten 1-4 und Joystick
        lcdWriteZahl(0,17,schalter,3,0);                        // DIP_Switch 1-8
        if (zeit <= 9999)
        {
            lcdWriteZahl(0,8,zeit,1,3);
        } 
        else
        {
            lcdWriteZahl(0,0,zeit,9,3);
        }
        
        rgbWrite(rot,gruen,blau);                
        
        ledWriteAll(ledMuster^ledInvert);
        
        pinWriteX1PortD(0,inputX1PortD1);

        //Matrixanimation bei Tastendruck auf joystick
        if(btnJoystick)
        {
            //Displayausgabe stoppen
            lcdWriteText(0,0,"Matrix Animation....");
            lcdWriteText(1,0,"                    ");
            lcdWriteText(2,0,"                    ");
            lcdWriteText(3,0,"                    ");
            for(x_pos=-18; x_pos<Anzahl_Spalten; x_pos++)       // 3x6 = 36 entspricht der Länge des Textes in Pixel.
            {
                matrixWriteText(x_pos,"_-->",1);    // Schrift schreiben
                matrixWriteText(x_pos,"_-->",2);    // Schrift löschen
            }
            //Dauertext wieder anzeigen
            lcdWriteText(0,0,"uC-Board     s S:   ");
            lcdWriteText(1,0,"P1/2:         L:    ");
            lcdWriteText(2,0,"M1/2:         C:    ");
            lcdWriteText(3,0,"Jx/y:         T:    ");
        }
    }
}

