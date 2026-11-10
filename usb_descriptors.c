/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
/// Original Source:
/// https://github.com/hathach/tinyusb/tree/master/examples/device/hid_composite

#include "usb_descriptors.h"
#include "bsp/board_api.h"
#include "tusb.h"

// Windows only treats a device as composite (separate driver per function)
// when it has a single configuration. The network device needs two
// configurations (RNDIS for Windows, CDC-ECM for macOS), so HID and network
// must never be combined: keyboard mode is HID only, config mode is network
// only. Separate PIDs keep Windows' per-device driver cache apart.
#define USB_VID 0xCafe
#define USB_PID_KEYBOARD 0x4010
#define USB_PID_NETWORK 0x4011
#define USB_BCD 0x0200

// String Descriptor Index
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_INTERFACE,
  STRID_MAC
};

static bool network_mode = false;

void usb_descriptors_set_network_mode(bool enabled) { network_mode = enabled; }

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
enum { CONFIG_ID_RNDIS = 0, CONFIG_ID_ECM = 1, CONFIG_ID_NET_COUNT };

static tusb_desc_device_t const desc_device_keyboard = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    // Class is defined per interface
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = USB_VID,
    .idProduct = USB_PID_KEYBOARD,
    .bcdDevice = 0x0100,

    .iManufacturer = STRID_MANUFACTURER,
    .iProduct = STRID_PRODUCT,
    .iSerialNumber = STRID_SERIAL,

    .bNumConfigurations = 1};

static tusb_desc_device_t const desc_device_network = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = USB_VID,
    .idProduct = USB_PID_NETWORK,
    .bcdDevice = 0x0100,

    .iManufacturer = STRID_MANUFACTURER,
    .iProduct = STRID_PRODUCT,
    .iSerialNumber = STRID_SERIAL,

    .bNumConfigurations = CONFIG_ID_NET_COUNT};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *)(network_mode ? &desc_device_network
                                        : &desc_device_keyboard);
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance;
  return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptors
//--------------------------------------------------------------------+

// Keyboard mode: a single HID interface
enum { ITF_KBD_HID = 0, ITF_KBD_TOTAL };

#define EPNUM_HID 0x81

#define KEYBOARD_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

static uint8_t const keyboard_configuration[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_KBD_TOTAL, 0, KEYBOARD_CONFIG_TOTAL_LEN, 0,
                          100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_KBD_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 1),
};

// Config mode: RNDIS (config 1, Windows/Linux) or CDC-ECM (config 2,
// macOS/Linux). Each creates two interfaces: control + data.
enum { ITF_NET_CTRL = 0, ITF_NET_DATA, ITF_NET_TOTAL };

#define EPNUM_NET_NOTIF 0x81
#define EPNUM_NET_OUT 0x02
#define EPNUM_NET_IN 0x82

#define RNDIS_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_RNDIS_DESC_LEN)
#define ECM_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_ECM_DESC_LEN)

static uint8_t const rndis_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(CONFIG_ID_RNDIS + 1, ITF_NET_TOTAL, 0,
                          RNDIS_CONFIG_TOTAL_LEN, 0, 100),

    // Interface number, string index, EP notification address and size, EP data
    // address (out, in) and size.
    TUD_RNDIS_DESCRIPTOR(ITF_NET_CTRL, STRID_INTERFACE, EPNUM_NET_NOTIF, 8,
                         EPNUM_NET_OUT, EPNUM_NET_IN,
                         CFG_TUD_NET_ENDPOINT_SIZE),
};

static uint8_t const ecm_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(CONFIG_ID_ECM + 1, ITF_NET_TOTAL, 0,
                          ECM_CONFIG_TOTAL_LEN, 0, 100),

    // Interface number, description string index, MAC address string index, EP
    // notification address and size, EP data address (out, in), and size, max
    // segment size.
    TUD_CDC_ECM_DESCRIPTOR(ITF_NET_CTRL, STRID_INTERFACE, STRID_MAC,
                           EPNUM_NET_NOTIF, 64, EPNUM_NET_OUT, EPNUM_NET_IN,
                           CFG_TUD_NET_ENDPOINT_SIZE, CFG_TUD_NET_MTU),
};

static uint8_t const *const network_configurations[CONFIG_ID_NET_COUNT] = {
    [CONFIG_ID_RNDIS] = rndis_configuration,
    [CONFIG_ID_ECM] = ecm_configuration};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  if (network_mode) {
    return (index < CONFIG_ID_NET_COUNT) ? network_configurations[index] : NULL;
  }
  return (index == 0) ? keyboard_configuration : NULL;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
static char const *string_desc_arr[] = {
    [STRID_LANGID] =
        (const char[]){0x09,
                       0x04}, // 0: is supported language is English (0x0409)
    [STRID_MANUFACTURER] = "Daniel T", // 1: Manufacturer
    [STRID_PRODUCT] = "Macropad",      // 2: Product
    [STRID_SERIAL] = NULL, // 3: Serials will use unique ID if possible
    [STRID_INTERFACE] = "Macropad Network",
    [STRID_MAC] = NULL,
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  size_t chr_count = 0;

  switch (index) {
  case STRID_LANGID:
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
    break;

  case STRID_SERIAL:
    chr_count = board_usb_get_serial(_desc_str + 1, 32);
    break;
  case STRID_MAC:
    for (unsigned i = 0; i < sizeof(tud_network_mac_address); i++) {
      _desc_str[1 + chr_count++] =
          "0123456789ABCDEF"[(tud_network_mac_address[i] >> 4) & 0xF];

      _desc_str[1 + chr_count++] =
          "0123456789ABCDEF"[tud_network_mac_address[i] & 0xF];
    }
    break;
  default:
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
      return NULL;

    const char *str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    size_t const max_count =
        sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
    if (chr_count > max_count)
      chr_count = max_count;

    // Convert ASCII string into UTF-16
    for (size_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
    break;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}
