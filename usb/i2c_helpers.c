#include <furi_hal_i2c.h>
#include "i2c_helper.h"
#include "i2c_usb_defs.h"
#include <furi.h>

static FuriHalI2cBusHandle* i2cbus = &furi_hal_i2c_handle_external;

uint8_t i2c_read(uint8_t dev_addr, uint8_t* buf, uint16_t len) {
    bool ok = false;    
    FuriHalI2cBegin begin = FuriHalI2cBeginStart;
    FuriHalI2cEnd end = FuriHalI2cEndStop;

    ok = furi_hal_i2c_rx_ext(i2cbus, dev_addr, false, buf, len, begin, end, I2C_TIMEOUT);
    return (ok ? STATUS_ADDRESS_ACK : STATUS_ADDRESS_NACK);
}

uint8_t i2c_write(uint8_t dev_addr, uint8_t* buf, uint16_t len) {
    bool ok = false;
    FuriHalI2cBegin begin = FuriHalI2cBeginStart;
    FuriHalI2cEnd end = FuriHalI2cEndStop;

    ok = furi_hal_i2c_tx_ext(i2cbus, dev_addr, false, buf, len, begin, end, I2C_TIMEOUT);
    return (ok ? STATUS_ADDRESS_ACK : STATUS_ADDRESS_NACK);
}

uint8_t i2c_device_ready(uint8_t dev_addr) {
    bool ready = false;

    // furi_hal_i2c_acquire(i2cbus);
    ready = furi_hal_i2c_is_device_ready(i2cbus, dev_addr, I2C_TIMEOUT);
    // furi_hal_i2c_release(i2cbus);

    return (ready ? STATUS_ADDRESS_ACK : STATUS_ADDRESS_NACK);
}

void log8bits(uint8_t data) {
    char tmp_str[] = "0xFFFFFFFF";
    itoa(data, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts(" ");
}

void log16bits(uint16_t data) {
    char tmp_str[] = "0xFFFFFFFF";
    itoa(data, tmp_str, 16);
    furi_log_puts(tmp_str);
    furi_log_puts(" ");

}
