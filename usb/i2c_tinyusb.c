#include <furi.h>
#include <furi_hal_version.h>
#include "i2c_tinyusb.h"
#include "i2c_helper.h"

// #define DEBUG

static uint16_t delay = 0;
static uint8_t i2c_status = STATUS_IDLE;
static uint8_t i2c_reply_buf[EP_DEFAULT_SIZE];
static FuriSemaphore *i2c_state_sem;
static const unsigned long func = I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;

/* string descriptors */
static const struct usb_string_descriptor dev_manuf_desc = USB_STRING_DESC("Flipper Devices");
static const struct usb_string_descriptor dev_prod_desc = USB_STRING_DESC("USB I2C bridge");

/* Device descriptor */
static const struct usb_device_descriptor usb_i2c_device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_CLASS_PER_INTERFACE,
    .bDeviceSubClass = USB_SUBCLASS_NONE,
    .bDeviceProtocol = USB_PROTO_NONE,
    .bMaxPacketSize0 = USB_EP0_SIZE, // USB_EP0_SIZE
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = VERSION_BCD(0, 0, 1),
    .iManufacturer = 1, // UsbDevManuf
    .iProduct = 2, // UsbDevProduct
    .iSerialNumber = NO_DESCRIPTOR,
    .bNumConfigurations = 1,
};

/* Device configuration descriptor */
static const struct usb_i2c_config_dscr usb_i2c_cfg_desc = {
    .config =
        {
            .bLength = sizeof(struct usb_config_descriptor),
            .bDescriptorType = USB_DTYPE_CONFIGURATION,
            .wTotalLength = sizeof(struct usb_i2c_config_dscr),
            .bNumInterfaces = 1,
            .bConfigurationValue = 1,
            .iConfiguration = NO_DESCRIPTOR,
            .bmAttributes = USB_CFG_ATTR_RESERVED,
            .bMaxPower = USB_CFG_POWER_MA(100),
        },
    .intf =
        {
            .bLength = sizeof(struct usb_interface_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFACE,
            .bInterfaceNumber = 0,
            .bAlternateSetting = 0,
            .bNumEndpoints = 1,
            .bInterfaceClass = USB_CLASS_VENDOR,
            .bInterfaceSubClass = USB_SUBCLASS_VENDOR,
            .bInterfaceProtocol = USB_PROTO_VENDOR,
            .iInterface = NO_DESCRIPTOR,
        },
    .ep_in =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = I2C_EP_IN,
            .bmAttributes = I2C_EP_IN_TYPE,
            .wMaxPacketSize = EP_DEFAULT_SIZE,
            .bInterval = 0x0,

        },
};

static void usb_i2c_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
static void usb_i2c_deinit(usbd_device* dev);
static void usb_i2c_on_wakeup(usbd_device* dev);
static void usb_i2c_on_suspend(usbd_device* dev);
static usbd_respond usb_i2c_ep_config(usbd_device* dev, uint8_t cfg);
static usbd_respond
    usb_i2c_ctrlreq_handler(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback);

static struct UsbI2Cdevice usbd = {.usb_dev = NULL, .prev_intf = NULL, .usb_connected = false};

static void usb_i2c_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    UNUSED(intf);
    UNUSED(ctx);

    usbd.usb_dev = dev;

    usbd_reg_config(dev, usb_i2c_ep_config);
    usbd_reg_control(dev, usb_i2c_ctrlreq_handler);

    // lock the i2c bus through the prog runs
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

    if(i2c_state_sem == NULL){ 
        i2c_state_sem = furi_semaphore_alloc(1, 1);
    }

    usbd_connect(dev, true);
}

static void usb_i2c_deinit(usbd_device* dev) {
    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);

    // unlock i2c bus
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
    
    if(i2c_state_sem != NULL){ 
        furi_semaphore_free(i2c_state_sem);
    }

}

static void usb_i2c_on_wakeup(usbd_device* dev) {
    UNUSED(dev);
    usbd.usb_connected = true;
}

static void usb_i2c_on_suspend(usbd_device* dev) {
    UNUSED(dev);
    if(usbd.usb_connected) {
        usbd.usb_connected = false;
    }
}

static void usb_rx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
}

static void usb_tx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
}

static void usb_txrx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    switch(event) {
    case usbd_evt_eptx:
        usb_tx_ep_callback(dev, event, ep);
        break;
    case usbd_evt_eprx:
        usb_rx_ep_callback(dev, event, ep);
        break;
    default:
        break;
    }
}

/* Configure endpoints */
static usbd_respond usb_i2c_ep_config(usbd_device* dev, uint8_t cfg) {
    switch(cfg) {
    case 0:
        /* deconfiguring device */
        usbd_ep_deconfig(dev, I2C_EP_IN);
        usbd_reg_endpoint(dev, I2C_EP_IN, 0);

        return usbd_ack;
    case 1:
        /* configuring device */
        usbd_ep_config(dev, I2C_EP_IN, I2C_EP_IN_TYPE, EP_DEFAULT_SIZE);
        usbd_reg_endpoint(dev, I2C_EP_IN, usb_txrx_ep_callback);

        return usbd_ack;
    default:
        return usbd_fail;
    }
}

/* Control requests handler */
static usbd_respond
    usb_i2c_ctrlreq_handler(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    UNUSED(callback);
    UNUSED(dev);
    UNUSED(req);

    uint8_t usb_req_type = req->bmRequestType;

    // ignore standard requests
    if(!(usb_req_type & USB_REQ_VENDOR)) {
        return usbd_fail;
    }

    uint8_t cmd = req->bRequest;

    switch(cmd) {
    case CMD_ECHO:
        uint16_t reply = req->wValue;
        memcpy(i2c_reply_buf, &reply, sizeof(reply));
        dev->status.data_ptr = i2c_reply_buf;
        dev->status.data_count = sizeof(reply);

        return usbd_ack;
    case CMD_GET_FUNC:
        dev->status.data_ptr = (void*)&func;
        dev->status.data_count = sizeof(func);

        return usbd_ack;
    case CMD_SET_DELAY:
        // TODO
        delay = req->wValue;
        return usbd_ack;

    case CMD_I2C_IO:
    case CMD_I2C_IO | CMD_I2C_BEGIN:
    case CMD_I2C_IO | CMD_I2C_END:
    case CMD_I2C_IO | CMD_I2C_BEGIN | CMD_I2C_END:
        uint8_t do_read = req->wValue & I2C_M_RD;
        uint8_t addr = req->wIndex;
        uint16_t size = req->wLength;

        // avoid overflow
        if(size >= EP_DEFAULT_SIZE) {
            size = EP_DEFAULT_SIZE - 1;
            return usbd_fail;
        }

        addr = addr << 1;

#ifdef DEBUG
        furi_log_puts("\n\r---\n\r");
        furi_log_puts("cmd: "); log8bits(cmd); furi_log_puts("\n\r");
        furi_log_puts("addr: "); log8bits(addr); furi_log_puts("\n\r");
        furi_log_puts("size: "); log16bits(size); furi_log_puts("\n\r");
        furi_log_puts("wValue: "); log8bits(req->wValue); furi_log_puts("\n\r");
#endif

        furi_semaphore_acquire(i2c_state_sem, FuriWaitForever);
        // TODO: i2c-detect works (maybe ?), read/write needs to be tested.
        // TODO: use worker thread to handle the following actions
        if(IS_BITS_SET(usb_req_type, (USB_REQ_INTERFACE | USB_REQ_DEVTOHOST))) {
            if(do_read) { // read
                i2c_status = i2c_read(addr, i2c_reply_buf, size);
#ifdef DEBUG
            if(i2c_status == STATUS_ADDRESS_ACK) {
                furi_log_puts("read ok\n\r");
                furi_log_puts("data: ");
                for(uint16_t i = 0; i < size; i++) {
                    log8bits(i2c_reply_buf[i]);
                }
            } else {
                furi_log_puts("read failed\n\r");
            }
#endif 
                dev->status.data_ptr = i2c_reply_buf;
                dev->status.data_count = size;
            }
        } else if(IS_BITS_SET(usb_req_type, (USB_REQ_INTERFACE | USB_REQ_HOSTTODEV))){
            if(!do_read && size == 0) { // check ready, used by i2c scanning
                i2c_status = i2c_device_ready(addr);
#ifdef DEBUG
                if(i2c_status == STATUS_ADDRESS_ACK) {
                    furi_log_puts("i2c devcheck ack\n\r");
                } else {
                    furi_log_puts("i2c devcheck nack\n\r");
                }  
#endif 
                dev->status.data_ptr = i2c_reply_buf;
                dev->status.data_count = 0;
            } else if(!do_read && size != 0) { // write
                i2c_status = i2c_write(addr, req->data, size);
#ifdef DEBUG
                if(i2c_status == STATUS_ADDRESS_ACK) {
                    furi_log_puts("write ok\n\r");
                    furi_log_puts("data: ");
                    for(uint16_t i = 0; i < size; i++) {
                        log8bits(req->data[i]);
                    }
                } else {
                    furi_log_puts("write failed\n\r");
                }  
#endif          
            }
        }else {
            furi_crash("unimplemented i2c-usb cmd");
        }
        furi_semaphore_release(i2c_state_sem);

        return usbd_ack;

    case CMD_GET_STATUS:
        furi_semaphore_acquire(i2c_state_sem, FuriWaitForever);
        memcpy(i2c_reply_buf, &i2c_status, sizeof(i2c_status));
        dev->status.data_ptr = i2c_reply_buf;
        dev->status.data_count = sizeof(i2c_status);
        furi_semaphore_release(i2c_state_sem);

        return usbd_ack;

    default:
        furi_crash("unimplemented usb feature");
        return usbd_fail;
    }

    return usbd_fail;
}

static FuriHalUsbInterface usb_i2c_intf = {
    .init = usb_i2c_init,
    .deinit = usb_i2c_deinit,
    .wakeup = usb_i2c_on_wakeup,
    .suspend = usb_i2c_on_suspend,

    .dev_descr = (struct usb_device_descriptor*)&usb_i2c_device_desc,
    .str_manuf_descr = (void*)&dev_manuf_desc,
    .str_prod_descr = (void*)&dev_prod_desc,
    .str_serial_descr = NULL,
    .cfg_descr = (void*)&usb_i2c_cfg_desc,
};

FuriStatus usb_i2c_connect() {
    if(furi_hal_usb_is_locked()) {
        FURI_LOG_E(LOG_TAG, "usb is locked by other threads");
        return FuriStatusError;
    } else {
        usbd.prev_intf = furi_hal_usb_get_config();
        furi_hal_usb_set_config(&usb_i2c_intf, NULL);
        return FuriStatusOk;
    }
}

FuriStatus usb_i2c_disconnect() {
    furi_hal_usb_set_config(usbd.prev_intf, NULL);

    return FuriStatusOk;
}

bool usb_i2c_is_connected(void) {
    return usbd.usb_connected;
}
