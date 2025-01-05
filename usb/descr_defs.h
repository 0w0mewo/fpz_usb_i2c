#ifndef DESCR_DEFS_H_H
#define DESCR_DEFS_H_H

#include "usb.h"
#include "usb_hid.h"
#include "usb_cdc.h"

struct usb_i2c_config_dscr {
    struct usb_config_descriptor config;
    struct usb_interface_descriptor intf;
    struct usb_endpoint_descriptor ep_in;
    // struct usb_endpoint_descriptor ep_out;
} FURI_PACKED;
#endif
