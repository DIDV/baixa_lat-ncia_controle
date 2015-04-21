/************************************************************************
	tlc5940.h

    TLC5940 LED driver PIC library
    Copyright (C) 2011 Kevin Risden

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Email: compuwizard123@gmail.com

************************************************************************/

#ifndef TLC5940_H
#define TLC5940_H
#include <p18f4550.h>

// TLC5940 Hardware mapping definitions

// Note: SIN, SCLK and GSCLK are tied to module functions on the PIC
#define TLC5940_SIN	PORTCbits.RC7	//SDO     pin 26
#define TLC5940_SCLK	PORTBbits.RB1	//SCK/SCL pin 34
#define TLC5940_GSCLK	PORTCbits.RC2	//CCP1    pin 17

// These you can assign to other pins if required
#define TLC5940_XLAT	PORTEbits.RE1   //pin 09
// #define TLC5940_VPRG	PORTAbits.RA1   //pin 03
#define TLC5940_BLANK	PORTAbits.RA5   //pin 07

// XLAT timer period
#define XLATCOUNTTIMER  35356   // 50Hz

// The number of cascaded TLC5940s
#define NUMBEROF5940	4

// Function prototypes
void setInitialGrayScaleValues(void);
void processXLATinterrupt(void);
void initialiseTlc5940(void);
void setGrayScaleValue(unsigned char channel, int value);
unsigned char updateTlc5940(void);

#endif
