void HandleUSBHID();
void InitializeUSBHIDData();
void ProcessUSBDevice();
int GetUSBDeviceState();

/*union PS4Buttons {
        struct {
                uint8_t dpad : 4;
                uint8_t square : 1;
                uint8_t cross : 1;
                uint8_t circle : 1;
                uint8_t triangle : 1;

                uint8_t l1 : 1;
                uint8_t r1 : 1;
                uint8_t l2 : 1;
                uint8_t r2 : 1;
                uint8_t share : 1;
                uint8_t options : 1;
                uint8_t l3 : 1;
                uint8_t r3 : 1;

                uint8_t ps : 1;
                uint8_t touchpad : 1;
                uint8_t reportCounter : 6;
        } __attribute__((packed));
        uint32_t val : 24;
} __attribute__((packed));

struct touchpadXY {
        uint8_t dummy; // I can not figure out what this data is for, it seems to change randomly, maybe a timestamp?
        struct {
                uint8_t counter : 7; // Increments every time a finger is touching the touchpad
                uint8_t touching : 1; // The top bit is cleared if the finger is touching the touchpad
                uint16_t x : 12;
                uint16_t y : 12;
        } __attribute__((packed)) finger[2]; // 0 = first finger, 1 = second finger
} __attribute__((packed));

struct PS4Status {
        uint8_t battery : 4;
        uint8_t usb : 1;
        uint8_t audio : 1;
        uint8_t mic : 1;
        uint8_t unknown : 1; // Extension port?
} __attribute__((packed));

struct PS4Data {
        // Button and joystick values
        uint8_t hatValue[4];
        PS4Buttons btn;
        uint8_t trigger[2];

        // Gyro and accelerometer values
        uint8_t dummy[3]; // First two looks random, while the third one might be some kind of status - it increments once in a while
        int16_t gyroY, gyroZ, gyroX;
        int16_t accX, accZ, accY;

        uint8_t dummy2[5];
        PS4Status status;
        uint8_t dummy3[3];

        // The rest is data for the touchpad
        touchpadXY xy[3]; // It looks like it sends out three coordinates each time, this might be because the microcontroller inside the PS4 controller is much faster than the Bluetooth connection.
                          // The last data is read from the last position in the array while the oldest measurement is from the first position.
                          // The first position will also keep it's value after the finger is released, while the other two will set them to zero.
                          // Note that if you read fast enough from the device, then only the first one will contain any data.

        // The last three bytes are always: 0x00, 0x80, 0x00
} __attribute__((packed));

struct PS4Output {
        uint8_t bigRumble, smallRumble; // Rumble
        uint8_t r, g, b; // RGB
        uint8_t flashOn, flashOff; // Time to flash bright/dark (255 = 2.5 seconds)
        bool reportChanged; // The data is send when data is received from the controller
} __attribute__((packed));
*/