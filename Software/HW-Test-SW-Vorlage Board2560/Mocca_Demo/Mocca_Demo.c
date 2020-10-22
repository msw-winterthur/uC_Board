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

unsigned int line = 0;

#define ENABLE_0          PORTJ &= ~0x08; // PJ3 Zeilen/Mux ausschalten
#define ENABLE_1          PORTJ |=  0x08; // PJ3 Zeilen/Mux einschalten
#define CLOCK_0           PORTJ &= ~0x10; // PJ4 Clock = 0
#define CLOCK_1           PORTJ |=  0x10; // PJ4 Clock = 1
#define STROBE_0          PORTJ &= ~0x40; // PJ6 Strobe = 0
#define STROBE_1          PORTJ |=  0x40; // PJ6 Strobe = 1

#define DATA_Modul_1_ON   PORTJ |=  0x20; // PJ5 Data = 1
#define DATA_Modul_1_OFF  PORTJ &= ~0x20; // PJ5 Data = 0

// Vom Benutzer vorgegebene Parameter, die sich je nach Matrix-Grösse unterscheiden
#define Anzahl_Spalten     56             // Wieviele Spalten hat die Matrix?
#define Zeichenabstand      1             // Zeichenabstand zwischen zwei Zeichen bei der Textausgabe

//------------------------------------------------------------------------------------

//							        Länge,Spalten ...                              Nr.
const u8 ASCII_Tab[64][10]= {4,  0,  0,  0,  0,  0,  0,  0,	0,  0,		//  0    " "
	5,  0,  0, 95,  0,  0,  0,  0,	0,  0,		//  1    !
	5,  0,  7,  0,  7,  0,  0,  0,	0,  0,		//  2    "
	5, 20,127, 20,127, 20,  0,  0,	0,  0,		//  3    #
	5,  4, 42,107, 42, 16,  0,  0,	0,  0,		//  4    $
	5, 35, 19,  8,100, 98,  0,  0,	0,  0,		//  5    %
	5, 54, 73, 85, 34, 80,  0,  0,	0,  0,		//  6    &
	4,  0,  5,  3,  0,  0,  0,  0,	0,  0,		//  7    '
	5,  0, 28, 34, 65,  0,  0,  0,	0,  0,		//  8    (
	5,  0, 65, 34, 28,  0,  0,  0,	0,  0,		//  9    )
	5, 20,  8, 62,  8, 20,  0,  0,	0,  0,		// 10    *
	5,  8,  8, 62,  8,  8,  0,  0,	0,  0,		// 11    +
	4,  0, 80, 48,  0,  0,  0,  0,	0,  0,		// 12    ,
	5,  8,  8,  8,  8,  8,  0,  0,	0,  0,		// 13    -
	4,  0, 96, 96,  0,  0,  0,  0,	0,  0,		// 14    .
	5, 16,  8,  4,  2,  1,  0,  0,	0,  0,		// 15    /
	5, 62, 65, 65, 65, 62,  0,  0,	0,  0,		// 16    0
	3, 66,127, 64,  0,  0,  0,  0,	0,  0,		// 17    1
	5, 66, 97, 81, 73, 70,  0,  0,	0,  0,		// 18    2
	5, 34, 65, 73, 73, 54,  0,  0,	0,  0,		// 19    3
	5, 24, 20, 18,127, 16,  0,  0,	0,  0,		// 20    4
	5, 39, 69, 69, 69, 57,  0,  0,	0,  0,		// 21    5
	5, 60, 74, 73, 73, 48,  0,  0,	0,  0,		// 22    6
	5,  1,113,  9,  5,  3,  0,  0,	0,  0,		// 23    7
	5, 54, 73, 73, 73, 54,  0,  0,	0,  0,		// 24    8
	5,  6, 73, 73, 41, 30,  0,  0,	0,  0,		// 25    9
	5, 54, 54,  0,  0,  0,  0,  0,	0,  0,		// 26    :
	5, 86, 54,  0,  0,  0,  0,  0,	0,  0,		// 27    ;
	5,  8, 20, 34, 65,  0,  0,  0,	0,  0,		// 28    <
	5, 20, 20, 20, 20, 20,  0,  0,	0,  0,		// 29    =
	5, 65, 34, 20,  8,  0,  0,  0,	0,  0,		// 30    >
	5,  2,  1, 81,  9,  6,  0,  0,	0,  0,		// 31    ?
	5, 62, 65, 93, 85, 30,  0,  0,	0,  0,		// 32    @
	5,126, 17, 17, 17,126,  0,  0,	0,  0,		// 33    A
	5,127, 73, 73, 73, 54,  0,  0,	0,  0,		// 34    B
	5, 62, 65, 65, 65, 34,  0,  0,	0,  0,		// 35    C
	5,127, 65, 65, 34, 28,  0,  0,	0,  0,		// 36    D
	5,127, 73, 73, 73, 65,  0,  0,	0,  0,		// 37    E
	5,127,  9,  9,  9,  1,  0,  0,	0,  0,		// 38    F
	5, 62, 65, 73, 73,122,  0,  0,	0,  0,		// 39    G
	5,127,  8,  8,  8,127,  0,  0,	0,  0,		// 40    H
	3, 65,127, 65,  0,  0,  0,  0,	0,  0, 		// 41    I
	5, 32, 64, 65, 63,  1,  0,  0,	0,  0,		// 42    J
	5,127,  8, 20, 34, 65,  0,  0,	0,  0,		// 43    K
	5,127, 64, 64, 64, 64,  0,  0,	0,  0,		// 44    L
	5,127,  2, 12,  2,127,  0,  0,	0,  0,		// 45    M
	5,127,  4,  8, 16,127,  0,  0,	0,  0, 		// 46    N
	5, 62, 65, 65, 65, 62,  0,  0,	0,  0,		// 47    O
	5,127,  9,  9,  9,  6,  0,  0,	0,  0,		// 48    P
	5, 62, 65, 81, 33, 94,  0,  0,	0,  0,		// 49    Q
	5,127,  9, 25, 41, 70,  0,  0,	0,  0,		// 50    R
	5, 70, 73, 73, 73, 49,  0,  0,	0,  0,		// 51    S
	5,  1,  1,127,  1,  1,  0,  0,	0,  0,		// 52    T
	5, 63, 64, 64, 64, 63,  0,  0,	0,  0,		// 53    U
	5, 31, 32, 64, 32, 31,  0,  0,	0,  0,		// 54    V
	5, 63, 64, 48, 64, 63,  0,  0,	0,  0,		// 55    W
	5, 99, 20,  8, 20, 99,  0,  0,	0,  0,		// 56    X
	5,  7,  8,112,  8,  7,  0,  0,	0,  0,		// 57    Y
	5, 97, 81, 73, 69, 67,  0,  0,	0,  0,		// 58    Z
	2,127,127,  0,  0,  0,  0,  0,	0,  0, 		// 59
	6,124,126, 27, 27,126,124,  0,	0,  0, 		// 60
	6,124,126, 27, 27,126,124,  0,	0,  0, 		// 61
	7,127,127,  6, 12, 24, 63,127,	0,  0,      // 62
    7, 62,73, 85,	81,	85,	73,	62,0,0};		// 63    Smile

u16 matrix[Anzahl_Spalten];         // Diese Tabelle representiert die einzelnen Pixel der Matrix

u8 write_text_finished = 0;


void fill_matrix(u8 wert)
{ u8 i;
	//while(write_text_finished == 0);                          // Warten, bis alle 8 Zeilen geschrieben wurden.
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
void write_Zeichen_Matrix(u16 Matrix_Spalten_Nr, u16 Zeichen_Nr)
{
	u8 i, breite;
	
	breite = ASCII_Tab[Zeichen_Nr][0];                          // Breite eines Zeichens aus Tabelle lesen
	for (i=0; i<breite; i++)                                    // Je nach Zeichenbreite alle Spalten kopieren
	{ if(Matrix_Spalten_Nr+i < Anzahl_Spalten)              // Nicht über die Matrix hinaus schreiben !!!
		{ matrix[Matrix_Spalten_Nr+i]= ASCII_Tab[Zeichen_Nr][i+1];
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
void write_Text_Matrix(int Matrix_Spalten_Nr, char *str_ptr, u16 logic)
{ int matrix_buffer_pos;
	u8 breite, i, z_nr;
	u8 str_p = 0;                                            // Dieser Zeiger zeigt auf das zu schreibende Zeichen im Text
	
	//while(write_text_finished == 0);                          // Warten, bis alle 8 Zeilen geschrieben wurden.
	matrix_buffer_pos = Matrix_Spalten_Nr;
	while (str_ptr[str_p] != 0)
	{
		breite = ASCII_Tab[str_ptr[str_p]-32][0];               // Breite eines Zeichens aus Tabelle lesen
		z_nr = str_ptr[str_p]-32;                               // Zeichen-Nr aus Text lesen
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

void wait_Laufschrift(u16 time)
{
	wait_5ms(time);
}


void write_next_line(void)
{
	const  u8 Bit_Muster_Tab[8] = {1,2,4,8,16,32,64,128};
	static u16 Zeilen_Nr, Bit_Muster;
	u8 i;
	
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
	
	ENABLE_0;	                            // +5V aller Zeilen abschalten
	PORTJ &= ~0x07;
	PORTJ |= Zeilen_Nr;                     // gewünschte Zeile einstellen
	STROBE_1;                             // Daten ins Latch schreiben
	ENABLE_1;                             // +5V Speisung der eingestellten Zeile einschalten

	// Die Zeilen werden nacheinander eingeschaltet --> zyklisch 0,1,2,3,4,5,6,7, 0,1,2,3, ...
	if (Zeilen_Nr < 7)
	{ Zeilen_Nr++;
		write_text_finished = 0;
	}
	else
	{ Zeilen_Nr = 0;
		write_text_finished = 1;
	}
	
}

int main(void)
{
	
	int x_pos, i;
	u16 Poti_1, Poti_2;
	u16 Licht, Magnet_1, Magnet_2, Temp;
	u16 JS_x, JS_y;
	u16 Tasten, DIP_Switch;

	init_mocca();
	init_lcd();		
	init_ADC();

    wait_5ms(100);
	init_RGB();
	


	
	 fill_matrix(0);   // Matrix löschen
	 write_Zeichen_Matrix( 0, 63);
	 write_Zeichen_Matrix(50, 30);
	 write_Text_Matrix(20,"MSW",1);
	 wait_5ms(500);
	 
	 fill_matrix(0);   // Matrix löschen
	 
	 write_text(1,0,"P1/2:         L:    ");
	 write_text(2,0,"M1/2:         C:    ");
	 write_text(3,0,"Jx/y:         T:    ");
	 
	 PWM_RGB(0,0,0);
	 
	while(1)
    {
		
		if((PINE >> 4) == 1)
		{
			for(x_pos=-18; x_pos<Anzahl_Spalten; x_pos++)     // 3x6 = 36 entspricht der Länge des Textes in Pixel.
			{	write_Text_Matrix(18,"HELLO",1);
				write_Text_Matrix(x_pos,"_-->",1);    // Schrift schreiben
				wait_Laufschrift(8);
				write_Text_Matrix(x_pos,"_-->",2);    // Schrift löschen
			}
		}
		
		
		Poti_1     = read_ADC( 8);									// Potentiometer 1
		Poti_2     = read_ADC( 9);									// Potentiometer 2
		Licht      = read_ADC(12);									// Lichtsensor
		Magnet_1   = read_ADC(14);									// Magnet 1 
		Magnet_2   = read_ADC(15);									// Magnet 2
		Temp       = read_ADC(13);									// Temperatur
		JS_x	   = read_ADC(10);									// Joystick x-Achse
		JS_y	   = read_ADC(11);									// Joystick y-Achse
		Tasten     = PINE>>4;										// Tasten 1-4
		DIP_Switch = PINC;
		
		write_zahl(1, 9,Poti_1,4,0,0);								// Potentiometer 1
		write_zahl(1, 5,Poti_2,4,0,0);								// Potentiometer 2
		write_zahl(1,16,Licht,4,0,0);								// Lichtsensor
		
		write_zahl(2, 5,Magnet_1,4,0,0);							// Magnet 1 
		write_zahl(2, 9,Magnet_2,4,0,0);							// Magnet 2
		write_zahl(2,16,Temp,4,0,0);								// Temperatur
		
		write_zahl(3, 5,JS_x,4,0,0);								// Joystick x-Achse
		write_zahl(3, 9,JS_y,4,0,0);								// Joystick y-Achse
		
		write_zahl(3,16,Tasten,4,0,0);								// Tasten 1-4
		write_zahl(0,16,DIP_Switch,4,0,0);							// DIP_Switch 1-8
		
		PWM_RGB(JS_x/2,Poti_2/2,JS_y/2);							// RGB-LED: rot = JS_x, grün = Poti_2, blau = JS_y
		
		
		/*if((PINE &0xF0)>0)
		{
			PWM_RGB(0,0,0);
			if((PINE&0x10) >0)RGB_Rot(1023);
			if((PINE&0x20) >0)RGB_Gruen(1023);
			if((PINE&0x40) >0)RGB_Blau(1023);
			if((PINE&0x80) >0)PWM_RGB(1023,1023,1023);
		}
		else PWM_RGB(read_ADC(3),read_ADC(2),read_ADC(1));
		*/
    }
}