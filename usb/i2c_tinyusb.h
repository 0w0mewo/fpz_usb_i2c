#ifndef __COMPOSITE_H_H
#define __COMPOSITE_H_H

#include <furi_hal_usb.h>
#include <furi_hal_usb_i.h>
#include <furi.h>
#include <furi_hal.h>
#include "usb_cdc.h"
#include "usb_hid.h"
#include "descr_defs.h"
#include "i2c_usb_defs.h"

#define USBD_VID        0x0403
#define USBD_PID        0xc631
#define I2C_EP_IN       0x81
#define I2C_EP_IN_TYPE  USB_EPTYPE_CONTROL
#define EP_DEFAULT_SIZE 64

#define LOG_TAG "i2c-tiny-usb"

#define HIGH8_WORD(x) ((x >> 8) & 0xff)


struct UsbI2Cdevice {
    usbd_device* usb_dev;
    FuriHalUsbInterface* prev_intf;
    bool usb_connected;
};

FuriStatus usb_i2c_connect();
FuriStatus usb_i2c_disconnect();
bool usb_i2c_is_connected(void);

#endif
