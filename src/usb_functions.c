#include "init_usb.h"
#include "tlc5940.h"
#include <delays.h>

void usb_install(void)
{

    initCDC(); // setup the CDC state machine
    usb_init(cdc_device_descriptor, cdc_config_descriptor, cdc_str_descs, USB_NUM_STRINGS); // initialize USB. TODO: Remove magic with macro
    usb_start(); //start the USB peripheral

    do {
        usb_handler(); //only needed for polling!!
    } while (usb_device_state < CONFIGURED_STATE);
    usb_register_sof_handler(CDCFlushOnTimeout);

}

void USBSuspend(void) {} // declarado para a stack funcionar.

void Delay_uS(char uSec) {

  do {
    Delay1TCY();            // 1
    Delay1TCY();            // 1
    Delay1TCY();            // 1
    Delay1TCY();            // 1
    Nop();                        // 1
    Nop();                        // 1
    Nop();                        // 1
  } while(--uSec);        // 3
}

void Delay_ms(int mSec){
  int i;
  for(i=0; i<mSec; i++)
  Delay100TCYx(120);
}

// #pragma code


