/* 
 * File:   init_usb.h
 * Author: plgr
 *
 * Created on February 28, 2015, 11:18 AM
 */

#ifndef INIT_USB_H
#define	INIT_USB_H

#include "usb_stack_globals.h"    // USB stack only defines Not function related.
#include "descriptors.h"	// JTR Only included in main.c
#include "configwords.h"	// JTR only included in main.c

// USB stack by JTR and Honken
// CC-BY


// PIC18F Move reset vectors for bootloader compatibility
#ifdef __18CXX
	#define REMAPPED_RESET_VECTOR_ADDRESS		0x800
	#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
	#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
#endif

void InterruptHandlerHigh();
void InterruptHandlerLow();
void USBSuspend(void);
void usb_cycle(void);
void usb_install(void);
#pragma udata
extern BYTE usb_device_state;

#pragma code

#endif	/* INIT_USB_H */

