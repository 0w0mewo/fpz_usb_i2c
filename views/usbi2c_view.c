#include "usbi2c_view.h"
#include <furi.h>
#include <math.h>

// callbacks
static void view_on_enter(void* ctx);
static void view_on_draw(Canvas* canvas, void* ctx);
static bool view_on_input(InputEvent* event, void* ctx);

UsbI2cView* usbi2c_view_new() {
    UsbI2cView* v = (UsbI2cView*)(malloc(sizeof(UsbI2cView)));

    // allocate view
    v->view = view_alloc();

    // pass HelloWorldView as context to callbacks
    view_set_context(v->view, v);

    // attatch draw, input and other callbacks to the view
    view_set_draw_callback(v->view, view_on_draw);
    view_set_input_callback(v->view, view_on_input);
    view_set_enter_callback(v->view, view_on_enter);

    return v;
}

void usbi2c_view_delete(UsbI2cView* v) {
    furi_assert(v);
    view_free(v->view);

    // restore usb config
    usb_i2c_disconnect();

    free(v);
}

View* usbi2c_view_get_view(UsbI2cView* v) {
    return v->view;
}

void usbi2c_init(UsbI2cView* v) {
    UNUSED(v);
    
    // config usb
    usb_i2c_connect();
}

// on enter callback, HelloWorldView as ctx
static void view_on_enter(void* ctx) {
    furi_assert(ctx);

    UsbI2cView* v = (UsbI2cView*)ctx;
    usbi2c_init(v);
}

// view draw callback, HelloWorldModel as ctx
static void view_on_draw(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    // draw border
    canvas_draw_frame(canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    canvas_draw_str_aligned(canvas, 43, 3, AlignLeft, AlignTop, "C0->SCL");
    canvas_draw_str_aligned(canvas, 43, 13, AlignLeft, AlignTop, "C1->SDA");
    canvas_draw_str_aligned(canvas, 43, 23, AlignLeft, AlignTop, "GND->GND");
}

// keys input event callback, HelloWorldView as ctx
static bool view_on_input(InputEvent* event, void* ctx) {
    UNUSED(ctx);

    bool consumed = false; // flag to notify view_dispacther
        //that the callback function is processed.

    // move
    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyOk:
            break;

        default:
            break;
        }
        consumed = true;
    }

    return consumed;
}

