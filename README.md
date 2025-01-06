## Buggy, hack-together clone of i2c-usb-tiny for flipper zero

### What it is

A simple demo to make flipper zero as a i2c-usb-tiny device.

NOTE: It's not fully tested.

### How to use
```
# modprobe i2c-dev
# i2cdetect -l
```

### Tested
Tested with PCF8574
- `i2cdetect` works from  
- `i2cget` read one byte
- `i2cset` set one byte

