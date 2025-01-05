#include "views/helloworld_view.h"
#include "app.h"

static void register_view(ViewDispatcher* dispatcher, View* view, uint32_t viewid);
static uint32_t view_exit(void* ctx);

int32_t app_main(void* p) {
    UNUSED(p);

    HelloUSBApp* app = app_new();

    app_run(app);

    app_delete(app);

    return 0;
}

HelloUSBApp* app_new(void) {
    HelloUSBApp* app = (HelloUSBApp*)(malloc(sizeof(HelloUSBApp)));

    // open gui
    app->gui = furi_record_open(RECORD_GUI);

    // setup view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);

    // attach view dispatcher to gui
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // attach views to the dispatcher
    // helloworld view
    app->helloworld_view = helloworld_view_new();
    register_view(app->view_dispatcher, helloworld_view_get_view(app->helloworld_view), 0xff);

    // switch to default view
    view_dispatcher_switch_to_view(app->view_dispatcher, 0xff);

    return app;
}

void app_delete(HelloUSBApp* app) {
    furi_assert(app);

    // delete views
    view_dispatcher_remove_view(app->view_dispatcher, 0xff);
    helloworld_view_delete(app->helloworld_view); // hello world view

    // delete view dispatcher
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    // self
    free(app);
}

void app_run(HelloUSBApp* app) {
    view_dispatcher_run(app->view_dispatcher);
}

static void register_view(ViewDispatcher* dispatcher, View* view, uint32_t viewid) {
    view_dispatcher_add_view(dispatcher, viewid, view);

    view_set_previous_callback(view, view_exit);
}

static uint32_t view_exit(void* ctx) {
    furi_assert(ctx);

    return VIEW_NONE;
}
