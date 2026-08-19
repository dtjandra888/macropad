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

// Unique PID per example: guarantees re-enumeration on re-flash and a fresh
// host driver match.
#define USB_PID 0x400f

// String Descriptor Index
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_INTERFACE,
  STRID_MAC
};

enum { CONFIG_ID_RNDIS = 0, CONFIG_ID_ECM = 1, CONFIG_ID_COUNT };

#define USB_VID 0xCafe
#define USB_BCD 0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
static tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = STRID_MANUFACTURER,
    .iProduct = STRID_PRODUCT,
    .iSerialNumber = STRID_SERIAL,

    .bNumConfigurations = CONFIG_ID_COUNT};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
    //     TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
    //     TUD_HID_REPORT_DESC_STYLUS_PEN(HID_REPORT_ID(REPORT_ID_STYLUS_PEN)),
    //     TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL)),
    //     TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD))
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance;
  return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum { ITF_NUM_HID = 0, ITF_NUM_NET, ITF_NUM_TOTAL };

#define MAIN_CONFIG_TOTAL_LEN                                                  \
  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_RNDIS_DESC_LEN)

#define ALT_CONFIG_TOTAL_LEN                                                   \
  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_ECM_DESC_LEN)

//#define NCM_CONFIG_TOTAL_LEN                                                   \
//  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_NCM_DESC_LEN)

#define EPNUM_HID 0x81
#define EPNUM_NET_NOTIF 0x82
#define EPNUM_NET_OUT 0x03
#define EPNUM_NET_IN 0x83

static uint8_t const rndis_configuration[] = {
    // Config number (index+1), interface count, string index, total length,
    // attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(CONFIG_ID_RNDIS + 1, ITF_NUM_TOTAL, 0,
                          MAIN_CONFIG_TOTAL_LEN, 0, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 5),

    // Interface number, string index, EP notification address and size, EP data
    // address (out, in) and size.
    TUD_RNDIS_DESCRIPTOR(ITF_NUM_NET, STRID_INTERFACE, EPNUM_NET_NOTIF, 8,
                         EPNUM_NET_OUT, EPNUM_NET_IN,
                         CFG_TUD_NET_ENDPOINT_SIZE),
};

static uint8_t const ecm_configuration[] = {
    // Config number (index+1), interface count, string index, total length,
    // attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(CONFIG_ID_ECM + 1, ITF_NUM_TOTAL, 0,
                          ALT_CONFIG_TOTAL_LEN, 0, 100),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 5),

    // Interface number, description string index, MAC address string index, EP
    // notification address and size, EP data address (out, in), and size, max
    // segment size.
    TUD_CDC_ECM_DESCRIPTOR(ITF_NUM_NET, STRID_INTERFACE, STRID_MAC,
                           EPNUM_NET_NOTIF, 64, EPNUM_NET_OUT, EPNUM_NET_IN,
                           CFG_TUD_NET_ENDPOINT_SIZE, CFG_TUD_NET_MTU),
};

static uint8_t const *const configuration_arr[CONFIG_ID_COUNT] = {
    [CONFIG_ID_RNDIS] = rndis_configuration,
    [CONFIG_ID_ECM] = ecm_configuration};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  return (index < CONFIG_ID_COUNT) ? configuration_arr[index] : NULL;
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
