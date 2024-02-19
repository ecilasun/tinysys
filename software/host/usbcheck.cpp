#include <stdio.h>

// NOTE: Make sure to 'sudo apt-get install libusb-dev' for this on Linux
#include <usb.h>

int main()
{
    printf("USB access test\n");

    struct usb_bus *bus;
    struct usb_device *dev;

    const char *cmd = "12345678";

    usb_init();
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_busses; bus; bus = bus->next)
        for (dev = bus->devices; dev; dev = dev->next)
        {
            // Is this vendor=0xFFFF product=0x0001 ?
            if (dev->descriptor.idVendor == 0xFFFF)// && dev->descriptor.idProduct == 0x0001)
            {
                printf("Found device %s/%s\n", bus->dirname, dev->filename);
                printf("\tID_VENDOR = 0x%04x\n", dev->descriptor.idVendor);
                printf("\tID_PRODUCT = 0x%04x\n", dev->descriptor.idProduct);

                // EP1 interrupt write to control
                usb_dev_handle *handle = usb_open(dev);
                if (handle)
                    int res = usb_bulk_write(handle, 3, cmd, 8, 10);
                usb_close(handle);
            }
        }

    return 0;
}
