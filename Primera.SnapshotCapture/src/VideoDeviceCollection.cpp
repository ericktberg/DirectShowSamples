#include <comdef.h>
#include <iomanip>
#include <sstream>
#include <strmif.h>
#include <uuids.h>

#include "VideoDeviceCollection.h"
#include <iostream>

template <typename T>
MaybeMessage<T> maybeFailComMessage(std::string description, HRESULT errorCode) {
    std::stringstream builder;
    builder << ": " << std::hex << errorCode;

    return maybeFail<T>(description.append(builder.str()));
}

MaybeMessage<VideoDeviceCollection> acquireVideoDevices() {
    HRESULT result;

    ICreateDevEnum* createDeviceEnumerator = nullptr;
    result = CoCreateInstance(
        CLSID_SystemDeviceEnum,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ICreateDevEnum,
        (void**) &createDeviceEnumerator);

    if (result != NOERROR) {
        return maybeFailComMessage<VideoDeviceCollection>("Error creating device enumerator", result);
    }

    IEnumMoniker* videoEnumerator = nullptr;
    result = createDeviceEnumerator->CreateClassEnumerator(
        CLSID_VideoInputDeviceCategory,
        &videoEnumerator,
        0);

    if (result != NOERROR) {
        createDeviceEnumerator->Release();
        return maybeFailComMessage<VideoDeviceCollection>("Error enumerating video devices", result);
    }

    videoEnumerator->Reset();
    IMoniker* videoList[10];
    ULONG videoDevicesFound;
    if (result = videoEnumerator->Next(10, videoList, &videoDevicesFound), result == S_OK) {
        std::cout << "Discovered max number of video devices. There may be more." << std::endl;
    }

    std::cout << "Discovered " << videoDevicesFound << "video devices." << std::endl;

    VideoCaptureDevice** devices = new VideoCaptureDevice * [videoDevicesFound];  // Use delete to free this memory later
    for (int i = 0; i < videoDevicesFound; i++) {
        IMoniker* nextDevice = videoList[i];

        /* DEBUG */
        IPropertyBag* deviceProperties = NULL;
        HRESULT bindResult = nextDevice->BindToStorage(0, 0, IID_IPropertyBag, (void**) &deviceProperties);
        if (bindResult >= 0) {
            VARIANT var;
            var.vt = VT_BSTR;
            bindResult = deviceProperties->Read(L"FriendlyName", &var, NULL);

            fprintf(stdout, "Device: %s\n", var.bstrVal);
            if (bindResult == NOERROR)
            {
                SysFreeString(var.bstrVal);
            }
        }
        deviceProperties->Release();
        /* DEBUG END */
        
        devices[i] = new VideoCaptureDevice(nextDevice);  // Make sure to use delete to free this memory later
        nextDevice->Release();  // We add a ref inside the VideoCaptureDevice constructor since we are copying the pointer.
    }

    createDeviceEnumerator->Release();
    videoEnumerator->Release();

    return maybeSuccess<VideoDeviceCollection>(new VideoDeviceCollection(devices, (int) videoDevicesFound));
}

VideoDeviceCollection::VideoDeviceCollection(VideoCaptureDevice** devices, int length)
{
    _devices = devices;
    _length = length;
}

VideoDeviceCollection::~VideoDeviceCollection()
{
    for (int i = 0; i < _length; i++) {
        delete _devices[i];
    };
    delete _devices;
}

VideoCaptureDevice** VideoDeviceCollection::allDevices()
{
    return _devices;
}

int VideoDeviceCollection::length()
{
    return _length;
}