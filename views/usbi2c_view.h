#ifndef __HELLOWORLD_H__
#define __HELLOWORLD_H__

#include <furi.h>
#include <furi_hal.h>
#include <gui/view.h>

#include "../usb/i2c_tinyusb.h"

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define SCREEN_CENTER_X (SCREEN_WIDTH / 2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT / 2)

typedef struct {
    View* view;
    FuriHalUsbInterface* prev_usb_interface;
} UsbI2cView;

// functions
// allocate helloworld view
UsbI2cView* usbi2c_view_new();
void usbi2c_init(UsbI2cView* v); // set initial state

// delete helloworld view
void usbi2c_view_delete(UsbI2cView* v);

// return view
View* usbi2c_view_get_view(UsbI2cView* v);

#endif // __HELLOWORLD_H__
