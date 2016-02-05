#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>
#include <linux/usb/input.h>

#define FAKE

#ifdef FAKE
u8 fake_data[][18] = {
#include "pens-ltr"
{}};
#endif

struct sb680 {
  struct hid_device *hdev;
  struct input_dev *input;

  unsigned index;
};

static int sb680_open(struct input_dev *input) {
	struct sb680 *sb680 = input_get_drvdata(input);

  printk(KERN_INFO "sb680: input device opened");
  return hid_hw_open(sb680->hdev);
}

static void sb680_close(struct input_dev *input) {
	struct sb680 *sb680 = input_get_drvdata(input);

  printk(KERN_INFO "sb680: input device closed");
	hid_hw_close(sb680->hdev);
}

static int sb680_init_input(struct sb680 *sb680, const struct hid_device_id *id) {
	struct input_dev *input;
  int error;
  
	input = input_allocate_device();
	if (!input)
    return -ENOMEM;

  input->name = "SMART SB680 Interactive Whiteboard";
	//input->phys = "asdasdasd";
  input->dev.parent = &sb680->hdev->dev;

  input->open = sb680_open;
	input->close = sb680_close;
	input->uniq = sb680->hdev->uniq;
	input->id.bustype = id->bus;
	input->id.vendor  = id->vendor;
	input->id.product = id->product;
	input_set_drvdata(input, sb680);

  __set_bit(INPUT_PROP_DIRECT, input->propbit);
  
	input_set_abs_params(input, ABS_X, 0, 2047, 0, 0);
	input_set_abs_params(input, ABS_Y, 0, 2047, 0, 0);

	input_set_capability(input, EV_KEY, BTN_TOUCH);
	input_set_capability(input, EV_KEY, BTN_TOOL_FINGER);
	input_set_capability(input, EV_KEY, BTN_TOOL_PEN);
  input_set_capability(input, EV_KEY, BTN_TOOL_RUBBER);
	input_set_capability(input, EV_KEY, BTN_0); // black
	input_set_capability(input, EV_KEY, BTN_1); // red
	input_set_capability(input, EV_KEY, BTN_2); // green
	input_set_capability(input, EV_KEY, BTN_3); // blue
	input_set_capability(input, EV_KEY, BTN_4); // eraser

  error = input_register_device(input);
  if (error) {
    hid_err(sb680->hdev, "register_device failed\n");
    goto fail_register;
   }
  
  sb680->input = input;
  return 0;
  
fail_register:
  input_free_device(input);
  return error;
}

static void sb680_exit_input(struct sb680 *sb680) {
  struct input_dev *input = sb680->input;
  sb680->input = NULL;
  
  input_unregister_device(input);
  input_free_device(input);
}

static int sb680_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size) {
  struct sb680 *sb680 = hid_get_drvdata(hdev);
  struct input_dev *input = sb680->input;
  
  //printk(KERN_INFO "sb680: raw_event called");

#ifdef FAKE
  data = fake_data[sb680->index];
  size = 18;
  if (++sb680->index == sizeof(fake_data) / sizeof(fake_data[0]))
    sb680->index = 0;
#endif
  
  if (size == 0 || size > 18) {
    printk(KERN_INFO "sb680: bad message size %d", size);
    return -1;
  }

  if (data[0] == 0x01) {
    unsigned down;
    u32 x, y;
    
    if (size < 5) {
      printk(KERN_INFO "sb680: bad message size %d in major code %02x", size, data[0]);
      return -1;
    }
    
    down = data[1] & 0x01;
    x = data[2] | (((u32) data[3] & 0x0f) << 8);
    y = (data[3] >> 4) | (((u32) data[4]) << 4);
    
    input_report_key(input, BTN_TOUCH, down);
    input_report_abs(input, ABS_X, x);
    input_report_abs(input, ABS_Y, y);

    printk(KERN_INFO "sb680: mouse (%d, %d)", x, y);
  } else if (data[0] == 0x02) {
    u8 checksum = 0;
    int i;
    for (i = 1; i != size; ++i)
      checksum ^= data[i];

    if (checksum != 0) {
      printk(KERN_INFO "sb680: bad checksum 0x%02x", checksum);
      return -1;
    }

    if (size < 2) {
      printk(KERN_INFO "sb680: bad message size %d in major code %02x", size, data[0]);
      return -1;
    }

    if (data[1] == 0xc3)
      return 0;
    else if (data[1] == 0xd2) {
      if (size < 3) {
        printk(KERN_INFO "sb680: bad message size %d in minor code %02x", size, data[1]);
        return -1;
      }

      if (data[2] == 0x05) {
        int pens;

        if (size < 4) {
          printk(KERN_INFO "sb680: bad message size %d in sub code %02x", size, data[2]);
          return -1;
        }
        
        pens = data[3] & 0x1f;
        printk(KERN_INFO "sb680: reporting pens %02x", pens);
        input_report_key(input, BTN_0, pens & 0x10); // black
        input_report_key(input, BTN_1, pens & 0x08); // red
        input_report_key(input, BTN_2, pens & 0x02); // green
        input_report_key(input, BTN_3, pens & 0x01); // blue
        input_report_key(input, BTN_4, pens & 0x04); // eraser

        // report exactly one of these as down
        input_report_key(input, BTN_TOOL_FINGER, !pens);
        input_report_key(input, BTN_TOOL_PEN, !(pens & 0x04) && pens);
        input_report_key(input, BTN_TOOL_RUBBER, pens & 0x04);
      } else if (data[2] == 0x06)
        return 1;
      else {
        printk(KERN_INFO "sb680: unknown sub code 0x%02x", data[2]);
        return 1;
      }
    } else {
      printk(KERN_INFO "sb680: unknown minor code 0x%02x", data[1]);
      return 1;
    }
  } else {
    printk(KERN_INFO "sb680: unknown major code 0x%02x", data[0]);
    return 1;
  }
  
  input_sync(input);
  return 1;
}

static int sb680_probe(struct hid_device *hdev, const struct hid_device_id *id) {
  struct sb680 *sb680;
  int error;
  
  printk(KERN_INFO "sb680: probing...");

  sb680 = kzalloc(sizeof(struct sb680), GFP_KERNEL);
  if (!sb680)
    return -ENOMEM;

  hid_set_drvdata(hdev, sb680);
  sb680->hdev = hdev;
  sb680->index = 0;

  error = hid_parse(hdev);
  if (error) {
    hid_err(hdev, "parse failed\n");
    goto fail_parse;
  }

  error = sb680_init_input(sb680, id);
  if (error)
    goto fail_input;

  error = hid_hw_start(hdev, HID_CONNECT_HIDRAW);
  if (error) {
    hid_err(hdev, "hw_start failed\n");
    goto fail_hw_start;
  }

  printk(KERN_INFO "sb680: probing successful");
  return 0;

fail_hw_start:
  sb680_exit_input(sb680);  
fail_input:
fail_parse:
	hid_set_drvdata(hdev, NULL);
	kfree(sb680);
	return error;
}

static void sb680_remove(struct hid_device *hdev) {
  struct sb680 *sb680 = hid_get_drvdata(hdev);

  printk(KERN_INFO "sb680: removing device");
  hid_hw_stop(hdev);
  sb680_exit_input(sb680);  
	hid_set_drvdata(hdev, NULL);
	kfree(sb680);
}


// SMART Technologies Inc.
#define VENDOR_ID 0x046d // 0x0b8c
#define PRODUCT_ID 0xc06c// 0x0001

const struct hid_device_id sb680_ids[] = {
  { HID_USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, // SB680
  { }
};

MODULE_DEVICE_TABLE(hid, sb680_ids);

static struct hid_driver sb680_driver = {
  .name      = "sb680",
  .id_table  = sb680_ids,
  .probe     = sb680_probe,
  .remove    = sb680_remove,
  .raw_event = sb680_raw_event,
};

module_hid_driver(sb680_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Christian Sattler");
MODULE_DESCRIPTION("A skeleton driver for the SMART interactive whiteboard SB680");
MODULE_VERSION("0.1");

