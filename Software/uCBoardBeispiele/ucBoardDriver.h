/*
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


#define ADC_00_X4_PORTF_BIT0    0
#define ADC_01_X4_PORTF_BIT1    1
#define ADC_02_X4_PORTF_BIT2    2
#define ADC_03_X4_PORTF_BIT3    3
#define ADC_04_NOT_USED         4
#define ADC_05_NOT_USED         5
#define ADC_06_NOT_USED         6
#define ADC_07_NOT_USED         7

#define ADC_08_POTI_1           8
#define ADC_09_POTI_2           9
#define ADC_10_JOYSTICK_X       10
#define ADC_11_JOYSTICK_Y       11
#define ADC_12_LICHT            12
#define ADC_13_TEMPERATUR       13
#define ADC_14_MAGNET_1         14
#define ADC_15_MAGNET_2         15

typedef enum ioType_t{OUTPUT, INPUT, INPUT_PULLUP, INPUT_ADC} ioType_t;
    

// Vom Benutzer vorgegebene Parameter, die sich je nach Matrix-Grösse unterscheiden
#define Anzahl_Spalten     56             // Wieviele Spalten hat die Matrix?
#define Zeichenabstand      1             // Zeichenabstand zwischen zwei Zeichen bei der Textausgabe


void initBoard (void);


void wait5msTick(uint16_t faktor);
uint64_t getSystemTimeMs(void);


void initRs232 (uint16_t baudratenregister);
void rs232SendeZeichen(char zeichen);
void rs232SendeString(char *Text);

void initUsb(void);
void usbSendeZeichen(char zeichen);
void usbSendeString(char *Text);

void initAdc(void);
uint16_t adcRead(uint8_t kanal);

void initRgb(void);
void rgbPwm(uint16_t Rot,uint16_t Gruen,uint16_t Blau);
void rgbRot(uint16_t Rot);
void rgbGruen(uint16_t Gruen);
void rgbBlau(uint16_t Blau);

void initLcd(void);
void lcdLight(uint8_t hellighkeit);
void lcdWriteText(uint8_t y_pos, uint8_t x_pos, char *str_ptr);
void lcdWriteZahl(uint8_t x_pos, uint8_t y_pos, uint64_t zahl_v, uint8_t s_vk, uint8_t s_nk);
void lcdClear(void);

void initMatrix(void);
void matrixStart(void);
void matrixStop(void);
void matrixFill(uint8_t wert);
void matrixWriteText(int16_t Matrix_Spalten_Nr, char *str_ptr, uint16_t logic);
void matrixWriteZeichen(uint16_t Matrix_Spalten_Nr, uint16_t Zeichen_Nr);

void initPinX1PortD(uint8_t bitNr0_7, ioType_t type);
void initPinX4PortL(uint8_t bitNr2_5, ioType_t type);
void initPinX4PortF(uint8_t bitNr0_3, ioType_t type);
void pinWriteX1PortD(uint8_t bitNr0_7, uint8_t val0_1);
void pinWriteX4PortL(uint8_t bitNr2_5, uint8_t val0_1);
void pinWriteX4PortF(uint8_t bitNr0_3, uint8_t val0_1);
void ledWrite(uint8_t bitNr0_15, uint8_t val0_1);
void ledWriteAll(uint16_t bitMuster);
uint8_t pinReadX1PortD(uint8_t bitNr0_7);
uint8_t pinReadX4PortL(uint8_t bitNr2_5);
uint8_t pinReadX4PortF(uint8_t bitNr0_3);
uint8_t switchRead(uint8_t bitNr0_7);
uint8_t switchReadAll();


#endif /* UCBOARDDRIVER_H_ */