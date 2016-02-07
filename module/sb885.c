#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>
#include <linux/usb/input.h>

//#define FAKE

#ifdef FAKE
u8 fake_data[][100] = {
  //#include "../nicolai/logs/allbuttons/data.h"
  //#include "../nicolai/logs/buttonspressed/data.h"
  //#include "../nicolai/logs/connectedToDisconnected/data.h"
  #include "../nicolai/logs/drawOnePen/data.h"
  //#include "../nicolai/logs/drawTwoSimultaneous/data.h"
  //#include "../nicolai/logs/pickupEraser/data.h"
  //#include "../nicolai/logs/pickupPen1/data.h"
  //#include "../nicolai/logs/pickupPen2/data.h"
  //#include "../nicolai/logs/pressSixCentralButtonsOneByOneThenReleaseOneByOne/data.h"
  //#include "../nicolai/logs/putbackPen1then2/data.h"
  //#include "../nicolai/logs/putEraserBack/data.h"
  //#include "../nicolai/logs/takeEraserTakePenSimultaneouslyEraseWrite/data.h"
  //#include "../nicolai/logs/tryBlue/data.h"
  //#include "../nicolai/logs/turnOn/data.h"
  //#include "../nicolai/logs/turnWBon/data.h"
{}};
#endif

struct sb885 {
  struct hid_device *hdev;
  struct input_dev *input;

  unsigned index;
};

static int sb885_open(struct input_dev *input) {
	struct sb885 *sb885 = input_get_drvdata(input);

  printk(KERN_INFO "sb885: input device opened");
  return hid_hw_open(sb885->hdev);
}

static void sb885_close(struct input_dev *input) {
	struct sb885 *sb885 = input_get_drvdata(input);

  printk(KERN_INFO "sb885: input device closed");
	hid_hw_close(sb885->hdev);
}

static int sb885_init_input(struct sb885 *sb885, const struct hid_device_id *id) {
	struct input_dev *input;
  int error;
  
	input = input_allocate_device();
	if (!input)
    return -ENOMEM;

  input->name = "SMART SB885 Interactive Whiteboard";
	//input->phys = "asdasdasd";
  input->dev.parent = &sb885->hdev->dev;

  input->open = sb885_open;
	input->close = sb885_close;
	input->uniq = sb885->hdev->uniq;
	input->id.bustype = id->bus;
	input->id.vendor  = id->vendor;
	input->id.product = id->product;
	input_set_drvdata(input, sb885);

  __set_bit(INPUT_PROP_DIRECT, input->propbit);
  
	input_set_abs_params(input, ABS_X, 0, 4095, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, 4095, 0, 0);

	input_set_capability(input, EV_KEY, BTN_TOUCH);
	input_set_capability(input, EV_KEY, BTN_TOOL_FINGER);
	input_set_capability(input, EV_KEY, BTN_TOOL_PEN);
  input_set_capability(input, EV_KEY, BTN_TOOL_RUBBER);

	input_set_capability(input, EV_KEY, BTN_0); // pen 0
	input_set_capability(input, EV_KEY, BTN_1); // pen 1
	input_set_capability(input, EV_KEY, BTN_2); // eraser

	input_set_capability(input, EV_KEY, BTN_3); // black
	input_set_capability(input, EV_KEY, BTN_4); // red
	input_set_capability(input, EV_KEY, BTN_5); // green
	input_set_capability(input, EV_KEY, BTN_6); // blue

  error = input_register_device(input);
  if (error) {
    hid_err(sb885->hdev, "register_device failed\n");
    goto fail_register;
   }
  
  sb885->input = input;
  return 0;
  
fail_register:
  input_free_device(input);
  return error;
}

static void sb885_exit_input(struct sb885 *sb885) {
  struct input_dev *input = sb885->input;
  sb885->input = NULL;
  
  input_unregister_device(input);
  input_free_device(input);
}

static int sb885_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size) {
  struct sb885 *sb885 = hid_get_drvdata(hdev);
  struct input_dev *input = sb885->input;
  
  //printk(KERN_INFO "sb885: raw_event called");

#ifdef FAKE
  data = fake_data[sb885->index];
  size = 100;
  if (++sb885->index == sizeof(fake_data) / sizeof(fake_data[0]))
    sb885->index = 0;
#endif

  if (size == 0 || size > 100) {
    printk(KERN_INFO "sb885: bad message size %d", size);
    return -1;
  }

  if (data[0] == 0x05) {
    unsigned down;
    u32 x, y;
    
    if (size < 8) {
      printk(KERN_INFO "sb885: bad message size %d in major code %02x", size, data[0]);
      return -1;
    }
    
    down = data[1];
    x = data[4] | (((u32) data[5]) << 8);
    y = data[6] | (((u32) data[7]) << 8);
    
    input_report_key(input, BTN_TOUCH, down);
    input_report_abs(input, ABS_X, x);
    input_report_abs(input, ABS_Y, y);

    printk(KERN_INFO "sb885: mouse (%d, %d) -- modifier %d", x, y, down);
  } else if (data[0] == 0x02) {
    u8 checksum = 0;
    int i;
    for (i = 1; i != size; ++i)
      checksum ^= data[i];

    if (checksum != 0) {
      printk(KERN_INFO "sb885: bad checksum 0x%02x", checksum);
      return -1;
    }

    if (size < 6) {
      printk(KERN_INFO "sb885: bad message size %d in major code %02x", size, data[0]);
      return -1;
    }

    if (data[1] != 0xc3) {
      printk(KERN_INFO "sb885: unknown data[1] %02x", data[1]);
      return -1;
    }

    if (data[2] == 0)
      return 0;
    else if (data[2] == 0xd5) {
      int pens = data[5] & 0x07;

      printk(KERN_INFO "sb885: reporting pens %02x", pens);
      input_report_key(input, BTN_0, pens & 0x04); // pen 1
      input_report_key(input, BTN_1, pens & 0x02); // eraser
      input_report_key(input, BTN_2, pens & 0x01); // pen 2

      input_report_key(input, BTN_TOOL_FINGER, !pens);
      input_report_key(input, BTN_TOOL_PEN, pens & 0x05);
      input_report_key(input, BTN_TOOL_RUBBER, pens & 0x02);
    } else {
      printk(KERN_INFO "sb885: unknown minor code 0x%02x", data[1]);
      return 1;
    }
  } else if (data[0] == 0x06) {
    int buttons;
    if (size < 2) {
      printk(KERN_INFO "sb885: bad message size %d in major code %02x", size, data[0]);
      return -1;
    }

    buttons = data[1] & 0x1e;
    printk(KERN_INFO "sb885: reporting buttons %02x", buttons);
    input_report_key(input, BTN_3, buttons & 0x10); // black
    input_report_key(input, BTN_4, buttons & 0x08); // red
    input_report_key(input, BTN_5, buttons & 0x04); // green
    input_report_key(input, BTN_6, buttons & 0x02); // blue
  }

  else {
    printk(KERN_INFO "sb885: unknown major code 0x%02x", data[0]);
    return 1;
  }
  
  input_sync(input);
  return 1;
}

static int sb885_probe(struct hid_device *hdev, const struct hid_device_id *id) {
  struct sb885 *sb885;
  int error;
  
  printk(KERN_INFO "sb885: probing...");

  sb885 = kzalloc(sizeof(struct sb885), GFP_KERNEL);
  if (!sb885)
    return -ENOMEM;

  hid_set_drvdata(hdev, sb885);
  sb885->hdev = hdev;
  sb885->index = 0;

  error = hid_parse(hdev);
  if (error) {
    hid_err(hdev, "parse failed\n");
    goto fail_parse;
  }

  error = sb885_init_input(sb885, id);
  if (error)
    goto fail_input;

  error = hid_hw_start(hdev, HID_CONNECT_HIDRAW);
  if (error) {
    hid_err(hdev, "hw_start failed\n");
    goto fail_hw_start;
  }

  printk(KERN_INFO "sb885: probing successful");
  return 0;

fail_hw_start:
  sb885_exit_input(sb885);  
fail_input:
fail_parse:
	hid_set_drvdata(hdev, NULL);
	kfree(sb885);
	return error;
}

static void sb885_remove(struct hid_device *hdev) {
  struct sb885 *sb885 = hid_get_drvdata(hdev);

  printk(KERN_INFO "sb885: removing device");
  hid_hw_stop(hdev);
  sb885_exit_input(sb885);  
	hid_set_drvdata(hdev, NULL);
	kfree(sb885);
}


#ifndef FAKE
// SMART Technologies Inc.
#define VENDOR_ID 0x0b8c
#define PRODUCT_ID 0x0160
#else
// Logitech Inc., Optical Mouse
#define VENDOR_ID 0x046d
#define PRODUCT_ID 0xc06c
#endif

const struct hid_device_id sb885_ids[] = {
  { HID_USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
  { }
};

MODULE_DEVICE_TABLE(hid, sb885_ids);

static struct hid_driver sb885_driver = {
  .name      = "sb885",
  .id_table  = sb885_ids,
  .probe     = sb885_probe,
  .remove    = sb885_remove,
  .raw_event = sb885_raw_event,
};

module_hid_driver(sb885_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Christian Sattler");
MODULE_DESCRIPTION("A skeleton driver for the SMART interactive whiteboard SB885");
MODULE_VERSION("0.1");

