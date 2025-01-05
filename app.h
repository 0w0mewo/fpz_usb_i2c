#ifndef __APP_H__
#define __APP_H__

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>

// app
typedef struct {
    Gui* gui; // gui object
    ViewDispatcher* view_dispatcher; // view dispacther of the gui

    // views
    HelloWorldView* helloworld_view;
} HelloUSBApp;

HelloUSBApp* app_new(void);
void app_delete(HelloUSBApp* app);
void app_run(HelloUSBApp* app);

#endif