/***********************************************************************
 *                                                                     
 *  DATE        : 28.11.2014
 *  DESCRIPTION : Treiber für Mocca
 *  Author:		: 
 *	Version		: 1.1                                         
 *                                                                     
 ***********************************************************************/

#ifndef mocca_define
#define mocca_define

#include <avr/io.h>
#include <avr/interrupt.h>

#define FALSE 0
#define TRUE !0
#define u16 unsigned int
#define u8  unsigned char

void init_sirius(void);
void init_mocca(void);

void init_5ms_timer(void);
void wait_5ms(uint16_t faktor);
void wait_1us(uint16_t volatile faktor);

void init_rs232 (uint16_t baudratenregister);
void rs232_sende_zeichen(char zeichen);
void rs232_sende_string(char *Text);

void init_usb(void);
void USB_sende_zeichen(char zeichen);
void USB_sende_string(char *Text);

void init_ADC(void);
uint16_t read_ADC(uint8_t kanal);

void init_RGB(void);
void PWM_RGB(uint16_t Rot,uint16_t Gruen,uint16_t Blau);
void RGB_Rot(uint16_t Rot);
void RGB_Gruen(uint16_t Gruen);
void RGB_Blau(uint16_t Blau);

void init_lcd(void);
void lcd_light(uint8_t hellighkeit);
void lcd_on_off(uint8_t status);
void write_text(uint8_t y_pos, uint8_t x_pos, char *str_ptr);
void write_zahl(uint8_t x_pos, uint8_t y_pos, uint16_t zahl_v, uint8_t s_vk, uint8_t s_nk, uint8_t komma);
void clear_lcd_f(void);

#endif
