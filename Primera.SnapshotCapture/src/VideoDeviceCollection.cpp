#include <comdef.h>
#include <iomanip>
#include <sstream>
#include <strmif.h>
#include <uuids.h>

#include "VideoDeviceCollection.h"
#include <iostream>

template <typename T>
MaybeMessage<T> maybeFailComMessage(std::wstring description, HRESULT errorCode) {
    std::wstringstream builder;
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
        return maybeFailComMessage<VideoDeviceCollection>(L"Error creating device enumerator", result);
    }

    IEnumMoniker* videoEnumerator = nullptr;
    result = createDeviceEnumerator->CreateClassEnumerator(
        CLSID_VideoInputDeviceCategory,
        &videoEnumerator,
        0);

    if (result != NOERROR) {
        createDeviceEnumerator->Release();
        return maybeFailComMessage<VideoDeviceCollection>(L"Error enumerating video devices", result);
    }

    videoEnumerator->Reset();
    IMoniker* videoList[10];
    ULONG videoDevicesFoundParam;
    if (result = videoEnumerator->Next(10, videoList, &videoDevicesFoundParam), result == S_OK) {
        std::cout << "Discovered max number of video devices. There may be more." << std::endl;
    }

    std::cout << "Discovered " << videoDevicesFoundParam << " video devices." << std::endl;

    int videoDevicesFound = (int) videoDevicesFoundParam;
    VideoCaptureDevice** devices = new VideoCaptureDevice * [videoDevicesFoundParam];  // Use delete to free this memory later
    for (int i = 0; i < videoDevicesFound; i++) {
        IMoniker* nextDevice = videoList[i];

        devices[i] = new VideoCaptureDevice(nextDevice);  // Make sure to use delete to free this memory later
        nextDevice->Release();  // We add a ref inside the VideoCaptureDevice constructor since we are copying the pointer.
    }

    createDeviceEnumerator->Release();
    videoEnumerator->Release();

    return maybeSuccess<VideoDeviceCollection>(new VideoDeviceCollection(devices, videoDevicesFound));
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

VideoCaptureDevice* VideoDeviceCollection::getDeviceAt(int index)
{
    assert(index < _length);

    return _devices[index];
}

int VideoDeviceCollection::length()
{
    return _length;
}