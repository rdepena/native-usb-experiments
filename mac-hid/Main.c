#include <stdio.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

//Define our Hid Manager
static IOHIDManagerRef hidManager = 0x0;

#define BUF_SIZE 256

void getProductDescription(IOHIDDeviceRef device, char* buf);
int getIntDeviceProperty(IOHIDDeviceRef device, CFStringRef key);

int main(void) {
    CFIndex numDevices;
    IOHIDDeviceRef * tIOHIDDeviceRefs = NULL;

    //Init the HID Manager
    //In the docs the following line is written as :hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    //kCFAllocatorDefault: I did not find any docs on it, aparently its NULL. as per email http://lists.apple.com/archives/carbon-dev/2005/Jul/msg01027.html
    //kIOHIDOptionsTypeNone is 0.
    hidManager = IOHIDManagerCreate(NULL, kIOHIDOptionsTypeNone);

    //if we wanted to restrict the devices we can connect to, the second parameter is a deviceMatchDictionary
    IOHIDManagerSetDeviceMatching(hidManager, NULL);

    //Get a list of the Devices
    CFSetRef deviceRefs = IOHIDManagerCopyDevices(hidManager);

    //get the number of devices;
    numDevices = CFSetGetCount(deviceRefs);
    printf("Number of devices connected: %ld\n", numDevices);

    //allocate a block of memory to extact the device ref's from the set into
    IOHIDDeviceRef *devices = calloc(numDevices, sizeof(IOHIDDeviceRef));
    //get the values
    CFSetGetValues(deviceRefs, (const void **) devices);

     //lets iterate over the device list
    for (int i = 0; i < numDevices; ++i) {

        //allocate a block of memory to get the product description.
        char *prodDescription = calloc(BUF_SIZE, sizeof(char));
        int vendorId,
            deviceId;

        //Lets get some of the properties and print them.
        //property keys are defined: IOKit/hid/IOHIDKeys.h
        vendorId = getIntDeviceProperty(devices[i], CFSTR(kIOHIDVendorIDKey));
        deviceId = getIntDeviceProperty(devices[i], CFSTR(kIOHIDProductIDKey));
        getProductDescription(devices[i], prodDescription);

        printf("Vendor ID: %d Product ID: %d\nProduct name: %s\n", vendorId, deviceId, prodDescription);
        free(prodDescription);
    }

    free(tIOHIDDeviceRefs);
    CFRelease(hidManager);
}

//read a int property from the device
int getIntDeviceProperty(IOHIDDeviceRef device, CFStringRef key) {
    CFTypeRef ref = NULL;
    int value = 0;

    ref = IOHIDDeviceGetProperty(device, key);
    CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value);
    return value;
}

//read the product description from the device.
void getProductDescription(IOHIDDeviceRef device, char* buf) {
    CFStringRef str;
    str = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
    CFIndex strLen = CFStringGetLength(str);

    CFRange range;
    range.location = 0;
    range.length = ((size_t)strLen > BUF_SIZE)? BUF_SIZE: (size_t)strLen;
    CFIndex usedBufLen;
    buf[0] = 0;

    CFStringGetBytes(str, range, kCFStringEncodingUTF8, (char)'?', FALSE, (UInt8*)buf,
        BUF_SIZE, &usedBufLen);
}
