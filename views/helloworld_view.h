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

#define MAX_SCALE     80
#define MAX_MOVE_STEP 16
#define MIN_MOVE_STEP 2

typedef enum {
    CountDownTimerMinuteUp,
    HelloworldMoveDown,
    HelloworldMoveLeft,
    HelloworldMoveRight,
    HelloworldReset
} HelloworldViewCmd;

typedef struct {
    bool start;
} HelloWorldModel;

typedef struct {
    View* view;
    FuriTimer* timer; // timer
    FuriHalUsbInterface* prev_usb_interface;
} HelloWorldView;

// functions
// allocate helloworld view
HelloWorldView* helloworld_view_new();
void helloworld_init(HelloWorldView* hwv); // set initial state

// delete helloworld view
void helloworld_view_delete(HelloWorldView* hwv);

// return view
View* helloworld_view_get_view(HelloWorldView* hwv);

void ok_toggle_callback(HelloWorldView* hwv);

#endif // __HELLOWORLD_H__
